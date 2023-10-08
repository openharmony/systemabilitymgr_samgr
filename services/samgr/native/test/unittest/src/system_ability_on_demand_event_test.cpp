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
    constexpr bool TEST_PERSISTENCE = false;
    constexpr bool TEST_ENABLEONCE = false;
    constexpr int32_t BELOW_SIZE = -1;
    constexpr int32_t OVER_SIZE = 10000;
    constexpr int32_t TEST_SIZE = 1;
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
 * @tc.name: WriteOnDemandEventsToParcel002
 * @tc.desc: Test WriteOnDemandEventsToParcel, with ondemandeventId is online
 * @tc.type: FUNC
 * @tc.require: I76X9Q
 */
HWTEST_F(SystemAbilityOnDemandEventTest, WriteOnDemandEventsToParcel002, TestSize.Level3)
{
    DTEST_LOG << "WriteOnDemandEventsToParcel002" << std::endl;
    SystemAbilityOnDemandCondition condition = {OnDemandEventId::DEVICE_ONLINE, TESTNAME, VALUE};
    std::vector<SystemAbilityOnDemandCondition> conditions {condition};
    SystemAbilityOnDemandEvent abilityOnDemandEvent = {OnDemandEventId::DEVICE_ONLINE,
        TESTNAME, VALUE, false, conditions, false};
    std::vector<SystemAbilityOnDemandEvent> abilityOnDemandEvents {abilityOnDemandEvent};
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
 * @tc.name: WriteOnDemandEventToParcel002
 * @tc.desc: Test WriteOnDemandEventToParcel, with ondemandeventId is online
 * @tc.type: FUNC
 * @tc.require: I76X9Q
 */
HWTEST_F(SystemAbilityOnDemandEventTest, WriteOnDemandEventToParcel002, TestSize.Level3)
{
    DTEST_LOG << "WriteOnDemandEventToParcel002" << std::endl;
    SystemAbilityOnDemandCondition condition = {OnDemandEventId::DEVICE_ONLINE, TESTNAME, VALUE};
    std::vector<SystemAbilityOnDemandCondition> conditions {condition};
    SystemAbilityOnDemandEvent event = {OnDemandEventId::DEVICE_ONLINE, TESTNAME, VALUE, false,
        conditions, false};
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
 * @tc.name: WriteOnDemandConditionToParcel002
 * @tc.desc: Test WriteOnDemandConditionToParcel, with invalid ondemandeventId
 * @tc.type: FUNC
 * @tc.require: I76X9Q
 */
HWTEST_F(SystemAbilityOnDemandEventTest, WriteOnDemandConditionToParcel002, TestSize.Level3)
{
    DTEST_LOG << "WriteOnDemandConditionToParcel002" << std::endl;
    SystemAbilityOnDemandCondition condition;
    condition.eventId = (OnDemandEventId)6;
    MessageParcel data;
    bool ret = OnDemandEventToParcel::WriteOnDemandConditionToParcel(condition, data);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: WriteOnDemandConditionToParcel003
 * @tc.desc: Test WriteOnDemandConditionToParcel, with condition.name is null
 * @tc.type: FUNC
 * @tc.require: I76X9Q
 */
HWTEST_F(SystemAbilityOnDemandEventTest, WriteOnDemandConditionToParcel003, TestSize.Level3)
{
    DTEST_LOG << "WriteOnDemandConditionToParcel003" << std::endl;
    SystemAbilityOnDemandCondition condition;
    condition.name = "";
    MessageParcel data;
    bool ret = OnDemandEventToParcel::WriteOnDemandConditionToParcel(condition, data);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: WriteOnDemandConditionToParcel004
 * @tc.desc: Test WriteOnDemandConditionToParcel, with condition.value is null
 * @tc.type: FUNC
 * @tc.require: I76X9Q
 */
HWTEST_F(SystemAbilityOnDemandEventTest, WriteOnDemandConditionToParcel004, TestSize.Level3)
{
    DTEST_LOG << "WriteOnDemandConditionToParcel004" << std::endl;
    SystemAbilityOnDemandCondition condition;
    condition.value = "";
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

/**
 * @tc.name: WriteOnDemandEventsToParcel003
 * @tc.desc: Test WriteOnDemandEventsToParcel with event has condition
 * @tc.type: FUNC
 * @tc.require: I76Z5F
 */
HWTEST_F(SystemAbilityOnDemandEventTest, WriteOnDemandEventsToParcel003, TestSize.Level3)
{
    std::vector<SystemAbilityOnDemandEvent> abilityOnDemandEvents;
    SystemAbilityOnDemandEvent event;
    SystemAbilityOnDemandCondition condition;
    event.conditions.emplace_back(condition);
    abilityOnDemandEvents.emplace_back(event);
    MessageParcel data;
    bool ret = OnDemandEventToParcel::WriteOnDemandEventsToParcel(abilityOnDemandEvents, data);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: ReadOnDemandEventsFromParcel002
 * @tc.desc: Test ReadOnDemandEventsFromParcel with size is oversized
 * @tc.type: FUNC
 * @tc.require: I76Z5F
 */
HWTEST_F(SystemAbilityOnDemandEventTest, ReadOnDemandEventsFromParcel002, TestSize.Level3)
{
    std::vector<SystemAbilityOnDemandEvent> abilityOnDemandEvents;
    SystemAbilityOnDemandEvent event;
    SystemAbilityOnDemandCondition condition;
    event.conditions.emplace_back(condition);
    abilityOnDemandEvents.emplace_back(event);
    MessageParcel reply;
    reply.WriteInt32(OVER_SIZE);
    bool ret = OnDemandEventToParcel::ReadOnDemandEventsFromParcel(abilityOnDemandEvents, reply);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: ReadOnDemandEventsFromParcel003
 * @tc.desc: Test ReadOnDemandEventsFromParcel with size is less than 0
 * @tc.type: FUNC
 * @tc.require: I76Z5F
 */
HWTEST_F(SystemAbilityOnDemandEventTest, ReadOnDemandEventsFromParcel003, TestSize.Level3)
{
    std::vector<SystemAbilityOnDemandEvent> abilityOnDemandEvents;
    SystemAbilityOnDemandEvent event;
    SystemAbilityOnDemandCondition condition;
    event.conditions.emplace_back(condition);
    abilityOnDemandEvents.emplace_back(event);
    MessageParcel reply;
    reply.WriteInt32(BELOW_SIZE);
    bool ret = OnDemandEventToParcel::ReadOnDemandEventsFromParcel(abilityOnDemandEvents, reply);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: ReadOnDemandEventsFromParcel004
 * @tc.desc: Test ReadOnDemandEventsFromParcel with conditionSize is oversized
 * @tc.type: FUNC
 * @tc.require: I76Z5F
 */
HWTEST_F(SystemAbilityOnDemandEventTest, ReadOnDemandEventsFromParcel004, TestSize.Level3)
{
    std::vector<SystemAbilityOnDemandEvent> abilityOnDemandEvents;
    SystemAbilityOnDemandEvent event;
    event.eventId = OnDemandEventId::DEVICE_ONLINE;
    event.name = TESTNAME;
    event.value = VALUE;
    SystemAbilityOnDemandCondition condition;
    condition.eventId = OnDemandEventId::DEVICE_ONLINE;
    condition.name = TESTNAME;
    condition.value = VALUE;
    MessageParcel reply;
    reply.WriteInt32(TEST_SIZE);
    OnDemandEventToParcel::WriteOnDemandEventToParcel(event, reply);
    reply.WriteInt32(OVER_SIZE);
    OnDemandEventToParcel::WriteOnDemandConditionToParcel(condition, reply);
    bool ret = OnDemandEventToParcel::ReadOnDemandEventsFromParcel(abilityOnDemandEvents, reply);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: ReadOnDemandEventsFromParcel005
 * @tc.desc: Test ReadOnDemandEventsFromParcel with less than 0
 * @tc.type: FUNC
 * @tc.require: I76Z5F
 */
HWTEST_F(SystemAbilityOnDemandEventTest, ReadOnDemandEventsFromParcel005, TestSize.Level3)
{
    std::vector<SystemAbilityOnDemandEvent> abilityOnDemandEvents;
    SystemAbilityOnDemandEvent event;
    event.eventId = OnDemandEventId::DEVICE_ONLINE;
    event.name = TESTNAME;
    event.value = VALUE;
    SystemAbilityOnDemandCondition condition;
    condition.eventId = OnDemandEventId::DEVICE_ONLINE;
    condition.name = TESTNAME;
    condition.value = VALUE;
    MessageParcel reply;
    reply.WriteInt32(TEST_SIZE);
    OnDemandEventToParcel::WriteOnDemandEventToParcel(event, reply);
    reply.WriteInt32(BELOW_SIZE);
    OnDemandEventToParcel::WriteOnDemandConditionToParcel(condition, reply);
    bool ret = OnDemandEventToParcel::ReadOnDemandEventsFromParcel(abilityOnDemandEvents, reply);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: ReadOnDemandEventsFromParcel006
 * @tc.desc: Test ReadOnDemandEventsFromParcel, read onDemandEvent successfully
 * @tc.type: FUNC
 * @tc.require: I76Z5F
 */
HWTEST_F(SystemAbilityOnDemandEventTest, ReadOnDemandEventsFromParcel006, TestSize.Level3)
{
    std::vector<SystemAbilityOnDemandEvent> abilityOnDemandEvents;
    SystemAbilityOnDemandEvent event;
    event.eventId = OnDemandEventId::DEVICE_ONLINE;
    event.name = TESTNAME;
    event.value = VALUE;
    SystemAbilityOnDemandCondition condition;
    condition.eventId = OnDemandEventId::DEVICE_ONLINE;
    condition.name = TESTNAME;
    condition.value = VALUE;
    MessageParcel reply;
    reply.WriteInt32(TEST_SIZE);
    OnDemandEventToParcel::WriteOnDemandEventToParcel(event, reply);
    reply.WriteInt32(TEST_PERSISTENCE);
    reply.WriteInt32(TEST_SIZE);
    OnDemandEventToParcel::WriteOnDemandConditionToParcel(condition, reply);
    reply.WriteBool(TEST_ENABLEONCE);
    bool ret = OnDemandEventToParcel::ReadOnDemandEventsFromParcel(abilityOnDemandEvents, reply);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: ReadOnDemandEventFromParcel002
 * @tc.desc: Test ReadOnDemandEventsFromParcel with eventID, name and value
 * @tc.type: FUNC
 * @tc.require: I76Z5F
 */
HWTEST_F(SystemAbilityOnDemandEventTest, ReadOnDemandEventFromParcel002, TestSize.Level3)
{
    SystemAbilityOnDemandEvent event;
    event.eventId = OnDemandEventId::DEVICE_ONLINE;
    event.name = TESTNAME;
    event.value = VALUE;
    MessageParcel reply;
    reply.WriteInt32(static_cast<int32_t>(OnDemandEventId::DEVICE_ONLINE));
    reply.WriteString(TESTNAME);
    reply.WriteString(VALUE);
    bool ret = OnDemandEventToParcel::ReadOnDemandEventFromParcel(event, reply);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: ReadOnDemandConditionFromParcel002
 * @tc.desc: Test ReadOnDemandConditionFromParcel with eventID, name and value
 * @tc.type: FUNC
 * @tc.require: I76Z5F
 */
HWTEST_F(SystemAbilityOnDemandEventTest, ReadOnDemandConditionFromParcel002, TestSize.Level3)
{
    SystemAbilityOnDemandCondition condition;
    condition.eventId = OnDemandEventId::DEVICE_ONLINE;
    condition.name = TESTNAME;
    condition.value = VALUE;
    MessageParcel reply;
    reply.WriteInt32(static_cast<int32_t>(OnDemandEventId::DEVICE_ONLINE));
    reply.WriteString(TESTNAME);
    reply.WriteString(VALUE);
    bool ret = OnDemandEventToParcel::ReadOnDemandConditionFromParcel(condition, reply);
    EXPECT_TRUE(ret);
}
} // namespace OHOS