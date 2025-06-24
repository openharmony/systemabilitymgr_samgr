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
from devicetest.core.test_case import TestCase, CheckPoint
from hypium import UiDriver
import disk_drop_log


class case25_selinux001(TestCase):

    def __init__(self, configs):
        self.TAG = self.__class__.__name__
        TestCase.__init__(self, self.TAG, configs)
        self.tests = [
            "test_step"
        ]
        self.driver = UiDriver(self.device1)

    def setup(self):
        self.log.info("case25_selinux001 start")
        self.driver.shell("rm -r /data/log/hilog")
        self.driver.shell("hilog -d /system/bin/samgr")
        self.driver.System.reboot()

    def test_step(self):
        log_revice_path = os.path.join(self.get_case_report_path(), "disk_drop")
        disk_drop_log.pulling_disk_dropping_logs(log_revice_path, self.driver)
        disk_drop_log.parse_disk_dropping_logs(log_revice_path)
        result = disk_drop_log.check_disk_dropping_logs(log_revice_path, "scontext=u:r:samgr:s0")
        CheckPoint("The log does not contain 'scontext=u:r:samgr:s0'")
        assert result is False

    def teardown(self):
        self.log.info("case25_selinux001 done")
