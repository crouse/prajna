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
int child_process(char *file, char *const argv[], char *const envp[]);
int do_command(char *file, char *argv[], char *envp[]);
FILE* xpopen(const char *command, char *const argv[], char *const envp[], const char *mode);
int xpclose(FILE *file);


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

    openlog("exector", LOG_PID|LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "exector start");

    init_db_connections();

    /*[logic code here] */
    while(TRUE) {
        sleep(10);
        do_command("/bin/abc", NULL, NULL);
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

int child_process(char *file, char *const argv[], char *const envp[])
{
    FILE *fp;
    char line[1024];
    char buf[4096];
    char *p;
    char tmp[1024];
    int len = 0;
    int ret;
    fp = xpopen("/bin/abc", NULL, NULL, "r");
    if (fp != NULL) {
        while(fgets(line, 1024, fp) != NULL) {
            len += snprintf(buf + len, strlen(line), "%s", line);
            syslog(LOG_INFO, "%s", line);
        }
    }

    syslog(LOG_INFO, "%s", buf);
    syslog(LOG_INFO, "len: %d", len);

    ret = xpclose(fp);
    syslog(LOG_INFO, "xclose return: %d", ret);
}

int do_command(char *file, char *argv[], char *envp[])
{
    pid_t pid;
    syslog(LOG_INFO, "do_command");
    /* fork to become asynchronous */
    switch(fork()) {
        case -1:
            syslog(LOG_ERR, "pid: [%d] fork error", getpid());
            break;
        case 0:
            /* child process */
            child_process(file, argv, envp);
            syslog(LOG_INFO, "[%d] child process done, exiting\n", getpid());
            _exit(0);
            break;
        default:
            pid = wait(NULL);
            syslog(LOG_INFO, "do_command [%d]\n", pid);
            break;
    }
}


FILE* xpopen(const char *command, char *const argv[], char *const envp[], const char *mode)
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
        syslog(LOG_INFO, "xpopen pid:%d", getpid());
        
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
    do {
        pid = waitpid(cur->pid, &status, 0);
    } while (pid == -1 && errno == EINTR);

    syslog(LOG_INFO, "xpclose: [%d]", getpid());

    /* release the entry for the now closed pipe */

    free(cur);

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    errno = ECHILD;
    return -1;
}
