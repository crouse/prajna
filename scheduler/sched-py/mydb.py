#!/usr/bin/env python

import sys
import time
import MySQLdb
import traceback

class MySQL(object):
    def __init__(
            self, 
            host='localhost', 
            user = 'root',
            passwd = '123456', 
            db = 'prajna',
            port = 3306, 
            charset = 'utf8'
            ):
        self.host = host
        self.user = user
        self.passwd = passwd
        self.db = db
        self.port = port
        self.charset = charset
        self.conn = None
        self._conn()

    def _conn(self):
        try:
            self.conn = MySQLdb.connect(self.host, self.user, self.passwd, self.db, port=3306, charset=self.charset)
            return True
        except:
            return False

    def _reConn(self, num = 28800, stime = 3):
            _number = 0
            _status = True
            while _status and _number <= num:
                try:
                    self.conn.ping()
                    _status = False
                except:
                    if self._conn() == True:
                        _status = False
                        break
                    _number +=1
                    time.sleep(stime)
    def select(self, sql=''):
        try:
            self._reConn()
            self.cursor = self.conn.cursor(MySQLdb.cursors.DictCursor)
            self.conn.commit()
            self.cursor.execute(sql)
            result = self.cursor.fetchall()
            self.cursor.close()
            return True, result
        except MySQLdb.Error, e:
            return False, e

    def query(self, sql=''):
        try:
            self._reConn() 
            self.cursor = self.conn.cursor(MySQLdb.cursors.DictCursor)
            self.cursor.execute("set names utf8")
            result = self.cursor.execute(sql) 
            self.conn.commit()
            self.cursor.close()
            return True, result
        except MySQLdb.Error, e:
            return False, e

    def close(self):
        self.conn.close()


#if __name__ == '__main__':
#    my = MySQL('localhost', 'root', '123456', 'prajna', 3306)
#    print my.select('select dt from sched_status where sched_id = 2')
#    my.close()
