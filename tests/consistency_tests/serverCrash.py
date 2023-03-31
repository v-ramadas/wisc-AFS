#! /usr/bin/env python3

import os
import fs_util
import sys
import time

TEST_DATA_DIR = '/tmp/fs/server_crashes'

def run_test():
    if not fs_util.path_exists(TEST_DATA_DIR):
        fs_util.mkdir(TEST_DATA_DIR)
    if not fs_util.path_exists("/tmp/fs/server_crashes/killed_server.txt"):
        fs_util.create_file("/tmp/fs/server_crashes/killed_server.txt")
    ##PREWRITTEN FILE, This needs to have data to simulate this scenario
    fd = fs_util.open_file("/tmp/fs/server_crashes/killed_server.txt")
    comp_str = fs_util.read_file(fd, 32768)
    cur_str = fs_util.gen_str_by_repeat('a', 100)
    fs_util.write_file(fd, cur_str)
    try:
        x = fs_util.close_file(fd)
    except:
	##THE Server needs to be killed in the grpc reciever of the close function and this 20 sec delay is a timer to allow for server restart, to simulate consistency
        time.sleep(20)
        fd = fs_util.open_file("/tmp/fs/server_crashes/killed_server.txt")
        cur_str = fs_util.read_file(fd, 32768)
        print(cur_str, len(cur_str), "PASS" if comp_str == cur_str else "FAIL")
        
if __name__ == '__main__':
    run_test()
