CREATE DATABASE IF NOT EXISTS `prajna`;

USE `prajna`;

CREATE TABLE `sched_status` (
  `id` int(11) NOT NULL AUTO_INCREMENT COMMENT 'Id',
  `sched_id` int(11) NOT NULL COMMENT '任务唯一ID 1',
  `depends` varchar(2047) DEFAULT NULL COMMENT '任务依赖关系',
  `hostname` varchar(31) NOT NULL DEFAULT 'localhost' COMMENT '主机名',
  `user_os` varchar(31) NOT NULL DEFAULT 'root' COMMENT '操作系统用户',
  `env` varchar(255) DEFAULT NULL COMMENT '环境变量，默认为空，则使用系统环境变量',
  `sched_type` tinyint(4) DEFAULT '0' COMMENT '0: bash, 1: hadoop, 2: spark, 3:other',
  `crontab` varchar(127) NOT NULL COMMENT 'crontab 表达式',
  `priority` tinyint(3) unsigned DEFAULT '0' COMMENT '执行优先级，越小优先级越大',
  `appname` varchar(63) DEFAULT NULL COMMENT '程序名称，或者绝对路径',
  `appparas_temp` varchar(511) DEFAULT NULL COMMENT '参数模板',
  `appparas` varchar(511) DEFAULT NULL COMMENT '程序参数列表，根据模板自动生成',
  `dbname` varchar(63) DEFAULT NULL COMMENT '如果执行存过，则提供数据库名称, 其他情况为空',
  `timeout` int(3) unsigned NOT NULL DEFAULT '30' COMMENT '程序执行超时时间，默认为30分钟',
  `desc` varchar(255) DEFAULT NULL COMMENT '任务描述',
  `gen_dt` datetime DEFAULT NULL COMMENT '生成此记录的时间，也就是调度开始执行时间，并非实际执行时间',
  `dt` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '修改时间',
  `status` tinyint(4) NOT NULL DEFAULT '0' COMMENT '运行状态: #0: 未执行 #1: 可执行 #2: 执行成功 #3 执行失败 #4: 超时',
  `pid` int(11) DEFAULT '-1' COMMENT '进程PID, 如果是操作系统进程，则有此项',
  `label` varchar(63) DEFAULT NULL COMMENT 'HADOOP，或SPARK的进程号，待议',
  PRIMARY KEY (`id`),
  KEY `sched_id` (`sched_id`),
  CONSTRAINT `sched_status_ibfk_1` FOREIGN KEY (`sched_id`) REFERENCES `sched_formal` (`sched_id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

CREATE TABLE `sched_formal` (
  `sched_id` int(11) NOT NULL AUTO_INCREMENT COMMENT '任务唯一ID 1',
  `depends` varchar(2048) DEFAULT NULL COMMENT '任务依赖关系',
  `hostname` varchar(31) NOT NULL DEFAULT 'localhost' COMMENT '主机名',
  `user_os` varchar(31) NOT NULL DEFAULT 'root' COMMENT '操作系统用户',
  `env` varchar(255) DEFAULT NULL COMMENT '环境变量，默认为空，则使用系统环境变量',
  `sched_type` tinyint(4) DEFAULT '0' COMMENT '0: bash, 1: hadoop, 2: spark, 3:other',
  `crontab` varchar(127) NOT NULL COMMENT 'crontab 表达式',
  `priority` tinyint(3) unsigned DEFAULT '0' COMMENT '执行优先级，越小优先级越大',
  `appname` varchar(63) DEFAULT NULL COMMENT '程序名称，或者绝对路径',
  `appparas_temp` varchar(511) DEFAULT NULL COMMENT '参数模板',
  `appparas` varchar(511) DEFAULT NULL COMMENT '程序参数列表',
  `dbname` varchar(63) DEFAULT NULL COMMENT '如果执行存过，则提供数据库名称, 其他情况为空',
  `timeout` int(3) unsigned NOT NULL DEFAULT '30' COMMENT '程序执行超时时间，默认为30分钟',
  `desc` varchar(255) DEFAULT NULL COMMENT '任务描述',
  `dt` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '修改时间',
  `mstatus` tinyint(3) unsigned DEFAULT '0' COMMENT '任务状态, #0: 正常 #1: 删除 #2 过期',
  PRIMARY KEY (`sched_id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;
