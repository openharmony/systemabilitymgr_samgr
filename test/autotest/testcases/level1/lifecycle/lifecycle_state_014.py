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


class lifecycle_state_014(TestCase):

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
        self.log.info("lifecycle_state_014 start")
        need_source = {"cfg": True, "fwk": False, "listen_test": True, "audio_ability": False, "ondemand": True,
                       "proxy": True, "para": True}
        self.source_path = get_source_path(need_source=need_source, casename="level1/lifecycle/lifecycle_state_014")
        push_source(source_path=self.source_path, driver=self.driver, sn=self.sn)

    def test_step(self):
        driver = self.driver
        device_logger = DeviceLogger(driver).set_filter_string("Scheduler SA:1494")
        device_logger.start_log(get_report_dir() + "//lifecycle_state_014.txt")
        driver.System.execute_command("ondemand test 14")
        device_logger.stop_log()
        device_logger.check_log("Scheduler SA:1494 loaded", EXCEPTION=True)
        device_logger.check_log("Scheduler SA:1494 unloadable", EXCEPTION=True)
        device_logger.check_log("Scheduler SA:1494 not loaded", EXCEPTION=True)

    def teardown(self):
        remove_source(source_path=self.source_path, driver=self.driver, sn=self.sn)
        self.log.info("lifecycle_state_014 down")
