#!/usr/bin/python
import os
import sys
import time
import json
import threading
from datetime import datetime
from datetime import timedelta
from croniter import croniter
from mydb import MySQL
from depending import DependingTest
from daemon import Daemon
from comm import status_code

import logging
logger = logging.getLogger("sched")
formatter = logging.Formatter('%(name)-12s %(asctime)s %(levelname)-8s \
        %(message)s', '%a, %d %b %Y %H:%M:%S',)
file_handler = logging.FileHandler("/var/log/sched.log")
file_handler.setFormatter(formatter)
stream_handler = logging.StreamHandler(sys.stderr)
logger.addHandler(file_handler)
logger.addHandler(stream_handler)
logger.setLevel(logging.DEBUG)


class Producer(threading.Thread):
    def __init__(self, threadName, dbhandler):
        threading.Thread.__init__(self, name = threadName)
        self.mydb = dbhandler
    def update__sched_paras(self, sched_id, gen_dt):
        sql = """SELECT `id`, `appparas_temp` 
                FROM `sched_status` 
                WHERE `sched_id` = '{0}' 
                AND `gen_dt` = '{1}'
        """.format(sched_id, gen_dt)
        ret = self.mydb.select(sql)

        if ret[0] == False: 
            logger.error(ret[1])
            return ret

        if len(ret[1]) == 0: 
            return True, "{0} with no record".format(' '.join(sql.split(" ")))

        result = ret[1][0]
        appparas_temp = result['appparas_temp']
        sched_status__id = result['id']
        try:
            appparas = gen_dt.strftime(appparas_temp)
        except:
            logger.error('sched_id: {0} appparas_temp error {1}'.format(sched_id, appparas_temp))
            return False, 'appparas_temp error {0}'.format(appparas_temp)

        up = """UPDATE `sched_status` SET `appparas` = '{0}' WHERE `id` = '{1}'
        """.format(appparas, sched_status__id)
        return self.mydb.query(up)

    def update__sched_status(self, sched_id, gen_dt):
        sql = """SELECT `id`, `status` 
            FROM `sched_status` 
            WHERE `sched_id` = '{0}' 
            AND `gen_dt` = '{1}'
        """.format(sched_id, gen_dt)
        ret = self.mydb.select(sql)
        if not ret[0]: return ret
        if len(ret[1]) == 0: 
            insert_sql = """
                INSERT INTO `sched_status` (
                `sched_id`,
                `depends`,
                `hostname`,
                `user_os`,
                `env`,
                `sched_type`,
                `crontab`,
                `priority`,
                `appname`,
                `appparas_temp`,
                `appparas`,
                `dbname`,
                `timeout`,
                `gen_dt`
                ) SELECT
                `sched_id`,
                `depends`,
                `hostname`,
                `user_os`,
                `env`,
                `sched_type`,
                `crontab`,
                `priority`,
                `appname`,
                `appparas_temp`,
                `appparas`,
                `dbname`,
                `timeout`,
                DATE_FORMAT(NOW(), '%Y-%m-%d %H:%i:00') AS `gen_dt`
                FROM `sched_formal`
                WHERE `sched_id` = '{0}' 
            """.format(sched_id)
            logger.info("insert to sched_status")
            return self.mydb.query(insert_sql)

        sched_status__id = ret[1][0]['id']
        sched_status__status = ret[1][0]['status']
        if sched_status__status != 0: 
            return True, 'already update'

        update_sql = """UPDATE `sched_status` SET `status` = '{0}' WHERE `id` = '{1}'
        """.format(status_code['exec'], sched_status__id)

        return self.mydb.query(update_sql)

    def produce(self):
        while True:
            s = time.time()
            ret = self.mydb.select('''SELECT * FROM `sched_formal` WHERE `mstatus` = 0''')
            if ret[0] == False:
                logger.error(ret)
                time.sleep(30)
                continue
            results = ret[1]
            if len(results) == 0: 
                continue
            for x in results: 
                now = datetime.now().replace(second=0, microsecond=0)
                iters = croniter(x['crontab'], now - timedelta(minutes=1))
                iter_next = iters.get_next(datetime)
                if iter_next == now:
                    logger.info('find match time: {0}'.format(now))
                    rt = self.update__sched_status(x['sched_id'], now)
                    if rt[0] == False: logger.error(rt)
                    rt = self.update__sched_paras(x['sched_id'], now)
                    if rt[0] == False: logger.error(rt)
            e = time.time()
            stime = (60 - (e - s + 1.0))
            time.sleep(stime)

    def run(self):
        self.produce()

class Depender(threading.Thread):
    def __init__(self, threadName, dbhandler):
        threading.Thread.__init__(self, name = threadName)
        self.mydb = dbhandler
        self.depending__test = DependingTest(self.mydb)

    def update_status(self, sched_status__id, status):
        sql = "update sched_status set status = '{0}' where id = '{1}'".format(status, sched_status__id)
        logger.info(sql)
        return self.mydb.query(sql)

    def query_every_scheduling_job_update_status(self):
        while True:
            s = time.time()
            sql = '''SELECT `id`, `sched_id`, `crontab`, `status` FROM `sched_status` WHERE `status` = 0'''
            ret = self.mydb.select(sql)

            if ret[0] == False:
                os.exit(-1)
            results = ret[1]
    
            if len(results) == 0:
                logger.info('sleep and continue')
                time.sleep(60)
                continue

            for single_sched in results:
                sched_status__id = single_sched['id']
                sched_status__sched_id = single_sched['sched_id']
                rt = self.depending__test.test_all_depends(sched_status__id)
                if rt[0] == True:
                    rt = self.update_status(sched_status__id, status_code['exec'])
                else: continue
            e = time.time()
            time.sleep(60 - (e - s + 1.0))

    def run(self):
        self.query_every_scheduling_job_update_status()

class MyDaemon(Daemon):
    def run(self):
        mydb__producer_dbhandler = MySQL('localhost', 'root', '123456', 'prajna', 3306)
        producerThread = Producer('producerThread', mydb__producer_dbhandler)
        producerThread.start()

        mydb__depender_dbhandler = MySQL('localhost', 'root', '123456', 'prajna', 3306)
        dependerThread = Depender('dependerThread', mydb__depender_dbhandler)
        dependerThread.start()


if __name__ == '__main__':
    daemon = MyDaemon('/var/run/sched.pid')
    if len(sys.argv) == 2:
        if 'start' == sys.argv[1]:
            daemon.start()
        elif 'stop' == sys.argv[1]:
            daemon.stop()
        elif 'restart' == sys.argv[1]:
            daemon.restart()
        else:
            logger.error('Unknow command')
            sys.exit(2)
        sys.exit(0)
    else:
        print "Usage: %s start|stop|restart" % sys.argv[0]
        sys.exit(0)

