/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include "system_ability_mgr_test.h"
#include "hisysevent_adapter.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "itest_transaction_service.h"
#include "sam_mock_permission.h"
#include "parameter.h"
#include "parameters.h"
#include "sa_profiles.h"
#include "sa_status_change_mock.h"
#include "string_ex.h"
#include "system_ability_definition.h"
#include "samgr_err_code.h"
#include "system_process_status_change_proxy.h"
#include "system_ability_manager_util.h"
#include "test_log.h"
#include "ability_death_recipient.h"
#define private public
#define protected public
#include "ipc_skeleton.h"
#ifdef SUPPORT_ACCESS_TOKEN
#include "accesstoken_kit.h"
#endif
#include "system_ability_manager.h"
#ifdef SUPPORT_COMMON_EVENT
#include "common_event_collect.h"
#include "ability_death_recipient.h"
#endif

using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS;

namespace OHOS {
namespace {
enum {
        SHEEFT_CRITICAL = 0,
        SHEEFT_HIGH,
        SHEEFT_NORMAL,
        SHEEFT_DEFAULT,
        SHEEFT_PROTO,
};

constexpr int32_t SAID = 1234;
constexpr int32_t TEST_VALUE = 2021;
constexpr int32_t TEST_REVERSE_VALUE = 1202;
constexpr int32_t REPEAT = 10;
constexpr int32_t TEST_EXCEPTION_HIGH_SA_ID = LAST_SYS_ABILITY_ID + 1;
constexpr int32_t TEST_EXCEPTION_LOW_SA_ID = -1;
constexpr int32_t TEST_SYSTEM_ABILITY1 = 1491;
constexpr int32_t TEST_SYSTEM_ABILITY2 = 1492;
constexpr int32_t SHFIT_BIT = 32;
constexpr int32_t MAX_COUNT = INT32_MAX - 1000000;
const unsigned int DUMP_FLAG_PRIORITY_CRITICAL = 1 << SHEEFT_CRITICAL;
const unsigned int DUMP_FLAG_PRIORITY_HIGH = 1 << SHEEFT_HIGH;
const unsigned int DUMP_FLAG_PRIORITY_NORMAL = 1 << SHEEFT_NORMAL;
const unsigned int DUMP_FLAG_PRIORITY_DEFAULT = 1 << SHEEFT_DEFAULT;
const unsigned int DUMP_FLAG_PRIORITY_ALL = DUMP_FLAG_PRIORITY_CRITICAL |
        DUMP_FLAG_PRIORITY_HIGH | DUMP_FLAG_PRIORITY_NORMAL | DUMP_FLAG_PRIORITY_DEFAULT;
const unsigned int DUMP_FLAG_PROTO = 1 << SHEEFT_PROTO;

const std::u16string PROCESS_NAME = u"test_process_name";
const std::u16string DEVICE_NAME = u"test_name";

class DumpLocalAbilityManager final : public IRemoteStub<ILocalAbilityManager> {
public:
    bool StartAbility(int32_t systemAbilityId, const std::string& eventStr) override { return true; }
    bool StopAbility(int32_t systemAbilityId, const std::string& eventStr) override { return true; }
    bool ActiveAbility(int32_t systemAbilityId, const nlohmann::json& activeReason) override { return true; }
    bool IdleAbility(int32_t systemAbilityId, const nlohmann::json& idleReason, int32_t& delayTime) override
    {
        return true;
    }
    bool SendStrategyToSA(int32_t type, int32_t systemAbilityId, int32_t level, std::string& action) override
    {
        return true;
    }
    bool IpcStatCmdProc(int32_t fd, int32_t cmd) override
    {
        ++ipcStatCallCount_;
        return true;
    }
    bool FfrtStatCmdProc(int32_t fd, int32_t cmd) override { return true; }
    bool FfrtDumperProc(std::string& result) override { return true; }
    int32_t SystemAbilityExtProc(const std::string& extension, int32_t said,
        SystemAbilityExtensionPara* callback, bool isAsync) override
    {
        return ERR_OK;
    }
    int32_t ServiceControlCmd(int32_t fd, int32_t systemAbilityId,
        const std::vector<std::u16string>& args) override
    {
        return ERR_OK;
    }

    int32_t ipcStatCallCount_ = 0;
};

void InitSaMgr(sptr<SystemAbilityManager>& saMgr)
{
    std::weak_ptr<BaseSystemAbilityManager> weakMgr;
    saMgr->abilityDeath_ = sptr<IRemoteObject::DeathRecipient>(
        new AbilityDeathRecipient(weakMgr));
    saMgr->systemProcessDeath_ = sptr<IRemoteObject::DeathRecipient>(
        new SystemProcessDeathRecipient(weakMgr));
    saMgr->abilityStatusDeath_ = sptr<IRemoteObject::DeathRecipient>(
        new AbilityStatusDeathRecipient(weakMgr));
    saMgr->abilityCallbackDeath_ = sptr<IRemoteObject::DeathRecipient>(
        new AbilityCallbackDeathRecipient(weakMgr));
    saMgr->remoteCallbackDeath_ = sptr<IRemoteObject::DeathRecipient>(
        new RemoteCallbackDeathRecipient(weakMgr));
    saMgr->workHandler_ = make_shared<FFRTHandler>("workHandler");
    saMgr->collectManager_ = sptr<DeviceStatusCollectManager>(
        new DeviceStatusCollectManager(weakMgr));
    saMgr->abilityStateScheduler_ = std::make_shared<SystemAbilityStateScheduler>(weakMgr);
}

#ifdef SUPPORT_MULTI_INSTANCE
void InitUserLifecycleManager(const sptr<SystemAbilityManager>& saMgr)
{
    saMgr->userLifecycleManager_.SetSaProfiles(&saMgr->allSaProfiles_);
}
#endif
}

void SystemProcessStatusChange::OnSystemProcessStarted(SystemProcessInfo& systemProcessInfo)
{
    DTEST_LOG << "OnSystemProcessStarted, processName: ";
}

void SystemProcessStatusChange::OnSystemProcessStopped(SystemProcessInfo& systemProcessInfo)
{
    DTEST_LOG << "OnSystemProcessStopped, processName: ";
}

void SystemAbilityMgrTest::SetUpTestCase()
{
    DTEST_LOG << "SetUpTestCase" << std::endl;
}

void SystemAbilityMgrTest::TearDownTestCase()
{
    DTEST_LOG << "TearDownTestCase" << std::endl;
}

void SystemAbilityMgrTest::SetUp()
{
    SamMockPermission::MockPermission();
    DTEST_LOG << "SetUp" << std::endl;
}

void SystemAbilityMgrTest::TearDown()
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->CleanFfrt();
    DTEST_LOG << "TearDown" << std::endl;
}

/**
 * @tc.name: AddSystemAbility001
 * @tc.desc: add system ability, input invalid parameter
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, AddSystemAbility001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    ISystemAbilityManager::SAExtraProp extraProp(false, DUMP_FLAG_PRIORITY_DEFAULT, u"", u"");
    int32_t result = saMgr->AddSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID, nullptr, extraProp);
    DTEST_LOG << "add TestTransactionService result = " << result << std::endl;
    EXPECT_TRUE(result != ERR_OK);
}

/**
 * @tc.name: AddSystemAbility002
 * @tc.desc: add system ability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, AddSystemAbility002, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    ISystemAbilityManager::SAExtraProp extraProp(false, DUMP_FLAG_PRIORITY_DEFAULT, u"", u"");
    int32_t result = saMgr->AddSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID, testAbility, extraProp);
    DTEST_LOG << "add TestTransactionService result = " << result << std::endl;
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AddSystemAbility003
 * @tc.desc: add system ability saId exception.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, AddSystemAbility003, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    ISystemAbilityManager::SAExtraProp extraProp(false, DUMP_FLAG_PRIORITY_DEFAULT, u"", u"");
    int32_t result = saMgr->AddSystemAbility(TEST_EXCEPTION_HIGH_SA_ID, testAbility, extraProp);
    EXPECT_TRUE(result != ERR_OK);
    result = saMgr->AddSystemAbility(TEST_EXCEPTION_LOW_SA_ID, testAbility, extraProp);
    EXPECT_TRUE(result != ERR_OK);
}

/**
 * @tc.name: AddSystemAbility004
 * @tc.desc: add system ability with empty capability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, AddSystemAbility004, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    int32_t systemAbilityId = DISTRIBUTED_SCHED_TEST_TT_ID;
    ISystemAbilityManager::SAExtraProp saExtraProp(false, ISystemAbilityManager::DUMP_FLAG_PRIORITY_DEFAULT,
        u"", u"");
    int32_t ret = saMgr->AddSystemAbility(systemAbilityId, new TestTransactionService(), saExtraProp);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: AddSystemAbility005
 * @tc.desc: add system ability, saExtraProp diff from saProfileMap_.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, AddSystemAbility005, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    int32_t systemAbilityId = DISTRIBUTED_SCHED_TEST_SO_ID;
    std::u16string capability = u"{\"Capabilities\":{\"aaa\":\"[10.4, 20.5]\",\"bbb\":\"[11, 55]\",\
        \"ccc\":\"this is string\", \"ddd\":\"[aa, bb, cc, dd]\", \"eee\":5.60, \"fff\":4545, \"ggg\":true}}";
    ISystemAbilityManager::SAExtraProp saExtraProp(true, ISystemAbilityManager::DUMP_FLAG_PRIORITY_DEFAULT,
        capability, u"");
    CommonSaProfile saProfile;
    saProfile.process = u"test";
    saProfile.distributed = false;
    saProfile.saId = systemAbilityId;
    saMgr->saProfileMap_[systemAbilityId] = saProfile;
    int32_t ret = saMgr->AddSystemAbility(systemAbilityId, new TestTransactionService(), saExtraProp);
    saMgr->saProfileMap_.erase(systemAbilityId);
    saMgr->RemoveSystemAbility(systemAbilityId);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AddSystemAbility006
 * @tc.desc: add system ability, ERR_OK.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, AddSystemAbility006, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    saMgr->abilityStateScheduler_ = std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
    CommonSaProfile saProfile;
    saProfile.process = u"test";
    saProfile.distributed = true;
    saProfile.saId = SAID;
    saMgr->saProfileMap_[SAID] = saProfile;
    ISystemAbilityManager::SAExtraProp extraProp(true, DUMP_FLAG_PRIORITY_DEFAULT, u"", u"");
    int32_t ret = saMgr->AddSystemAbility(SAID, new TestTransactionService(), extraProp);
    saMgr->saProfileMap_.erase(SAID);
    saMgr->RemoveSystemAbility(SAID);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: RemoveSystemAbility001
 * @tc.desc: remove not exist system ability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, RemoveSystemAbility001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    int32_t result = saMgr->RemoveSystemAbility(-1);
    EXPECT_TRUE(result != ERR_OK);
}

/**
 * @tc.name: RemoveSystemAbility002
 * @tc.desc: remove system ability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, RemoveSystemAbility002, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    ISystemAbilityManager::SAExtraProp extraProp(false, DUMP_FLAG_PRIORITY_DEFAULT, u"", u"");
    saMgr->AddSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID, testAbility, extraProp);
    int32_t result = saMgr->RemoveSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: RemoveSystemAbility003
 * @tc.desc: remove system ability. abilityStateScheduler_ is nullptr
 * @tc.type: FUNC
 * @tc.require: I6MO6A
 */
HWTEST_F(SystemAbilityMgrTest, RemoveSystemAbility003, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    saMgr->abilityStateScheduler_ = nullptr;
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    ISystemAbilityManager::SAExtraProp extraProp;
    saMgr->AddSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID, testAbility, extraProp);
    int32_t result = saMgr->RemoveSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: RemoveSystemAbility004
 * @tc.desc: remove not exist system ability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, RemoveSystemAbility004, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    int32_t result = saMgr->RemoveSystemAbility(-1);
    EXPECT_TRUE(result != ERR_OK);
}

/**
 * @tc.name: RemoveSystemAbility006
 * @tc.desc: test RemoveSystemAbility, ERR_INVALID_VALUE.
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrTest, RemoveSystemAbility006, TestSize.Level0)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    ISystemAbilityManager::SAExtraProp saExtraProp;
    saMgr->AddSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID, testAbility, saExtraProp);
    std::shared_ptr<SystemAbilityStateScheduler> saScheduler = saMgr->abilityStateScheduler_;
    saMgr->abilityStateScheduler_ = nullptr;
    int32_t ret = saMgr->RemoveSystemAbility(testAbility);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: RemoveSystemAbility007
 * @tc.desc: test RemoveSystemAbility, ERR_OK.
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrTest, RemoveSystemAbility007, TestSize.Level0)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    ISystemAbilityManager::SAExtraProp saExtraProp;
    saMgr->AddSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID, testAbility, saExtraProp);
    CommonSaProfile saProfile = {u"test", DISTRIBUTED_SCHED_TEST_TT_ID};
    saProfile.cacheCommonEvent = true;
    saMgr->saProfileMap_[DISTRIBUTED_SCHED_TEST_TT_ID] = saProfile;
    int32_t ret = saMgr->RemoveSystemAbility(testAbility);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: GetSystemAbility001
 * @tc.desc: get not exist system ability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, GetSystemAbility001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    auto ability = saMgr->GetSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID);
    EXPECT_EQ(ability, nullptr);
}

/**
 * @tc.name: GetSystemAbility002
 * @tc.desc: get system ability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, GetSystemAbility002, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    ISystemAbilityManager::SAExtraProp extraProp(false, DUMP_FLAG_PRIORITY_DEFAULT, u"", u"");
    saMgr->AddSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID, testAbility, extraProp);
    auto ability = saMgr->GetSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID);
    EXPECT_TRUE(ability != nullptr);
}

/**
 * @tc.name: GetSystemAbility003
 * @tc.desc: get system ability and then transaction.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, GetSystemAbility003, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    ISystemAbilityManager::SAExtraProp extraProp(false, DUMP_FLAG_PRIORITY_DEFAULT, u"", u"");
    saMgr->AddSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID, testAbility, extraProp);
    auto ability = saMgr->GetSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID);
    EXPECT_TRUE(ability != nullptr);
    sptr<ITestTransactionService> targetAblility = iface_cast<ITestTransactionService>(ability);
    EXPECT_TRUE(targetAblility != nullptr);
    int32_t rep = 0;
    int32_t result = targetAblility->ReverseInt(TEST_VALUE, rep);
    DTEST_LOG << "testAbility ReverseInt result = " << result << ", get reply = " << rep << std::endl;
    EXPECT_EQ(rep, TEST_REVERSE_VALUE);
}

/**
 * @tc.name: GetSystemAbility004
 * @tc.desc: get system ability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, GetSystemAbility004, TestSize.Level2)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    ISystemAbilityManager::SAExtraProp extraProp(false, DUMP_FLAG_PRIORITY_DEFAULT, u"", u"");
    for (int32_t i = 0; i < REPEAT; ++i) {
        auto result = saMgr->AddSystemAbility((DISTRIBUTED_SCHED_TEST_SO_ID + i),
            new TestTransactionService(), extraProp);
        EXPECT_EQ(result, ERR_OK);
    }
    for (int32_t i = 0; i < REPEAT; ++i) {
        int32_t saId = DISTRIBUTED_SCHED_TEST_SO_ID + i;
        auto saObject = saMgr->GetSystemAbility(saId);
        EXPECT_TRUE(saObject != nullptr);
    }
}

/**
 * @tc.name: GetSystemAbility005
 * @tc.desc: get remote device system ability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, GetSystemAbility005, TestSize.Level2)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    string fakeDeviceId = "fake_dev";
    auto abilityObj = saMgr->GetSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID, fakeDeviceId);
    EXPECT_EQ(abilityObj, nullptr);
}

/**
 * @tc.name: CheckSystemAbility001
 * @tc.desc: check system ability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, CheckSystemAbility001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    int32_t systemAbilityId = DISTRIBUTED_SCHED_TEST_TT_ID;
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    ISystemAbilityManager::SAExtraProp extraProp(false, DUMP_FLAG_PRIORITY_DEFAULT, u"", u"");
    saMgr->AddSystemAbility(systemAbilityId, testAbility, extraProp);
    sptr<IRemoteObject> abilityObj = saMgr->CheckSystemAbility(systemAbilityId);
    EXPECT_TRUE(abilityObj != nullptr);
}

/**
 * @tc.name: CheckSystemAbility002
 * @tc.desc: check system ability. abilityStateScheduler_ is nullptr
 * @tc.type: FUNC
 * @tc.require: I6MO6A
 */
HWTEST_F(SystemAbilityMgrTest, CheckSystemAbility002, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    saMgr->abilityStateScheduler_ = nullptr;
    int32_t systemAbilityId = DISTRIBUTED_SCHED_TEST_TT_ID;
    bool isExist = true;
    sptr<IRemoteObject> abilityObj = saMgr->CheckSystemAbility(systemAbilityId, isExist);
    EXPECT_EQ(abilityObj, nullptr);
}

/**
 * @tc.name: CheckSystemAbility003
 * @tc.desc: test CheckSystemAbility with  abilityStateScheduler_ is nullptr
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrTest, CheckSystemAbility003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    saMgr->abilityStateScheduler_ = nullptr;
    bool isExist = true;
    sptr<IRemoteObject> ret = saMgr->CheckSystemAbility(SAID, isExist);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: CheckSystemAbility004
 * @tc.desc: test CheckSystemAbility with systemAbilityId is unloading
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrTest, CheckSystemAbility004, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemAbilityContext->ownProcessContext = systemProcessContext;
    saMgr->abilityStateScheduler_->abilityContextMap_[SAID] = systemAbilityContext;
    systemAbilityContext->state = SystemAbilityState::UNLOADING;
    bool isExist = true;
    sptr<IRemoteObject> ret = saMgr->CheckSystemAbility(SAID, isExist);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: CheckSystemAbility005
 * @tc.desc: check system ability. abilityStateScheduler_ is nullptr
 * @tc.type: FUNC
 * @tc.require: I6MO6A
 */
HWTEST_F(SystemAbilityMgrTest, CheckSystemAbility005, TestSize.Level3)
{
    DTEST_LOG << " CheckSystemAbility005 " << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    int32_t systemAbilityId = DISTRIBUTED_SCHED_TEST_TT_ID;
    bool isExist = true;
    saMgr->abilityStateScheduler_ = nullptr;
    sptr<IRemoteObject> abilityObj = saMgr->CheckSystemAbility(systemAbilityId, isExist);
    EXPECT_EQ(abilityObj, nullptr);
}

/**
 * @tc.name: ListSystemAbility001
 * @tc.desc: list all system abilities.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, ListSystemAbility001, TestSize.Level1)
{
    int32_t systemAbilityId = DISTRIBUTED_SCHED_TEST_TT_ID;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    ISystemAbilityManager::SAExtraProp extraProp(false, DUMP_FLAG_PRIORITY_DEFAULT, u"", u"");
    saMgr->AddSystemAbility(systemAbilityId, new TestTransactionService(), extraProp);
    auto saList = saMgr->ListSystemAbilities(DUMP_FLAG_PRIORITY_ALL);
    EXPECT_TRUE(!saList.empty());
    auto iter = std::find(saList.begin(), saList.end(), to_utf16(std::to_string(systemAbilityId)));
    EXPECT_TRUE(iter != saList.end());
}

/**
 * @tc.name: OnRemoteDied001
 * @tc.desc: test OnRemoteDied, remove registered callback.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, OnRemoteDied001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    sptr<ISystemAbilityLoadCallback> callback = new SystemAbilityLoadCallbackMock();
    saMgr->OnAbilityCallbackDied(callback->AsObject());
    EXPECT_TRUE(saMgr->startingAbilityMap_.empty());
}

/**
 * @tc.name: DoMakeRemoteBinder001
 * @tc.desc: load system ability with invalid systemAbilityId.
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, DoMakeRemoteBinder001, TestSize.Level2)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    saMgr->dBinderService_ = DBinderService::GetInstance();
    std::string deviceId = "1111111";
    int32_t systemAbilityId = 0;
    auto remoteObject = saMgr->DoMakeRemoteBinder(systemAbilityId, 0, 0, deviceId);
    EXPECT_TRUE(remoteObject == nullptr);
}

/**
 * @tc.name: DoMakeRemoteBinder002
 * @tc.desc: load system ability with invalid systemAbilityId.
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, DoMakeRemoteBinder002, TestSize.Level2)
{
    DTEST_LOG << " DoMakeRemoteBinder002 " << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    saMgr->dBinderService_ = DBinderService::GetInstance();
    std::string deviceId = "2222222";
    int32_t systemAbilityId = -1;
    auto remoteObject = saMgr->DoMakeRemoteBinder(systemAbilityId, 0, 0, deviceId);
    EXPECT_TRUE(remoteObject == nullptr);
}

/**
 * @tc.name: startingAbilityMap_ test
 * @tc.desc: startingAbilityMap_ init
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, startingAbilityMapTest001, TestSize.Level1)
{
    DTEST_LOG << " startingAbilityMapTest001 start " << std::endl;
    /**
     * @tc.steps: step1. init startingAbilityMap_
     * @tc.expected: step1. init startingAbilityMap_
     */
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback3 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback4 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback5 = new SystemAbilityLoadCallbackMock();

    std::map<std::string, SystemAbilityManager::CallbackList> mockCallbackMap1 = {
        {"111111", {{mockLoadCallback1, 0}}},
        {"222222", {{mockLoadCallback1, 0}, {mockLoadCallback2, 0}}},
        {"333333", {{mockLoadCallback2, 0}, {mockLoadCallback3, 1}}}
    };
    std::map<std::string, SystemAbilityManager::CallbackList> mockCallbackMap2 = {
        {"111111", {{mockLoadCallback1, 0}}},
        {"222222", {{mockLoadCallback1, 0}, {mockLoadCallback2, 0}}},
        {"333333", {{mockLoadCallback2, 0}, {mockLoadCallback3, 1}}}
    };
    std::map<std::string, SystemAbilityManager::CallbackList> mockCallbackMap3 = {
        {"111111", {{mockLoadCallback2, 0}}},
        {"222222", {{mockLoadCallback3, 0}, {mockLoadCallback2, 0}}},
        {"333333", {{mockLoadCallback4, 0}, {mockLoadCallback5, 1}}}
    };
    SystemAbilityManager::AbilityItem mockAbilityItem1 = {
        SystemAbilityManager::AbilityState::INIT, mockCallbackMap1
    };
    SystemAbilityManager::AbilityItem mockAbilityItem2 = {
        SystemAbilityManager::AbilityState::INIT, mockCallbackMap2
    };

    saMgr->startingAbilityMap_.emplace(TEST_SYSTEM_ABILITY2, mockAbilityItem1);
    ASSERT_EQ(saMgr->startingAbilityMap_.size(), 1);
    saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY1] = mockAbilityItem1;
    ASSERT_TRUE(saMgr->startingAbilityMap_.size() > 1);
}

/**
 * @tc.name: startingAbilityMap_ test
 * @tc.desc: test for callback dead, with one device, one callback
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, startingAbilityMapTest002, TestSize.Level1)
{
    DTEST_LOG << " startingAbilityMapTest002 start " << std::endl;
    /**
     * @tc.steps: step1. init startingAbilityMap_
     * @tc.expected: step1. init startingAbilityMap_
     */
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();

    std::map<std::string, SystemAbilityManager::CallbackList> mockCallbackMap1 = {
        {"111111", {{mockLoadCallback1, 0}}}
    };
    SystemAbilityManager::AbilityItem mockAbilityItem1 = {
        SystemAbilityManager::AbilityState::INIT, mockCallbackMap1
    };

    saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY1] = mockAbilityItem1;

    saMgr->OnAbilityCallbackDied(mockLoadCallback1->AsObject());
    ASSERT_EQ(saMgr->startingAbilityMap_.size(), 0);
}

/**
 * @tc.name: startingAbilityMap_ test
 * @tc.desc: test for callback dead, with one device, some callback
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, startingAbilityMapTest003, TestSize.Level1)
{
    DTEST_LOG << " startingAbilityMapTest003 start " << std::endl;
    /**
     * @tc.steps: step1. init startingAbilityMap_
     * @tc.expected: step1. init startingAbilityMap_
     */
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);

    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();

    std::map<std::string, SystemAbilityManager::CallbackList> mockCallbackMap1 = {
        {"111111", {{mockLoadCallback1, 0}, {mockLoadCallback2, 1}}}
    };
    SystemAbilityManager::AbilityItem mockAbilityItem1 = {
        SystemAbilityManager::AbilityState::INIT, mockCallbackMap1
    };

    saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY1] = mockAbilityItem1;
    saMgr->OnAbilityCallbackDied(mockLoadCallback1->AsObject());
    ASSERT_EQ(saMgr->startingAbilityMap_.size(), 1);
    ASSERT_EQ(saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY1].callbackMap["111111"].size(), 1);
}

/**
 * @tc.name: startingAbilityMap_ test
 * @tc.desc: test for callback dead, with no registered callback
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, startingAbilityMapTest004, TestSize.Level1)
{
    DTEST_LOG << " startingAbilityMapTest004 start " << std::endl;
    /**
     * @tc.steps: step1. init startingAbilityMap_
     * @tc.expected: step1. init startingAbilityMap_
     */
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);

    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();

    std::map<std::string, SystemAbilityManager::CallbackList> mockCallbackMap1 = {
        {"111111", {{mockLoadCallback1, 1}}}
    };
    SystemAbilityManager::AbilityItem mockAbilityItem1 = {
        SystemAbilityManager::AbilityState::INIT, mockCallbackMap1
    };

    saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY1] = mockAbilityItem1;
    saMgr->OnAbilityCallbackDied(mockLoadCallback2->AsObject());
    ASSERT_EQ(saMgr->startingAbilityMap_.size(), 1);
    ASSERT_EQ(saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY1].callbackMap["111111"].size(), 1);
}

/**
 * @tc.name: startingAbilityMap_ test
 * @tc.desc: test for callback dead, with some device, some callback
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, startingAbilityMapTest005, TestSize.Level1)
{
    DTEST_LOG << " startingAbilityMapTest004 start " << std::endl;
    /**
     * @tc.steps: step1. init startingAbilityMap_
     * @tc.expected: step1. init startingAbilityMap_
     */
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);

    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();
    std::map<std::string, SystemAbilityManager::CallbackList> mockCallbackMap1 = {
        {"111111", {{mockLoadCallback1, 1}}},
        {"222222", {{mockLoadCallback2, 1}}}
    };
    SystemAbilityManager::AbilityItem mockAbilityItem1 = {
        SystemAbilityManager::AbilityState::INIT, mockCallbackMap1
    };

    saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY1] = mockAbilityItem1;
    saMgr->OnAbilityCallbackDied(mockLoadCallback2->AsObject());
    ASSERT_EQ(saMgr->startingAbilityMap_.size(), 1);
    ASSERT_EQ(saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY1].callbackMap.size(), 1);
    ASSERT_EQ(saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY1].callbackMap["111111"].size(), 1);
}

/**
 * @tc.name: startingAbilityMap_ test
 * @tc.desc: test for callback dead, with some device, one callback
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, startingAbilityMapTest006, TestSize.Level1)
{
    DTEST_LOG << " startingAbilityMapTest006 start " << std::endl;
    /**
     * @tc.steps: step1. init startingAbilityMap_
     * @tc.expected: step1. init startingAbilityMap_
     */
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);

    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();

    std::map<std::string, SystemAbilityManager::CallbackList> mockCallbackMap1 = {
        {"111111", {{mockLoadCallback1, 1}}},
        {"222222", {{mockLoadCallback1, 0}}}
    };
    SystemAbilityManager::AbilityItem mockAbilityItem1 = {
        SystemAbilityManager::AbilityState::INIT, mockCallbackMap1
    };

    saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY1] = mockAbilityItem1;
    saMgr->OnAbilityCallbackDied(mockLoadCallback1->AsObject());
    ASSERT_EQ(saMgr->startingAbilityMap_.size(), 0);
}

/**
 * @tc.name: startingAbilityMap_ test
 * @tc.desc: test for callback dead, with one device, some callback, some sa
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, startingAbilityMapTest007, TestSize.Level1)
{
    DTEST_LOG << " startingAbilityMapTest007 start " << std::endl;
    /**
     * @tc.steps: step1. init startingAbilityMap_
     * @tc.expected: step1. init startingAbilityMap_
     */
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);

    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();

    std::map<std::string, SystemAbilityManager::CallbackList> mockCallbackMap1 = {
        {"111111", {{mockLoadCallback1, 1}, {mockLoadCallback2, 1}}}
    };
    SystemAbilityManager::AbilityItem mockAbilityItem1 = {
        SystemAbilityManager::AbilityState::INIT, mockCallbackMap1
    };

    saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY1] = mockAbilityItem1;
    saMgr->OnAbilityCallbackDied(mockLoadCallback1->AsObject());
    ASSERT_EQ(saMgr->startingAbilityMap_.size(), 1);
    ASSERT_EQ(saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY1].callbackMap.size(), 1);
}

/**
 * @tc.name: startingAbilityMap_ test
 * @tc.desc: test for callback dead, with one device, some callback
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, startingAbilityMapTest008, TestSize.Level1)
{
    DTEST_LOG << " startingAbilityMapTest007 start " << std::endl;
    /**
     * @tc.steps: step1. init startingAbilityMap_
     * @tc.expected: step1. init startingAbilityMap_
     */
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);

    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();

    std::map<std::string, SystemAbilityManager::CallbackList> mockCallbackMap1 = {
        {"111111", {{mockLoadCallback1, 1}}}
    };
    SystemAbilityManager::AbilityItem mockAbilityItem1 = {
        SystemAbilityManager::AbilityState::INIT, mockCallbackMap1
    };

    saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY1] = mockAbilityItem1;
    saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY2] = mockAbilityItem1;
    saMgr->OnAbilityCallbackDied(mockLoadCallback1->AsObject());
    ASSERT_EQ(saMgr->startingAbilityMap_.size(), 0);
    saMgr->OnAbilityCallbackDied(mockLoadCallback2->AsObject());
    ASSERT_EQ(saMgr->startingAbilityMap_.size(), 0);
}

/**
 * @tc.name: startingAbilityMap_ test
 * @tc.desc: test for callback dead, with one device, some callback, some sa
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, startingAbilityMapTest009, TestSize.Level1)
{
    DTEST_LOG << " startingAbilityMapTest009 start " << std::endl;
    /**
     * @tc.steps: step1. init startingAbilityMap_
     * @tc.expected: step1. init startingAbilityMap_
     */
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);

    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();

    std::map<std::string, SystemAbilityManager::CallbackList> mockCallbackMap1 = {
        {"111111", {{mockLoadCallback1, 1}, {mockLoadCallback2, 1}}}
    };
    SystemAbilityManager::AbilityItem mockAbilityItem1 = {
        SystemAbilityManager::AbilityState::INIT, mockCallbackMap1
    };

    saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY1] = mockAbilityItem1;
    saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY2] = mockAbilityItem1;
    saMgr->OnAbilityCallbackDied(mockLoadCallback1->AsObject());
    ASSERT_TRUE(saMgr->startingAbilityMap_.size() > 1);
    ASSERT_EQ(saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY1].callbackMap.size(), 1);
}

/**
 * @tc.name: startingAbilityMap_ test
 * @tc.desc: test for callback dead, with some device, some callback, some sa
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, startingAbilityMapTest010, TestSize.Level1)
{
    DTEST_LOG << " startingAbilityMapTest010 start " << std::endl;
    /**
     * @tc.steps: step1. init startingAbilityMap_
     * @tc.expected: step1. init startingAbilityMap_
     */
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);

    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();

    std::map<std::string, SystemAbilityManager::CallbackList> mockCallbackMap1 = {
        {"111111", {{mockLoadCallback1, 1}}}
    };

    std::map<std::string, SystemAbilityManager::CallbackList> mockCallbackMap2 = {
        {"111111", {{mockLoadCallback1, 1}}}
    };
    SystemAbilityManager::AbilityItem mockAbilityItem1 = {
        SystemAbilityManager::AbilityState::INIT, mockCallbackMap1
    };
    SystemAbilityManager::AbilityItem mockAbilityItem2 = {
        SystemAbilityManager::AbilityState::INIT, mockCallbackMap2
    };

    saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY1] = mockAbilityItem1;
    saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY2] = mockAbilityItem2;
    saMgr->OnAbilityCallbackDied(mockLoadCallback1->AsObject());
    ASSERT_EQ(saMgr->startingAbilityMap_.size(), 0);
    saMgr->OnAbilityCallbackDied(mockLoadCallback2->AsObject());
    ASSERT_EQ(saMgr->startingAbilityMap_.size(), 0);
}

/**
 * @tc.name: startingAbilityMap_ test
 * @tc.desc: test for callback dead, with one device, some callback, some sa
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, startingAbilityMapTest011, TestSize.Level1)
{
    DTEST_LOG << " startingAbilityMapTest010 start " << std::endl;
    /**
     * @tc.steps: step1. init startingAbilityMap_
     * @tc.expected: step1. init startingAbilityMap_
     */
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);

    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();

    std::map<std::string, SystemAbilityManager::CallbackList> mockCallbackMap1 = {
        {"111111", {{mockLoadCallback1, 1}}}
    };

    std::map<std::string, SystemAbilityManager::CallbackList> mockCallbackMap2 = {
        {"111111", {{mockLoadCallback2, 1}}}
    };
    SystemAbilityManager::AbilityItem mockAbilityItem1 = {
        SystemAbilityManager::AbilityState::INIT, mockCallbackMap1
    };

    SystemAbilityManager::AbilityItem mockAbilityItem2 = {
        SystemAbilityManager::AbilityState::INIT, mockCallbackMap2
    };

    saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY1] = mockAbilityItem1;
    saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY2] = mockAbilityItem2;
    saMgr->OnAbilityCallbackDied(mockLoadCallback1->AsObject());
    ASSERT_EQ(saMgr->startingAbilityMap_.size(), 1);
    saMgr->OnAbilityCallbackDied(mockLoadCallback2->AsObject());
    ASSERT_EQ(saMgr->startingAbilityMap_.size(), 0);
}

/**
 * @tc.name: startingAbilityMap_ test
 * @tc.desc: test for callback dead, with one device, some callback, some sa
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, startingAbilityMapTest012, TestSize.Level1)
{
    DTEST_LOG << " startingAbilityMapTest010 start " << std::endl;
    /**
     * @tc.steps: step1. init startingAbilityMap_
     * @tc.expected: step1. init startingAbilityMap_
     */
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);

    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();

    std::map<std::string, SystemAbilityManager::CallbackList> mockCallbackMap1 = {
        {"222222", {{mockLoadCallback1, 0}}},
        {"111111", {{mockLoadCallback1, 1}}}
    };

    std::map<std::string, SystemAbilityManager::CallbackList> mockCallbackMap2 = {
        {"22222", {{mockLoadCallback2, 1}}}
    };
    SystemAbilityManager::AbilityItem mockAbilityItem1 = {
        SystemAbilityManager::AbilityState::INIT, mockCallbackMap1
    };

    SystemAbilityManager::AbilityItem mockAbilityItem2 = {
        SystemAbilityManager::AbilityState::INIT, mockCallbackMap2
    };

    saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY1] = mockAbilityItem1;
    saMgr->startingAbilityMap_[TEST_SYSTEM_ABILITY2] = mockAbilityItem2;
    saMgr->OnAbilityCallbackDied(mockLoadCallback1->AsObject());
    ASSERT_EQ(saMgr->startingAbilityMap_.size(), 1);
    saMgr->OnAbilityCallbackDied(mockLoadCallback2->AsObject());
    ASSERT_EQ(saMgr->startingAbilityMap_.size(), 0);
}

/**
 * @tc.name: OnRemoteCallbackDied001 test
 * @tc.desc: test for callback dead, with one device, some callback
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, OnRemoteCallbackDied001, TestSize.Level1)
{
    DTEST_LOG << " OnRemoteCallbackDied001 start " << std::endl;
    /**
     * @tc.steps: step1. init remoteCallbacks_
     * @tc.expected: step1. init remoteCallbacks_
     */
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);

    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();
    saMgr->remoteCallbacks_ = {
        {"11111", {mockLoadCallback1, mockLoadCallback2}}
    };
    /**
     * @tc.steps: step2. remove nullptr
     * @tc.expected: step2. remove nothing and not crash
     */
    saMgr->OnAbilityCallbackDied(nullptr);
    ASSERT_EQ(saMgr->remoteCallbacks_.size(), 1);
}

/**
 * @tc.name: OnRemoteCallbackDied002 test
 * @tc.desc: test for callback dead, with one device, some callback
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, OnRemoteCallbackDied002, TestSize.Level1)
{
    DTEST_LOG << " OnRemoteCallbackDied002 start " << std::endl;
    /**
     * @tc.steps: step1. init remoteCallbacks_ with one device and one callback
     * @tc.expected: step1. init remoteCallbacks_
     */
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    saMgr->remoteCallbacks_ = {
        {"11111", {mockLoadCallback1}}
    };
    /**
     * @tc.steps: step2. remove one callback
     * @tc.expected: step2. remoteCallbacks_ size 0
     */
    saMgr->OnRemoteCallbackDied(mockLoadCallback1->AsObject());
    ASSERT_EQ(saMgr->remoteCallbacks_.size(), 0);
}

/**
 * @tc.name: OnRemoteCallbackDied003 test
 * @tc.desc: test for callback dead, with one device, some callback
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, OnRemoteCallbackDied003, TestSize.Level1)
{
    DTEST_LOG << " OnRemoteCallbackDied003 start " << std::endl;
    /**
     * @tc.steps: step1. init remoteCallbacks_ with one device and one callback
     * @tc.expected: step1. init remoteCallbacks_
     */
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();
    saMgr->remoteCallbacks_ = {
        {"11111", {mockLoadCallback1}}
    };
    /**
     * @tc.steps: step2. remove other callback
     * @tc.expected: step2. remove nothing
     */
    saMgr->OnRemoteCallbackDied(mockLoadCallback2->AsObject());
    ASSERT_EQ(saMgr->remoteCallbacks_.size(), 1);
}

/**
 * @tc.name: OnRemoteCallbackDied004 test
 * @tc.desc: test for callback dead, with one device, some callback
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, OnRemoteCallbackDied004, TestSize.Level1)
{
    DTEST_LOG << " OnRemoteCallbackDied004 start " << std::endl;
    /**
     * @tc.steps: step1. init remoteCallbacks_
     * @tc.expected: step1. init remoteCallbacks_
     */
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();

    saMgr->remoteCallbacks_ = {
        {"11111", {mockLoadCallback1, mockLoadCallback2}}
    };
    /**
     * @tc.steps: step2. remove one callback
     * @tc.expected: step2. remoteCallbacks_ size 1
     */
    saMgr->OnRemoteCallbackDied(mockLoadCallback1->AsObject());
    ASSERT_EQ(saMgr->remoteCallbacks_["11111"].size(), 1);
    ASSERT_EQ(saMgr->remoteCallbacks_.size(), 1);
}

/**
 * @tc.name: OnRemoteCallbackDied005 test
 * @tc.desc: test for callback dead, with one device, some callback
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, OnRemoteCallbackDied005, TestSize.Level1)
{
    DTEST_LOG << " OnRemoteCallbackDied005 start " << std::endl;
    /**
     * @tc.steps: step1. init remoteCallbacks_
     * @tc.expected: step1. init remoteCallbacks_
     */
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();

    saMgr->remoteCallbacks_ = {
        {"11111", {mockLoadCallback1, mockLoadCallback2}}
    };
    /**
     * @tc.steps: step2. remove all callback
     * @tc.expected: step2. remoteCallbacks_ empty
     */
    saMgr->OnRemoteCallbackDied(mockLoadCallback1->AsObject());
    saMgr->OnRemoteCallbackDied(mockLoadCallback2->AsObject());
    ASSERT_EQ(saMgr->remoteCallbacks_.size(), 0);
}

/**
 * @tc.name: OnRemoteCallbackDied006 test
 * @tc.desc: test for callback dead, with one device, some callback
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, OnRemoteCallbackDied006, TestSize.Level1)
{
    DTEST_LOG << " OnRemoteCallbackDied006 start " << std::endl;
    /**
     * @tc.steps: step1. init remoteCallbacks_
     * @tc.expected: step1. init remoteCallbacks_
     */
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();

    saMgr->remoteCallbacks_ = {
        {"11111", {mockLoadCallback1}},
        {"22222", {mockLoadCallback2}}
    };
    /**
     * @tc.steps: step2. remove all callback
     * @tc.expected: step2. remoteCallbacks_ empty
     */
    saMgr->OnRemoteCallbackDied(mockLoadCallback1->AsObject());
    ASSERT_EQ(saMgr->remoteCallbacks_["22222"].size(), 1);
    ASSERT_EQ(saMgr->remoteCallbacks_.size(), 1);
}

/**
 * @tc.name: OnRemoteCallbackDied007 test
 * @tc.desc: test for callback dead, with one device, some callback
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, OnRemoteCallbackDied007, TestSize.Level1)
{
    DTEST_LOG << " OnRemoteCallbackDied007 start " << std::endl;
    /**
     * @tc.steps: step1. init remoteCallbacks_
     * @tc.expected: step1. init remoteCallbacks_
     */
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();

    saMgr->remoteCallbacks_ = {
        {"11111", {mockLoadCallback1, mockLoadCallback2}},
        {"22222", {mockLoadCallback2}}
    };
    /**
     * @tc.steps: step2. remove mockLoadCallback1
     * @tc.expected: step2. remoteCallbacks_ empty
     */
    saMgr->OnRemoteCallbackDied(mockLoadCallback1->AsObject());
    ASSERT_TRUE(saMgr->remoteCallbacks_.size() > 1);
}

/**
 * @tc.name: OnRemoteCallbackDied008 test
 * @tc.desc: test for callback dead, with one device, some callback
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, OnRemoteCallbackDied008, TestSize.Level1)
{
    DTEST_LOG << " OnRemoteCallbackDied008 start " << std::endl;
    /**
     * @tc.steps: step1. init remoteCallbacks_
     * @tc.expected: step1. init remoteCallbacks_
     */
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);

    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();

    saMgr->remoteCallbacks_ = {
        {"11111", {mockLoadCallback1, mockLoadCallback2}},
        {"22222", {mockLoadCallback2}}
    };
    /**
     * @tc.steps: step2. remove one mockLoadCallback2
     * @tc.expected: step2. remoteCallbacks_ remove all mockLoadCallback2
     */
    saMgr->OnRemoteCallbackDied(mockLoadCallback2->AsObject());
    ASSERT_EQ(saMgr->remoteCallbacks_.size(), 1);
}

/**
 * @tc.name: DoLoadRemoteSystemAbility001 test
 * @tc.desc: test for callback dead, with one device, some callback
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, DoLoadRemoteSystemAbility001, TestSize.Level1)
{
    DTEST_LOG << " DoLoadRemoteSystemAbility001 start " << std::endl;
    /**
     * @tc.steps: step1. init remoteCallbacks_
     * @tc.expected: step1. init remoteCallbacks_
     */
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);

    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();

    saMgr->remoteCallbacks_ = {
        {"11111_111", {mockLoadCallback1, mockLoadCallback2}},
        {"11111_222", {mockLoadCallback2}}
    };
    /**
     * @tc.steps: step2. mockLoadCallback1 load complete
     * @tc.expected: step2. remoteCallbacks_ remove mockLoadCallback1
     */
    saMgr->DoLoadRemoteSystemAbility(11111, 0, 0, "111", mockLoadCallback1);
    ASSERT_EQ(saMgr->remoteCallbacks_["11111_111"].size(), 1);
    ASSERT_TRUE(saMgr->remoteCallbacks_.size() > 1);
}

/**
 * @tc.name: DoLoadRemoteSystemAbility002 test
 * @tc.desc: test for load complete, with one device, one callback
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, DoLoadRemoteSystemAbility002, TestSize.Level1)
{
    DTEST_LOG << " DoLoadRemoteSystemAbility002 start " << std::endl;
    /**
     * @tc.steps: step1. init remoteCallbacks_
     * @tc.expected: step1. init remoteCallbacks_
     */
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);

    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();

    saMgr->remoteCallbacks_ = {
        {"11111_222", {mockLoadCallback2}}
    };
    /**
     * @tc.steps: step2. remove one mockLoadCallback2
     * @tc.expected: step2. remoteCallbacks_ remove all mockLoadCallback2
     */
    saMgr->DoLoadRemoteSystemAbility(11111, 0, 0, "222", mockLoadCallback2);
    ASSERT_EQ(saMgr->remoteCallbacks_.size(), 0);
}

/**
 * @tc.name: DoLoadRemoteSystemAbility003 test
 * @tc.desc: test for load complete, with one device, some callback
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, DoLoadRemoteSystemAbility003, TestSize.Level1)
{
    DTEST_LOG << " DoLoadRemoteSystemAbility003 start " << std::endl;
    /**
     * @tc.steps: step1. init remoteCallbacks_
     * @tc.expected: step1. init remoteCallbacks_
     */
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);

    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();

    saMgr->remoteCallbacks_ = {
        {"11111_111", {mockLoadCallback1, mockLoadCallback2}},
        {"11111_222", {mockLoadCallback2}}
    };
    /**
     * @tc.steps: step2. remove one mockLoadCallback2
     * @tc.expected: step2. remoteCallbacks_ remove all mockLoadCallback2
     */
    saMgr->DoLoadRemoteSystemAbility(11111, 0, 0, "222", mockLoadCallback2);
    ASSERT_EQ(saMgr->remoteCallbacks_.size(), 1);
}

/**
 * @tc.name: DoLoadRemoteSystemAbility004 test
 * @tc.desc: test for load complete, callback is nullptr
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, DoLoadRemoteSystemAbility004, TestSize.Level1)
{
    DTEST_LOG << " DoLoadRemoteSystemAbility004 start " << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    saMgr->DoLoadRemoteSystemAbility(11111, 0, 0, "222", nullptr);
    ASSERT_EQ(saMgr->remoteCallbacks_.size(), 0);
}

/**
 * @tc.name: param check samgr ready event
 * @tc.desc: param check samgr ready event
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, SamgrReady001, TestSize.Level1)
{
    DTEST_LOG << " SamgrReady001 start " << std::endl;
    /**
     * @tc.steps: step1. param check samgr ready event
     * @tc.expected: step1. param check samgr ready event
     */
    auto ret = WaitParameter("bootevent.samgr.ready", "true", 1);
    ASSERT_EQ(ret, 0);
}

/**
 * @tc.name: ReportGetSAFre001
 * @tc.desc: ReportGetSAFre001
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, ReportGetSAFre001, TestSize.Level3)
{
    DTEST_LOG << " ReportGetSAFre001 start " << std::endl;
    ReportGetSAFrequency(1, 1, 1);
    uint32_t realUid = 1;
    uint32_t readSaid = 1;
    uint64_t key = SamgrUtil::GenerateFreKey(realUid, readSaid);
    DTEST_LOG << " key 001 :  " << key << std::endl;
    uint32_t expectSid = static_cast<uint32_t>(key);
    uint32_t expectUid = key >> SHFIT_BIT;
    DTEST_LOG << " key 002 :  " << key << std::endl;
    ASSERT_EQ(expectUid, realUid);
    ASSERT_EQ(readSaid, expectSid);
}

/**
 * @tc.name: ReportGetSAFre002
 * @tc.desc: ReportGetSAFre002
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, ReportGetSAFre002, TestSize.Level3)
{
    DTEST_LOG << " ReportGetSAFre002 start " << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    int32_t uid = 1;
    int32_t count = saMgr->UpdateSaFreMap(uid, TEST_SYSTEM_ABILITY1);
    ASSERT_EQ(saMgr->saFrequencyMap_.size(), 1);
    saMgr->ReportGetSAPeriodically();
    ASSERT_EQ(saMgr->saFrequencyMap_.size(), 0);
}

/**
 * @tc.name: ReportGetSAFre003
 * @tc.desc: ReportGetSAFre003
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, ReportGetSAFre003, TestSize.Level3)
{
    DTEST_LOG << " ReportGetSAFre003 start " << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    int32_t uid = -1;
    int32_t count = saMgr->UpdateSaFreMap(uid, TEST_SYSTEM_ABILITY1);
    saMgr->ReportGetSAPeriodically();
    ASSERT_EQ(saMgr->saFrequencyMap_.size(), 0);
}

/**
 * @tc.name: ReportGetSAFre004
 * @tc.desc: ReportGetSAFre004
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, ReportGetSAFre004, TestSize.Level3)
{
    DTEST_LOG << " ReportGetSAFre004 start " << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    int32_t uid = 1;
    uint64_t key = SamgrUtil::GenerateFreKey(uid, TEST_SYSTEM_ABILITY1);
    saMgr->saFrequencyMap_[key] = MAX_COUNT;
    int32_t count = saMgr->UpdateSaFreMap(uid, TEST_SYSTEM_ABILITY1);
    EXPECT_EQ(saMgr->saFrequencyMap_[key], MAX_COUNT);
}

/**
 * @tc.name: Test GetSystemProcessInfo001
 * @tc.desc: GetRunningSystemProcess001
 * @tc.type: FUNC
 * @tc.require: I7VQQG
 */
HWTEST_F(SystemAbilityMgrTest, GetSystemProcessInfo001, TestSize.Level3)
{
    DTEST_LOG << " GetSystemProcessInfo001 " << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    SystemProcessInfo ProcessInfo;
    int32_t ret = saMgr->GetSystemProcessInfo(SAID, ProcessInfo);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: Test GetSystemProcessInfo002
 * @tc.desc: GetRunningSystemProcess002
 * @tc.type: FUNC
 * @tc.require: I7VQQG
 */
HWTEST_F(SystemAbilityMgrTest, GetSystemProcessInfo002, TestSize.Level3)
{
    DTEST_LOG << " GetSystemProcessInfo002 " << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    SystemProcessInfo ProcessInfo;
    saMgr->abilityStateScheduler_ = nullptr;
    int32_t ret = saMgr->GetSystemProcessInfo(SAID, ProcessInfo);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: Test GetRunningSystemProcess001
 * @tc.desc: GetRunningSystemProcess001
 * @tc.type: FUNC
 * @tc.require: I6H10P
 */
HWTEST_F(SystemAbilityMgrTest, GetRunningSystemProcess001, TestSize.Level3)
{
    DTEST_LOG << " GetRunningSystemProcess001 " << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    std::list<SystemProcessInfo> systemProcessInfos;
    int32_t ret = saMgr->GetRunningSystemProcess(systemProcessInfos);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: Test GetRunningSystemProcess002
 * @tc.desc: GetRunningSystemProcess002
 * @tc.type: FUNC
 * @tc.require: I6H10P
 */
HWTEST_F(SystemAbilityMgrTest, GetRunningSystemProcess002, TestSize.Level3)
{
    DTEST_LOG << " GetRunningSystemProcess002 " << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    std::list<SystemProcessInfo> systemProcessInfos;
    saMgr->abilityStateScheduler_ = nullptr;
    int32_t ret = saMgr->GetRunningSystemProcess(systemProcessInfos);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: GetRunningSystemProcess003
 * @tc.desc: test GetRunningSystemProcess with abilityStateScheduler_ is nullptr
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrTest, GetRunningSystemProcess003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    saMgr->abilityStateScheduler_ = nullptr;
    std::list<SystemProcessInfo> systemProcessInfos;
    int32_t ret = saMgr->GetRunningSystemProcess(systemProcessInfos);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: watchdoginit001
 * @tc.desc: test watchdoginit, waitState is not WAITTING
 * @tc.type: FUNC
 * @tc.require: I6MO6A
 */
HWTEST_F(SystemAbilityMgrTest, WatchDogInit001, TestSize.Level3)
{
    DTEST_LOG << " WatchDogInit001 " << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    EXPECT_NE(saMgr, nullptr);
}

/**
 * @tc.name: AddSystemProcess001
 * @tc.desc: test AddSystemProcess, abilityStateScheduler_ is nullptr
 * @tc.type: FUNC
 * @tc.require: I6MO6A
 */
HWTEST_F(SystemAbilityMgrTest, AddSystemProcess001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    std::u16string procName = u"test";
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    saMgr->abilityStateScheduler_ = nullptr;
    int32_t result = saMgr->AddSystemProcess(procName, testAbility);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: RemoveSystemProcess001
 * @tc.desc: test RemoveSystemProcess, abilityStateScheduler_ is nullptr
 * @tc.type: FUNC
 * @tc.require: I6MO6A
 */
HWTEST_F(SystemAbilityMgrTest, RemoveSystemProcess001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    saMgr->abilityStateScheduler_ = nullptr;
    int32_t result = saMgr->RemoveSystemProcess(testAbility);
    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    std::map<std::string, BaseSystemAbilityManager::CallbackList> mockCallbackMap1 = {
        {"111111", {}}
    };
    BaseSystemAbilityManager::AbilityItem mockAbilityItem1 = {
        BaseSystemAbilityManager::AbilityState::INIT, mockCallbackMap1
    };
    saMgr->RemoveStartingAbilityCallbackForDevice(
        mockAbilityItem1, testAbility);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: RemoveSystemProcess002
 * @tc.desc: test RemoveSystemProcess, abilityStateScheduler_ is nullptr
 * @tc.type: FUNC
 * @tc.require: I6MO6A
 */
HWTEST_F(SystemAbilityMgrTest, RemoveSystemProcess002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    saMgr->abilityStateScheduler_ = nullptr;
    saMgr->systemProcessMap_[u"test"] = testAbility;
    int32_t result = saMgr->RemoveSystemProcess(testAbility);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: OnAbilityCallbackDied001
 * @tc.desc: test OnAbilityCallbackDied with remoteObject is nullptr
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrTest, OnAbilityCallbackDied001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    saMgr->OnAbilityCallbackDied(nullptr);
    EXPECT_TRUE(saMgr->startingAbilityMap_.empty());
}

HWTEST_F(SystemAbilityMgrTest, OnRemoteDied002, TestSize.Level3)
{
    DTEST_LOG<<"OnRemoteDied002 BEGIN";
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    
    saMgr->abilityCallbackDeath_ = sptr<IRemoteObject::DeathRecipient>(
        new AbilityDeathRecipient(std::weak_ptr<BaseSystemAbilityManager>{}));
    EXPECT_NE(nullptr, saMgr->abilityCallbackDeath_);
    saMgr->abilityCallbackDeath_->OnRemoteDied(nullptr);
    
    saMgr->remoteCallbackDeath_ = sptr<IRemoteObject::DeathRecipient>(
        new RemoteCallbackDeathRecipient(std::weak_ptr<BaseSystemAbilityManager>{}));
    EXPECT_NE(nullptr, saMgr->remoteCallbackDeath_);
    saMgr->remoteCallbackDeath_->OnRemoteDied(nullptr);
    DTEST_LOG<<"OnRemoteDied002 END";
}

/**
 * @tc.name: Test GetLocalAbilityManagerProxy001
 * @tc.desc: GetLocalAbilityManagerProxy001
 * @tc.type: FUNC
 * @tc.require: I7VQQG
 */
HWTEST_F(SystemAbilityMgrTest, GetLocalAbilityManagerProxy001, TestSize.Level3)
{
    DTEST_LOG << "GetLocalAbilityManagerProxy001 BEGIN" << std::endl;

    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    saMgr->saProfileMap_.clear();
    auto ret = saMgr->GetLocalAbilityManagerProxy(SAID);
    EXPECT_TRUE(ret == nullptr);
    DTEST_LOG << "GetLocalAbilityManagerProxy001 END" << std::endl;
}

/**
 * @tc.name: Test GetLocalAbilityManagerProxy002
 * @tc.desc: GetLocalAbilityManagerProxy001
 * @tc.type: FUNC
 * @tc.require: I7VQQG
 */
HWTEST_F(SystemAbilityMgrTest, GetLocalAbilityManagerProxy002, TestSize.Level3)
{
    DTEST_LOG << "GetLocalAbilityManagerProxy002 BEGIN" << std::endl;

    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    CommonSaProfile saProfile;
    saProfile.process = u"test";
    saProfile.saId = SAID;
    saMgr->saProfileMap_[SAID] = saProfile;

    sptr<IRemoteObject> testAbility = new TestTransactionService();
    EXPECT_FALSE(testAbility == nullptr);
    saMgr->systemProcessMap_[u"test"] = testAbility;

    auto ret = saMgr->GetLocalAbilityManagerProxy(SAID);
    EXPECT_FALSE(ret == nullptr);

    DTEST_LOG << "GetLocalAbilityManagerProxy002 END" << std::endl;
}

/**
 * @tc.name: UnloadProcess001
 * @tc.desc: Test UnloadProcess
 * @tc.type: FUNC
 * @tc.require: I7VQQG
 */
HWTEST_F(SystemAbilityMgrTest, UnloadProcess001, TestSize.Level3)
{
    DTEST_LOG << "UnloadProcess001 BEGIN" << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    std::vector<std::u16string> processList;
    int32_t ret = saMgr->UnloadProcess(processList);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    saMgr->abilityStateScheduler_ = std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
    ret = saMgr->UnloadProcess(processList);
    EXPECT_NE(ret, ERR_INVALID_VALUE);
    DTEST_LOG << "UnloadProcess001 END" << std::endl;
}

/**
 * @tc.name: SetSamgrIpcPrior001
 * @tc.desc: Test SetSamgrIpcPrior with enable=true
 * @tc.type: FUNC
 * @tc.require: I7VQQG
 */
HWTEST_F(SystemAbilityMgrTest, SetSamgrIpcPrior001, TestSize.Level3)
{
    DTEST_LOG << "SetSamgrIpcPrior001 BEGIN" << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    // Initialize necessary members
    saMgr->workHandler_ = make_shared<FFRTHandler>("workHandler");
    saMgr->isSupportSetPrior_ = true;

    // Test enabling IPC priority
    int32_t result = saMgr->SetSamgrIpcPrior(true);
    EXPECT_EQ(result, ERR_OK);

    // Verify priorEnable_ is set to true
    EXPECT_TRUE(saMgr->priorEnable_);

    // Verify reference count is incremented
    EXPECT_EQ(saMgr->priorRefCnt_, 1);
    saMgr->workHandler_->CleanFfrt();
    DTEST_LOG << "SetSamgrIpcPrior001 END" << std::endl;
}

/**
 * @tc.name: SetSamgrIpcPrior002
 * @tc.desc: Test SetSamgrIpcPrior with enable=false
 * @tc.type: FUNC
 * @tc.require: I7VQQG
 */
HWTEST_F(SystemAbilityMgrTest, SetSamgrIpcPrior002, TestSize.Level3)
{
    DTEST_LOG << "SetSamgrIpcPrior002 BEGIN" << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    // Initialize necessary members
    saMgr->workHandler_ = make_shared<FFRTHandler>("workHandler");
    saMgr->isSupportSetPrior_ = true;

    // First enable
    saMgr->SetSamgrIpcPrior(true);
    EXPECT_TRUE(saMgr->priorEnable_);

    // Then disable
    int32_t result = saMgr->SetSamgrIpcPrior(false);
    EXPECT_EQ(result, ERR_OK);

    // Verify priorEnable_ is set to false
    EXPECT_FALSE(saMgr->priorEnable_);

    // Verify reference count is decremented
    EXPECT_EQ(saMgr->priorRefCnt_, 0);
    saMgr->workHandler_->CleanFfrt();
    DTEST_LOG << "SetSamgrIpcPrior002 END" << std::endl;
}


/**
 * @tc.name: SetSamgrIpcPrior003
 * @tc.desc: Test SetSamgrIpcPrior reference counting mechanism
 * @tc.type: FUNC
 * @tc.require: I7VQQG
 */
HWTEST_F(SystemAbilityMgrTest, SetSamgrIpcPrior003, TestSize.Level3)
{
    DTEST_LOG << "SetSamgrIpcPrior003 BEGIN" << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    // Initialize necessary members
    saMgr->workHandler_ = make_shared<FFRTHandler>("workHandler");
    saMgr->isSupportSetPrior_ = true;

    // Reset state
    saMgr->priorRefCnt_ = 0;
    saMgr->priorEnable_ = false;

    // First enable
    int32_t result1 = saMgr->SetSamgrIpcPrior(true);
    EXPECT_EQ(result1, ERR_OK);
    EXPECT_TRUE(saMgr->priorEnable_);
    EXPECT_EQ(saMgr->priorRefCnt_, 1);

    // Second enable (should increment ref count)
    int32_t result2 = saMgr->SetSamgrIpcPrior(true);
    EXPECT_EQ(result2, ERR_OK);
    EXPECT_TRUE(saMgr->priorEnable_);
    EXPECT_EQ(saMgr->priorRefCnt_, 2);

    // First disable (ref count > 1, should not disable yet)
    int32_t result3 = saMgr->SetSamgrIpcPrior(false);
    EXPECT_EQ(result3, ERR_OK);
    EXPECT_TRUE(saMgr->priorEnable_); // Should still be enabled
    EXPECT_EQ(saMgr->priorRefCnt_, 1);

    // Second disable (ref count == 1, should disable now)
    int32_t result4 = saMgr->SetSamgrIpcPrior(false);
    EXPECT_EQ(result4, ERR_OK);
    EXPECT_FALSE(saMgr->priorEnable_); // Should be disabled now
    EXPECT_EQ(saMgr->priorRefCnt_, 0);
    saMgr->workHandler_->CleanFfrt();
    DTEST_LOG << "SetSamgrIpcPrior003 END" << std::endl;
}

/**
 * @tc.name: SetSamgrIpcPrior004
 * @tc.desc: Test SetSamgrIpcPrior with multiple consecutive enable calls
 * @tc.type: FUNC
 * @tc.require: I7VQQG
 */
HWTEST_F(SystemAbilityMgrTest, SetSamgrIpcPrior004, TestSize.Level3)
{
    DTEST_LOG << "SetSamgrIpcPrior004 BEGIN" << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    // Initialize necessary members
    saMgr->workHandler_ = make_shared<FFRTHandler>("workHandler");
    saMgr->isSupportSetPrior_ = true;

    // Reset state
    saMgr->priorRefCnt_ = 0;
    saMgr->priorEnable_ = false;

    // Multiple enable calls
    for (int32_t i = 0; i < 5; i++) {
        int32_t result = saMgr->SetSamgrIpcPrior(true);
        EXPECT_EQ(result, ERR_OK);
        EXPECT_TRUE(saMgr->priorEnable_);
        EXPECT_EQ(saMgr->priorRefCnt_, i + 1);
    }
    saMgr->workHandler_->CleanFfrt();
    DTEST_LOG << "SetSamgrIpcPrior004 END" << std::endl;
}

/**
 * @tc.name: SetSamgrIpcPrior005
 * @tc.desc: Test SetSamgrIpcPrior under stress conditions
 * @tc.type: FUNC
 * @tc.require: I7VQQG
 */
HWTEST_F(SystemAbilityMgrTest, SetSamgrIpcPrior005, TestSize.Level3)
{
    DTEST_LOG << "SetSamgrIpcPrior005 BEGIN" << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    // Initialize necessary members
    saMgr->workHandler_ = make_shared<FFRTHandler>("workHandler");
    saMgr->isSupportSetPrior_ = true;
    saMgr->priorRefCnt_ = 0;
    saMgr->priorEnable_ = false;

    // Stress test with multiple enable/disable cycles
    for (int32_t i = 0; i < 100; i++) {
        int32_t result1 = saMgr->SetSamgrIpcPrior(true);
        EXPECT_EQ(result1, ERR_OK);

        int32_t result2 = saMgr->SetSamgrIpcPrior(false);
        EXPECT_EQ(result2, ERR_OK);

        EXPECT_EQ(saMgr->priorRefCnt_, 0);
        EXPECT_FALSE(saMgr->priorEnable_);
    }
    saMgr->workHandler_->CleanFfrt();
    DTEST_LOG << "SetSamgrIpcPrior005 END" << std::endl;
}

/**
 * @tc.name: SetSamgrIpcPrior006
 * @tc.desc: Test SetSamgrIpcPrior with enable=false when already disabled
 * @tc.type: FUNC
 * @tc.require: I7VQQG
 */
HWTEST_F(SystemAbilityMgrTest, SetSamgrIpcPrior006, TestSize.Level2)
{
    DTEST_LOG << "SetSamgrIpcPrior006 start" << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);

    // Initialize necessary members
    saMgr->workHandler_ = make_shared<FFRTHandler>("workHandler");
    saMgr->isSupportSetPrior_ = true;
    saMgr->priorRefCnt_ = 0;
    saMgr->priorEnable_ = false;

    // Try to disable when already disabled
    int32_t result = saMgr->SetSamgrIpcPrior(false);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_FALSE(saMgr->priorEnable_);
    EXPECT_EQ(saMgr->priorRefCnt_, 0);
    saMgr->workHandler_->CleanFfrt();

    DTEST_LOG << "SetSamgrIpcPrior006 end" << std::endl;
}

/**
 * @tc.name: SetSamgrIpcPrior007
 * @tc.desc: Test SetSamgrIpcPrior with enable=true when already enabled
 * @tc.type: FUNC
 * @tc.require: I7VQQG
 */
HWTEST_F(SystemAbilityMgrTest, SetSamgrIpcPrior007, TestSize.Level2)
{
    DTEST_LOG << "SetSamgrIpcPrior007 start" << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);

    // Initialize necessary members
    saMgr->workHandler_ = make_shared<FFRTHandler>("workHandler");
    saMgr->isSupportSetPrior_ = true;
    saMgr->priorRefCnt_ = 0;
    saMgr->priorEnable_ = false;

    // First enable
    saMgr->SetSamgrIpcPrior(true);
    int32_t initialRefCount = saMgr->priorRefCnt_;

    // Enable again
    int32_t result = saMgr->SetSamgrIpcPrior(true);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_TRUE(saMgr->priorEnable_);
    EXPECT_EQ(saMgr->priorRefCnt_, initialRefCount + 1);
    saMgr->workHandler_->CleanFfrt();

    DTEST_LOG << "SetSamgrIpcPrior007 end" << std::endl;
}
} // namespace OHOS

#ifdef SUPPORT_MULTI_INSTANCE
namespace OHOS {
/**
 * @tc.name: MultiUserExplicitQuery001
 * @tc.desc: Test explicit-user query APIs with an active multi-user manager.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, MultiUserExplicitQuery001, TestSize.Level3)
{
    constexpr int32_t testUserId = 100;
    constexpr int32_t testSaId = 2235;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    InitUserLifecycleManager(saMgr);
    ASSERT_EQ(saMgr->OnUserStateChanged(testUserId, USER_STATE_ACTIVATING), ERR_OK);
    ASSERT_EQ(saMgr->OnUserStateChanged(testUserId, USER_STATE_SWITCHING), ERR_OK);

    bool isExist = true;
    SystemProcessInfo processInfo;
    EXPECT_EQ(saMgr->GetSystemAbility(testSaId, testUserId), nullptr);
    EXPECT_EQ(saMgr->CheckSystemAbility(testSaId, testUserId), nullptr);
    EXPECT_EQ(saMgr->CheckSystemAbilityByUserId(testSaId, isExist, testUserId), nullptr);
    EXPECT_FALSE(isExist);
    EXPECT_EQ(saMgr->GetSystemProcessInfo(testSaId, processInfo, testUserId), ERR_INVALID_VALUE);
    EXPECT_EQ(saMgr->GetLocalAbilityManagerProxy(testSaId, testUserId), nullptr);

    sptr<ISystemAbilityLoadCallback> loadCallback = nullptr;
    sptr<ISystemAbilityStatusChange> statusListener = nullptr;
    sptr<ISystemProcessStatusChange> processListener = nullptr;
    EXPECT_NE(saMgr->LoadSystemAbility(testSaId, loadCallback, testUserId), ERR_INVALID_VALUE);
    EXPECT_EQ(saMgr->SubscribeSystemAbility(testSaId, statusListener, testUserId), ERR_INVALID_VALUE);
    EXPECT_EQ(saMgr->UnSubscribeSystemAbility(testSaId, statusListener, testUserId), ERR_INVALID_VALUE);
    EXPECT_EQ(saMgr->SubscribeSystemProcess(processListener, testUserId), ERR_INVALID_VALUE);
    EXPECT_EQ(saMgr->UnSubscribeSystemProcess(processListener, testUserId), ERR_INVALID_VALUE);

    auto manager = saMgr->GetMultiUserManager(testUserId);
    ASSERT_NE(manager, nullptr);
    manager->Destroy();
    saMgr->userLifecycleManager_.multiUserManagers_.clear();

    saMgr->userLifecycleManager_.validUserIds_.insert(testUserId);
    EXPECT_EQ(saMgr->GetSystemAbility(testSaId, testUserId), nullptr);
    EXPECT_EQ(saMgr->CheckSystemAbility(testSaId, testUserId), nullptr);
    isExist = true;
    EXPECT_EQ(saMgr->CheckSystemAbilityByUserId(testSaId, isExist, testUserId), nullptr);
    EXPECT_FALSE(isExist);
    EXPECT_EQ(saMgr->GetSystemProcessInfo(testSaId, processInfo, testUserId), ERR_INVALID_VALUE);
    EXPECT_EQ(saMgr->GetLocalAbilityManagerProxy(testSaId, testUserId), nullptr);
    EXPECT_EQ(saMgr->LoadSystemAbility(testSaId, loadCallback, testUserId), ERR_INVALID_VALUE);
    EXPECT_EQ(saMgr->SubscribeSystemAbility(testSaId, statusListener, testUserId), ERR_INVALID_VALUE);
    EXPECT_EQ(saMgr->UnSubscribeSystemAbility(testSaId, statusListener, testUserId), ERR_INVALID_VALUE);
    EXPECT_EQ(saMgr->SubscribeSystemProcess(processListener, testUserId), ERR_INVALID_VALUE);
    EXPECT_EQ(saMgr->UnSubscribeSystemProcess(processListener, testUserId), ERR_INVALID_VALUE);
}

/**
 * @tc.name: MultiUserExplicitInvalidUser001
 * @tc.desc: Test explicit-user APIs reject an invalid target user before manager lookup.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, MultiUserExplicitInvalidUser001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    sptr<ISystemAbilityLoadCallback> loadCallback = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityStatusChange> abilityListener = new SaStatusChangeMock();
    sptr<ISystemProcessStatusChange> processListener = new SystemProcessStatusChange();
    SystemProcessInfo processInfo;
    bool isExist = true;

    EXPECT_EQ(saMgr->GetSystemAbility(SAID, SAMGR_INVALID_USER_ID), nullptr);
    EXPECT_EQ(saMgr->CheckSystemAbility(SAID, SAMGR_INVALID_USER_ID), nullptr);
    EXPECT_EQ(saMgr->CheckSystemAbilityByUserId(SAID, isExist, SAMGR_INVALID_USER_ID), nullptr);
    EXPECT_FALSE(isExist);
    EXPECT_EQ(saMgr->GetSystemProcessInfo(SAID, processInfo, SAMGR_INVALID_USER_ID), INVALID_CALLING_USER_ID);
    EXPECT_EQ(saMgr->GetLocalAbilityManagerProxy(SAID, SAMGR_INVALID_USER_ID), nullptr);
    EXPECT_EQ(saMgr->LoadSystemAbility(SAID, loadCallback, SAMGR_INVALID_USER_ID), INVALID_CALLING_USER_ID);
    EXPECT_EQ(saMgr->SubscribeSystemAbility(SAID, abilityListener, SAMGR_INVALID_USER_ID), INVALID_CALLING_USER_ID);
    EXPECT_EQ(saMgr->UnSubscribeSystemAbility(SAID, abilityListener, SAMGR_INVALID_USER_ID), INVALID_CALLING_USER_ID);
    EXPECT_EQ(saMgr->SubscribeSystemProcess(processListener, SAMGR_INVALID_USER_ID), INVALID_CALLING_USER_ID);
    EXPECT_EQ(saMgr->UnSubscribeSystemProcess(processListener, SAMGR_INVALID_USER_ID), INVALID_CALLING_USER_ID);
}

/**
 * @tc.name: MultiUserRoutingDecision001
 * @tc.desc: Test base-user and explicit-user routing for multi-instance abilities.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, MultiUserRoutingDecision001, TestSize.Level3)
{
    constexpr int32_t userId = 124;
    constexpr int32_t multiInstanceSaId = 2236;
    constexpr int32_t regularSaId = 2237;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    ASSERT_NE(saMgr, nullptr);
    saMgr->multiInstanceSaIds_.insert(multiInstanceSaId);
    saMgr->userLifecycleManager_.validUserIds_.insert(userId);
    saMgr->userLifecycleManager_.foregroundUserId_.store(userId);

    EXPECT_TRUE(saMgr->IsValidCallingUserId(BASE_USER));
    EXPECT_TRUE(saMgr->IsValidCallingUserId(userId));
    EXPECT_FALSE(saMgr->IsValidCallingUserId(SAMGR_INVALID_USER_ID));
    EXPECT_EQ(saMgr->RouteForUser(regularSaId, userId), BASE_USER);
    EXPECT_EQ(saMgr->RouteForUser(multiInstanceSaId, BASE_USER), userId);
    EXPECT_EQ(saMgr->RouteForUser(multiInstanceSaId, userId), userId);
    EXPECT_EQ(saMgr->RouteForSa(multiInstanceSaId, BASE_USER), SA_OPERATION_NOT_ALLOWED);
    EXPECT_EQ(saMgr->RouteForSa(regularSaId, BASE_USER), SAMGR_OK);
    EXPECT_EQ(saMgr->RouteForSa(multiInstanceSaId, userId), SAMGR_OK);
    EXPECT_EQ(saMgr->RouteForSa(regularSaId, userId), SA_OPERATION_NOT_ALLOWED);
}

/**
 * @tc.name: MultiUserImplicitQueryRouting001
 * @tc.desc: Test base-user implicit queries route multi-instance abilities to the foreground user.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, MultiUserImplicitQueryRouting001, TestSize.Level3)
{
    constexpr int32_t userId = 125;
    constexpr int32_t multiInstanceSaId = 2238;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    InitUserLifecycleManager(saMgr);
    saMgr->multiInstanceSaIds_.insert(multiInstanceSaId);
    ASSERT_EQ(saMgr->OnUserStateChanged(userId, USER_STATE_ACTIVATING), ERR_OK);
    ASSERT_EQ(saMgr->OnUserStateChanged(userId, USER_STATE_SWITCHING), ERR_OK);

    bool isExist = true;
    SystemProcessInfo processInfo;
    sptr<ISystemAbilityLoadCallback> callback = nullptr;
    EXPECT_EQ(saMgr->GetSystemAbility(multiInstanceSaId), nullptr);
    EXPECT_EQ(saMgr->CheckSystemAbility(multiInstanceSaId), nullptr);
    EXPECT_EQ(saMgr->CheckSystemAbility(multiInstanceSaId, isExist), nullptr);
    EXPECT_FALSE(isExist);
    EXPECT_EQ(saMgr->GetSystemProcessInfo(multiInstanceSaId, processInfo), ERR_INVALID_VALUE);
    EXPECT_EQ(saMgr->GetLocalAbilityManagerProxy(multiInstanceSaId), nullptr);
    EXPECT_NE(saMgr->LoadSystemAbility(multiInstanceSaId, callback), ERR_OK);
    EXPECT_EQ(saMgr->OnUserStateChanged(userId, USER_STATE_STOPPING), ERR_OK);
}

/**
 * @tc.name: MultiUserGlobalSubscriptionRouting001
 * @tc.desc: Test base-user subscriptions are propagated to an active multi-user manager.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, MultiUserGlobalSubscriptionRouting001, TestSize.Level3)
{
    constexpr int32_t userId = 126;
    constexpr int32_t multiInstanceSaId = 2239;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    InitUserLifecycleManager(saMgr);
    saMgr->multiInstanceSaIds_.insert(multiInstanceSaId);
    sptr<ISystemAbilityStatusChange> saListener = new SaStatusChangeMock();
    sptr<ISystemProcessStatusChange> processListener = new SystemProcessStatusChange();
    ASSERT_EQ(saMgr->OnUserStateChanged(userId, USER_STATE_ACTIVATING), ERR_OK);

    EXPECT_EQ(saMgr->SubscribeSystemAbility(multiInstanceSaId, saListener), ERR_OK);
    EXPECT_EQ(saMgr->UnSubscribeSystemAbility(multiInstanceSaId, saListener), ERR_OK);
    EXPECT_EQ(saMgr->SubscribeSystemProcess(processListener), ERR_OK);
    EXPECT_EQ(saMgr->UnSubscribeSystemProcess(processListener), ERR_OK);
    EXPECT_EQ(saMgr->OnUserStateChanged(userId, USER_STATE_STOPPING), ERR_OK);
}

/**
 * @tc.name: MultiUserSendStrategyRouting001
 * @tc.desc: Test strategy dispatch separates base and multi-instance ability IDs.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, MultiUserSendStrategyRouting001, TestSize.Level3)
{
    constexpr int32_t userId = 128;
    constexpr int32_t multiInstanceSaId = 2240;
    constexpr int32_t regularSaId = 2241;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    InitUserLifecycleManager(saMgr);
    saMgr->multiInstanceSaIds_.insert(multiInstanceSaId);
    ASSERT_EQ(saMgr->OnUserStateChanged(userId, USER_STATE_ACTIVATING), ERR_OK);
    SamMockPermission::MockProcess("resource_schedule_service");
    std::vector<int32_t> emptyIds;
    std::vector<int32_t> mixedIds { regularSaId, multiInstanceSaId };
    std::vector<int32_t> unknownIds { 2242 };
    std::string action = "strategy";

    EXPECT_EQ(saMgr->SendStrategy(0, emptyIds, 0, action), ERR_OK);
    EXPECT_EQ(saMgr->SendStrategy(0, mixedIds, 0, action), ERR_INVALID_VALUE);
    EXPECT_EQ(saMgr->SendStrategy(0, unknownIds, 0, action), ERR_INVALID_VALUE);
    EXPECT_EQ(saMgr->OnUserStateChanged(userId, USER_STATE_STOPPING), ERR_OK);
}

/**
 * @tc.name: MultiUserExplicitApiInvalidUser001
 * @tc.desc: Verify explicit user APIs reject an unknown target user consistently.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, MultiUserExplicitApiInvalidUser001, TestSize.Level3)
{
    constexpr int32_t invalidUserId = -1;
    constexpr int32_t systemAbilityId = 2243;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    sptr<ISystemAbilityStatusChange> statusListener = new SaStatusChangeMock();
    sptr<ISystemProcessStatusChange> processListener = new SystemProcessStatusChange();
    sptr<ISystemAbilityLoadCallback> callback = nullptr;
    SystemProcessInfo processInfo;
    bool isExist = true;
    ASSERT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
    InitUserLifecycleManager(saMgr);

    EXPECT_EQ(saMgr->OnUserStateChanged(invalidUserId, USER_STATE_ACTIVATING), ERR_INVALID_VALUE);
    EXPECT_EQ(saMgr->GetSystemAbility(systemAbilityId, invalidUserId), nullptr);
    EXPECT_EQ(saMgr->CheckSystemAbility(systemAbilityId, invalidUserId), nullptr);
    EXPECT_EQ(saMgr->CheckSystemAbilityByUserId(systemAbilityId, isExist, invalidUserId), nullptr);
    EXPECT_FALSE(isExist);
    EXPECT_EQ(saMgr->GetSystemProcessInfo(systemAbilityId, processInfo, invalidUserId), INVALID_CALLING_USER_ID);
    EXPECT_EQ(saMgr->GetLocalAbilityManagerProxy(systemAbilityId, invalidUserId), nullptr);
    EXPECT_EQ(saMgr->LoadSystemAbility(systemAbilityId, callback, invalidUserId), INVALID_CALLING_USER_ID);
    EXPECT_EQ(saMgr->SubscribeSystemAbility(systemAbilityId, statusListener, invalidUserId), INVALID_CALLING_USER_ID);
    EXPECT_EQ(saMgr->UnSubscribeSystemAbility(systemAbilityId, statusListener, invalidUserId), INVALID_CALLING_USER_ID);
    EXPECT_EQ(saMgr->SubscribeSystemProcess(processListener, invalidUserId), INVALID_CALLING_USER_ID);
    EXPECT_EQ(saMgr->UnSubscribeSystemProcess(processListener, invalidUserId), INVALID_CALLING_USER_ID);
}

/**
 * @tc.name: MultiUserExplicitApiMissingManager001
 * @tc.desc: Verify explicit user APIs reject a valid user whose manager has been removed.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, MultiUserExplicitApiMissingManager001, TestSize.Level3)
{
    constexpr int32_t userId = 129;
    constexpr int32_t systemAbilityId = 2244;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    sptr<ISystemAbilityStatusChange> statusListener = new SaStatusChangeMock();
    sptr<ISystemProcessStatusChange> processListener = new SystemProcessStatusChange();
    sptr<ISystemAbilityLoadCallback> callback = nullptr;
    SystemProcessInfo processInfo;
    bool isExist = true;
    ASSERT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
    InitUserLifecycleManager(saMgr);
    ASSERT_EQ(saMgr->OnUserStateChanged(userId, USER_STATE_ACTIVATING), ERR_OK);
    saMgr->userLifecycleManager_.multiUserManagers_.erase(userId);

    EXPECT_EQ(saMgr->GetSystemAbility(systemAbilityId, userId), nullptr);
    EXPECT_EQ(saMgr->CheckSystemAbility(systemAbilityId, userId), nullptr);
    EXPECT_EQ(saMgr->CheckSystemAbilityByUserId(systemAbilityId, isExist, userId), nullptr);
    EXPECT_FALSE(isExist);
    EXPECT_EQ(saMgr->GetSystemProcessInfo(systemAbilityId, processInfo, userId), ERR_INVALID_VALUE);
    EXPECT_EQ(saMgr->GetLocalAbilityManagerProxy(systemAbilityId, userId), nullptr);
    EXPECT_EQ(saMgr->LoadSystemAbility(systemAbilityId, callback, userId), ERR_INVALID_VALUE);
    EXPECT_EQ(saMgr->SubscribeSystemAbility(systemAbilityId, statusListener, userId), ERR_INVALID_VALUE);
    EXPECT_EQ(saMgr->UnSubscribeSystemAbility(systemAbilityId, statusListener, userId), ERR_INVALID_VALUE);
    EXPECT_EQ(saMgr->SubscribeSystemProcess(processListener, userId), ERR_INVALID_VALUE);
    EXPECT_EQ(saMgr->UnSubscribeSystemProcess(processListener, userId), ERR_INVALID_VALUE);
}

/**
 * @tc.name: MultiUserExplicitQuerySuccess002
 * @tc.desc: Verify explicit-user query APIs forward to an active user manager.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, MultiUserExplicitQuerySuccess002, TestSize.Level3)
{
    constexpr int32_t userId = 130;
    constexpr int32_t processPid = 321;
    constexpr int32_t processUid = 654;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    auto manager = std::make_shared<MultiSystemAbilityManager>(userId);
    ASSERT_NE(saMgr, nullptr);
    ASSERT_NE(manager, nullptr);
    manager->Init({});
    saMgr->userLifecycleManager_.validUserIds_.insert(userId);
    saMgr->userLifecycleManager_.multiUserManagers_[userId] = manager;
    sptr<IRemoteObject> ability = new TestTransactionService();
    manager->abilityMap_[SAID] = {ability, false};
    CommonSaProfile profile;
    profile.process = PROCESS_NAME;
    manager->saProfileMap_[SAID] = profile;
    manager->systemProcessMap_[PROCESS_NAME] = ability;
    auto abilityContext = std::make_shared<SystemAbilityContext>();
    abilityContext->ownProcessContext = std::make_shared<SystemProcessContext>();
    abilityContext->ownProcessContext->pid = processPid;
    abilityContext->ownProcessContext->uid = processUid;
    manager->abilityStateScheduler_->abilityContextMap_[SAID] = abilityContext;
    bool isExist = false;
    SystemProcessInfo processInfo;
    EXPECT_EQ(saMgr->GetSystemAbility(SAID, userId), ability);
    EXPECT_EQ(saMgr->CheckSystemAbility(SAID, userId), ability);
    EXPECT_EQ(saMgr->CheckSystemAbilityByUserId(SAID, isExist, userId), ability);
    EXPECT_TRUE(isExist);
    EXPECT_EQ(saMgr->GetSystemProcessInfo(SAID, processInfo, userId), ERR_OK);
    EXPECT_EQ(saMgr->GetLocalAbilityManagerProxy(SAID, userId), ability);
    manager->Destroy();
}

/**
 * @tc.name: MultiUserExplicitSubscriptionSuccess002
 * @tc.desc: Verify explicit-user subscription APIs forward successfully.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, MultiUserExplicitSubscriptionSuccess002, TestSize.Level3)
{
    constexpr int32_t userId = 131;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    auto manager = std::make_shared<MultiSystemAbilityManager>(userId);
    ASSERT_NE(saMgr, nullptr);
    ASSERT_NE(manager, nullptr);
    ASSERT_EQ(manager->Init({}), ERR_OK);
    saMgr->userLifecycleManager_.validUserIds_.insert(userId);
    saMgr->userLifecycleManager_.multiUserManagers_[userId] = manager;
    sptr<ISystemAbilityStatusChange> abilityListener = new SaStatusChangeMock();
    sptr<ISystemProcessStatusChange> processListener = new SystemProcessStatusChange();
    EXPECT_EQ(saMgr->SubscribeSystemAbility(SAID, abilityListener, userId), ERR_OK);
    EXPECT_EQ(saMgr->UnSubscribeSystemAbility(SAID, abilityListener, userId), ERR_OK);
    EXPECT_EQ(saMgr->SubscribeSystemProcess(processListener, userId), ERR_OK);
    EXPECT_EQ(saMgr->UnSubscribeSystemProcess(processListener, userId), ERR_OK);
    manager->Destroy();
    EXPECT_TRUE(manager->listenerMap_[SAID].empty());
}

/**
 * @tc.name: MultiUserImplicitQuerySuccess002
 * @tc.desc: Verify implicit multi-instance queries route to the foreground manager.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, MultiUserImplicitQuerySuccess002, TestSize.Level3)
{
    constexpr int32_t userId = 132;
    constexpr int32_t multiInstanceSaId = 2245;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    auto manager = std::make_shared<MultiSystemAbilityManager>(userId);
    ASSERT_NE(saMgr, nullptr);
    ASSERT_NE(manager, nullptr);
    ASSERT_EQ(manager->Init({}), ERR_OK);
    saMgr->multiInstanceSaIds_.insert(multiInstanceSaId);
    saMgr->userLifecycleManager_.validUserIds_.insert(userId);
    saMgr->userLifecycleManager_.foregroundUserId_.store(userId);
    saMgr->userLifecycleManager_.multiUserManagers_[userId] = manager;
    sptr<IRemoteObject> ability = new TestTransactionService();
    manager->abilityMap_[multiInstanceSaId] = {ability, false};
    bool isExist = false;
    EXPECT_EQ(saMgr->GetSystemAbility(multiInstanceSaId), ability);
    EXPECT_EQ(saMgr->CheckSystemAbility(multiInstanceSaId), ability);
    EXPECT_EQ(saMgr->CheckSystemAbility(multiInstanceSaId, isExist), ability);
    EXPECT_TRUE(isExist);
}

/**
 * @tc.name: MultiUserAggregateQueryBranches003
 * @tc.desc: Verify aggregate queries include an active multi-user manager.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, MultiUserAggregateQueryBranches003, TestSize.Level3)
{
    constexpr int32_t userId = 133;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    auto manager = std::make_shared<MultiSystemAbilityManager>(userId);
    ASSERT_NE(saMgr, nullptr);
    ASSERT_NE(manager, nullptr);
    InitSaMgr(saMgr);
    ASSERT_EQ(manager->Init({}), ERR_OK);
    saMgr->userLifecycleManager_.validUserIds_.insert(userId);
    saMgr->userLifecycleManager_.multiUserManagers_[userId] = manager;
    std::list<SystemProcessInfo> processInfos;
    std::vector<sptr<IRemoteObject>> abilityList;
    std::vector<ISystemAbilityManager::SaExtensionInfo> extensionInfos;
    EXPECT_EQ(saMgr->GetRunningSystemProcess(processInfos), ERR_OK);
    EXPECT_EQ(saMgr->GetExtensionRunningSaList("test", abilityList), ERR_OK);
    EXPECT_EQ(saMgr->GetRunningSaExtensionInfoList("test", extensionInfos), ERR_OK);
    manager->Destroy();
    EXPECT_TRUE(processInfos.empty());
}

/**
 * @tc.name: MultiUserExplicitLoadSuccess003
 * @tc.desc: Verify explicit-user loading forwards an already loaded ability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, MultiUserExplicitLoadSuccess003, TestSize.Level3)
{
    constexpr int32_t userId = 134;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    auto manager = std::make_shared<MultiSystemAbilityManager>(userId);
    ASSERT_NE(saMgr, nullptr);
    ASSERT_NE(manager, nullptr);
    ASSERT_EQ(manager->Init({}), ERR_OK);
    saMgr->userLifecycleManager_.validUserIds_.insert(userId);
    saMgr->userLifecycleManager_.multiUserManagers_[userId] = manager;
    sptr<IRemoteObject> ability = new TestTransactionService();
    manager->abilityMap_[SAID] = {ability, false};
    CommonSaProfile profile;
    profile.process = PROCESS_NAME;
    manager->saProfileMap_[SAID] = profile;
    auto abilityContext = std::make_shared<SystemAbilityContext>();
    abilityContext->systemAbilityId = SAID;
    abilityContext->state = SystemAbilityState::LOADED;
    abilityContext->ownProcessContext = std::make_shared<SystemProcessContext>();
    abilityContext->ownProcessContext->state = SystemProcessState::STARTED;
    manager->abilityStateScheduler_->abilityContextMap_[SAID] = abilityContext;
    sptr<ISystemAbilityLoadCallback> callback = new SystemAbilityLoadCallbackMock();
    EXPECT_EQ(saMgr->LoadSystemAbility(SAID, callback, userId), ERR_OK);
    EXPECT_EQ(manager->abilityMap_[SAID].remoteObj, ability);
    manager->Destroy();
}

/**
 * @tc.name: MultiUserUnloadProcess004
 * @tc.desc: Verify process unloading visits an active user manager.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, MultiUserUnloadProcess004, TestSize.Level3)
{
    constexpr int32_t userId = 135;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    auto manager = std::make_shared<MultiSystemAbilityManager>(userId);
    ASSERT_NE(saMgr, nullptr);
    ASSERT_NE(manager, nullptr);
    InitSaMgr(saMgr);
    ASSERT_EQ(manager->Init({}), ERR_OK);
    saMgr->userLifecycleManager_.validUserIds_.insert(userId);
    saMgr->userLifecycleManager_.multiUserManagers_[userId] = manager;
    std::vector<std::u16string> processList;
    EXPECT_EQ(saMgr->UnloadProcess(processList), ERR_OK);
    manager->Destroy();
    EXPECT_TRUE(processList.empty());
}

/**
 * @tc.name: MultiUserBaseLifecycleGuard005
 * @tc.desc: Verify base and invalid user IDs are rejected before lifecycle dispatch.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, MultiUserBaseLifecycleGuard005, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    ASSERT_NE(saMgr, nullptr);

    EXPECT_EQ(saMgr->OnUserStateChanged(BASE_USER, USER_STATE_ACTIVATING), ERR_INVALID_VALUE);
    EXPECT_EQ(saMgr->OnUserStateChanged(SAMGR_INVALID_USER_ID, USER_STATE_ACTIVATING), ERR_INVALID_VALUE);
}

/**
 * @tc.name: MultiUserMissingManagerAggregation006
 * @tc.desc: Verify aggregate operations skip valid users whose manager is absent.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, MultiUserMissingManagerAggregation006, TestSize.Level3)
{
    constexpr int32_t userId = 136;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    ASSERT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
    saMgr->userLifecycleManager_.validUserIds_.insert(userId);
    std::list<SystemProcessInfo> processInfos;
    std::vector<sptr<IRemoteObject>> abilityList;
    std::vector<ISystemAbilityManager::SaExtensionInfo> extensionInfos;

    EXPECT_EQ(saMgr->GetRunningSystemProcess(processInfos), ERR_OK);
    EXPECT_EQ(saMgr->GetExtensionRunningSaList("missing", abilityList), ERR_OK);
    EXPECT_EQ(saMgr->GetRunningSaExtensionInfoList("missing", extensionInfos), ERR_OK);
    bool dispatched = false;
    std::vector<int32_t> systemAbilityIds = {SAID};
    std::string action;
    EXPECT_EQ(saMgr->SendStrategyToUsers(0, systemAbilityIds, 0, action, dispatched), ERR_OK);
    EXPECT_FALSE(dispatched);
}

/**
 * @tc.name: MultiUserStrategyErrorAggregation007
 * @tc.desc: Verify a user strategy error is retained after dispatch to an active manager.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, MultiUserStrategyErrorAggregation007, TestSize.Level3)
{
    constexpr int32_t userId = 137;
    auto manager = std::make_shared<MultiSystemAbilityManager>(userId);
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    ASSERT_NE(manager, nullptr);
    ASSERT_NE(saMgr, nullptr);
    ASSERT_EQ(manager->Init({}), ERR_OK);
    saMgr->userLifecycleManager_.validUserIds_.insert(userId);
    saMgr->userLifecycleManager_.multiUserManagers_[userId] = manager;
    std::vector<int32_t> systemAbilityIds = {SAID};
    std::string action;
    bool dispatched = false;

    EXPECT_EQ(saMgr->SendStrategyToUsers(0, systemAbilityIds, 0, action, dispatched), ERR_PERMISSION_DENIED);
    EXPECT_TRUE(dispatched);
    manager->Destroy();
}

} // namespace OHOS
#endif


namespace OHOS {
/**
 * @tc.name: SystemAbilityManagerRemoteAndDump001
 * @tc.desc: Test remote SA visibility, on-demand enumeration, and listener dump dispatch.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, SystemAbilityManagerRemoteAndDump001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    constexpr int32_t remoteSaId = 2234;
    sptr<IRemoteObject> remoteObject = new TestTransactionService();

    EXPECT_EQ(saMgr->GetSystemAbilityFromRemote(-1), nullptr);
    saMgr->abilityMap_[remoteSaId] = {remoteObject, false};
    EXPECT_EQ(saMgr->GetSystemAbilityFromRemote(remoteSaId), nullptr);
    saMgr->abilityMap_[remoteSaId].isDistributed = true;
    EXPECT_EQ(saMgr->GetSystemAbilityFromRemote(remoteSaId), remoteObject);

    CommonSaProfile profile;
    profile.saId = remoteSaId + 1;
    saMgr->saProfileMap_[profile.saId] = profile;
    std::list<int32_t> ondemandSaIds = saMgr->GetAllOndemandSa();
    EXPECT_EQ(ondemandSaIds.size(), 1U);
    EXPECT_EQ(ondemandSaIds.front(), profile.saId);

    SamMockPermission::MockProcess("hidumper_service");
    sptr<ISystemAbilityStatusChange> listener = new SaStatusChangeMock();
    saMgr->listenerMap_[remoteSaId].emplace_back(listener, IPCSkeleton::GetCallingPid(), ListenerState::INIT);
    std::vector<std::u16string> dumpArgs {u"--listener", u"-l", u"-sa"};
    EXPECT_EQ(saMgr->Dump(STDOUT_FILENO, dumpArgs), ERR_OK);
    EXPECT_FALSE(saMgr->IpcStatSamgrProc(-1, IPC_STAT_CMD_START));
    std::vector<std::u16string> emptyDumpArgs;
    EXPECT_EQ(saMgr->Dump(-1, emptyDumpArgs), ERR_INVALID_VALUE);
}

/**
 * @tc.name: SystemAbilityManagerIpcDumpProcess001
 * @tc.desc: Test IPC dump dispatch for populated local system processes.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, SystemAbilityManagerIpcDumpProcess001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    sptr<DumpLocalAbilityManager> localManager = new DumpLocalAbilityManager();
    ASSERT_NE(localManager, nullptr);
    const std::u16string processName = u"samgr_dump_process";
    saMgr->systemProcessMap_[processName] = localManager;

    saMgr->IpcDumpAllProcess(STDOUT_FILENO, IPC_STAT_CMD_START);
    EXPECT_EQ(localManager->ipcStatCallCount_, 1);
    saMgr->IpcDumpSingleProcess(STDOUT_FILENO, IPC_STAT_CMD_STOP, "samgr_dump_process");
    EXPECT_EQ(localManager->ipcStatCallCount_, 2);
}

/**
 * @tc.name: SystemAbilityManagerGuardPath001
 * @tc.desc: Test manager guard paths before optional helpers are initialized.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, SystemAbilityManagerGuardPath001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    ASSERT_NE(saMgr, nullptr);

    saMgr->OndemandLoadForPerf();
    saMgr->NotifyRpcLoadCompleted("device", 2236, nullptr);
    saMgr->AddSamgrToAbilityMap();
    ASSERT_EQ(saMgr->abilityMap_.count(0), 1U);
    EXPECT_EQ(saMgr->GetSystemAbilityFromRemote(0), nullptr);
}
} // namespace OHOS
