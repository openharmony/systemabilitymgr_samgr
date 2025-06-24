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
import time

from devicetest.core.test_case import TestCase
from devicetest.utils.file.file_util import get_resource_path
from hypium import *
from hypium.action.host import host

from aw import NetWorking

sa_listen_cfg_path = get_resource_path(
    "resource/level/networking/sub_samgr_sa_load_002/listen_test.cfg",
    isdir=None)

sa_listen_json_path = get_resource_path(
    "resource/level/networking/sub_samgr_sa_load_002/listen_test.json",
    isdir=None)

sa_lib_test_path = get_resource_path(
    "resource/SO_RESOURCE/liblisten_test.z.so",
    isdir=None)

sa_lib_ability_c_path = get_resource_path(
    "resource/SO_RESOURCE/libtest_audio_ability.z.so",
    isdir=None)

sa_ondemand_path = get_resource_path(
    "resource/SO_RESOURCE/ondemand",
    isdir=None)

sa_proxy_path = get_resource_path(
    "resource/SO_RESOURCE/libtest_sa_proxy_cache.z.so",
    isdir=None)


class Sub_Samgr_SA_Load_002(TestCase):

    def __init__(self, configs):
        sele.TAG = self.__class__.__name__
        TestCase.__init__(self, self.TAG, configs)
        self.driver1 = UiDriver(self.device1)
        self.driver1 = UiDriver(self.device2)
        self.sn1 = self.device1.device_sn
        self.sn1 = self.device2.device_sn

    def setup(self):
        driver = self.driver1
        driver.Wifi.disable()
        driver.Bluetooth.disable()  #避免开始就组网成功
        # host.shell()用来在pc端执行shell命令，亦可直接执行bat脚本(bat脚本中是push所需要的测试文件)
        host.shell("hdc -t {} shell kill -9 `pidof listen_test`".format(self.sn1))
        # 推入资源文件进入设备
        host.shell("hdc -t {} target mount".format(self.sn1))
        driver.Storage.push_file(local_path=sa_lib_test_path, device_path="/systemlib/lib/")
        host.shell("hdc -t {} shell chmod 644 /system/lib/lib/liblisten_test.z.so".format(self.sn1))

        driver.Storage.push_file(local_path=sa_listen_cfg_path, device_path="/system/etc/init/")
        host.shell("hdc -t {} shell chmod 644 /system/etc/init/listen_test.cfg".format(self.sn1))

        driver.Storage.push_file(local_path=sa_listen_json_path, device_path="/system/profile/")
        host.shell("hdc -t {} shell chmod 644 /system/profile/listen_test.json".format(self.sn1))

        driver.Storage.push_file(local_path=sa_ondemand_path, device_path="/systemlib/bin/")
        host.shell("hdc -t {} shell chmod 755 /system/bin/ondemand".format(self.sn2))

        driver.Storage.push_file(local_path=sa_proxy_path, device_path="/systemlib/lib/")
        host.shell("hdc -t {} shell chmod 644 /system/lib/lib/libtest_sa_proxy_cache.z.so".format(self.sn1))

        driver2 = self.driver2
        host.shell("hdc -t {} shell kill -9 `pidof listen_test`".format(self.sn2))
        # 推入资源文件进入设备
        host.shell("hdc -t {} target mount".format(self.sn2))
        driver2.Storage.push_file(local_path=sa_lib_test_path, device_path="/systemlib/lib/")
        host.shell("hdc -t {} shell chmod 644 /system/lib/lib/liblisten_test.z.so".format(self.sn2))

        driver2.Storage.push_file(local_path=sa_listen_cfg_path, device_path="/system/etc/init/")
        host.shell("hdc -t {} shell chmod 644 /system/etc/init/listen_test.cfg".format(self.sn2))

        driver2.Storage.push_file(local_path=sa_listen_json_path, device_path="/system/profile/")
        host.shell("hdc -t {} shell chmod 644 /system/profile/listen_test.json".format(self.sn2))

        driver2.Storage.push_file(local_path=sa_ondemand_path, device_path="/systemlib/bin/")
        host.shell("hdc -t {} shell chmod 755 /system/bin/ondemand".format(self.sn2))

        driver2.Storage.push_file(local_path=sa_proxy_path, device_path="/systemlib/lib/")
        host.shell("hdc -t {} shell chmod 644 /system/lib/lib/libtest_sa_proxy_cache.z.so".format(self.sn2))
        
        driver.System.reboot()
        driver2.System.reboot()


    def process(self):
        # 执行步骤
        #1、执行push.bat脚本， 推入once - online测试资源
        #2、组网后hidumper-ls查看
        #3、wifi断开后hidumper -ls再次查看
        #4、组网后hidumper-ls再次查看
        #5、wifi断开后hidumper -ls再次查看

        # 预期结果
        #1、DistributedDeviceProfile存在
        #2、20s后DistributedDeviceProfile不存在
        #3、DistributedDeviceProfile存在
        #4、DistributedDeviceProfile存在

        #3
        # 设备A断开wifi蓝牙
        driver.wifi.disable()
        driver.Bluetooth.disable()
        time.sleep(25)
        # 判断组网成功
        NetWorking.determine_newwork(driver)
        result = driver1.System.execute_command("hidumper -ls ")
        assert "1494" in result
        
        #4
        driver.wifi.disable()
        driver.Bluetooth.disable()
        time.sleep(50)
        # 判断组网成功
        NetWorking.determine_newwork(driver)
        result = driver1.System.execute_command("hidumper -ls ")
        assert "1494" in result
        
        #5
        # 设备A断开蓝牙wifi
        driver.wifi.disable()
        driver.Bluetooth.disable()
        time.sleep(25)
        result = driver1.System.execute_command("hidumper -ls ")
        assert "1494" in result

    def teardown(self):
        self.driver1.System.execute_command("kill -9 `pidof listen_test`")
        host.shell("hdc -t {} target mount".format(self.sn1))
        self.driver1.Storage.remove_file("/system/lib/lib/liblisten_test.z.so")
        self.driver1.Storage.remove_file("/system/lib/lib/libtest_sa_proxy_cache.z.so")
        self.driver1Storage.remove_file("/system/lib/lib/libtest_audio_ability.z.so")
        self.driver1.Storage.remove_file("/system/etc/init/listen_test.cfg")
        self.driver1.Storage.remove_file("/system/etc/init/listen_test.json")
        self.driver1.Storage.remove_file("/system/bin/ondemand")
        
        self.driver2.System.execute_command("kill -9 `pidof listen_test`")
        host.shell("hdc -t {} target mount".format(self.sn2))
        self.driver2.Storage.remove_file("/system/lib/lib/liblisten_test.z.so")
        self.driver2.Storage.remove_file("/system/lib/lib/libtest_sa_proxy_cache.z.so")
        self.driver2.Storage.remove_file("/system/lib/lib/libtest_audio_ability.z.so")
        self.driver2.Storage.remove_file("/system/etc/init/listen_test.cfg")
        self.driver2.Storage.remove_file("/system/etc/init/listen_test.json")
        self.driver2.Storage.remove_file("/system/bin/ondemand")
        self.log.info("done")