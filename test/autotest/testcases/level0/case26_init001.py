#!/usr/bin/env python3
#-*- coding: utf-8 -*-

# Copyright (c) 2024 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import time
from devicetest.core.test_case import TestCase, CheckPoint
from hypium import *
from hypium.action.host import host
import subprocess
import shlex


def run_command_with_timeout(command, timeout):
    proc = subprocess.Popen(shlex.split(command))
    start_time = time.monotonic()
    while proc.poll() is None:
        time.sleep(0.1)
        if time.monotonic() - start_time > timeout:
            proc.kill()
            return -1
    return proc.returncode


class case26_init001(TestCase):

    def __init__(self, configs):
        self.TAG = self.__class__.__name__
        TestCase.__init__(self, self.TAG, configs)
        self.tests = [
            "test_step"
        ]
        self.driver = UiDriver(self.device1)
        self.sn = self.device1.device_sn

    def setup(self):
        pass

    def test_step(self):
        driver = self.driver
        CheckPoint("First kill samgr, enter fastboot or restart")
        result = driver.System.get_pid("samgr")
        assert result is not None
        time.sleep(1)
        host.shell("hdc -t {} shell kill -9 `pidof samgr`".format(self.sn))
        command = "fastboot reboot"
        timeout = 3
        current_number = 0
        max_number = 10
        return_code = -1
        while current_number < max_number and return_code == -1:
            return_code = run_command_with_timeout(command, timeout)
            current_number += 1
        if return_code == 0:
            time.sleep(30)

        CheckPoint("Second kill samgr, enter fastboot or restart")
        result = driver.System.get_pid("samgr")
        assert result is not None
        host.shell("hdc -t {} shell kill -9 `pidof samgr`".format(self.sn))
        current_number = 0
        return_code = -1
        while current_number < max_number and return_code == -1:
            return_code = run_command_with_timeout(command, timeout)
            current_number += 1
        if return_code == 0:
            time.sleep(30)

    def teardown(self):
        self.log.info("case26_init001 done")
