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

#include "gtest/gtest.h"
#include "system_process_status_change_stub.h"
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
constexpr int32_t SAID = 1234;
constexpr int64_t RESTART_TIME_INTERVAL_LIMIT = 20 * 1000;
constexpr int32_t RESTART_TIMES_LIMIT = 4;
constexpr int32_t STATENUMS = 1;
const std::u16string process = u"test";
const std::u16string process_invalid = u"test_invalid";
const std::string LOCAL_DEVICE = "local";
}

class SystemAbilityStateSchedulerProcTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

class SystemProcessNewStatusChange : public SystemProcessStatusChangeStub {
public:
    void OnSystemProcessStarted(SystemProcessInfo& systemProcessInfo) override;
    void OnSystemProcessStopped(SystemProcessInfo& systemProcessInfo) override;
};
void SystemProcessNewStatusChange::OnSystemProcessStarted(SystemProcessInfo& systemProcessInfo)
{
    std::cout << "OnSystemProcessStarted, processName: " << systemProcessInfo.processName << " pid:"
        << systemProcessInfo.pid << " uid:" << systemProcessInfo.uid << std::endl;
}
void SystemProcessNewStatusChange::OnSystemProcessStopped(SystemProcessInfo& systemProcessInfo)
{
    std::cout << "OnSystemProcessStopped, processName: " << systemProcessInfo.processName << " pid:"
        << systemProcessInfo.pid << " uid:" << systemProcessInfo.uid << std::endl;
}

void SystemAbilityStateSchedulerProcTest::SetUpTestCase()
{
    DTEST_LOG << "SetUpTestCase" << std::endl;
}
void SystemAbilityStateSchedulerProcTest::TearDownTestCase()
{
    DTEST_LOG << "TearDownTestCase" << std::endl;
}
void SystemAbilityStateSchedulerProcTest::SetUp()
{
    DTEST_LOG << "SetUp" << std::endl;
}
void SystemAbilityStateSchedulerProcTest::TearDown()
{
    DTEST_LOG << "TearDown" << std::endl;
}

HWTEST_F(SystemAbilityStateSchedulerProcTest, KillSystemProcessLocked002, TestSize.Level3)
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
 * @tc.name: CanRestartProcessLocked001
 * @tc.desc: test CanRestartProcessLocked, with enableRestart is true
 * @tc.type: FUNC
 * @tc.require: I70I3W
 */
HWTEST_F(SystemAbilityStateSchedulerProcTest, CanRestartProcessLocked001, TestSize.Level3)
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
HWTEST_F(SystemAbilityStateSchedulerProcTest, CanRestartProcessLocked002, TestSize.Level3)
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
HWTEST_F(SystemAbilityStateSchedulerProcTest, CanRestartProcessLocked003, TestSize.Level3)
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
HWTEST_F(SystemAbilityStateSchedulerProcTest, CanRestartProcessLocked004, TestSize.Level3)
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
HWTEST_F(SystemAbilityStateSchedulerProcTest, CanRestartProcessLocked005, TestSize.Level3)
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
 * @tc.name: GetProcessInfo001
 * @tc.desc: test GetProcessInfo, GetSystemProcessContext failed
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */
HWTEST_F(SystemAbilityStateSchedulerProcTest, GetProcessInfo001, TestSize.Level3)
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
HWTEST_F(SystemAbilityStateSchedulerProcTest, GetProcessInfo002, TestSize.Level3)
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
 * @tc.name: InitSteteContext001
 * @tc.desc: call InitSteteContext with SaProfiles's process is empty
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */
HWTEST_F(SystemAbilityStateSchedulerProcTest, InitSteteContext001, TestSize.Level3)
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
HWTEST_F(SystemAbilityStateSchedulerProcTest, InitSteteContext002, TestSize.Level3)
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
 * @tc.name: GetSystemProcessContext001
 * @tc.desc: test GetSystemProcessContext with empty processContextMap_
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */
HWTEST_F(SystemAbilityStateSchedulerProcTest, GetSystemProcessContext001, TestSize.Level3)
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
HWTEST_F(SystemAbilityStateSchedulerProcTest, GetSystemProcessContext002, TestSize.Level3)
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
HWTEST_F(SystemAbilityStateSchedulerProcTest, GetSystemProcessContext003, TestSize.Level3)
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
 * @tc.name: HandleLoadAbilityEventLocked002
 * @tc.desc: test HandleLoadAbilityEventLocked, process is stopping
 * @tc.type: FUNC
 * @tc.require: I6LQ18
 */
HWTEST_F(SystemAbilityStateSchedulerProcTest, HandleLoadAbilityEventLocked002, TestSize.Level3)
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
 * @tc.name: SendProcessStateEvent001
 * @tc.desc: test SendProcessStateEvent
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */
HWTEST_F(SystemAbilityStateSchedulerProcTest, SendProcessStateEvent001, TestSize.Level3)
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
 * @tc.name: TryKillSystemProcess002
 * @tc.desc: test TryKillSystemProcess, can kill process
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */
HWTEST_F(SystemAbilityStateSchedulerProcTest, TryKillSystemProcess002, TestSize.Level3)
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
HWTEST_F(SystemAbilityStateSchedulerProcTest, TryKillSystemProcess003, TestSize.Level3)
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
HWTEST_F(SystemAbilityStateSchedulerProcTest, CanKillSystemProcess001, TestSize.Level3)
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
HWTEST_F(SystemAbilityStateSchedulerProcTest, CanKillSystemProcess002, TestSize.Level3)
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
HWTEST_F(SystemAbilityStateSchedulerProcTest, OnProcessStartedLocked001, TestSize.Level3)
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
HWTEST_F(SystemAbilityStateSchedulerProcTest, OnProcessStartedLocked002, TestSize.Level3)
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
HWTEST_F(SystemAbilityStateSchedulerProcTest, OnProcessStartedLocked003, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    sptr<ISystemProcessStatusChange> listener = new SystemProcessNewStatusChange();
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
HWTEST_F(SystemAbilityStateSchedulerProcTest, OnProcessNotStartedLocked001, TestSize.Level3)
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
HWTEST_F(SystemAbilityStateSchedulerProcTest, OnProcessNotStartedLocked002, TestSize.Level3)
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
 * @tc.name: GetSystemProcessInfo003
 * @tc.desc: test GetSystemProcessInfo, valid process info
 * @tc.type: FUNC
 * @tc.require: I7VQQG
 */
HWTEST_F(SystemAbilityStateSchedulerProcTest, GetSystemProcessInfo003, TestSize.Level3)
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
HWTEST_F(SystemAbilityStateSchedulerProcTest, GetRunningSystemProcess001, TestSize.Level3)
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
HWTEST_F(SystemAbilityStateSchedulerProcTest, GetRunningSystemProcess002, TestSize.Level3)
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
HWTEST_F(SystemAbilityStateSchedulerProcTest, GetRunningSystemProcess003, TestSize.Level3)
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
 * @tc.name: ProcessEvent001
 * @tc.desc: test ProcessEvent, event is nullptr
 * @tc.type: FUNC
 * @tc.require: I6OU0A
 */
HWTEST_F(SystemAbilityStateSchedulerProcTest, ProcessEvent001, TestSize.Level3)
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
HWTEST_F(SystemAbilityStateSchedulerProcTest, ProcessEvent002, TestSize.Level3)
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
HWTEST_F(SystemAbilityStateSchedulerProcTest, ProcessEvent003, TestSize.Level3)
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
 * @tc.name: GetSystemProcessInfo001
 * @tc.desc: test GetSystemProcessInfo, systemAbilityContext is nullptr
 * @tc.type: FUNC
 * @tc.require: I7VQQG
 */
HWTEST_F(SystemAbilityStateSchedulerProcTest, GetSystemProcessInfo001, TestSize.Level3)
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
HWTEST_F(SystemAbilityStateSchedulerProcTest, GetSystemProcessInfo002, TestSize.Level3)
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
 * @tc.name: SubscribeSystemProcess001
 * @tc.desc: test SubscribeSystemProcess, listener is not exist in list
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */
HWTEST_F(SystemAbilityStateSchedulerProcTest, SubscribeSystemProcess001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    sptr<SystemProcessNewStatusChange> listener = new SystemProcessNewStatusChange();
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
HWTEST_F(SystemAbilityStateSchedulerProcTest, SubscribeSystemProcess002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    sptr<SystemProcessNewStatusChange> listener = new SystemProcessNewStatusChange();
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
HWTEST_F(SystemAbilityStateSchedulerProcTest, SubscribeSystemProcess003, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    sptr<SystemProcessNewStatusChange> listener = new SystemProcessNewStatusChange();
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
HWTEST_F(SystemAbilityStateSchedulerProcTest, UnSubscribeSystemProcess001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    sptr<SystemProcessNewStatusChange> listener = new SystemProcessNewStatusChange();
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
HWTEST_F(SystemAbilityStateSchedulerProcTest, UnSubscribeSystemProcess002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    sptr<SystemProcessNewStatusChange> listener = new SystemProcessNewStatusChange();
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
HWTEST_F(SystemAbilityStateSchedulerProcTest, UnSubscribeSystemProcess003, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    sptr<SystemProcessNewStatusChange> listener = new SystemProcessNewStatusChange();
    systemAbilityStateScheduler->processListenerDeath_ = nullptr;
    systemAbilityStateScheduler->processListeners_.clear();
    systemAbilityStateScheduler->processListeners_.push_back(listener);
    int32_t ret = systemAbilityStateScheduler->UnSubscribeSystemProcess(listener);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: ProcessDelayUnloadEvent001
 * @tc.desc: test ProcessDelayUnloadEvent, invalid SA
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */
HWTEST_F(SystemAbilityStateSchedulerProcTest, ProcessDelayUnloadEvent001, TestSize.Level3)
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
HWTEST_F(SystemAbilityStateSchedulerProcTest, ProcessDelayUnloadEvent002, TestSize.Level3)
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
HWTEST_F(SystemAbilityStateSchedulerProcTest, ProcessDelayUnloadEvent003, TestSize.Level3)
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
}