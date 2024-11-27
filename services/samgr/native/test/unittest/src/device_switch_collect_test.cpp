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

#include "common_event_manager.h"
#include "common_event_support.h"
#include "matching_skills.h"
#include "device_status_collect_manager.h"
#include "icollect_plugin.h"
#include "sa_profiles.h"
#include "system_ability_definition.h"
#include "test_log.h"
#include "sam_log.h"

#define private public
#include "collect/device_switch_collect.h"
#include "system_ability_manager.h"

using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS;

namespace OHOS {
namespace {
constexpr int32_t COMMON_EVENT_ID = 3299;
constexpr int32_t INVALID_SAID = -1;
constexpr int32_t BLUETOOTH_STATE_TURN_ON = 1;
constexpr int32_t BLUETOOTH_STATE_TURN_OFF = 3;
constexpr int32_t WIFI_ON = 3;
constexpr int32_t WIFI_OFF = 1;
static const std::string BLUETOOTH_NAME = "bluetooth_status";
static const std::string DEVICE_ID = "local";
static const std::string INVALID_ACTION = "test";
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
 * @tc.name: InitCommonEventSubscriber001
 * @tc.desc: test InitCommonEventSubscriber, initialize deviceswitchcollect's CommonEventSubscriber_
 * @tc.type: FUNC
 * @tc.require: I7RSCL
 */

HWTEST_F(DeviceSwitchCollectTest, InitCommonEventSubscriber001, TestSize.Level3)
{
    DTEST_LOG << "InitCommonEventSubscriber001 begin" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    deviceSwitchCollect->InitCommonEventSubscriber();
    EXPECT_NE(deviceSwitchCollect->switchEventSubscriber_, nullptr);
    DTEST_LOG << "InitCommonEventSubscriber001 end" << std::endl;
}

/**
 * @tc.name: CheckSwitchEvent001
 * @tc.desc: test CheckSwitchEvent with ondemand event
 * @tc.type: FUNC
 * @tc.require: I7RSCL
 */

HWTEST_F(DeviceSwitchCollectTest, CheckSwitchEvent001, TestSize.Level3)
{
    DTEST_LOG << "CheckSwitchEvent001 begin" << std::endl;
    OnDemandEvent onDemandEvent;
    onDemandEvent.eventId = SETTING_SWITCH;
    onDemandEvent.name = WIFI_NAME;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    int32_t result = deviceSwitchCollect->CheckSwitchEvent(onDemandEvent);
    EXPECT_EQ(result, ERR_OK);

    onDemandEvent.name = BLUETOOTH_NAME;
    result = deviceSwitchCollect->CheckSwitchEvent(onDemandEvent);
    EXPECT_EQ(result, ERR_OK);

    onDemandEvent.name = INVALID_ACTION;
    result = deviceSwitchCollect->CheckSwitchEvent(onDemandEvent);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    onDemandEvent.eventId = TIMED_EVENT;
    result = deviceSwitchCollect->CheckSwitchEvent(onDemandEvent);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    DTEST_LOG << "CheckSwitchEvent001 end" << std::endl;
}

/**
 * @tc.name: DeviceSwitchCollectInit001
 * @tc.desc: test DeviceSwitchCollectInit, initialize deviceswitchcollect
 * @tc.type: FUNC
 * @tc.require: I7RSCL
 */

HWTEST_F(DeviceSwitchCollectTest, DeviceSwitchCollectInit001, TestSize.Level3)
{
    DTEST_LOG << "DeviceSwitchCollectInit001 begin" << std::endl;
    std::list<SaProfile> saProfiles;
    SaProfile saProfile;
    OnDemandEvent onDemandEvent = {SETTING_SWITCH, WIFI_NAME, "on"};
    saProfile.startOnDemand.onDemandEvents.emplace_back(onDemandEvent);
    saProfile.stopOnDemand.onDemandEvents.emplace_back(onDemandEvent);
    saProfiles.emplace_back(saProfile);
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    deviceSwitchCollect->Init(saProfiles);
    EXPECT_TRUE(deviceSwitchCollect->needListenSwitchEvent_);
    DTEST_LOG << "DeviceSwitchCollectInit001 end" << std::endl;
}

/**
 * @tc.name: CollectSubscribeSwitchEvent001
 * @tc.desc: test SubscribeSwitchEvent with switchEventSubscriber_ is nullptr
 * @tc.type: FUNC
 * @tc.require: I7RSCL
 */

HWTEST_F(DeviceSwitchCollectTest, CollectSubscribeSwitchEvent001, TestSize.Level3)
{
    DTEST_LOG << "CollectSubscribeSwitchEvent001 begin" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    deviceSwitchCollect->switchEventSubscriber_ = nullptr;
    int32_t ret = deviceSwitchCollect->SubscribeSwitchEvent();
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    DTEST_LOG << "CollectSubscribeSwitchEvent001 end" << std::endl;
}

/**
 * @tc.name: CollectSubscribeSwitchEvent002
 * @tc.desc: test SubscribeCommonEventService
 * @tc.type: FUNC
 * @tc.require: I7RSCL
 */

HWTEST_F(DeviceSwitchCollectTest, CollectSubscribeSwitchEvent002, TestSize.Level3)
{
    DTEST_LOG << "CollectSubscribeSwitchEvent002 begin" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    deviceSwitchCollect->InitCommonEventSubscriber();
    deviceSwitchCollect->switchEventSubscriber_->isListenSwitchEvent_ = true;
    int32_t ret = deviceSwitchCollect->SubscribeSwitchEvent();
    EXPECT_EQ(ret, ERR_OK);
    DTEST_LOG << "CollectSubscribeSwitchEvent002 end" << std::endl;
}

/**
 * @tc.name: CollectSubscribeSwitchEvent003
 * @tc.desc: test SubscribeCommonEventService
 * @tc.type: FUNC
 * @tc.require: I7RSCL
 */
HWTEST_F(DeviceSwitchCollectTest, CollectSubscribeSwitchEvent003, TestSize.Level3)
{
    DTEST_LOG << "CollectSubscribeSwitchEvent003 begin" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    deviceSwitchCollect->InitCommonEventSubscriber();
    deviceSwitchCollect->switchEventSubscriber_->isListenSwitchEvent_ = false;
    int32_t ret = deviceSwitchCollect->SubscribeSwitchEvent();
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(deviceSwitchCollect->switchEventSubscriber_->isListenSwitchEvent_, true);
    ret = deviceSwitchCollect->switchEventSubscriber_->UnSubscribeSwitchEvent();
    EXPECT_EQ(ret, ERR_OK);
    DTEST_LOG << "CollectSubscribeSwitchEvent003 end" << std::endl;
}

/**
 * @tc.name: OnStart001
 * @tc.desc: test OnStart with needListenSwitchEvent_ is false
 * @tc.type: FUNC
 * @tc.require: I7RSCL
 */

HWTEST_F(DeviceSwitchCollectTest, OnStart001, TestSize.Level3)
{
    DTEST_LOG << "OnStart001 begin" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->workHandler_ = make_shared<FFRTHandler>("workHandler");
    deviceSwitchCollect->needListenSwitchEvent_ = false;
    int32_t ret = deviceSwitchCollect->OnStart();
    EXPECT_EQ(ret, ERR_OK);
    DTEST_LOG << "OnStart001 end" << std::endl;
}

/**
 * @tc.name: OnStart002
 * @tc.desc: test OnStart with needListenSwitchEvent_ is true
 * @tc.type: FUNC
 * @tc.require: I7RSCL
 */

HWTEST_F(DeviceSwitchCollectTest, OnStart002, TestSize.Level3)
{
    DTEST_LOG << "OnStart002 begin" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    deviceSwitchCollect->InitCommonEventSubscriber();
    deviceSwitchCollect->needListenSwitchEvent_ = true;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->workHandler_ = make_shared<FFRTHandler>("workHandler");
    saMgr->subscribeCountMap_.clear();
    int32_t ret = deviceSwitchCollect->OnStart();
    EXPECT_EQ(ret, ERR_OK);
    DTEST_LOG << "OnStart002 end" << std::endl;
}

/**
 * @tc.name: OnStop001
 * @tc.desc: test OnStop with both cesStateListener_ and switchEventSubscriber_ are nullptr
 * @tc.type: FUNC
 * @tc.require: I7RSCL
 */

HWTEST_F(DeviceSwitchCollectTest, OnStop001, TestSize.Level3)
{
    DTEST_LOG << "OnStop001 begin" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    int32_t ret = deviceSwitchCollect->OnStop();
    EXPECT_EQ(ret, ERR_OK);
    DTEST_LOG << "OnStop001 end" << std::endl;
}

/**
 * @tc.name: OnStop002
 * @tc.desc: test OnStop with both cesStateListener_ is not nullptr
 * @tc.type: FUNC
 * @tc.require: I7RSCL
 */

HWTEST_F(DeviceSwitchCollectTest, OnStop002, TestSize.Level3)
{
    DTEST_LOG << "OnStop002 begin" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    deviceSwitchCollect->InitCommonEventSubscriber();
    deviceSwitchCollect->switchEventSubscriber_ = nullptr;
    int32_t ret = deviceSwitchCollect->OnStop();
    EXPECT_EQ(ret, ERR_OK);
    DTEST_LOG << "OnStop002 end" << std::endl;
}

/**
 * @tc.name: OnStop003
 * @tc.desc: test OnStop with both switchEventSubscriber_ is not nullptr
 * @tc.type: FUNC
 * @tc.require: I7RSCL
 */

HWTEST_F(DeviceSwitchCollectTest, OnStop003, TestSize.Level3)
{
    DTEST_LOG << "OnStop003 begin" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    deviceSwitchCollect->InitCommonEventSubscriber();
    int32_t ret = deviceSwitchCollect->OnStop();
    EXPECT_EQ(ret, ERR_OK);
    DTEST_LOG << "OnStop003 end" << std::endl;
}

/**
 * @tc.name: AddCollectEvent001
 * @tc.desc: test AddCollectEvent with ondemandevent's name is not SETTING_SWITCH
 * @tc.type: FUNC
 * @tc.require: I7RSCL
 */

HWTEST_F(DeviceSwitchCollectTest, AddCollectEvent001, TestSize.Level3)
{
    DTEST_LOG << "AddCollectEvent001 begin" << std::endl;
    OnDemandEvent onDemandEvent;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    int32_t ret = deviceSwitchCollect->AddCollectEvent(onDemandEvent);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    DTEST_LOG << "AddCollectEvent001 end" << std::endl;
}

/**
 * @tc.name: AddCollectEvent002
 * @tc.desc: test AddCollectEvent with proper ondemandevent name
 * @tc.type: FUNC
 * @tc.require: I7RSCL
 */

HWTEST_F(DeviceSwitchCollectTest, AddCollectEvent002, TestSize.Level3)
{
    DTEST_LOG << "AddCollectEvent002 begin" << std::endl;
    OnDemandEvent onDemandEvent = {SETTING_SWITCH, WIFI_NAME, "on"};
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    deviceSwitchCollect->InitCommonEventSubscriber();
    SystemAbilityManager::GetInstance()->subscribeCountMap_.clear();
    int32_t ret = deviceSwitchCollect->AddCollectEvent(onDemandEvent);
    EXPECT_EQ(ret, ERR_OK);
    DTEST_LOG << "AddCollectEvent002 end" << std::endl;
}

/**
 * @tc.name: OnAddSystemAbility002
 * @tc.desc: test OnAddSystemAbility with deviceSwitchCollect is nullptr
 * @tc.type: FUNC
 * @tc.require: I7RSCL
 */

HWTEST_F(DeviceSwitchCollectTest, OnAddSystemAbility002, TestSize.Level3)
{
    DTEST_LOG << "OnAddSystemAbility002 begin" << std::endl;
    sptr<CesStateListener> cesStateListener = new CesStateListener(nullptr);
    cesStateListener->OnAddSystemAbility(COMMON_EVENT_ID, DEVICE_ID);
    auto deviceSwitchCollect = cesStateListener->deviceSwitchCollect_.promote();
    EXPECT_EQ(deviceSwitchCollect, nullptr);
    DTEST_LOG << "OnAddSystemAbility002 end" << std::endl;
}

/**
 * @tc.name: SubscribeSwitchEvent001
 * @tc.desc: test SubscribeSwitchEvent with deviceswitchcollect is initialized
 * @tc.type: FUNC
 * @tc.require: I7RSCL
 */

HWTEST_F(DeviceSwitchCollectTest, SubscribeSwitchEvent001, TestSize.Level3)
{
    DTEST_LOG << "SubscribeSwitchEvent001 begin" << std::endl;
    std::list<SaProfile> saProfiles;
    SaProfile saProfile;
    OnDemandEvent onDemandEvent = {SETTING_SWITCH, WIFI_NAME, "on"};
    saProfile.startOnDemand.onDemandEvents.emplace_back(onDemandEvent);
    saProfiles.emplace_back(saProfile);
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    deviceSwitchCollect->Init(saProfiles);
    deviceSwitchCollect->switchEventSubscriber_->SubscribeSwitchEvent();
    EXPECT_NE(deviceSwitchCollect->switchEventSubscriber_, nullptr);
    DTEST_LOG << "SubscribeSwitchEvent001 end" << std::endl;
}

/**
 * @tc.name: UnSubscribeSwitchEvent001
 * @tc.desc: test UnSubscribeSwitchEvent with deviceswitchcollect is initialized
 * @tc.type: FUNC
 * @tc.require: I7RSCL
 */

HWTEST_F(DeviceSwitchCollectTest, UnSubscribeSwitchEvent001, TestSize.Level3)
{
    DTEST_LOG << "UnSubscribeSwitchEvent001 begin" << std::endl;
    std::list<SaProfile> saProfiles;
    SaProfile saProfile;
    OnDemandEvent onDemandEvent = {SETTING_SWITCH, WIFI_NAME, "on"};
    saProfile.startOnDemand.onDemandEvents.emplace_back(onDemandEvent);
    saProfiles.emplace_back(saProfile);
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    deviceSwitchCollect->Init(saProfiles);
    deviceSwitchCollect->switchEventSubscriber_->SubscribeSwitchEvent();
    EXPECT_NE(deviceSwitchCollect->switchEventSubscriber_, nullptr);
    DTEST_LOG << "UnSubscribeSwitchEvent001 end" << std::endl;
}

/**
 * @tc.name: ReportEvent001
 * @tc.desc: test ReportEvent with deviceswitchcollect is not nullptr
 * @tc.type: FUNC
 * @tc.require: I7RSCL
 */

HWTEST_F(DeviceSwitchCollectTest, ReportEvent001, TestSize.Level3)
{
    DTEST_LOG << "ReportEvent001 begin" << std::endl;
    OnDemandEvent onDemandEvent = {SETTING_SWITCH, WIFI_NAME, "on"};
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    deviceSwitchCollect->InitCommonEventSubscriber();
    deviceSwitchCollect->switchEventSubscriber_->ReportEvent(onDemandEvent);
    EXPECT_NE(deviceSwitchCollect->switchEventSubscriber_->deviceSwitchCollect_, nullptr);
    DTEST_LOG << "ReportEvent001 end" << std::endl;
}

/**
 * @tc.name: OnReceiveWifiEvent001
 * @tc.desc: test OnReceiveWifiEvent with wifi is turn on
 * @tc.type: FUNC
 * @tc.require: I7RSCL
 */

HWTEST_F(DeviceSwitchCollectTest, OnReceiveWifiEvent001, TestSize.Level3)
{
    DTEST_LOG << "OnReceiveWifiEvent001 begin" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    deviceSwitchCollect->InitCommonEventSubscriber();
    AAFwk::Want want;
    EventFwk::CommonEventData data;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_WIFI_POWER_STATE);
    data.SetWant(want);
    data.SetCode(WIFI_ON);
    deviceSwitchCollect->switchEventSubscriber_->OnReceiveWifiEvent(data);
    EXPECT_NE(deviceSwitchCollect->switchEventSubscriber_->deviceSwitchCollect_, nullptr);
    DTEST_LOG << "OnReceiveWifiEvent001 end" << std::endl;
}

/**
 * @tc.name: OnReceiveWifiEvent002
 * @tc.desc: test OnReceiveWifiEvent with wifi is turn off
 * @tc.type: FUNC
 * @tc.require: I7RSCL
 */

HWTEST_F(DeviceSwitchCollectTest, OnReceiveWifiEvent002, TestSize.Level3)
{
    DTEST_LOG << "OnReceiveWifiEvent002 begin" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    deviceSwitchCollect->InitCommonEventSubscriber();
    AAFwk::Want want;
    EventFwk::CommonEventData data;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_WIFI_POWER_STATE);
    data.SetWant(want);
    data.SetCode(WIFI_OFF);
    deviceSwitchCollect->switchEventSubscriber_->OnReceiveWifiEvent(data);
    EXPECT_NE(deviceSwitchCollect->switchEventSubscriber_->deviceSwitchCollect_, nullptr);
    DTEST_LOG << "OnReceiveWifiEvent002 end" << std::endl;
}

/**
 * @tc.name: OnReceiveWifiEvent003
 * @tc.desc: test OnReceiveWifiEvent with invalid action
 * @tc.type: FUNC
 * @tc.require: I7RSCL
 */

HWTEST_F(DeviceSwitchCollectTest, OnReceiveWifiEvent003, TestSize.Level3)
{
    DTEST_LOG << "OnReceiveWifiEvent003 begin" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    deviceSwitchCollect->InitCommonEventSubscriber();
    AAFwk::Want want;
    EventFwk::CommonEventData data;
    want.SetAction(INVALID_ACTION);
    data.SetWant(want);
    data.SetCode(WIFI_OFF);
    deviceSwitchCollect->switchEventSubscriber_->OnReceiveWifiEvent(data);
    EXPECT_NE(deviceSwitchCollect->switchEventSubscriber_->deviceSwitchCollect_, nullptr);
    DTEST_LOG << "OnReceiveWifiEvent003 end" << std::endl;
}

/**
 * @tc.name: OnReceiveBluetoothEvent001
 * @tc.desc: test OnReceiveBluetoothEvent with bluetooth is turn on
 * @tc.type: FUNC
 * @tc.require: I7RSCL
 */

HWTEST_F(DeviceSwitchCollectTest, OnReceiveBluetoothEvent001, TestSize.Level3)
{
    DTEST_LOG << "OnReceiveBluetoothEvent001 begin" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    deviceSwitchCollect->InitCommonEventSubscriber();
    AAFwk::Want want;
    EventFwk::CommonEventData data;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE);
    data.SetWant(want);
    data.SetCode(BLUETOOTH_STATE_TURN_ON);
    deviceSwitchCollect->switchEventSubscriber_->OnReceiveBluetoothEvent(data);
    EXPECT_NE(deviceSwitchCollect->switchEventSubscriber_->deviceSwitchCollect_, nullptr);
    DTEST_LOG << "OnReceiveBluetoothEvent001 end" << std::endl;
}

/**
 * @tc.name: OnReceiveBluetoothEvent002
 * @tc.desc: test OnReceiveBluetoothEvent with bluetooth is turn off
 * @tc.type: FUNC
 * @tc.require: I7RSCL
 */

HWTEST_F(DeviceSwitchCollectTest, OnReceiveBluetoothEvent002, TestSize.Level3)
{
    DTEST_LOG << "OnReceiveBluetoothEvent002 begin" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    deviceSwitchCollect->InitCommonEventSubscriber();
    AAFwk::Want want;
    EventFwk::CommonEventData data;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE);
    data.SetWant(want);
    data.SetCode(BLUETOOTH_STATE_TURN_OFF);
    deviceSwitchCollect->switchEventSubscriber_->OnReceiveBluetoothEvent(data);
    EXPECT_NE(deviceSwitchCollect->switchEventSubscriber_->deviceSwitchCollect_, nullptr);
    DTEST_LOG << "OnReceiveBluetoothEvent002 end" << std::endl;
}

/**
 * @tc.name: OnReceiveBluetoothEvent003
 * @tc.desc: test OnReceiveBluetoothEvent with invalid action
 * @tc.type: FUNC
 * @tc.require: I7RSCL
 */

HWTEST_F(DeviceSwitchCollectTest, OnReceiveBluetoothEvent003, TestSize.Level3)
{
    DTEST_LOG << "OnReceiveBluetoothEvent003 begin" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    deviceSwitchCollect->InitCommonEventSubscriber();
    AAFwk::Want want;
    EventFwk::CommonEventData data;
    want.SetAction(INVALID_ACTION);
    data.SetWant(want);
    data.SetCode(BLUETOOTH_STATE_TURN_ON);
    deviceSwitchCollect->switchEventSubscriber_->OnReceiveBluetoothEvent(data);
    EXPECT_NE(deviceSwitchCollect->switchEventSubscriber_->deviceSwitchCollect_, nullptr);
    DTEST_LOG << "OnReceiveBluetoothEvent003 end" << std::endl;
}

/**
 * @tc.name: OnReceiveEvent001
 * @tc.desc: test OnReceiveEvent with wifi event
 * @tc.type: FUNC
 * @tc.require: I7RSCL
 */

HWTEST_F(DeviceSwitchCollectTest, OnReceiveEvent001, TestSize.Level3)
{
    DTEST_LOG << "OnReceiveEvent001 begin" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    deviceSwitchCollect->InitCommonEventSubscriber();
    AAFwk::Want want;
    EventFwk::CommonEventData data;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_WIFI_POWER_STATE);
    data.SetWant(want);
    data.SetCode(WIFI_ON);
    deviceSwitchCollect->switchEventSubscriber_->OnReceiveEvent(data);
    EXPECT_NE(deviceSwitchCollect->switchEventSubscriber_->deviceSwitchCollect_, nullptr);
    DTEST_LOG << "OnReceiveEvent001 end" << std::endl;
}

/**
 * @tc.name: OnReceiveEvent002
 * @tc.desc: test OnReceiveEvent with bluetooth event
 * @tc.type: FUNC
 * @tc.require: I7RSCL
 */

HWTEST_F(DeviceSwitchCollectTest, OnReceiveEvent002, TestSize.Level3)
{
    DTEST_LOG << "OnReceiveEvent002 begin" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    deviceSwitchCollect->InitCommonEventSubscriber();
    AAFwk::Want want;
    EventFwk::CommonEventData data;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE);
    data.SetWant(want);
    data.SetCode(BLUETOOTH_STATE_TURN_ON);
    deviceSwitchCollect->switchEventSubscriber_->OnReceiveEvent(data);
    EXPECT_NE(deviceSwitchCollect->switchEventSubscriber_->deviceSwitchCollect_, nullptr);
    DTEST_LOG << "OnReceiveEvent002 end" << std::endl;
}

/**
 * @tc.name: OnReceiveEvent003
 * @tc.desc: test OnReceiveEvent with invalid action
 * @tc.type: FUNC
 * @tc.require: I7RSCL
 */

HWTEST_F(DeviceSwitchCollectTest, OnReceiveEvent003, TestSize.Level3)
{
    DTEST_LOG << "OnReceiveEvent003 begin" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    deviceSwitchCollect->InitCommonEventSubscriber();
    AAFwk::Want want;
    EventFwk::CommonEventData data;
    want.SetAction(INVALID_ACTION);
    data.SetWant(want);
    data.SetCode(BLUETOOTH_STATE_TURN_ON);
    deviceSwitchCollect->switchEventSubscriber_->OnReceiveEvent(data);
    EXPECT_NE(deviceSwitchCollect->switchEventSubscriber_->deviceSwitchCollect_, nullptr);
    DTEST_LOG << "OnReceiveEvent003 end" << std::endl;
}
}