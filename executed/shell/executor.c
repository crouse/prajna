#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <my_global.h>
#include <mysql.h>
#include <getopt.h>
#include <sys/wait.h>
#include <syslog.h>
#include <errno.h>
#include <sys/types.h>
     
#define CON_NUM 3
#define HOST_LEN 63
#define COMMON_NAME_LEN 63
#define PASS_LEN 31

#define ERROR_INIT_MYSQL 1
#define ERROR_CONNECT_MYSQL 2
#define ERROR_QUERY_MYSQL 3

#define READ_PIPE 0
#define WRITE_PIPE 1
#define STDIN 0
#define STDOUT 1

void help();
int init_db_connections();
void hide_arg(int argc, char** argv, const char* arg);

int child_process(int sched_status__id, char *file, char *const argv[], char *const envp[]);
int do_command(int sched_status__id, char *file, char *argv[], char *envp[]);
FILE* xpopen(int sched_status__id, const char *command, char *const argv[], char *const envp[], const char *mode);
int xpclose(FILE *file);

static void wait_for_child(int sig)
{
    int status;  
    pid_t pid;  

    while( (pid = waitpid(-1,&status,WNOHANG)) > 0)  
    {  
        if ( WIFEXITED(status) )  
        {  
            syslog(LOG_INFO, "child process revoked. pid[%6d], exit code[%d]\n",pid,WEXITSTATUS(status));  
        }  
        else  
            syslog(LOG_INFO, "child process revoked.but ...\n");  
    }  
}

static int get_array_len(char *s) 
{
    int len = 0;
    char *p = s;
    char *q = NULL;

    while(*p != '\0') {
        if ((*p == ' '|| *p == '\t') && *p != *q) {
            len++;
        }   
        q = p;
        p++;
    }   

    return len + 1;
}

int find_then__do_commands(MYSQL *con)
{
    char *id;
    char *user_os;
    char *appname;
    char *apparas;
    int len;

    char *sql = "SELECT `id`, `user_os`, `appname`, `apparas`, `env` FROM `sched_status` ORDER BY `priority`";

    if (mysql_query(con, sql)) {
        goto sql_err;
    }

    MYSQL_RES *result = mysql_store_result(con);
    if (result == NULL) {
        goto sql_err;
    }

    MYSQL_ROW row;
    while((row = mysql_fetch_row(result))) {
        id = row[0];
        user_os = row[1];
        appname = row[2];
        apparas = row[3];

        len = get_array_len(apparas);

        int i = 0;
        char *paras[len + 1];
        paras[len+1] = NULL;
        char *token = strtok(apparas, " ");

        while(token != NULL) {
            paras[i] = token;
            token = strtok(NULL, " ");
            i++;
        }

       // do_command(id, appname, argv, envp);
    }

    mysql_free_result(result);

    return 0;

sql_err: 
    return -1;
    syslog(LOG_ERR, "MYSQL: [%s]", mysql_error(con));
}

char *appname = NULL;
typedef struct {
    MYSQL *con[CON_NUM];
    char hostname[HOST_LEN];
    char username[COMMON_NAME_LEN];
    char password[COMMON_NAME_LEN];
    char dbname[COMMON_NAME_LEN];
} gconf_t;

gconf_t gconf;

typedef struct pinfo {
    FILE         *file;
    pid_t         pid;
    struct pinfo *next;
} pinfo;

static pinfo *plist = NULL;

/*[s] get_opt */
static char* const short_options = "h:u:p:n:";

struct option long_options[] = {
    {"hostname", 1, NULL, 'h'},
    {"username", 1, NULL, 'u'},
    {"password", 1, NULL, 'p'},
    {"dbname", 1, NULL, 'n'},
    {0, 0, 0, 0},
};
/*[e] get opt */

int main(int argc, char *argv[])
{
    memset(&gconf, '\0', sizeof(gconf_t));
    /*[s] set default database name */
    char* default_dbname = "prajna";
    memcpy(gconf.dbname, default_dbname, COMMON_NAME_LEN);
    /*[e] set default database name */

    appname = argv[0];
    if (argc < 7) { help(); }

    int c;
    int initdb = FALSE; // 1: init database if is not created
    while((c = getopt_long(argc, argv, short_options, long_options, NULL)) != -1)
    {
        switch (c) {
            case 'h':
                memcpy(gconf.hostname, optarg, strlen(optarg));
                break;
            case 'u': 
                memcpy(gconf.username, optarg, strlen(optarg));
                break;
            case 'p':
                memcpy(gconf.password, optarg, strlen(optarg));
                break;
            case 'n':
                memset(gconf.dbname, '\0', COMMON_NAME_LEN);
                memcpy(gconf.dbname, optarg, strlen(optarg));
                break;
            default:
                help();
                break;
        }
    }

    hide_arg(argc, argv, "--password");
    hide_arg(argc, argv, "-p");

    daemon(0, 0);

    struct sigaction sa;
    sa.sa_handler = wait_for_child;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    openlog("exector", LOG_PID|LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "exector start");

    init_db_connections();

    /*[logic code here] */
    int m = 2;
    int sched_status__id = 0;
    while(1) {
        sleep(10);
        if (m > 0) {
            do_command(sched_status__id, "/bin/abc", NULL, NULL);
            syslog(LOG_INFO, "[%lu] AFTER DO COMMAND !!!", time(NULL));
        }
        m--;
    }

    closelog();
    return 0;
}

void help()
{
    printf("Usage: %s --hostname xxx --dbname prajna --username xxx --password xxx\n", appname);
    printf("\t\"hostname: mysql hostname\"\n");
    printf("\t\"username: mysql username\"\n");
    printf("\t\"password: mysql password\"\n");
    exit(-1);
}

int init_db_connections() // init database connections
{
   int i;
   for(i = 0; i < CON_NUM; i++) {
       gconf.con[i] = mysql_init(NULL);
       if (gconf.con[i] == NULL) {
           fprintf(stderr, "%s\n", mysql_error(gconf.con[i]));
           return ERROR_INIT_MYSQL;
       }

       if (mysql_real_connect(gconf.con[i], gconf.hostname, gconf.username, \
                   gconf.password, gconf.dbname, 0, NULL, 0) == NULL) {
           fprintf(stderr, "%s\n", mysql_error(gconf.con[i]));
           mysql_close(gconf.con[i]);
           return ERROR_CONNECT_MYSQL;
       }


       if (mysql_query(gconf.con[i], "set character set utf8")) {
           fprintf(stderr, "%s\n", mysql_error(gconf.con[i]));
           mysql_close(gconf.con[i]);
           return ERROR_CONNECT_MYSQL;
       }
   }

   return 0;
}

void hide_arg(int argc, char** argv, const char* arg)
{
    int i;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], arg) || i == (argc - 1)) 
            continue;
        i++;
        int j = strlen(argv[i]);
        for (j = j - 1; j >= 0; j--)
        {
            argv[i][j] = 'x';
        }
    }
}

int child_process(int sched_status__id, char *file, char *const argv[], char *const envp[])
{
#define LINE_LEN 1024
#define BUF_LEN 4096
    FILE *fp;
    int len = 0;
    int line_len;
    int ret;

    char *p;
    char line[LINE_LEN];
    char buf[BUF_LEN];
    char tmp[LINE_LEN];

    memset(buf, '\0', BUF_LEN);

    fp = xpopen(sched_status__id, "/bin/abc", NULL, NULL, "r");

    if (fp != NULL) {
        while(fgets(line, LINE_LEN, fp) != NULL) {
            line_len = strlen(line);

            memcpy(buf + len, line, line_len);
            len += line_len;
            memset(line, '\0', LINE_LEN);
        }
        syslog(LOG_INFO, "<%s>", buf);
    }

    xpclose(fp);
}

int do_command(int sched_status__id, char *file, char *argv[], char *envp[])
{
    pid_t pid;
    /* fork to become asynchronous */
    switch(fork()) {
        case -1:
            break;
        case 0:
            /* child process */
            child_process(sched_status__id, file, argv, envp);
            _exit(0);
            break;
        default:
            break;
    }

    return 0;
}


FILE* xpopen(int sched_status__id, const char *command, char *const argv[], char *const envp[], const char *mode)
{
    int fd[2];
    pinfo *cur, *old;

    if (mode[0] != 'r' && mode[0] != 'w') {
        errno = EINVAL;
        return NULL;
    }

    if (mode[1] != 0) {
        errno = EINVAL;
        return NULL;
    }

    if (pipe(fd)) {
        return NULL;
    }

    cur = (pinfo *) malloc(sizeof(pinfo));
    if (! cur) {
        errno = ENOMEM;
        return NULL;
    }

    cur->pid = fork();
    switch (cur->pid) {
        
    case -1:                    /* fork() failed */
        close(fd[0]);
        close(fd[1]);
        free(cur);
        return NULL;
        
    case 0:                     /* child */
        for (old = plist; old; old = old->next) {
            close(fileno(old->file));
        }
        
        if (mode[0] == 'r') {
            dup2(fd[1], STDOUT_FILENO);
        } else {
            dup2(fd[0], STDIN_FILENO);
        }
        close(fd[0]);   /* close other pipe fds */
        close(fd[1]);

        syslog(LOG_INFO, "execl [%d]", getpid());
        // sched_status__id pid status
        
        execl("/bin/sh", "sh", "-c", command, (char *) NULL);
        _exit(1);

    default:                    /* parent */
        if (mode[0] == 'r') {
            close(fd[1]);
            if (!(cur->file = fdopen(fd[0], mode))) {
                close(fd[0]);
            }
        } else {
            close(fd[0]);
            if (!(cur->file = fdopen(fd[1], mode))) {
                close(fd[1]);
            }
        }
        cur->next = plist;
        plist = cur;
    }

    return cur->file;
}

int xpclose(FILE *file)
{
    pinfo *last, *cur;
    int status;
    pid_t pid;

    /* search for an entry in the list of open pipes */

    for (last = NULL, cur = plist; cur; last = cur, cur = cur->next) {
        if (cur->file == file) break;
    }
    if (! cur) {
        errno = EINVAL;
        return -1;
    }

    /* remove entry from the list */

    if (last) {
        last->next = cur->next;
    } else {
        plist = cur->next;
    }

    /* close stream and wait for process termination */
    
    fclose(file);

    /*
    do {
        pid = waitpid(cur->pid, &status, 0);
    } while (pid == -1 && errno == EINTR);
    syslog(LOG_INFO, "in xclose: [%d]", pid);
    */


    /* release the entry for the now closed pipe */

    free(cur);

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    errno = ECHILD;
    return -1;
}
