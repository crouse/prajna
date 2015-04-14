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

void help();
int init_db_connections();
int create_tables();

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
static char* const short_options = "h:u:p:n:i";

struct option long_options[] = {
    {"hostname", 1, NULL, 'h'},
    {"username", 1, NULL, 'u'},
    {"password", 1, NULL, 'p'},
    {"dbname", 1, NULL, 'n'},
    {"init", 0, NULL, 'i'},
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
            case 'i':
                initdb = TRUE;
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

    init_db_connections();
    if (initdb == TRUE) create_tables();
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

int create_tables()
{
    char *t1 = "CREATE TABLE IF NOT EXISTS `sched_manage` (\
                `sched_id` int(11) NOT NULL AUTO_INCREMENT COMMENT '任务唯一ID，自增',\
                `depends` varchar(255) DEFAULT NULL COMMENT '任务依赖关系',\
                `hostname` varchar(31) NOT NULL DEFAULT 'localhost' COMMENT '主机名',\
                `user_os` varchar(31) NOT NULL DEFAULT 'root' COMMENT '操作系统用户',\
                `env` varchar(255) DEFAULT NULL COMMENT '环境变量，默认为空，则使用系统环境变量',\
                `sched_type` tinyint(4) DEFAULT '0' COMMENT '0: bash, 1: hadoop, 2: spark, 3:other',\
                `crontab` varchar(127) NOT NULL COMMENT 'crontab 表达式',\
                `priority` tinyint(3) unsigned DEFAULT '0' COMMENT '执行优先级，越小优先级越大',\
                `cmdline` varchar(255) DEFAULT NULL COMMENT 'bash 任务，命令行',\
                `appname` varchar(63) DEFAULT NULL COMMENT '程序名称，或者绝对路径',\
                `appparas` varchar(511) DEFAULT NULL COMMENT '程序参数列表',\
                `dbname` varchar(63) DEFAULT NULL COMMENT '如果执行存过，则提供数据库名称, 其他情况为空',\
                `timeout` int(3) unsigned NOT NULL DEFAULT '30' COMMENT '程序执行超时时间，默认为30分钟',\
                `cstatus` tinyint(3) unsigned NOT NULL DEFAULT '0' COMMENT '0: 新添加未调度, 1: 已经调度, 2: 修改, 3: 需要删除',\
                `desc` varchar(255) DEFAULT NULL COMMENT '任务描述',\
                `dt` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '修改时间',\
                PRIMARY KEY (`sched_id`)\
                ) ENGINE=InnoDB DEFAULT CHARSET=utf8;\
                ";

    if (mysql_query(gconf.con[0], t1)) goto MY_ERROR; // create table sched_manage

    char *t2 = "CREATE TABLE IF NOT EXISTS `sched_formal` (\
                `sched_id` int(11) NOT NULL COMMENT '任务唯一ID，取自表: sched_manage: sched_id',\
                `depends` varchar(255) DEFAULT NULL COMMENT '任务依赖关系',\
                `hostname` varchar(31) NOT NULL DEFAULT 'localhost' COMMENT '主机名',\
                `user_os` varchar(31) NOT NULL DEFAULT 'root' COMMENT '操作系统用户',\
                `env` varchar(255) DEFAULT NULL COMMENT '环境变量，默认为空，则使用系统环境变量',\
                `sched_type` tinyint(4) DEFAULT '0' COMMENT '0: bash, 1: hadoop, 2: spark, 3:other',\
                `crontab` varchar(127) NOT NULL COMMENT 'crontab 表达式',\
                `priority` tinyint(3) unsigned DEFAULT '0' COMMENT '执行优先级，越小优先级越大',\
                `cmdline` varchar(255) DEFAULT NULL COMMENT 'bash 任务，命令行',\
                `appname` varchar(63) DEFAULT NULL COMMENT '程序名称，或者绝对路径',\
                `appparas` varchar(511) DEFAULT NULL COMMENT '程序参数列表',\
                `dbname` varchar(63) DEFAULT NULL COMMENT '如果执行存过，则提供数据库名称, 其他情况为空',\
                `timeout` int(3) unsigned NOT NULL DEFAULT '30' COMMENT '程序执行超时时间，默认为30分钟',\
                `desc` varchar(255) DEFAULT NULL COMMENT '任务描述',\
                `dt` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '修改时间',\
                PRIMARY KEY (`sched_id`)\
                ) ENGINE=InnoDB DEFAULT CHARSET=utf8;\
                ";

    if (mysql_query(gconf.con[0], t2)) goto MY_ERROR; // create table sched_manage

    char *t3 = "CREATE TABLE IF NOT EXISTS `sched_status` (\
                `sched_id` int(11) NOT NULL COMMENT '任务唯一ID，取自表: sched_manage: sched_id',\
                `depends` varchar(255) DEFAULT NULL COMMENT '任务依赖关系',\
                `hostname` varchar(31) NOT NULL DEFAULT 'localhost' COMMENT '主机名',\
                `user_os` varchar(31) NOT NULL DEFAULT 'root' COMMENT '操作系统用户',\
                `env` varchar(255) DEFAULT NULL COMMENT '环境变量，默认为空，则使用系统环境变量',\
                `sched_type` tinyint(4) DEFAULT '0' COMMENT '0: bash, 1: hadoop, 2: spark, 3:other',\
                `crontab` varchar(127) NOT NULL COMMENT 'crontab 表达式',\
                `priority` tinyint(3) unsigned DEFAULT '0' COMMENT '执行优先级，越小优先级越大',\
                `cmdline` varchar(255) DEFAULT NULL COMMENT 'bash 任务，命令行',\
                `appname` varchar(63) DEFAULT NULL COMMENT '程序名称，或者绝对路径',\
                `appparas` varchar(511) DEFAULT NULL COMMENT '程序参数列表',\
                `dbname` varchar(63) DEFAULT NULL COMMENT '如果执行存过，则提供数据库名称, 其他情况为空',\
                `timeout` int(3) unsigned NOT NULL DEFAULT '30' COMMENT '程序执行超时时间，默认为30分钟',\
                `runstatus` tinyint(3) unsigned NOT NULL DEFAULT '0' COMMENT '运行状态，0：未运行，1：运行中 2：超时 3：错误',\
                `desc` varchar(255) DEFAULT NULL COMMENT '任务描述',\
                `dt` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '修改时间',\
                PRIMARY KEY (`sched_id`)\
                ) ENGINE=InnoDB DEFAULT CHARSET=utf8;\
                ";

    if (mysql_query(gconf.con[0], t3)) goto MY_ERROR; // create table sched_manage

    char *t4 = "CREATE TABLE IF NOT EXISTS `sched_history` (\
                `sched_id` int(11) NOT NULL COMMENT '任务唯一ID，取自表: sched_manage: sched_id',\
                `depends` varchar(255) DEFAULT NULL COMMENT '任务依赖关系',\
                `hostname` varchar(31) NOT NULL DEFAULT 'localhost' COMMENT '主机名',\
                `user_os` varchar(31) NOT NULL DEFAULT 'root' COMMENT '操作系统用户',\
                `env` varchar(255) DEFAULT NULL COMMENT '环境变量，默认为空，则使用系统环境变量',\
                `sched_type` tinyint(4) DEFAULT '0' COMMENT '0: bash, 1: hadoop, 2: spark, 3:other',\
                `crontab` varchar(127) NOT NULL COMMENT 'crontab 表达式',\
                `priority` tinyint(3) unsigned DEFAULT '0' COMMENT '执行优先级，越小优先级越大',\
                `cmdline` varchar(255) DEFAULT NULL COMMENT 'bash 任务，命令行',\
                `appname` varchar(63) DEFAULT NULL COMMENT '程序名称，或者绝对路径',\
                `appparas` varchar(511) DEFAULT NULL COMMENT '程序参数列表',\
                `dbname` varchar(63) DEFAULT NULL COMMENT '如果执行存过，则提供数据库名称, 其他情况为空',\
                `timeout` int(3) unsigned NOT NULL DEFAULT '30' COMMENT '程序执行超时时间，默认为30分钟',\
                `desc` varchar(255) DEFAULT NULL COMMENT '任务描述',\
                `dt` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '修改时间',\
                PRIMARY KEY (`sched_id`)\
                ) ENGINE=InnoDB DEFAULT CHARSET=utf8;\
                ";

    if (mysql_query(gconf.con[0], t4)) goto MY_ERROR; // create table sched_manage

    return 0;

MY_ERROR:
    fprintf(stderr, "[%d] %s\n", ERROR_QUERY_MYSQL, mysql_error(gconf.con[0]));
    mysql_close(gconf.con[0]);
    return ERROR_QUERY_MYSQL;
}
