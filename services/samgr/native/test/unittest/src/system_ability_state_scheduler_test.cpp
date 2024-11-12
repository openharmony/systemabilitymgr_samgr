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
#include "samgr_err_code.h"
#include "ability_death_recipient.h"
#include "datetime_ex.h"
#include "sa_status_change_mock.h"
#include "test_log.h"

#define private public
#include "schedule/system_ability_state_scheduler.h"
#include "event_handler.h"
#include "system_ability_manager.h"

using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS;

namespace OHOS {
namespace {

constexpr int64_t RESTART_TIME_INTERVAL_LIMIT = 20 * 1000;
constexpr int32_t RESTART_TIMES_LIMIT = 4;
constexpr int32_t SAID_INVALID = -1;
constexpr int32_t SAID = 1234;
constexpr int32_t STATENUMS = 1;
constexpr int32_t DELAY_TIME = 2;
constexpr int32_t INVALID_DELAY_TIME = -1;
constexpr int32_t MAX_DELAY_TIME_TEST = 5 * 60 * 1000;
constexpr int32_t BEYOND_DELAY_TIME_TEST = 5 * 60 * 1000 + 1;
constexpr int32_t TEST_SYSTEM_ABILITY1 = 1491;
constexpr int32_t TEST_SYSTEM_ABILITY2 = 1492;
constexpr int32_t ONCE_DELAY_TIME = 10 * 1000; //ms
constexpr int32_t MAX_DURATION = 11 * 60 * 1000; // ms
const std::u16string process = u"test";
const std::u16string process_invalid = u"test_invalid";
const std::string LOCAL_DEVICE = "local";
const std::string SA_TAG_DEVICE_ON_LINE = "deviceonline";
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
    systemAbilityStateScheduler->abilityContextMap_[SAID] = nullptr;
    bool ret = systemAbilityStateScheduler->GetSystemAbilityContext(SAID, systemAbilityContext);
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
    systemAbilityStateScheduler->processContextMap_[process] = nullptr;
    bool ret = systemAbilityStateScheduler->GetSystemProcessContext(process, systemProcessContext);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: GetSystemProcessContext003
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
    systemProcessContext->state = SystemProcessState::STOPPING;
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
 * @tc.desc: test HandleLoadAbilityEvent, SA is invalid
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
    EXPECT_EQ(ret, GET_SA_CONTEXT_FAIL);
}

/**
 * @tc.name: HandleLoadAbilityEvent006
 * @tc.desc: test HandleLoadAbilityEvent, SA is unloadable
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
    EXPECT_EQ(ret, ACTIVE_SA_FAIL);
}

/**
 * @tc.name: HandleLoadAbilityEvent007
 * @tc.desc: test HandleLoadAbilityEvent with systemProcessContext's state is NOT_STARTED
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, HandleLoadAbilityEvent007, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemAbilityStateScheduler->abilityContextMap_.clear();
    systemAbilityContext->state = SystemAbilityState::NOT_LOADED;
    systemProcessContext->state = SystemProcessState::NOT_STARTED;
    systemAbilityContext->ownProcessContext = systemProcessContext;
    systemAbilityStateScheduler->abilityContextMap_[SAID] = systemAbilityContext;
    bool isExist = false;
    int32_t ret = systemAbilityStateScheduler->HandleLoadAbilityEvent(SAID, isExist);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: HandleLoadAbilityEvent008
 * @tc.desc: test HandleLoadAbilityEvent with systemProcessContext's state is STARTED
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, HandleLoadAbilityEvent008, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemAbilityStateScheduler->abilityContextMap_.clear();
    systemAbilityContext->state = SystemAbilityState::NOT_LOADED;
    systemProcessContext->state = SystemProcessState::STARTED;
    systemAbilityContext->ownProcessContext = systemProcessContext;
    systemAbilityStateScheduler->abilityContextMap_[SAID] = systemAbilityContext;
    bool isExist = false;
    int32_t ret = systemAbilityStateScheduler->HandleLoadAbilityEvent(SAID, isExist);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: HandleLoadAbilityEventLocked001
 * @tc.desc: test HandleLoadAbilityEventLocked, SA is unloading
 * @tc.type: FUNC
 * @tc.require: I6LQ18
 */

HWTEST_F(SystemAbilityStateSchedulerTest, HandleLoadAbilityEventLocked001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);

    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    systemAbilityStateScheduler->abilityContextMap_.clear();
    systemAbilityContext->ownProcessContext = systemProcessContext;
    systemAbilityStateScheduler->abilityContextMap_[SAID] = systemAbilityContext;
    LoadRequestInfo loadRequestInfo;
    loadRequestInfo.systemAbilityId = SAID;
    loadRequestInfo.callback = new SystemAbilityLoadCallbackMock();
    systemAbilityContext->state = SystemAbilityState::UNLOADING;
    int32_t ret = systemAbilityStateScheduler->HandleLoadAbilityEventLocked(systemAbilityContext, loadRequestInfo);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: HandleLoadAbilityEventLocked002
 * @tc.desc: test HandleLoadAbilityEventLocked, process is stopping
 * @tc.type: FUNC
 * @tc.require: I6LQ18
 */

HWTEST_F(SystemAbilityStateSchedulerTest, HandleLoadAbilityEventLocked002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemProcessContext->state = SystemProcessState::STOPPING;
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    systemAbilityStateScheduler->abilityContextMap_.clear();
    systemAbilityContext->ownProcessContext = systemProcessContext;
    systemAbilityStateScheduler->abilityContextMap_[SAID] = systemAbilityContext;
    LoadRequestInfo loadRequestInfo;
    loadRequestInfo.systemAbilityId = SAID;
    loadRequestInfo.callback = new SystemAbilityLoadCallbackMock();
    systemAbilityContext->state = SystemAbilityState::NOT_LOADED;
    int32_t ret = systemAbilityStateScheduler->HandleLoadAbilityEventLocked(systemAbilityContext, loadRequestInfo);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: HandleLoadAbilityEventLocked003
 * @tc.desc: test HandleLoadAbilityEventLocked, SA is loading
 * @tc.type: FUNC
 * @tc.require: I6LQ18
 */

HWTEST_F(SystemAbilityStateSchedulerTest, HandleLoadAbilityEventLocked003, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    systemAbilityStateScheduler->abilityContextMap_.clear();
    systemAbilityContext->ownProcessContext = systemProcessContext;
    systemAbilityStateScheduler->abilityContextMap_[SAID] = systemAbilityContext;
    LoadRequestInfo loadRequestInfo;
    loadRequestInfo.systemAbilityId = SAID;
    loadRequestInfo.callback = new SystemAbilityLoadCallbackMock();
    systemAbilityContext->state = SystemAbilityState::LOADING;
    int32_t ret = systemAbilityStateScheduler->HandleLoadAbilityEventLocked(systemAbilityContext, loadRequestInfo);
    EXPECT_EQ(ret, ERR_OK);
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
    std::shared_ptr<UnloadRequestInfo> unloadRequestInfo =
        std::make_shared<UnloadRequestInfo>(onDemandEvent, SAID, -1);
    int32_t ret = systemAbilityStateScheduler->HandleUnloadAbilityEvent(unloadRequestInfo);
    EXPECT_EQ(ret, GET_SA_CONTEXT_FAIL);
    ret = systemAbilityStateScheduler->HandleUnloadAbilityEvent(nullptr);
    EXPECT_EQ(ret, UNLOAD_REQUEST_NULL);
}

/**
 * @tc.name: HandleUnloadAbilityEvent002
 * @tc.desc: test HandleUnloadAbilityEvent, SA is loading
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
    std::shared_ptr<UnloadRequestInfo> unloadRequestInfo =
        std::make_shared<UnloadRequestInfo>(onDemandEvent, SAID, -1);
    int32_t ret = systemAbilityStateScheduler->HandleUnloadAbilityEvent(unloadRequestInfo);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: HandleUnloadAbilityEvent003
 * @tc.desc: test HandleUnloadAbilityEvent, interface call when SA is loaded
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
    std::shared_ptr<UnloadRequestInfo> unloadRequestInfo =
        std::make_shared<UnloadRequestInfo>(onDemandEvent, SAID, -1);
    int32_t ret = systemAbilityStateScheduler->HandleUnloadAbilityEvent(unloadRequestInfo);
    EXPECT_EQ(ret, GET_SA_CONTEXT_FAIL);
}

/**
 * @tc.name: HandleUnloadAbilityEvent004
 * @tc.desc: test HandleUnloadAbilityEvent, interface call when SA is not loaded
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
    std::shared_ptr<UnloadRequestInfo> unloadRequestInfo =
        std::make_shared<UnloadRequestInfo>(onDemandEvent, SAID, -1);
    int32_t ret = systemAbilityStateScheduler->HandleUnloadAbilityEvent(unloadRequestInfo);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: HandleUnloadAbilityEvent005
 * @tc.desc: test HandleUnloadAbilityEvent, device online when SA is loaded
 * @tc.type: FUNC
 * @tc.require: I6LQ18
 */

HWTEST_F(SystemAbilityStateSchedulerTest, HandleUnloadAbilityEvent005, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    systemAbilityStateScheduler->unloadEventHandler_ = nullptr;

    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    systemAbilityStateScheduler->abilityContextMap_.clear();
    systemAbilityContext->ownProcessContext = systemProcessContext;
    systemAbilityStateScheduler->abilityContextMap_[SAID] = systemAbilityContext;
    systemAbilityContext->state = SystemAbilityState::LOADED;
    OnDemandEvent onDemandEvent = {DEVICE_ONLINE};
    std::shared_ptr<UnloadRequestInfo> unloadRequestInfo =
        std::make_shared<UnloadRequestInfo>(onDemandEvent, SAID, -1);
    int32_t ret = systemAbilityStateScheduler->HandleUnloadAbilityEvent(unloadRequestInfo);
    EXPECT_EQ(ret, UNLOAD_EVENT_HANDLER_NULL);
}

/**
 * @tc.name: HandleUnloadAbilityEventLock001
 * @tc.desc: test HandleUnloadAbilityEventLock, unloadRequestInfo is nullptr
 * @tc.type: FUNC
 * @tc.require: I6LQ18
 */
HWTEST_F(SystemAbilityStateSchedulerTest, HandleUnloadAbilityEventLock001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    int32_t ret = systemAbilityStateScheduler->HandleUnloadAbilityEventLocked(systemAbilityContext, nullptr);
    EXPECT_EQ(ret, UNLOAD_REQUEST_NULL);
}

HWTEST_F(SystemAbilityStateSchedulerTest, KillSystemProcessLocked002, TestSize.Level3)
{
    cout << "begin KillSystemProcessLocked002 "<< endl;
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemProcessContext->processName = u"1234567890123456789012345678901234567890123456789"
        "01234567890123456789012345678901234567890123456";
    int result = systemAbilityStateScheduler->KillSystemProcessLocked(systemProcessContext);
    cout << "begin KillSystemProcessLocked002 result is "<< result << endl;
    EXPECT_EQ(result, 102);
}

/**
 * @tc.name: HandleCancelUnloadAbilityEvent001
 * @tc.desc: test HandleCancelUnloadAbilityEvent, SA is invalid
 * @tc.type: FUNC
 * @tc.require: I6LQ18
 */

HWTEST_F(SystemAbilityStateSchedulerTest, HandleCancelUnloadAbilityEvent001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    systemAbilityStateScheduler->abilityContextMap_.clear();
    int32_t ret = systemAbilityStateScheduler->HandleCancelUnloadAbilityEvent(SAID);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: HandleCancelUnloadAbilityEvent002
 * @tc.desc: test HandleCancelUnloadAbilityEvent, SA is loaded
 * @tc.type: FUNC
 * @tc.require: I6LQ18
 */

HWTEST_F(SystemAbilityStateSchedulerTest, HandleCancelUnloadAbilityEvent002, TestSize.Level3)
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
    int32_t ret = systemAbilityStateScheduler->HandleCancelUnloadAbilityEvent(SAID);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: HandleCancelUnloadAbilityEvent003
 * @tc.desc: test HandleCancelUnloadAbilityEvent, SA is unloadable
 * @tc.type: FUNC
 * @tc.require: I6LQ18
 */

HWTEST_F(SystemAbilityStateSchedulerTest, HandleCancelUnloadAbilityEvent003, TestSize.Level3)
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
    systemAbilityContext->state = SystemAbilityState::UNLOADABLE;
    int32_t ret = systemAbilityStateScheduler->HandleCancelUnloadAbilityEvent(SAID);
    EXPECT_EQ(ret, ACTIVE_SA_FAIL);
}

/**
 * @tc.name: SendDelayUnloadEventLocked001
 * @tc.desc: test SendDelayUnloadEventLocked, handler is nullptr
 * @tc.type: FUNC
 * @tc.require: I6LQ18
 */

HWTEST_F(SystemAbilityStateSchedulerTest, SendDelayUnloadEventLocked001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    systemAbilityStateScheduler->unloadEventHandler_ = nullptr;
    int32_t ret = systemAbilityStateScheduler->SendDelayUnloadEventLocked(SAID, DELAY_TIME);
    EXPECT_EQ(ret, UNLOAD_EVENT_HANDLER_NULL);
}

/**
 * @tc.name: SendDelayUnloadEventLocked002
 * @tc.desc: test SendDelayUnloadEventLocked, has delay event
 * @tc.type: FUNC
 * @tc.require: I6LQ18
 */

HWTEST_F(SystemAbilityStateSchedulerTest, SendDelayUnloadEventLocked002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    systemAbilityStateScheduler->SendDelayUnloadEventLocked(SAID, DELAY_TIME);
    int32_t ret = systemAbilityStateScheduler->SendDelayUnloadEventLocked(SAID, DELAY_TIME);
    EXPECT_EQ(ret, ERR_OK);
    systemAbilityStateScheduler->RemoveDelayUnloadEventLocked(SAID);
}

/**
 * @tc.name: SendDelayUnloadEventLocked003
 * @tc.desc: test SendDelayUnloadEventLocked, not has delay event
 * @tc.type: FUNC
 * @tc.require: I6LQ18
 */

HWTEST_F(SystemAbilityStateSchedulerTest, SendDelayUnloadEventLocked003, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    int32_t ret = systemAbilityStateScheduler->SendDelayUnloadEventLocked(SAID, DELAY_TIME);
    EXPECT_EQ(ret, ERR_OK);
    systemAbilityStateScheduler->RemoveDelayUnloadEventLocked(SAID);
}

/**
 * @tc.name: RemoveDelayUnloadEventLocked001
 * @tc.desc: test RemoveDelayUnloadEventLocked, handler is nullptr
 * @tc.type: FUNC
 * @tc.require: I6LQ18
 */

HWTEST_F(SystemAbilityStateSchedulerTest, RemoveDelayUnloadEventLocked001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    systemAbilityStateScheduler->unloadEventHandler_ = nullptr;
    int32_t ret = systemAbilityStateScheduler->RemoveDelayUnloadEventLocked(SAID);
    EXPECT_EQ(ret, UNLOAD_EVENT_HANDLER_NULL);
}

/**
 * @tc.name: RemoveDelayUnloadEventLocked002
 * @tc.desc: test RemoveDelayUnloadEventLocked, has delay event
 * @tc.type: FUNC
 * @tc.require: I6LQ18
 */

HWTEST_F(SystemAbilityStateSchedulerTest, RemoveDelayUnloadEventLocked002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    systemAbilityStateScheduler->SendDelayUnloadEventLocked(SAID, DELAY_TIME);
    int32_t ret = systemAbilityStateScheduler->RemoveDelayUnloadEventLocked(SAID);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: RemoveDelayUnloadEventLocked003
 * @tc.desc: test RemoveDelayUnloadEventLocked, not has delay event
 * @tc.type: FUNC
 * @tc.require: I6LQ18
 */

HWTEST_F(SystemAbilityStateSchedulerTest, RemoveDelayUnloadEventLocked003, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    int32_t ret = systemAbilityStateScheduler->RemoveDelayUnloadEventLocked(SAID);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: PendLoadEventLocked001
 * @tc.desc: test PendLoadEventLocked, callback is nullptr
 * @tc.type: FUNC
 * @tc.require: I6LQ18
 */

HWTEST_F(SystemAbilityStateSchedulerTest, PendLoadEventLocked001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);

    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    LoadRequestInfo loadRequestInfo;
    loadRequestInfo.systemAbilityId = SAID;
    loadRequestInfo.callback = nullptr;
    int32_t ret = systemAbilityStateScheduler->PendLoadEventLocked(systemAbilityContext, loadRequestInfo);
    EXPECT_EQ(ret, CALLBACK_NULL);
}

/**
 * @tc.name: PendLoadEventLocked002
 * @tc.desc: test PendLoadEventLocked, callback not exist in list
 * @tc.type: FUNC
 * @tc.require: I6LQ18
 */

HWTEST_F(SystemAbilityStateSchedulerTest, PendLoadEventLocked002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);

    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    LoadRequestInfo loadRequestInfo;
    loadRequestInfo.systemAbilityId = SAID;
    loadRequestInfo.callback = new SystemAbilityLoadCallbackMock();
    int32_t ret = systemAbilityStateScheduler->PendLoadEventLocked(systemAbilityContext, loadRequestInfo);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: PendLoadEventLocked003
 * @tc.desc: test PendLoadEventLocked, callback already exist in list
 * @tc.type: FUNC
 * @tc.require: I6LQ18
 */

HWTEST_F(SystemAbilityStateSchedulerTest, PendLoadEventLocked003, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);

    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    LoadRequestInfo loadRequestInfo;
    loadRequestInfo.systemAbilityId = SAID;
    loadRequestInfo.callback = new SystemAbilityLoadCallbackMock();
    systemAbilityStateScheduler->PendLoadEventLocked(systemAbilityContext, loadRequestInfo);
    int32_t ret = systemAbilityStateScheduler->PendLoadEventLocked(systemAbilityContext, loadRequestInfo);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: PendLoadEventLocked004
 * @tc.desc: test PendLoadEventLocked with loadRequestInfo's callback is nullptr
 * @tc.type: FUNC
 * @tc.require: I6LQ18
 */

HWTEST_F(SystemAbilityStateSchedulerTest, PendLoadEventLocked004, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    LoadRequestInfo loadRequestInfo;
    int32_t ret = systemAbilityStateScheduler->PendLoadEventLocked(systemAbilityContext, loadRequestInfo);
    EXPECT_EQ(ret, CALLBACK_NULL);
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
 * @tc.name: HandlePendingLoadEventLocked001
 * @tc.desc: test HandlePendingLoadEventLocked, no pending event
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(SystemAbilityStateSchedulerTest, HandlePendingLoadEventLocked001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    systemAbilityContext->pendingEvent = PendingEvent::NO_EVENT;
    int32_t ret = systemAbilityStateScheduler->HandlePendingLoadEventLocked(systemAbilityContext);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: HandlePendingLoadEventLocked002
 * @tc.desc: test HandlePendingLoadEventLocked, load pending event
 * @tc.type: FUNC
 * @tc.require: I6LQ18
 */

HWTEST_F(SystemAbilityStateSchedulerTest, HandlePendingLoadEventLocked002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    systemAbilityContext->pendingEvent = PendingEvent::LOAD_ABILITY_EVENT;
    int32_t ret = systemAbilityStateScheduler->HandlePendingLoadEventLocked(systemAbilityContext);
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
    OnDemandEvent onDemandEvent = {INTERFACE_CALL};
    systemAbilityContext->pendingUnloadEvent = std::make_shared<UnloadRequestInfo>(onDemandEvent, SAID);
    int32_t ret = systemAbilityStateScheduler->HandlePendingUnloadEventLocked(systemAbilityContext);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: HandlePendingUnloadEventLocked002
 * @tc.desc: test HandlePendingUnloadEventLocked
 * @tc.type: FUNC
 * @tc.require: I6LQ18
 */

HWTEST_F(SystemAbilityStateSchedulerTest, HandlePendingUnloadEventLocked002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    systemAbilityContext->pendingEvent = PendingEvent::UNLOAD_ABILITY_EVENT;
    systemAbilityContext->state = SystemAbilityState::NOT_LOADED;
    OnDemandEvent onDemandEvent = {INTERFACE_CALL};
    systemAbilityContext->pendingUnloadEvent = std::make_shared<UnloadRequestInfo>(onDemandEvent, SAID);
    int32_t ret = systemAbilityStateScheduler->HandlePendingUnloadEventLocked(systemAbilityContext);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: HandlePendingUnloadEventLocked003
 * @tc.desc: test HandlePendingUnloadEventLocked with pendingUnloadEvent is nullptr
 * @tc.type: FUNC
 * @tc.require: I6LQ18
 */

HWTEST_F(SystemAbilityStateSchedulerTest, HandlePendingUnloadEventLocked003, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    systemAbilityContext->pendingEvent = PendingEvent::UNLOAD_ABILITY_EVENT;
    systemAbilityContext->state = SystemAbilityState::NOT_LOADED;
    systemAbilityContext->pendingUnloadEvent = nullptr;
    int32_t ret = systemAbilityStateScheduler->HandlePendingUnloadEventLocked(systemAbilityContext);
    EXPECT_EQ(ret, UNLOAD_REQUEST_NULL);
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
 * @tc.desc: test TryUnloadAllSystemAbility, can unload
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
    EXPECT_EQ(ret, TRANSIT_PROC_STATE_FAIL);
}

/**
 * @tc.name: CanUnloadAllSystemAbility001
 * @tc.desc: test CanUnloadAllSystemAbility, unloadable SA <= 0
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
 * @tc.name: CanUnloadAllSystemAbility002
 * @tc.desc: test CanUnloadAllSystemAbility, invalid SA count
 * @tc.type: FUNC
 * @tc.require: I6LQ18
 */

HWTEST_F(SystemAbilityStateSchedulerTest, CanUnloadAllSystemAbility002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemProcessContext->abilityStateCountMap.clear();
    systemProcessContext->abilityStateCountMap[SystemAbilityState::UNLOADABLE] = 1;
    bool ret = systemAbilityStateScheduler->CanUnloadAllSystemAbility(systemProcessContext);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: CanUnloadAllSystemAbility003
 * @tc.desc: test CanUnloadAllSystemAbility, valid SA count
 * @tc.type: FUNC
 * @tc.require: I6LQ18
 */

HWTEST_F(SystemAbilityStateSchedulerTest, CanUnloadAllSystemAbility003, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemProcessContext->saList.push_back(SAID);
    systemProcessContext->abilityStateCountMap.clear();
    systemProcessContext->abilityStateCountMap[SystemAbilityState::UNLOADABLE] = 1;
    bool ret = systemAbilityStateScheduler->CanUnloadAllSystemAbility(systemProcessContext);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: UnloadAllSystemAbilityLocked001
 * @tc.desc: test UnloadAllSystemAbilityLocked, invalid SA
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
    EXPECT_EQ(ret, TRANSIT_PROC_STATE_FAIL);
}

/**
 * @tc.name: UnloadAllSystemAbilityLocked002
 * @tc.desc: test UnloadAllSystemAbilityLocked, SA is unloadable
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
    EXPECT_EQ(ret, TRANSIT_PROC_STATE_FAIL);
}

/**
 * @tc.name: UnloadAllSystemAbilityLocked003
 * @tc.desc: test UnloadAllSystemAbilityLocked, SA is loaded
 * @tc.type: FUNC
 * @tc.require: I6LQ18
 */

HWTEST_F(SystemAbilityStateSchedulerTest, UnloadAllSystemAbilityLocked003, TestSize.Level3)
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
    systemAbilityContext->state = SystemAbilityState::LOADED;
    int32_t ret = systemAbilityStateScheduler->UnloadAllSystemAbilityLocked(systemProcessContext);
    EXPECT_EQ(ret, TRANSIT_PROC_STATE_FAIL);
}

/**
 * @tc.name: TryKillSystemProcess001
 * @tc.desc: test TryKillSystemProcess, processContext is nullptr
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
 * @tc.desc: test TryKillSystemProcess, can kill process
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
 * @tc.desc: test TryKillSystemProcess, cannot kill process
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
 * @tc.desc: test CanKillSystemProcess, can kill process
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
 * @tc.desc: test CanKillSystemProcess, cannot kill process
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
 * @tc.name: OnProcessStartedLocked001
 * @tc.desc: test OnProcessStartedLocked, invalid process
 * @tc.type: FUNC
 * @tc.require: I6OU0A
 */

HWTEST_F(SystemAbilityStateSchedulerTest, OnProcessStartedLocked001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemAbilityStateScheduler->processContextMap_.clear();
    systemAbilityStateScheduler->OnProcessStartedLocked(process);
    EXPECT_TRUE(systemAbilityStateScheduler->processContextMap_.empty());
}

/**
 * @tc.name: OnProcessStartedLocked002
 * @tc.desc: test OnProcessStartedLocked, valid process
 * @tc.type: FUNC
 * @tc.require: I6OU0A
 */

HWTEST_F(SystemAbilityStateSchedulerTest, OnProcessStartedLocked002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemAbilityStateScheduler->processContextMap_.clear();
    systemAbilityStateScheduler->processContextMap_[process] = systemProcessContext;
    systemAbilityStateScheduler->OnProcessNotStartedLocked(process);
    EXPECT_FALSE(systemAbilityStateScheduler->processContextMap_.empty());
}

/**
 * @tc.name: OnProcessStartedLocked003
 * @tc.desc: test OnProcessStartedLocked, listener is not nullptr
 * @tc.type: FUNC
 * @tc.require: I6OU0A
 */

HWTEST_F(SystemAbilityStateSchedulerTest, OnProcessStartedLocked003, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    sptr<ISystemProcessStatusChange> listener = new SystemProcessStatusChange();
    systemAbilityStateScheduler->processListeners_.push_back(listener);
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemAbilityStateScheduler->processContextMap_.clear();
    systemAbilityStateScheduler->processContextMap_[process] = systemProcessContext;
    systemAbilityStateScheduler->OnProcessNotStartedLocked(process);
    EXPECT_FALSE(systemAbilityStateScheduler->processContextMap_.empty());
}

/**
 * @tc.name: OnProcessNotStartedLocked001
 * @tc.desc: test OnProcessNotStartedLocked, invalid process
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
 * @tc.desc: test OnProcessNotStartedLocked, valid process
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
 * @tc.desc: test OnAbilityNotLoadedLocked, invalid SA
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
 * @tc.desc: test OnAbilityNotLoadedLocked, valid SA
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
 * @tc.desc: test OnAbilityLoadedLocked, invalid SA
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
 * @tc.desc: test OnAbilityUnloadableLocked, invalid SA
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
    systemAbilityStateScheduler->OnAbilityUnloadableLocked(SAID_INVALID);
    EXPECT_EQ(systemAbilityStateScheduler->abilityContextMap_.count(SAID_INVALID), 0);
}

/**
 * @tc.name: GetSystemProcessInfo001
 * @tc.desc: test GetSystemProcessInfo, systemAbilityContext is nullptr
 * @tc.type: FUNC
 * @tc.require: I7VQQG
 */

HWTEST_F(SystemAbilityStateSchedulerTest, GetSystemProcessInfo001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    systemAbilityStateScheduler->abilityContextMap_[SAID] = nullptr;
    SystemProcessInfo processInfo;
    int32_t ret = systemAbilityStateScheduler->GetSystemProcessInfo(SAID, processInfo);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: GetSystemProcessInfo002
 * @tc.desc: test GetSystemProcessInfo, processContext is nullptr
 * @tc.type: FUNC
 * @tc.require: I7VQQG
 */

HWTEST_F(SystemAbilityStateSchedulerTest, GetSystemProcessInfo002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::shared_ptr<SystemAbilityContext> systemAbilityContext =
        std::make_shared<SystemAbilityContext>();
    systemAbilityContext->ownProcessContext = nullptr;
    systemAbilityStateScheduler->abilityContextMap_.clear();
    systemAbilityStateScheduler->abilityContextMap_[SAID] = systemAbilityContext;
    SystemProcessInfo processInfo;
    int32_t ret = systemAbilityStateScheduler->GetSystemProcessInfo(SAID, processInfo);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: GetSystemProcessInfo003
 * @tc.desc: test GetSystemProcessInfo, valid process info
 * @tc.type: FUNC
 * @tc.require: I7VQQG
 */

HWTEST_F(SystemAbilityStateSchedulerTest, GetSystemProcessInfo003, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::shared_ptr<SystemAbilityContext> systemAbilityContext =
        std::make_shared<SystemAbilityContext>();
    std::shared_ptr<SystemProcessContext> systemProcessContext =
        std::make_shared<SystemProcessContext>();
    systemAbilityContext->ownProcessContext = systemProcessContext;
    systemAbilityStateScheduler->abilityContextMap_.clear();
    systemAbilityStateScheduler->abilityContextMap_[SAID] = systemAbilityContext;
    SystemProcessInfo processInfo;
    int32_t ret = systemAbilityStateScheduler->GetSystemProcessInfo(SAID, processInfo);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: GetRunningSystemProcess001
 * @tc.desc: test GetRunningSystemProcess, processContext is nullptr
 * @tc.type: FUNC
 * @tc.require: I6LQ18
 */

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
 * @tc.desc: test GetRunningSystemProcess, process is started
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
 * @tc.name: GetRunningSystemProcess003
 * @tc.desc: test GetRunningSystemProcess, process is not started
 * @tc.type: FUNC
 * @tc.require: I6LQ18
 */

HWTEST_F(SystemAbilityStateSchedulerTest, GetRunningSystemProcess003, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    std::list<SystemProcessInfo> systemProcessInfos;
    systemProcessContext->state = SystemProcessState::NOT_STARTED;
    systemAbilityStateScheduler->processContextMap_.clear();
    systemAbilityStateScheduler->processContextMap_[process] = systemProcessContext;
    int32_t ret = systemAbilityStateScheduler->GetRunningSystemProcess(systemProcessInfos);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: SubscribeSystemProcess001
 * @tc.desc: test SubscribeSystemProcess, listener is not exist in list
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
    systemAbilityStateScheduler->processListeners_.clear();
    int32_t ret = systemAbilityStateScheduler->SubscribeSystemProcess(listener);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: SubscribeSystemProcess002
 * @tc.desc: test SubscribeSystemProcess, listener is exist in list
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
    systemAbilityStateScheduler->processListeners_.clear();
    systemAbilityStateScheduler->processListeners_.push_back(listener);
    int32_t ret = systemAbilityStateScheduler->SubscribeSystemProcess(listener);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: SubscribeSystemProcess003
 * @tc.desc: test SubscribeSystemProcess, processListenerDeath is nullptr
 * @tc.type: FUNC
 * @tc.require: I6LQ18
 */

HWTEST_F(SystemAbilityStateSchedulerTest, SubscribeSystemProcess003, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    sptr<SystemProcessStatusChange> listener = new SystemProcessStatusChange();
    systemAbilityStateScheduler->processListenerDeath_ = nullptr;
    systemAbilityStateScheduler->processListeners_.clear();
    systemAbilityStateScheduler->processListeners_.push_back(listener);
    int32_t ret = systemAbilityStateScheduler->SubscribeSystemProcess(listener);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: UnSubscribeSystemProcess001
 * @tc.desc: test UnSubscribeSystemProcess, listener is not exist in list
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
    systemAbilityStateScheduler->processListeners_.clear();
    systemAbilityStateScheduler->processListeners_.push_back(listener);
    int32_t ret = systemAbilityStateScheduler->UnSubscribeSystemProcess(listener);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: UnSubscribeSystemProcess002
 * @tc.desc: test UnSubscribeSystemProcess, listener is exist in list
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
    systemAbilityStateScheduler->processListeners_.clear();
    int32_t ret = systemAbilityStateScheduler->UnSubscribeSystemProcess(listener);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: UnSubscribeSystemProcess003
 * @tc.desc: test UnSubscribeSystemProcess, processListenerDeath is nullptr
 * @tc.type: FUNC
 * @tc.require: I6LQ18
 */

HWTEST_F(SystemAbilityStateSchedulerTest, UnSubscribeSystemProcess003, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    sptr<SystemProcessStatusChange> listener = new SystemProcessStatusChange();
    systemAbilityStateScheduler->processListenerDeath_ = nullptr;
    systemAbilityStateScheduler->processListeners_.clear();
    systemAbilityStateScheduler->processListeners_.push_back(listener);
    int32_t ret = systemAbilityStateScheduler->UnSubscribeSystemProcess(listener);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: OnAbilityUnloadableLocked002
 * @tc.desc: test OnAbilityUnloadableLocked, valid SA
 * @tc.type: FUNC
 * @tc.require: I6LQ18
 */

HWTEST_F(SystemAbilityStateSchedulerTest, OnAbilityUnloadableLocked002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    systemAbilityStateScheduler->abilityContextMap_.clear();
    systemAbilityStateScheduler->abilityContextMap_[SAID] = systemAbilityContext;
    systemAbilityStateScheduler->OnAbilityLoadedLocked(SAID);
    EXPECT_EQ(systemAbilityStateScheduler->abilityContextMap_.count(SAID), 1);
}

/**
 * @tc.name: ProcessDelayUnloadEvent001
 * @tc.desc: test ProcessDelayUnloadEvent, invalid SA
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
    EXPECT_EQ(ret, GET_SA_CONTEXT_FAIL);
}

/**
 * @tc.name: ProcessDelayUnloadEvent002
 * @tc.desc: test ProcessDelayUnloadEvent, SA is not loaded
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
    OnDemandEvent onDemandEvent = {INTERFACE_CALL};
    systemAbilityContext->unloadRequest = std::make_shared<UnloadRequestInfo>(onDemandEvent, SAID);
    int32_t ret = systemAbilityStateScheduler->ProcessDelayUnloadEvent(SAID);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: ProcessDelayUnloadEvent003
 * @tc.desc: test ProcessDelayUnloadEvent, SA is loaded
 * @tc.type: FUNC
 * @tc.require: I6LQ18
 */

HWTEST_F(SystemAbilityStateSchedulerTest, ProcessDelayUnloadEvent003, TestSize.Level3)
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
    systemAbilityContext->unloadRequest = std::make_shared<UnloadRequestInfo>(onDemandEvent, SAID);
    int32_t ret = systemAbilityStateScheduler->ProcessDelayUnloadEvent(SAID);
    EXPECT_EQ(ret, IDLE_SA_FAIL);
}

/**
 * @tc.name: Test CheckStartEnableOnce001
 * @tc.desc: CheckStartEnableOnce001
 * @tc.type: FUNC
 * @tc.require: I6H10P
 */
HWTEST_F(SystemAbilityStateSchedulerTest, CheckStartEnableOnce001, TestSize.Level3)
{
    DTEST_LOG << " CheckStartEnableOnce001 " << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    OnDemandEvent event1 = { DEVICE_ONLINE, SA_TAG_DEVICE_ON_LINE, "on" };
    OnDemandEvent event2 = { DEVICE_ONLINE, SA_TAG_DEVICE_ON_LINE, "on" };
    bool res = (event1 == event2);
    EXPECT_EQ(res, true);
    SaControlInfo saControl = { START_ON_DEMAND, TEST_SYSTEM_ABILITY1};
    sptr<ISystemAbilityLoadCallback> callback = new SystemAbilityLoadCallbackMock();
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    int32_t result = systemAbilityStateScheduler->CheckStartEnableOnce(event1, saControl, callback);
    EXPECT_EQ(result, GET_SA_CONTEXT_FAIL);
    saMgr->RemoveSystemAbility(TEST_SYSTEM_ABILITY1);
}

/**
 * @tc.name: Test CheckStartEnableOnce002
 * @tc.desc: CheckStartEnableOnce002
 * @tc.type: FUNC
 * @tc.require: I6H10P
 */
HWTEST_F(SystemAbilityStateSchedulerTest, CheckStartEnableOnce002, TestSize.Level3)
{
    DTEST_LOG << " CheckStartEnableOnce002 " << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    OnDemandEvent event = { DEVICE_ONLINE, SA_TAG_DEVICE_ON_LINE, "on" };
    SaControlInfo saControl = { START_ON_DEMAND, TEST_SYSTEM_ABILITY1, true};
    sptr<ISystemAbilityLoadCallback> callback = new SystemAbilityLoadCallbackMock();
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    int32_t result = systemAbilityStateScheduler->CheckStartEnableOnce(event, saControl, callback);
    EXPECT_EQ(result, GET_SA_CONTEXT_FAIL);
    saMgr->RemoveSystemAbility(TEST_SYSTEM_ABILITY1);
}

/**
 * @tc.name: Test CheckStartEnableOnce003
 * @tc.desc: CheckStartEnableOnce003
 * @tc.type: FUNC
 * @tc.require: I6H10P
 */
HWTEST_F(SystemAbilityStateSchedulerTest, CheckStartEnableOnce003, TestSize.Level3)
{
    DTEST_LOG << " CheckStartEnableOnce003 " << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    OnDemandEvent event = { DEVICE_ONLINE, SA_TAG_DEVICE_ON_LINE, "on" };
    SaControlInfo saControl = { START_ON_DEMAND, TEST_SYSTEM_ABILITY1, true};
    sptr<ISystemAbilityLoadCallback> callback = new SystemAbilityLoadCallbackMock();
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    systemAbilityStateScheduler->startEnableOnceMap_[saControl.saId].emplace_back(event);
    int32_t result = systemAbilityStateScheduler->CheckStartEnableOnce(event, saControl, callback);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    saMgr->RemoveSystemAbility(TEST_SYSTEM_ABILITY1);
}

/**
 * @tc.name: Test CheckStartEnableOnce004
 * @tc.desc: CheckStartEnableOnce004 saControl.enableOnce is true
 * @tc.type: FUNC
 * @tc.require: I6MO6A
 */
HWTEST_F(SystemAbilityStateSchedulerTest, CheckStartEnableOnce004, TestSize.Level3)
{
    DTEST_LOG << " CheckStartEnableOnce004 " << std::endl;
    SaControlInfo saControl;
    saControl.enableOnce = true;
    OnDemandEvent event;
    sptr<ISystemAbilityLoadCallback> callback = new SystemAbilityLoadCallbackMock();
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    systemAbilityStateScheduler->startEnableOnceMap_[saControl.saId].emplace_back(event);
    int32_t result = systemAbilityStateScheduler->CheckStartEnableOnce(event, saControl, callback);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: CheckStartEnableOnce005
 * @tc.desc: test CheckStartEnableOnce with startEnableOnceMap_ contains saControl's SaID and event is same
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityStateSchedulerTest, CheckStartEnableOnce005, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    OnDemandEvent onDemandEvent;
    std::list<OnDemandEvent> onDemandList;
    onDemandList.emplace_back(onDemandEvent);
    systemAbilityStateScheduler->startEnableOnceMap_.clear();
    systemAbilityStateScheduler->startEnableOnceMap_[SAID] = onDemandList;
    SaControlInfo saControlInfo;
    saControlInfo.saId = SAID;
    saControlInfo.enableOnce = true;
    sptr<ISystemAbilityLoadCallback> callback = new SystemAbilityLoadCallbackMock();

    int32_t ret = systemAbilityStateScheduler->CheckStartEnableOnce(onDemandEvent, saControlInfo, callback);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: CheckStartEnableOnce006
 * @tc.desc: test CheckStartEnableOnce with startEnableOnceMap_ is empty
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityStateSchedulerTest, CheckStartEnableOnce006, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    OnDemandEvent onDemandEvent;
    SaControlInfo saControlInfo;
    saControlInfo.saId = SAID;
    saControlInfo.enableOnce = true;
    systemAbilityStateScheduler->startEnableOnceMap_.clear();
    sptr<ISystemAbilityLoadCallback> callback = new SystemAbilityLoadCallbackMock();
    int32_t ret = systemAbilityStateScheduler->CheckStartEnableOnce(onDemandEvent, saControlInfo, callback);
    EXPECT_EQ(ret, GET_SA_CONTEXT_FAIL);
}

/**
 * @tc.name: CheckStartEnableOnce007
 * @tc.desc: test CheckStartEnableOnce with startEnableOnceMap_ is not empty and event is not same
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityStateSchedulerTest, CheckStartEnableOnce007, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    OnDemandEvent anotherOnDemandEvent;
    OnDemandEvent onDemandEvent;
    std::list<OnDemandEvent> onDemandList;
    onDemandList.emplace_back(onDemandEvent);
    systemAbilityStateScheduler->startEnableOnceMap_.clear();
    systemAbilityStateScheduler->startEnableOnceMap_[SAID] = onDemandList;
    SaControlInfo saControlInfo;
    saControlInfo.enableOnce = true;
    saControlInfo.saId = SAID;
    sptr<ISystemAbilityLoadCallback> callback = new SystemAbilityLoadCallbackMock();
    int32_t ret = systemAbilityStateScheduler->CheckStartEnableOnce(anotherOnDemandEvent, saControlInfo, callback);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: CheckStartEnableOnce008
 * @tc.desc: test CheckStartEnableOnce with saControl's enableOnce is false
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityStateSchedulerTest, CheckStartEnableOnce008, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    OnDemandEvent onDemandEvent;
    systemAbilityStateScheduler->startEnableOnceMap_.clear();
    SaControlInfo saControlInfo;
    sptr<ISystemAbilityLoadCallback> callback = new SystemAbilityLoadCallbackMock();
    int32_t ret = systemAbilityStateScheduler->CheckStartEnableOnce(onDemandEvent, saControlInfo, callback);
    EXPECT_EQ(ret, GET_SA_CONTEXT_FAIL);
}

/**
 * @tc.name: Test CheckStopEnableOnce001
 * @tc.desc: CheckStopEnableOnce001
 * @tc.type: FUNC
 * @tc.require: I6H10P
 */
HWTEST_F(SystemAbilityStateSchedulerTest, CheckStopEnableOnce001, TestSize.Level3)
{
    DTEST_LOG << " CheckStopEnableOnce001 " << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    OnDemandEvent event = { DEVICE_ONLINE, SA_TAG_DEVICE_ON_LINE, "off" };
    SaControlInfo saControl = { STOP_ON_DEMAND, TEST_SYSTEM_ABILITY1};
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    int32_t result = systemAbilityStateScheduler->CheckStopEnableOnce(event, saControl);
    EXPECT_EQ(result, GET_SA_CONTEXT_FAIL);
    saMgr->RemoveSystemAbility(TEST_SYSTEM_ABILITY1);
}

/**
 * @tc.name: Test CheckStopEnableOnce002
 * @tc.desc: CheckStopEnableOnce002
 * @tc.type: FUNC
 * @tc.require: I6H10P
 */
HWTEST_F(SystemAbilityStateSchedulerTest, CheckStopEnableOnce002, TestSize.Level3)
{
    DTEST_LOG << " CheckStopEnableOnce002 " << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    OnDemandEvent event = { DEVICE_ONLINE, SA_TAG_DEVICE_ON_LINE, "off" };
    SaControlInfo saControl = { STOP_ON_DEMAND, TEST_SYSTEM_ABILITY1, true};
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    int32_t result = systemAbilityStateScheduler->CheckStopEnableOnce(event, saControl);
    EXPECT_EQ(result, GET_SA_CONTEXT_FAIL);
    saMgr->RemoveSystemAbility(TEST_SYSTEM_ABILITY1);
}

/**
 * @tc.name: Test CheckStopEnableOnce003
 * @tc.desc: CheckStopEnableOnce003
 * @tc.type: FUNC
 * @tc.require: I6H10P
 */
HWTEST_F(SystemAbilityStateSchedulerTest, CheckStopEnableOnce003, TestSize.Level3)
{
    DTEST_LOG << " CheckStopEnableOnce003 " << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    OnDemandEvent event = { DEVICE_ONLINE, SA_TAG_DEVICE_ON_LINE, "off" };
    SaControlInfo saControl = { STOP_ON_DEMAND, TEST_SYSTEM_ABILITY1, true};
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    systemAbilityStateScheduler->stopEnableOnceMap_[saControl.saId].emplace_back(event);
    int32_t result = systemAbilityStateScheduler->CheckStopEnableOnce(event, saControl);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    saMgr->RemoveSystemAbility(TEST_SYSTEM_ABILITY1);
}

/**
 * @tc.name: ProcessEvent001
 * @tc.desc: test ProcessEvent, event is nullptr
 * @tc.type: FUNC
 * @tc.require: I6OU0A
 */
HWTEST_F(SystemAbilityStateSchedulerTest, ProcessEvent001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityStateScheduler::UnloadEventHandler> unloadEventHandler =
        std::make_shared<SystemAbilityStateScheduler::UnloadEventHandler>(systemAbilityStateScheduler);
    AppExecFwk::InnerEvent *event = nullptr;
    auto destructor = [](AppExecFwk::InnerEvent *event) {
        if (event != nullptr) {
            delete event;
        }
    };
    unloadEventHandler->ProcessEvent(0);
    EXPECT_EQ(event, nullptr);
}

/**
 * @tc.name: ProcessEvent002
 * @tc.desc: test ProcessEvent, stateScheduler_ is nullptr
 * @tc.type: FUNC
 * @tc.require: I6OU0A
 */
HWTEST_F(SystemAbilityStateSchedulerTest, ProcessEvent002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler = nullptr;
    std::shared_ptr<SystemAbilityStateScheduler::UnloadEventHandler> unloadEventHandler =
        std::make_shared<SystemAbilityStateScheduler::UnloadEventHandler>(systemAbilityStateScheduler);
    AppExecFwk::InnerEvent *event = new AppExecFwk::InnerEvent();
    auto destructor = [](AppExecFwk::InnerEvent *event) {
        if (event != nullptr) {
            delete event;
        }
    };
    unloadEventHandler->ProcessEvent(0);
    EXPECT_NE(event, nullptr);
}

/**
 * @tc.name: ProcessEvent003
 * @tc.desc: test ProcessEvent, stateScheduler_ is not nullptr
 * @tc.type: FUNC
 * @tc.require: I6OU0A
 */
HWTEST_F(SystemAbilityStateSchedulerTest, ProcessEvent003, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityStateScheduler::UnloadEventHandler> unloadEventHandler =
        std::make_shared<SystemAbilityStateScheduler::UnloadEventHandler>(systemAbilityStateScheduler);
    AppExecFwk::InnerEvent *event = new AppExecFwk::InnerEvent();
    auto destructor = [](AppExecFwk::InnerEvent *event) {
        if (event != nullptr) {
            delete event;
        }
    };
    unloadEventHandler->ProcessEvent(0);
    EXPECT_NE(event, nullptr);
}

/**
 * @tc.name: CanRestartProcessLocked001
 * @tc.desc: test CanRestartProcessLocked, with enableRestart is true
 * @tc.type: FUNC
 * @tc.require: I70I3W
 */

HWTEST_F(SystemAbilityStateSchedulerTest, CanRestartProcessLocked001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::shared_ptr<SystemProcessContext> processContext = std::make_shared<SystemProcessContext>();
    processContext->enableRestart = true;
    bool ret = systemAbilityStateScheduler->CanRestartProcessLocked(processContext);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: CanRestartProcessLocked002
 * @tc.desc: test CanRestartProcessLocked, with enableRestart is false
 * @tc.type: FUNC
 * @tc.require: I736XA
 */

HWTEST_F(SystemAbilityStateSchedulerTest, CanRestartProcessLocked002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::shared_ptr<SystemProcessContext> processContext = std::make_shared<SystemProcessContext>();
    processContext->enableRestart = false;
    bool ret = systemAbilityStateScheduler->CanRestartProcessLocked(processContext);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: CanRestartProcessLocked003
 * @tc.desc: test CanRestartProcessLocked, with restartCountsCtrl size is 4, the time limit is reached
 * @tc.type: FUNC
 * @tc.require: I736XA
 */

HWTEST_F(SystemAbilityStateSchedulerTest, CanRestartProcessLocked003, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::shared_ptr<SystemProcessContext> processContext = std::make_shared<SystemProcessContext>();
    processContext->enableRestart = true;
    int64_t curtime = GetTickCount();
    for (int i = 0; i < RESTART_TIMES_LIMIT; i++) {
        processContext->restartCountsCtrl.push_back(curtime);
    }
    bool ret = systemAbilityStateScheduler->CanRestartProcessLocked(processContext);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: CanRestartProcessLocked004
 * @tc.desc: test CanRestartProcessLocked, with restartCountsCtrl size is 4, the time limit is not reached
 * @tc.type: FUNC
 * @tc.require: I736XA
 */

HWTEST_F(SystemAbilityStateSchedulerTest, CanRestartProcessLocked004, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::shared_ptr<SystemProcessContext> processContext = std::make_shared<SystemProcessContext>();
    processContext->enableRestart = true;
    int64_t curtime = GetTickCount() - RESTART_TIME_INTERVAL_LIMIT;
    for (int i = 0; i < RESTART_TIMES_LIMIT; i++) {
        processContext->restartCountsCtrl.push_back(curtime);
    }
    bool ret = systemAbilityStateScheduler->CanRestartProcessLocked(processContext);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: CanRestartProcessLocked005
 * @tc.desc: test CanRestartProcessLocked, with restartCountsCtrl size is invalid
 * @tc.type: FUNC
 * @tc.require: I736XA
 */

HWTEST_F(SystemAbilityStateSchedulerTest, CanRestartProcessLocked005, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::shared_ptr<SystemProcessContext> processContext = std::make_shared<SystemProcessContext>();
    processContext->enableRestart = true;
    int64_t curtime = GetTickCount();
    for (int i = 0; i <= RESTART_TIMES_LIMIT; i++) {
        processContext->restartCountsCtrl.push_back(curtime);
    }
    bool ret = systemAbilityStateScheduler->CanRestartProcessLocked(processContext);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: HandleAbnormallyDiedAbilityLocked001
 * @tc.desc: test HandleAbnormallyDiedAbilityLocked, abnormallyDiedAbilityList is empty
 * @tc.type: FUNC
 * @tc.require: I736XA
 */

HWTEST_F(SystemAbilityStateSchedulerTest, HandleAbnormallyDiedAbilityLocked001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> abilityContext = std::make_shared<SystemAbilityContext>();
    std::shared_ptr<SystemProcessContext> processContext = std::make_shared<SystemProcessContext>();
    abilityContext->isAutoRestart = true;
    systemAbilityStateScheduler->abilityContextMap_.clear();
    abilityContext->ownProcessContext = processContext;
    abilityContext->systemAbilityId = SAID;
    systemAbilityStateScheduler->abilityContextMap_[SAID] = abilityContext;
    std::list<std::shared_ptr<SystemAbilityContext>> abnormallyDiedAbilityList;
    int32_t ret = systemAbilityStateScheduler->HandleAbnormallyDiedAbilityLocked(
        processContext, abnormallyDiedAbilityList);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: HandleAbnormallyDiedAbilityLocked002
 * @tc.desc: test HandleAbnormallyDiedAbilityLocked, abnormallyDiedAbilityList is not empty
 * @tc.type: FUNC
 * @tc.require: I736XA
 */

HWTEST_F(SystemAbilityStateSchedulerTest, HandleAbnormallyDiedAbilityLocked002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> abilityContext = std::make_shared<SystemAbilityContext>();
    std::shared_ptr<SystemProcessContext> processContext = std::make_shared<SystemProcessContext>();
    abilityContext->isAutoRestart = true;
    systemAbilityStateScheduler->abilityContextMap_.clear();
    abilityContext->ownProcessContext = processContext;
    abilityContext->systemAbilityId = SAID;
    systemAbilityStateScheduler->abilityContextMap_[SAID] = abilityContext;
    std::list<std::shared_ptr<SystemAbilityContext>> abnormallyDiedAbilityList;
    abnormallyDiedAbilityList.emplace_back(abilityContext);

    int32_t ret = systemAbilityStateScheduler->HandleAbnormallyDiedAbilityLocked(
        processContext, abnormallyDiedAbilityList);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: HandleAbilityDiedEvent001
 * @tc.desc: test HandleAbilityDiedEvent, with saId is valid
 * @tc.type: FUNC
 * @tc.require: I736XA
 */

HWTEST_F(SystemAbilityStateSchedulerTest, HandleAbilityDiedEvent001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::shared_ptr<SystemAbilityContext> abilityContext = std::make_shared<SystemAbilityContext>();
    abilityContext->systemAbilityId = SAID;
    int32_t ret = systemAbilityStateScheduler->HandleAbilityDiedEvent(abilityContext->systemAbilityId);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: LimitDelayUnloadTime001
 * @tc.desc: test LimitDelayUnloadTime with delay time is less than 0
 * @tc.type: FUNC
 * @tc.require: I7FBV6
 */

HWTEST_F(SystemAbilityStateSchedulerTest, LimitDelayUnloadTime001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    int32_t ret = systemAbilityStateScheduler->LimitDelayUnloadTime(INVALID_DELAY_TIME);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: LimitDelayUnloadTime002
 * @tc.desc: test LimitDelayUnloadTime with delay time is more than max_delay_time
 * @tc.type: FUNC
 * @tc.require: I7FBV6
 */
HWTEST_F(SystemAbilityStateSchedulerTest, LimitDelayUnloadTime002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    int32_t ret = systemAbilityStateScheduler->LimitDelayUnloadTime(BEYOND_DELAY_TIME_TEST);
    EXPECT_EQ(ret, MAX_DELAY_TIME_TEST);
}

/**
 * @tc.name: UpdateLimitDelayUnloadTimeTask001
 * @tc.desc: test UpdateLimitDelayUnloadTimeTask with saId invalid
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */
HWTEST_F(SystemAbilityStateSchedulerTest, UpdateLimitDelayUnloadTimeTask001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::shared_ptr<SystemAbilityContext> abilityContext = std::make_shared<SystemAbilityContext>();
    abilityContext->systemAbilityId = -1;
    abilityContext->lastStartTime = 0;
    systemAbilityStateScheduler->UpdateLimitDelayUnloadTimeTask(abilityContext->systemAbilityId);
    EXPECT_EQ(abilityContext->lastStartTime, 0);
}

/**
 * @tc.name: UpdateLimitDelayUnloadTimeTask002
 * @tc.desc: test UpdateLimitDelayUnloadTimeTask with lastStartTime not equal to 0
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */
HWTEST_F(SystemAbilityStateSchedulerTest, UpdateLimitDelayUnloadTimeTask002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    int said = 401;
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> abilityContext = std::make_shared<SystemAbilityContext>();
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemAbilityStateScheduler->abilityContextMap_.clear();
    abilityContext->ownProcessContext = systemProcessContext;
    systemAbilityStateScheduler->abilityContextMap_[said] = abilityContext;
    abilityContext->lastStartTime = GetTickCount();
    int32_t delaytime = abilityContext->delayUnloadTime;
    systemAbilityStateScheduler->UpdateLimitDelayUnloadTimeTask(said);
    delaytime += ONCE_DELAY_TIME;
    EXPECT_EQ(delaytime, abilityContext->delayUnloadTime);
}

/**
 * @tc.name: UpdateLimitDelayUnloadTimeTask003
 * @tc.desc: test UpdateLimitDelayUnloadTimeTask with duation greater than duration
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */
HWTEST_F(SystemAbilityStateSchedulerTest, UpdateLimitDelayUnloadTimeTask003, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    int said = 401;
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> abilityContext = std::make_shared<SystemAbilityContext>();
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemAbilityStateScheduler->abilityContextMap_.clear();
    abilityContext->ownProcessContext = systemProcessContext;
    systemAbilityStateScheduler->abilityContextMap_[said] = abilityContext;
    abilityContext->lastStartTime = GetTickCount() - MAX_DURATION;
    int32_t delaytime = abilityContext->delayUnloadTime;
    systemAbilityStateScheduler->UpdateLimitDelayUnloadTimeTask(said);
    EXPECT_EQ(delaytime, abilityContext->delayUnloadTime);
}

/**
 * @tc.name: PostUnloadTimeoutTask001
 * @tc.desc: test PostUnloadTimeoutTask with state is SystemProcessState::STOPPING
 * @tc.type: FUNC
 * @tc.require: I7FBV6
 */

HWTEST_F(SystemAbilityStateSchedulerTest, PostUnloadTimeoutTask001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    systemProcessContext->state = SystemProcessState::STOPPING;
    int32_t ret = systemAbilityStateScheduler->PostUnloadTimeoutTask(systemProcessContext);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: PostUnloadTimeoutTask002
 * @tc.desc: test PostUnloadTimeoutTask with state is SystemProcessState::STARTED
 * @tc.type: FUNC
 * @tc.require: I7FBV6
 */

HWTEST_F(SystemAbilityStateSchedulerTest, PostUnloadTimeoutTask002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    systemProcessContext->state = SystemProcessState::STARTED;
    int32_t ret = systemAbilityStateScheduler->PostUnloadTimeoutTask(systemProcessContext);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: GetAllSystemAbilityInfo001
 * @tc.desc: test GetAllSystemAbilityInfo
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityStateSchedulerTest, GetAllSystemAbilityInfo001, TestSize.Level3)
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
    string result;
    systemAbilityStateScheduler->GetAllSystemAbilityInfo(result);
    EXPECT_NE(result.size(), 0);
}

/**
 * @tc.name: GetSystemAbilityInfo001
 * @tc.desc: test GetSystemAbilityInfo001, GetSystemAbilityContext failed
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityStateSchedulerTest, GetSystemAbilityInfo001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    string result;
    int32_t invalidSaid = 101;
    systemAbilityStateScheduler->GetSystemAbilityInfo(invalidSaid, result);
    EXPECT_EQ(result, "said is not exist");
}

/**
 * @tc.name: GetSystemAbilityInfo002
 * @tc.desc: test GetSystemAbilityInfo001, GetSystemAbilityContext success
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityStateSchedulerTest, GetSystemAbilityInfo002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    int said = 401;
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemAbilityStateScheduler->abilityContextMap_.clear();
    systemAbilityContext->ownProcessContext = systemProcessContext;
    systemAbilityStateScheduler->abilityContextMap_[said] = systemAbilityContext;
    string result;
    systemAbilityStateScheduler->GetSystemAbilityInfo(said, result);
    EXPECT_NE(result, "said is not exist");
}

/**
 * @tc.name: GetProcessInfo001
 * @tc.desc: test GetProcessInfo, GetSystemProcessContext failed
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityStateSchedulerTest, GetProcessInfo001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    string result;
    string processName = "invalid process";
    systemAbilityStateScheduler->GetProcessInfo(processName, result);
    EXPECT_EQ(result, "process is not exist");
}

/**
 * @tc.name: GetProcessInfo002
 * @tc.desc: test GetProcessInfo, GetSystemProcessContext success
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityStateSchedulerTest, GetProcessInfo002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    string processName = "deviceprofile";
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemAbilityStateScheduler->processContextMap_.clear();
    systemAbilityStateScheduler->processContextMap_[Str8ToStr16(processName)] = systemProcessContext;
    string result;
    systemAbilityStateScheduler->GetProcessInfo(processName, result);
    EXPECT_NE(result, "process is not exist");
}

/**
 * @tc.name: GetAllSystemAbilityInfoByState001
 * @tc.desc: test GetAllSystemAbilityInfoByState
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityStateSchedulerTest, GetAllSystemAbilityInfoByState001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    int said = 401;
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemAbilityStateScheduler->abilityContextMap_.clear();
    systemAbilityContext->ownProcessContext = systemProcessContext;
    systemAbilityContext->state = SystemAbilityState::LOADED;
    systemAbilityStateScheduler->abilityContextMap_[said] = systemAbilityContext;
    string result;
    string state = "LOADED";
    systemAbilityStateScheduler->GetAllSystemAbilityInfoByState(state, result);
    EXPECT_NE(result.size(), 0);
}
}