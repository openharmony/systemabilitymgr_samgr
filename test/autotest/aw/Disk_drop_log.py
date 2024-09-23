#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Copyright(c) 2024, Huawei Technologies Co., HUTAF xDeivce

import os
from hypium.action.host import host


def pulling_disk_dropping_logs(path, sn):
    """
    @func: Pull and drop logs to disk
    @param path: Path of log disk storage
    @param sn: device SN
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
    @func count the number of occurrences of shutdown words in the log disk
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
    @func judge whether a certain keyword exists
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
