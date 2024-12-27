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

import os
import time

from devicetest.core.test_case import TestCase
from devicetest.utils.file.file_util import get_resource_path
from hypium import UiExplore
from hypium.action.host import host
from hypium.action.os_hypium.device_logger import AsyncCommand


sa_lib_test_path = get_resource_path(
    "resource/SO_RESOURCE/liblisten_test.z.so",
    isdir=None)

sa_lib_ability_c_path = get_resource_path(
    "resource/SO_RESOURCE/libtest_audio_ability.z.so",
    isdir=None)

sa_proxy_path = get_resource_path(
    "resource/SO_RESOURCE/libtest_sa_proxy_cache.z.so",
    isdir=None)

sa_listen_cfg_path = get_resource_path(
    "resource/level/lifecycle_state_023/listen_test.cfg",
    isdir=None)

sa_listen_json_path = get_resource_path(
    "resource/level/lifecycle_state_023/listen_test.json",
    isdir=None)

sa_ondemand_path = get_resource_path(
    "resource/SO_RESOURCE/ondemand",
    isdir=None)

sa_tool_path = get_resource_path(
    "resource/SO_RESOURCE/TestTool",
    isdir=None)


class LifeCycle_State_023(TestCase):

    def __init__(self, controllers):
        sele.TAG = self.__class__.__name__
        TestCase.__init__(self, self.TAG, controllers)
        self.tests = [
            "test_step"
        ]
        self.driver = UiExplore(self.driver1)
        self.sn = self.driver1.device_sn
    
    def setup(self):
        driver = self.driver
        host.shell("hdc -t {} shell kill -9 `pidof listen_test`".format(self.sn))
        # host.shell("hdc -t {} target mount".format(self.sn))
        # driver.Storage.push_file(local_path=sa_lib_test_path, device_path="/systemlib/lib/")
        # host.shell("hdc -t {} shell chmod 644 /system/lib/lib/liblisten_test.z.so".format(self.sn))

        # driver.Storage.push_file(local_path=sa_proxy_path, device_path="/systemlib/lib/")
        # host.shell("hdc -t {} shell chmod 644 /system/lib/lib/libtest_sa_proxy_cache.z.so".format(self.sn))

        # driver.Storage.push_file(local_path=sa_lib_ability_c_path, device_path="/systemlib/lib/")
        # host.shell("hdc -t {} shell chmod 644 /system/lib/lib/libtest_audio_ability.z.so".format(self.sn))

        # driver.Storage.push_file(local_path=sa_listen_cfg_path, device_path="/system/etc/init/")
        # host.shell("hdc -t {} shell chmod 644 /system/etc/init/listen_test.cfg".format(self.sn))

        # driver.Storage.push_file(local_path=sa_listen_json_path, device_path="/system/profile/")
        # host.shell("hdc -t {} shell chmod 644 /system/profile/listen_test.json".format(self.sn))

        # driver.Storage.push_file(local_path=sa_ondemand_path, device_path="/systemlib/bin/")
        # host.shell("hdc -t {} shell chmod 755 /system/bin/ondemand".format(self.sn))

        # driver.Storage.push_file(local_path=sa_tool_path, device_path="/systemlib/bin/")
        # host.shell("hdc -t {} shell chmod 755 /system/bin/TestTool".format(self.sn))
        # driver.System.reboot()

    def test_step(self):
        driver = self.driver
        # 用例同步
        # 执行"ondemand test 24"命令, 执行测试程序
        # 预期结果
        # 控制台打印:
        # GetSystemAbility result: success
        result = driver.System.execute_command("ondemand test 23")
        assert "GetSystemAbility systemAbilityId:1494 faild" in result

    def teardown(self):
        self.driver.System.execute_command("kill -9 `pidof listen_test`")
        self.driver.Storage.remove_file("/system/lib/lib/liblisten_test.z.so")
        self.driver.Storage.remove_file("/system/lib/lib/libtest_sa_proxy_cache.z.so")
        self.driver.Storage.remove_file("/system/lib/lib/libtest_audio_ability.z.so")
        self.driver.Storage.remove_file("/system/etc/init/listen_test.cfg")
        self.driver.Storage.remove_file("/system/etc/init/listen_test.json")
        self.driver.Storage.remove_file("/system/bin/ondemand")
        self.log.info("done")