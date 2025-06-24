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

import time
from devicetest.core.test_case import TestCase, CheckPoint
from hypium import UiDriver
from get_source_path import get_source_path
from push_remove_source import push_source, remove_source


class case14_dysaprofile003(TestCase):

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
        self.log.info("case14_dysaprofile003 start")
        need_source = {"cfg": True, "listen_test": False, "audio_ability": False, "ondemand": False,
                       "proxy": False, "para": False}
        self.source_path = get_source_path(need_source=need_source, casename="level0/case14_dysaprofile003")
        push_source(source_path=self.source_path, driver=self.driver, sn=self.sn)

    def test_step(self):
        driver = self.driver
        driver.System.execute_command("ondemand policy update start 1494 4")
        result = driver.System.execute_command("ondemand policy get start 1494")
        assert "eventId: 4 name:usual.event.SCREEN_ON" in result
        driver.System.execute_command("uinput -T -m 425 1000 425 400;power-shell wakeup")
        max_wait_time = 5
        wait_time = 0
        result = driver.System.execute_command("hidumper -ls")
        while ("1494" not in result and wait_time <= max_wait_time):
            wait_time += 1
            time.sleep(1)
        result = driver.System.execute_command("hidumper -ls")
        CheckPoint(
            "The condition update was successful, and 1494 was successfully loaded after the screen was turned on")
        assert "1494" in result

    def teardown(self):
        remove_source(source_path=self.source_path, driver=self.driver, sn=self.sn)
        self.log.info("case14_dysaprofile003 down")
