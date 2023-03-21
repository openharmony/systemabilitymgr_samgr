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
    commonEventCollect->commonEventNames_.push_back("test");
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
    DTEST_LOG << " init001 BEGIN" << std::endl;
    sptr<CommonEventCollect> commonEventCollect = new CommonEventCollect(nullptr);
    const std::list<SaProfile> onDemandSaProfiles;
    commonEventCollect->Init(onDemandSaProfiles);
    int32_t ret = commonEventCollect->AddCommonListener();
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: init002
 * @tc.desc: test init
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventCollectTest, init002, TestSize.Level3)
{
    DTEST_LOG << " init002 BEGIN" << std::endl;
    shared_ptr<CommonEventCollect> commonEventCollect = make_shared<CommonEventCollect>(nullptr);
    SaProfile saProfile;
    saProfile.startOnDemand.push_back({COMMON_EVENT, "", ""});
    saProfile.stopOnDemand.push_back({COMMON_EVENT, "", ""});
    std::list<SaProfile> onDemandSaProfiles;
    onDemandSaProfiles.push_back(saProfile);
    commonEventCollect->Init(onDemandSaProfiles);
    EXPECT_NE(nullptr, commonEventCollect);
}

/**
 * @tc.name: IsCesReady001
 * @tc.desc: test IsCesReady
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventCollectTest, IsCesReady001, TestSize.Level3)
{
    DTEST_LOG << " IsCesReady001 BEGIN" << std::endl;
    sptr<CommonEventCollect> commonEventCollect = new CommonEventCollect(nullptr);
    bool ret = commonEventCollect->IsCesReady();
    EXPECT_EQ(false, ret);
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
    EXPECT_NE(commonEventStatusSubscriber, nullptr);
}
} // namespace OHOS