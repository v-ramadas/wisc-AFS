#! /usr/bin/env python3

import os
import fs_util
import sys
import time

TEST_DATA_DIR = '/tmp/fs/server_crashes'

def run_test():
    if not fs_util.path_exists(TEST_DATA_DIR):
        fs_util.mkdir(TEST_DATA_DIR)
    if not fs_util.path_exists("/tmp/fs/server_crashes/killed_server_reader.txt"):
        fs_util.create_file("/tmp/fs/server_crashes/killed_server_reader.txt")
    init_str = fs_util.gen_str_by_repeat('0', 32768)
    fd = fs_util.open_file("/tmp/fs/server_crashes/killed_server_reader.txt")    
    fs_util.write_file(fd, init_str)
    try:
        x = fs_util.close_file(fd)
    except:
        ##The Server needs to be killed in the grpc reciever for close function and this 20 sec delay is a timer to allow for server restart, to simulate consistency
	time.sleep(20) ##MANUAL time to restart the server
        fd = fs_util.open_file("/tmp/fs/server_crashes/killed_server_reader.txt")
        cur_str = fs_util.read_file(fd, 32768)
        print(cur_str, len(cur_str), "PASS"  if len(cur_str) == 0  else "FAIL" ); 

        
if __name__ == '__main__':
    run_test()
