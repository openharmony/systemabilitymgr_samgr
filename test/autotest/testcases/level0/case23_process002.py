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

from devicetest.core.test_case import TestCase, CheckPoint
from devicetest.utils.file_util import get_resource_path
from hypium import *

sa_ondemand_path = get_resource_path(
    "resource/soResource/ondemand",
    isdir=None)


class case23_process002(TestCase):

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
        CheckPoint("Print death callback information")
        result = driver.System.execute_command("ondemand proc initp")
        assert "OnSystemProcessStopped, processName: media_service" in result
        assert "OnSystemProcessStarted, processName: media_service" in result
        target_str = 'OnSystemProcessStopped, processName: media_service'
        count = result.count(target_str)
        assert count == 1
        target_str = 'OnSystemProcessStarted, processName: media_service'
        count = result.count(target_str)
        assert count == 1

    def teardown(self):
        driver = self.driver
        driver.Storage.remove_file("/system/bin/ondemand")
        self.log.info("case23_process002 down")
