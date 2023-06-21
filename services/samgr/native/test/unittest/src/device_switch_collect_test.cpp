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
constexpr int32_t INVALID_CODE = -1;
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
 * @tc.name: DeviceSwitchCollectOnStart002
 * @tc.desc: test DeviceSwitchCollectOnStart with swithes is not empty
 * @tc.type: FUNC
 * @tc.require: I76X9Q
 */

HWTEST_F(DeviceSwitchCollectTest, DeviceSwitchCollectOnStart002, TestSize.Level3)
{
    OnDemandEvent onDemandEvent1 = {SETTING_SWITCH, WIFI_NAME, "on"};
    OnDemandEvent onDemandEvent2 = {SETTING_SWITCH, BLUETOOTH_NAME, "on"};
    OnDemandEvent onDemandEvent3 = {SETTING_SWITCH, UNRELATED_NAME, "on"};
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    deviceSwitchCollect->SetSwitchEvent(onDemandEvent1);
    deviceSwitchCollect->SetSwitchEvent(onDemandEvent2);
    deviceSwitchCollect->SetSwitchEvent(onDemandEvent3);
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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->subscribeCountMap_.clear();
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

/**
 * @tc.name: AddCollectEvent004
 * @tc.desc: test AddCollectEvent with no event
 * @tc.type: FUNC
 * @tc.require: I76X9Q
 */

HWTEST_F(DeviceSwitchCollectTest, AddCollectEvent004, TestSize.Level3)
{
    OnDemandEvent onDemandEvent;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    int32_t ret = deviceSwitchCollect->AddCollectEvent(onDemandEvent);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AddCollectEvent005
 * @tc.desc: test AddCollectEvent with event is already existed
 * @tc.type: FUNC
 * @tc.require: I6V388
 */

HWTEST_F(DeviceSwitchCollectTest, AddCollectEvent005, TestSize.Level3)
{
    OnDemandEvent onDemandEvent = {SETTING_SWITCH, BLUETOOTH_NAME, "on"};
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    deviceSwitchCollect->switches_.insert(BLUETOOTH_NAME);
    int32_t ret = deviceSwitchCollect->AddCollectEvent(onDemandEvent);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: isContainSwitch001
 * @tc.desc: cover isContainSwitch
 * @tc.type: FUNC
 * @tc.require: I7FBV6
 */

HWTEST_F(DeviceSwitchCollectTest, isContainSwitch001, TestSize.Level3)
{
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    deviceSwitchCollect->switches_.insert(WIFI_NAME);
    bool ret = deviceSwitchCollect->isContainSwitch(WIFI_NAME);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: OnAddSystemAbility001
 * @tc.desc: test OnAddSystemAbility with correct saID and does not have switch
 * @tc.type: FUNC
 * @tc.require: I7FBV6
 */

HWTEST_F(DeviceSwitchCollectTest, OnAddSystemAbility001, TestSize.Level3)
{
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    sptr<SwitchStateListener> listener = new SwitchStateListener(deviceSwitchCollect);
    listener->OnAddSystemAbility(COMMON_EVENT_ID, DEVICE_ID);
    EXPECT_TRUE(deviceSwitchCollect->switches_.empty());
}

/**
 * @tc.name: OnAddSystemAbility002
 * @tc.desc: test OnAddSystemAbility with correct said and has wifi status
 * @tc.type: FUNC
 * @tc.require: I7FBV6
 */

HWTEST_F(DeviceSwitchCollectTest, OnAddSystemAbility002, TestSize.Level3)
{
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    deviceSwitchCollect->switches_.clear();
    deviceSwitchCollect->switches_.insert(WIFI_NAME);
    sptr<SwitchStateListener> listener = new SwitchStateListener(deviceSwitchCollect);
    listener->OnAddSystemAbility(COMMON_EVENT_ID, DEVICE_ID);
    EXPECT_EQ(deviceSwitchCollect->switches_.size(), 1);
}

/**
 * @tc.name: OnAddSystemAbility003
 * @tc.desc: test OnAddSystemAbility with correct said and has bluetooth status
 * @tc.type: FUNC
 * @tc.require: I7FBV6
 */

HWTEST_F(DeviceSwitchCollectTest, OnAddSystemAbility003, TestSize.Level3)
{
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
        deviceSwitchCollect->switches_.clear();
    deviceSwitchCollect->switches_.insert(BLUETOOTH_NAME);
    sptr<SwitchStateListener> listener = new SwitchStateListener(deviceSwitchCollect);
    listener->OnAddSystemAbility(COMMON_EVENT_ID, DEVICE_ID);
    EXPECT_EQ(deviceSwitchCollect->switches_.size(), 1);
}

/**
 * @tc.name: OnAddSystemAbility004
 * @tc.desc: test OnAddSystemAbility with correct said and has both wifi status and bluetooth status
 * @tc.type: FUNC
 * @tc.require: I7FBV6
 */

HWTEST_F(DeviceSwitchCollectTest, OnAddSystemAbility004, TestSize.Level3)
{
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    deviceSwitchCollect->switches_.clear();
    deviceSwitchCollect->switches_.insert(WIFI_NAME);
    deviceSwitchCollect->switches_.insert(BLUETOOTH_NAME);
    sptr<SwitchStateListener> listener = new SwitchStateListener(deviceSwitchCollect);
    listener->OnAddSystemAbility(COMMON_EVENT_ID, DEVICE_ID);
    EXPECT_EQ(deviceSwitchCollect->switches_.size(), 2);
}

/**
 * @tc.name: OnAddSystemAbility005
 * @tc.desc: test OnAddSystemAbility with incorrect saID
 * @tc.type: FUNC
 * @tc.require: I7FBV6
 */

HWTEST_F(DeviceSwitchCollectTest, OnAddSystemAbility005, TestSize.Level3)
{
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    sptr<SwitchStateListener> listener = new SwitchStateListener(deviceSwitchCollect);
    listener->OnAddSystemAbility(INVALID_SAID, DEVICE_ID);
    EXPECT_TRUE(deviceSwitchCollect->switches_.empty());
}

/**
 * @tc.name: BlueToothSwitchCollectWatchState001
 * @tc.desc: cover BlueToothSwitchCollectWatchState
 * @tc.type: FUNC
 * @tc.require: I7FBV6
 */

HWTEST_F(DeviceSwitchCollectTest, BlueToothSwitchCollectWatchState001, TestSize.Level3)
{
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    std::shared_ptr<WifiSwitchCollect> btSwitchCollect = std::make_shared<WifiSwitchCollect>();
    btSwitchCollect->WatchState(deviceSwitchCollect);
    EXPECT_TRUE(deviceSwitchCollect->switches_.empty());
}

/**
 * @tc.name: BluetoothEventSubscriberOnReceiveEvent001
 * @tc.desc: test BluetoothEventSubscriberOnReceiveEvent with incorrect action
 * @tc.type: FUNC
 * @tc.require: I7FBV6
 */

HWTEST_F(DeviceSwitchCollectTest, BlueToothSwitchCollectOnReceiveEvent001, TestSize.Level3)
{
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    AAFwk::Want want;
    EventFwk::CommonEventData data;
    want.SetAction(INVALID_ACTION);
    data.SetWant(want);
    data.SetCode(BLUETOOTH_STATE_TURN_ON);
    EventFwk::MatchingSkills skill = EventFwk::MatchingSkills();
    skill.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE);
    EventFwk::CommonEventSubscribeInfo info(skill);
    std::shared_ptr<EventFwk::CommonEventSubscriber> bluetoothEventSubscriber
        = std::make_shared<BluetoothEventSubscriber>(info, deviceSwitchCollect);
    bluetoothEventSubscriber->OnReceiveEvent(data);
    EXPECT_TRUE(deviceSwitchCollect->switches_.empty());
}

/**
 * @tc.name: BluetoothEventSubscriberOnReceiveEvent002
 * @tc.desc: test BlueToothSwitchCollectOnReceiveEvent with bluetooth status is turn on
 * @tc.type: FUNC
 * @tc.require: I7FBV6
 */

HWTEST_F(DeviceSwitchCollectTest, BluetoothEventSubscriberOnReceiveEvent002, TestSize.Level3)
{
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    AAFwk::Want want;
    EventFwk::CommonEventData data;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE);
    data.SetWant(want);
    data.SetCode(BLUETOOTH_STATE_TURN_ON);
    EventFwk::MatchingSkills skill = EventFwk::MatchingSkills();
    skill.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE);
    EventFwk::CommonEventSubscribeInfo info(skill);
    std::shared_ptr<EventFwk::CommonEventSubscriber> bluetoothEventSubscriber
        = std::make_shared<BluetoothEventSubscriber>(info, deviceSwitchCollect);
    bluetoothEventSubscriber->OnReceiveEvent(data);
    EXPECT_TRUE(deviceSwitchCollect->switches_.empty());
}

/**
 * @tc.name: BluetoothEventSubscriberOnReceiveEvent003
 * @tc.desc: test BluetoothEventSubscriberOnReceiveEvent with bluetooth status is turn off
 * @tc.type: FUNC
 * @tc.require: I7FBV6
 */

HWTEST_F(DeviceSwitchCollectTest, BluetoothEventSubscriberOnReceiveEvent003, TestSize.Level3)
{
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    AAFwk::Want want;
    EventFwk::CommonEventData data;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE);
    data.SetWant(want);
    data.SetCode(BLUETOOTH_STATE_TURN_OFF);
    EventFwk::MatchingSkills skill = EventFwk::MatchingSkills();
    skill.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE);
    EventFwk::CommonEventSubscribeInfo info(skill);
    std::shared_ptr<EventFwk::CommonEventSubscriber> bluetoothEventSubscriber
        = std::make_shared<BluetoothEventSubscriber>(info, deviceSwitchCollect);
    bluetoothEventSubscriber->OnReceiveEvent(data);
    EXPECT_TRUE(deviceSwitchCollect->switches_.empty());
}

/**
 * @tc.name:BluetoothEventSubscriberOnReceiveEvent004
 * @tc.desc: test BluetoothEventSubscriberOnReceiveEvent with incorrect code
 * @tc.type: FUNC
 * @tc.require: I7FBV6
 */

HWTEST_F(DeviceSwitchCollectTest, BluetoothEventSubscriberOnReceiveEvent004, TestSize.Level3)
{
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    AAFwk::Want want;
    EventFwk::CommonEventData data;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE);
    data.SetWant(want);
    data.SetCode(INVALID_CODE);
    EventFwk::MatchingSkills skill = EventFwk::MatchingSkills();
    skill.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE);
    EventFwk::CommonEventSubscribeInfo info(skill);
    std::shared_ptr<EventFwk::CommonEventSubscriber> bluetoothEventSubscriber
        = std::make_shared<BluetoothEventSubscriber>(info, deviceSwitchCollect);
    bluetoothEventSubscriber->OnReceiveEvent(data);
    EXPECT_TRUE(deviceSwitchCollect->switches_.empty());
}

/**
 * @tc.name:  WifiSwitchCollecttWatchState001
 * @tc.desc: cover  WifiSwitchCollectCollectWatchState
 * @tc.type: FUNC
 * @tc.require: I7FBV6
 */

HWTEST_F(DeviceSwitchCollectTest, WifiSwitchCollecttWatchState001, TestSize.Level3)
{
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    std::shared_ptr<WifiSwitchCollect> wifiSwitchCollect = std::make_shared<WifiSwitchCollect>();
    wifiSwitchCollect->WatchState(deviceSwitchCollect);
    EXPECT_TRUE(deviceSwitchCollect->switches_.empty());
}

/**
 * @tc.name: WifiEventSubscriberOnReceiveEvent001
 * @tc.desc: test WifiEventSubscriberOnReceiveEvent with incorrect action
 * @tc.type: FUNC
 * @tc.require: I7FBV6
 */

HWTEST_F(DeviceSwitchCollectTest, WifiEventSubscriberOnReceiveEvent001, TestSize.Level3)
{
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    AAFwk::Want want;
    EventFwk::CommonEventData data;
    want.SetAction(INVALID_ACTION);
    data.SetWant(want);
    data.SetCode(BLUETOOTH_STATE_TURN_ON);
    EventFwk::MatchingSkills skill = EventFwk::MatchingSkills();
    skill.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE);
    EventFwk::CommonEventSubscribeInfo info(skill);
    std::shared_ptr<EventFwk::CommonEventSubscriber> wifiEventSubscriber
        = std::make_shared<WifiEventSubscriber>(info, deviceSwitchCollect);
    wifiEventSubscriber->OnReceiveEvent(data);
    EXPECT_TRUE(deviceSwitchCollect->switches_.empty());
}

/**
 * @tc.name: WifiEventSubscriberOnReceiveEvent002
 * @tc.desc: test WifiEventSubscriberOnReceiveEvent with wifi status is turn on
 * @tc.type: FUNC
 * @tc.require: I7FBV6
 */

HWTEST_F(DeviceSwitchCollectTest, WifiEventSubscriberOnReceiveEvent002, TestSize.Level3)
{
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    AAFwk::Want want;
    EventFwk::CommonEventData data;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE);
    data.SetWant(want);
    data.SetCode(WIFI_ON);
    EventFwk::MatchingSkills skill = EventFwk::MatchingSkills();
    skill.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE);
    EventFwk::CommonEventSubscribeInfo info(skill);
    std::shared_ptr<EventFwk::CommonEventSubscriber> wifiEventSubscriber
        = std::make_shared<WifiEventSubscriber>(info, deviceSwitchCollect);
    wifiEventSubscriber->OnReceiveEvent(data);
    EXPECT_TRUE(deviceSwitchCollect->switches_.empty());
}

/**
 * @tc.name: WifiEventSubscriberOnReceiveEvent003
 * @tc.desc: test WifiEventSubscriberOnReceiveEvent with wifi status is turn off
 * @tc.type: FUNC
 * @tc.require: I7FBV6
 */

HWTEST_F(DeviceSwitchCollectTest, WifiEventSubscriberOnReceiveEvent003, TestSize.Level3)
{
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    AAFwk::Want want;
    EventFwk::CommonEventData data;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE);
    data.SetWant(want);
    data.SetCode(WIFI_OFF);
    EventFwk::MatchingSkills skill = EventFwk::MatchingSkills();
    skill.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE);
    EventFwk::CommonEventSubscribeInfo info(skill);
    std::shared_ptr<EventFwk::CommonEventSubscriber> wifiEventSubscriber
        = std::make_shared<WifiEventSubscriber>(info, deviceSwitchCollect);
    wifiEventSubscriber->OnReceiveEvent(data);
    EXPECT_TRUE(deviceSwitchCollect->switches_.empty());
}

/**
 * @tc.name: WifiEventSubscriberOnReceiveEvent004
 * @tc.desc: test WifiEventSubscriberOnReceiveEvent with incorrect code
 * @tc.type: FUNC
 * @tc.require: I7FBV6
 */

HWTEST_F(DeviceSwitchCollectTest, WifiEventSubscriberOnReceiveEvent004, TestSize.Level3)
{
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    AAFwk::Want want;
    EventFwk::CommonEventData data;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE);
    data.SetWant(want);
    data.SetCode(INVALID_CODE);
    EventFwk::MatchingSkills skill = EventFwk::MatchingSkills();
    skill.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE);
    EventFwk::CommonEventSubscribeInfo info(skill);
    std::shared_ptr<EventFwk::CommonEventSubscriber> wifiEventSubscriber
        = std::make_shared<WifiEventSubscriber>(info, deviceSwitchCollect);
    wifiEventSubscriber->OnReceiveEvent(data);
    EXPECT_TRUE(deviceSwitchCollect->switches_.empty());
}
}