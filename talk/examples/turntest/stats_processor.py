#!/usr/bin/env python
# import sys
import os
import json

def get_file_list():
  log_files = [f for f in os.listdir(".") if f.endswith(".log")]
  return log_files

def process_log_file(log_file):
  f = open(log_file, "r")
  raw_thread_stats = []
  for json_line in f:
    data = json.loads(json_line)
    if "process_stats" in data:
      process_stats = print_process_stats(data)
      message_cnt = data["process_stats"]["message_cnt"]
    elif "thread_stats" in data:
      raw_thread_stats.append(data)
  thread_stats = print_threads_stats(raw_thread_stats, message_cnt)
  return {'process_stats': process_stats, 'thread_stats': thread_stats}

def print_process_stats(stats):
  ps = stats["process_stats"];
  print "Process with %s threads, from port %s" % (ps["thread_cnt"], ps["start_port"])

def print_threads_stats(stats, message_cnt):
  thread_cnt = len(stats)
  allocate_null = 0
  allocate_error = 0
  bind_error = 0
  bind_null = 0
  client_to_peer = 0
  peer_to_client = 0
  for thread_stat in stats:
    ts = thread_stat["thread_stats"]
    allocation_state = ts["allocation_state"];
    binding_state = ts["binding_state"];
    if allocation_state == 1:
      allocate_error += 1
    elif allocation_state == 5:
      allocate_null += 1
    if binding_state == 1:
      bind_error += 1
    elif binding_state == 5:
      bind_null += 1

    client_to_peer += ts["data_client_peer_error_cnt"]
    peer_to_client += ts["data_peer_client_error_cnt"]

  print "  Allocations & Binds failures (%s total)" % (thread_cnt)
  print "    Allocation errors: %s" % (allocate_error)
  print "    Allocation NULLs:  %s" % (allocate_null)
  print "    Bind errors:       %s" % (bind_error)
  print "    Bind NULLs:        %s" % (bind_error)
  print "  Data transfer failures (%s each way)" % (message_cnt)
  print "    Client -> Peer:    %s" % (bind_error)
  print "    Peer -> Client:    %s" % (bind_error)
  print "-----------------------------------------"

# Gather our code in a main() function
def main():
  log_files = get_file_list()
  for log_file in log_files:
    process_log_file(log_file)

if __name__ == '__main__':
  main()
