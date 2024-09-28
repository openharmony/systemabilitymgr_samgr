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
from hypium.action.host import host


def pulling_disk_dropping_logs(path, sn):
    """
    @func: Pull and drop logs to disk
    @param: path: Path of log disk storage
    @param: sn: device SN
    """
    host.shell(f"hdc -t {sn}  file recv data/log/hilog/ {path}")


def parse_disk_dropping_logs(path):
    """
    @func: Analyze log storage on disk
    @param path: log path
    """
    host.shell(f"hilog parse -i {path} -d {path}")


def count_keys_disk_dropping_logs(path, keys) -> int:
    """
    @func: count the number of occurrences of shutdown words in the log disk
    @param path: log download path
    @param keys: Keywords to be queried
    @return: The number of times the keyword to be queried appears
    """
    list_count = [0]
    dirs = os.listdir(path)
    for file in dirs:
        if file.endswith(".txt"):
            with open(f"{path}/{file}", "r", encoding="utf-8", errors="ignore") as read_file:
                count = read_file.read().count(keys)
                list_count.append(count)
    return sum(list_count)


def check_disk_dropping_logs(path, keys) -> bool:
    """
    @func: judge whether a certain keyword exists
    @param path: log path
    @param keys: Keywords to be queried
    @return: whether a certain keyword exists
    """

    flag = False
    dirs = os.listdir(path)
    for file in dirs:
        if file.endswith(".txt"):
            with open(f"{path}/{file}", "r", encoding="utf-8", errors="ignore") as read_file:
                for line in read_file.readlines():
                    result = line.find(keys)
                    if result != -1:
                        flag = True
                        break
            if flag:
                break
    return flag
