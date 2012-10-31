#!/usr/bin/env python
from __future__ import with_statement
from fabric.api import cd, run, execute, local, env, put, roles, parallel
import sys
import subprocess

env.roledefs = {
    'test_servers': [
        '127.0.0.1',
        '127.0.0.2',
    ]
}


def pack():
    local('tar cfvj turn.tar.bz2 turntest turntest_wrapper.sh')


def clean():
    local('rm turn.tar.bz2')


@roles('test_servers')
def upload():
    put('turn.tar.bz2', '~/')
    run('rm -rf turntest')
    run('mkdir turntest')
    with cd('~/turntest'):
        run('tar xvf ~/turn.tar.bz2')
        run('rm ~/turn.tar.bz2')


@roles('test_servers')
@parallel
def launch():
    cmd = "./turntest_wrapper.sh -t 1 -m 10 -j 1 -r 127.0.0.5 -c %s" % env.host
    with cd('~/turntest'):
        run(cmd)


@roles('test_servers')
@parallel
def killall():
    """ Run killall turntest on the servers """
    run('killall turntest')


@roles('test_servers')
@parallel
def remove():
    """ Remove turntest directory from the servers """
    run('rm -rf turntest')


def deploy():
    """ Deploy a turntest suite on the servers """
    pack()
    execute(upload)


@roles('test_servers')
@parallel
def collect_stats():
    pass


def show_stats():
    pass


def start():
    """ Execute test and show stats """
    execute(launch)
    execute(collect_stats)
    show_stats()


if __name__ == '__main__':
    if len(sys.argv) > 1:
        subprocess.call(['fab', '-f', __file__] + sys.argv[1:])
    else:
        print "1) deploy"
        print "2) start"
        print "3) remove if necessary"
        print "4) ???"
        print "5) Profit!"
        print ""
        subprocess.call(['fab', '-f', __file__, '--list'])
