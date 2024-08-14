/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "device_status_collect_manager_test.h"

#include "sa_profiles.h"
#include "string_ex.h"
#include "test_log.h"

#define private public
#include "common_event_collect.h"
#include "device_status_collect_manager.h"
#ifdef SUPPORT_COMMON_EVENT
#include "common_event_collect.h"
#include "common_event_manager.h"
#include "device_switch_collect.h"
#endif

#ifdef SUPPORT_DEVICE_MANAGER
#include "device_networking_collect.h"
#endif

#ifdef SUPPORT_SWITCH_COLLECT
#include "device_switch_collect.h"
#endif

using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS;

namespace OHOS {
namespace {
constexpr int32_t MAX_WAIT_TIME = 10000;
constexpr int64_t EXTRA_ID = 1;
const std::string SA_TAG_DEVICE_ON_LINE = "deviceonline";
const std::string WIFI_NAME = "wifi_status";
constexpr int32_t MOCK_PLUGIN = 20;
sptr<DeviceStatusCollectManager> collect;
}

void DeviceStatusCollectManagerTest::SetUpTestCase()
{
    DTEST_LOG << "SetUpTestCase" << std::endl;
}

void DeviceStatusCollectManagerTest::TearDownTestCase()
{
    DTEST_LOG << "TearDownTestCase" << std::endl;
}

void DeviceStatusCollectManagerTest::SetUp()
{
    collect = new DeviceStatusCollectManager();
    DTEST_LOG << "SetUp" << std::endl;
}

void DeviceStatusCollectManagerTest::TearDown()
{
    collect->CleanFfrt();
    DTEST_LOG << "TearDown" << std::endl;
}

void DeviceStatusCollectManagerTest::PostTask(
    std::shared_ptr<FFRTHandler>& collectHandler)
{
    isCaseDone = false;
    auto caseDoneNotifyTask = [this]() {
        std::lock_guard<std::mutex> autoLock(caseDoneLock_);
        isCaseDone = true;
        caseDoneCondition_.notify_one();
    };
    if (collectHandler != nullptr) {
        collectHandler->PostTask(caseDoneNotifyTask);
    }
    std::unique_lock<std::mutex> lock(caseDoneLock_);
    caseDoneCondition_.wait_for(lock, std::chrono::milliseconds(MAX_WAIT_TIME),
        [&] () { return isCaseDone; });
}

bool MockCollectPlugin::CheckCondition(const OnDemandCondition& condition)
{
    return mockCheckConditionResult_;
}

/**
 * @tc.name: FilterOnDemandSaProfiles001
 * @tc.desc: test FilterOnDemandSaProfiles with different parameters
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusCollectManagerTest, FilterOnDemandSaProfiles001, TestSize.Level3)
{
    DTEST_LOG << " FilterOnDemandSaProfiles001 BEGIN" << std::endl;
    std::list<SaProfile> saProfiles;
    collect->FilterOnDemandSaProfiles(saProfiles);
    EXPECT_EQ(true, collect->onDemandSaProfiles_.empty());
    SaProfile saProfile;
    OnDemandEvent event = { DEVICE_ONLINE, SA_TAG_DEVICE_ON_LINE, "on" };
    saProfile.startOnDemand.onDemandEvents.emplace_back(event);
    saProfiles.emplace_back(saProfile);
    collect->FilterOnDemandSaProfiles(saProfiles);
    EXPECT_EQ(false, collect->onDemandSaProfiles_.empty());
    OnDemandEvent event1 = { DEVICE_ONLINE, SA_TAG_DEVICE_ON_LINE, "off" };
    saProfile.stopOnDemand.onDemandEvents.emplace_back(event1);
    saProfiles.emplace_back(saProfile);
    collect->FilterOnDemandSaProfiles(saProfiles);
    EXPECT_EQ(false, collect->onDemandSaProfiles_.empty());
    DTEST_LOG << " FilterOnDemandSaProfiles001 END" << std::endl;
}

/**
 * @tc.name: GetSaControlListByEvent001
 * @tc.desc: test GetSaControlListByEvent with different parameters
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusCollectManagerTest, GetSaControlListByEvent001, TestSize.Level3)
{
    DTEST_LOG << " GetSaControlListByEvent001 BEGIN" << std::endl;
    collect->collectPluginMap_[DEVICE_ONLINE] = new MockCollectPlugin(collect);
    OnDemandEvent event = { DEVICE_ONLINE, SA_TAG_DEVICE_ON_LINE, "on" };
    std::list<SaControlInfo> saControlList;
    collect->GetSaControlListByEvent(event, saControlList);
    EXPECT_EQ(true, saControlList.empty());
    SaProfile saProfile;
    OnDemandEvent event1 = { DEVICE_ONLINE, SA_TAG_DEVICE_ON_LINE, "on" };
    OnDemandEvent event2 = { DEVICE_ONLINE, SA_TAG_DEVICE_ON_LINE, "off" };
    saProfile.startOnDemand.onDemandEvents.emplace_back(event1);
    saProfile.stopOnDemand.onDemandEvents.emplace_back(event2);
    collect->onDemandSaProfiles_.emplace_back(saProfile);
    collect->GetSaControlListByEvent(event, saControlList);
    EXPECT_EQ(false, saControlList.empty());
    saControlList.clear();
    event.value = "off";
    collect->GetSaControlListByEvent(event, saControlList);
    EXPECT_EQ(false, saControlList.empty());
    saControlList.clear();
    event.value = "";
    collect->GetSaControlListByEvent(event, saControlList);
    EXPECT_EQ(true, saControlList.empty());
    event.name = "settingswitch";
    collect->GetSaControlListByEvent(event, saControlList);
    EXPECT_EQ(true, saControlList.empty());
    event.eventId = SETTING_SWITCH;
    collect->GetSaControlListByEvent(event, saControlList);
    EXPECT_EQ(true, saControlList.empty());
    DTEST_LOG << " GetSaControlListByEvent001 END" << std::endl;
}

/**
 * @tc.name: SortSaControlListByLoadPriority001
 * @tc.desc: test SortSaControlListByLoadPriority with saprofiles that have different ondemand priority
 * @tc.type: FUNC
 * @tc.require: I7VXXO
 */
HWTEST_F(DeviceStatusCollectManagerTest, SortSaControlListByLoadPriority001, TestSize.Level3)
{
    DTEST_LOG << " SaControlListByEvent001 BEGIN" << std::endl;
    collect->collectPluginMap_[DEVICE_ONLINE] = new MockCollectPlugin(collect);
    OnDemandEvent event = { DEVICE_ONLINE, SA_TAG_DEVICE_ON_LINE, "on" };
    OnDemandEvent event1 = { DEVICE_ONLINE, SA_TAG_DEVICE_ON_LINE, "on" };
    event1.loadPriority = 2;
    OnDemandEvent event2 = { DEVICE_ONLINE, SA_TAG_DEVICE_ON_LINE, "on" };
    event2.loadPriority = 1;
    SaProfile saProfile1;
    saProfile1.startOnDemand.onDemandEvents.emplace_back(event1);
    SaProfile saProfile2;
    saProfile2.startOnDemand.onDemandEvents.emplace_back(event2);
    collect->onDemandSaProfiles_.emplace_back(saProfile1);
    collect->onDemandSaProfiles_.emplace_back(saProfile2);

    std::list<SaControlInfo> saControlList;
    collect->GetSaControlListByEvent(event, saControlList);
    EXPECT_EQ(2, saControlList.front().loadPriority);
    EXPECT_EQ(1, saControlList.back().loadPriority);

    collect->SortSaControlListByLoadPriority(saControlList);
    EXPECT_EQ(1, saControlList.front().loadPriority);
    EXPECT_EQ(2, saControlList.back().loadPriority);
}

/**
 * @tc.name: UnInit001
 * @tc.desc: test UnInit
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusCollectManagerTest, UnInit001, TestSize.Level3)
{
    DTEST_LOG << " UnInit001 BEGIN" << std::endl;
    collect->UnInit();
    EXPECT_EQ(true, collect->collectPluginMap_.empty());

    sptr<CommonEventCollect> commonEventCollect = new CommonEventCollect(nullptr);
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    sptr<ICollectPlugin> networkingCollect = new DeviceNetworkingCollect(nullptr);
    collect->collectPluginMap_[COMMON_EVENT] = commonEventCollect;
    collect->collectPluginMap_[SETTING_SWITCH] = deviceSwitchCollect;
    collect->collectPluginMap_[DEVICE_ONLINE] = networkingCollect;
    collect->collectHandler_ = std::make_shared<FFRTHandler>("collect");

    collect->SetFfrt();
    collect->CleanFfrt();
    collect->UnInit();
    EXPECT_EQ(true, collect->collectPluginMap_.empty());
    DTEST_LOG << " UnInit001 END" << std::endl;
}

/**
 * @tc.name: GetSaExtraDataIdList001
 * @tc.desc: test GetSaExtraDataIdList
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusCollectManagerTest, GetSaExtraDataIdList001, TestSize.Level3)
{
    OnDemandEvent event = { DEVICE_ONLINE, SA_TAG_DEVICE_ON_LINE, "on" };
    std::list<SaControlInfo> saControlList;
    collect->SaveCacheCommonEventSaExtraId(event, saControlList);
    collect->SaveSaExtraDataId(1, 1);
    collect->ClearSaExtraDataId(1);

    std::vector<int64_t> extraDataIdList;
    int32_t ret = collect->GetSaExtraDataIdList(1, extraDataIdList, "test");
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    sptr<CommonEventCollect> commonEventCollect = new CommonEventCollect(nullptr);
    collect->collectPluginMap_[COMMON_EVENT] = commonEventCollect;

    ret = collect->GetSaExtraDataIdList(1, extraDataIdList, "");
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: StartCollect001
 * @tc.desc: test StartCollect with empty collectHandler.
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusCollectManagerTest, StartCollect001, TestSize.Level3)
{
    DTEST_LOG << " StartCollect001 BEGIN" << std::endl;
    collect->StartCollect();
    EXPECT_EQ(nullptr, collect->collectHandler_);
    DTEST_LOG << " StartCollect001 END" << std::endl;
}

/**
 * @tc.name: CheckConditions001
 * @tc.desc: test CheckConditions, with empty conditions.
 * @tc.type: FUNC
 * @tc.require: I6JE38
 */
HWTEST_F(DeviceStatusCollectManagerTest, CheckConditions001, TestSize.Level3)
{
    DTEST_LOG << " CheckConditions001 BEGIN" << std::endl;
    std::list<SaProfile> saProfiles;

    OnDemandEvent event;
    bool result = collect->CheckConditions(event);
    EXPECT_EQ(result, true);
    DTEST_LOG << " CheckConditions001 END" << std::endl;
}

/**
 * @tc.name: CheckConditions002
 * @tc.desc: test CheckConditions, with invalid condition eventId.
 * @tc.type: FUNC
 * @tc.require: I6JE38
 */
HWTEST_F(DeviceStatusCollectManagerTest, CheckConditions002, TestSize.Level3)
{
    DTEST_LOG << " CheckConditions002 BEGIN" << std::endl;
    std::list<SaProfile> saProfiles;
    OnDemandCondition condition;
    condition.eventId = -1;
    OnDemandEvent event;
    event.conditions.push_back(condition);
    bool result = collect->CheckConditions(event);
    EXPECT_EQ(result, false);
    DTEST_LOG << " CheckConditions002 END" << std::endl;
}

/**
 * @tc.name: CheckConditions003
 * @tc.desc: test CheckConditions, with collect plugin is nullptr.
 * @tc.type: FUNC
 * @tc.require: I6JE38
 */
HWTEST_F(DeviceStatusCollectManagerTest, CheckConditions003, TestSize.Level3)
{
    DTEST_LOG << " CheckConditions003 BEGIN" << std::endl;
    std::list<SaProfile> saProfiles;
    collect->collectPluginMap_[MOCK_PLUGIN] = nullptr;
    OnDemandCondition condition;
    condition.eventId = MOCK_PLUGIN;
    OnDemandEvent event;
    event.conditions.push_back(condition);
    bool result = collect->CheckConditions(event);
    EXPECT_EQ(result, false);
    DTEST_LOG << " CheckConditions003 END" << std::endl;
}

/**
 * @tc.name: CheckConditions004
 * @tc.desc: test CheckConditions, with condition not pass.
 * @tc.type: FUNC
 * @tc.require: I6JE38
 */
HWTEST_F(DeviceStatusCollectManagerTest, CheckConditions004, TestSize.Level3)
{
    DTEST_LOG << " CheckConditions004 BEGIN" << std::endl;
    std::list<SaProfile> saProfiles;
    collect->collectPluginMap_[MOCK_PLUGIN] = new MockCollectPlugin(collect);
    OnDemandCondition condition;
    condition.eventId = MOCK_PLUGIN;
    OnDemandEvent event;
    event.conditions.push_back(condition);
    bool result = collect->CheckConditions(event);
    EXPECT_EQ(result, false);
    DTEST_LOG << " CheckConditions004 END" << std::endl;
}

/**
 * @tc.name: CheckConditions005
 * @tc.desc: test CheckConditions, with condition pass.
 * @tc.type: FUNC
 * @tc.require: I6JE38
 */
HWTEST_F(DeviceStatusCollectManagerTest, CheckConditions005, TestSize.Level3)
{
    DTEST_LOG << " CheckConditions005 BEGIN" << std::endl;
    std::list<SaProfile> saProfiles;
    sptr<MockCollectPlugin> mockCollectPlugin = new MockCollectPlugin(collect);
    mockCollectPlugin->mockCheckConditionResult_ = true;
    collect->collectPluginMap_[MOCK_PLUGIN] = mockCollectPlugin;
    OnDemandCondition condition;
    condition.eventId = MOCK_PLUGIN;
    OnDemandEvent event;
    event.conditions.push_back(condition);
    bool result = collect->CheckConditions(event);
    EXPECT_EQ(result, true);
    DTEST_LOG << " CheckConditions005 END" << std::endl;
}

/**
 * @tc.name: CheckExtraMessages001
 * @tc.desc: test CheckExtraMessages, with empty OnDemandEvent.
 * @tc.type: FUNC
 * @tc.require: I6JE38
 */
HWTEST_F(DeviceStatusCollectManagerTest, CheckExtraMessages001, TestSize.Level3)
{
    DTEST_LOG << " CheckExtraMessages001 BEGIN" << std::endl;
    OnDemandEvent event;
    bool result = collect->CheckExtraMessages(event, event);
    EXPECT_EQ(result, false);
    DTEST_LOG << " CheckExtraMessages001 END" << std::endl;
}

/**
 * @tc.name: CheckExtraMessages002
 * @tc.desc: test CheckExtraMessages with COMMON_EVENT is not in collectPluginMap_
 * @tc.type: FUNC
 * @tc.require: I6W735
 */
HWTEST_F(DeviceStatusCollectManagerTest, CheckExtraMessages002, TestSize.Level3)
{
    collect->collectPluginMap_.clear();
    OnDemandEvent event;
    event.eventId = COMMON_EVENT;
    int32_t ret = collect->CheckExtraMessages(event, event);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: CheckExtraMessages003
 * @tc.desc: test CheckExtraMessages with collectPluginMap_[COMMON_EVENT] is nullptr
 * @tc.type: FUNC
 * @tc.require: I6W735
 */
HWTEST_F(DeviceStatusCollectManagerTest, CheckExtraMessages003, TestSize.Level3)
{
    collect->collectPluginMap_.clear();
    collect->collectPluginMap_[COMMON_EVENT] = nullptr;
    collect->RemoveWhiteCommonEvent();
    OnDemandEvent event;
    event.eventId = COMMON_EVENT;
    int32_t ret = collect->CheckExtraMessages(event, event);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: CheckExtraMessages004
 * @tc.desc: test CheckExtraMessages, pass
 * @tc.type: FUNC
 * @tc.require: I6W735
 */
HWTEST_F(DeviceStatusCollectManagerTest, CheckExtraMessages004, TestSize.Level3)
{
    DTEST_LOG << " CheckExtraMessages004 BEGIN" << std::endl;
    sptr<CommonEventCollect> commonEventCollect = new CommonEventCollect(nullptr);
    std::shared_ptr<CommonHandler> commonHandler = std::make_shared<CommonHandler>(commonEventCollect);

    std::map<std::string, std::string> want;
    want["1"] = "1";
    OnDemandReasonExtraData extraData = OnDemandReasonExtraData(1, "", want);
    commonEventCollect->extraDatas_[1] = extraData;
    OnDemandEvent event, profile;
    event.extraDataId = 1;
    event.eventId = COMMON_EVENT;
    profile.eventId = COMMON_EVENT;
    profile.extraMessages["1"] = "1";

    collect->collectPluginMap_.clear();
    collect->collectPluginMap_[COMMON_EVENT] = commonEventCollect;
    collect->RemoveWhiteCommonEvent();
    bool result = collect->CheckExtraMessages(event, profile);
    EXPECT_EQ(result, true);
    DTEST_LOG << " CheckExtraMessages004 END" << std::endl;
}

/**
 * @tc.name: CheckExtraMessages005
 * @tc.desc: test CheckExtraMessages, not pass
 * @tc.type: FUNC
 * @tc.require: I6W735
 */
HWTEST_F(DeviceStatusCollectManagerTest, CheckExtraMessages005, TestSize.Level3)
{
    DTEST_LOG << " CheckExtraMessages005 BEGIN" << std::endl;
    sptr<CommonEventCollect> commonEventCollect = new CommonEventCollect(nullptr);
    std::shared_ptr<CommonHandler> commonHandler = std::make_shared<CommonHandler>(commonEventCollect);
    
    std::map<std::string, std::string> want;
    want["1"] = "1";
    OnDemandReasonExtraData extraData = OnDemandReasonExtraData(1, "", want);
    commonEventCollect->extraDatas_[1] = extraData;
    OnDemandEvent event, profile;
    event.extraDataId = 1;
    event.eventId = COMMON_EVENT;
    profile.eventId = COMMON_EVENT;
    profile.extraMessages["1"] = "2";
    
    collect->collectPluginMap_.clear();
    collect->collectPluginMap_[COMMON_EVENT] = commonEventCollect;
    bool result = collect->CheckExtraMessages(event, profile);
    EXPECT_EQ(result, false);
    DTEST_LOG << " CheckExtraMessages005 END" << std::endl;
}


/**
 * @tc.name: ReportEvent001
 * @tc.desc: test ReportEvent, with empty collectHandler.
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusCollectManagerTest, ReportEvent001, TestSize.Level3)
{
    DTEST_LOG << " ReportEvent001 BEGIN" << std::endl;
    OnDemandEvent event;
    collect->ReportEvent(event);
    EXPECT_EQ(nullptr, collect->collectHandler_);
    DTEST_LOG << " ReportEvent001 END" << std::endl;
}

/**
 * @tc.name: ReportEvent002
 * @tc.desc: test ReportEvent, with empty saControlList.
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusCollectManagerTest, ReportEvent002, TestSize.Level3)
{
    DTEST_LOG << " ReportEvent002 BEGIN" << std::endl;
    std::list<SaProfile> saProfiles;
    collect->Init(saProfiles);
    OnDemandEvent event;
    collect->ReportEvent(event);
    EXPECT_EQ(true, collect->collectHandler_ != nullptr);
    PostTask(collect->collectHandler_);
    collect->PostDelayTask(nullptr, -1);
    collect->PostDelayTask(nullptr, std::numeric_limits<int32_t>::max());
    event = { DEVICE_ONLINE, SA_TAG_DEVICE_ON_LINE, "on" };
    collect->ReportEvent(event);
    DTEST_LOG << " ReportEvent002 END" << std::endl;
}

/**
 * @tc.name: ReportEvent003
 * @tc.desc: test ReportEvent, report success.
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusCollectManagerTest, ReportEvent003, TestSize.Level3)
{
    DTEST_LOG << " ReportEvent003 BEGIN" << std::endl;
    std::list<SaProfile> saProfiles;
    collect->Init(saProfiles);
    OnDemandEvent event = { DEVICE_ONLINE, SA_TAG_DEVICE_ON_LINE, "on" };
    std::list<SaControlInfo> saControlList;
    SaProfile saProfile;
    OnDemandEvent event1 = { DEVICE_ONLINE, SA_TAG_DEVICE_ON_LINE, "on" };
    OnDemandEvent event2 = { DEVICE_ONLINE, SA_TAG_DEVICE_ON_LINE, "off" };
    saProfile.startOnDemand.onDemandEvents.emplace_back(event1);
    saProfile.stopOnDemand.onDemandEvents.emplace_back(event2);
    collect->onDemandSaProfiles_.emplace_back(saProfile);
    collect->ReportEvent(event);
    EXPECT_EQ(true, collect->collectHandler_ != nullptr);
    PostTask(collect->collectHandler_);
    DTEST_LOG << " ReportEvent003 END" << std::endl;
}

/**
 * @tc.name: AddCollectEvents001
 * @tc.desc: test AddCollectEvents, with events
 * @tc.type: FUNC
 * @tc.require: I6UUNW
 */
HWTEST_F(DeviceStatusCollectManagerTest, AddCollectEvents001, TestSize.Level3)
{
    DTEST_LOG << "AddCollectEvents001 begin" << std::endl;
    std::vector<OnDemandEvent> events;
    int32_t ret = collect->AddCollectEvents(events);
    EXPECT_EQ(ret, ERR_OK);
    DTEST_LOG << "AddCollectEvents001 end" << std::endl;
}

/**
 * @tc.name: AddCollectEvents002
 * @tc.desc: test AddCollectEvents, with event wifi on
 * @tc.type: FUNC
 * @tc.require: I76X9Q
 */
HWTEST_F(DeviceStatusCollectManagerTest, AddCollectEvents002, TestSize.Level3)
{
    DTEST_LOG << "AddCollectEvents002 begin" << std::endl;
    OnDemandEvent onDemandEvent = {SETTING_SWITCH, WIFI_NAME, "on"};
    std::vector<OnDemandEvent> events {onDemandEvent};
    int32_t ret = collect->AddCollectEvents(events);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    DTEST_LOG << "AddCollectEvents002 end" << std::endl;
}

/**
 * @tc.name: AddCollectEvents003
 * @tc.desc: test AddCollectEvents, with eventID is invalid
 * @tc.type: FUNC
 * @tc.require: I76X9Q
 */
HWTEST_F(DeviceStatusCollectManagerTest, AddCollectEvents003, TestSize.Level3)
{
    DTEST_LOG << "AddCollectEvents003 begin" << std::endl;
    OnDemandEvent onDemandEvent = {-1, WIFI_NAME, "on"};
    std::vector<OnDemandEvent> events {onDemandEvent};
    int32_t ret = collect->AddCollectEvents(events);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    DTEST_LOG << "AddCollectEvents003 end" << std::endl;
}

/**
 * @tc.name: GetOnDemandEvents001
 * @tc.desc: test GetOnDemandEvents, systemAbilityId is invalid, OnDemandPolicyType is START_POLICY
 * @tc.type: FUNC
 * @tc.require: I6UUNW
 */
HWTEST_F(DeviceStatusCollectManagerTest, GetOnDemandEvents001, TestSize.Level3)
{
    DTEST_LOG << "GetOnDemandEvents001 begin" << std::endl;
    int32_t systemAbilityId = -1;
    OnDemandPolicyType type = OnDemandPolicyType::START_POLICY;
    std::vector<OnDemandEvent> events;
    int32_t ret = collect->GetOnDemandEvents(systemAbilityId, type, events);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    DTEST_LOG << "GetOnDemandEvents001 end" << std::endl;
}

/**
 * @tc.name: GetOnDemandEvents002
 * @tc.desc: test GetOnDemandEvents, systemAbilityId is valid, OnDemandPolicyType is STOP_POLICY
 * @tc.type: FUNC
 * @tc.require: I6UUNW
 */
HWTEST_F(DeviceStatusCollectManagerTest, GetOnDemandEvents002, TestSize.Level3)
{
    DTEST_LOG << "GetOnDemandEvents002 begin" << std::endl;
    int32_t systemAbilityId = 0;
    OnDemandPolicyType type = OnDemandPolicyType::STOP_POLICY;
    std::vector<OnDemandEvent> events;
    int32_t ret = collect->GetOnDemandEvents(systemAbilityId, type, events);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    DTEST_LOG << "GetOnDemandEvents002 end" << std::endl;
}

/**
 * @tc.name: GetOnDemandEvents003
 * @tc.desc: test GetOnDemandEvents, with event wifi on, OnDemandPolicyType is STOP_POLICY
 * @tc.type: FUNC
 * @tc.require: I76X9Q
 */
HWTEST_F(DeviceStatusCollectManagerTest, GetOnDemandEvents003, TestSize.Level3)
{
    DTEST_LOG << "GetOnDemandEvents003 begin" << std::endl;
    int32_t systemAbilityId = 123;
    OnDemandPolicyType type = OnDemandPolicyType::STOP_POLICY;
    OnDemandEvent onDemandEvent = {SETTING_SWITCH, WIFI_NAME, "on"};
    SaProfile saprofile = {u"test", systemAbilityId};
    collect->onDemandSaProfiles_.push_back(saprofile);
    std::vector<OnDemandEvent> events {onDemandEvent};
    int32_t ret = collect->GetOnDemandEvents(systemAbilityId, type, events);
    EXPECT_EQ(ret, ERR_OK);
    DTEST_LOG << "GetOnDemandEvents003 end" << std::endl;
}

/**
 * @tc.name: GetOnDemandEvents004
 * @tc.desc: test GetOnDemandEvents, with event wifi on, OnDemandPolicyType is START_POLICY
 * @tc.type: FUNC
 * @tc.require: I76X9Q
 */
HWTEST_F(DeviceStatusCollectManagerTest, GetOnDemandEvents004, TestSize.Level3)
{
    DTEST_LOG << "GetOnDemandEvents004 begin" << std::endl;
    int32_t systemAbilityId = 123;
    OnDemandPolicyType type = OnDemandPolicyType::START_POLICY;
    OnDemandEvent onDemandEvent = {SETTING_SWITCH, WIFI_NAME, "on"};
    SaProfile saprofile = {u"test", systemAbilityId};
    collect->onDemandSaProfiles_.push_back(saprofile);
    std::vector<OnDemandEvent> events {onDemandEvent};
    int32_t ret = collect->GetOnDemandEvents(systemAbilityId, type, events);
    EXPECT_EQ(ret, ERR_OK);
    DTEST_LOG << "GetOnDemandEvents004 end" << std::endl;
}

/**
 * @tc.name: GetOnDemandEvents005
 * @tc.desc: test GetOnDemandEvents, with event wifi on, OnDemandPolicyType is invalid
 * @tc.type: FUNC
 * @tc.require: I76X9Q
 */
HWTEST_F(DeviceStatusCollectManagerTest, GetOnDemandEvents005, TestSize.Level3)
{
    DTEST_LOG << "GetOnDemandEvents005 begin" << std::endl;
    int32_t systemAbilityId = 123;
    OnDemandPolicyType invalidType = (OnDemandPolicyType)2;
    OnDemandEvent onDemandEvent = {SETTING_SWITCH, WIFI_NAME, "on"};
    std::vector<OnDemandEvent> events {onDemandEvent};
    int32_t ret = collect->GetOnDemandEvents(systemAbilityId, invalidType, events);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    DTEST_LOG << "GetOnDemandEvents005 end" << std::endl;
}

/**
 * @tc.name: UpdateOnDemandEvents001
 * @tc.desc: test UpdateOnDemandEvents, systemAbilityId is invalid, OnDemandPolicyType is START_POLICY
 * @tc.type: FUNC
 * @tc.require: I6UUNW
 */
HWTEST_F(DeviceStatusCollectManagerTest, UpdateOnDemandEvents001, TestSize.Level3)
{
    DTEST_LOG << "UpdateOnDemandEvents001 begin" << std::endl;
    int32_t systemAbilityId = -1;
    OnDemandPolicyType type = OnDemandPolicyType::START_POLICY;
    std::vector<OnDemandEvent> events;
    int32_t ret = collect->UpdateOnDemandEvents(systemAbilityId, type, events);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    DTEST_LOG << "UpdateOnDemandEvents001 end" << std::endl;
}

/**
 * @tc.name: UpdateOnDemandEvents002
 * @tc.desc: test UpdateOnDemandEvents, systemAbilityId is valid, OnDemandPolicyType is STOP_POLICY
 * @tc.type: FUNC
 * @tc.require: I6UUNW
 */
HWTEST_F(DeviceStatusCollectManagerTest, UpdateOnDemandEvents002, TestSize.Level3)
{
    DTEST_LOG << "UpdateOnDemandEvents002 begin" << std::endl;
    int32_t systemAbilityId = 0;
    OnDemandPolicyType type = OnDemandPolicyType::STOP_POLICY;
    std::vector<OnDemandEvent> events;
    int32_t ret = collect->UpdateOnDemandEvents(systemAbilityId, type, events);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    DTEST_LOG << "UpdateOnDemandEvents002 end" << std::endl;
}

/**
 * @tc.name: UpdateOnDemandEvents003
 * @tc.desc: test UpdateOnDemandEvents, with event wifi on
 * @tc.type: FUNC
 * @tc.require: I76X9Q
 */
HWTEST_F(DeviceStatusCollectManagerTest, UpdateOnDemandEvents003, TestSize.Level3)
{
    DTEST_LOG << "UpdateOnDemandEvents003 begin" << std::endl;
    int32_t systemAbilityId = 123;
    OnDemandPolicyType type = OnDemandPolicyType::STOP_POLICY;
    OnDemandEvent onDemandEvent = {SETTING_SWITCH, WIFI_NAME, "on"};
    SaProfile saprofile = {u"test", systemAbilityId};
    collect->onDemandSaProfiles_.push_back(saprofile);
    std::vector<OnDemandEvent> events {onDemandEvent};
    int32_t ret = collect->UpdateOnDemandEvents(systemAbilityId, type, events);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    DTEST_LOG << "UpdateOnDemandEvents003 end" << std::endl;
}

/**
 * @tc.name: GetOnDemandReasonExtraData001
 * @tc.desc: test GetOnDemandReasonExtraData with COMMON_EVENT is not in collectPluginMap_
 * @tc.type: FUNC
 * @tc.require: I6W735
 */
HWTEST_F(DeviceStatusCollectManagerTest, GetOnDemandReasonExtraData001, TestSize.Level3)
{
    collect->collectPluginMap_.clear();
    OnDemandReasonExtraData onDemandReasonExtraData;
    int32_t ret = collect->GetOnDemandReasonExtraData(EXTRA_ID, onDemandReasonExtraData);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: GetOnDemandReasonExtraData002
 * @tc.desc: test GetOnDemandReasonExtraData with collectPluginMap_[COMMON_EVENT] is nullptr
 * @tc.type: FUNC
 * @tc.require: I6W735
 */
HWTEST_F(DeviceStatusCollectManagerTest, GetOnDemandReasonExtraData002, TestSize.Level3)
{
    collect->collectPluginMap_.clear();
    collect->collectPluginMap_[COMMON_EVENT] = nullptr;
    OnDemandReasonExtraData onDemandReasonExtraData;
    int32_t ret = collect->GetOnDemandReasonExtraData(EXTRA_ID, onDemandReasonExtraData);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: GetOnDemandReasonExtraData003
 * @tc.desc: test GetOnDemandReasonExtraData with collectPluginMap_[COMMON_EVENT]'s extraDataId is not correct
 * @tc.type: FUNC
 * @tc.require: I6W735
 */
HWTEST_F(DeviceStatusCollectManagerTest, GetOnDemandReasonExtraData003, TestSize.Level3)
{
    sptr<CommonEventCollect> commonEventCollect = new CommonEventCollect(nullptr);
    collect->collectPluginMap_.clear();
    collect->collectPluginMap_[COMMON_EVENT] = commonEventCollect;
    OnDemandReasonExtraData onDemandReasonExtraData;
    int32_t ret = collect->GetOnDemandReasonExtraData(EXTRA_ID, onDemandReasonExtraData);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: GetOnDemandReasonExtraData004
 * @tc.desc: test GetOnDemandReasonExtraData, get extraData
 * @tc.type: FUNC
 * @tc.require: I6W735
 */
HWTEST_F(DeviceStatusCollectManagerTest, GetOnDemandReasonExtraData004, TestSize.Level3)
{
    sptr<CommonEventCollect> commonEventCollect = new CommonEventCollect(nullptr);
    commonEventCollect->workHandler_ = std::make_shared<CommonHandler>(commonEventCollect);
    EventFwk::CommonEventData eventData;
    commonEventCollect->SaveOnDemandReasonExtraData(eventData);
    collect->collectPluginMap_.clear();
    collect->collectPluginMap_[COMMON_EVENT] = commonEventCollect;
    OnDemandReasonExtraData onDemandReasonExtraData;
    int32_t ret = collect->GetOnDemandReasonExtraData(EXTRA_ID, onDemandReasonExtraData);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: AddCollectEvents004
 * @tc.desc: test AddCollectEvents with nullptr
 * @tc.type: FUNC
 * @tc.require: I7FBV6
 */
HWTEST_F(DeviceStatusCollectManagerTest, AddCollectEvents004, TestSize.Level3)
{
    collect->collectPluginMap_[SETTING_SWITCH] = nullptr;
    OnDemandEvent onDemandEvent = { SETTING_SWITCH, WIFI_NAME, "on" };
    std::vector<OnDemandEvent> events {onDemandEvent};
    int32_t ret = collect->AddCollectEvents(events);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AddCollectEvents005
 * @tc.desc: test AddCollectEvents with invalid event name
 * @tc.type: FUNC
 * @tc.require: I7FBV6
 */
HWTEST_F(DeviceStatusCollectManagerTest, AddCollectEvents005, TestSize.Level3)
{
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    collect->collectPluginMap_[SETTING_SWITCH] = deviceSwitchCollect;
    OnDemandEvent onDemandEvent = { SETTING_SWITCH, "test", "on" };
    std::vector<OnDemandEvent> events {onDemandEvent};
    int32_t ret = collect->AddCollectEvents(events);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: RemoveUnusedEventsLocked001
 * @tc.desc: test RemoveUnusedEventsLocked, with events is empty
 * @tc.type: FUNC
 * @tc.require: I7VZ98
 */
HWTEST_F(DeviceStatusCollectManagerTest, RemoveUnusedEventsLocked001, TestSize.Level3)
{
    std::vector<OnDemandEvent> events;
    int32_t ret = collect->RemoveUnusedEventsLocked(events);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: RemoveUnusedEventsLocked002
 * @tc.desc: test RemoveUnusedEventsLocked, with event not in collectPluginMap_
 * @tc.type: FUNC
 * @tc.require: I7VZ98
 */
HWTEST_F(DeviceStatusCollectManagerTest, RemoveUnusedEventsLocked002, TestSize.Level3)
{
    OnDemandEvent onDemandEvent = {-1, WIFI_NAME, "on"};
    std::vector<OnDemandEvent> events {onDemandEvent};
    int32_t ret = collect->RemoveUnusedEventsLocked(events);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: RemoveUnusedEventsLocked003
 * @tc.desc: test RemoveUnusedEventsLocked with nullptr
 * @tc.type: FUNC
 * @tc.require: I7VZ98
 */
HWTEST_F(DeviceStatusCollectManagerTest, RemoveUnusedEventsLocked003, TestSize.Level3)
{
    collect->collectPluginMap_[SETTING_SWITCH] = nullptr;
    OnDemandEvent onDemandEvent = { SETTING_SWITCH, WIFI_NAME, "on" };
    std::vector<OnDemandEvent> events {onDemandEvent};
    int32_t ret = collect->RemoveUnusedEventsLocked(events);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: RemoveUnusedEventsLocked004
 * @tc.desc: test RemoveUnusedEventsLocked with eventUsed is false
 * @tc.type: FUNC
 * @tc.require: I7VZ98
 */
HWTEST_F(DeviceStatusCollectManagerTest, RemoveUnusedEventsLocked004, TestSize.Level3)
{
    sptr<DeviceSwitchCollect> deviceSwitchCollect =
        new DeviceSwitchCollect(collect);
    collect->collectPluginMap_[SETTING_SWITCH] = deviceSwitchCollect;
    OnDemandEvent onDemandEvent = { SETTING_SWITCH, "test", "on" };
    std::vector<OnDemandEvent> events {onDemandEvent};
    int32_t ret = collect->RemoveUnusedEventsLocked(events);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: TypeAndSaidToString001
 * @tc.desc: test TypeAndSaidToString
 * @tc.type: FUNC
 * @tc.require: I7VZ98
 */
HWTEST_F(DeviceStatusCollectManagerTest, TypeAndSaidToString001, TestSize.Level3)
{
    OnDemandPolicyType type = OnDemandPolicyType::START_POLICY;
    std::string str = collect->TypeAndSaidToString(type, 1);
    EXPECT_EQ(str, "start#1#");

    type = OnDemandPolicyType::STOP_POLICY;
    str = collect->TypeAndSaidToString(type, 1);
    EXPECT_EQ(str, "stop#1#");
}

/**
 * @tc.name: StringToTypeAndSaid001
 * @tc.desc: test StringToTypeAndSaid
 * @tc.type: FUNC
 * @tc.require: I7VZ98
 */
HWTEST_F(DeviceStatusCollectManagerTest, StringToTypeAndSaid001, TestSize.Level3)
{
    OnDemandPolicyType type = OnDemandPolicyType::START_POLICY;
    int32_t said = 0;
    collect->StringToTypeAndSaid("start#1#", type, said);
    EXPECT_EQ(type, OnDemandPolicyType::START_POLICY);
    EXPECT_EQ(said, 1);

    said = 0;
    collect->StringToTypeAndSaid("stop#1#", type, said);
    EXPECT_EQ(type, OnDemandPolicyType::STOP_POLICY);
    EXPECT_EQ(said, 1);

    said = 0;
    collect->StringToTypeAndSaid("test#1#", type, said);
    EXPECT_EQ(said, 0);

    said = 0;
    collect->StringToTypeAndSaid("start#", type, said);
    EXPECT_EQ(said, 0);
}

/**
 * @tc.name: NeedPersistOnDemandEvent001
 * @tc.desc: test NeedPersistOnDemandEvent
 * @tc.type: FUNC
 * @tc.require: I7VZ98
 */
HWTEST_F(DeviceStatusCollectManagerTest, NeedPersistOnDemandEvent001, TestSize.Level3)
{
    OnDemandEvent onDemandEvent = { SETTING_SWITCH, "test", "on" };
    bool ret = collect->NeedPersistOnDemandEvent(onDemandEvent);
    EXPECT_EQ(ret, false);

    onDemandEvent.eventId = TIMED_EVENT;
    onDemandEvent.name = "test";
    ret = collect->NeedPersistOnDemandEvent(onDemandEvent);
    EXPECT_EQ(ret, false);

    onDemandEvent.eventId = TIMED_EVENT;
    onDemandEvent.name = "timedevent";
    onDemandEvent.persistence = false;
    ret = collect->NeedPersistOnDemandEvent(onDemandEvent);
    EXPECT_EQ(ret, false);

    onDemandEvent.persistence = true;
    ret = collect->NeedPersistOnDemandEvent(onDemandEvent);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: IsSameEventName001
 * @tc.desc: test IsSameEventName with event1 and event2 is the same
 * @tc.type: FUNC
 * @tc.require: I7VZ98
 */
HWTEST_F(DeviceStatusCollectManagerTest, IsSameEventName001, TestSize.Level3)
{
    OnDemandEvent event1 = { SETTING_SWITCH, "test", "on" };
    OnDemandEvent event2 = { SETTING_SWITCH, "test", "off" };
    bool ret = collect->IsSameEventName(event1, event2);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: IsSameEventName002
 * @tc.desc: test IsSameEventName with event1 and event2 is not the same
 * @tc.type: FUNC
 * @tc.require: I7VZ98
 */
HWTEST_F(DeviceStatusCollectManagerTest, IsSameEventName002, TestSize.Level3)
{
    OnDemandEvent event1 = { SETTING_SWITCH, "test", "on" };
    OnDemandEvent event2 = { SETTING_SWITCH, "test1", "off" };
    bool ret = collect->IsSameEventName(event1, event2);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: IsSameEventName003
 * @tc.desc: test IsSameEventName with event1 and event2 is the same
 * @tc.type: FUNC
 * @tc.require: I7VZ98
 */
HWTEST_F(DeviceStatusCollectManagerTest, IsSameEventName003, TestSize.Level3)
{
    OnDemandEvent event1 = { TIMED_EVENT, "loopevent", "60" };
    OnDemandEvent event2 = { TIMED_EVENT, "loopevent", "60" };
    bool ret = collect->IsSameEventName(event1, event2);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: IsSameEventName004
 * @tc.desc: test IsSameEventName with event1 and event2 is not the same
 * @tc.type: FUNC
 * @tc.require: I7VZ98
 */
HWTEST_F(DeviceStatusCollectManagerTest, IsSameEventName004, TestSize.Level3)
{
    OnDemandEvent event1 = { TIMED_EVENT, "loopevent", "60" };
    OnDemandEvent event2 = { TIMED_EVENT, "loopevent", "30" };
    bool ret = collect->IsSameEventName(event1, event2);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: CheckEventUsedLocked001
 * @tc.desc: test CheckEventUsedLocked with event exist
 * @tc.require: I7VZ98
 */
HWTEST_F(DeviceStatusCollectManagerTest, CheckEventUsedLocked001, TestSize.Level3)
{
    SaProfile saProfile;
    OnDemandEvent event1 = { DEVICE_ONLINE, SA_TAG_DEVICE_ON_LINE, "on" };
    OnDemandEvent event2 = { DEVICE_ONLINE, SA_TAG_DEVICE_ON_LINE, "off" };
    saProfile.startOnDemand.onDemandEvents.emplace_back(event1);
    saProfile.stopOnDemand.onDemandEvents.emplace_back(event2);
    collect->onDemandSaProfiles_.emplace_back(saProfile);
    bool ret = collect->CheckEventUsedLocked(event1);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: CheckEventUsedLocked002
 * @tc.desc: test CheckEventUsedLocked with onDemandSaProfiles_ is empty
 * @tc.require: I7VZ98
 */
HWTEST_F(DeviceStatusCollectManagerTest, CheckEventUsedLocked002, TestSize.Level3)
{
    collect->onDemandSaProfiles_.clear();
    OnDemandEvent event1 = { DEVICE_ONLINE, SA_TAG_DEVICE_ON_LINE, "on" };
    bool ret = collect->CheckEventUsedLocked(event1);
    EXPECT_EQ(ret, false);
}
} // namespace OHOS
