/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "device_switch_collect_test.h"

#include "device_status_collect_manager.h"
#include "icollect_plugin.h"
#include "sa_profiles.h"
#include "system_ability_definition.h"
#include "test_log.h"
#include "sam_log.h"

#define private public
#include "collect/device_switch_collect.h"

using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS;

namespace OHOS {
namespace {
static const std::string BLUETOOTH_NAME = "bluetooth_status";
static const std::string UNRELATED_NAME = "test";
static const std::string WIFI_NAME = "wifi_status";
}

void DeviceSwitchCollectTest::SetUpTestCase()
{
    DTEST_LOG << "SetUpTestCase" << std::endl;
}

void DeviceSwitchCollectTest::TearDownTestCase()
{
    DTEST_LOG << "TearDownTestCase" << std::endl;
}

void DeviceSwitchCollectTest::SetUp()
{
    DTEST_LOG << "SetUp" << std::endl;
}

void DeviceSwitchCollectTest::TearDown()
{
    DTEST_LOG << "TearDown" << std::endl;
}

/**
 * @tc.name: DeviceSwitchCollectInit001
 * @tc.desc: test DeviceSwitchCollectInit with wifi event
 * @tc.type: FUNC
 * @tc.require: I6V388
 */

HWTEST_F(DeviceSwitchCollectTest, DeviceSwitchCollectInit001, TestSize.Level3)
{
    std::list<SaProfile> saProfiles;
    SaProfile saProfile;
    OnDemandEvent onDemandEvent = {SETTING_SWITCH, WIFI_NAME, "on"};
    saProfile.startOnDemand.onDemandEvents.emplace_back(onDemandEvent);
    saProfiles.emplace_back(saProfile);
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    deviceSwitchCollect->Init(saProfiles);
    EXPECT_FALSE(deviceSwitchCollect->switches_.empty());
}

/**
 * @tc.name: SetSwitchEvent001
 * @tc.desc: test SetSwitchEvent with wifi event
 * @tc.type: FUNC
 * @tc.require: I6V388
 */

HWTEST_F(DeviceSwitchCollectTest, SetSwitchEvent001, TestSize.Level3)
{
    OnDemandEvent onDemandEvent = {SETTING_SWITCH, WIFI_NAME, "on"};
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    deviceSwitchCollect->SetSwitchEvent(onDemandEvent);
    EXPECT_FALSE(deviceSwitchCollect->switches_.empty());
}

/**
 * @tc.name: DeviceSwitchCollectOnStart001
 * @tc.desc: test DeviceSwitchCollectOnStart with swithes is empty
 * @tc.type: FUNC
 * @tc.require: I6V388
 */

HWTEST_F(DeviceSwitchCollectTest, DeviceSwitchCollectOnStart001, TestSize.Level3)
{
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    deviceSwitchCollect->switches_.clear();
    int32_t ret = deviceSwitchCollect->OnStart();
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: DeviceSwitchCollectOnStop001
 * @tc.desc: test DeviceSwitchCollectOnStop
 * @tc.type: FUNC
 * @tc.require: I6V388
 */

HWTEST_F(DeviceSwitchCollectTest, DeviceSwitchCollectOnStop001, TestSize.Level3)
{
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    int32_t ret = deviceSwitchCollect->OnStop();
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: AddCollectEvent001
 * @tc.desc: test AddCollectEvent with wifi event
 * @tc.type: FUNC
 * @tc.require: I6V388
 */

HWTEST_F(DeviceSwitchCollectTest, AddCollectEvent001, TestSize.Level3)
{
    OnDemandEvent onDemandEvent = {SETTING_SWITCH, WIFI_NAME, "on"};
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    deviceSwitchCollect->AddCollectEvent(onDemandEvent);
    EXPECT_FALSE(deviceSwitchCollect->switches_.empty());
}

/**
 * @tc.name: AddCollectEvent002
 * @tc.desc: test AddCollectEvent with bluetooth event
 * @tc.type: FUNC
 * @tc.require: I6V388
 */

HWTEST_F(DeviceSwitchCollectTest, AddCollectEvent002, TestSize.Level3)
{
    OnDemandEvent onDemandEvent = {SETTING_SWITCH, BLUETOOTH_NAME, "on"};
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    deviceSwitchCollect->AddCollectEvent(onDemandEvent);
    EXPECT_FALSE(deviceSwitchCollect->switches_.empty());
}

/**
 * @tc.name: AddCollectEvent003
 * @tc.desc: test AddCollectEvent with unrelated event
 * @tc.type: FUNC
 * @tc.require: I6V388
 */

HWTEST_F(DeviceSwitchCollectTest, AddCollectEvent003, TestSize.Level3)
{
    OnDemandEvent onDemandEvent = {SETTING_SWITCH, UNRELATED_NAME, "on"};
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    int32_t ret = deviceSwitchCollect->AddCollectEvent(onDemandEvent);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}
}