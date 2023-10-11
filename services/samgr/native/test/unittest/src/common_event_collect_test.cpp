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
    EXPECT_EQ(ERR_INVALID_VALUE, ret);
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
    DTEST_LOG << " init002 BEGIN" << std::endl;
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
    auto runner = AppExecFwk::EventRunner::Create("collect_test1");
    commonEventCollect->workHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    int32_t ret = commonEventCollect->workHandler_->SendEvent(COMMON_DIED_EVENT + 1);
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
    auto runner = AppExecFwk::EventRunner::Create("collect_test1");
    std::shared_ptr<CommonHandler> commonHandler = std::make_shared<CommonHandler>(runner, commonEventCollect);
    AppExecFwk::InnerEvent *event = nullptr;
    auto destructor = [](AppExecFwk::InnerEvent *event) {
        if (event != nullptr) {
            delete event;
        }
    };
    commonHandler->ProcessEvent(AppExecFwk::InnerEvent::Pointer(event, destructor));
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
    auto runner = AppExecFwk::EventRunner::Create("collect_test1");
    std::shared_ptr<CommonHandler> commonHandler = std::make_shared<CommonHandler>(runner, commonEventCollect);
    AppExecFwk::InnerEvent *event = new AppExecFwk::InnerEvent();
    auto destructor = [](AppExecFwk::InnerEvent *event) {
        if (event != nullptr) {
            delete event;
        }
    };
    commonHandler->ProcessEvent(AppExecFwk::InnerEvent::Pointer(event, destructor));
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
    auto runner = AppExecFwk::EventRunner::Create("collect_test1");
    std::shared_ptr<CommonHandler> commonHandler = std::make_shared<CommonHandler>(runner, commonEventCollect);
    AppExecFwk::InnerEvent *event = new AppExecFwk::InnerEvent();
    event->innerEventId_ = -1;
    auto destructor = [](AppExecFwk::InnerEvent *event) {
        if (event != nullptr) {
            delete event;
        }
    };
    commonHandler->ProcessEvent(AppExecFwk::InnerEvent::Pointer(event, destructor));
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
    auto runner = AppExecFwk::EventRunner::Create("collect_test1");
    commonEventCollect->workHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
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
    OnDemandEvent event;
    int32_t ret = commonEventCollect->AddCollectEvent(event);
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
    auto runner = AppExecFwk::EventRunner::Create("collect_test1");
    commonEventCollect->workHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
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
    auto runner = AppExecFwk::EventRunner::Create("collect_test1");
    commonEventCollect->workHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    SaProfile saProfile;
    std::map<std::string, std::string> extraMessages;
    extraMessages["12"] = "56";
    extraMessages["34"] = "34";
    extraMessages["abc"] = "abc";
    extraMessages["xxx"] = "true";
    extraMessages["yyy"] = "false";
    std::vector<OnDemandCondition> condition;
    OnDemandEvent event = {COMMON_EVENT, "", "", -1, false, condition, false, 3, extraMessages};
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
    auto runner = AppExecFwk::EventRunner::Create("collect_test1");
    commonEventCollect->workHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
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
    auto runner = AppExecFwk::EventRunner::Create("collect_test1");
    commonEventCollect->workHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
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
    auto runner = AppExecFwk::EventRunner::Create("collect_test1");
    commonEventCollect->workHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    commonEventCollect->extraDatas_.clear();
    OnDemandReasonExtraData onDemandReasonExtraData;
    EventFwk::CommonEventData eventData;
    commonEventCollect->SaveOnDemandReasonExtraData(eventData);
    bool ret = commonEventCollect->GetOnDemandReasonExtraData(1, onDemandReasonExtraData);
    EXPECT_TRUE(ret);
}
} // namespace OHOS