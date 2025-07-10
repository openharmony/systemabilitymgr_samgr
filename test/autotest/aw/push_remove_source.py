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


def push_source(source_path, driver, sn, update_param=False):
    """
    @func: push resources to devices
    @param source_path: the path of resources required by a case
    @param driver: the device
    @param sn: device sn
    """
    driver.shell("kill -9 `pidof listen_test`")
    driver.hdc("target mount")
    driver.shell("rm -r /data/log/hilog")
    driver.shell("hilog -d /system/bin/samgr")
    if "sa_listen_cfg_path" in source_path:
        driver.Storage.push_file(local_path=source_path["sa_listen_cfg_path"], device_path="/system/etc/init")
        driver.shell("chmod 644 /system/etc/init/listen_test.cfg")
        driver.Storage.push_file(local_path=source_path["sa_listen_json_path"], device_path="/system/profile/")
        driver.shell("chmod 644 /system/profile/listen_test.json")

    if "sa_lib_listen_test_path" in source_path:
        driver.Storage.push_file(local_path=source_path["sa_lib_listen_test_path"], device_path="/system/lib/")
        driver.shell("chmod 644 /system/lib/liblisten_test.z.so")

    if "sa_proxy_path" in source_path:
        driver.Storage.push_file(local_path=source_path["sa_proxy_path"], device_path="/system/lib/")
        driver.shell("chmod 644 /system/lib/libtest_sa_proxy.z.so")


    if "sa_lib_audio_ability" in source_path:
        driver.Storage.push_file(local_path=source_path["sa_lib_audio_ability"], device_path="/system/lib/")
        driver.shell("chmod 644 /system/lib/libtest_audio_ability.z.so")

    if "sa_ondemand_path" in source_path:
        driver.Storage.push_file(local_path=source_path["sa_ondemand_path"], device_path="/system/bin/")
        driver.shell("chmod 755 /system/bin/ondemand")

    if "sa_para_path" in source_path:
        driver.Storage.push_file(local_path=source_path["sa_para_path"], device_path="/system/etc/param/")
        driver.shell("chmod 755 /system/etc/param/samgr.para")
        driver.Storage.push_file(local_path=source_path["sa_para_dac_path"], device_path="/system/etc/param/")
        driver.shell("chmod 755 /system/etc/param/samgr.para.dac")
    if update_param:
        driver.System.execute_command("ondemand param true")
    driver.System.reboot()


def remove_source(source_path, driver, sn):
    """
    @func: push resources from devices
    @param source_path: the path of resources required by a case
    @param driver: the device
    @param sn: device sn
    """
    driver.shell("kill -9 `pidof listen_test`")
    driver.shell("target mount")
    if "sa_listen_cfg_path" in source_path:
        driver.Storage.remove_file("/system/etc/init/listen_test.cfg")
        driver.Storage.remove_file("/system/etc/init/listen_test.json")

    if "sa_para_path" in source_path:
        driver.Storage.remove_file("/system/etc/param/samgr.para")
        driver.Storage.remove_file("/system/etc/param/samgr.para.dac")
        driver.Storage.push_file(local_path=source_path["sa_para_origin"], device_path="/system/etc/param/")
        driver.Storage.push_file(local_path=source_path["sa_para_dac_origin"], device_path="/system/etc/param/")
