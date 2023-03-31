#! /usr/bin/env python3

import os
import fs_util
import sys
import time
'''
This is ClientA.
'''
duration = 0.080500000
cs739_env_vars = [
    'CS739_CLIENT_A', 'CS739_CLIENT_B', 'CS739_SERVER', 'CS739_MOUNT_POINT'
]
ENV_VARS = {var_name: os.environ.get(var_name) for var_name in cs739_env_vars}
for env_var in ENV_VARS.items():
    print(env_var)
    assert env_var is not None


def run_test():
    FNAME = '/tmp/fs/winner.txt'
    host_b = ENV_VARS['CS739_CLIENT_B']
    print(host_b)
    assert fs_util.test_ssh_access(host_b)
    signal_name_gen = fs_util.get_fs_signal_name()

    # init
    if not fs_util.path_exists(FNAME):
        fs_util.create_file(FNAME)

    cur_signal_name = next(signal_name_gen)

    # time for client_b to work, hzzost_b should read the all-zero file
    fs_util.start_winner_test(host_b, 1, 'B', cur_signal_name)

    fd = fs_util.open_file(FNAME)
    init_str = fs_util.gen_str_by_repeat('a', 1024)
    fs_util.write_file(fd, init_str)
    time.sleep(duration)
    fs_util.close_file(fd)
if __name__ == '__main__':
    run_test()
