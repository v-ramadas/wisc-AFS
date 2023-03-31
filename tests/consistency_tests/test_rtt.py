#! /usr/bin/env python3

import os
import fs_util
import sys
import time
'''
This is ClientA.
'''
def run_test():
    FNAME = '/tmp/fs/rtt.txt'
    if not fs_util.path_exists(FNAME):
        fs_util.create_file(FNAME)

    fd = fs_util.open_file(FNAME)
    init_str = fs_util.gen_str_by_repeat('b', 1*16*1024)
    fs_util.write_file(fd, init_str)
    fs_util.close_file(fd)
if __name__ == '__main__':
    run_test()
