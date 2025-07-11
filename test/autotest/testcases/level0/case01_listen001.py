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

from devicetest.core.test_case import TestCase, CheckPoint, get_report_dir
from hypium import UiDriver
import time
from hypium.action.os_hypium.device_logger import DeviceLogger


class case01_listen001(TestCase):

    def __init__(self, configs):
        self.TAG = self.__class__.__name__
        TestCase.__init__(self, self.TAG, configs)
        self.tests = [
            "test_step"
        ]
        self.driver = UiDriver(self.device1)
        self.sn = self.device1.device_sn

    def setup(self):
        self.log.info("case01_listen001 start")
        self.driver.shell("rm -r /data/log/hilog")
        self.driver.shell("hilog -d /system/bin/samgr")

    def test_step(self):
        driver = self.driver
        device_logger = DeviceLogger(driver).set_filter_string("01800")
        device_logger.start_log(get_report_dir() + "//case01_listen001.txt")
        driver.System.execute_command("ps -e | grep resource_schedule_service")
        driver.System.execute_command("kill -9 `pidof resource_schedule_service`")
        time.sleep(5)
        device_logger.stop_log()
        CheckPoint("'AddProc:resource_schedule_service and rm DeadProc:resource_schedule_service' is present in the logs")
        device_logger.check_log("AddProc:resource_schedule_service", EXCEPTION=True)
        device_logger.check_log("rm DeadProc:resource_schedule_service", EXCEPTION=True)

    def teardown(self):
        self.log.info("case01_listen001 down")
