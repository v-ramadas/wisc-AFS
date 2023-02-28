#! /usr/bin/env python3

import os
import fs_util
import sys
import test1_clientA as test1  # Simply do not want to do test1_common
'''
This is ClientB.
'''

TEST_DATA_DIR = '/tmp/fs/test_consistency'
FNAME = f'{TEST_DATA_DIR}/case1'

def run_test():
    print(test1.FNAME)
    signal_name_gen = fs_util.get_fs_signal_name()

    cur_signal_name = next(signal_name_gen)
    fs_util.record_test_result(test1.TEST_CASE_NO, 'B',
                               f'START fname:{FNAME}')
    fs_util.wait_for_signal(cur_signal_name)

    # first execution, read all-zero file
    if not fs_util.path_exists(FNAME):
        fs_util.record_test_result(test1.TEST_CASE_NO, 'B', 'not exist')
        sys.exit(1)
    last_signal_name = cur_signal_name
    cur_signal_name = next(signal_name_gen)
    fs_util.wait_for_signal(cur_signal_name, last_signal_name=last_signal_name)

   
    read = os.remove(FNAME)
    
    fs_util.record_test_result(test1.TEST_CASE_NO, 'B', 'OK')


if __name__ == '__main__':
    run_test()
