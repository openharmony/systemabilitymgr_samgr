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
from hypium import UiDriver
from tools.get_source_path import get_source_path
from tools.push_remove_source import push_source, remove_source


class case20_timer003(TestCase):

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
        self.log.info("case20_timer003 start")
        need_source = {"cfg": True, "fwk": False, "listen_test": True, "audio_ability": False, "ondemand": True,
                       "proxy": True, "para": True}
        self.source_path = get_source_path(need_source=need_source, casename="level0/case20_timer003")
        push_source(source_path=self.source_path, driver=self.driver, sn=self.sn)

    def test_step(self):
        driver = self.driver
        driver.System.execute_command("ondemand param true")
        time.sleep(30)
        max_wait_time = 5
        wait_time = 0
        result = driver.System.execute_command("hidumper -ls")
        while ("1494" not in result and wait_time <= max_wait_time):
            wait_time += 1
            time.sleep(1)
            result = driver.System.execute_command("hidumper -ls")
        CheckPoint("param set to true 1494 successfully loaded")
        assert "1494" in result

        driver.System.execute_command("ondemand param false")
        time.sleep(30)
        result = driver.System.execute_command("hidumper -ls")
        max_wait_time = 20
        wait_time = 0
        while ("1494" in result and wait_time <= max_wait_time):
            wait_time += 1
            time.sleep(1)
            result = driver.System.execute_command("hidumper -ls")
        CheckPoint("Param set to false 1494 and successfully unloaded after 20 seconds")
        assert "1494" not in result

    def teardown(self):
        remove_source(source_path=self.source_path, driver=self.driver, sn=self.sn)
        self.log.info("case20_timer003 down")
