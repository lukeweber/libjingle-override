#!/usr/bin/env python
from __future__ import with_statement
from fabric.api import cd, run, execute, local, env, put, roles, parallel
import sys
import subprocess

env.roledefs = {
  'test_servers' : [
    '127.0.0.1',
    '127.0.0.2',
  ]
}

def pack():
  local('tar cfvj turn.tar.bz2 turntest turntest_wrapper.sh stats_processor.py')

def clean():
  local('rm turn.tar.bz2')

@roles('test_servers')
def deploy():
  put('turn.tar.bz2', '~/')
  run('rm -rf turntest')
  run('mkdir turntest')
  with cd('~/turntest'):
    run('tar xvf ~/turn.tar.bz2')
  run('rm ~/turn.tar.bz2')

@roles('test_servers')
@parallel
def launch():
  cmd = "./turntest_wrapper.sh -t 10 -m 10 -j 1 -r 127.0.0.5 -c %s" % env.host
  with cd('~/turntest'):
    run(cmd)

@roles('test_servers')
def run_stats():
  with cd('~/turntest'):
    run('./stats_processor.py')

@roles('test_servers')
@parallel
def killall():
  """ Run killall turntest on all ther servers """
  run('killall turntest')

@roles('test_servers')
@parallel
def remove():
  """ Remove turntest directory from all the servers """
  run('rm -rf turntest')

def start():
  """ Main method to run """
  pack()
  execute(deploy)
  execute(launch)
  execute(run_stats)
  clean()

if __name__ == '__main__':
    if len(sys.argv) > 1:
        subprocess.call(['fab', '-f', __file__] + sys.argv[1:])
    else:
        subprocess.call(['fab', '-f', __file__, '--list'])
