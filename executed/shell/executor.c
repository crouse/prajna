#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <my_global.h>
#include <mysql.h>
#include <getopt.h>
#include <sys/wait.h>

#define CON_NUM 3
#define HOST_LEN 63
#define COMMON_NAME_LEN 63
#define PASS_LEN 31

#define ERROR_INIT_MYSQL 1
#define ERROR_CONNECT_MYSQL 2
#define ERROR_QUERY_MYSQL 3

void help();
int init_db_connections();
void hide_arg(int argc, char** argv, const char* arg);
int do_command(const char *file, char *const argv[], char *const envp[]);

char *appname = NULL;
typedef struct {
    MYSQL *con[CON_NUM];
    char hostname[HOST_LEN];
    char username[COMMON_NAME_LEN];
    char password[COMMON_NAME_LEN];
    char dbname[COMMON_NAME_LEN];
} gconf_t;

gconf_t gconf;

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
    //daemon(0, 0);

    init_db_connections();

    /*[logic code here] */
    char *args[] = {NULL};
    char *env[] = {"PATH=/bin:/usr/bin:/usr/local/bin", NULL};

    if(fork() == 0) {
        do_command("/bin/m.sh", args, env);
    }

    while(TRUE) {
        sleep(5);
    }

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

int do_command(const char *file, char *const argv[], char *const envp[])
{
    pid_t pid = fork();
    if (pid == 0) {
        printf("child[%lu]\n", getpid());
        execvpe(file, argv, envp);
    } else if (pid > 0) {
        wait(NULL);
        printf("parent[%lu]\n", getpid());
        printf("parent return\n");
    } else {
        printf("fork error\n");
        return 1;
    }
    return 0;
}
