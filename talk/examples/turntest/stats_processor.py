#!/usr/bin/env python
from __future__ import division

import os
import json
import sys


class StatsProcessor(object):
    def __init__(self, path):
        self._total_stats = {}
        self._total_stats["mc"] = 0
        self._total_stats["tc"] = 0
        self._total_stats["ae"] = 0
        self._total_stats["an"] = 0
        self._total_stats["be"] = 0
        self._total_stats["bn"] = 0
        self._total_stats["cp"] = 0
        self._total_stats["pc"] = 0
        self._error_lines = []
        self._path = path

    def get_file_list(self):
        log_files = [f for f in os.listdir(self._path) if f.endswith(".log")]
        return log_files

    def process_log_file(self, log_file):
        f = open(self._path + '/' + log_file, "r")
        raw_thread_stats = []
        message_cnt = 0
        thread_cnt = 0
        for json_line in f:
            try:
                data = json.loads(json_line)
            except ValueError:
                error_line = "%s : %s" % (log_file, json_line)
                self._error_lines.append(error_line.rstrip("\n"))
                continue
            if "process_stats" in data:
                self.print_process_stats(data)
                thread_cnt = data["process_stats"]["thread_cnt"]
                message_cnt = thread_cnt * data["process_stats"]["message_cnt"]
                self._total_stats["mc"] += message_cnt
                self._total_stats["tc"] += thread_cnt
            elif "thread_stats" in data:
                raw_thread_stats.append(data)
        if raw_thread_stats:
            self.process_threads_stats(raw_thread_stats)
        else:
            self._error_lines.append("file %s is empty" % log_file)

    def process_threads_stats(self, stats):
        for thread_stat in stats:
            ts = thread_stat["thread_stats"]
            allocation_state = ts["allocation_state"]
            binding_state = ts["binding_state"]
            if allocation_state == 1:
                self._total_stats["ae"] += 1
            elif allocation_state == 5:
                self._total_stats["an"] += 1
            if binding_state == 1:
                self._total_stats["be"] += 1
            elif binding_state == 5:
                self._total_stats["bn"] += 1
            self._total_stats["cp"] += ts["data_client_peer_error_cnt"]
            self._total_stats["pc"] += ts["data_client_peer_error_cnt"]

    def print_process_stats(self, process_stats):
        ps = process_stats["process_stats"]
        print "Process with %(thread_cnt)s threads, from port %(start_port)s" \
                % ps

    def print_total_stats(self):
        self._total_stats["aep"] = (self._total_stats["ae"] /
                                    self._total_stats["tc"]) * 100
        self._total_stats["anp"] = (self._total_stats["an"] /
                                    self._total_stats["tc"]) * 100
        self._total_stats["bep"] = (self._total_stats["be"] /
                                    self._total_stats["tc"]) * 100
        self._total_stats["bnp"] = (self._total_stats["bn"] /
                                    self._total_stats["tc"]) * 100

        self._total_stats["cpp"] = (self._total_stats["cp"] /
                                    self._total_stats["mc"]) * 100
        self._total_stats["pcp"] = (self._total_stats["pc"] /
                                    self._total_stats["mc"]) * 100
        print ""
        print "========================================="
        print "============   Total Stats   ============"
        print "========================================="
        print " Allocations & Binds failures (%(tc)s tot)" % self._total_stats
        print "  Allocation errors: %(ae)s (%(aep)0.2f %%)" % self._total_stats
        print "  Allocation NULLs:  %(an)s (%(anp)0.2f %%)" % self._total_stats
        print "  Bind errors:       %(be)s (%(bep)0.2f %%)" % self._total_stats
        print "  Bind NULLs:        %(bn)s (%(bnp)0.2f %%)" % self._total_stats
        print " Data transfer failures (%(mc)s each way)" % self._total_stats
        print "   Client -> Peer:   %(cp)s (%(cpp)0.2f %%)" % self._total_stats
        print "   Peer -> Client:   %(pc)s (%(pcp)0.2f %%)" % self._total_stats

    def process(self):
        log_files = self.get_file_list()
        if (log_files):
            for log_file in log_files:
                self.process_log_file(log_file)
            self.print_total_stats()
            if (len(self._error_lines) > 0):
                print ""
                print ""
                print "*****************************************"
                print "**************   Errors   ***************"
                print "*****************************************"
                for line in self._error_lines:
                    print line
        else:
            print "No log files found"


def main(log_dir=None):
    if log_dir is None:
        sp = StatsProcessor(".")
    else:
        sp = StatsProcessor(log_dir[0])
    sp.process()

if __name__ == '__main__':
    if len(sys.argv) > 1:
        main(sys.argv[1:])
    else:
        main()
