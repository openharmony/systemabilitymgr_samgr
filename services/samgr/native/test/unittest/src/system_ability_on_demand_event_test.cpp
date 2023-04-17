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
#include "system_ability_on_demand_event_test.h"

#include "device_status_collect_manager.h"
#include "icollect_plugin.h"
#include "sa_profiles.h"
#include "sa_status_change_mock.h"
#include "sam_log.h"
#include "system_ability_definition.h"
#include "system_ability_manager.h"
#include "system_ability_on_demand_event.h"
#include "test_log.h"

using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS;

namespace OHOS {
namespace {
    const std::string TESTNAME = "testname";
    const std::string VALUE = "testvalue";
}
void SystemAbilityOnDemandEventTest::SetUpTestCase()
{
    DTEST_LOG << "SetUpTestCase" << std::endl;
}

void SystemAbilityOnDemandEventTest::TearDownTestCase()
{
    DTEST_LOG << "TearDownTestCase" << std::endl;
}

void SystemAbilityOnDemandEventTest::SetUp()
{
    DTEST_LOG << "SetUp" << std::endl;
}

void SystemAbilityOnDemandEventTest::TearDown()
{
    DTEST_LOG << "TearDown" << std::endl;
}

/**
 * @tc.name: WriteOnDemandEventsToParcel001
 * @tc.desc: Test WriteOnDemandEventsToParcel, with abilityOnDemandEvents and data
 * @tc.type: FUNC
 * @tc.require: I6V4AX
 */
HWTEST_F(SystemAbilityOnDemandEventTest, WriteOnDemandEventsToParcel001, TestSize.Level3)
{
    DTEST_LOG << "WriteOnDemandEventsToParcel001" << std::endl;
    std::vector<SystemAbilityOnDemandEvent> abilityOnDemandEvents;
    MessageParcel data;
    bool ret = OnDemandEventToParcel::WriteOnDemandEventsToParcel(abilityOnDemandEvents, data);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: WriteOnDemandEventToParcel001
 * @tc.desc: Test WriteOnDemandEventToParcel, with event and data
 * @tc.type: FUNC
 * @tc.require: I6V4AX
 */
HWTEST_F(SystemAbilityOnDemandEventTest, WriteOnDemandEventToParcel001, TestSize.Level3)
{
    DTEST_LOG << "WriteOnDemandEventToParcel001" << std::endl;
    SystemAbilityOnDemandEvent event;
    MessageParcel data;
    bool ret = OnDemandEventToParcel::WriteOnDemandEventToParcel(event, data);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: WriteOnDemandConditionToParcel001
 * @tc.desc: Test WriteOnDemandConditionToParcel, with condition and data
 * @tc.type: FUNC
 * @tc.require: I6V4AX
 */
HWTEST_F(SystemAbilityOnDemandEventTest, WriteOnDemandConditionToParcel001, TestSize.Level3)
{
    DTEST_LOG << "WriteOnDemandConditionToParcel001" << std::endl;
    SystemAbilityOnDemandCondition condition;
    MessageParcel data;
    bool ret = OnDemandEventToParcel::WriteOnDemandConditionToParcel(condition, data);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: ReadOnDemandEventsFromParcel001
 * @tc.desc: Test ReadOnDemandEventsFromParcel, with abilityOnDemandEvents and reply
 * @tc.type: FUNC
 * @tc.require: I6V4AX
 */
HWTEST_F(SystemAbilityOnDemandEventTest, ReadOnDemandEventsFromParcel001, TestSize.Level3)
{
    DTEST_LOG << "ReadOnDemandEventsFromParcel001" << std::endl;
    std::vector<SystemAbilityOnDemandEvent> abilityOnDemandEvents;
    MessageParcel reply;
    bool ret = OnDemandEventToParcel::ReadOnDemandEventsFromParcel(abilityOnDemandEvents, reply);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: ReadOnDemandEventFromParcel001
 * @tc.desc: Test ReadOnDemandEventFromParcel, with eventId is DEVICE_ONLINE, name is TESTNAME,value is VALUE
 * @tc.type: FUNC
 * @tc.require: I6V4AX
 */
HWTEST_F(SystemAbilityOnDemandEventTest, ReadOnDemandEventFromParcel001, TestSize.Level3)
{
    DTEST_LOG << "ReadOnDemandEventFromParcel001" << std::endl;
    SystemAbilityOnDemandEvent event;
    event.eventId = OnDemandEventId::DEVICE_ONLINE;
    event.name = TESTNAME;
    event.value = VALUE;
    MessageParcel reply;
    bool ret = OnDemandEventToParcel::ReadOnDemandEventFromParcel(event, reply);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: ReadOnDemandConditionFromParcel001
 * @tc.desc: Test ReadOnDemandConditionFromParcel, with condition and reply
 * @tc.type: FUNC
 * @tc.require: I6V4AX
 */
HWTEST_F(SystemAbilityOnDemandEventTest, ReadOnDemandConditionFromParcel001, TestSize.Level3)
{
    DTEST_LOG << "ReadOnDemandConditionFromParcel001" << std::endl;
    SystemAbilityOnDemandCondition condition;
    condition.name = TESTNAME;
    MessageParcel reply;
    bool ret = OnDemandEventToParcel::ReadOnDemandConditionFromParcel(condition, reply);
    EXPECT_EQ(ret, false);
}
} // namespace OHOS