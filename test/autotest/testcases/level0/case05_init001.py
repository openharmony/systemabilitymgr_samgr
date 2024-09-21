"""
 Copyright (c) 2024 Huawei Device Co., Ltd.
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
"""

from devicetest.core.test_case import TestCase, CheckPoint
from hypium import *
from hypium.action.host import host


class case05_init001(TestCase):

    def __init__(self, configs):
        self.TAG = self.__class__.__name__
        TestCase.__init__(self, self.TAG, configs)
        self.tests = [
            "test_step"
        ]
        self.driver = UiDriver(self.device1)
        self.sn = self.device1.device_sn

    def setup(self):
        host.shell("hdc -t {} shell rm -r /data/log/hilog".format(self.sn))

    def test_step(self):
        driver = self.driver
        result = driver.System.execute_command("param get bootevent.samgr.ready")
        CheckPoint("The system can detect the startup event")
        assert "true" in result

    def teardown(self):
        self.log.info("case05_init001 down")
