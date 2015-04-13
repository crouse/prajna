#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <my_global.h>
#include <mysql.h>
#include <getopt.h>

#define CON_NUM 3
#define HOST_LEN 63
#define COMMON_NAME_LEN 63
#define PASS_LEN 31

#define ERROR_INIT_MYSQL 1
#define ERROR_CONNECT_MYSQL 2
#define ERROR_QUERY_MYSQL 3

int create_dbs();

char *appname = NULL;
typedef struct {
    MYSQL *con[CON_NUM];
    char hostname[HOST_LEN];
    char username[COMMON_NAME_LEN];
    char password[COMMON_NAME_LEN];
    char dbname[COMMON_NAME_LEN];
} gconf_t;

gconf_t gconf;

int init_db() // init database connections
{
   int i;
   for(i = 0; i < CON_NUM; i++) {
       gconf.con[i] = mysql_init(NULL);
       if (gconf.con[i] == NULL) {
           fprintf(stderr, "%s\n", mysql_error(gconf.con[i]));
           return ERROR_INIT_MYSQL;
       }

       if (mysql_real_connect(gconf.con[i], gconf.hostname, gconf.username, \
                   gconf.password, NULL, 0, NULL, 0) == NULL) {
           fprintf(stderr, "%s\n", mysql_error(gconf.con[i]));
           mysql_close(gconf.con[i]);
           return ERROR_CONNECT_MYSQL;
       }
   }

   return 0;
}

/*[s] get_opt */
static char* const short_options = "h:u:p:i";

struct option long_options[] = {
    {"hostname", 1, NULL, 'h'},
    {"username", 1, NULL, 'u'},
    {"password", 1, NULL, 'p'},
    {"init", 0, NULL, 'i'},
    {0, 0, 0, 0},
};
/*[e] get opt */


void help()
{
    printf("Usage: %s --hostname xxx --username xxx --password xxx\n", appname);
    exit(-1);
}

int main(int argc, char *argv[])
{
    appname = argv[0];
    if (argc != 7) { help(); }

    int c;
    int initdb = 0; // 1: init database if is not created
    while((c = getopt_long(argc, argv, short_options, long_options, NULL)) != -1)
    {
        switch (c) {
            case 'h':
                memcpy(gconf.hostname, optarg, strlen(optarg));
                break;
            case 'u': 
                memcpy(gconf.dbname, optarg, strlen(optarg));
                break;
            case 'p':
                memcpy(gconf.password, optarg, strlen(optarg));
                break;
            case 'i':
                initdb = 1;
            default:
                help();
                break;
        }
    }

    init_db();
    create_dbs();
    return 0;
}

int create_dbs()
{
    char *dbsql = "create database prajna;";
    if (mysql_query(gconf.con[0], dbsql)) {
        fprintf(stderr, "%s\n", mysql_error(gconf.con[0]));
        mysql_close(gconf.con[0]);
        return ERROR_QUERY_MYSQL;
    }

    char *sql = "\
                 CREATE TABLE `sched_modify`
        ";
    return 0;
}



























