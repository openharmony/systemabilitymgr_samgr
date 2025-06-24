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

from devicetest.utils.file_util import get_resource_path


def get_source_path(need_source, casename):
    """
    @func: get the resource path required for a case
    @param need_source: the resources required for a case
    @param casename: the case name
    @return: source absolute paths
    """

    source_path = {}
    if need_source["cfg"]:
        cfg_relative_path = "resource/" + casename + "/listen_test.cfg"
        json_relative_path = "resource/" + casename + "/listen_test.json"
        sa_listen_cfg_path = get_resource_path(
            cfg_relative_path,
            isdir=None)
        sa_listen_json_path = get_resource_path(
            json_relative_path,
            isdir=None)
        source_path["sa_listen_cfg_path"] = sa_listen_cfg_path
        source_path["sa_listen_json_path"] = sa_listen_json_path


    if need_source["listen_test"]:
        sa_lib_listen_test_path = get_resource_path(
            "resource/soResource/liblisten_test.z.so",
            isdir=None)
        source_path["sa_lib_listen_test_path"] = sa_lib_listen_test_path

    if need_source["audio_ability"]:
        sa_lib_audio_ability = get_resource_path(
            "resource/soResource/libtest_audio_ability.z.so",
            isdir=None)
        source_path["sa_lib_audio_ability"] = sa_lib_audio_ability

    if need_source["ondemand"]:
        sa_ondemand_path = get_resource_path(
            "resource/soResource/ondemand",
            isdir=None)
        source_path["sa_ondemand_path"] = sa_ondemand_path

    if need_source["proxy"]:
        sa_proxy_path = get_resource_path(
            "resource/soResource/libtest_sa_proxy_cache.z.so",
            isdir=None)
        source_path["sa_proxy_path"] = sa_proxy_path

    if need_source["para"]:
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
        source_path["sa_para_path"] = sa_para_path
        source_path["sa_para_dac_path"] = sa_para_dac_path
        source_path["sa_para_origin"] = sa_para_origin
        source_path["sa_para_dac_origin"] = sa_para_dac_origin

    return source_path
