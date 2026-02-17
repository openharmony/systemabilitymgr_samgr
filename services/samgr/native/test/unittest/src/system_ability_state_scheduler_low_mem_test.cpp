/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "system_ability_state_scheduler_low_mem_test.h"
#include "ability_death_recipient.h"
#include "datetime_ex.h"
#include "ipc_skeleton.h"
#include "sa_profiles.h"
#include "sa_status_change_mock.h"
#include "samgr_err_code.h"
#include "string_ex.h"
#include "system_process_status_change_proxy.h"
#include "system_process_status_change_stub.h"
#include "test_log.h"

#define private public
#include "schedule/system_ability_state_scheduler.h"
#include "system_ability_manager.h"

using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS;

namespace {
constexpr int32_t SAID = 1234;
constexpr int32_t MAX_SUBSCRIBE_COUNT = 256;

class SystemProcessStatusChange : public SystemProcessStatusChangeStub {
public:
    void OnSystemProcessStarted(SystemProcessInfo& systemProcessInfo) override
    {
        std::cout << "OnSystemProcessStarted, processName: " << systemProcessInfo.processName
                  << " pid:" << systemProcessInfo.pid << " uid:" << systemProcessInfo.uid << std::endl;
    }

    void OnSystemProcessStopped(SystemProcessInfo& systemProcessInfo) override
    {
        std::cout << "OnSystemProcessStopped, processName: " << systemProcessInfo.processName
                  << " pid:" << systemProcessInfo.pid << " uid:" << systemProcessInfo.uid << std::endl;
    }

    void OnSystemProcessActivated(SystemProcessInfo& systemProcessInfo) override
    {
        std::cout << "OnSystemProcessActivated, processName: " << systemProcessInfo.processName
                  << " pid:" << systemProcessInfo.pid << " uid:" << systemProcessInfo.uid << std::endl;
        lastEvent_ = "Activated";
    }

    void OnSystemProcessIdled(SystemProcessInfo& systemProcessInfo) override
    {
        std::cout << "OnSystemProcessIdled, processName: " << systemProcessInfo.processName
                  << " pid:" << systemProcessInfo.pid << " uid:" << systemProcessInfo.uid << std::endl;
        lastEvent_ = "Idled";
    }
    std::string lastEvent_{};
};
} // namespace

void SystemAbilityStateSchedulerLowMemTest::SetUpTestCase()
{
    DTEST_LOG << "SetUpTestCase" << std::endl;
}

void SystemAbilityStateSchedulerLowMemTest::TearDownTestCase()
{
    DTEST_LOG << "TearDownTestCase" << std::endl;
}

void SystemAbilityStateSchedulerLowMemTest::SetUp()
{
    DTEST_LOG << "SetUp" << std::endl;
}

void SystemAbilityStateSchedulerLowMemTest::TearDown()
{
    DTEST_LOG << "TearDown" << std::endl;
}

HWTEST_F(SystemAbilityStateSchedulerLowMemTest, InitLowMemProcessList001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    systemAbilityStateScheduler->processContextMap_.clear();
    systemAbilityStateScheduler->lowMemoryProcessList_.clear();
    systemAbilityStateScheduler->InitLowMemProcessList(saProfiles);
    EXPECT_TRUE(systemAbilityStateScheduler->lowMemoryProcessList_.empty());
}

HWTEST_F(SystemAbilityStateSchedulerLowMemTest, InitLowMemProcessList002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    SaProfile lowMemProf{};
    lowMemProf.process = u"test_process";
    lowMemProf.recycleStrategy = LOW_MEMORY;
    saProfiles.push_back(lowMemProf);

    systemAbilityStateScheduler->Init(saProfiles);
    systemAbilityStateScheduler->processContextMap_.clear();

    auto processContext = std::make_shared<SystemProcessContext>();
    processContext->processName = u"test_process";
    systemAbilityStateScheduler->processContextMap_[u"test_process"] = processContext;
    systemAbilityStateScheduler->lowMemoryProcessList_.clear();

    systemAbilityStateScheduler->InitLowMemProcessList(saProfiles);
    EXPECT_FALSE(systemAbilityStateScheduler->lowMemoryProcessList_.empty());

    sptr<SystemProcessStatusChange> listenerStub = new SystemProcessStatusChange();
    sptr<SystemProcessStatusChangeProxy> listenerProxy = new SystemProcessStatusChangeProxy{listenerStub};
    systemAbilityStateScheduler->SubscribeLowMemSystemProcess(listenerProxy);
    systemAbilityStateScheduler->NotifyProcessActivated(processContext);
    EXPECT_EQ(listenerStub->lastEvent_, "Activated");
    listenerStub->lastEvent_.clear();
    systemAbilityStateScheduler->NotifyProcessIdled(processContext);
    EXPECT_EQ(listenerStub->lastEvent_, "Idled");
}

HWTEST_F(SystemAbilityStateSchedulerLowMemTest, InitLowMemProcessList003, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    systemAbilityStateScheduler->processContextMap_.clear();

    systemAbilityStateScheduler->processContextMap_[u"test_process"] = nullptr;
    systemAbilityStateScheduler->lowMemoryProcessList_.clear();

    systemAbilityStateScheduler->InitLowMemProcessList(saProfiles);
    EXPECT_TRUE(systemAbilityStateScheduler->lowMemoryProcessList_.empty());
}

HWTEST_F(SystemAbilityStateSchedulerLowMemTest, IsProcessActivatedLocked001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);

    auto processContext = std::make_shared<SystemProcessContext>();
    processContext->processName = u"test_process";
    processContext->abilityStateCountMap[SystemAbilityState::LOADED] = 1;

    bool ret = systemAbilityStateScheduler->IsProcessActivatedLocked(processContext);
    EXPECT_FALSE(ret);
}

HWTEST_F(SystemAbilityStateSchedulerLowMemTest, IsProcessActivatedLocked002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);

    auto processContext = std::make_shared<SystemProcessContext>();
    processContext->processName = u"test_process";
    processContext->abilityStateCountMap[SystemAbilityState::LOADED] = 1;

    sptr<ISystemProcessStatusChange> listener = new SystemProcessStatusChange();
    systemAbilityStateScheduler->procListenerMap_[u"test_process"].push_back(listener);

    bool ret = systemAbilityStateScheduler->IsProcessActivatedLocked(processContext);
    EXPECT_TRUE(ret);
}

HWTEST_F(SystemAbilityStateSchedulerLowMemTest, IsProcessActivatedLocked003, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);

    auto processContext = std::make_shared<SystemProcessContext>();
    processContext->processName = u"test_process";
    processContext->abilityStateCountMap[SystemAbilityState::LOADED] = 2;

    sptr<ISystemProcessStatusChange> listener = new SystemProcessStatusChange();
    systemAbilityStateScheduler->procListenerMap_[u"test_process"].push_back(listener);

    bool ret = systemAbilityStateScheduler->IsProcessActivatedLocked(processContext);
    EXPECT_FALSE(ret);
}

HWTEST_F(SystemAbilityStateSchedulerLowMemTest, IsProcessIdledLocked001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);

    auto processContext = std::make_shared<SystemProcessContext>();
    processContext->processName = u"test_process";
    processContext->abilityStateCountMap[SystemAbilityState::LOADED] = 0;
    processContext->abilityStateCountMap[SystemAbilityState::LOADING] = 0;

    bool ret = systemAbilityStateScheduler->IsProcessIdledLocked(processContext);
    EXPECT_FALSE(ret);
}

HWTEST_F(SystemAbilityStateSchedulerLowMemTest, IsProcessIdledLocked002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);

    auto processContext = std::make_shared<SystemProcessContext>();
    processContext->processName = u"test_process";
    processContext->abilityStateCountMap[SystemAbilityState::LOADED] = 0;
    processContext->abilityStateCountMap[SystemAbilityState::LOADING] = 0;

    sptr<ISystemProcessStatusChange> listener = new SystemProcessStatusChange();
    systemAbilityStateScheduler->procListenerMap_[u"test_process"].push_back(listener);

    bool ret = systemAbilityStateScheduler->IsProcessIdledLocked(processContext);
    EXPECT_TRUE(ret);
}

HWTEST_F(SystemAbilityStateSchedulerLowMemTest, IsProcessIdledLocked003, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);

    auto processContext = std::make_shared<SystemProcessContext>();
    processContext->processName = u"test_process";
    processContext->abilityStateCountMap[SystemAbilityState::LOADED] = 1;
    processContext->abilityStateCountMap[SystemAbilityState::LOADING] = 0;

    sptr<ISystemProcessStatusChange> listener = new SystemProcessStatusChange();
    systemAbilityStateScheduler->procListenerMap_[u"test_process"].push_back(listener);

    bool ret = systemAbilityStateScheduler->IsProcessIdledLocked(processContext);
    EXPECT_FALSE(ret);
}

HWTEST_F(SystemAbilityStateSchedulerLowMemTest, IsProcessIdledLocked004, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);

    auto processContext = std::make_shared<SystemProcessContext>();
    processContext->processName = u"test_process";
    processContext->abilityStateCountMap[SystemAbilityState::LOADED] = 0;
    processContext->abilityStateCountMap[SystemAbilityState::LOADING] = 1;

    sptr<ISystemProcessStatusChange> listener = new SystemProcessStatusChange();
    systemAbilityStateScheduler->procListenerMap_[u"test_process"].push_back(listener);

    bool ret = systemAbilityStateScheduler->IsProcessIdledLocked(processContext);
    EXPECT_FALSE(ret);
}

HWTEST_F(SystemAbilityStateSchedulerLowMemTest, NotifyProcessActivated001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);

    auto processContext = std::make_shared<SystemProcessContext>();
    processContext->processName = u"test_process";
    processContext->pid = 1234;
    processContext->uid = 1000;

    sptr<ISystemProcessStatusChange> listener = new SystemProcessStatusChange();
    systemAbilityStateScheduler->procListenerMap_[u"test_process"].push_back(listener);

    systemAbilityStateScheduler->NotifyProcessActivated(processContext);
    EXPECT_FALSE(systemAbilityStateScheduler->procListenerMap_[u"test_process"].empty());
}

HWTEST_F(SystemAbilityStateSchedulerLowMemTest, NotifyProcessActivated002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);

    auto processContext = std::make_shared<SystemProcessContext>();
    processContext->processName = u"test_process";
    processContext->pid = 1234;
    processContext->uid = 1000;

    systemAbilityStateScheduler->NotifyProcessActivated(processContext);
    EXPECT_TRUE(systemAbilityStateScheduler->procListenerMap_[u"test_process"].empty());
}

HWTEST_F(SystemAbilityStateSchedulerLowMemTest, NotifyProcessIdled001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);

    auto processContext = std::make_shared<SystemProcessContext>();
    processContext->processName = u"test_process";
    processContext->pid = 1234;
    processContext->uid = 1000;

    sptr<ISystemProcessStatusChange> listener = new SystemProcessStatusChange();
    systemAbilityStateScheduler->procListenerMap_[u"test_process"].push_back(listener);

    systemAbilityStateScheduler->NotifyProcessIdled(processContext);
    EXPECT_FALSE(systemAbilityStateScheduler->procListenerMap_[u"test_process"].empty());
}

HWTEST_F(SystemAbilityStateSchedulerLowMemTest, NotifyProcessIdled002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);

    auto processContext = std::make_shared<SystemProcessContext>();
    processContext->processName = u"test_process";
    processContext->pid = 1234;
    processContext->uid = 1000;

    systemAbilityStateScheduler->NotifyProcessIdled(processContext);
    EXPECT_TRUE(systemAbilityStateScheduler->procListenerMap_[u"test_process"].empty());
}

HWTEST_F(SystemAbilityStateSchedulerLowMemTest, SubscribeLowMemSystemProcess001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);

    sptr<ISystemProcessStatusChange> listener = nullptr;
    int32_t ret = systemAbilityStateScheduler->SubscribeLowMemSystemProcess(listener);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

HWTEST_F(SystemAbilityStateSchedulerLowMemTest, SubscribeLowMemSystemProcess002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);

    auto processContext = std::make_shared<SystemProcessContext>();
    processContext->processName = u"test_process";
    processContext->pid = 1234;
    processContext->uid = 1000;
    systemAbilityStateScheduler->processContextMap_[u"test_process"] = processContext;
    systemAbilityStateScheduler->lowMemoryProcessList_.push_back(u"test_process");

    sptr<ISystemProcessStatusChange> listener = new SystemProcessStatusChange();
    int32_t ret = systemAbilityStateScheduler->SubscribeLowMemSystemProcess(listener);
    EXPECT_EQ(ret, ERR_OK);
}

HWTEST_F(SystemAbilityStateSchedulerLowMemTest, UnSubscribeLowMemSystemProcess001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);

    sptr<ISystemProcessStatusChange> listener = nullptr;
    int32_t ret = systemAbilityStateScheduler->UnSubscribeLowMemSystemProcess(listener);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

HWTEST_F(SystemAbilityStateSchedulerLowMemTest, UnSubscribeLowMemSystemProcess002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);

    auto processContext = std::make_shared<SystemProcessContext>();
    processContext->processName = u"test_process";
    processContext->pid = 1234;
    processContext->uid = 1000;
    systemAbilityStateScheduler->processContextMap_[u"test_process"] = processContext;
    systemAbilityStateScheduler->lowMemoryProcessList_.push_back(u"test_process");

    sptr<ISystemProcessStatusChange> listener = new SystemProcessStatusChange();
    systemAbilityStateScheduler->SubscribeLowMemSystemProcess(listener);

    int32_t ret = systemAbilityStateScheduler->UnSubscribeLowMemSystemProcess(listener);
    EXPECT_EQ(ret, ERR_OK);
}

HWTEST_F(SystemAbilityStateSchedulerLowMemTest, SubscribeSystemProcessList001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);

    std::list<std::u16string> procNames = {u"test_process"};
    sptr<ISystemProcessStatusChange> listener = nullptr;
    int32_t ret = systemAbilityStateScheduler->SubscribeSystemProcessList(procNames, listener);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

HWTEST_F(SystemAbilityStateSchedulerLowMemTest, SubscribeSystemProcessList002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);

    auto processContext = std::make_shared<SystemProcessContext>();
    processContext->processName = u"test_process";
    processContext->pid = 1234;
    processContext->uid = 1000;
    systemAbilityStateScheduler->processContextMap_[u"test_process"] = processContext;

    std::list<std::u16string> procNames = {u"test_process"};
    sptr<ISystemProcessStatusChange> listener = new SystemProcessStatusChange();
    int32_t ret = systemAbilityStateScheduler->SubscribeSystemProcessList(procNames, listener);
    EXPECT_EQ(ret, ERR_OK);
}

HWTEST_F(SystemAbilityStateSchedulerLowMemTest, SubscribeSystemProcessList003, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);

    auto processContext1 = std::make_shared<SystemProcessContext>();
    processContext1->processName = u"test_process1";
    processContext1->pid = 1234;
    processContext1->uid = 1000;
    systemAbilityStateScheduler->processContextMap_[u"test_process1"] = processContext1;

    auto processContext2 = std::make_shared<SystemProcessContext>();
    processContext2->processName = u"test_process2";
    processContext2->pid = 1235;
    processContext2->uid = 1001;
    systemAbilityStateScheduler->processContextMap_[u"test_process2"] = processContext2;

    std::list<std::u16string> procNames = {u"test_process1", u"test_process2"};
    sptr<ISystemProcessStatusChange> listener = new SystemProcessStatusChange();
    int32_t ret = systemAbilityStateScheduler->SubscribeSystemProcessList(procNames, listener);
    EXPECT_EQ(ret, ERR_OK);
}

HWTEST_F(SystemAbilityStateSchedulerLowMemTest, SubscribeSystemProcessList004, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);

    auto processContext = std::make_shared<SystemProcessContext>();
    processContext->processName = u"test_process";
    processContext->pid = 1234;
    processContext->uid = 1000;
    systemAbilityStateScheduler->processContextMap_[u"test_process"] = processContext;

    std::list<std::u16string> procNames = {u"test_process"};
    sptr<ISystemProcessStatusChange> listener = new SystemProcessStatusChange();
    systemAbilityStateScheduler->SubscribeSystemProcessList(procNames, listener);

    int32_t ret = systemAbilityStateScheduler->SubscribeSystemProcessList(procNames, listener);
    EXPECT_EQ(ret, ERR_OK);
}

HWTEST_F(SystemAbilityStateSchedulerLowMemTest, SubscribeSystemProcessList005, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);

    auto processContext = std::make_shared<SystemProcessContext>();
    processContext->processName = u"test_process";
    processContext->pid = 1234;
    processContext->uid = 1000;
    systemAbilityStateScheduler->processContextMap_[u"test_process"] = processContext;

    std::list<std::u16string> procNames = {u"test_process"};

    auto callingPid = IPCSkeleton::GetCallingPid();
    systemAbilityStateScheduler->subscribeProcCountMap_[callingPid] = MAX_SUBSCRIBE_COUNT;

    sptr<ISystemProcessStatusChange> listener = new SystemProcessStatusChange();
    int32_t ret = systemAbilityStateScheduler->SubscribeSystemProcessList(procNames, listener);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

HWTEST_F(SystemAbilityStateSchedulerLowMemTest, SubscribeSystemProcessList006, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);

    std::list<std::u16string> procNames = {u"nonexistent_process"};
    sptr<ISystemProcessStatusChange> listener = new SystemProcessStatusChange();
    int32_t ret = systemAbilityStateScheduler->SubscribeSystemProcessList(procNames, listener);
    EXPECT_EQ(ret, ERR_OK);
}

HWTEST_F(SystemAbilityStateSchedulerLowMemTest, UnSubscribeSystemProcessList001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);

    std::list<std::u16string> procNames = {u"test_process"};
    sptr<ISystemProcessStatusChange> listener = nullptr;
    int32_t ret = systemAbilityStateScheduler->UnSubscribeSystemProcessList(procNames, listener);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

HWTEST_F(SystemAbilityStateSchedulerLowMemTest, UnSubscribeSystemProcessList002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);

    auto processContext = std::make_shared<SystemProcessContext>();
    processContext->processName = u"test_process";
    processContext->pid = 1234;
    processContext->uid = 1000;
    systemAbilityStateScheduler->processContextMap_[u"test_process"] = processContext;

    std::list<std::u16string> procNames = {u"test_process"};
    sptr<ISystemProcessStatusChange> listener = new SystemProcessStatusChange();
    systemAbilityStateScheduler->SubscribeSystemProcessList(procNames, listener);

    int32_t ret = systemAbilityStateScheduler->UnSubscribeSystemProcessList(procNames, listener);
    EXPECT_EQ(ret, ERR_OK);
}

HWTEST_F(SystemAbilityStateSchedulerLowMemTest, UnSubscribeSystemProcessList003, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);

    auto processContext1 = std::make_shared<SystemProcessContext>();
    processContext1->processName = u"test_process1";
    processContext1->pid = 1234;
    processContext1->uid = 1000;
    systemAbilityStateScheduler->processContextMap_[u"test_process1"] = processContext1;

    auto processContext2 = std::make_shared<SystemProcessContext>();
    processContext2->processName = u"test_process2";
    processContext2->pid = 1235;
    processContext2->uid = 1001;
    systemAbilityStateScheduler->processContextMap_[u"test_process2"] = processContext2;

    std::list<std::u16string> procNames = {u"test_process1", u"test_process2"};
    sptr<ISystemProcessStatusChange> listener = new SystemProcessStatusChange();
    systemAbilityStateScheduler->SubscribeSystemProcessList(procNames, listener);

    int32_t ret = systemAbilityStateScheduler->UnSubscribeSystemProcessList(procNames, listener);
    EXPECT_EQ(ret, ERR_OK);
}

HWTEST_F(SystemAbilityStateSchedulerLowMemTest, UnSubscribeSystemProcessList004, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);

    std::list<std::u16string> procNames = {u"nonexistent_process"};
    sptr<ISystemProcessStatusChange> listener = new SystemProcessStatusChange();
    int32_t ret = systemAbilityStateScheduler->UnSubscribeSystemProcessList(procNames, listener);
    EXPECT_EQ(ret, ERR_OK);
}

HWTEST_F(SystemAbilityStateSchedulerLowMemTest, UnSubscribeSystemProcessList005, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);

    auto processContext = std::make_shared<SystemProcessContext>();
    processContext->processName = u"test_process";
    processContext->pid = 1234;
    processContext->uid = 1000;
    systemAbilityStateScheduler->processContextMap_[u"test_process"] = processContext;

    std::list<std::u16string> procNames = {u"test_process"};
    sptr<ISystemProcessStatusChange> listener = new SystemProcessStatusChange();

    int32_t ret = systemAbilityStateScheduler->UnSubscribeSystemProcessList(procNames, listener);
    EXPECT_EQ(ret, ERR_OK);
}

HWTEST_F(SystemAbilityStateSchedulerLowMemTest, OnAbilityLoadedLockedWithActivation001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);

    auto processContext = std::make_shared<SystemProcessContext>();
    processContext->processName = u"test_process";
    processContext->pid = 1234;
    processContext->uid = 1000;
    processContext->abilityStateCountMap[SystemAbilityState::LOADED] = 1;

    auto abilityContext = std::make_shared<SystemAbilityContext>();
    abilityContext->systemAbilityId = SAID;
    abilityContext->state = SystemAbilityState::LOADED;
    abilityContext->ownProcessContext = processContext;

    systemAbilityStateScheduler->processContextMap_[u"test_process"] = processContext;
    systemAbilityStateScheduler->abilityContextMap_[SAID] = abilityContext;

    sptr<ISystemProcessStatusChange> listener = new SystemProcessStatusChange();
    systemAbilityStateScheduler->procListenerMap_[u"test_process"].push_back(listener);

    systemAbilityStateScheduler->OnAbilityLoadedLocked(SAID);
    EXPECT_FALSE(systemAbilityStateScheduler->procListenerMap_[u"test_process"].empty());
}

HWTEST_F(SystemAbilityStateSchedulerLowMemTest, OnAbilityUnloadableLockedWithIdled001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);

    auto processContext = std::make_shared<SystemProcessContext>();
    processContext->processName = u"test_process";
    processContext->pid = 1234;
    processContext->uid = 1000;
    processContext->abilityStateCountMap[SystemAbilityState::LOADED] = 0;
    processContext->abilityStateCountMap[SystemAbilityState::LOADING] = 0;

    auto abilityContext = std::make_shared<SystemAbilityContext>();
    abilityContext->systemAbilityId = SAID;
    abilityContext->state = SystemAbilityState::UNLOADABLE;
    abilityContext->ownProcessContext = processContext;

    systemAbilityStateScheduler->processContextMap_[u"test_process"] = processContext;
    systemAbilityStateScheduler->abilityContextMap_[SAID] = abilityContext;

    sptr<ISystemProcessStatusChange> listener = new SystemProcessStatusChange();
    systemAbilityStateScheduler->procListenerMap_[u"test_process"].push_back(listener);

    systemAbilityStateScheduler->OnAbilityUnloadableLocked(SAID);
    EXPECT_FALSE(systemAbilityStateScheduler->procListenerMap_[u"test_process"].empty());
}
