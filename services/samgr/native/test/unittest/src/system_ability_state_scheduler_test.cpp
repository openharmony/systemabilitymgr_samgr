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

#include "system_ability_state_scheduler_test.h"

#include "ability_death_recipient.h"
#include "sa_status_change_mock.h"
#include "test_log.h"

#define private public
#include "schedule/system_ability_state_scheduler.h"

using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS;

namespace OHOS {
namespace {

constexpr int32_t SAID_INVALID = -1;
constexpr int32_t SAID = 1234;
constexpr int32_t STATENUMS = 1;
const std::u16string process = u"test";
const std::u16string process_invalid = u"test_invalid";
const std::string LOCAL_DEVICE = "local";
}
void SystemAbilityStateSchedulerTest::SetUpTestCase()
{
    DTEST_LOG << "SetUpTestCase" << std::endl;
}

void SystemAbilityStateSchedulerTest::TearDownTestCase()
{
    DTEST_LOG << "TearDownTestCase" << std::endl;
}

void SystemAbilityStateSchedulerTest::SetUp()
{
    DTEST_LOG << "SetUp" << std::endl;
}

void SystemAbilityStateSchedulerTest::TearDown()
{
    DTEST_LOG << "TearDown" << std::endl;
}


/**
 * @tc.name: InitSteteContext001
 * @tc.desc: call InitSteteContext with SaProfiles's process is empty
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, InitSteteContext001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    SaProfile saProfile;
    std::list<SaProfile> saProfiles;
    saProfiles.push_back(saProfile);
    systemAbilityStateScheduler->InitStateContext(saProfiles);
    EXPECT_TRUE(systemAbilityStateScheduler->processContextMap_.empty());
}

/**
 * @tc.name: InitSteteContext002
 * @tc.desc: call InitSteteContext with SaProfiles
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, InitSteteContext002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    SaProfile saProfile;
    saProfile.process = process;
    std::list<SaProfile> saProfiles;
    saProfiles.push_back(saProfile);
    systemAbilityStateScheduler->processContextMap_.clear();
    systemAbilityStateScheduler->InitStateContext(saProfiles);
    EXPECT_FALSE(systemAbilityStateScheduler->processContextMap_.empty());
}

/**
 * @tc.name: GetSystemAbilityContext001
 * @tc.desc: test GetSystemAbilityContext with empty abilityContextMap_
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, GetSystemAbilityContext001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    systemAbilityStateScheduler->abilityContextMap_.clear();
    bool ret = systemAbilityStateScheduler->GetSystemAbilityContext(SAID, systemAbilityContext);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: GetSystemAbilityContext002
 * @tc.desc: test GetSystemAbilityContext with abilityContext is nullptr
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, GetSystemAbilityContext002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    systemAbilityStateScheduler->abilityContextMap_.clear();
    systemAbilityStateScheduler->abilityContextMap_[SAID] = systemAbilityContext;
    bool ret = systemAbilityStateScheduler->GetSystemAbilityContext(SAID_INVALID, systemAbilityContext);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: GetSystemAbilityContext003
 * @tc.desc: test GetSystemAbilityContext with abilityContext's ownProcessContext is nullptr
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, GetSystemAbilityContext003, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    systemAbilityStateScheduler->abilityContextMap_.clear();
    systemAbilityStateScheduler->abilityContextMap_[SAID] = systemAbilityContext;
    bool ret = systemAbilityStateScheduler->GetSystemAbilityContext(SAID, systemAbilityContext);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: GetSystemAbilityContext004
 * @tc.desc: test GetSystemAbilityContext,report success
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, GetSystemAbilityContext004, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemAbilityStateScheduler->abilityContextMap_.clear();
    systemAbilityContext->ownProcessContext = systemProcessContext;
    systemAbilityStateScheduler->abilityContextMap_[SAID] = systemAbilityContext;
    bool ret = systemAbilityStateScheduler->GetSystemAbilityContext(SAID, systemAbilityContext);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: GetSystemProcessContext001
 * @tc.desc: test GetSystemProcessContext with empty processContextMap_
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, GetSystemProcessContext001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemAbilityStateScheduler->processContextMap_.clear();
    bool ret = systemAbilityStateScheduler->GetSystemProcessContext(process, systemProcessContext);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: GetSystemProcessContext002
 * @tc.desc: test GetSystemProcessContext with processContext is nullptr
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, GetSystemProcessContext002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemAbilityStateScheduler->processContextMap_.clear();
    systemAbilityStateScheduler->processContextMap_[process] = systemProcessContext;
    bool ret = systemAbilityStateScheduler->GetSystemProcessContext(process_invalid, systemProcessContext);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: GetSystemProcessContext002
 * @tc.desc: test GetSystemProcessContext,report success
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, GetSystemProcessContext003, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemAbilityStateScheduler->processContextMap_.clear();
    systemAbilityStateScheduler->processContextMap_[process] = systemProcessContext;
    bool ret = systemAbilityStateScheduler->GetSystemProcessContext(process, systemProcessContext);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: IsSystemAbilityUnloading001
 * @tc.desc: test IsSystemAbilityUnloading with invalid systemAbilityID
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, IsSystemAbilityUnloading001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    systemAbilityStateScheduler->abilityContextMap_.clear();
    int32_t systemAbilityID = -1;
    bool ret = systemAbilityStateScheduler->IsSystemAbilityUnloading(systemAbilityID);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IsSystemAbilityUnloading002
 * @tc.desc: test IsSystemAbilityUnloading with abilityContext's state is UNLOADING
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, IsSystemAbilityUnloading002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemAbilityStateScheduler->abilityContextMap_.clear();
    systemAbilityContext->state = SystemAbilityState::UNLOADING;
    systemAbilityContext->ownProcessContext = systemProcessContext;
    systemAbilityStateScheduler->abilityContextMap_[SAID] = systemAbilityContext;
    bool ret = systemAbilityStateScheduler->IsSystemAbilityUnloading(SAID);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: IsSystemAbilityUnloading003
 * @tc.desc: test IsSystemAbilityUnloading with ownProcessContext's state is STOPPING
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, IsSystemAbilityUnloading003, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemAbilityStateScheduler->abilityContextMap_.clear();
    systemProcessContext->state = SystemProcessState::STOPPING;
    systemAbilityContext->ownProcessContext = systemProcessContext;
    systemAbilityStateScheduler->abilityContextMap_[SAID] = systemAbilityContext;
    bool ret = systemAbilityStateScheduler->IsSystemAbilityUnloading(SAID);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: IsSystemAbilityUnloading004
 * @tc.desc: test IsSystemAbilityUnloading
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, IsSystemAbilityUnloading004, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemAbilityStateScheduler->abilityContextMap_.clear();
    systemProcessContext->state = SystemProcessState::STOPPING;
    systemAbilityContext->state = SystemAbilityState::UNLOADING;
    systemAbilityContext->ownProcessContext = systemProcessContext;
    systemAbilityStateScheduler->abilityContextMap_[SAID] = systemAbilityContext;
    bool ret = systemAbilityStateScheduler->IsSystemAbilityUnloading(SAID);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: HandleLoadAbilityEvent001
 * @tc.desc: test HandleLoadAbilityEvent with abilityContextMap_ is empty
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, HandleLoadAbilityEvent001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    systemAbilityStateScheduler->abilityContextMap_.clear();
    bool isExist = false;
    int32_t ret = systemAbilityStateScheduler->HandleLoadAbilityEvent(SAID_INVALID, isExist);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: HandleLoadAbilityEvent002
 * @tc.desc: test HandleLoadAbilityEvent with systemAbilityContext's state is UNLOADING
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, HandleLoadAbilityEvent002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemAbilityStateScheduler->abilityContextMap_.clear();
    systemAbilityContext->state = SystemAbilityState::UNLOADING;
    systemAbilityContext->ownProcessContext = systemProcessContext;
    systemAbilityStateScheduler->abilityContextMap_[SAID] = systemAbilityContext;
    bool isExist = false;
    int32_t ret = systemAbilityStateScheduler->HandleLoadAbilityEvent(SAID, isExist);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: HandleLoadAbilityEvent003
 * @tc.desc: test HandleLoadAbilityEvent with  systemProcessContext's state is STOPPING
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, HandleLoadAbilityEvent003, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemAbilityStateScheduler->abilityContextMap_.clear();
    systemProcessContext->state = SystemProcessState::STOPPING;
    systemAbilityContext->ownProcessContext = systemProcessContext;
    systemAbilityStateScheduler->abilityContextMap_[SAID] = systemAbilityContext;
    bool isExist = false;
    int32_t ret = systemAbilityStateScheduler->HandleLoadAbilityEvent(SAID, isExist);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: HandleLoadAbilityEvent004
 * @tc.desc: test HandleLoadAbilityEvent
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, HandleLoadAbilityEvent004, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemAbilityStateScheduler->abilityContextMap_.clear();
    systemProcessContext->state = SystemProcessState::STOPPING;
    systemAbilityContext->state = SystemAbilityState::UNLOADING;
    systemAbilityContext->ownProcessContext = systemProcessContext;
    systemAbilityStateScheduler->abilityContextMap_[SAID] = systemAbilityContext;
    bool isExist = false;
    int32_t ret = systemAbilityStateScheduler->HandleLoadAbilityEvent(SAID, isExist);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: HandleLoadAbilityEvent005
 * @tc.desc: test HandleLoadAbilityEvent
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, HandleLoadAbilityEvent005, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    LoadRequestInfo loadRequestInfo;
    loadRequestInfo.callback = new SystemAbilityLoadCallbackMock();
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    systemAbilityStateScheduler->abilityContextMap_.clear();
    int32_t ret = systemAbilityStateScheduler->HandleLoadAbilityEvent(loadRequestInfo);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: HandleLoadAbilityEvent006
 * @tc.desc: test HandleLoadAbilityEvent
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, HandleLoadAbilityEvent006, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemAbilityStateScheduler->abilityContextMap_.clear();
    systemAbilityContext->ownProcessContext = systemProcessContext;
    systemAbilityStateScheduler->abilityContextMap_[SAID] = systemAbilityContext;
    LoadRequestInfo loadRequestInfo;
    loadRequestInfo.systemAbilityId = SAID;
    loadRequestInfo.callback = new SystemAbilityLoadCallbackMock();
    systemAbilityContext->state = SystemAbilityState::UNLOADABLE;
    int32_t ret = systemAbilityStateScheduler->HandleLoadAbilityEvent(loadRequestInfo);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: HandleUnloadAbilityEvent001
 * @tc.desc: test HandleUnloadAbilityEvent with abilityContextMap_ is empty
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, HandleUnloadAbilityEvent001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    systemAbilityStateScheduler->abilityContextMap_.clear();
    OnDemandEvent onDemandEvent = {INTERFACE_CALL};
    UnloadRequestInfo unloadRequestInfo = {SAID, onDemandEvent};
    int32_t ret = systemAbilityStateScheduler->HandleUnloadAbilityEvent(unloadRequestInfo);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: HandleUnloadAbilityEvent002
 * @tc.desc: test HandleUnloadAbilityEvent
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, HandleUnloadAbilityEvent002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemAbilityStateScheduler->abilityContextMap_.clear();
    systemAbilityContext->ownProcessContext = systemProcessContext;
    systemAbilityStateScheduler->abilityContextMap_[SAID] = systemAbilityContext;
    systemAbilityContext->state = SystemAbilityState::LOADING;
    OnDemandEvent onDemandEvent = {INTERFACE_CALL};
    UnloadRequestInfo unloadRequestInfo = {SAID, onDemandEvent};
    int32_t ret = systemAbilityStateScheduler->HandleUnloadAbilityEvent(unloadRequestInfo);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: HandleUnloadAbilityEvent003
 * @tc.desc: test HandleUnloadAbilityEvent
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, HandleUnloadAbilityEvent003, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemAbilityStateScheduler->abilityContextMap_.clear();
    systemAbilityContext->ownProcessContext = systemProcessContext;
    systemAbilityStateScheduler->abilityContextMap_[SAID] = systemAbilityContext;
    systemAbilityContext->state = SystemAbilityState::LOADED;
    OnDemandEvent onDemandEvent = {INTERFACE_CALL};
    UnloadRequestInfo unloadRequestInfo = {SAID, onDemandEvent};
    int32_t ret = systemAbilityStateScheduler->HandleUnloadAbilityEvent(unloadRequestInfo);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: HandleUnloadAbilityEvent004
 * @tc.desc: test HandleUnloadAbilityEvent
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, HandleUnloadAbilityEvent004, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemAbilityStateScheduler->abilityContextMap_.clear();
    systemAbilityContext->ownProcessContext = systemProcessContext;
    systemAbilityStateScheduler->abilityContextMap_[SAID] = systemAbilityContext;
    OnDemandEvent onDemandEvent = {INTERFACE_CALL};
    UnloadRequestInfo unloadRequestInfo = {SAID, onDemandEvent};
    int32_t ret = systemAbilityStateScheduler->HandleUnloadAbilityEvent(unloadRequestInfo);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: SendAbilityStateEvent001
 * @tc.desc: test SendAbilityStateEvent with abilityContextMap_ is empty
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, SendAbilityStateEvent001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    systemAbilityStateScheduler->abilityContextMap_.clear();
    int32_t ret =
        systemAbilityStateScheduler->SendAbilityStateEvent(SAID, AbilityStateEvent::ABILITY_LOAD_SUCCESS_EVENT);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: SendProcessStateEvent001
 * @tc.desc: test SendProcessStateEvent
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, SendProcessStateEvent001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemAbilityStateScheduler->processContextMap_.clear();
    ProcessInfo processInfo;
    processInfo.processName = process;
    int32_t ret =
        systemAbilityStateScheduler->SendProcessStateEvent(processInfo, ProcessStateEvent ::PROCESS_STARTED_EVENT);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: PendLoadEventLocked001
 * @tc.desc: test PendLoadEventLocked with loadRequestInfo's callback is nullptr
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, PendLoadEventLocked001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    LoadRequestInfo loadRequestInfo;
    int32_t ret = systemAbilityStateScheduler->PendLoadEventLocked(systemAbilityContext, loadRequestInfo);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: RemovePendingUnloadEventLocked001
 * @tc.desc: test RemovePendingUnloadEventLocked with abilityContext's pendingEvent is UNLOAD_ABILITY_EVENT
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, RemovePendingUnloadEventLocked001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    systemAbilityContext->pendingEvent = PendingEvent::UNLOAD_ABILITY_EVENT;
    int32_t ret = systemAbilityStateScheduler->RemovePendingUnloadEventLocked(systemAbilityContext);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: HandlePendingUnloadEventLocked001
 * @tc.desc: test HandlePendingUnloadEventLocked
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, HandlePendingUnloadEventLocked001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    systemAbilityContext->pendingEvent = PendingEvent::LOAD_ABILITY_EVENT;
    systemAbilityContext->state = SystemAbilityState::NOT_LOADED;
    int32_t ret = systemAbilityStateScheduler->HandlePendingUnloadEventLocked(systemAbilityContext);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: TryUnloadAllSystemAbility001
 * @tc.desc: test TryUnloadAllSystemAbility with processContext is nullptr
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, TryUnloadAllSystemAbility001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    int32_t ret = systemAbilityStateScheduler->TryUnloadAllSystemAbility(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: TryUnloadAllSystemAbility002
 * @tc.desc: test TryUnloadAllSystemAbility
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, TryUnloadAllSystemAbility002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemProcessContext->saList.push_back(SAID);
    systemProcessContext->abilityStateCountMap.clear();
    systemProcessContext->abilityStateCountMap[SystemAbilityState::UNLOADABLE] = 1;
    int32_t ret = systemAbilityStateScheduler->TryUnloadAllSystemAbility(systemProcessContext);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: CanUnloadAllSystemAbility001
 * @tc.desc: test CanUnloadAllSystemAbility
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, CanUnloadAllSystemAbility001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemProcessContext->abilityStateCountMap.clear();
    bool ret = systemAbilityStateScheduler->CanUnloadAllSystemAbility(systemProcessContext);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: UnloadAllSystemAbilityLocked001
 * @tc.desc: test UnloadAllSystemAbilityLocked
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, UnloadAllSystemAbilityLocked001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemAbilityStateScheduler->processContextMap_.clear();
    systemProcessContext->saList.push_back(SAID);
    int32_t ret = systemAbilityStateScheduler->UnloadAllSystemAbilityLocked(systemProcessContext);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: UnloadAllSystemAbilityLocked002
 * @tc.desc: test UnloadAllSystemAbilityLocked
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, UnloadAllSystemAbilityLocked002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemAbilityStateScheduler->abilityContextMap_.clear();
    systemAbilityContext->ownProcessContext = systemProcessContext;
    systemAbilityStateScheduler->abilityContextMap_[SAID] = systemAbilityContext;
    systemProcessContext->saList.push_back(SAID);
    systemAbilityContext->state = SystemAbilityState::UNLOADABLE;
    int32_t ret = systemAbilityStateScheduler->UnloadAllSystemAbilityLocked(systemProcessContext);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: TryKillSystemProcess001
 * @tc.desc: test TryKillSystemProcess
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, TryKillSystemProcess001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    int32_t ret = systemAbilityStateScheduler->TryKillSystemProcess(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: TryKillSystemProcess002
 * @tc.desc: test TryKillSystemProcess
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, TryKillSystemProcess002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    int32_t ret = systemAbilityStateScheduler->TryKillSystemProcess(systemProcessContext);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: TryKillSystemProcess003
 * @tc.desc: test TryKillSystemProcess
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, TryKillSystemProcess003, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemProcessContext->abilityStateCountMap.clear();
    systemProcessContext->abilityStateCountMap[SystemAbilityState::NOT_LOADED] = STATENUMS;
    int32_t ret = systemAbilityStateScheduler->TryKillSystemProcess(systemProcessContext);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: CanKillSystemProcess001
 * @tc.desc: test CanKillSystemProcess
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, CanKillSystemProcess001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemProcessContext->abilityStateCountMap.clear();
    int32_t ret = systemAbilityStateScheduler->CanKillSystemProcess(systemProcessContext);
    EXPECT_TRUE(ret);
}


/**
 * @tc.name: CanKillSystemProcess002
 * @tc.desc: test CanKillSystemProcess
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, CanKillSystemProcess002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemProcessContext->abilityStateCountMap.clear();
    systemProcessContext->saList.push_back(SAID);
    bool ret = systemAbilityStateScheduler->CanKillSystemProcess(systemProcessContext);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: OnProcessNotStartedLocked001
 * @tc.desc: test OnProcessNotStartedLocked
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, OnProcessNotStartedLocked001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemAbilityStateScheduler->processContextMap_.clear();
    systemAbilityStateScheduler->OnProcessNotStartedLocked(process);
    EXPECT_TRUE(systemAbilityStateScheduler->processContextMap_.empty());
}

/**
 * @tc.name: OnProcessNotStartedLocked002
 * @tc.desc: test OnProcessNotStartedLocked
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, OnProcessNotStartedLocked002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemProcessContext->saList.push_back(SAID);
    systemAbilityStateScheduler->abilityContextMap_.clear();
    systemAbilityStateScheduler->processContextMap_.clear();
    systemAbilityStateScheduler->processContextMap_[process] = systemProcessContext;
    systemAbilityStateScheduler->OnProcessNotStartedLocked(process);
    EXPECT_TRUE(systemAbilityStateScheduler->abilityContextMap_.empty());
}

/**
 * @tc.name: OnAbilityNotLoadedLocked001
 * @tc.desc: test OnAbilityNotLoadedLocked
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, OnAbilityNotLoadedLocked001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    systemAbilityStateScheduler->abilityContextMap_.clear();
    systemAbilityStateScheduler->OnAbilityNotLoadedLocked(SAID);
    EXPECT_TRUE(systemAbilityStateScheduler->processContextMap_.empty());
}

/**
 * @tc.name: OnAbilityNotLoadedLocked002
 * @tc.desc: test OnAbilityNotLoadedLocked
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, OnAbilityNotLoadedLocked002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemAbilityStateScheduler->abilityContextMap_.clear();
    systemAbilityStateScheduler->abilityContextMap_[SAID] = systemAbilityContext;
    systemAbilityContext->ownProcessContext = systemProcessContext;
    systemProcessContext->state = SystemProcessState::STOPPING;
    systemAbilityStateScheduler->OnAbilityNotLoadedLocked(SAID);
    EXPECT_TRUE(systemAbilityStateScheduler->processContextMap_.empty());
}

/**
 * @tc.name: OnAbilityLoadedLocked001
 * @tc.desc: test OnAbilityLoadedLocked
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, OnAbilityLoadedLocked001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    systemAbilityStateScheduler->abilityContextMap_.clear();
    systemAbilityStateScheduler->abilityContextMap_[SAID] = systemAbilityContext;
    systemAbilityStateScheduler->OnAbilityLoadedLocked(SAID_INVALID);
    EXPECT_EQ(systemAbilityStateScheduler->abilityContextMap_.count(SAID_INVALID), 0);
}

/**
 * @tc.name: OnAbilityUnloadableLocked001
 * @tc.desc: test OnAbilityUnloadableLocked
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, OnAbilityUnloadableLocked001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    systemAbilityStateScheduler->abilityContextMap_.clear();
    systemAbilityStateScheduler->abilityContextMap_[SAID] = systemAbilityContext;
    systemAbilityStateScheduler->OnAbilityLoadedLocked(SAID_INVALID);
    EXPECT_EQ(systemAbilityStateScheduler->abilityContextMap_.count(SAID_INVALID), 0);
}

HWTEST_F(SystemAbilityStateSchedulerTest, GetRunningSystemProcess001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SystemProcessInfo> systemProcessInfos;
    systemAbilityStateScheduler->processContextMap_.clear();
    systemAbilityStateScheduler->processContextMap_[process] = nullptr;
    int32_t ret = systemAbilityStateScheduler->GetRunningSystemProcess(systemProcessInfos);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: GetRunningSystemProcess002
 * @tc.desc: test GetRunningSystemProcess
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, GetRunningSystemProcess002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    std::list<SystemProcessInfo> systemProcessInfos;
    systemProcessContext->state = SystemProcessState::STARTED;
    systemAbilityStateScheduler->processContextMap_.clear();
    systemAbilityStateScheduler->processContextMap_[process] = systemProcessContext;
    int32_t ret = systemAbilityStateScheduler->GetRunningSystemProcess(systemProcessInfos);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: SubscribeSystemProcess001
 * @tc.desc: test SubscribeSystemProcess
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, SubscribeSystemProcess001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    sptr<SystemProcessStatusChange> listener = new SystemProcessStatusChange();
    systemAbilityStateScheduler->processListenerDeath_ =
        sptr<IRemoteObject::DeathRecipient>(new SystemProcessListenerDeathRecipient());
    systemAbilityStateScheduler->processListeners.clear();
    int32_t ret = systemAbilityStateScheduler->SubscribeSystemProcess(listener);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: SubscribeSystemProcess002
 * @tc.desc: test SubscribeSystemProcess
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, SubscribeSystemProcess002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    sptr<SystemProcessStatusChange> listener = new SystemProcessStatusChange();
    systemAbilityStateScheduler->processListenerDeath_ =
        sptr<IRemoteObject::DeathRecipient>(new SystemProcessListenerDeathRecipient());
    systemAbilityStateScheduler->processListeners.clear();
    systemAbilityStateScheduler->processListeners.push_back(listener);
    int32_t ret = systemAbilityStateScheduler->SubscribeSystemProcess(listener);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: UnSubscribeSystemProcess001
 * @tc.desc: test UnSubscribeSystemProcess
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, UnSubscribeSystemProcess001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    sptr<SystemProcessStatusChange> listener = new SystemProcessStatusChange();
    systemAbilityStateScheduler->processListenerDeath_ =
        sptr<IRemoteObject::DeathRecipient>(new SystemProcessListenerDeathRecipient());
    systemAbilityStateScheduler->processListeners.clear();
    systemAbilityStateScheduler->processListeners.push_back(listener);
    int32_t ret = systemAbilityStateScheduler->UnSubscribeSystemProcess(listener);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: UnSubscribeSystemProcess002
 * @tc.desc: test UnSubscribeSystemProcess
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, UnSubscribeSystemProcess002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    sptr<SystemProcessStatusChange> listener = new SystemProcessStatusChange();
    systemAbilityStateScheduler->processListenerDeath_ =
        sptr<IRemoteObject::DeathRecipient>(new SystemProcessListenerDeathRecipient());
    systemAbilityStateScheduler->processListeners.clear();
    int32_t ret = systemAbilityStateScheduler->UnSubscribeSystemProcess(listener);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: ProcessDelayUnloadEvent001
 * @tc.desc: test ProcessDelayUnloadEvent
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, ProcessDelayUnloadEvent001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    systemAbilityStateScheduler->abilityContextMap_.clear();
    int32_t ret = systemAbilityStateScheduler->ProcessDelayUnloadEvent(SAID);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: ProcessDelayUnloadEvent002
 * @tc.desc: test ProcessDelayUnloadEvent
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, ProcessDelayUnloadEvent002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemAbilityStateScheduler->abilityContextMap_.clear();
    systemAbilityContext->ownProcessContext = systemProcessContext;
    systemAbilityStateScheduler->abilityContextMap_[SAID] = systemAbilityContext;
    systemAbilityContext->state = SystemAbilityState::NOT_LOADED;
    int32_t ret = systemAbilityStateScheduler->ProcessDelayUnloadEvent(SAID);
    EXPECT_EQ(ret, ERR_OK);
}
}