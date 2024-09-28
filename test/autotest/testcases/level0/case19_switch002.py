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


class case19_switch002(TestCase):

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
        self.log.info("case19_switch002 start")
        self.driver.Screen.wake_up()
        self.driver.Bluetooth.disable()
        need_source = {"cfg": True, "fwk": False, "listen_test": True, "audio_ability": False, "ondemand": True,
                       "proxy": True, "para": False}
        self.source_path = get_source_path(need_source=need_source, casename="level0/case19_switch002")
        push_source(source_path=self.source_path, driver=self.driver, sn=self.sn)

    def test_step(self):
        driver = self.driver
        result = driver.System.execute_command("hidumper -ls")
        assert "1494" not in result

        driver.Bluetooth.enable()
        result = driver.System.execute_command("hidumper -ls")
        max_wait_time = 10
        wait_time = 0
        while ("1494" not in result and wait_time <= max_wait_time):
            wait_time += 1
            time.sleep(1)
            result = driver.System.execute_command("hidumper -ls")
        CheckPoint("After turning on Bluetooth, 1494 was successfully loaded")
        assert "1494" in result

        driver.Bluetooth.disable()
        time.sleep(20)
        result = driver.System.execute_command("hidumper -ls")
        max_wait_time = 5
        wait_time = 0
        while ("1494" in result and wait_time <= max_wait_time):
            wait_time += 1
            time.sleep(1)
            result = driver.System.execute_command("hidumper -ls")
        CheckPoint("After turning off Bluetooth, 1494 was successfully unloaded")
        assert "1494" not in result

    def teardown(self):
        self.driver.Screen.close()
        remove_source(source_path=self.source_path, driver=self.driver, sn=self.sn)
        self.log.info("case19_switch002 down")
