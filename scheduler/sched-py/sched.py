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
    def update__sched_status(self, sched_id, gen_dt):
        sql = """SELECT `id`, `status` FROM `sched_status` WHERE `sched_id` = '{0}' AND `gen_dt` = '{1}'
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
                `cmdline`,
                `appname`,
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
                `cmdline`,
                `appname`,
                `appparas`,
                `dbname`,
                `timeout`,
                DATE_FORMAT(NOW(), '%Y-%m-%d %H:%i:00') AS `gen_dt`
                FROM `sched_formal`
                WHERE `sched_id` = '{0}' 
            """.format(sched_id)
            return self.mydb.query(insert_sql)

        sched_status__id = ret[1][0]['id']
        sched_status__status = ret[1][0]['status']
        logger.debug("status_id: %d" % sched_status__id)
        if sched_status__status != 0: 
            logger.debug('already updated')
            return True, 'already update'

        update_sql = """UPDATE `sched_status` SET `status` = '{0}' WHERE `id` = '{1}'
        """.format(status_code['exec'], sched_status__id)

        return self.mydb.query(update_sql)

    def produce(self):
        while True:
            s = time.time()
            ret = self.mydb.select('''select * from sched_formal''')
            if not ret[0]: continue
            results = ret[1]
            for x in results: 
                now = datetime.now().replace(second=0, microsecond=0)
                iters = croniter(x['crontab'], now - timedelta(minutes=1))
                iter_next = iters.get_next(datetime)
                if iter_next == now:
                    self.update__sched_status(x['sched_id'], now)
                    logger.debug("sched_id: %d, crontab: %s, depends: %s, now: %s" % (x['sched_id'], x['crontab'], x['depends'], str(now)))
            e = time.time()
            stime = (60 - (e - s + 1.0))
            time.sleep(stime)

    def run(self):
        self.produce()

class Depender(threading.Thread):
    def __init__(self, threadName, dbhandler):
        threading.Thread.__init__(self, name = threadName)
        self.mydb = dbhandler
        self.status = {'scheduling': 0, 'exec': 1, 'runing': 2, 'success': 3, 'failed': 4, 'timeout': 5}
        self.depending__test = DependingTest(self.mydb)

    def update_status(self, s_id, status):
        sql = "update sched_status set status = '{0}' where id = '{1}".format(status, s_id)
        return self.mydb.query(sql)

    def query_every_scheduling_job_update_status(self):
        while True:
            s = time.time()
            ''' query from database every time '''
            result = mydb.select('''select * from sched_status where status = 0''')
            if not result:
                time.sleep(60)
                continue

            for single_sched in result:
                self.depending__test.test_all_depends(sched_status__id)

    def run(self):
        self.query_every_scheduling_job_update_status()


if __name__ == '__main__':
    mydb__produce_dbhandler = MySQL('localhost', 'root', '123456', 'prajna', 3306)
    producer = Producer('producerThread', mydb__produce_dbhandler)
    producer.run()

    mydb__depender_dbhandler = MySQL('localhost', 'root', '123456', 'prajna', 3306)
    depender = Depender('dependerThread', mydb__depender_dbhandler)
    depender.run()
