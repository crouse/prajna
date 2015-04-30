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
#include <stdint.h>

/******************************************/
#define LEN_SQL 254
#define CNT_DB_CON 3
#define DB_CON_QUERY 0
#define DB_CON_UPDATE 1
#define DB_CON_UPDATE2 2
/******************************************/
#define SQL_OK 0
#define SQL_QUERY_ERROR 1
/******************************************/

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

#define APP_SCHED 1
#define APP_EXEC 2
#define APP_SUCCESS 3
#define APP_FAILED 4

char *appname = NULL;
typedef struct {
    MYSQL *con[CNT_DB_CON];
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

void help();
int init_db_cons();
void hide_arg(int argc, char** argv, const char* arg);
int find_then__do_commands();

int child_process(uint64_t sched_status__id, char *user, char *file, char *argv[], char *const envp[]);
int do_command(uint64_t sched_status__id, char *user, char *file, char *argv[], char *envp[]);

FILE* xpopen(uint64_t sched_status__id, const char *user, const char *command, 
        char *argv[], char *const envp[], const char *mode);

int xpclose(FILE *file);
int update__sched_status(uint64_t sched_status__id, int status, pid_t pid);
int update__sched_status_after_wait(int status, pid_t pid);
static void wait_for_child(int sig);

/*[s] get_opt */
static char* const short_options = "h:u:p:n:f";

struct option long_options[] = {
    {"hostname", 1, NULL, 'h'},
    {"username", 1, NULL, 'u'},
    {"password", 1, NULL, 'p'},
    {"dbname", 1, NULL, 'n'},
    {"foreground", 0, NULL, 'f'},
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
    //if (argc < 7) { help(); }

    int c, is_daemon = 1;
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
            case 'f':
                is_daemon = 0;
                break;
            default:
                help();
                break;
        }
    }

    hide_arg(argc, argv, "--password");
    hide_arg(argc, argv, "-p");

    if (is_daemon) {
        daemon(0, 0);
    }

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

    init_db_cons();

    /*[logic code here] */
    while(1) {
        find_then__do_commands();
        sleep(5);
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

int init_db_cons() // init database connections
{
   int i;
   for(i = 0; i < CNT_DB_CON; i++) {
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

int child_process(uint64_t sched_status__id, char *user, char *file, char *argv[], char *const envp[])
{
#define LINE_LEN 1024
#define BUF_LEN 4096
    FILE *fp;
    int len = 0;
    int line_len;

    char line[LINE_LEN];
    char buf[BUF_LEN];

    memset(buf, '\0', BUF_LEN);

    fp = xpopen(sched_status__id, user, file, argv, envp, "r");

    if (fp != NULL) {
        while(fgets(line, LINE_LEN, fp) != NULL) {
            line_len = strlen(line);

            memcpy(buf + len, line, line_len);
            len += line_len;
            memset(line, '\0', LINE_LEN);
        }
        syslog(LOG_INFO, "<%s>", buf);
    }

    return xpclose(fp);
}

int do_command(uint64_t sched_status__id, char *user, char *file, char *argv[], char *envp[])
{
    /* fork to become asynchronous */
    switch(fork()) {
        case -1:
            break;
        case 0:
            /* child process */
            child_process(sched_status__id, user, file, argv, envp);
            _exit(0);
            break;
        default:
            break;
    }

    return 0;
}


FILE* xpopen(uint64_t sched_status__id, const char *user, const char *command, char *argv[], 
        char *const envp[], const char *mode)
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
        syslog(LOG_INFO, "command: [%s]", command);
        update__sched_status(sched_status__id, APP_EXEC, getpid());
        syslog(LOG_INFO, "Here we are\n");
        execvpe(command, argv, NULL);
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
    pid_t pid;
    int status;
    int ret = 0;

    for (last = NULL, cur = plist; cur; last = cur, cur = cur->next) {
        if (cur->file == file) 
            break;
    }

    if (! cur) {
        errno = EINVAL;
        return -1;
    }

    if (last) {
        last->next = cur->next;
    } else {
        plist = cur->next;
    }

    fclose(file);

    do {
        pid = waitpid(cur->pid, &status, 0);
    } while (pid == -1 &errno == EINTR);

    free(cur);

    if (WIFEXITED(status)) {
        ret = WEXITSTATUS(status);
        update__sched_status_after_wait(ret, pid);
        syslog(LOG_INFO, "xpclose PID: [%d], status: [%d]", pid, ret);
        return ret;
    } else {
        ret = -1;
        update__sched_status_after_wait(ret, pid);
        syslog(LOG_INFO, "xpclose PID: [%d], status: [%d]", pid, ret);
        return ret;
    }

    return ret;
}

int find_then__do_commands()
{
    char *id;
    char *user_os;
    char *appname;
    char *apparas;
    char *env;
    char *token;
    int len;
    int i;
    char **argv;
    char *sql = "SELECT id, user_os, appname, appparas, env FROM sched_status where status = 1 ORDER BY priority";

    syslog(LOG_INFO, "find_then__do_commands");

    if (mysql_query(gconf.con[0], sql)) {
        goto sql_err;
    }

    MYSQL_RES *result = mysql_store_result(gconf.con[DB_CON_QUERY]);
    if (result == NULL) {
        goto sql_err;
    }

    MYSQL_ROW row;

    while((row = mysql_fetch_row(result))) {
        id = row[0];
        user_os = row[1];
        appname = row[2];
        apparas = row[3];
        env = row[4];

        printf("[%s]\n", appname);
        char *a[10];

        a[0] = appname;
        i = 1;
        a[i] = strtok(apparas, " ");

        while(a[i] && i < 10) {
            a[++i] = strtok(NULL, " ");
        }
        a[++i] = NULL;

        do_command(atoi(id), user_os, appname, a, NULL);
    }

    mysql_free_result(result);

    return 0;

sql_err: 
    syslog(LOG_ERR, "MYSQL: [%s]", mysql_error(gconf.con[1]));
    return -1;
}

/* [UPDATE TABLE SCHED_STATUS]
 *  call when exec and finished the task
 */
int update__sched_status(uint64_t sched_status__id, int status, pid_t pid)
{
    char sql[LEN_SQL];
    snprintf(sql, LEN_SQL, "UPDATE `sched_status` SET `status` = '%d', \
            `pid` = '%d' WHERE `id` = '%lu'", status, pid, sched_status__id); 

    syslog(LOG_INFO, "SQL[%s]", sql);

    if (mysql_query(gconf.con[DB_CON_UPDATE], sql)) 
    {
        syslog(LOG_ERR, "QUERY ERROR");
        return SQL_QUERY_ERROR;
    }

    return SQL_OK;
}

int update__sched_status_after_wait(int status, pid_t pid)
{
    int st;
    char sql[LEN_SQL];
    if (status == 0) st = APP_SUCCESS;
    else st = APP_FAILED;

    snprintf(sql, LEN_SQL, "UPDATE `sched_status` SET `status` = '%d' WHERE `pid` = '%lu'", st, pid); 

    syslog(LOG_INFO, "SQL[%s]", sql);

    if (mysql_query(gconf.con[DB_CON_UPDATE], sql)) 
    {
        syslog(LOG_ERR, "QUERY ERROR");
        return SQL_QUERY_ERROR;
    }

    return SQL_OK;
}

static void wait_for_child(int sig)
{
    int status;  
    pid_t pid;  

    while((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {  
        if (WIFEXITED(status)) 
        {  
            syslog(LOG_INFO, "EXIST PID: [%d] EXIST STATUS: [%d]", 
                    pid, WEXITSTATUS(status));  
            //update__sched_status_after_wait(status, pid);
        } else {
            syslog(LOG_INFO, "EXIST PID: %d", pid);
            //update__sched_status_after_wait(-1, pid);
        }
    }  
}
