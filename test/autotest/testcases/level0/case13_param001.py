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
from devicetest.utils.file_util import get_resource_path
from hypium import *
from hypium.action.host import host

sa_listen_cfg_path = get_resource_path(
    "resource/level0/case13_param001/listen_test.cfg",
    isdir=None)
sa_listen_json_path = get_resource_path(
    "resource/level0/case13_param001/listen_test.json",
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
sa_para_path = get_resource_path(
    "resource/level0/case13_param001/samgr.para",
    isdir=None)
sa_para_dac_path = get_resource_path(
    "resource/level0/case13_param001/samgr.para.dac",
    isdir=None)
sa_para_origin = get_resource_path(
    "resource/originFile/samgr.para",
    isdir=None)
sa_para_dac_origin = get_resource_path(
    "resource/originFile/samgr.para.dac",
    isdir=None)


class case13_param001(TestCase):

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
        driver.Storage.push_file(local_path=sa_lib_listen_test_path, device_path="/system/lib/")
        host.shell("hdc -t {} shell chmod 644 /system/lib/liblisten_test.z.so".format(self.sn))
        driver.Storage.push_file(local_path=sa_proxy_path, device_path="/system/lib/")
        host.shell("hdc -t {} shell chmod 644 /system/lib/libtest_sa_proxy.z.so".format(self.sn))
        driver.Storage.push_file(local_path=sa_listen_cfg_path, device_path="/system/etc/init")
        host.shell("hdc -t {} shell chmod 644 /system/etc/init/listen_test.cfg".format(self.sn))
        driver.Storage.push_file(local_path=sa_listen_json_path, device_path="/system/profile/")
        host.shell("hdc -t {} shell chmod 644 /system/profile/listen_test.json".format(self.sn))
        driver.Storage.push_file(local_path=sa_ondemand_path, device_path="/system/bin/")
        host.shell("hdc -t {} shell chmod 755 /system/bin/ondemand".format(self.sn))
        driver.Storage.push_file(local_path=sa_para_path, device_path="/system/etc/param/")
        host.shell("hdc -t {} shell chmod 755 /system/etc/param/samgr.para".format(self.sn))
        driver.Storage.push_file(local_path=sa_para_dac_path, device_path="/system/etc/param/")
        host.shell("hdc -t {} shell chmod 755 /system/etc/param/samgr.para.dac".format(self.sn))
        driver.System.reboot()

    def test_step(self):
        driver = self.driver
        driver.System.execute_command("ondemand param true")
        max_wait_time = 10
        wait_time = 0
        result = driver.System.execute_command("hidumper -ls")
        while ("1494" not in result and wait_time <= max_wait_time):
            wait_time += 1
            time.sleep(1)
            result = driver.System.execute_command("hidumper -ls")
        CheckPoint("param set to true 1494 successfully loaded")
        assert "1494" in result

        driver.System.execute_command("ondemand param false")
        time.sleep(20)
        result = driver.System.execute_command("hidumper -ls")
        max_wait_time = 5
        wait_time = 0
        while ("1494" in result and wait_time <= max_wait_time):
            wait_time += 1
            time.sleep(1)
            result = driver.System.execute_command("hidumper -ls")
        CheckPoint("param set to false 1494 and successfully unloaded after 20 seconds")
        assert "1494" not in result

    def teardown(self):
        driver = self.driver
        host.shell("hdc -t {} shell kill -9 `pidof listen_test`".format(self.sn))
        host.shell("hdc -t {} target mount".format(self.sn))
        driver.Storage.remove_file("/system/lib/liblisten_test.z.so")
        driver.Storage.remove_file("/system/lib/libtest_sa_proxy.z.so")
        driver.Storage.remove_file("/system/etc/init/listen_test.cfg")
        driver.Storage.remove_file("/system/etc/init/listen_test.json")
        driver.Storage.remove_file("/system/bin/ondemand")
        driver.Storage.remove_file("/system/etc/param/samgr.para")
        driver.Storage.remove_file("/system/etc/param/samgr.para.dac")
        driver.Storage.push_file(local_path=sa_para_origin, device_path="/system/etc/param/")
        driver.Storage.push_file(local_path=sa_para_dac_origin, device_path="/system/etc/param/")
        self.log.info("case13_param001 down")
