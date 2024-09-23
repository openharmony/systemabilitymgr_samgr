#!/usr/bin/env python
#-*- coding: utf-8 -*-
# Copyright(c) 2024, Huawei Technologies Co., HUTAF xDeivce

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
