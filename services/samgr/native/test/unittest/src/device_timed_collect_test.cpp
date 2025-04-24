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

#include "device_timed_collect_test.h"

#include "sa_profiles.h"
#include "test_log.h"

#define private public
#include "device_status_collect_manager.h"
#include "device_timed_collect.h"
#ifdef PREFERENCES_ENABLE
#include "preferences_errno.h"
#include "preferences_helper.h"
#include "preferences_value.h"
#include "device_timed_collect_tool.h"
#endif

using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS;

namespace OHOS {
void DeviceTimedCollectTest::SetUpTestCase()
{
    DTEST_LOG << "SetUpTestCase" << std::endl;
}

void DeviceTimedCollectTest::TearDownTestCase()
{
    DTEST_LOG << "TearDownTestCase" << std::endl;
}

void DeviceTimedCollectTest::SetUp()
{
    DTEST_LOG << "SetUp" << std::endl;
}

void DeviceTimedCollectTest::TearDown()
{
    DTEST_LOG << "TearDown" << std::endl;
}

/**
 * @tc.name: Init001
 * @tc.desc: test Init
 * @tc.type: FUNC
 */
HWTEST_F(DeviceTimedCollectTest, Init001, TestSize.Level3)
{
    DTEST_LOG << "Init001 begin" << std::endl;
    std::list<SaProfile> saProfiles;
    SaProfile saProfile;
    OnDemandEvent onDemandEvent = {TIMED_EVENT, "loopevent", "40"};
    saProfile.startOnDemand.onDemandEvents.push_back(onDemandEvent);
    saProfiles.push_back(saProfile);
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    std::shared_ptr<DeviceTimedCollect> deviceTimedCollect =
        std::make_shared<DeviceTimedCollect>(collect);
    deviceTimedCollect->Init(saProfiles);
    EXPECT_EQ(deviceTimedCollect->nonPersitenceLoopEventSet_.size(), 1);
    DTEST_LOG << "Init001 end" << std::endl;
}

/**
 * @tc.name: Init002
 * @tc.desc: test Init
 * @tc.type: FUNC
 */
HWTEST_F(DeviceTimedCollectTest, Init002, TestSize.Level3)
{
    DTEST_LOG << "Init002 begin" << std::endl;
    std::list<SaProfile> saProfiles;
    SaProfile saProfile;
    OnDemandEvent onDemandEvent = {TIMED_EVENT, "mockevent", "40"};
    saProfile.startOnDemand.onDemandEvents.push_back(onDemandEvent);
    saProfiles.push_back(saProfile);
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    std::shared_ptr<DeviceTimedCollect> deviceTimedCollect =
        std::make_shared<DeviceTimedCollect>(collect);
    deviceTimedCollect->Init(saProfiles);
    EXPECT_EQ(deviceTimedCollect->nonPersitenceLoopEventSet_.size(), 0);
    DTEST_LOG << "Init002 end" << std::endl;
}

/**
 * @tc.name: Init003
 * @tc.desc: test Init
 * @tc.type: FUNC
 */
HWTEST_F(DeviceTimedCollectTest, Init003, TestSize.Level3)
{
    DTEST_LOG << "Init003 begin" << std::endl;
    std::list<SaProfile> saProfiles;
    SaProfile saProfile;
    OnDemandEvent onDemandEvent = {TIMED_EVENT, "loopevent", "40"};
    saProfile.stopOnDemand.onDemandEvents.push_back(onDemandEvent);
    saProfiles.push_back(saProfile);
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    std::shared_ptr<DeviceTimedCollect> deviceTimedCollect =
        std::make_shared<DeviceTimedCollect>(collect);
    deviceTimedCollect->Init(saProfiles);
    EXPECT_EQ(deviceTimedCollect->nonPersitenceLoopEventSet_.size(), 1);
    DTEST_LOG << "Init003 end" << std::endl;
}

/**
 * @tc.name: Init004
 * @tc.desc: test Init
 * @tc.type: FUNC
 */
HWTEST_F(DeviceTimedCollectTest, Init004, TestSize.Level3)
{
    DTEST_LOG << "Init004 begin" << std::endl;
    std::list<SaProfile> saProfiles;
    SaProfile saProfile;
    OnDemandEvent onDemandEvent = {TIMED_EVENT, "mockevent", "40"};
    saProfile.stopOnDemand.onDemandEvents.push_back(onDemandEvent);
    saProfiles.push_back(saProfile);
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    std::shared_ptr<DeviceTimedCollect> deviceTimedCollect =
        std::make_shared<DeviceTimedCollect>(collect);
    deviceTimedCollect->Init(saProfiles);
    EXPECT_EQ(deviceTimedCollect->nonPersitenceLoopEventSet_.size(), 0);
    DTEST_LOG << "Init004 end" << std::endl;
}

/**
 * @tc.name: Init005
 * @tc.desc: test Init
 * @tc.type: FUNC
 */
HWTEST_F(DeviceTimedCollectTest, Init005, TestSize.Level3)
{
    DTEST_LOG << "Init005 begin" << std::endl;
    std::list<SaProfile> saProfiles;
    SaProfile saProfile;
    OnDemandEvent onDemandEvent = {TIMED_EVENT, "loopevent", "20"};
    saProfile.stopOnDemand.onDemandEvents.push_back(onDemandEvent);
    saProfiles.push_back(saProfile);
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    std::shared_ptr<DeviceTimedCollect> deviceTimedCollect =
        std::make_shared<DeviceTimedCollect>(collect);
    deviceTimedCollect->Init(saProfiles);
    EXPECT_EQ(deviceTimedCollect->nonPersitenceLoopEventSet_.size(), 0);
    DTEST_LOG << "Init005 end" << std::endl;
}

/**
 * @tc.name: Init006
 * @tc.desc: test Init
 * @tc.type: FUNC
 */
HWTEST_F(DeviceTimedCollectTest, Init006, TestSize.Level3)
{
    DTEST_LOG << "Init006 begin" << std::endl;
    std::list<SaProfile> saProfiles;
    SaProfile saProfile;
    OnDemandEvent onDemandEvent = {TIMED_EVENT, "loopevent", "20"};
    saProfile.startOnDemand.onDemandEvents.push_back(onDemandEvent);
    saProfiles.push_back(saProfile);
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    std::shared_ptr<DeviceTimedCollect> deviceTimedCollect =
        std::make_shared<DeviceTimedCollect>(collect);
    deviceTimedCollect->Init(saProfiles);
    EXPECT_EQ(deviceTimedCollect->nonPersitenceLoopEventSet_.size(), 0);
    DTEST_LOG << "Init006 end" << std::endl;
}

/**
 * @tc.name: Init007
 * @tc.desc: test Init
 * @tc.type: FUNC
 */
HWTEST_F(DeviceTimedCollectTest, Init007, TestSize.Level3)
{
    DTEST_LOG << "Init007 begin" << std::endl;
    std::list<SaProfile> saProfiles;
    SaProfile saProfile;
    OnDemandEvent onDemandEvent = {TIMED_EVENT, "loopevent", "invalid"};
    saProfile.startOnDemand.onDemandEvents.push_back(onDemandEvent);
    saProfiles.push_back(saProfile);
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    std::shared_ptr<DeviceTimedCollect> deviceTimedCollect =
        std::make_shared<DeviceTimedCollect>(collect);
    deviceTimedCollect->Init(saProfiles);
    EXPECT_EQ(deviceTimedCollect->nonPersitenceLoopEventSet_.size(), 0);
    DTEST_LOG << "Init007 end" << std::endl;
}

/**
 * @tc.name: Init008
 * @tc.desc: test Init
 * @tc.type: FUNC
 */
HWTEST_F(DeviceTimedCollectTest, Init008, TestSize.Level3)
{
    DTEST_LOG << "Init008 begin" << std::endl;
    std::list<SaProfile> saProfiles;
    SaProfile saProfile;
    OnDemandEvent onDemandEvent = {TIMED_EVENT, "loopevent", "3600"};
    saProfile.startOnDemand.onDemandEvents.push_back(onDemandEvent);
    saProfiles.push_back(saProfile);
    SaProfile saProfile2;
    OnDemandEvent onDemandEvent2 = {TIMED_EVENT, "awakeloopevent", "3601"};
    saProfile2.startOnDemand.onDemandEvents.push_back(onDemandEvent2);
    saProfiles.push_back(saProfile2);
    SaProfile saProfile3;
    OnDemandEvent onDemandEvent3 = {TIMED_EVENT, "awakeloopevent", "36"};
    saProfile3.startOnDemand.onDemandEvents.push_back(onDemandEvent3);
    saProfiles.push_back(saProfile3);
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    std::shared_ptr<DeviceTimedCollect> deviceTimedCollect =
        std::make_shared<DeviceTimedCollect>(collect);
    deviceTimedCollect->Init(saProfiles);
    EXPECT_EQ(deviceTimedCollect->nonPersitenceLoopEventSet_.size(), 2);
    deviceTimedCollect->timeInfos_.clear();
    DTEST_LOG << "Init008 end" << std::endl;
}

/**
 * @tc.name: OnStart001
 * @tc.desc: test OnStart
 * @tc.type: FUNC
 */
HWTEST_F(DeviceTimedCollectTest, OnStart001, TestSize.Level3)
{
    DTEST_LOG << "OnStart001 begin" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    std::shared_ptr<DeviceTimedCollect> deviceTimedCollect =
        std::make_shared<DeviceTimedCollect>(collect);
    deviceTimedCollect->nonPersitenceLoopEventSet_.insert(101);
    deviceTimedCollect->OnStart();
    EXPECT_NE(collect, nullptr);
    deviceTimedCollect->timeInfos_.clear();
    DTEST_LOG << "OnStart001 end" << std::endl;
}

/**
 * @tc.name: OnStart002
 * @tc.desc: test OnStart
 * @tc.type: FUNC
 */
HWTEST_F(DeviceTimedCollectTest, OnStart002, TestSize.Level3)
{
    DTEST_LOG << "OnStart002 begin" << std::endl;
#ifdef PREFERENCES_ENABLE
    std::shared_ptr<PreferencesUtil> preferencesUtil_ = PreferencesUtil::GetInstance();
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    std::shared_ptr<DeviceTimedCollect> deviceTimedCollect =
        std::make_shared<DeviceTimedCollect>(collect);
    int64_t currentTime = TimeUtils::GetTimestamp();
    preferencesUtil_->SaveString("timedevent_2017-9-1-16:59:10", "start#149999#");
    preferencesUtil_->SaveLong("2017-9-1-16:59:10", currentTime + 10);
    preferencesUtil_->SaveLong("2017-9-1-16:59:11", currentTime - 10);
    deviceTimedCollect->OnStart();
    EXPECT_NE(collect, nullptr);
    preferencesUtil_->RefreshSync();
    preferencesUtil_->IsExist("2017-9-1-16:59:10");
    preferencesUtil_->Remove("timedevent_2017-9-1-16:59:10");
    preferencesUtil_->Remove("2017-9-1-16:59:10");
    preferencesUtil_->Remove("2017-9-1-16:59:11");
    preferencesUtil_->IsExist("2017-9-1-16:59:10");
    deviceTimedCollect->timeInfos_.clear();
#endif
    DTEST_LOG << "OnStart002 end" << std::endl;
}

/**
 * @tc.name: ReportEventByTimeInfo001
 * @tc.desc: test ReportEventByTimeInfo, collect is nullptr
 * @tc.type: FUNC
 * @tc.require: I6OU0A
 */
HWTEST_F(DeviceTimedCollectTest, ReportEventByTimeInfo001, TestSize.Level3)
{
    DTEST_LOG << "ReportEventByTimeInfo001 begin" << std::endl;
    sptr<DeviceStatusCollectManager> collect = nullptr;
    std::shared_ptr<DeviceTimedCollect> deviceTimedCollect =
        std::make_shared<DeviceTimedCollect>(collect);
    TimeInfo info;
    info.awake = true;
    info.normal = true;
    deviceTimedCollect->timeInfos_[3600] = info;
    deviceTimedCollect->ReportEventByTimeInfo(3600, false);
    EXPECT_EQ(collect, nullptr);
    DTEST_LOG << "ReportEventByTimeInfo001 end" << std::endl;
}

/**
 * @tc.name: PostDelayTaskByTimeInfo001
 * @tc.desc: test PostDelayTaskByTimeInfo, collect is nullptr
 * @tc.type: FUNC
 * @tc.require: I6OU0A
 */
HWTEST_F(DeviceTimedCollectTest, PostDelayTaskByTimeInfo001, TestSize.Level3)
{
    DTEST_LOG << "PostDelayTaskByTimeInfo001 begin" << std::endl;
    sptr<DeviceStatusCollectManager> collect = nullptr;
    std::shared_ptr<DeviceTimedCollect> deviceTimedCollect =
        std::make_shared<DeviceTimedCollect>(collect);
    TimeInfo info;
    info.awake = true;
    info.normal = true;
    deviceTimedCollect->timeInfos_[3600] = info;
    deviceTimedCollect->PostDelayTaskByTimeInfo(deviceTimedCollect->nonPersitenceLoopTasks_[0], 3600, 3600);
    EXPECT_EQ(collect, nullptr);
    deviceTimedCollect->timeInfos_.clear();
    DTEST_LOG << "PostDelayTaskByTimeInfo001 end" << std::endl;
}

/**
 * @tc.name: ReportEvent001
 * @tc.desc: test ReportEvent, report is nullptr
 * @tc.type: FUNC
 * @tc.require: I6OU0A
 */
HWTEST_F(DeviceTimedCollectTest, ReportEvent001, TestSize.Level3)
{
    DTEST_LOG << "ReportEvent001 begin" << std::endl;
    sptr<DeviceStatusCollectManager> collect = nullptr;
    std::shared_ptr<DeviceTimedCollect> deviceTimedCollect =
        std::make_shared<DeviceTimedCollect>(collect);
    OnDemandEvent event;
    deviceTimedCollect->ReportEvent(event);
    EXPECT_EQ(collect, nullptr);
    DTEST_LOG << "ReportEvent001 end" << std::endl;
}

/**
 * @tc.name: PostDelayTask001
 * @tc.desc: test ReportEvent, report is nullptr
 * @tc.type: FUNC
 * @tc.require: I6OU0A
 */
HWTEST_F(DeviceTimedCollectTest, PostDelayTask001, TestSize.Level3)
{
    DTEST_LOG << "PostDelayTask001 begin" << std::endl;
    sptr<DeviceStatusCollectManager> collect = nullptr;
    std::shared_ptr<DeviceTimedCollect> deviceTimedCollect =
        std::make_shared<DeviceTimedCollect>(collect);
    deviceTimedCollect->PostDelayTask(deviceTimedCollect->nonPersitenceLoopTasks_[0], 0);
    EXPECT_EQ(collect, nullptr);
    DTEST_LOG << "PostDelayTask001 end" << std::endl;
}

/**
 * @tc.name: PostDelayTask002
 * @tc.desc: test ReportEvent, report is not nullptr
 * @tc.type: FUNC
 * @tc.require: I6OU0A
 */
HWTEST_F(DeviceTimedCollectTest, PostDelayTask002, TestSize.Level3)
{
    DTEST_LOG << "PostDelayTask002 begin" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    collect->collectHandler_ = std::make_shared<FFRTHandler>("collect");
    std::shared_ptr<DeviceTimedCollect> deviceTimedCollect =
        std::make_shared<DeviceTimedCollect>(collect);
    deviceTimedCollect->nonPersitenceLoopTasks_[0] = [] () {};
    deviceTimedCollect->PostDelayTask(deviceTimedCollect->nonPersitenceLoopTasks_[0], 0);
    EXPECT_NE(collect, nullptr);
    DTEST_LOG << "PostDelayTask002 end" << std::endl;
}

/**
 * @tc.name: AddCollectEvent001
 * @tc.desc: test AddCollectEvent, with event
 * @tc.type: FUNC
 * @tc.require: I6UUNW
 */
HWTEST_F(DeviceTimedCollectTest, AddCollectEvent001, TestSize.Level3)
{
    DTEST_LOG << "AddCollectEvent001 begin" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    std::shared_ptr<DeviceTimedCollect> deviceTimedCollect =
        std::make_shared<DeviceTimedCollect>(collect);
    OnDemandEvent event;
    std::vector<OnDemandEvent> events;
    events.emplace_back(event);
    int32_t ret = deviceTimedCollect->AddCollectEvent(events);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    DTEST_LOG << "AddCollectEvent001 end" << std::endl;
}

/**
 * @tc.name: AddCollectEvent002
 * @tc.desc: test AddCollectEvent, with interval is less than MIN_INTERVAL
 * @tc.type: FUNC
 * @tc.require: I7FBV6
 */
HWTEST_F(DeviceTimedCollectTest, AddCollectEvent002, TestSize.Level3)
{
    DTEST_LOG << "AddCollectEvent002 begin" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    std::shared_ptr<DeviceTimedCollect> deviceTimedCollect =
        std::make_shared<DeviceTimedCollect>(collect);
    OnDemandEvent event = {TIMED_EVENT, "mockevent", "10"};
    std::vector<OnDemandEvent> events;
    events.emplace_back(event);
    int32_t ret = deviceTimedCollect->AddCollectEvent(events);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    DTEST_LOG << "AddCollectEvent002 end" << std::endl;
}

/**
 * @tc.name: AddCollectEvent003
 * @tc.desc: test AddCollectEvent with the interval is less than MIN_INTERVAL
 * @tc.type: FUNC
 * @tc.require: I7G775
 */
HWTEST_F(DeviceTimedCollectTest, AddCollectEvent003, TestSize.Level3)
{
    DTEST_LOG << "AddCollectEvent003 begin" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    std::shared_ptr<DeviceTimedCollect> deviceTimedCollect =
        std::make_shared<DeviceTimedCollect>(collect);
    OnDemandEvent event = {TIMED_EVENT, "loopevent", "10"};
    std::vector<OnDemandEvent> events;
    events.emplace_back(event);
    int32_t ret = deviceTimedCollect->AddCollectEvent(events);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    DTEST_LOG << "AddCollectEvent003 end" << std::endl;
}

/**
 * @tc.name: AddCollectEvent004
 * @tc.desc: test AddCollectEvent with the interval is less than MIN_INTERVAL
 * @tc.type: FUNC
 * @tc.require: I7G775
 */
HWTEST_F(DeviceTimedCollectTest, AddCollectEvent004, TestSize.Level3)
{
    DTEST_LOG << "AddCollectEvent004 begin" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    std::shared_ptr<DeviceTimedCollect> deviceTimedCollect =
        std::make_shared<DeviceTimedCollect>(collect);
    OnDemandEvent event = {TIMED_EVENT, "order_timed_event", "10", -1, true};
    std::vector<OnDemandEvent> events;
    events.emplace_back(event);
    int32_t ret = deviceTimedCollect->AddCollectEvent(events);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    DTEST_LOG << "AddCollectEvent004 end" << std::endl;
}

/**
 * @tc.name: AddCollectEvent005
 * @tc.desc: test AddCollectEvent with the interval is less than MIN_INTERVAL
 * @tc.type: FUNC
 * @tc.require: I7G775
 */
HWTEST_F(DeviceTimedCollectTest, AddCollectEvent005, TestSize.Level3)
{
    DTEST_LOG << "AddCollectEvent005 begin" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    std::shared_ptr<DeviceTimedCollect> deviceTimedCollect =
        std::make_shared<DeviceTimedCollect>(collect);
    OnDemandEvent event = {TIMED_EVENT, "order_timed_event", "10"};
    std::vector<OnDemandEvent> events;
    events.emplace_back(event);
    int32_t ret = deviceTimedCollect->AddCollectEvent(events);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    DTEST_LOG << "AddCollectEvent005 end" << std::endl;
}

/**
 * @tc.name: AddCollectEvent006
 * @tc.desc: test AddCollectEvent with the interval is less than MIN_INTERVAL
 * @tc.type: FUNC
 * @tc.require: I7G775
 */
HWTEST_F(DeviceTimedCollectTest, AddCollectEvent006, TestSize.Level3)
{
    DTEST_LOG << "AddCollectEvent006 begin" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    std::shared_ptr<DeviceTimedCollect> deviceTimedCollect =
        std::make_shared<DeviceTimedCollect>(collect);
    OnDemandEvent event = {TIMED_EVENT, "mockevent", "10", -1, true};
    std::vector<OnDemandEvent> events;
    events.emplace_back(event);
    int32_t ret = deviceTimedCollect->AddCollectEvent(events);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    DTEST_LOG << "AddCollectEvent006 end" << std::endl;
}

/**
 * @tc.name: AddCollectEvent007
 * @tc.type: FUNC
 * @tc.require: I7G775
 */
HWTEST_F(DeviceTimedCollectTest, AddCollectEvent007, TestSize.Level3)
{
    DTEST_LOG << "AddCollectEvent007 begin" << std::endl;
#ifdef PREFERENCES_ENABLE
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    std::shared_ptr<DeviceTimedCollect> deviceTimedCollect =
        std::make_shared<DeviceTimedCollect>(collect);
    deviceTimedCollect->preferencesUtil_ = PreferencesUtil::GetInstance();
    std::vector<OnDemandEvent> events;
    OnDemandEvent event = {TIMED_EVENT, "timedevent", "2017-9-1-16:59:10", -1, true};
    events.emplace_back(event);
    int32_t ret = deviceTimedCollect->AddCollectEvent(events);
    EXPECT_EQ(ret, ERR_OK);
    OnDemandEvent event1 = {TIMED_EVENT, "timedevent", "2099-9-1-16:59:10", -1, true};
    events.emplace_back(event1);
    ret = deviceTimedCollect->AddCollectEvent(events);
    EXPECT_EQ(ret, ERR_OK);
    OnDemandEvent event2 = {TIMED_EVENT, "awakeloopevent", "3601", -1, false};
    events.emplace_back(event2);
    ret = deviceTimedCollect->AddCollectEvent(events);
    EXPECT_EQ(ret, ERR_OK);
    OnDemandEvent event3 = {TIMED_EVENT, "awakeloopevent", "100", -1, true};
    events.emplace_back(event3);
    ret = deviceTimedCollect->AddCollectEvent(events);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    OnDemandEvent event4 = {TIMED_EVENT, "awakeloopevent", "100", -1, false};
    events.emplace_back(event4);
    ret = deviceTimedCollect->AddCollectEvent(events);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    deviceTimedCollect->timeInfos_.clear();
#endif
    DTEST_LOG << "AddCollectEvent007 end" << std::endl;
}

/**
 * @tc.name: OnStop001
 * @tc.desc: cover OnStop
 * @tc.type: FUNC
 * @tc.require: I7G775
 */
HWTEST_F(DeviceTimedCollectTest, OnStop001, TestSize.Level3)
{
    DTEST_LOG << "OnStop001 begin" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    std::shared_ptr<DeviceTimedCollect> deviceTimedCollect =
        std::make_shared<DeviceTimedCollect>(collect);
    int32_t ret = deviceTimedCollect->OnStop();
    EXPECT_EQ(ret, ERR_OK);
    DTEST_LOG << "OnStop001 end" << std::endl;
}

/**
 * @tc.name: RemoveUnusedEvent001
 * @tc.desc: test RemoveUnusedEvent, with event.name is invalid
 * @tc.type: FUNC
 * @tc.require: I7VZ98
 */
HWTEST_F(DeviceTimedCollectTest, RemoveUnusedEvent001, TestSize.Level3)
{
    DTEST_LOG << "RemoveUnusedEvent001 begin" << std::endl;
    sptr<IReport> report;
    std::shared_ptr<DeviceTimedCollect> deviceTimedCollect =
        std::make_shared<DeviceTimedCollect>(report);
    OnDemandEvent event = {TIMED_EVENT, "invalid", "10"};
    int32_t ret = deviceTimedCollect->RemoveUnusedEvent(event);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    DTEST_LOG << "RemoveUnusedEvent001 end" << std::endl;
}

/**
 * @tc.name: RemoveUnusedEvent002
 * @tc.desc: test RemoveUnusedEvent, with event.name is not in nonPersitenceLoopEventSet_
 * @tc.type: FUNC
 * @tc.require: I7VZ98
 */
HWTEST_F(DeviceTimedCollectTest, RemoveUnusedEvent002, TestSize.Level3)
{
    DTEST_LOG << "RemoveUnusedEvent002 begin" << std::endl;
    sptr<IReport> report;
    std::shared_ptr<DeviceTimedCollect> deviceTimedCollect =
        std::make_shared<DeviceTimedCollect>(report);
    OnDemandEvent event = {TIMED_EVENT, "loopevent", "10"};
    deviceTimedCollect->nonPersitenceLoopEventSet_.clear();
    int32_t ret = deviceTimedCollect->RemoveUnusedEvent(event);
    EXPECT_EQ(ret, ERR_OK);
    DTEST_LOG << "RemoveUnusedEvent002 end" << std::endl;
}

/**
 * @tc.name: RemoveUnusedEvent003
 * @tc.desc: test RemoveUnusedEvent, with event.name in nonPersitenceLoopEventSet_
 * @tc.type: FUNC
 * @tc.require: I7VZ98
 */
HWTEST_F(DeviceTimedCollectTest, RemoveUnusedEvent003, TestSize.Level3)
{
    DTEST_LOG << "RemoveUnusedEvent003 begin" << std::endl;
    sptr<IReport> report;
    std::shared_ptr<DeviceTimedCollect> deviceTimedCollect =
        std::make_shared<DeviceTimedCollect>(report);
    OnDemandEvent event = {TIMED_EVENT, "loopevent", "10"};
    deviceTimedCollect->nonPersitenceLoopEventSet_.insert(10);
    int32_t ret = deviceTimedCollect->RemoveUnusedEvent(event);
    EXPECT_EQ(ret, ERR_OK);
    DTEST_LOG << "RemoveUnusedEvent003 end" << std::endl;
}

/**
 * @tc.name: RemoveUnusedEvent004
 * @tc.desc: test RemoveUnusedEvent, with event.name in nonPersitenceLoopEventSet_
 * @tc.type: FUNC
 * @tc.require: I7VZ98
 */
HWTEST_F(DeviceTimedCollectTest, RemoveUnusedEvent004, TestSize.Level3)
{
    DTEST_LOG << "RemoveUnusedEvent004 begin" << std::endl;
    sptr<IReport> report;
    std::shared_ptr<DeviceTimedCollect> deviceTimedCollect =
        std::make_shared<DeviceTimedCollect>(report);
    OnDemandEvent event = {TIMED_EVENT, "loopevent", "10", -1, true};
    deviceTimedCollect->nonPersitenceLoopEventSet_.insert(10);
    int32_t ret = deviceTimedCollect->RemoveUnusedEvent(event);
    EXPECT_EQ(ret, ERR_OK);
    DTEST_LOG << "RemoveUnusedEvent004 end" << std::endl;
}

/**
 * @tc.name: RemoveUnusedEvent005
 * @tc.desc: test RemoveUnusedEvent, with event.name in nonPersitenceLoopEventSet_
 * @tc.type: FUNC
 * @tc.require: I7VZ98
 */
HWTEST_F(DeviceTimedCollectTest, RemoveUnusedEvent005, TestSize.Level3)
{
    DTEST_LOG << "RemoveUnusedEvent005 begin" << std::endl;
    sptr<IReport> report;
    std::shared_ptr<DeviceTimedCollect> deviceTimedCollect =
        std::make_shared<DeviceTimedCollect>(report);
    OnDemandEvent event = {TIMED_EVENT, "awakeloopevent", "3600", -1, true};
    deviceTimedCollect->nonPersitenceLoopEventSet_.insert(3600);
    int32_t ret = deviceTimedCollect->RemoveUnusedEvent(event);
    EXPECT_EQ(ret, ERR_OK);
    OnDemandEvent event2 = {TIMED_EVENT, "loopevent", "3600", -1, true};
    deviceTimedCollect->nonPersitenceLoopEventSet_.insert(3600);
    ret = deviceTimedCollect->RemoveUnusedEvent(event2);
    EXPECT_EQ(ret, ERR_OK);
    deviceTimedCollect->timeInfos_.clear();
    DTEST_LOG << "RemoveUnusedEvent005 end" << std::endl;
}

/**
 * @tc.name: SaveInner001
 * @tc.desc: test SaveInner.
 * @tc.type: FUNC
 * @tc.require: I7VZ98
 */
HWTEST_F(DeviceTimedCollectTest, SaveInner001, TestSize.Level3)
{
    DTEST_LOG << "SaveInner001 begin" << std::endl;
#ifdef PREFERENCES_ENABLE
    std::shared_ptr<PreferencesUtil> preferencesUtil_ = PreferencesUtil::GetInstance();
    const std::string key;
    int64_t value = 0;
    preferencesUtil_->ptr_ = nullptr;
    bool ret = preferencesUtil_->SaveInner(preferencesUtil_->ptr_, key, value);
    EXPECT_EQ(ret, false);
#endif
    DTEST_LOG << "SaveInner001 end" << std::endl;
}

/**
 * @tc.name: SaveInner002
 * @tc.desc: test SaveInner.
 * @tc.type: FUNC
 * @tc.require: I7VZ98
 */
HWTEST_F(DeviceTimedCollectTest, SaveInner002, TestSize.Level3)
{
    DTEST_LOG << "SaveInner002 begin" << std::endl;
#ifdef PREFERENCES_ENABLE
    std::shared_ptr<PreferencesUtil> preferencesUtil_ = PreferencesUtil::GetInstance();
    const std::string key;
    const std::string value;
    preferencesUtil_->ptr_ = nullptr;
    bool ret = preferencesUtil_->SaveInner(preferencesUtil_->ptr_, key, value);
    EXPECT_EQ(ret, false);
#endif
    DTEST_LOG << "SaveInner002 end" << std::endl;
}

/**
 * @tc.name: ObtainLong001
 * @tc.desc: test ObtainInner.
 * @tc.type: FUNC
 * @tc.require: I7VZ98
 */
HWTEST_F(DeviceTimedCollectTest, ObtainLong001, TestSize.Level3)
{
    DTEST_LOG << "ObtainLong001 begin" << std::endl;
#ifdef PREFERENCES_ENABLE
    std::shared_ptr<PreferencesUtil> preferencesUtil_ = PreferencesUtil::GetInstance();
    const std::string key;
    const int64_t defValue = 0;
    int64_t ret = preferencesUtil_->ObtainLong(key, defValue);
    EXPECT_EQ(ret, 0);
#endif
    DTEST_LOG << "ObtainLong001 end" << std::endl;
}

#ifdef PREFERENCES_ENABLE
HWTEST_F(DeviceTimedCollectTest, ProcessPersistenceTimedTask001, TestSize.Level3)
{
    DTEST_LOG << "ProcessPersistenceTimedTask001 begin" << std::endl;
    sptr<IReport> report;
    std::shared_ptr<DeviceTimedCollect> deviceTimedCollect = std::make_shared<DeviceTimedCollect>(report);
    deviceTimedCollect->preferencesUtil_ = PreferencesUtil::GetInstance();
    int64_t disTime = 0;
    std::string strTime = "IntervalTime";
    deviceTimedCollect->ProcessPersistenceTimedTask(disTime, strTime);
    EXPECT_EQ(disTime, 0);

    disTime = 5;
    deviceTimedCollect->ProcessPersistenceTimedTask(disTime, strTime);
    EXPECT_NE(disTime, 0);
    DTEST_LOG << "ProcessPersistenceTimedTask001 end" << std::endl;
}
#endif

#ifdef PREFERENCES_ENABLE
HWTEST_F(DeviceTimedCollectTest, PostPersistenceDelayTask001, TestSize.Level3)
{
    DTEST_LOG << "PostPersistenceDelayTask001 begin" << std::endl;
    sptr<IReport> report;
    std::shared_ptr<DeviceTimedCollect> deviceTimedCollect = std::make_shared<DeviceTimedCollect>(report);
    deviceTimedCollect->preferencesUtil_ = PreferencesUtil::GetInstance();
    int32_t interval = 1;
    int32_t disTime = 1;
    
    std::function<void()> postTask = [] () {};
    deviceTimedCollect->PostPersistenceDelayTask(postTask, interval, disTime);
    EXPECT_EQ(disTime, 1);
    DTEST_LOG << "PostPersistenceDelayTask001 end" << std::endl;
}
#endif

HWTEST_F(DeviceTimedCollectTest, PostNonPersistenceTimedTaskLocked001, TestSize.Level3)
{
    DTEST_LOG << "PostNonPersistenceTimedTaskLocked001 begin" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    std::shared_ptr<DeviceTimedCollect> deviceTimedCollect = std::make_shared<DeviceTimedCollect>(collect);
    std::string strTime = "timedevent";
    int64_t timeGap = 0;
    deviceTimedCollect->PostNonPersistenceTimedTaskLocked(strTime, timeGap);
    EXPECT_EQ(timeGap, 0);
    DTEST_LOG << "PostNonPersistenceTimedTaskLocked001 end" << std::endl;
}

#ifdef PREFERENCES_ENABLE
HWTEST_F(DeviceTimedCollectTest, ProcessPersistenceTimedTask003, TestSize.Level3)
{
    DTEST_LOG << " ProcessPersistenceTimedTask003 begin" << std::endl;
    sptr<IReport> report;
    std::shared_ptr<DeviceTimedCollect> deviceTimedCollect = std::make_shared<DeviceTimedCollect>(report);
    deviceTimedCollect->preferencesUtil_ = PreferencesUtil::GetInstance();
    int64_t disTime = 0;
    std::string strTime = "IntervalTime";
    deviceTimedCollect->ProcessPersistenceTimedTask(disTime, strTime);
    EXPECT_EQ(disTime, 0);

    disTime = 5;
    deviceTimedCollect->ProcessPersistenceTimedTask(disTime, strTime);
    EXPECT_NE(disTime, 0);
    DTEST_LOG << " ProcessPersistenceTimedTask003 end" << std::endl;
}
#endif

#ifdef PREFERENCES_ENABLE
HWTEST_F(DeviceTimedCollectTest, PostPersistenceDelayTask002, TestSize.Level3)
{
    DTEST_LOG << " PostPersistenceDelayTask002 begin" << std::endl;
    sptr<IReport> report;
    std::shared_ptr<DeviceTimedCollect> deviceTimedCollect = std::make_shared<DeviceTimedCollect>(report);
    deviceTimedCollect->preferencesUtil_ = PreferencesUtil::GetInstance();
    int32_t interVal = 1;
    int32_t disTime = 1;
    
    std::function<void()> postTask = [] () {};
    deviceTimedCollect->PostPersistenceDelayTask(postTask, interVal, disTime);
    EXPECT_EQ(disTime, 1);
    DTEST_LOG << " PostPersistenceDelayTask002 end" << std::endl;
}
#endif

HWTEST_F(DeviceTimedCollectTest, PostNonPersistenceTimedTaskLocked002, TestSize.Level3)
{
    DTEST_LOG << " PostNonPersistenceTimedTaskLocked002 begin" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    std::shared_ptr<DeviceTimedCollect> deviceTimedCollect = std::make_shared<DeviceTimedCollect>(collect);
    std::string strTime = "timedevent";
    int64_t timeGap = 0;
    deviceTimedCollect->PostNonPersistenceTimedTaskLocked(strTime, timeGap);
    EXPECT_EQ(timeGap, 0);
    DTEST_LOG << " PostNonPersistenceTimedTaskLocked002 end" << std::endl;
}

#ifdef PREFERENCES_ENABLE
HWTEST_F(DeviceTimedCollectTest, ProcessPersistenceTimedTask002, TestSize.Level3)
{
    DTEST_LOG << " ProcessPersistenceTimedTask002 begin" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    std::shared_ptr<DeviceTimedCollect> deviceTimedCollect = std::make_shared<DeviceTimedCollect>(collect);
    deviceTimedCollect->preferencesUtil_ = PreferencesUtil::GetInstance();

    int64_t disTime = -1;
    std::string strTime = "time_2017_9-1-16:59:10";
    deviceTimedCollect->ProcessPersistenceTimedTask(disTime, strTime);
    EXPECT_NE(nullptr, deviceTimedCollect->preferencesUtil_);
    disTime = 100;
    deviceTimedCollect->ProcessPersistenceTimedTask(disTime, strTime);
    EXPECT_NE(nullptr, deviceTimedCollect->preferencesUtil_);
    DTEST_LOG << " ProcessPersistenceTimedTask002 end" << std::endl;
}
#endif

#ifdef PREFERENCES_ENABLE
HWTEST_F(DeviceTimedCollectTest, ProcessPersistenceLoopTask001, TestSize.Level3)
{
    DTEST_LOG << " ProcessPersistenceLoopTask001 begin" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    std::shared_ptr<DeviceTimedCollect> deviceTimedCollect = std::make_shared<DeviceTimedCollect>(collect);
    deviceTimedCollect->preferencesUtil_ = PreferencesUtil::GetInstance();

    int64_t disTime = 0;
    int triggerTime = 1729909174;
    std::string strInterval = "1729909174";
    deviceTimedCollect->ProcessPersistenceLoopTask(disTime, triggerTime, strInterval);
    EXPECT_EQ(0, disTime);
    disTime = 100;
    deviceTimedCollect->ProcessPersistenceLoopTask(disTime, triggerTime, strInterval);
    EXPECT_NE(0, disTime);
    DTEST_LOG << " ProcessPersistenceLoopTask001 end" << std::endl;
}
#endif

HWTEST_F(DeviceTimedCollectTest, PostNonPersistenceLoopTaskLocked001, TestSize.Level3)
{
    DTEST_LOG << " PostNonPersistenceLoopTaskLocked001 begin" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    std::shared_ptr<DeviceTimedCollect> deviceTimedCollect = std::make_shared<DeviceTimedCollect>(collect);
    int32_t interVal = 65;
    deviceTimedCollect->nonPersitenceLoopTasks_[interVal] = [interVal] () {};
    deviceTimedCollect->PostNonPersistenceLoopTaskLocked(interVal);
    EXPECT_FALSE(deviceTimedCollect->nonPersitenceLoopTasks_.empty());
    
    deviceTimedCollect->nonPersitenceLoopTasks_.clear();
    deviceTimedCollect->nonPersitenceLoopEventSet_.insert(interVal);
    deviceTimedCollect->PostNonPersistenceLoopTaskLocked(interVal);
    EXPECT_FALSE(deviceTimedCollect->nonPersitenceLoopTasks_.empty());

    deviceTimedCollect->RemoveNonPersistenceLoopTask(interVal);
    EXPECT_TRUE(deviceTimedCollect->nonPersitenceLoopTasks_.empty());
    DTEST_LOG << " PostNonPersistenceLoopTaskLocked001 end" << std::endl;
}

#ifdef PREFERENCES_ENABLE
HWTEST_F(DeviceTimedCollectTest, PostPersistenceDelayTaskd001, TestSize.Level3)
{
    DTEST_LOG << " PostPersistenceDelayTaskd001 begin" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    std::shared_ptr<DeviceTimedCollect> deviceTimedCollect = std::make_shared<DeviceTimedCollect>(collect);
    deviceTimedCollect->preferencesUtil_ = PreferencesUtil::GetInstance();

    std::function<void()> postTask = [] () {};
    int32_t interVal = 1;
    int32_t disTime = 1;
    deviceTimedCollect->PostPersistenceDelayTask(postTask, interVal, disTime);
    EXPECT_NE(nullptr, deviceTimedCollect->preferencesUtil_);
    DTEST_LOG << " PostPersistenceDelayTaskd001 end" << std::endl;
}
#endif

HWTEST_F(DeviceTimedCollectTest, PostNonPersistenceTimedTaskLocked003, TestSize.Level3)
{
    DTEST_LOG << " PostNonPersistenceTimedTaskLocked003 begin" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    std::shared_ptr<DeviceTimedCollect> deviceTimedCollect = std::make_shared<DeviceTimedCollect>(collect);
    std::string strTime = "10";
    int64_t timeGap = 0;
    deviceTimedCollect->PostNonPersistenceTimedTaskLocked(strTime, timeGap);
    EXPECT_EQ(0, timeGap);
    timeGap = 5;
    deviceTimedCollect->PostNonPersistenceTimedTaskLocked(strTime, timeGap);
    EXPECT_NE(0, timeGap);
    DTEST_LOG << " PostNonPersistenceTimedTaskLocked003 end" << std::endl;
}

HWTEST_F(DeviceTimedCollectTest, RemovePersistenceLoopTask001, TestSize.Level3)
{
    DTEST_LOG << " RemovePersistenceLoopTask001 begin" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    std::shared_ptr<DeviceTimedCollect> deviceTimedCollect = std::make_shared<DeviceTimedCollect>(collect);
    
    int32_t interVal = 64;
    deviceTimedCollect->persitenceLoopEventSet_.insert(interVal);
    deviceTimedCollect->persitenceLoopTasks_[interVal] = [interVal] () {};
    deviceTimedCollect->RemovePersistenceLoopTask(interVal);
    EXPECT_TRUE(deviceTimedCollect->persitenceLoopEventSet_.empty());
    DTEST_LOG << " RemovePersistenceLoopTask001 end" << std::endl;
}
}
