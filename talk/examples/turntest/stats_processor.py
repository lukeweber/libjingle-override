#!/usr/bin/env python
# import sys
import os
import json

class StatsProcessor(object):
  def __init__(self, path):
    self._total_message_cnt = 0
    self._total_thread_cnt = 0
    self._total_allocate_error = 0
    self._total_allocate_null = 0
    self._total_bind_error = 0
    self._total_bind_null = 0
    self._total_client_to_peer = 0
    self._total_peer_to_client = 0

    self._path = path

  def get_file_list(self):
    log_files = [f for f in os.listdir(self._path) if f.endswith(".log")]
    return log_files

  def process_log_file(self, log_file):
    f = open(log_file, "r")
    raw_thread_stats = []
    for json_line in f:
      data = json.loads(json_line)
      if "process_stats" in data:
        self.print_process_stats(data)
        message_cnt = data["process_stats"]["message_cnt"]
        thread_cnt = data["process_stats"]["thread_cnt"]
        self._total_message_cnt += message_cnt
        self._total_thread_cnt += thread_cnt
      elif "thread_stats" in data:
        raw_thread_stats.append(data)
    self.process_threads_stats(raw_thread_stats, message_cnt)

  def process_threads_stats(self, stats, message_cnt):
    thread_cnt = len(stats)
    thread_stats = {}
    thread_stats["allocate_null"] = 0
    thread_stats["allocate_error"] = 0
    thread_stats["bind_error"] = 0
    thread_stats["bind_null"] = 0
    thread_stats["client_to_peer"] = 0
    thread_stats["peer_to_client"] = 0
    for thread_stat in stats:
      ts = thread_stat["thread_stats"]
      allocation_state = ts["allocation_state"];
      binding_state = ts["binding_state"];
      if allocation_state == 1:
        thread_stats["allocate_error"] += 1
      elif allocation_state == 5:
        thread_stats["allocate_null"] += 1
      if binding_state == 1:
        thread_stats["bind_error"] += 1
      elif binding_state == 5:
        thread_stats["bind_null"] += 1

      thread_stats["client_to_peer"] += ts["data_client_peer_error_cnt"]
      thread_stats["peer_to_client"] += ts["data_peer_client_error_cnt"]

    self._total_allocate_error += thread_stats["allocate_error"]
    self._total_allocate_null += thread_stats["allocate_null"]
    self._total_bind_error += thread_stats["bind_error"]
    self._total_bind_null += thread_stats["bind_null"]
    self._total_client_to_peer += thread_stats["client_to_peer"]
    self._total_peer_to_client += thread_stats["peer_to_client"]

    self.print_threads_stats(thread_stats, thread_cnt, message_cnt)

  def print_process_stats(self, process_stats):
    ps = process_stats["process_stats"];
    print "Process with %s threads, from port %s" % (ps["thread_cnt"], ps["start_port"])

  def print_threads_stats(self, stats, thread_cnt, message_cnt):
    print "-----------------------------------------"
    print "  Allocations & Binds failures (%s total)" % (thread_cnt)
    print "    Allocation errors: %s" % (stats["allocate_error"])
    print "    Allocation NULLs:  %s" % (stats["allocate_null"])
    print "    Bind errors:       %s" % (stats["bind_error"])
    print "    Bind NULLs:        %s" % (stats["bind_error"])
    print "  Data transfer failures (%s each way)" % (message_cnt)
    print "    Client -> Peer:    %s" % (stats["bind_error"])
    print "    Peer -> Client:    %s" % (stats["bind_error"])

  def print_total_stats(self):
    print ""
    print "========================================="
    print "============   Total Stats   ============"
    print "========================================="
    print "  Allocations & Binds failures (%s total)" % (self._total_thread_cnt)
    print "    Allocation errors: %s" % (self._total_allocate_error)
    print "    Allocation NULLs:  %s" % (self._total_allocate_null)
    print "    Bind errors:       %s" % (self._total_bind_error)
    print "    Bind NULLs:        %s" % (self._total_bind_error)
    print "  Data transfer failures (%s each way)" % (self._total_message_cnt)
    print "    Client -> Peer:    %s" % (self._total_bind_error)
    print "    Peer -> Client:    %s" % (self._total_bind_error)

  def process(self):
    log_files = self.get_file_list()
    for log_file in log_files:
      self.process_log_file(log_file)
    self.print_total_stats()

# Gather our code in a main() function
def main():
  sp = StatsProcessor(".")
  sp.process()

if __name__ == '__main__':
  main()
