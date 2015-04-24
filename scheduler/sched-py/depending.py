#!/usr/bin/python
import os
import json
from datetime import datetime
from datetime import timedelta
from croniter import croniter
from comm import status_code
from mydb import MySQL

class DependingTest(object):
    def __init__(self, dbhandle):
        self.dbhandle = dbhandle

    def get_crontab_by__sched_id(self, sched_id):
        sql = "select crontab from sched_formal where sched_id = {0}".format(sched_id)
        return self.dbhandle.select(sql)

    def test_single_depends(self, basetime, single_dep_dict):
        dep__datetime_list = []
        dep__sched_id = single_dep_dict['sched_id']

        ret = self.get_crontab_by__sched_id(dep__sched_id)
        if ret[0] == False: return ret
        dep__crontab = ret[1][0]['crontab']

        dep__starttime = basetime + eval(single_dep_dict['starttime'])
        dep__endtime = basetime + eval(single_dep_dict['endtime'])
        if dep__starttime == dep__endtime:
            dep__datetime_list.append(dep__starttime)
        else:
            dep__datetime_list.append(dep__starttime)
            iters = croniter(dep__crontab, dep__starttime)
            next_dt = iters.get_next(datetime)
            while next_dt <= dep__endtime: 
                dep__datetime_list.append(next_dt)
                next_dt = iters.get_next(datetime)

        lens = len(dep__datetime_list)
        cond = ""
        if lens == 0: 
            return False, 'error, depends setting'
        elif lens == 1:
            cond = """ ('{0}') """.format(str(dep__datetime_list[0]))
        else:
            cond = """({0})""".format(", ".join(["'%s'" % str(x) for x in dep__datetime_list ]))

        sql = """
            SELECT 
            `id`,
            `sched_id`,
            `depends`,
            `timeout`,
             DATE_FORMAT(`gen_dt`, '%Y-%m-%d %H:%i:%S') as gen_dt,
             DATE_FORMAT(`dt`, '%Y-%m-%d %H:%i:%S') as dt,
            `status`
            FROM `prajna`.`sched_status`
            WHERE `sched_id` = '{0}'
            AND `gen_dt` IN {1}
        """.format(dep__sched_id, cond)

        ret = self.dbhandle.select(sql)
        if ret[0] == False: return ret

        result = ret[1]
        success__job_list = [x for x in result if x['status'] == status_code['success']]
        if len(success__job_list) == lens:
            return True, 'test success'
        failed__job_list = [x for x in result if x['status'] != status_code['success']]
        return False, failed__job_list 

    def test_all_depends(self, sched_status__id):
        status__list = []
        sql = '''
        	SELECT 
            `sched_id`,
            `depends`,
            `timeout`,
            `gen_dt`,
            `dt`,
            `status`
            FROM `prajna`.`sched_status`
            WHERE `id` = '{0}'
        '''.format(sched_status__id)
        ret = self.dbhandle.select(sql)
        if ret[0] == False: return ret
        result = ret[1][0]

        try:
            depends = json.loads(result['depends'])
        except ValueError, e:
            return False, e

        basetime = result['gen_dt']
        sched_status__depends = depends
        if len(sched_status__depends) == 0: 
            return True, 'no depends'
        for single_dep_dict in sched_status__depends:
            ret = self.test_single_depends(basetime, single_dep_dict)
            if ret[0] == False and ret[1]:
                status__list.append(ret[1])
        if len(status__list) == 0:
            return True, 'all succeed'
        else: 
            return False, status__list 

#my = MySQL('localhost', 'root', '123456', 'prajna', 3306)
#d = DependingTest(my)
#d.test_all_depends(11)
