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
from devicetest.core.test_case import TestCase, CheckPoint, get_report_dir
from devicetest.utils.file_util import get_resource_path
from hypium import *
from hypium.action.host import host
from hypium.action.os_hypium.device_logger import DeviceLogger

sa_listen_cfg_path = get_resource_path(
    "resource/level0/case21_add001/listen_test.cfg",
    isdir=None)
sa_listen_json_path = get_resource_path(
    "resource/level0/case21_add001/listen_test.json",
    isdir=None)
sa_lib_fwk_path = get_resource_path(
    "resource/soResource/libsystem_ability_fwk.z.so",
    isdir=None)
sa_lib_listen_test_path = get_resource_path(
    "resource/soResource/liblisten_test.z.so",
    isdir=None)
sa_ondemand_path = get_resource_path(
    "resource/soResource/ondemand",
    isdir=None)
sa_proxy_path = get_resource_path(
    "resource/soResource/libtest_sa_proxy_cache.z.so",
    isdir=None)
sa_lib_audio_ability = get_resource_path(
    "resource/soResource/libtest_audio_ability.z.so",
    isdir=None)


class case21_add001(TestCase):

    def __init__(self, configs):
        self.TAG = self.__class__.__name__
        TestCase.__init__(self, self.TAG, configs)
        self.tests = [
            "test_step"
        ]
        self.driver = UiDriver(self.device1)
        self.sn = self.device1.device_sn

    def setup(self):
        driver = self.driver
        host.shell("hdc -t {} shell kill -9 `pidof listen_test`".format(self.sn))
        host.shell("hdc -t {} target mount".format(self.sn))
        host.shell("hdc -t {} shell rm -r /data/log/hilog".format(self.sn))
        host.shell("hdc -t {} shell hilog -d /system/bin/samgr".format(self.sn))
        driver.Storage.push_file(local_path=sa_lib_listen_test_path, device_path="/system/lib/")
        host.shell("hdc -t {} shell chmod 644 /system/lib/liblisten_test.z.so".format(self.sn))
        driver.Storage.push_file(local_path=sa_proxy_path, device_path="/system/lib/")
        host.shell("hdc -t {} shell chmod 644 /system/lib/libtest_sa_proxy.z.so".format(self.sn))
        driver.Storage.push_file(local_path=sa_lib_fwk_path, device_path="/system/lib/")
        host.shell("hdc -t {} shell chmod 644 /system/lib/libsystem_ability_fwk.z.so".format(self.sn))
        driver.Storage.push_file(local_path=sa_lib_audio_ability, device_path="/system/lib/")
        host.shell("hdc -t {} shell chmod 644 /system/lib/libtest_audio_ability.z.so".format(self.sn))
        driver.Storage.push_file(local_path=sa_listen_cfg_path, device_path="/system/etc/init")
        host.shell("hdc -t {} shell chmod 644 /system/etc/init/listen_test.cfg".format(self.sn))
        driver.Storage.push_file(local_path=sa_listen_json_path, device_path="/system/profile/")
        host.shell("hdc -t {} shell chmod 644 /system/profile/listen_test.json".format(self.sn))
        driver.Storage.push_file(local_path=sa_ondemand_path, device_path="/system/bin/")
        host.shell("hdc -t {} shell chmod 755 /system/bin/ondemand".format(self.sn))
        driver.System.reboot()

    def test_step(self):
        driver = self.driver
        driver.Screen.close()
        time.sleep(20)
        result = driver.System.execute_command("hidumper -ls")
        max_wait_time = 5
        wait_time = 0
        while ("1494" in result and wait_time <= max_wait_time):
            wait_time += 1
            time.sleep(1)
            result = driver.System.execute_command("hidumper -ls")
        CheckPoint("Initially, 1494 was not loaded")
        assert "1494" not in result

        device_logger = DeviceLogger(driver).set_filter_string("01800")
        device_logger.start_log(get_report_dir() + "//case21_add001.txt")
        driver.System.execute_command("./system/bin/sa_main /system/profile/listen_test.json")
        device_logger.stop_log()
        device_logger.check_log("AddSystemProcessInner PERMISSION DENIED!", EXCEPTION=True)

        result = driver.System.execute_command("hidumper -ls")
        max_wait_time = 5
        wait_time = 0
        while ("1494" in result and wait_time <= max_wait_time):
            wait_time += 1
            time.sleep(1)
            result = driver.System.execute_command("hidumper -ls")
        CheckPoint("Non native process add intercepted, 1494 not loaded")
        assert "1494" not in result

    def teardown(self):
        driver = self.driver
        driver.Screen.close()
        host.shell("hdc -t {} shell kill -9 `pidof listen_test`".format(self.sn))
        host.shell("hdc -t {} target mount".format(self.sn))
        driver.Storage.remove_file("/system/lib/liblisten_test.z.so")
        driver.Storage.remove_file("/system/lib/libtest_sa_proxy.z.so")
        driver.Storage.remove_file("/system/etc/init/listen_test.cfg")
        driver.Storage.remove_file("/system/etc/init/listen_test.json")
        driver.Storage.remove_file("/system/bin/ondemand")
        driver.Storage.remove_file("/system/lib/libtest_audio_ability.z.so")
        driver.Storage.remove_file("/system/lib/libsystem_ability_fwk.z.so")
        self.log.info("case21_add001 down")
