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
#include "common_event_collect_test.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "sa_profiles.h"
#include "string_ex.h"
#include "test_log.h"

#define private public
#include "common_event_collect.h"
#include "device_status_collect_manager.h"
#include "event_handler.h"
#undef private

using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS;

namespace OHOS {
namespace {
constexpr uint32_t COMMON_DIED_EVENT = 11;
constexpr const char* COMMON_EVENT_ACTION_NAME = "common_event_action_name";
}


void CommonEventCollectTest::SetUpTestCase()
{
    DTEST_LOG << "SetUpTestCase" << std::endl;
}

void CommonEventCollectTest::TearDownTestCase()
{
    DTEST_LOG << "TearDownTestCase" << std::endl;
}

void CommonEventCollectTest::SetUp()
{
    DTEST_LOG << "SetUp" << std::endl;
}

void CommonEventCollectTest::TearDown()
{
    DTEST_LOG << "TearDown" << std::endl;
}

/**
 * @tc.name: OnStart001
 * @tc.desc: test Onstart
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventCollectTest, OnStart001, TestSize.Level3)
{
    DTEST_LOG << " OnStart001 BEGIN" << std::endl;
    sptr<CommonEventCollect> commonEventCollect = new CommonEventCollect(nullptr);
    int32_t ret = commonEventCollect->OnStart();
    EXPECT_EQ(ERR_OK, ret);
}

/**
 * @tc.name: OnStart002
 * @tc.desc: test Onstart
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventCollectTest, OnStart002, TestSize.Level3)
{
    DTEST_LOG << " OnStart001 BEGIN" << std::endl;
    sptr<CommonEventCollect> commonEventCollect = new CommonEventCollect(nullptr);
    commonEventCollect->commonEventNames_.insert("test");
    int32_t ret = commonEventCollect->OnStart();
    EXPECT_EQ(ERR_OK, ret);
}

/**
 * @tc.name: OnStop001
 * @tc.desc: test OnStop
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventCollectTest, OnStop001, TestSize.Level3)
{
    DTEST_LOG << " OnStop001 BEGIN" << std::endl;
    sptr<CommonEventCollect> commonEventCollect = new CommonEventCollect(nullptr);
    commonEventCollect->workHandler_ = std::make_shared<CommonHandler>(commonEventCollect);
    int32_t ret = commonEventCollect->OnStop();
    EXPECT_EQ(ERR_OK, ret);
}

/**
 * @tc.name: OnStop002
 * @tc.desc: test OnStop
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventCollectTest, OnStop002, TestSize.Level3)
{
    DTEST_LOG << " OnStop002 BEGIN" << std::endl;
    sptr<CommonEventCollect> commonEventCollect = new CommonEventCollect(nullptr);
    commonEventCollect->workHandler_ = nullptr;
    int32_t ret = commonEventCollect->OnStop();
    EXPECT_EQ(ERR_OK, ret);
}

/**
 * @tc.name: init001
 * @tc.desc: test init
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventCollectTest, init001, TestSize.Level3)
{
    DTEST_LOG << " init001 BEGIN" << std::endl;
    shared_ptr<CommonEventCollect> commonEventCollect = make_shared<CommonEventCollect>(nullptr);
    SaProfile saProfile;
    saProfile.startOnDemand.onDemandEvents.push_back({COMMON_EVENT, "", ""});
    saProfile.stopOnDemand.onDemandEvents.push_back({COMMON_EVENT, "", ""});
    std::list<SaProfile> onDemandSaProfiles;
    onDemandSaProfiles.push_back(saProfile);
    commonEventCollect->Init(onDemandSaProfiles);
    commonEventCollect->workHandler_ = nullptr;
    int32_t ret = commonEventCollect->OnStop();
    EXPECT_EQ(ERR_OK, ret);
}

/**
 * @tc.name: ProcessEvent001
 * @tc.desc: test ProcessEvent
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventCollectTest, ProcessEvent001, TestSize.Level3)
{
    DTEST_LOG << " ProcessEvent001 BEGIN" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<CommonEventCollect> commonEventCollect = new CommonEventCollect(collect);
    commonEventCollect->workHandler_ = nullptr;
    int32_t ret = commonEventCollect->SendEvent(COMMON_DIED_EVENT + 1);
    EXPECT_EQ(false, ret);
    commonEventCollect->workHandler_ = std::make_shared<CommonHandler>(commonEventCollect);
    ret = commonEventCollect->workHandler_->SendEvent(COMMON_DIED_EVENT + 1);
    EXPECT_EQ(true, ret);
    auto workHandler = std::static_pointer_cast<CommonHandler>(commonEventCollect->workHandler_);
    workHandler->commonCollect_ = nullptr;
    ret = commonEventCollect->workHandler_->SendEvent(COMMON_DIED_EVENT + 1);
    EXPECT_EQ(true, ret);
    ret = commonEventCollect->workHandler_->SendEvent(COMMON_DIED_EVENT);
    EXPECT_EQ(true, ret);
    DTEST_LOG << " ProcessEvent001 END" << std::endl;
}

/**
 * @tc.name: ProcessEvent002
 * @tc.desc: test ProcessEvent, event is nullptr
 * @tc.type: FUNC
 * @tc.require: I6OU0A
 */
HWTEST_F(CommonEventCollectTest, ProcessEvent002, TestSize.Level3)
{
    DTEST_LOG << "ProcessEvent002 begin" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<CommonEventCollect> commonEventCollect = new CommonEventCollect(collect);
    std::shared_ptr<CommonHandler> commonHandler = std::make_shared<CommonHandler>(commonEventCollect);
    AppExecFwk::InnerEvent *event = nullptr;
    auto destructor = [](AppExecFwk::InnerEvent *event) {
        if (event != nullptr) {
            delete event;
        }
    };
    commonHandler->ProcessEvent(COMMON_DIED_EVENT, 0);
    EXPECT_EQ(event, nullptr);
    DTEST_LOG << "ProcessEvent002 end" << std::endl;
}

/**
 * @tc.name: ProcessEvent003
 * @tc.desc: test ProcessEvent, commonCollect_ is nullptr
 * @tc.type: FUNC
 * @tc.require: I6OU0A
 */
HWTEST_F(CommonEventCollectTest, ProcessEvent003, TestSize.Level3)
{
    DTEST_LOG << "ProcessEvent003 begin" << std::endl;
    sptr<CommonEventCollect> commonEventCollect = nullptr;
    std::shared_ptr<CommonHandler> commonHandler = std::make_shared<CommonHandler>(commonEventCollect);
    AppExecFwk::InnerEvent *event = new AppExecFwk::InnerEvent();
    auto destructor = [](AppExecFwk::InnerEvent *event) {
        if (event != nullptr) {
            delete event;
        }
    };
    commonHandler->ProcessEvent(COMMON_DIED_EVENT, 0);
    EXPECT_NE(event, nullptr);
    DTEST_LOG << "ProcessEvent003 end" << std::endl;
}

/**
 * @tc.name: ProcessEvent004
 * @tc.desc: test ProcessEvent, eventId is invalid
 * @tc.type: FUNC
 * @tc.require: I6OU0A
 */
HWTEST_F(CommonEventCollectTest, ProcessEvent004, TestSize.Level3)
{
    DTEST_LOG << "ProcessEvent004 begin" << std::endl;
    sptr<CommonEventCollect> commonEventCollect = nullptr;
    std::shared_ptr<CommonHandler> commonHandler = std::make_shared<CommonHandler>(commonEventCollect);
    AppExecFwk::InnerEvent *event = new AppExecFwk::InnerEvent();
    event->innerEventId_ = static_cast<uint32_t>(-1);
    auto destructor = [](AppExecFwk::InnerEvent *event) {
        if (event != nullptr) {
            delete event;
        }
    };
    commonHandler->ProcessEvent(-1, 0);
    EXPECT_NE(event, nullptr);
    DTEST_LOG << "ProcessEvent004 end" << std::endl;
}

/**
 * @tc.name: OnReceiveEvent001
 * @tc.desc: test OnReceiveEvent
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventCollectTest, OnReceiveEvent001, TestSize.Level3)
{
    DTEST_LOG << " OnReceiveEvent001 BEGIN" << std::endl;
    sptr<CommonEventCollect> commonEventCollect = new CommonEventCollect(nullptr);
    EventFwk::MatchingSkills skill = EventFwk::MatchingSkills();
    EventFwk::CommonEventSubscribeInfo info(skill);
    std::shared_ptr<CommonEventSubscriber> commonEventStatusSubscriber
        = std::make_shared<CommonEventSubscriber>(info, commonEventCollect);
    EXPECT_NE(commonEventStatusSubscriber, nullptr);
    commonEventCollect->workHandler_ = std::make_shared<CommonHandler>(commonEventCollect);
    EventFwk::CommonEventData eventData;
    commonEventStatusSubscriber->OnReceiveEvent(eventData);
    std::string action = EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON;
    commonEventCollect->SaveAction(action);
    action = EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF;
    commonEventCollect->SaveAction(action);
    action = EventFwk::CommonEventSupport::COMMON_EVENT_CHARGING;
    commonEventCollect->SaveAction(action);
    action = EventFwk::CommonEventSupport::COMMON_EVENT_DISCHARGING;
    commonEventCollect->SaveAction(action);
    action = EventFwk::CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED;
    commonEventCollect->SaveAction(action);
    commonEventCollect->workHandler_ = nullptr;
    int32_t ret = commonEventCollect->OnStop();
    EXPECT_EQ(ERR_OK, ret);
}

/**
 * @tc.name: AddCollectEvent001
 * @tc.desc: test AddCollectEvent, with event
 * @tc.type: FUNC
 * @tc.require: I6UUNW
 */
HWTEST_F(CommonEventCollectTest, AddCollectEvent001, TestSize.Level3)
{
    DTEST_LOG << "AddCollectEvent001 begin" << std::endl;
    sptr<CommonEventCollect> commonEventCollect = new CommonEventCollect(nullptr);
    OnDemandEvent event = {COMMON_EVENT, "TEST", "TEST"};
    int32_t ret = commonEventCollect->AddCollectEvent(event);
    EXPECT_EQ(ret, ERR_OK);
    ret = commonEventCollect->AddCollectEvent(event);
    EXPECT_EQ(ret, ERR_OK);
    DTEST_LOG << "AddCollectEvent001 end" << std::endl;
}

/**
 * @tc.name: RemoveUnusedEvent001
 * @tc.desc: test RemoveUnusedEvent, with event.name is not in commonEventNames_
 * @tc.type: FUNC
 * @tc.require: I7VZ98
 */
HWTEST_F(CommonEventCollectTest, RemoveUnusedEvent001, TestSize.Level3)
{
    sptr<CommonEventCollect> commonEventCollect = new CommonEventCollect(nullptr);
    OnDemandEvent event = {COMMON_EVENT, "", ""};
    OnDemandEvent event1 = {COMMON_EVENT, "", ""};
    OnDemandEvent event2 = {COMMON_EVENT, "TEST", "TEST"};
    EXPECT_EQ(event == event1, true);
    EXPECT_EQ(event1 == event2, false);
    int32_t ret = commonEventCollect->RemoveUnusedEvent(event);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: RemoveUnusedEvent002
 * @tc.desc: test RemoveUnusedEvent, with event.name in commonEventNames_
 * @tc.type: FUNC
 * @tc.require: I7VZ98
 */
HWTEST_F(CommonEventCollectTest, RemoveUnusedEvent002, TestSize.Level3)
{
    sptr<CommonEventCollect> commonEventCollect = new CommonEventCollect(nullptr);
    commonEventCollect->commonEventNames_.insert("usual.event.SCREEN_ON");
    OnDemandEvent event = {COMMON_EVENT, "usual.event.SCREEN_ON", ""};
    commonEventCollect->RemoveUnusedEvent(event);
    EXPECT_EQ(commonEventCollect->commonEventNames_.size(), 0);
}

/**
 * @tc.name: SaveOnDemandReasonExtraData001
 * @tc.desc: test SaveOnDemandReasonExtraData with one CommonEventData
 * @tc.type: FUNC
 * @tc.require: I6W735
 */
HWTEST_F(CommonEventCollectTest, SaveOnDemandReasonExtraData001, TestSize.Level3)
{
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<CommonEventCollect> commonEventCollect = new CommonEventCollect(collect);
    commonEventCollect->workHandler_ = std::make_shared<CommonHandler>(commonEventCollect);
    EventFwk::CommonEventData eventData;
    int64_t ret = commonEventCollect->SaveOnDemandReasonExtraData(eventData);
    EXPECT_EQ(ret, 1);
}

/**
 * @tc.name: SaveOnDemandReasonExtraData002
 * @tc.desc: test SaveOnDemandReasonExtraData ,parse Want
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventCollectTest, SaveOnDemandReasonExtraData002, TestSize.Level3)
{
    DTEST_LOG << "SaveOnDemandReasonExtraData002 begin" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<CommonEventCollect> commonEventCollect = new CommonEventCollect(collect);
    commonEventCollect->workHandler_ = std::make_shared<CommonHandler>(commonEventCollect);
    SaProfile saProfile;
    std::map<std::string, std::string> extraMessages;
    extraMessages["12"] = "56";
    extraMessages["34"] = "34";
    extraMessages["abc"] = "abc";
    extraMessages["xxx"] = "true";
    extraMessages["yyy"] = "false";
    std::vector<OnDemandCondition> conditions;
    OnDemandEvent event = {COMMON_EVENT, "", "", -1, false, conditions, false, 3, extraMessages};
    saProfile.startOnDemand.onDemandEvents.push_back(event);
    std::list<SaProfile> onDemandSaProfiles;
    onDemandSaProfiles.push_back(saProfile);
    commonEventCollect->Init(onDemandSaProfiles);
    for (auto pair: extraMessages) {
        EXPECT_TRUE(commonEventCollect->extraDataKey_[""].count(pair.first));
    }

    EventFwk::CommonEventData eventData;
    auto want = eventData.GetWant();
    want.SetParam((const std::string)"12", 56);
    want.SetParam((const std::string)"34", (const std::string)"34");
    want.SetParam((const std::string)"abc", (const std::string)"abc");
    want.SetParam((const std::string)"xxx", true);
    want.SetParam((const std::string)"yyy", false);
    eventData.SetWant(want);
    int64_t extraDataId = commonEventCollect->SaveOnDemandReasonExtraData(eventData);
    OnDemandReasonExtraData onDemandReasonExtraData;
    commonEventCollect->GetOnDemandReasonExtraData(extraDataId, onDemandReasonExtraData);
    std::map<std::string, std::string> want2 = onDemandReasonExtraData.GetWant();
    for (auto pair : extraMessages) {
        EXPECT_TRUE(want2[pair.first] == pair.second);
    }
    DTEST_LOG << "SaveOnDemandReasonExtraData002 end" << std::endl;
}

/**
 * @tc.name: RemoveOnDemandReasonExtraData001
 * @tc.desc: test RemoveOnDemandReasonExtraData
 * @tc.type: FUNC
 * @tc.require: I6W735
 */
HWTEST_F(CommonEventCollectTest, RemoveOnDemandReasonExtraData001, TestSize.Level3)
{
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<CommonEventCollect> commonEventCollect = new CommonEventCollect(collect);
    commonEventCollect->workHandler_ = std::make_shared<CommonHandler>(commonEventCollect);
    EventFwk::CommonEventData eventData;
    commonEventCollect->extraDatas_.clear();
    commonEventCollect->SaveOnDemandReasonExtraData(eventData);
    commonEventCollect->RemoveOnDemandReasonExtraData(1);
    EXPECT_TRUE(commonEventCollect->extraDatas_.empty());
}

/**
 * @tc.name: GetOnDemandReasonExtraData001
 * @tc.desc: test GetOnDemandReasonExtraData while ExtraData is not exist
 * @tc.type: FUNC
 * @tc.require: I6W735
 */
HWTEST_F(CommonEventCollectTest, GetOnDemandReasonExtraData001, TestSize.Level3)
{
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<CommonEventCollect> commonEventCollect = new CommonEventCollect(collect);
    commonEventCollect->workHandler_ = std::make_shared<CommonHandler>(commonEventCollect);
    commonEventCollect->extraDatas_.clear();
    OnDemandReasonExtraData onDemandReasonExtraData;
    bool ret = commonEventCollect->GetOnDemandReasonExtraData(1, onDemandReasonExtraData);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: RemoveOnDemandReasonExtraData002
 * @tc.desc: test GetOnDemandReasonExtraData while ExtraData is exist
 * @tc.type: FUNC
 * @tc.require: I6W735
 */
HWTEST_F(CommonEventCollectTest, GetOnDemandReasonExtraData002, TestSize.Level3)
{
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<CommonEventCollect> commonEventCollect = new CommonEventCollect(collect);
    commonEventCollect->workHandler_ = std::make_shared<CommonHandler>(commonEventCollect);
    commonEventCollect->extraDatas_.clear();
    OnDemandReasonExtraData onDemandReasonExtraData;
    EventFwk::CommonEventData eventData;
    commonEventCollect->SaveOnDemandReasonExtraData(eventData);
    bool ret = commonEventCollect->GetOnDemandReasonExtraData(1, onDemandReasonExtraData);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: InitCommonEventState001
 * @tc.desc: test InitCommonEventState with COMMON_EVENT
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventCollectTest, InitCommonEventState001, TestSize.Level3)
{
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<CommonEventCollect> commonEventCollect = new CommonEventCollect(collect);
    commonEventCollect->workHandler_ = std::make_shared<CommonHandler>(commonEventCollect);
    
    std::map<std::string, std::string> extraMessages;
    extraMessages["12"] = "56";
    extraMessages["abc"] = "abc";
    std::vector<OnDemandCondition> conditions;
    OnDemandEvent event = {COMMON_EVENT, "1", "", -1, false, conditions, false, 3, extraMessages};
    commonEventCollect->InitCommonEventState(event);
    EXPECT_EQ(commonEventCollect->commonEventNames_.size(), 1);
    EXPECT_EQ(commonEventCollect->extraDataKey_["1"].size(), 2);
}

/**
 * @tc.name: InitCommonEventState002
 * @tc.desc: test InitCommonEventState with conditions
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventCollectTest, InitCommonEventState002, TestSize.Level3)
{
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<CommonEventCollect> commonEventCollect = new CommonEventCollect(collect);
    commonEventCollect->workHandler_ = std::make_shared<CommonHandler>(commonEventCollect);
    
    std::vector<OnDemandCondition> conditions;
    std::map<std::string, std::string> extraMessages;
    extraMessages["12"] = "56";
    OnDemandCondition condition = {COMMON_EVENT, "1", "", extraMessages};
    conditions.push_back(condition);
    condition = {COMMON_EVENT, "2", ""};
    conditions.push_back(condition);
    condition = {PARAM, "3", ""};
    conditions.push_back(condition);
    OnDemandEvent event = {PARAM, "1", "", -1, false, conditions, false, 3, extraMessages};

    commonEventCollect->InitCommonEventState(event);
    EXPECT_EQ(commonEventCollect->commonEventNames_.size(), 2);
    EXPECT_EQ(commonEventCollect->commonEventConditionExtraData_["1"].size(), 1);
}

/**
 * @tc.name: GetParamFromWant001
 * @tc.desc: test GetParamFromWant
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventCollectTest, GetParamFromWant001, TestSize.Level3)
{
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<CommonEventCollect> commonEventCollect = new CommonEventCollect(collect);
    commonEventCollect->workHandler_ = std::make_shared<CommonHandler>(commonEventCollect);
    AAFwk::Want want;
    want.SetParam("0", false);
    want.SetParam("1", true);
    want.SetParam("2", 2);
    std::string stringValue = "3";
    want.SetParam("3", stringValue);
    EXPECT_EQ(commonEventCollect->GetParamFromWant("0", want), "false");
    EXPECT_EQ(commonEventCollect->GetParamFromWant("1", want), "true");
    EXPECT_EQ(commonEventCollect->GetParamFromWant("2", want), "2");
    EXPECT_EQ(commonEventCollect->GetParamFromWant("3", want), "3");
    EXPECT_EQ(commonEventCollect->GetParamFromWant("4", want), "");
}

/**
 * @tc.name: GetExtraDataIdlist001
 * @tc.desc: test GetExtraDataIdlist001 with one CommonEventData
 * @tc.type: FUNC
 * @tc.require: I6W735
 */
HWTEST_F(CommonEventCollectTest, GetExtraDataIdlist001, TestSize.Level3)
{
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<CommonEventCollect> commonEventCollect = new CommonEventCollect(collect);
    commonEventCollect->workHandler_ = std::make_shared<CommonHandler>(commonEventCollect);
    commonEventCollect->extraDatas_.clear();
    int64_t extraId = 100000;
    int32_t saId = 1234;

    std::map<std::string, std::string> wantMap1;
    wantMap1[COMMON_EVENT_ACTION_NAME] = "common_event1";
    OnDemandReasonExtraData extraData1(1, "1", wantMap1);
    commonEventCollect->extraDatas_[extraId] = extraData1;

    std::map<std::string, std::string> wantMap2;
    wantMap2[COMMON_EVENT_ACTION_NAME] = "common_event2";
    OnDemandReasonExtraData extraData2(2, "2", wantMap2);
    commonEventCollect->extraDatas_[extraId + 1] = extraData2;

    commonEventCollect->SaveSaExtraDataId(saId, extraId);
    commonEventCollect->SaveSaExtraDataId(saId, extraId + 1);
    commonEventCollect->SaveSaExtraDataId(saId, extraId + 2);
    commonEventCollect->SaveSaExtraDataId(saId + 1, extraId);
    commonEventCollect->SaveSaExtraDataId(saId + 1, extraId + 1);
    commonEventCollect->SaveSaExtraDataId(saId + 1, extraId + 2);
    EXPECT_EQ(commonEventCollect->saExtraDataIdMap_.size(), 2);

    std::vector<int64_t> extraDataIdList;
    int32_t ret = commonEventCollect->GetSaExtraDataIdList(1, extraDataIdList);
    EXPECT_EQ(extraDataIdList.size(), 0);
    extraDataIdList.clear();

    ret = commonEventCollect->GetSaExtraDataIdList(saId, extraDataIdList);
    EXPECT_EQ(extraDataIdList.size(), 3);
    extraDataIdList.clear();

    ret = commonEventCollect->GetSaExtraDataIdList(saId, extraDataIdList, "common_event1");
    EXPECT_EQ(extraDataIdList.size(), 1);
    commonEventCollect->extraDatas_.clear();
    commonEventCollect->saExtraDataIdMap_.clear();
}

/**
 * @tc.name: GetExtraDataIdlist001
 * @tc.desc: test GetExtraDataIdlist001 with remove id
 * @tc.type: FUNC
 * @tc.require: I6W735
 */
HWTEST_F(CommonEventCollectTest, GetExtraDataIdlist002, TestSize.Level3)
{
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<CommonEventCollect> commonEventCollect = new CommonEventCollect(collect);
    commonEventCollect->workHandler_ = std::make_shared<CommonHandler>(commonEventCollect);
    commonEventCollect->extraDatas_.clear();
    int64_t extraId = 100000;
    int32_t saId = 1234;

    std::map<std::string, std::string> wantMap1;
    wantMap1[COMMON_EVENT_ACTION_NAME] = "common_event1";
    OnDemandReasonExtraData extraData1(1, "1", wantMap1);
    commonEventCollect->extraDatas_[extraId] = extraData1;

    std::map<std::string, std::string> wantMap2;
    wantMap2[COMMON_EVENT_ACTION_NAME] = "common_event2";
    OnDemandReasonExtraData extraData2(2, "2", wantMap2);
    commonEventCollect->extraDatas_[extraId + 1] = extraData2;

    commonEventCollect->SaveSaExtraDataId(saId, extraId);
    commonEventCollect->SaveSaExtraDataId(saId, extraId + 1);
    commonEventCollect->SaveSaExtraDataId(saId, extraId + 2);
    commonEventCollect->SaveSaExtraDataId(saId + 1, extraId);
    commonEventCollect->SaveSaExtraDataId(saId + 1, extraId + 1);
    commonEventCollect->SaveSaExtraDataId(saId + 1, extraId + 2);
    commonEventCollect->SaveSaExtraDataId(saId + 1, extraId + 3);
    EXPECT_EQ(commonEventCollect->saExtraDataIdMap_.size(), 2);

    std::vector<int64_t> extraDataIdList;
    commonEventCollect->RemoveSaExtraDataId(extraId + 1);
    int32_t ret = commonEventCollect->GetSaExtraDataIdList(saId, extraDataIdList, "common_event1");
    EXPECT_EQ(extraDataIdList.size(), 1);
    extraDataIdList.clear();

    commonEventCollect->RemoveSaExtraDataId(extraId);
    ret = commonEventCollect->GetSaExtraDataIdList(saId, extraDataIdList, "common_event1");
    EXPECT_EQ(extraDataIdList.size(), 0);
    extraDataIdList.clear();

    commonEventCollect->RemoveSaExtraDataId(extraId + 2);
    EXPECT_EQ(commonEventCollect->saExtraDataIdMap_.size(), 1);

    ret = commonEventCollect->GetSaExtraDataIdList(saId + 1, extraDataIdList);
    EXPECT_EQ(extraDataIdList.size(), 1);
    commonEventCollect->extraDatas_.clear();
    commonEventCollect->saExtraDataIdMap_.clear();
}

/**
 * @tc.name: CheckCondition001
 * @tc.desc: test CheckCondition while check ConditionExtraData_ failed
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventCollectTest, CheckCondition001, TestSize.Level3)
{
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<CommonEventCollect> commonEventCollect = new CommonEventCollect(collect);
    commonEventCollect->workHandler_ = std::make_shared<CommonHandler>(commonEventCollect);

    std::map<std::string, std::string> extraMessages;
    extraMessages["12"] = "56";
    OnDemandCondition condition = {COMMON_EVENT, "1", "", extraMessages};
    commonEventCollect->commonEventConditionExtraData_[condition.name]["12"] = "";

    bool ret = commonEventCollect->CheckCondition(condition);
    EXPECT_EQ(ret, false);
    DTEST_LOG << " CheckCondition001 END" << std::endl;
}

/**
 * @tc.name: CheckCondition002
 * @tc.desc: test CheckCondition while check commonEventConditionValue_ failed
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventCollectTest, CheckCondition002, TestSize.Level3)
{
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<CommonEventCollect> commonEventCollect = new CommonEventCollect(collect);
    commonEventCollect->workHandler_ = std::make_shared<CommonHandler>(commonEventCollect);

    std::map<std::string, std::string> extraMessages;
    extraMessages["12"] = "56";
    OnDemandCondition condition = {COMMON_EVENT, "1", "17", extraMessages};
    commonEventCollect->commonEventConditionExtraData_[condition.name] = extraMessages;
    commonEventCollect->commonEventConditionValue_[condition.name] = "14";

    bool ret = commonEventCollect->CheckCondition(condition);
    EXPECT_EQ(ret, false);
    DTEST_LOG << " CheckCondition002 END" << std::endl;
}

/**
 * @tc.name: CheckCondition003
 * @tc.desc: test CheckCondition while check commonEventWhitelist failed
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventCollectTest, CheckCondition003, TestSize.Level3)
{
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<CommonEventCollect> commonEventCollect = new CommonEventCollect(collect);
    commonEventCollect->workHandler_ = std::make_shared<CommonHandler>(commonEventCollect);

    std::map<std::string, std::string> extraMessages;
    extraMessages["12"] = "56";
    OnDemandCondition condition = {COMMON_EVENT,
        EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON, "", extraMessages};
    commonEventCollect->commonEventConditionExtraData_[condition.name] = extraMessages;

    bool ret = commonEventCollect->CheckCondition(condition);
    EXPECT_EQ(ret, false);
    DTEST_LOG << " CheckCondition003 END" << std::endl;
}

/**
 * @tc.name: CheckCondition004
 * @tc.desc: test CheckCondition passed
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventCollectTest, CheckCondition004, TestSize.Level3)
{
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<CommonEventCollect> commonEventCollect = new CommonEventCollect(collect);
    commonEventCollect->workHandler_ = std::make_shared<CommonHandler>(commonEventCollect);

    std::map<std::string, std::string> extraMessages;
    extraMessages["12"] = "56";
    OnDemandCondition condition = {COMMON_EVENT, "1", "", extraMessages};
    commonEventCollect->commonEventConditionExtraData_[condition.name] = extraMessages;

    bool ret = commonEventCollect->CheckCondition(condition);
    EXPECT_EQ(ret, true);
    DTEST_LOG << " CheckCondition004 END" << std::endl;
}

/**
 * @tc.name: CheckExtraMessage001
 * @tc.desc: test CheckExtraMessage, not get extraData
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventCollectTest, CheckExtraMessage001, TestSize.Level3)
{
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<CommonEventCollect> commonEventCollect = new CommonEventCollect(collect);
    commonEventCollect->workHandler_ = std::make_shared<CommonHandler>(commonEventCollect);

    std::map<std::string, std::string> want;
    want["1"] = "1";
    OnDemandReasonExtraData extraData = OnDemandReasonExtraData(1, "", want);
    int64_t extraDataId = 1;
    commonEventCollect->extraDatas_[extraDataId] = extraData;
    OnDemandEvent profile;
    profile.eventId = COMMON_EVENT;
    profile.extraMessages["1"] = "1";

    bool ret = commonEventCollect->CheckExtraMessage(2, profile);
    EXPECT_EQ(ret, false);
    DTEST_LOG << " CheckExtraMessages001 END" << std::endl;
}

/**
 * @tc.name: CheckExtraMessage002
 * @tc.desc: test CheckExtraMessage passed
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventCollectTest, CheckExtraMessage002, TestSize.Level3)
{
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<CommonEventCollect> commonEventCollect = new CommonEventCollect(collect);
    commonEventCollect->workHandler_ = std::make_shared<CommonHandler>(commonEventCollect);

    std::map<std::string, std::string> want;
    want["1"] = "1";
    OnDemandReasonExtraData extraData = OnDemandReasonExtraData(1, "", want);
    int64_t extraDataId = 1;
    commonEventCollect->extraDatas_[extraDataId] = extraData;
    OnDemandEvent profile;
    profile.eventId = COMMON_EVENT;
    profile.extraMessages["1"] = "1";

    bool ret = commonEventCollect->CheckExtraMessage(extraDataId, profile);
    EXPECT_EQ(ret, true);
    DTEST_LOG << " CheckExtraMessages002 END" << std::endl;
}

/**
 * @tc.name: CheckExtraMessage003
 * @tc.desc: test CheckExtraMessage failed
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventCollectTest, CheckExtraMessage003, TestSize.Level3)
{
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<CommonEventCollect> commonEventCollect = new CommonEventCollect(collect);
    commonEventCollect->workHandler_ = std::make_shared<CommonHandler>(commonEventCollect);

    std::map<std::string, std::string> want;
    want["1"] = "2";
    OnDemandReasonExtraData extraData = OnDemandReasonExtraData(1, "", want);
    int64_t extraDataId = 1;
    commonEventCollect->extraDatas_[extraDataId] = extraData;
    OnDemandEvent profile;
    profile.eventId = COMMON_EVENT;
    profile.extraMessages["1"] = "1";

    bool ret = commonEventCollect->CheckExtraMessage(extraDataId, profile);
    EXPECT_EQ(ret, false);
    DTEST_LOG << " CheckExtraMessages003 END" << std::endl;
}

/**
 * @tc.name: SaveOnDemandConditionExtraData001
 * @tc.desc: test SaveOnDemandConditionExtraData
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventCollectTest, SaveOnDemandConditionExtraData001, TestSize.Level3)
{
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<CommonEventCollect> commonEventCollect = new CommonEventCollect(collect);
    commonEventCollect->workHandler_ = std::make_shared<CommonHandler>(commonEventCollect);

    AAFwk::Want want;
    std::string action = "abc";
    want.SetParam("1", 1);
    want.SetAction(action);
    EventFwk::CommonEventData eventData = {want, 2, action};

    commonEventCollect->commonEventConditionExtraData_[action]["1"] = "";
    commonEventCollect->SaveOnDemandConditionExtraData(eventData);
    EXPECT_EQ(commonEventCollect->commonEventConditionValue_[action], "2");
    EXPECT_EQ(commonEventCollect->commonEventConditionExtraData_[action]["1"], "1");
    DTEST_LOG << " SaveOnDemandConditionExtraData001 END" << std::endl;
}

/**
 * @tc.name: RemoveWhiteCommonEvent001
 * @tc.desc: test RemoveWhiteCommonEvent
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventCollectTest, RemoveWhiteCommonEvent001, TestSize.Level3)
{
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<CommonEventCollect> commonEventCollect = new CommonEventCollect(collect);
    commonEventCollect->RemoveWhiteCommonEvent();
    EXPECT_NE(commonEventCollect, nullptr);
    DTEST_LOG << " RemoveWhiteCommonEvent001 END" << std::endl;
}

/**
 * @tc.name: CleanFailedEventLocked001
 * @tc.desc: test CleanFailedEventLocked
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventCollectTest, CleanFailedEventLocked001, TestSize.Level3)
{
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<CommonEventCollect> commonEventCollect = new CommonEventCollect(collect);
    EventFwk::MatchingSkills skill = EventFwk::MatchingSkills();
    EventFwk::CommonEventSubscribeInfo info(skill);
    std::shared_ptr<CommonEventSubscriber> commonEventStatusSubscriber
        = std::make_shared<CommonEventSubscriber>(info, commonEventCollect);
    commonEventCollect->commonEventSubscriber_ = commonEventStatusSubscriber;
    commonEventCollect->CleanFailedEventLocked("test");
    commonEventCollect->commonEventSubscriber_ = nullptr;
    commonEventCollect->CleanFailedEventLocked("test");
    EXPECT_NE(commonEventCollect, nullptr);
    DTEST_LOG << " CleanFailedEventLocked001 END" << std::endl;
}

/**
 * @tc.name: OnRemoveSystemAbility001
 * @tc.desc: test OnRemoveSystemAbility
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventCollectTest, OnRemoveSystemAbility001, TestSize.Level3)
{
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<CommonEventCollect> commonEventCollect = new CommonEventCollect(collect);
    sptr<CommonEventListener> commonEventListener = new CommonEventListener(commonEventCollect);
    commonEventListener->OnRemoveSystemAbility(1, "test");
    commonEventListener->commonEventCollect_ = nullptr;
    commonEventListener->OnAddSystemAbility(1, "test");
    EXPECT_NE(commonEventCollect, nullptr);
    DTEST_LOG << " OnRemoveSystemAbility001 END" << std::endl;
}
} // namespace OHOS