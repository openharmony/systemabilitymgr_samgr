#!/usr/bin/env python3
#-*- coding: utf-8 -*-

# Copyright (c) 2025 Huawei Device Co., Ltd.
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

import os
import sys

current_dir = os.path.abspath(os.path.dirname(__file__))
rootPath = os.path.split(current_dir)[0]
awPath = os.path.split(rootPath)[0]
sys.path.append(rootPath)
sys.path.append(os.path.join(awPath, "aw"))

import os.path
import time
from devicetest.core.test_case import TestCase, Step, CheckPoint, get_report_dir
from hypium import UiDriver
import disk_drop_log
from get_source_path import get_source_path
from push_remove_source import push_source, remove_source
from hypium.action.os_hypium.device_logger import DeviceLogger


class case24_sub001(TestCase):

    def __init__(self, configs):
        self.TAG = self.__class__.__name__
        TestCase.__init__(self, self.TAG, configs)
        self.tests = [
            "test_step"
        ]
        self.driver = UiDriver(self.device1)
        self.sn = self.device1.device_sn
        self.source_path = {}

    def setup(self):
        self.log.info("case24_sub001 start")
        need_source = {"cfg": True, "listen_test": False, "audio_ability": False, "ondemand": False,
                       "proxy": False, "para": False}
        self.source_path = get_source_path(need_source=need_source, casename="level0/case24_sub001")
        push_source(source_path=self.source_path, driver=self.driver, sn=self.sn)

    def test_step(self):
        driver = self.driver
        device_logger = DeviceLogger(driver).set_filter_string("01800")
        device_logger.start_log(get_report_dir() + "//case024_sub001.txt")
        driver.System.execute_command("ondemand sa load 1494")
        result = driver.System.execute_command("hidumper -ls")
        max_wait_time = 10
        wait_time = 0
        while ("1494" not in result and wait_time <= max_wait_time):
            wait_time += 1
            time.sleep(1)
            result = driver.System.execute_command("hidumper -ls")
        CheckPoint("1494 is loaded after booting up")
        assert "1494" in result

        time.sleep(5)
        device_logger.stop_log()  
        CheckPoint("The log contains 'ListenAbility: OnAddSystemAbility systemAbilityId:1901 added!'")
        device_logger.check_log("OnAddSystemAbility systemAbilityId:1901 added!", EXCEPTION=True)
        CheckPoint("The log contains 'ListenAbility: OnAddSystemAbility systemAbilityId:4700 added!'")
        device_logger.check_log("OnAddSystemAbility systemAbilityId:4700 added!", EXCEPTION=True)

    def teardown(self):
        remove_source(source_path=self.source_path, driver=self.driver, sn=self.sn)
        self.log.info("case24_sub001 down")
