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
    def __init__(self, threadName):
        threading.Thread.__init__(self, name = threadName)
    def produce(self):
        my = MySQL('localhost', 'root', '123456', 'prajna', 3306)
        while True:
            s = time.time()
            result = my.select('''select * from sched_formal''')
            if not result: continue
            for x in result: 
                now = datetime.now().replace(second=0, microsecond=0)
                iter = croniter(x['crontab'], now - timedelta(minutes=1))
                iter_next = iter.get_next(datetime)
                if iter_next == now:
                    print x['crontab'], now
            e = time.time()
            stime = (60 - (e - s))
            time.sleep(stime)

    def run(self):
        self.produce()

class Depend(threading.Thread):
    def __init__(self, threadName):
        threading.Thread.__init__(self, name = threadName)
        self.my = MySQL('localhost', 'root', '123456', 'prajna', 3306)
        self.status = {'scheduling': 0, 'exec': 1, 'runing': 2, 'success': 3, 'failed': 4, 'timeout': 5}
    def update_status(self, s_id, status):
        sql = "update sched_status set status = '{0}' where id = '{1}".format(status, s_id)
        return self.my(sql)

    def get_single_dep_dtlist_dict(self, single_json_dep_dict, basetime):
        '''
        single_json_dep_dict: { "sched_id": 1, "starttime": "timedelta(hours=-3)", "endtime": "timedelta(hours=5)"}
        basetime: query from sched_status, == gen_dt: select gen_dt from sched_status where id = ??
        single_dep_dtlist: {"sched_id": 2, "datetime_list": [ datetime(2015,4,22,10,30), datetime(2015,4,22,10,35) ...]}"
        '''
        single_dep_dtlist = []
        starttime = basetime + eval(single_json_dep_dict['starttime']) 
        endtime = basetime + eval(single_json_dep_dict['endtime'])

        '''starttime == endtime tells there is only one time depends '''
        if starttime == endtime: 
            return {'sched_id': single_json_dep_dict['sched_id'], 'dep_datetimelist': [starttime]}

        starttime = starttime - timedelta(minutes=1)
        record = my.select(""" select * from sched_status where sched_id = '{0}' """)[0]
        iters = croniter(record['crontab'], starttime)

        next_dt = iters.get_next(datetime)
        while next_dt <= endtime:
            single_dep_dtlist.append(next_dt)

        if single_dep_dtlist:
            return {'sched_id': single_json_dep_dict['sched_id'], 'dep_datetimelist': single_dep_dtlist}

        return None

    def get_single_dep_exec_history_status(self, single_json_dep_dict, basetime):
        '''
        select gen_dt from sched_history where gen_dt not in ('2015-04-22 14:10', '2015-04-22 13:20')
        '''
        #single_dep_dtlist_dict = self.get_single_dep_dtlist_dict(single_json_dep_dict, basetime)

        #return {'st': False, 'error_depends_list': [{'sched_id': 1, 'gen_dt': '2015-04-22 13:10'}, {'sched_id': 1, 'gen_dt': '2015-04-22 13:20'} } ]}
        return {'st': True}

    def query_every_scheduling_job_update_status(self):
        my = MySQL('localhost', 'root', '123456', 'prajna', 3306)
        while True:
            s = time.time()
            ''' query from database every time '''
            result = my.select('''select * from sched_status where status = 0''')
            if not result:
                time.sleep(60)
                continue

            for single_sched in result:
                ''' tbd error deal '''
                deps = json.loads(single_sched['depends'])
                if len(deps) == 0:
                    self.update_status(single_sched['id'], self.status['exec'])
                    continue
                for dep in deps:
                    ret = self.get_single_dep_exec_history_status(dep, single_sched['gen_dt'])
                    if ret['st'] == True:
                        self.update_status(single_sched['id'], self.status['exec'])
                        continue
                    log.info({"id": single_sched['id'], "sched_id": single_sched['sched_id'], "gen_dt": single_sched['gen_dt'], "depends": ret['error_depends_list'])
