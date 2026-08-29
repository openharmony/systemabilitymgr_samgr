/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#include "system_ability_state_scheduler_proc_test.h"
#include "samgr_err_code.h"
#include "ability_death_recipient.h"
#include "datetime_ex.h"
#include "itest_transaction_service.h"
#include "sa_status_change_mock.h"
#include "test_log.h"

#define private public
#define protected public
#include "schedule/system_ability_state_scheduler.h"
#include "system_ability_manager.h"
#include "multi_system_ability_manager.h"

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

class CountingDefaultProcessListener final : public SystemProcessStatusChange {
public:
    void OnSystemProcessStarted(SystemProcessInfo&) override
    {
        ++startedCount_;
    }

    void OnSystemProcessStopped(SystemProcessInfo&) override
    {
        ++stoppedCount_;
    }

    uint32_t startedCount_ = 0;
    uint32_t stoppedCount_ = 0;
};

class NullRemoteDefaultProcessListener final : public SystemProcessStatusChange {
public:
    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }
};

#ifdef SUPPORT_MULTI_INSTANCE
class CountingSystemProcessListener final : public SystemProcessStatusChange {
public:
    void OnSystemProcessStarted(SystemProcessInfo&) override
    {
        ++startedCount_;
    }

    void OnSystemProcessStopped(SystemProcessInfo&) override
    {
        ++stoppedCount_;
    }

    uint32_t startedCount_ = 0;
    uint32_t stoppedCount_ = 0;
};

class NullRemoteSystemProcessListener final : public SystemProcessStatusChange {
public:
    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }
};
#endif
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
        std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
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
        std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
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
        std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
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
        std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
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
        std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
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
        std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
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
        std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
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
        std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
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
        std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
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
        std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
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
        std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
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
        std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
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
        std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
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
        std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
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
        std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
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
        std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
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
        std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
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
        std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
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
        std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
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
        std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
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
        std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
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
        std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    sptr<ISystemProcessStatusChange> listener = new SystemProcessStatusChange();
    systemAbilityStateScheduler->processListeners_.emplace_back(listener);
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
        std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
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
        std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
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
        std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
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
        std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityStateScheduler::UnloadEventHandler> unloadEventHandler =
        std::make_shared<SystemAbilityStateScheduler::UnloadEventHandler>(systemAbilityStateScheduler);
    unloadEventHandler->ProcessEvent(0);
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
        std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateSchedulers = nullptr;
    std::shared_ptr<SystemAbilityStateScheduler::UnloadEventHandler> unloadEventHandler =
        std::make_shared<SystemAbilityStateScheduler::UnloadEventHandler>(systemAbilityStateSchedulers);
    unloadEventHandler->ProcessEvent(0);
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
        std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityStateScheduler::UnloadEventHandler> unloadEventHandler =
        std::make_shared<SystemAbilityStateScheduler::UnloadEventHandler>(systemAbilityStateScheduler);
    unloadEventHandler->ProcessEvent(0);
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    std::list<SystemProcessInfo> systemProcessInfos;
    systemProcessContext->state = SystemProcessState::NOT_STARTED;
    systemAbilityStateScheduler->processContextMap_.clear();
    systemAbilityStateScheduler->processContextMap_[process] = systemProcessContext;
    int32_t ret = systemAbilityStateScheduler->GetRunningSystemProcess(systemProcessInfos);
    EXPECT_EQ(ret, ERR_OK);
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
        std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
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
        std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
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
        std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
    sptr<SystemProcessStatusChange> listener = new SystemProcessStatusChange();
    systemAbilityStateScheduler->processListenerDeath_ =
        sptr<IRemoteObject::DeathRecipient>(
            new SystemProcessListenerDeathRecipient(std::weak_ptr<BaseSystemAbilityManager>{}));
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
        std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
    sptr<SystemProcessStatusChange> listener = new SystemProcessStatusChange();
    systemAbilityStateScheduler->processListenerDeath_ =
        sptr<IRemoteObject::DeathRecipient>(
            new SystemProcessListenerDeathRecipient(std::weak_ptr<BaseSystemAbilityManager>{}));
    systemAbilityStateScheduler->processListeners_.clear();
    systemAbilityStateScheduler->processListeners_.emplace_back(listener);
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
        std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
    sptr<SystemProcessStatusChange> listener = new SystemProcessStatusChange();
    systemAbilityStateScheduler->processListenerDeath_ = nullptr;
    systemAbilityStateScheduler->processListeners_.clear();
    systemAbilityStateScheduler->processListeners_.emplace_back(listener);
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
        std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
    sptr<SystemProcessStatusChange> listener = new SystemProcessStatusChange();
    systemAbilityStateScheduler->processListenerDeath_ =
        sptr<IRemoteObject::DeathRecipient>(
            new SystemProcessListenerDeathRecipient(std::weak_ptr<BaseSystemAbilityManager>{}));
    systemAbilityStateScheduler->processListeners_.clear();
    systemAbilityStateScheduler->processListeners_.emplace_back(listener);
    int32_t ret = systemAbilityStateScheduler->UnSubscribeSystemProcess(listener, false);
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
        std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
    sptr<SystemProcessStatusChange> listener = new SystemProcessStatusChange();
    systemAbilityStateScheduler->processListenerDeath_ =
        sptr<IRemoteObject::DeathRecipient>(
            new SystemProcessListenerDeathRecipient(std::weak_ptr<BaseSystemAbilityManager>{}));
    systemAbilityStateScheduler->processListeners_.clear();
    int32_t ret = systemAbilityStateScheduler->UnSubscribeSystemProcess(listener, false);
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
        std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
    sptr<SystemProcessStatusChange> listener = new SystemProcessStatusChange();
    systemAbilityStateScheduler->processListenerDeath_ = nullptr;
    systemAbilityStateScheduler->processListeners_.clear();
    systemAbilityStateScheduler->processListeners_.emplace_back(listener);
    int32_t ret = systemAbilityStateScheduler->UnSubscribeSystemProcess(listener, false);
    EXPECT_EQ(ret, ERR_OK);
}

HWTEST_F(SystemAbilityStateSchedulerProcTest, ProcessListenerInvalidInput001, TestSize.Level3)
{
    auto scheduler = std::make_shared<SystemAbilityStateScheduler>(std::weak_ptr<BaseSystemAbilityManager>{});
    sptr<ISystemProcessStatusChange> nullListener;
    EXPECT_EQ(scheduler->SubscribeSystemProcess(nullListener, false), ERR_INVALID_VALUE);
    EXPECT_EQ(scheduler->UnSubscribeSystemProcess(nullListener, false), ERR_INVALID_VALUE);
    scheduler->UnSubscribeSystemProcess(sptr<IRemoteObject>());
    EXPECT_TRUE(scheduler->processListeners_.empty());
}

HWTEST_F(SystemAbilityStateSchedulerProcTest, ProcessListenerRemoteRemoval001, TestSize.Level3)
{
    auto scheduler = std::make_shared<SystemAbilityStateScheduler>(std::weak_ptr<BaseSystemAbilityManager>{});
    sptr<SystemProcessStatusChange> listener = new SystemProcessStatusChange();
    ASSERT_EQ(scheduler->SubscribeSystemProcess(listener, false), ERR_OK);
    scheduler->UnSubscribeSystemProcess(listener->AsObject());
    EXPECT_TRUE(scheduler->processListeners_.empty());
}

HWTEST_F(SystemAbilityStateSchedulerProcTest, ProcessListenerSubscriptionSources001, TestSize.Level3)
{
    auto scheduler = std::make_shared<SystemAbilityStateScheduler>(std::weak_ptr<BaseSystemAbilityManager>{});
    sptr<CountingDefaultProcessListener> listener = new CountingDefaultProcessListener();
    ASSERT_NE(scheduler, nullptr);
    ASSERT_NE(listener, nullptr);

    EXPECT_EQ(scheduler->SubscribeSystemProcess(listener, true), ERR_OK);
    EXPECT_EQ(scheduler->SubscribeSystemProcess(listener, true), ERR_OK);
    EXPECT_EQ(scheduler->SubscribeSystemProcess(listener, false), ERR_OK);
    ASSERT_EQ(scheduler->processListeners_.size(), 1U);
    EXPECT_TRUE(scheduler->processListeners_.front().hasForegroundSubscription);
    EXPECT_TRUE(scheduler->processListeners_.front().hasDirectSubscription);

    auto processContext = std::make_shared<SystemProcessContext>();
    processContext->processName = process;
    scheduler->NotifyProcessStarted(processContext);
    scheduler->NotifyProcessStopped(processContext);
    EXPECT_EQ(listener->startedCount_, 1U);
    EXPECT_EQ(listener->stoppedCount_, 1U);

    EXPECT_EQ(scheduler->UnSubscribeSystemProcess(listener, true), ERR_OK);
    ASSERT_EQ(scheduler->processListeners_.size(), 1U);
    EXPECT_FALSE(scheduler->processListeners_.front().hasForegroundSubscription);
    EXPECT_TRUE(scheduler->processListeners_.front().hasDirectSubscription);
    EXPECT_EQ(scheduler->UnSubscribeSystemProcess(listener, false), ERR_OK);
    EXPECT_TRUE(scheduler->processListeners_.empty());
}

HWTEST_F(SystemAbilityStateSchedulerProcTest, ProcessListenerNullEntryRemoval001, TestSize.Level3)
{
    auto scheduler = std::make_shared<SystemAbilityStateScheduler>(std::weak_ptr<BaseSystemAbilityManager>{});
    sptr<CountingDefaultProcessListener> listener = new CountingDefaultProcessListener();
    sptr<CountingDefaultProcessListener> inactiveListener = new CountingDefaultProcessListener();
    sptr<ISystemProcessStatusChange> nullRemoteListener = new NullRemoteDefaultProcessListener();
    ASSERT_NE(scheduler, nullptr);
    ASSERT_NE(listener, nullptr);
    ASSERT_NE(inactiveListener, nullptr);
    ASSERT_NE(nullRemoteListener, nullptr);
    scheduler->processListeners_.emplace_back(nullptr);
    scheduler->processListeners_.emplace_back(nullRemoteListener);
    scheduler->processListeners_.emplace_back(listener, true);
    scheduler->processListeners_.emplace_back(inactiveListener);
    scheduler->processListeners_.back().hasDirectSubscription = false;

    auto processContext = std::make_shared<SystemProcessContext>();
    processContext->processName = process;
    scheduler->NotifyProcessStarted(processContext);
    scheduler->NotifyProcessStopped(processContext);
    EXPECT_EQ(listener->startedCount_, 1U);
    EXPECT_EQ(listener->stoppedCount_, 1U);
    EXPECT_EQ(inactiveListener->startedCount_, 1U);
    EXPECT_EQ(inactiveListener->stoppedCount_, 1U);

    scheduler->UnSubscribeSystemProcess(listener->AsObject());
    ASSERT_EQ(scheduler->processListeners_.size(), 2U);
    scheduler->processListeners_.remove_if([](const auto& item) {
        return item.listener == nullptr || item.listener->AsObject() == nullptr;
    });
    ASSERT_EQ(scheduler->processListeners_.size(), 1U);
    EXPECT_EQ(scheduler->processListeners_.front().listener->AsObject(), inactiveListener->AsObject());
}

HWTEST_F(SystemAbilityStateSchedulerProcTest, ProcessListenerNullRemote001, TestSize.Level3)
{
    auto scheduler = std::make_shared<SystemAbilityStateScheduler>(std::weak_ptr<BaseSystemAbilityManager>{});
    sptr<ISystemProcessStatusChange> listener = new NullRemoteDefaultProcessListener();
    ASSERT_NE(scheduler, nullptr);
    ASSERT_NE(listener, nullptr);
    EXPECT_EQ(scheduler->SubscribeSystemProcess(listener, false), ERR_INVALID_VALUE);
    EXPECT_EQ(scheduler->UnSubscribeSystemProcess(listener, false), ERR_INVALID_VALUE);
    EXPECT_TRUE(scheduler->processListeners_.empty());
}

HWTEST_F(SystemAbilityStateSchedulerProcTest, ProcessPendingLoadOverflowDefault001, TestSize.Level3)
{
    auto abilityContext = std::make_shared<SystemAbilityContext>();
    auto processContext = std::make_shared<SystemProcessContext>();
    abilityContext->ownProcessContext = processContext;
    abilityContext->pendingLoadEventCountMap[SAID] = 1;
    abilityContext->pendingLoadEventList.emplace_back();
    processContext->processName = process_invalid;

    auto expiredScheduler =
        std::make_shared<SystemAbilityStateScheduler>(std::weak_ptr<BaseSystemAbilityManager>{});
    expiredScheduler->ProcessPendingLoadOverflow(
        abilityContext, processContext, std::weak_ptr<BaseSystemAbilityManager>{});
    EXPECT_EQ(abilityContext->pendingLoadEventList.size(), 1U);
    EXPECT_FALSE(expiredScheduler->CheckProcessStarted(process));

    auto manager = std::make_shared<BaseSystemAbilityManager>();
    auto scheduler = std::make_shared<SystemAbilityStateScheduler>(manager);
    processContext->state = SystemProcessState::STARTED;
    scheduler->ProcessPendingLoadOverflow(abilityContext, processContext, manager);
    EXPECT_EQ(abilityContext->pendingLoadEventCountMap.size(), 1U);

    processContext->state = SystemProcessState::STOPPING;
    scheduler->ProcessPendingLoadOverflow(abilityContext, processContext, manager);
    EXPECT_TRUE(abilityContext->pendingLoadEventCountMap.empty());
    EXPECT_TRUE(abilityContext->pendingLoadEventList.empty());
    EXPECT_EQ(scheduler->GetUserId(), BASE_USER);
    EXPECT_EQ(expiredScheduler->GetUserId(), SAMGR_INVALID_USER_ID);
}

HWTEST_F(SystemAbilityStateSchedulerProcTest, ProcessPendingLoadOverflowRemoveProcess001, TestSize.Level3)
{
    auto manager = std::make_shared<BaseSystemAbilityManager>();
    auto scheduler = std::make_shared<SystemAbilityStateScheduler>(manager);
    manager->abilityStateScheduler_ = scheduler;
    sptr<IRemoteObject> processObject = new TestTransactionService();
    manager->systemProcessMap_[process_invalid] = processObject;
    auto abilityContext = std::make_shared<SystemAbilityContext>();
    auto processContext = std::make_shared<SystemProcessContext>();
    abilityContext->ownProcessContext = processContext;
    abilityContext->pendingLoadEventCountMap[SAID] = 1;
    abilityContext->pendingLoadEventList.emplace_back();
    processContext->processName = process_invalid;
    processContext->state = SystemProcessState::STOPPING;

    scheduler->ProcessPendingLoadOverflow(abilityContext, processContext, manager);
    EXPECT_TRUE(abilityContext->pendingLoadEventCountMap.empty());
    EXPECT_TRUE(abilityContext->pendingLoadEventList.empty());
    EXPECT_EQ(manager->GetSystemProcess(process_invalid), nullptr);
}

HWTEST_F(SystemAbilityStateSchedulerProcTest, HandlePendingLoadOverflowDefault001, TestSize.Level3)
{
    auto abilityContext = std::make_shared<SystemAbilityContext>();
    abilityContext->ownProcessContext = std::make_shared<SystemProcessContext>();
    abilityContext->ownProcessContext->processName = process_invalid;
    abilityContext->ownProcessContext->state = SystemProcessState::STARTED;
    auto expiredScheduler =
        std::make_shared<SystemAbilityStateScheduler>(std::weak_ptr<BaseSystemAbilityManager>{});
    EXPECT_EQ(expiredScheduler->HandlePendingLoadOverflow(abilityContext), ERR_INVALID_VALUE);

    auto manager = std::make_shared<BaseSystemAbilityManager>();
    auto scheduler = std::make_shared<SystemAbilityStateScheduler>(manager);
    EXPECT_EQ(scheduler->HandlePendingLoadOverflow(abilityContext), PEND_LOAD_EVENT_SIZE_LIMIT);
    ASSERT_NE(scheduler->recoverHandler_, nullptr);
    EXPECT_EQ(abilityContext->ownProcessContext->state, SystemProcessState::STARTED);
    scheduler->recoverHandler_->CleanFfrt();
}

#ifdef SUPPORT_MULTI_INSTANCE
/**
 * @tc.name: ProcessListenerSubscriptionSource001
 * @tc.desc: Test foreground and direct process subscriptions retain independent ownership
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityStateSchedulerProcTest, ProcessListenerSubscriptionSource001, TestSize.Level3)
{
    auto scheduler = std::make_shared<SystemAbilityStateScheduler>(std::weak_ptr<BaseSystemAbilityManager>{});
    sptr<CountingSystemProcessListener> listener = new CountingSystemProcessListener();

    EXPECT_EQ(scheduler->SubscribeSystemProcess(listener, true), ERR_OK);
    EXPECT_EQ(scheduler->SubscribeSystemProcess(listener, false), ERR_OK);
    EXPECT_EQ(scheduler->SubscribeSystemProcess(listener, true), ERR_OK);
    ASSERT_EQ(scheduler->processListeners_.size(), 1U);
    EXPECT_TRUE(scheduler->processListeners_.front().hasForegroundSubscription);
    EXPECT_TRUE(scheduler->processListeners_.front().hasDirectSubscription);
    sptr<ISystemProcessStatusChange> processListener = listener;
    EXPECT_EQ(scheduler->UnSubscribeSystemProcess(processListener), ERR_OK);
    ASSERT_EQ(scheduler->processListeners_.size(), 1U);
    EXPECT_TRUE(scheduler->processListeners_.front().hasForegroundSubscription);
    EXPECT_FALSE(scheduler->processListeners_.front().hasDirectSubscription);
    EXPECT_EQ(scheduler->UnSubscribeSystemProcess(listener, true), ERR_OK);
    EXPECT_TRUE(scheduler->processListeners_.empty());
}

/**
 * @tc.name: ProcessListenerSubscriptionSource002
 * @tc.desc: Test non-foreground users skip foreground-only process listeners
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityStateSchedulerProcTest, ProcessListenerSubscriptionSource002, TestSize.Level3)
{
    auto scheduler = std::make_shared<SystemAbilityStateScheduler>(std::weak_ptr<BaseSystemAbilityManager>{});
    sptr<CountingSystemProcessListener> foregroundListener = new CountingSystemProcessListener();
    sptr<CountingSystemProcessListener> directListener = new CountingSystemProcessListener();
    auto processContext = std::make_shared<SystemProcessContext>();
    processContext->processName = process;

    ASSERT_EQ(scheduler->SubscribeSystemProcess(foregroundListener, true), ERR_OK);
    ASSERT_EQ(scheduler->SubscribeSystemProcess(directListener, false), ERR_OK);
    scheduler->NotifyProcessStarted(processContext);
    scheduler->NotifyProcessStopped(processContext);
    EXPECT_EQ(foregroundListener->startedCount_, 0U);
    EXPECT_EQ(foregroundListener->stoppedCount_, 0U);
    EXPECT_EQ(directListener->startedCount_, 1U);
    EXPECT_EQ(directListener->stoppedCount_, 1U);
    scheduler->UnSubscribeSystemProcess(directListener->AsObject());
    ASSERT_EQ(scheduler->processListeners_.size(), 1U);
    EXPECT_EQ(scheduler->processListeners_.front().listener->AsObject(), foregroundListener->AsObject());
}
#endif

#ifdef SUPPORT_MULTI_INSTANCE
/**
 * @tc.name: ProcessPendingLoadOverflow001
 * @tc.desc: Test pending-load overflow returns safely when the manager has expired.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityStateSchedulerProcTest, ProcessPendingLoadOverflow001, TestSize.Level3)
{
    auto scheduler = std::make_shared<SystemAbilityStateScheduler>(std::weak_ptr<BaseSystemAbilityManager>{});
    auto abilityContext = std::make_shared<SystemAbilityContext>();
    auto processContext = std::make_shared<SystemProcessContext>();
    abilityContext->ownProcessContext = processContext;

    scheduler->ProcessPendingLoadOverflow(abilityContext, processContext, std::weak_ptr<BaseSystemAbilityManager>{});
    EXPECT_FALSE(scheduler->CheckProcessStarted(process));
#ifdef SUPPORT_MULTI_INSTANCE
    EXPECT_EQ(scheduler->ServiceControl("test", ServiceAction::STOP), ERR_INVALID_VALUE);
#endif
}

/**
 * @tc.name: ProcessPendingLoadOverflow002
 * @tc.desc: Test pending-load events remain unchanged while the process is not stopping.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityStateSchedulerProcTest, ProcessPendingLoadOverflow002, TestSize.Level3)
{
    auto manager = std::make_shared<MultiSystemAbilityManager>(100);
    auto scheduler = std::make_shared<SystemAbilityStateScheduler>(manager);
    auto abilityContext = std::make_shared<SystemAbilityContext>();
    auto processContext = std::make_shared<SystemProcessContext>();
    abilityContext->ownProcessContext = processContext;
    abilityContext->pendingLoadEventCountMap[SAID] = 1;
    abilityContext->pendingLoadEventList.emplace_back();
    processContext->processName = process;
    processContext->state = SystemProcessState::STARTED;

    scheduler->ProcessPendingLoadOverflow(abilityContext, processContext, manager);
    EXPECT_EQ(abilityContext->pendingLoadEventCountMap.size(), 1U);
    EXPECT_EQ(abilityContext->pendingLoadEventList.size(), 1U);
}

/**
 * @tc.name: ProcessPendingLoadOverflow003
 * @tc.desc: Test pending-load events are cleared before a missing stopping process is removed.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityStateSchedulerProcTest, ProcessPendingLoadOverflow003, TestSize.Level3)
{
    auto manager = std::make_shared<MultiSystemAbilityManager>(100);
    auto scheduler = std::make_shared<SystemAbilityStateScheduler>(manager);
    auto abilityContext = std::make_shared<SystemAbilityContext>();
    auto processContext = std::make_shared<SystemProcessContext>();
    abilityContext->ownProcessContext = processContext;
    abilityContext->pendingLoadEventCountMap[SAID] = 1;
    abilityContext->pendingLoadEventList.emplace_back();
    processContext->processName = process_invalid;
    processContext->state = SystemProcessState::STOPPING;

    scheduler->ProcessPendingLoadOverflow(abilityContext, processContext, manager);
    EXPECT_TRUE(abilityContext->pendingLoadEventCountMap.empty());
    EXPECT_TRUE(abilityContext->pendingLoadEventList.empty());
}

/**
 * @tc.name: ProcessListenerSubscriptionSource003
 * @tc.desc: Test process subscription APIs reject null listeners and remotes safely.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityStateSchedulerProcTest, ProcessListenerSubscriptionSource003, TestSize.Level3)
{
    auto scheduler = std::make_shared<SystemAbilityStateScheduler>(std::weak_ptr<BaseSystemAbilityManager>{});
    sptr<ISystemProcessStatusChange> listener;
    EXPECT_EQ(scheduler->SubscribeSystemProcess(listener, true), ERR_INVALID_VALUE);
    EXPECT_EQ(scheduler->UnSubscribeSystemProcess(listener), ERR_INVALID_VALUE);
    scheduler->UnSubscribeSystemProcess(sptr<IRemoteObject>());
}

/**
 * @tc.name: ServiceControlByUser001
 * @tc.desc: Test service-control dispatches to base and non-base user paths.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityStateSchedulerProcTest, ServiceControlByUser001, TestSize.Level3)
{
    constexpr char TEST_PROCESS[] = "samgr_ut_missing_process";
    auto multiUserManager = std::make_shared<MultiSystemAbilityManager>(100);
    auto multiUserScheduler = std::make_shared<SystemAbilityStateScheduler>(multiUserManager);
    EXPECT_EQ(multiUserScheduler->GetUserId(), 100);
    (void)multiUserScheduler->ServiceControl(TEST_PROCESS, ServiceAction::STOP);

    auto baseUserManager = std::make_shared<MultiSystemAbilityManager>(BASE_USER);
    auto baseUserScheduler = std::make_shared<SystemAbilityStateScheduler>(baseUserManager);
    EXPECT_EQ(baseUserScheduler->GetUserId(), BASE_USER);
    (void)baseUserScheduler->ServiceControl(TEST_PROCESS, ServiceAction::STOP);
}

/**
 * @tc.name: SchedulerExpiredManagerUser001
 * @tc.desc: Verify an expired scheduler manager returns the invalid user marker.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityStateSchedulerProcTest, SchedulerExpiredManagerUser001, TestSize.Level3)
{
    auto scheduler = std::make_shared<SystemAbilityStateScheduler>(std::weak_ptr<BaseSystemAbilityManager>{});
    ASSERT_NE(scheduler, nullptr);
    EXPECT_EQ(scheduler->GetUserId(), SAMGR_INVALID_USER_ID);
}

/**
 * @tc.name: SchedulerExpiredManagerServiceControl001
 * @tc.desc: Verify service control rejects requests when the manager has expired.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityStateSchedulerProcTest, SchedulerExpiredManagerServiceControl001, TestSize.Level3)
{
    auto scheduler = std::make_shared<SystemAbilityStateScheduler>(std::weak_ptr<BaseSystemAbilityManager>{});
    ASSERT_NE(scheduler, nullptr);
    EXPECT_EQ(scheduler->ServiceControl("samgr_missing_process", ServiceAction::STOP), ERR_INVALID_VALUE);
}

/**
 * @tc.name: ProcessListenerBaseUserBranches004
 * @tc.desc: Verify dual subscription sources notify and unsubscribe independently.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityStateSchedulerProcTest, ProcessListenerBaseUserBranches004, TestSize.Level3)
{
    auto manager = std::make_shared<MultiSystemAbilityManager>(BASE_USER);
    auto scheduler = std::make_shared<SystemAbilityStateScheduler>(manager);
    sptr<CountingSystemProcessListener> listener = new CountingSystemProcessListener();
    auto processContext = std::make_shared<SystemProcessContext>();
    ASSERT_NE(scheduler, nullptr);
    ASSERT_NE(listener, nullptr);
    processContext->processName = process;
    EXPECT_EQ(scheduler->SubscribeSystemProcess(listener, true), ERR_OK);
    EXPECT_EQ(scheduler->SubscribeSystemProcess(listener, false), ERR_OK);
    ASSERT_EQ(scheduler->processListeners_.size(), 1U);
    EXPECT_TRUE(scheduler->processListeners_.front().hasDirectSubscription);
    EXPECT_TRUE(scheduler->processListeners_.front().hasForegroundSubscription);
    scheduler->NotifyProcessStarted(processContext);
    scheduler->NotifyProcessStopped(processContext);
    EXPECT_EQ(listener->startedCount_, 1U);
    EXPECT_EQ(listener->stoppedCount_, 1U);
    EXPECT_EQ(scheduler->UnSubscribeSystemProcess(listener, true), ERR_OK);
    EXPECT_EQ(scheduler->processListeners_.size(), 1U);
    EXPECT_EQ(scheduler->UnSubscribeSystemProcess(listener, false), ERR_OK);
    EXPECT_TRUE(scheduler->processListeners_.empty());
}

/**
 * @tc.name: ProcessListenerForegroundUser005
 * @tc.desc: Verify a foreground-only listener receives current-user notifications.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityStateSchedulerProcTest, ProcessListenerForegroundUser005, TestSize.Level3)
{
    constexpr int32_t userId = 126;
    auto manager = std::make_shared<MultiSystemAbilityManager>(userId);
    auto scheduler = std::make_shared<SystemAbilityStateScheduler>(manager);
    sptr<CountingSystemProcessListener> listener = new CountingSystemProcessListener();
    sptr<SystemAbilityManager> samgr = SystemAbilityManager::GetInstance();
    auto processContext = std::make_shared<SystemProcessContext>();
    ASSERT_NE(scheduler, nullptr);
    ASSERT_NE(samgr, nullptr);
    const int32_t previousUserId = samgr->userLifecycleManager_.GetForegroundUserId();
    samgr->userLifecycleManager_.foregroundUserId_.store(userId);
    EXPECT_EQ(scheduler->SubscribeSystemProcess(listener, true), ERR_OK);
    scheduler->NotifyProcessStarted(processContext);
    scheduler->NotifyProcessStopped(processContext);
    samgr->userLifecycleManager_.foregroundUserId_.store(previousUserId);
    EXPECT_EQ(listener->startedCount_, 1U);
    EXPECT_EQ(listener->stoppedCount_, 1U);
}

/**
 * @tc.name: ProcessListenerInvalidRemote006
 * @tc.desc: Verify null remote listeners and stale list entries are rejected or removed.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityStateSchedulerProcTest, ProcessListenerInvalidRemote006, TestSize.Level3)
{
    auto scheduler = std::make_shared<SystemAbilityStateScheduler>(std::weak_ptr<BaseSystemAbilityManager>{});
    sptr<ISystemProcessStatusChange> nullRemoteListener = new NullRemoteSystemProcessListener();
    sptr<CountingSystemProcessListener> validListener = new CountingSystemProcessListener();
    ASSERT_NE(scheduler, nullptr);
    ASSERT_NE(nullRemoteListener, nullptr);
    ASSERT_NE(validListener, nullptr);
    EXPECT_EQ(scheduler->SubscribeSystemProcess(nullRemoteListener, true), ERR_INVALID_VALUE);
    EXPECT_EQ(scheduler->UnSubscribeSystemProcess(nullRemoteListener, true), ERR_INVALID_VALUE);
    scheduler->processListeners_.emplace_back(nullptr);
    scheduler->processListeners_.emplace_back(validListener);

    scheduler->UnSubscribeSystemProcess(validListener->AsObject());
    EXPECT_TRUE(scheduler->processListeners_.empty());
}
#endif

/**
 * @tc.name: ProcessDelayUnloadEvent001
 * @tc.desc: test ProcessDelayUnloadEvent, invalid SA
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */
HWTEST_F(SystemAbilityStateSchedulerProcTest, ProcessDelayUnloadEvent001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
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
        std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
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
        std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
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
 * @tc.name: SendProcessStateEvent002
 * @tc.desc: test SendProcessStateEvent
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityStateSchedulerProcTest, SendProcessStateEvent002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> scheduler = std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
    std::list<SaProfile> saProfiles;
    SaProfile saProfile = {u"test", SAID};
    saProfiles.emplace_back(saProfile);
    scheduler->Init(saProfiles);
    ProcessInfo processInfo = {u"test"};
    auto ret = scheduler->SendProcessStateEvent(processInfo, ProcessStateEvent::PROCESS_STARTED_EVENT);
    EXPECT_EQ(ret, ERR_OK);
    std::shared_ptr<SystemProcessContext> processContext;
    scheduler->GetSystemProcessContext(processInfo.processName, processContext);
    scheduler->AddRunningProcessLocked(processContext);
    ret = scheduler->SendProcessStateEvent(processInfo, ProcessStateEvent::PROCESS_STARTED_EVENT);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ret = scheduler->SendProcessStateEvent(processInfo, ProcessStateEvent::PROCESS_STOPPED_EVENT);
    scheduler->RemoveRunningProcessLocked(processContext);
    EXPECT_EQ(ret, ERR_OK);
}
}
