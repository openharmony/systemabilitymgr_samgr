/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
#define private public
#include "ipc_skeleton.h"
#include "accesstoken_kit.h"
#include "system_ability_manager.h"
#ifdef SUPPORT_COMMON_EVENT
#include "common_event_collect.h"
#endif

using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS;

namespace OHOS {
namespace {
constexpr int32_t SAID = 1234;
constexpr int32_t OTHER_ON_DEMAND = 3;
constexpr int32_t TEST_VALUE = 2021;
constexpr int32_t TEST_REVERSE_VALUE = 1202;
constexpr int32_t REPEAT = 10;
constexpr int32_t OVERFLOW_TIME = 257;
constexpr int32_t TEST_OVERFLOW_SAID = 99999;
constexpr int32_t TEST_EXCEPTION_HIGH_SA_ID = LAST_SYS_ABILITY_ID + 1;
constexpr int32_t TEST_EXCEPTION_LOW_SA_ID = -1;
constexpr int32_t TEST_SYSTEM_ABILITY1 = 1491;
constexpr int32_t TEST_SYSTEM_ABILITY2 = 1492;
constexpr int32_t SHFIT_BIT = 32;
constexpr int32_t ONDEMAND_SLEEP_TIME = 600 * 1000; // us
constexpr int32_t MAX_COUNT = INT32_MAX - 1000000;
constexpr int64_t ONDEMAND_EXTRA_DATA_ID = 1;

const std::u16string PROCESS_NAME = u"test_process_name";
const std::u16string DEVICE_NAME = u"test_name";
const std::u16string SAMANAGER_INTERFACE_TOKEN = u"ohos.samgr.accessToken";
const string ONDEMAND_PARAM = "persist.samgr.perf.ondemand";
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
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    int32_t result = sm->AddSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID, nullptr);
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
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    int32_t result = sm->AddSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID, testAbility);
    DTEST_LOG << "add TestTransactionService result = " << result << std::endl;
    EXPECT_EQ(result, ERR_OK);
    sm->RemoveSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID);
}

/**
 * @tc.name: AddSystemAbility003
 * @tc.desc: add system ability saId exception.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, AddSystemAbility003, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    int32_t result = sm->AddSystemAbility(TEST_EXCEPTION_HIGH_SA_ID, testAbility);
    EXPECT_TRUE(result != ERR_OK);
    result = sm->AddSystemAbility(TEST_EXCEPTION_LOW_SA_ID, testAbility);
    EXPECT_TRUE(result != ERR_OK);
    sm->RemoveSystemAbility(TEST_EXCEPTION_HIGH_SA_ID);
    sm->RemoveSystemAbility(TEST_EXCEPTION_LOW_SA_ID);
}

/**
 * @tc.name: AddSystemAbility004
 * @tc.desc: add system ability with empty capability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, AddSystemAbility004, TestSize.Level1)
{
    int32_t systemAbilityId = DISTRIBUTED_SCHED_TEST_TT_ID;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    ISystemAbilityManager::SAExtraProp saExtraProp(false, ISystemAbilityManager::DUMP_FLAG_PRIORITY_DEFAULT,
        u"", u"");
    int32_t ret = sm->AddSystemAbility(systemAbilityId, new TestTransactionService(), saExtraProp);
    EXPECT_EQ(ret, ERR_OK);
    sm->RemoveSystemAbility(systemAbilityId);
}

/**
 * @tc.name: AddSystemAbility005
 * @tc.desc: add system ability with validated capability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, AddSystemAbility005, TestSize.Level1)
{
    int32_t systemAbilityId = DISTRIBUTED_SCHED_TEST_SO_ID;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    std::u16string capability = u"{\"Capabilities\":{\"aaa\":\"[10.4, 20.5]\",\"bbb\":\"[11, 55]\",\
        \"ccc\":\"this is string\", \"ddd\":\"[aa, bb, cc, dd]\", \"eee\":5.60, \"fff\":4545, \"ggg\":true}}";
    ISystemAbilityManager::SAExtraProp saExtraProp(true, ISystemAbilityManager::DUMP_FLAG_PRIORITY_DEFAULT,
        capability, u"");
    int32_t ret = sm->AddSystemAbility(systemAbilityId, new TestTransactionService(), saExtraProp);
    EXPECT_EQ(ret, ERR_OK);
    sm->RemoveSystemAbility(systemAbilityId);
}

/**
 * @tc.name: RemoveSystemAbility001
 * @tc.desc: remove not exist system ability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, RemoveSystemAbility001, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    int32_t result = sm->RemoveSystemAbility(-1);
    EXPECT_TRUE(result != ERR_OK);
}

/**
 * @tc.name: RemoveSystemAbility002
 * @tc.desc: remove system ability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, RemoveSystemAbility002, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    sm->AddSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID, testAbility);
    int32_t result = sm->RemoveSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID);
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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    ISystemAbilityManager::SAExtraProp extraProp;
    saMgr->AddSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID, testAbility, extraProp);
    saMgr->abilityStateScheduler_ = nullptr;
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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    int32_t result = saMgr->RemoveSystemAbility(-1);
    EXPECT_TRUE(result != ERR_OK);
}

/**
 * @tc.name: GetSystemAbility001
 * @tc.desc: get not exist system ability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, GetSystemAbility001, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    auto ability = sm->GetSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID);
    EXPECT_EQ(ability, nullptr);
}

/**
 * @tc.name: GetSystemAbility002
 * @tc.desc: get system ability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, GetSystemAbility002, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    sm->AddSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID, testAbility);
    auto ability = sm->GetSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID);
    EXPECT_TRUE(ability != nullptr);
    sm->RemoveSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID);
}

/**
 * @tc.name: GetSystemAbility003
 * @tc.desc: get system ability and then transaction.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, GetSystemAbility003, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    sm->AddSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID, testAbility);
    auto ability = sm->GetSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID);
    EXPECT_TRUE(ability != nullptr);
    sptr<ITestTransactionService> targetAblility = iface_cast<ITestTransactionService>(ability);
    EXPECT_TRUE(targetAblility != nullptr);
    int32_t rep = 0;
    int32_t result = targetAblility->ReverseInt(TEST_VALUE, rep);
    DTEST_LOG << "testAbility ReverseInt result = " << result << ", get reply = " << rep << std::endl;
    EXPECT_EQ(rep, TEST_REVERSE_VALUE);
    sm->RemoveSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID);
}

/**
 * @tc.name: GetSystemAbility004
 * @tc.desc: get system ability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, GetSystemAbility004, TestSize.Level2)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    for (int32_t i = 0; i < REPEAT; ++i) {
        auto result = sm->AddSystemAbility((DISTRIBUTED_SCHED_TEST_SO_ID + i), new TestTransactionService());
        EXPECT_EQ(result, ERR_OK);
    }
    for (int32_t i = 0; i < REPEAT; ++i) {
        int32_t saId = DISTRIBUTED_SCHED_TEST_SO_ID + i;
        auto saObject = sm->GetSystemAbility(saId);
        EXPECT_TRUE(saObject != nullptr);
        sm->RemoveSystemAbility(saId);
    }
}

/**
 * @tc.name: GetSystemAbility005
 * @tc.desc: get remote device system ability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, GetSystemAbility005, TestSize.Level2)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    string fakeDeviceId = "fake_dev";
    auto abilityObj = sm->GetSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID, fakeDeviceId);
    EXPECT_EQ(abilityObj, nullptr);
}

/**
 * @tc.name: CheckSystemAbility001
 * @tc.desc: check system ability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, CheckSystemAbility001, TestSize.Level1)
{
    int32_t systemAbilityId = DISTRIBUTED_SCHED_TEST_TT_ID;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    sm->AddSystemAbility(systemAbilityId, testAbility);
    sptr<IRemoteObject> abilityObj = sm->CheckSystemAbility(systemAbilityId);
    EXPECT_TRUE(abilityObj != nullptr);
    sm->RemoveSystemAbility(systemAbilityId);
}

/**
 * @tc.name: CheckSystemAbility002
 * @tc.desc: check system ability. abilityStateScheduler_ is nullptr
 * @tc.type: FUNC
 * @tc.require: I6MO6A
 */
HWTEST_F(SystemAbilityMgrTest, CheckSystemAbility002, TestSize.Level1)
{
    int32_t systemAbilityId = DISTRIBUTED_SCHED_TEST_TT_ID;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    saMgr->abilityStateScheduler_ = systemAbilityStateScheduler;
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemAbilityStateScheduler->abilityContextMap_.clear();
    systemAbilityContext->ownProcessContext = systemProcessContext;
    systemAbilityStateScheduler->abilityContextMap_[SAID] = systemAbilityContext;
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
    int32_t systemAbilityId = DISTRIBUTED_SCHED_TEST_TT_ID;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    bool isExist = true;
    saMgr->abilityStateScheduler_ = nullptr;
    sptr<IRemoteObject> abilityObj = saMgr->CheckSystemAbility(systemAbilityId, isExist);
    saMgr->abilityStateScheduler_ = make_shared<SystemAbilityStateScheduler>();
    EXPECT_EQ(abilityObj, nullptr);
}

/**
 * @tc.name: CheckOnDemandSystemAbility001
 * @tc.desc: check on demand system ability.
 * @tc.type: FUNC
 * @tc.require: I6MO6A
 */
HWTEST_F(SystemAbilityMgrTest, CheckOnDemandSystemAbility001, TestSize.Level1)
{
    int32_t systemAbilityId = DISTRIBUTED_SCHED_TEST_TT_ID;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    sm->AddSystemAbility(systemAbilityId, new TestTransactionService());
    int32_t ret = sm->AddOnDemandSystemAbilityInfo(systemAbilityId, u"test_localmanagername");
    EXPECT_TRUE(ret != ERR_OK);
    sm->RemoveSystemAbility(systemAbilityId);
}

/**
 * @tc.name: CheckOnDemandSystemAbility002
 * @tc.desc: check on demand system ability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, CheckOnDemandSystemAbility002, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    ISystemAbilityManager::SAExtraProp saExtraProp(false, 0, u"", u"");
    int32_t systemAbilityId = DISTRIBUTED_SCHED_TEST_TT_ID;
    int32_t result = sm->AddSystemAbility(systemAbilityId, new TestTransactionService(), saExtraProp);
    EXPECT_EQ(result, ERR_OK);
    sptr<IRemoteObject> saObject = sm->CheckSystemAbility(systemAbilityId);
    result = sm->AddOnDemandSystemAbilityInfo(systemAbilityId, u"just_test");
    EXPECT_TRUE(result != ERR_OK);
    sm->RemoveSystemAbility(systemAbilityId);
}

/**
 * @tc.name: ListSystemAbility001
 * @tc.desc: list all system abilities.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, ListSystemAbility001, TestSize.Level1)
{
    int32_t systemAbilityId = DISTRIBUTED_SCHED_TEST_TT_ID;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    sm->AddSystemAbility(systemAbilityId, new TestTransactionService());
    auto saList = sm->ListSystemAbilities();
    EXPECT_TRUE(!saList.empty());
    auto iter = std::find(saList.begin(), saList.end(), to_utf16(std::to_string(systemAbilityId)));
    EXPECT_TRUE(iter != saList.end());
    sm->RemoveSystemAbility(systemAbilityId);
}

/**
 * @tc.name: LoadSystemAbility001
 * @tc.desc: load system ability with invalid systemAbilityId.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, LoadSystemAbility001, TestSize.Level0)
{
    int32_t systemAbilityId = TEST_EXCEPTION_LOW_SA_ID;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    int32_t result = sm->LoadSystemAbility(systemAbilityId, nullptr);
    EXPECT_TRUE(result != ERR_OK);
}

/**
 * @tc.name: LoadSystemAbility002
 * @tc.desc: load system ability with invalid systemAbilityId.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, LoadSystemAbility002, TestSize.Level0)
{
    int32_t systemAbilityId = TEST_EXCEPTION_HIGH_SA_ID;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    int32_t result = sm->LoadSystemAbility(systemAbilityId, nullptr);
    EXPECT_TRUE(result != ERR_OK);
}

/**
 * @tc.name: LoadSystemAbility003
 * @tc.desc: load system ability with invalid callback.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, LoadSystemAbility003, TestSize.Level0)
{
    int32_t systemAbilityId = DISTRIBUTED_SCHED_TEST_SO_ID;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    int32_t result = sm->LoadSystemAbility(systemAbilityId, nullptr);
    EXPECT_TRUE(result != ERR_OK);
}

/**
 * @tc.name: LoadSystemAbility004
 * @tc.desc: load system ability with not exist systemAbilityId.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, LoadSystemAbility004, TestSize.Level0)
{
    int32_t systemAbilityId = DISTRIBUTED_SCHED_TEST_SO_ID;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    sptr<ISystemAbilityLoadCallback> callback = new SystemAbilityLoadCallbackMock();
    int32_t result = sm->LoadSystemAbility(systemAbilityId, callback);
    EXPECT_TRUE(result != ERR_OK);
}

/**
 * @tc.name: LoadSystemAbility005
 * @tc.desc: test OnRemoteRequest, invalid interface token.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, LoadSystemAbility005, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    int32_t result = saMgr->OnRemoteRequest(static_cast<uint32_t>(SamgrInterfaceCode::LOAD_SYSTEM_ABILITY_TRANSACTION),
        data, reply, option);
    EXPECT_TRUE(result != ERR_NONE);
}

/**
 * @tc.name: LoadSystemAbility006
 * @tc.desc: test OnRemoteRequest, invalid systemAbilityId.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, LoadSystemAbility006, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN);
    MessageParcel reply;
    MessageOption option;
    int32_t result = saMgr->OnRemoteRequest(static_cast<uint32_t>(SamgrInterfaceCode::LOAD_SYSTEM_ABILITY_TRANSACTION),
        data, reply, option);
    EXPECT_TRUE(result != ERR_NONE);
}

/**
 * @tc.name: LoadSystemAbility007
 * @tc.desc: test OnRemoteRequest, invalid systemAbilityId.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, LoadSystemAbility007, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN);
    data.WriteInt32(TEST_EXCEPTION_HIGH_SA_ID);
    MessageParcel reply;
    MessageOption option;
    int32_t result = saMgr->OnRemoteRequest(static_cast<uint32_t>(SamgrInterfaceCode::LOAD_SYSTEM_ABILITY_TRANSACTION),
        data, reply, option);
    EXPECT_TRUE(result != ERR_NONE);
}

/**
 * @tc.name: LoadSystemAbility008
 * @tc.desc: test OnRemoteRequest, null callback.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, LoadSystemAbility008, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN);
    data.WriteInt32(DISTRIBUTED_SCHED_TEST_SO_ID);
    MessageParcel reply;
    MessageOption option;
    int32_t result = saMgr->OnRemoteRequest(static_cast<uint32_t>(SamgrInterfaceCode::LOAD_SYSTEM_ABILITY_TRANSACTION),
        data, reply, option);
    EXPECT_TRUE(result != ERR_NONE);
}

/**
 * @tc.name: LoadSystemAbility009
 * @tc.desc: test OnRemoteRequest, not exist systemAbilityId.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, LoadSystemAbility009, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN);
    data.WriteInt32(DISTRIBUTED_SCHED_TEST_SO_ID);
    sptr<ISystemAbilityLoadCallback> callback = new SystemAbilityLoadCallbackMock();
    data.WriteRemoteObject(callback->AsObject());
    MessageParcel reply;
    MessageOption option;
    int32_t result = saMgr->OnRemoteRequest(static_cast<uint32_t>(SamgrInterfaceCode::LOAD_SYSTEM_ABILITY_TRANSACTION),
        data, reply, option);
    EXPECT_TRUE(result != ERR_NONE);
}

/**
 * @tc.name: LoadSystemAbility010
 * @tc.desc: test LoadSystemAbility with saProfileMap_ is empty
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrTest, LoadSystemAbility010, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->saProfileMap_.clear();
    sptr<ISystemAbilityLoadCallback> callback = new SystemAbilityLoadCallbackMock();
    int32_t ret = saMgr->LoadSystemAbility(SAID, callback);
    EXPECT_EQ(ret, PROFILE_NOT_EXIST);
}

/**
 * @tc.name: LoadSystemAbility011
 * @tc.desc: test LoadSystemAbility with invalid said
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrTest, LoadSystemAbility011, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<ISystemAbilityLoadCallback> callback = new SystemAbilityLoadCallbackMock();
    int32_t ret = saMgr->LoadSystemAbility(-1, callback);
    EXPECT_EQ(ret, INVALID_INPUT_PARA);
}

/**
 * @tc.name: OnRemoteDied001
 * @tc.desc: test OnRemoteDied, remove registered callback.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, OnRemoteDied001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    sptr<ISystemAbilityLoadCallback> callback = new SystemAbilityLoadCallbackMock();
    saMgr->OnAbilityCallbackDied(callback->AsObject());
    EXPECT_TRUE(saMgr->startingAbilityMap_.empty());
}

/**
 * @tc.name: StartOnDemandAbility001
 * @tc.desc: test StartOnDemandAbility, invalid systemAbilityId.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, StartOnDemandAbility001, TestSize.Level0)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    bool isExist = false;
    int32_t result = saMgr->StartOnDemandAbility(TEST_EXCEPTION_LOW_SA_ID, isExist);
    EXPECT_TRUE(result != ERR_NONE);
}

/**
 * @tc.name: StartOnDemandAbility002
 * @tc.desc: test StartOnDemandAbility, invalid systemAbilityId.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, StartOnDemandAbility002, TestSize.Level0)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    bool isExist = false;
    int32_t result = saMgr->StartOnDemandAbility(TEST_EXCEPTION_HIGH_SA_ID, isExist);
    EXPECT_TRUE(result != ERR_NONE);
}

/**
 * @tc.name: StartOnDemandAbility003
 * @tc.desc: test StartOnDemandAbility, not exist systemAbilityId.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, StartOnDemandAbility003, TestSize.Level0)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    bool isExist = false;
    int32_t result = saMgr->StartOnDemandAbility(DISTRIBUTED_SCHED_TEST_SO_ID, isExist);
    EXPECT_TRUE(result != ERR_NONE);
}

/**
 * @tc.name: StartOnDemandAbility004
 * @tc.desc: test StartOnDemandAbility, not on-demand systemAbilityId.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, StartOnDemandAbility004, TestSize.Level0)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    bool isExist = false;
    int32_t result = saMgr->StartOnDemandAbility(DISTRIBUTED_SCHED_SA_ID, isExist);
    EXPECT_TRUE(result != ERR_NONE);
}

/**
 * @tc.name: StartOnDemandAbility005
 * @tc.desc: test StartOnDemandAbility, invalid systemAbilityId.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, StartOnDemandAbility005, TestSize.Level0)
{
    DTEST_LOG << " StartOnDemandAbility005 " << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    bool isExist = false;
    int32_t result = saMgr->StartOnDemandAbility(TEST_EXCEPTION_LOW_SA_ID, isExist);
    EXPECT_TRUE(result != ERR_NONE);
}

/**
 * @tc.name: AddOnDemandSystemAbilityInfo001
 * @tc.desc: test AddOnDemandSystemAbilityInfo, invalid systemAbilityId.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, AddOnDemandSystemAbilityInfo001, TestSize.Level0)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    int32_t result = saMgr->AddOnDemandSystemAbilityInfo(TEST_EXCEPTION_LOW_SA_ID, u"");
    EXPECT_TRUE(result != ERR_NONE);
}

/**
 * @tc.name: AddOnDemandSystemAbilityInfo002
 * @tc.desc: test AddOnDemandSystemAbilityInfo, invalid systemAbilityId.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, AddOnDemandSystemAbilityInfo002, TestSize.Level0)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    int32_t result = saMgr->AddOnDemandSystemAbilityInfo(TEST_EXCEPTION_HIGH_SA_ID, u"");
    EXPECT_TRUE(result != ERR_NONE);
}

/**
 * @tc.name: AddOnDemandSystemAbilityInfo003
 * @tc.desc: test AddOnDemandSystemAbilityInfo, invalid procName.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, AddOnDemandSystemAbilityInfo003, TestSize.Level0)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    int32_t result = saMgr->AddOnDemandSystemAbilityInfo(DISTRIBUTED_SCHED_TEST_SO_ID, u"");
    EXPECT_TRUE(result != ERR_NONE);
}

/**
 * @tc.name: AddOnDemandSystemAbilityInfo004
 * @tc.desc: test AddOnDemandSystemAbilityInfo, procName not registered.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, AddOnDemandSystemAbilityInfo004, TestSize.Level0)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    int32_t result = saMgr->AddOnDemandSystemAbilityInfo(DISTRIBUTED_SCHED_TEST_SO_ID, u"fake_process_name");
    EXPECT_TRUE(result != ERR_NONE);
}

/**
 * @tc.name: AddOnDemandSystemAbilityInfo005
 * @tc.desc: test AddOnDemandSystemAbilityInfo, invalid systemAbilityId.
 * @tc.type: FUNC
 * @tc.require: I6MO6A
 */
HWTEST_F(SystemAbilityMgrTest, AddOnDemandSystemAbilityInfo005, TestSize.Level0)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    int32_t said = -1;
    int32_t result = saMgr->AddOnDemandSystemAbilityInfo(said, u"");
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: OnLoadSystemAbilitySuccess001
 * @tc.desc: test OnLoadSystemAbilitySuccess, null callback.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, OnLoadSystemAbilitySuccess001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    saMgr->NotifySystemAbilityLoaded(DISTRIBUTED_SCHED_TEST_SO_ID, nullptr, nullptr);
    EXPECT_TRUE(callback->GetSystemAbilityId() == 0);
}

/**
 * @tc.name: OnLoadSystemAbilitySuccess002
 * @tc.desc: test OnLoadSystemAbilitySuccess, null IRemoteObject.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, OnLoadSystemAbilitySuccess002, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    saMgr->NotifySystemAbilityLoaded(DISTRIBUTED_SCHED_TEST_SO_ID, nullptr, callback);
    EXPECT_TRUE(callback->GetSystemAbilityId() == DISTRIBUTED_SCHED_TEST_SO_ID);
    EXPECT_TRUE(callback->GetRemoteObject() == nullptr);
}

/**
 * @tc.name: OnLoadSystemAbilitySuccess003
 * @tc.desc: test OnLoadSystemAbilitySuccess.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, OnLoadSystemAbilitySuccess003, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    sptr<IRemoteObject> remoteObject = new TestTransactionService();
    saMgr->NotifySystemAbilityLoaded(DISTRIBUTED_SCHED_TEST_SO_ID, remoteObject, callback);
    EXPECT_TRUE(callback->GetSystemAbilityId() == DISTRIBUTED_SCHED_TEST_SO_ID);
    EXPECT_TRUE(callback->GetRemoteObject() == remoteObject);
}

/**
 * @tc.name: OnLoadSystemAbilitySuccess004
 * @tc.desc: test OnLoadSystemAbilitySuccess, null callback.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, OnLoadSystemAbilitySuccess004, TestSize.Level1)
{
    DTEST_LOG << " OnLoadSystemAbilitySuccess004 " << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    saMgr->NotifySystemAbilityLoaded(DISTRIBUTED_SCHED_TEST_SO_ID, nullptr, nullptr);
    EXPECT_TRUE(callback->GetSystemAbilityId() == 0);
}

/**
 * @tc.name: ReportSubscribeOverflow001
 * @tc.desc: ReportSubscribeOverflow001
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, ReportSubscribeOverflow001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    ASSERT_TRUE(saMgr != nullptr);
    std::vector<sptr<SaStatusChangeMock>> tmpCallbak;
    sptr<SaStatusChangeMock> callback = nullptr;
    for (int i = 0; i < OVERFLOW_TIME; ++i) {
        callback = new SaStatusChangeMock();
        tmpCallbak.emplace_back(callback);
        saMgr->SubscribeSystemAbility(TEST_OVERFLOW_SAID, callback);
    }
    for (const auto& callback : tmpCallbak) {
        saMgr->UnSubscribeSystemAbility(TEST_OVERFLOW_SAID, callback);
    }
}

/**
 * @tc.name: UnSubscribeSystemAbilityDied001
 * @tc.desc: test UnSubscribeSystemAbility with OnRemoteDied
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, UnSubscribeSystemAbilityDied001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    ASSERT_TRUE(saMgr != nullptr);
    sptr<SaStatusChangeMock> callback(new SaStatusChangeMock());
    saMgr->listenerMap_[SAID].push_back({callback, SAID});
    auto& count = saMgr->subscribeCountMap_[SAID];
    ++count;
    saMgr->UnSubscribeSystemAbility(callback->AsObject());
}

/**
 * @tc.name: ReportLoadSAOverflow001
 * @tc.desc: ReportLoadSAOverflow001
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, ReportLoadSAOverflow001, TestSize.Level1)
{
    sptr<ISystemAbilityManager> saMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_TRUE(saMgr != nullptr);
    for (int i = 0; i < OVERFLOW_TIME; ++i) {
        sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
        saMgr->LoadSystemAbility(TEST_OVERFLOW_SAID, callback);
    }
}

/**
 * @tc.name: LoadRemoteSystemAbility001
 * @tc.desc: load system ability with invalid systemAbilityId.
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, LoadRemoteSystemAbility001, TestSize.Level2)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    int32_t systemAbilityId = TEST_EXCEPTION_LOW_SA_ID;
    std::string deviceId = "";
    int32_t result = sm->LoadSystemAbility(systemAbilityId, deviceId, nullptr);
    EXPECT_TRUE(result != ERR_OK);
}

/**
 * @tc.name: LoadRemoteSystemAbility002
 * @tc.desc: load system ability with invalid systemAbilityId.
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, LoadRemoteSystemAbility002, TestSize.Level2)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    int32_t systemAbilityId = TEST_EXCEPTION_LOW_SA_ID;
    std::string deviceId = "123456789";
    int32_t result = sm->LoadSystemAbility(systemAbilityId, deviceId, nullptr);
    EXPECT_TRUE(result != ERR_OK);
}

/**
 * @tc.name: LoadRemoteSystemAbility002
 * @tc.desc: load system ability with invalid systemAbilityId.
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, LoadRemoteSystemAbility003, TestSize.Level2)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    int32_t systemAbilityId = -1;
    std::string deviceId = "123456789";
    int32_t result = sm->LoadSystemAbility(systemAbilityId, deviceId, nullptr);
    EXPECT_TRUE(result != ERR_OK);
}

/**
 * @tc.name: LoadRemoteSystemAbility004
 * @tc.desc: load system ability with invalid systemAbilityId.
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, LoadRemoteSystemAbility004, TestSize.Level2)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    int32_t systemAbilityId = 0;
    std::string deviceId = "123456789";
    int32_t result = sm->LoadSystemAbility(systemAbilityId, deviceId, nullptr);
    EXPECT_TRUE(result != ERR_OK);
}

/**
 * @tc.name: LoadRemoteSystemAbility004
 * @tc.desc: load system ability with invalid systemAbilityId.
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, LoadRemoteSystemAbility005, TestSize.Level2)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    int32_t systemAbilityId = 0;
    std::string deviceId = "";
    int32_t result = sm->LoadSystemAbility(systemAbilityId, deviceId, nullptr);
    EXPECT_TRUE(result != ERR_OK);
}

/**
 * @tc.name: LoadRemoteSystemAbility004
 * @tc.desc: load system ability with invalid systemAbilityId.
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, LoadRemoteSystemAbility006, TestSize.Level2)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    int32_t systemAbilityId = -1;
    std::string deviceId = "";
    int32_t result = sm->LoadSystemAbility(systemAbilityId, deviceId, nullptr);
    EXPECT_TRUE(result != ERR_OK);
}

/**
 * @tc.name: LoadRemoteSystemAbility007
 * @tc.desc: load system ability with invalid systemAbilityId.
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, LoadRemoteSystemAbility007, TestSize.Level2)
{
    DTEST_LOG << " LoadRemoteSystemAbility007 " << std::endl;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    int32_t systemAbilityId = -1;
    std::string deviceId = "1234567890";
    int32_t result = sm->LoadSystemAbility(systemAbilityId, deviceId, nullptr);
    EXPECT_TRUE(result != ERR_OK);
}

/**
 * @tc.name: LoadSystemAbilityFromRpc001
 * @tc.desc: load system ability with invalid systemAbilityId.
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, LoadSystemAbilityFromRpc001, TestSize.Level2)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    std::string deviceId = "";
    int32_t systemAbilityId = -1;
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    bool ret = saMgr->LoadSystemAbilityFromRpc(deviceId, systemAbilityId, callback);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: LoadSystemAbilityFromRpc002
 * @tc.desc: load system ability with invalid systemAbilityId.
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, LoadSystemAbilityFromRpc002, TestSize.Level2)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    std::string deviceId = "";
    int32_t systemAbilityId = 0;
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    bool ret = saMgr->LoadSystemAbilityFromRpc(deviceId, systemAbilityId, callback);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: LoadSystemAbilityFromRpc003
 * @tc.desc: load system ability with invalid systemAbilityId.
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, LoadSystemAbilityFromRpc003, TestSize.Level2)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    std::string deviceId = "";
    int32_t systemAbilityId = 0;
    bool ret = saMgr->LoadSystemAbilityFromRpc(deviceId, systemAbilityId, nullptr);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: LoadSystemAbilityFromRpc004
 * @tc.desc: load system ability with invalid systemAbilityId.
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, LoadSystemAbilityFromRpc004, TestSize.Level2)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    std::string deviceId = "1111111";
    int32_t systemAbilityId = 0;
    bool ret = saMgr->LoadSystemAbilityFromRpc(deviceId, systemAbilityId, nullptr);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: LoadSystemAbilityFromRpc005
 * @tc.desc: load system ability with callback is nullptr.
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, LoadSystemAbilityFromRpc005, TestSize.Level2)
{
    DTEST_LOG << " LoadSystemAbilityFromRpc005 " << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    std::string deviceId = "2222222";
    int32_t systemAbilityId = 1;
    bool ret = saMgr->LoadSystemAbilityFromRpc(deviceId, systemAbilityId, nullptr);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: LoadSystemAbilityFromRpc006
 * @tc.desc: load system ability with sa profile distributed false.
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, LoadSystemAbilityFromRpc006, TestSize.Level2)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    std::string deviceId = "2222222";
    int32_t systemAbilityId = 1;
    SaProfile saProfile;
    saMgr->saProfileMap_[1] = saProfile;
    bool ret = saMgr->LoadSystemAbilityFromRpc(deviceId, systemAbilityId, nullptr);
    EXPECT_FALSE(ret);
    saMgr->saProfileMap_.clear();
}

/**
 * @tc.name: LoadSystemAbilityFromRpc007
 * @tc.desc: load system ability with abilityStateScheduler_ nullptr
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, LoadSystemAbilityFromRpc007, TestSize.Level2)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    std::string deviceId = "2222222";
    int32_t systemAbilityId = 1;
    SaProfile saProfile;
    saProfile.distributed = true;
    saMgr->saProfileMap_[1] = saProfile;
    saMgr->abilityStateScheduler_ = nullptr;
    bool ret = saMgr->LoadSystemAbilityFromRpc(deviceId, systemAbilityId, nullptr);
    EXPECT_FALSE(ret);
    saMgr->saProfileMap_.clear();
}

/**
 * @tc.name: LoadSystemAbilityFromRpc008
 * @tc.desc: load system ability with distributed true
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, LoadSystemAbilityFromRpc008, TestSize.Level2)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    std::string deviceId = "2222222";
    int32_t systemAbilityId = 1;
    SaProfile saProfile;
    saProfile.distributed = true;
    saMgr->saProfileMap_[1] = saProfile;
    bool ret = saMgr->LoadSystemAbilityFromRpc(deviceId, systemAbilityId, nullptr);
    EXPECT_FALSE(ret);
    saMgr->saProfileMap_.clear();
}

/**
 * @tc.name: UnloadSystemAbility001
 * @tc.desc: UnloadSystemAbility sa not exist
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, UnloadSystemAbility001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    int32_t result = saMgr->UnloadSystemAbility(1);
    EXPECT_EQ(result, PROFILE_NOT_EXIST);
}

/**
 * @tc.name: UnloadSystemAbility002
 * @tc.desc: UnloadSystemAbility, caller invalid
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, UnloadSystemAbility002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    SaProfile saProfile;
    saMgr->saProfileMap_[1] = saProfile;
    saMgr->abilityStateScheduler_ = nullptr;
    int32_t result = saMgr->UnloadSystemAbility(1);
    EXPECT_EQ(result, INVALID_CALL_PROC);
    saMgr->saProfileMap_.clear();
}

/**
 * @tc.name: DoMakeRemoteBinder001
 * @tc.desc: load system ability with invalid systemAbilityId.
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, DoMakeRemoteBinder001, TestSize.Level2)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->startingAbilityMap_.clear();

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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->startingAbilityMap_.clear();

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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->startingAbilityMap_.clear();

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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->startingAbilityMap_.clear();

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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->startingAbilityMap_.clear();

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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->startingAbilityMap_.clear();

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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->startingAbilityMap_.clear();

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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->startingAbilityMap_.clear();

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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->startingAbilityMap_.clear();

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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->startingAbilityMap_.clear();

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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->startingAbilityMap_.clear();

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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->startingAbilityMap_.clear();

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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();

    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();
    saMgr->remoteCallbacks_.clear();
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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->remoteCallbacks_.clear();
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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->remoteCallbacks_.clear();
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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->remoteCallbacks_.clear();
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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->remoteCallbacks_.clear();
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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->remoteCallbacks_.clear();
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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->remoteCallbacks_.clear();
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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->remoteCallbacks_.clear();

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
 * @tc.name: OnRemoteCallbackDied008 test
 * @tc.desc: test for callback dead, with one device, some callback
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, DoLoadRemoteSystemAbility001, TestSize.Level1)
{
    DTEST_LOG << " DoLoadRemoteSystemAbility001 start " << std::endl;
    /**
     * @tc.steps: step1. init remoteCallbacks_
     * @tc.expected: step1. init remoteCallbacks_
     */
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->remoteCallbacks_.clear();

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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->remoteCallbacks_.clear();

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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->remoteCallbacks_.clear();

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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->remoteCallbacks_.clear();
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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    int32_t uid = 1;
    saMgr->saFrequencyMap_.clear();
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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    int32_t uid = -1;
    saMgr->saFrequencyMap_.clear();
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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    int32_t uid = 1;
    uint64_t key = SamgrUtil::GenerateFreKey(uid, TEST_SYSTEM_ABILITY1);
    saMgr->saFrequencyMap_[key] = MAX_COUNT;
    int32_t count = saMgr->UpdateSaFreMap(uid, TEST_SYSTEM_ABILITY1);
    EXPECT_EQ(saMgr->saFrequencyMap_[key], MAX_COUNT);
    saMgr->saFrequencyMap_.clear();
}

/**
 * @tc.name: Get param debug
 * @tc.desc: ReportGetSAFre002
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, GetParamDebug001, TestSize.Level1)
{
    DTEST_LOG << " GetParamDebug001 " << std::endl;
    bool value = system::GetBoolParameter(ONDEMAND_PARAM, false);
    ASSERT_TRUE(value);
}

/**
 * @tc.name: Test OndemandLoadForPerf
 * @tc.desc: OndemandLoadForPerf001
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, OndemandLoadForPerf001, TestSize.Level3)
{
    DTEST_LOG << " OndemandLoadForPerf001 " << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->SetFfrt();
    saMgr->OndemandLoadForPerf();
    saMgr->Init();
    saMgr->OndemandLoadForPerf();
    usleep(ONDEMAND_SLEEP_TIME);
    bool value = system::GetBoolParameter(ONDEMAND_PARAM, false);
    ASSERT_TRUE(value);
}

/**
 * @tc.name: OndemandLoadForPerf002
 * @tc.desc: test OndemandLoadForPerf, workHandler_ is nullptr
 * @tc.type: FUNC
 * @tc.require: I6MO6A
 */
HWTEST_F(SystemAbilityMgrTest, OndemandLoadForPerf002, TestSize.Level3)
{
    DTEST_LOG << " OndemandLoadForPerf002 " << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->workHandler_ = nullptr;
    saMgr->OndemandLoadForPerf();
    EXPECT_NE(saMgr, nullptr);
}


/**
 * @tc.name: Test GetAllOndemandSa001
 * @tc.desc: GetAllOndemandSa001
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, GetAllOndemandSa001, TestSize.Level3)
{
    DTEST_LOG << " GetAllOndemandSa001 " << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    SaProfile saProfile;
    saMgr->saProfileMap_[1] = saProfile;
    saMgr->GetAllOndemandSa();
    bool value = system::GetBoolParameter(ONDEMAND_PARAM, false);
    EXPECT_TRUE(value);
    saMgr->saProfileMap_.clear();
}

/**
 * @tc.name: Test GetAllOndemandSa002
 * @tc.desc: GetAllOndemandSa002
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, GetAllOndemandSa002, TestSize.Level3)
{
    DTEST_LOG << " GetAllOndemandSa002 " << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    SaProfile saProfile;
    saMgr->saProfileMap_[1] = saProfile;
    SAInfo saInfo;
    saMgr->abilityMap_[1] = saInfo;
    saMgr->GetAllOndemandSa();
    bool value = system::GetBoolParameter(ONDEMAND_PARAM, false);
    EXPECT_TRUE(value);
    saMgr->saProfileMap_.clear();
    saMgr->abilityMap_.clear();
}

/**
 * @tc.name: GetAllOndemandSa003
 * @tc.desc: test GetAllOndemandSa with saProfileMap_ is empty
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrTest, GetAllOndemandSa003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->saProfileMap_.clear();
    auto ret = saMgr->GetAllOndemandSa();
    EXPECT_TRUE(ret.empty());
}

/**
 * @tc.name: GetAllOndemandSa004
 * @tc.desc: test GetAllOndemand with saProfileMap_ is not  empty
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrTest, GetAllOndemandSa004, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    SaProfile saProfile;
    saMgr->saProfileMap_.clear();
    saMgr->saProfileMap_[SAID] = saProfile;
    auto ret = saMgr->GetAllOndemandSa();
    EXPECT_FALSE(ret.empty());
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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    saMgr->abilityStateScheduler_ = systemAbilityStateScheduler;
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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    saMgr->abilityStateScheduler_ = systemAbilityStateScheduler;
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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->abilityStateScheduler_ = nullptr;
    std::list<SystemProcessInfo> systemProcessInfos;
    int32_t ret = saMgr->GetRunningSystemProcess(systemProcessInfos);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: Test SubscribeSystemProcess001
 * @tc.desc: SubscribeSystemProcess001
 * @tc.type: FUNC
 * @tc.require: I6H10P
 */
HWTEST_F(SystemAbilityMgrTest, SubscribeSystemProcess001, TestSize.Level3)
{
    DTEST_LOG << " SubscribeSystemProcess001 " << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<ISystemProcessStatusChange> systemProcessStatusChange = new SystemProcessStatusChange();
    saMgr->abilityStateScheduler_ = std::make_shared<SystemAbilityStateScheduler>();
    int32_t ret = saMgr->SubscribeSystemProcess(systemProcessStatusChange);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: Test SubscribeSystemProcess002
 * @tc.desc: SubscribeSystemProcess002
 * @tc.type: FUNC
 * @tc.require: I6H10P
 */
HWTEST_F(SystemAbilityMgrTest, SubscribeSystemProcess002, TestSize.Level3)
{
    DTEST_LOG << " SubscribeSystemProcess002 " << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->abilityStateScheduler_ = nullptr;
    sptr<ISystemProcessStatusChange> systemProcessStatusChange = new SystemProcessStatusChange();
    int32_t ret = saMgr->SubscribeSystemProcess(systemProcessStatusChange);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: SubscribeSystemProcess003
 * @tc.desc: test SubscribeSystemProcess with abilityStateScheduler_ is nullptr
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrTest, SubscribeSystemProcess003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<SystemProcessStatusChange> listener = new SystemProcessStatusChange();
    saMgr->abilityStateScheduler_ = nullptr;
    std::list<SystemProcessInfo> systemProcessInfos;
    int32_t ret = saMgr->SubscribeSystemProcess(listener);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: Test UnSubscribeSystemProcess001
 * @tc.desc: UnSubscribeSystemProcess001
 * @tc.type: FUNC
 * @tc.require: I6H10P
 */
HWTEST_F(SystemAbilityMgrTest, UnSubscribeSystemProcess001, TestSize.Level3)
{
    DTEST_LOG << " UnSubscribeSystemProcess001" << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<ISystemProcessStatusChange> systemProcessStatusChange = new SystemProcessStatusChange();
    int32_t ret = saMgr->UnSubscribeSystemProcess(systemProcessStatusChange);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: Test UnSubscribeSystemProcess002
 * @tc.desc: UnSubscribeSystemProcess002
 * @tc.type: FUNC
 * @tc.require: I6H10P
 */
HWTEST_F(SystemAbilityMgrTest, UnSubscribeSystemProcess002, TestSize.Level3)
{
    DTEST_LOG << " UnSubscribeSystemProcess002" << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<ISystemProcessStatusChange> systemProcessStatusChange = new SystemProcessStatusChange();
    saMgr->abilityStateScheduler_ = std::make_shared<SystemAbilityStateScheduler>();
    int32_t ret = saMgr->UnSubscribeSystemProcess(systemProcessStatusChange);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: UnSubscribeSystemProcess003
 * @tc.desc: test UnSubscribeSystemProcess with abilityStateScheduler_ is nullptr
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrTest, UnSubscribeSystemProcess003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<SystemProcessStatusChange> listener = new SystemProcessStatusChange();
    saMgr->abilityStateScheduler_ = nullptr;
    std::list<SystemProcessInfo> systemProcessInfos;
    int32_t ret = saMgr->UnSubscribeSystemProcess(listener);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: Test OnSystemProcessStarted001
 * @tc.desc: OnSystemProcessStarted
 * @tc.type: FUNC
 * @tc.require: I6H10P
 */
HWTEST_F(SystemAbilityMgrTest, OnSystemProcessStarted001, TestSize.Level3)
{
    DTEST_LOG << " OnSystemProcessStarted001" << std::endl;
    sptr<ISystemProcessStatusChange> systemProcessStatusChange = new SystemProcessStatusChange();
    SystemProcessInfo systemProcessInfos;
    systemProcessStatusChange->OnSystemProcessStarted(systemProcessInfos);
    EXPECT_NE(systemProcessStatusChange, nullptr);
}

/**
 * @tc.name: Test OnSystemProcessStopped001
 * @tc.desc: OnSystemProcessStopped001
 * @tc.type: FUNC
 * @tc.require: I6H10P
 */
HWTEST_F(SystemAbilityMgrTest, OnSystemProcessStopped001, TestSize.Level3)
{
    DTEST_LOG << " OnSystemProcessStopped001" << std::endl;
    sptr<SystemProcessStatusChangeStub> systemProcessStatusChange = new SystemProcessStatusChange();
    SystemProcessInfo systemProcessInfos;
    systemProcessStatusChange->OnSystemProcessStopped(systemProcessInfos);
    EXPECT_NE(systemProcessStatusChange, nullptr);
}

/**
 * @tc.name: Test SendRequestInner001
 * @tc.desc: SendRequestInner001
 * @tc.type: FUNC
 * @tc.require: I6H10P
 */
HWTEST_F(SystemAbilityMgrTest, SendRequestInner001, TestSize.Level3)
{
    DTEST_LOG << " SendRequestInner001" << std::endl;
    sptr<SystemProcessStatusChangeStub> stub = new SystemProcessStatusChange();
    sptr<SystemProcessStatusChangeProxy> systemProcessStatusChange = new SystemProcessStatusChangeProxy(stub);
    SystemProcessInfo systemProcessInfos;
    uint32_t code = 1;
    bool ret = systemProcessStatusChange->SendRequestInner(code, systemProcessInfos);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: Test SendRequestInner002
 * @tc.desc: SendRequestInner002
 * @tc.type: FUNC
 * @tc.require: I6H10P
 */
HWTEST_F(SystemAbilityMgrTest, SendRequestInner002, TestSize.Level3)
{
    DTEST_LOG << " SendRequestInner002" << std::endl;
    sptr<SystemProcessStatusChangeStub> stub = new SystemProcessStatusChange();
    sptr<SystemProcessStatusChangeProxy> systemProcessStatusChange = new SystemProcessStatusChangeProxy(stub);
    SystemProcessInfo systemProcessInfos;
    systemProcessInfos.processName = "test";
    uint32_t code = 1;
    bool ret = systemProcessStatusChange->SendRequestInner(code, systemProcessInfos);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: CancelUnloadSystemAbility001
 * @tc.desc: test CancelUnloadSystemAbility, said is invalid
 * @tc.type: FUNC
 * @tc.require: I6J4T7
 */
HWTEST_F(SystemAbilityMgrTest, CancelUnloadSystemAbility001, TestSize.Level3)
{
    DTEST_LOG << " CancelUnloadSystemAbility001 " << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    int32_t systemAbilityId = -1;
    int32_t ret = saMgr->CancelUnloadSystemAbility(systemAbilityId);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: CancelUnloadSystemAbility002
 * @tc.desc: test CancelUnloadSystemAbility, said is invalid
 * @tc.type: FUNC
 * @tc.require: I6J4T7
 */
HWTEST_F(SystemAbilityMgrTest, CancelUnloadSystemAbility002, TestSize.Level3)
{
    DTEST_LOG << " CancelUnloadSystemAbility002 " << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->saProfileMap_.erase(1);
    int32_t systemAbilityId = 1;
    int32_t ret = saMgr->CancelUnloadSystemAbility(systemAbilityId);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: CancelUnloadSystemAbility003
 * @tc.desc: test CancelUnloadSystemAbility, caller process is invalid
 * @tc.type: FUNC
 * @tc.require: I6J4T7
 */
HWTEST_F(SystemAbilityMgrTest, CancelUnloadSystemAbility003, TestSize.Level3)
{
    DTEST_LOG << " CancelUnloadSystemAbility003 " << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    SaProfile saProfile;
    saMgr->saProfileMap_[1] = saProfile;
    int32_t systemAbilityId = 1;
    int32_t ret = saMgr->CancelUnloadSystemAbility(systemAbilityId);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: CancelUnloadSystemAbility004
 * @tc.desc: test CancelUnloadSystemAbility, caller process is valid
 * @tc.type: FUNC
 * @tc.require: I6J4T7
 */
HWTEST_F(SystemAbilityMgrTest, CancelUnloadSystemAbility004, TestSize.Level3)
{
    DTEST_LOG << " CancelUnloadSystemAbility004 " << std::endl;
    SamMockPermission::MockProcess("mockProcess");
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    SaProfile saProfile;
    saProfile.process = u"mockProcess";
    saMgr->saProfileMap_[1] = saProfile;
    int32_t systemAbilityId = 1;
    int32_t ret = saMgr->CancelUnloadSystemAbility(systemAbilityId);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: CancelUnloadSystemAbility005
 * @tc.desc: test CancelUnloadSystemAbility, abilityStateScheduler_ is nullptr
 * @tc.type: FUNC
 * @tc.require: I6J4T7
 */
HWTEST_F(SystemAbilityMgrTest, CancelUnloadSystemAbility005, TestSize.Level3)
{
    DTEST_LOG << " CancelUnloadSystemAbility005 " << std::endl;
    SamMockPermission::MockProcess("mockProcess");
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    SaProfile saProfile;
    saProfile.process = u"mockProcess";
    saMgr->saProfileMap_[1] = saProfile;
    int32_t systemAbilityId = 1;
    saMgr->abilityStateScheduler_ = nullptr;
    int32_t ret = saMgr->CancelUnloadSystemAbility(systemAbilityId);
    saMgr->abilityStateScheduler_ = std::make_shared<SystemAbilityStateScheduler>();
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: CancelUnloadSystemAbility006
 * @tc.desc: test CancelUnloadSystemAbility, abilityStateScheduler_ is nullptr
 * @tc.type: FUNC
 * @tc.require: I6J4T7
 */
HWTEST_F(SystemAbilityMgrTest, CancelUnloadSystemAbility006, TestSize.Level3)
{
    DTEST_LOG << " CancelUnloadSystemAbility006 " << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    
    uint32_t accessToken = IPCSkeleton::GetCallingTokenID();
    Security::AccessToken::NativeTokenInfo nativeTokenInfo;
    int32_t result = Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(accessToken, nativeTokenInfo);
    EXPECT_TRUE(result == ERR_OK);
    SaProfile saProfile;
    saProfile.saId = TEST_OVERFLOW_SAID;
    saProfile.process = Str8ToStr16(nativeTokenInfo.processName);
    saMgr->saProfileMap_[TEST_OVERFLOW_SAID] = saProfile;
    std::shared_ptr<SystemAbilityStateScheduler> saScheduler = saMgr->abilityStateScheduler_;
    saMgr->abilityStateScheduler_ = nullptr;
    int32_t ret = saMgr->CancelUnloadSystemAbility(TEST_OVERFLOW_SAID);
    saMgr->abilityStateScheduler_ = saScheduler;
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    saMgr->saProfileMap_.erase(TEST_OVERFLOW_SAID);
}

/**
 * @tc.name: IdleSystemAbility001
 * @tc.desc: test IdleSystemAbility, said is invalid
 * @tc.type: FUNC
 * @tc.require: I6J4T7
 */
HWTEST_F(SystemAbilityMgrTest, IdleSystemAbility001, TestSize.Level3)
{
    DTEST_LOG << " IdleSystemAbility001 " << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    int32_t systemAbilityId = -1;
    std::u16string procName;
    nlohmann::json idleReason;
    int32_t delayTime = 0;
    bool ret = saMgr->IdleSystemAbility(systemAbilityId, procName, idleReason, delayTime);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IdleSystemAbility002
 * @tc.desc: test IdleSystemAbility, return false
 * @tc.type: FUNC
 * @tc.require: I6MO6A
 */
HWTEST_F(SystemAbilityMgrTest, IdleSystemAbility002, TestSize.Level3)
{
    DTEST_LOG << " IdleSystemAbility002 " << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    int32_t systemAbilityId = 401;
    std::u16string procName;
    nlohmann::json idleReason;
    int32_t delayTime = 0;
    bool ret = saMgr->IdleSystemAbility(systemAbilityId, procName, idleReason, delayTime);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IdleSystemAbility003
 * @tc.desc: test IdleSystemAbility, return false
 * @tc.type: FUNC
 * @tc.require: I6MO6A
 */
HWTEST_F(SystemAbilityMgrTest, IdleSystemAbility003, TestSize.Level3)
{
    DTEST_LOG << " IdleSystemAbility003 " << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<IRemoteObject> testAbility(new SaStatusChangeMock());
    SAInfo saInfo;
    saInfo.remoteObj = testAbility;
    saMgr->abilityMap_[TEST_OVERFLOW_SAID] = saInfo;
    nlohmann::json idleReason;
    int32_t delayTime = 0;
    bool ret = saMgr->IdleSystemAbility(TEST_OVERFLOW_SAID, u"test", idleReason, delayTime);
    EXPECT_FALSE(ret);
    saMgr->abilityMap_.erase(TEST_OVERFLOW_SAID);
}

/**
 * @tc.name: ActiveSystemAbility001
 * @tc.desc: test ActiveSystemAbility001, said is invalid
 * @tc.type: FUNC
 * @tc.require: I6J4T7
 */
HWTEST_F(SystemAbilityMgrTest, ActiveSystemAbility001, TestSize.Level3)
{
    DTEST_LOG << " ActiveSystemAbility001 " << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    int32_t systemAbilityId = -1;
    std::u16string procName;
    nlohmann::json activeReason;
    bool ret = saMgr->ActiveSystemAbility(systemAbilityId, procName, activeReason);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: ActiveSystemAbility002
 * @tc.desc: test ActiveSystemAbility002, said is valid
 * @tc.type: FUNC
 * @tc.require: I6MO6A
 */
HWTEST_F(SystemAbilityMgrTest, ActiveSystemAbility002, TestSize.Level3)
{
    DTEST_LOG << " ActiveSystemAbility002 " << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    int32_t systemAbilityId = 401;
    std::u16string procName;
    nlohmann::json activeReason;
    bool ret = saMgr->ActiveSystemAbility(systemAbilityId, procName, activeReason);
    EXPECT_FALSE(ret);
}


/**
 * @tc.name: ActiveSystemAbility003
 * @tc.desc: test ActiveSystemAbility, return false
 * @tc.type: FUNC
 * @tc.require: I6MO6A
 */
HWTEST_F(SystemAbilityMgrTest, ActiveSystemAbility003, TestSize.Level3)
{
    DTEST_LOG << " ActiveSystemAbility003 " << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<IRemoteObject> testAbility(new SaStatusChangeMock());
    SAInfo saInfo;
    saInfo.remoteObj = testAbility;
    saMgr->abilityMap_[TEST_OVERFLOW_SAID] = saInfo;
    nlohmann::json activeReason;
    bool ret = saMgr->ActiveSystemAbility(TEST_OVERFLOW_SAID, u"test", activeReason);
    EXPECT_FALSE(ret);
    saMgr->abilityMap_.erase(TEST_OVERFLOW_SAID);
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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_NE(saMgr, nullptr);
}

/**
 * @tc.name: ProcessOnDemandEvent001
 * @tc.desc: test ProcessOnDemandEvent, abilityStateScheduler_ is nullptr
 * @tc.type: FUNC
 * @tc.require: I6MO6A
 */
HWTEST_F(SystemAbilityMgrTest, ProcessOnDemandEvent001, TestSize.Level3)
{
    DTEST_LOG << " ProcessOnDemandEvent001 " << std::endl;
    OnDemandEvent event;
    std::list<SaControlInfo> saControlList;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->abilityStateScheduler_ = nullptr;
    saMgr->ProcessOnDemandEvent(event, saControlList);
    EXPECT_NE(saMgr, nullptr);
}

/**
 * @tc.name: ProcessOnDemandEvent002
 * @tc.desc: test ProcessOnDemandEvent, saControl.ondemandId == START_ON_DEMAND
 * @tc.type: FUNC
 * @tc.require: I6MO6A
 */
HWTEST_F(SystemAbilityMgrTest, ProcessOnDemandEvent002, TestSize.Level3)
{
    DTEST_LOG << " ProcessOnDemandEvent002 " << std::endl;
    OnDemandEvent event;
    std::list<SaControlInfo> saControlList;
    SaControlInfo saControlInfo;
    saControlInfo.ondemandId = START_ON_DEMAND;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->abilityStateScheduler_ = std::make_shared<SystemAbilityStateScheduler>();
    saMgr->ProcessOnDemandEvent(event, saControlList);
    EXPECT_NE(saMgr, nullptr);
}

/**
 * @tc.name: ProcessOnDemandEvent003
 * @tc.desc: test ProcessOnDemandEvent, saControl.ondemandId == STOP_ON_DEMAND
 * @tc.type: FUNC
 * @tc.require: I6MO6A
 */
HWTEST_F(SystemAbilityMgrTest, ProcessOnDemandEvent003, TestSize.Level3)
{
    DTEST_LOG << " ProcessOnDemandEvent003 " << std::endl;
    OnDemandEvent event;
    std::list<SaControlInfo> saControlList;
    SaControlInfo saControlInfo;
    saControlInfo.ondemandId = STOP_ON_DEMAND;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->ProcessOnDemandEvent(event, saControlList);
    EXPECT_NE(saMgr, nullptr);
}

/**
 * @tc.name: ProcessOnDemandEvent004
 * @tc.desc: test ProcessOnDemandEvent, saControl.ondemandId == other
 * @tc.type: FUNC
 * @tc.require: I6MO6A
 */
HWTEST_F(SystemAbilityMgrTest, ProcessOnDemandEvent004, TestSize.Level3)
{
    DTEST_LOG << " ProcessOnDemandEvent003 " << std::endl;
    OnDemandEvent event;
    std::list<SaControlInfo> saControlList;
    SaControlInfo saControlInfo;
    saControlInfo.ondemandId = OTHER_ON_DEMAND;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->ProcessOnDemandEvent(event, saControlList);
    EXPECT_NE(saMgr, nullptr);
}

/**
 * @tc.name: ProcessOnDemandEvent005
 * @tc.desc: test ProcessOnDemandEvent with saControl's ondemandId is START_ON_DEMAND
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrTest, ProcessOnDemandEvent005, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    saMgr->abilityStateScheduler_ = systemAbilityStateScheduler;
    OnDemandEvent onDemandEvent;
    std::list<OnDemandEvent> onDemandList;
    saMgr->abilityStateScheduler_->startEnableOnceMap_.clear();
    saMgr->abilityStateScheduler_->startEnableOnceMap_[SAID] = onDemandList;
    SaControlInfo saControlInfo;
    saControlInfo.saId = SAID;
    saControlInfo.ondemandId = START_ON_DEMAND;
    saControlInfo.enableOnce = true;
    std::list<SaControlInfo> saControlList;
    saControlList.emplace_back(saControlInfo);
    saMgr->ProcessOnDemandEvent(onDemandEvent, saControlList);
    EXPECT_TRUE(saMgr->abilityStateScheduler_->startEnableOnceMap_.empty());
}

/**
 * @tc.name: ProcessOnDemandEvent006
 * @tc.desc: test ProcessOnDemandEvent with saControl's ondemandId is STOP_ON_DEMAND
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrTest, ProcessOnDemandEvent006, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    saMgr->abilityStateScheduler_ = systemAbilityStateScheduler;
    OnDemandEvent onDemandEvent;
    std::list<OnDemandEvent> onDemandList;
    saMgr->abilityStateScheduler_->stopEnableOnceMap_.clear();
    saMgr->abilityStateScheduler_->stopEnableOnceMap_[SAID] = onDemandList;
    SaControlInfo saControlInfo;
    saControlInfo.saId = SAID;
    saControlInfo.ondemandId = STOP_ON_DEMAND;
    saControlInfo.enableOnce = true;
    std::list<SaControlInfo> saControlList;
    saControlList.emplace_back(saControlInfo);
    saMgr->ProcessOnDemandEvent(onDemandEvent, saControlList);
    EXPECT_TRUE(saMgr->abilityStateScheduler_->stopEnableOnceMap_.empty());
}

/**
 * @tc.name: ProcessOnDemandEvent007
 * @tc.desc: test ProcessOnDemandEvent, saControl.ondemandId == START_ON_DEMAND
 * @tc.type: FUNC
 * @tc.require: I6MO6A
 */
HWTEST_F(SystemAbilityMgrTest, ProcessOnDemandEvent007, TestSize.Level3)
{
    DTEST_LOG << " ProcessOnDemandEvent007 " << std::endl;
    OnDemandEvent event;
    std::list<SaControlInfo> saControlList;
    SaControlInfo saControlInfo;
    saControlInfo.ondemandId = START_ON_DEMAND;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->abilityStateScheduler_ = std::make_shared<SystemAbilityStateScheduler>();
    saMgr->ProcessOnDemandEvent(event, saControlList);
    EXPECT_NE(saMgr, nullptr);
}

/**
 * @tc.name: StopOnDemandAbilityInner001
 * @tc.desc: test StopOnDemandAbilityInner, procObject is empty
 * @tc.type: FUNC
 * @tc.require: I6MO6A
 */
HWTEST_F(SystemAbilityMgrTest, StopOnDemandAbilityInner001, TestSize.Level3)
{
    DTEST_LOG << " StopOnDemandAbilityInner001 " << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    std::u16string procName = u"listen_test1";
    int32_t systemAbilityId = 1494;
    OnDemandEvent event;
    bool ret = saMgr->StopOnDemandAbilityInner(procName, systemAbilityId, event);
    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    SystemAbilityManager::CallbackList mockCallbackMap1 = {{mockLoadCallback1, 1}};
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    saMgr->RemoveStartingAbilityCallback(mockCallbackMap1, testAbility);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: StopOnDemandAbilityInner002
 * @tc.desc: test StopOnDemandAbilityInner, procObject is no empty
 * @tc.type: FUNC
 * @tc.require: I6MO6A
 */
HWTEST_F(SystemAbilityMgrTest, StopOnDemandAbilityInner002, TestSize.Level3)
{
    DTEST_LOG << " StopOnDemandAbilityInner002 " << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    std::u16string procName = u"foundation";
    int32_t systemAbilityId = 401;
    OnDemandEvent event;
    bool ret = saMgr->StopOnDemandAbilityInner(procName, systemAbilityId, event);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: StopOnDemandAbility001
 * @tc.desc: test StopOnDemandAbility001
 * @tc.type: FUNC
 * @tc.require: I6MO6A
 */
HWTEST_F(SystemAbilityMgrTest, StopOnDemandAbility001, TestSize.Level3)
{
    DTEST_LOG << " StopOnDemandAbility001 " << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    std::u16string procName = u"";
    int32_t systemAbilityId = 1;
    OnDemandEvent event;
    bool ret = saMgr->StopOnDemandAbility(procName, systemAbilityId, event);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: DoLoadOnDemandAbility001
 * @tc.desc: test DoLoadOnDemandAbility, abilityProxy is no nullptr
 * @tc.type: FUNC
 * @tc.require: I6MO6A
 */
HWTEST_F(SystemAbilityMgrTest, DoLoadOnDemandAbility001, TestSize.Level0)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    ISystemAbilityManager::SAExtraProp saExtraProp;
    saMgr->AddSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID, testAbility, saExtraProp);
    bool isExist = true;
    bool result = saMgr->DoLoadOnDemandAbility(DISTRIBUTED_SCHED_TEST_TT_ID, isExist);
    EXPECT_EQ(result, true);
    SaProfile saProfile = {u"test", DISTRIBUTED_SCHED_TEST_TT_ID};
    saProfile.cacheCommonEvent = true;
    saMgr->saProfileMap_[DISTRIBUTED_SCHED_TEST_TT_ID] = saProfile;
    int32_t ret = saMgr->RemoveSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID);
    EXPECT_EQ(ret, ERR_OK);
    saMgr->saProfileMap_.erase(DISTRIBUTED_SCHED_TEST_TT_ID);
}

/**
 * @tc.name: RemoveSystemAbility006
 * @tc.desc: test RemoveSystemAbility, ERR_INVALID_VALUE.
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrTest, RemoveSystemAbility006, TestSize.Level0)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    ISystemAbilityManager::SAExtraProp saExtraProp;
    saMgr->AddSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID, testAbility, saExtraProp);
    std::shared_ptr<SystemAbilityStateScheduler> saScheduler = saMgr->abilityStateScheduler_;
    saMgr->abilityStateScheduler_ = nullptr;
    int32_t ret = saMgr->RemoveSystemAbility(testAbility);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    saMgr->abilityStateScheduler_ = saScheduler;
}

/**
 * @tc.name: RemoveSystemAbility007
 * @tc.desc: test RemoveSystemAbility, ERR_OK.
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrTest, RemoveSystemAbility007, TestSize.Level0)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    ISystemAbilityManager::SAExtraProp saExtraProp;
    saMgr->AddSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID, testAbility, saExtraProp);
    SaProfile saProfile = {u"test", DISTRIBUTED_SCHED_TEST_TT_ID};
    saProfile.cacheCommonEvent = true;
    saMgr->saProfileMap_[DISTRIBUTED_SCHED_TEST_TT_ID] = saProfile;
    int32_t ret = saMgr->RemoveSystemAbility(testAbility);
    EXPECT_EQ(ret, ERR_OK);
    saMgr->saProfileMap_.erase(DISTRIBUTED_SCHED_TEST_TT_ID);
}

/**
 * @tc.name: AddSystemProcess001
 * @tc.desc: test AddSystemProcess, abilityStateScheduler_ is nullptr
 * @tc.type: FUNC
 * @tc.require: I6MO6A
 */
HWTEST_F(SystemAbilityMgrTest, AddSystemProcess001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    saMgr->abilityStateScheduler_ = nullptr;
    int32_t result = saMgr->RemoveSystemProcess(testAbility);
    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    std::map<std::string, SystemAbilityManager::CallbackList> mockCallbackMap1 = {
        {"111111", {}}
    };
    SystemAbilityManager::AbilityItem mockAbilityItem1 = {
        SystemAbilityManager::AbilityState::INIT, mockCallbackMap1
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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    saMgr->abilityStateScheduler_ = nullptr;
    saMgr->systemProcessMap_[u"test"] = testAbility;
    int32_t result = saMgr->RemoveSystemProcess(testAbility);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: DoUnloadSystemAbility001
 * @tc.desc: test DoUnloadSystemAbility, targetObject is no nullptr
 * @tc.type: FUNC
 * @tc.require: I6MO6A
 */
HWTEST_F(SystemAbilityMgrTest, DoUnloadSystemAbility001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    std::u16string procName = u"foundation";
    int32_t said = 401;
    OnDemandEvent event;
    bool result = saMgr->DoUnloadSystemAbility(said, procName, event);
    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    std::map<std::string, SystemAbilityManager::CallbackList> mockCallbackMap1 = {
        {"111111", {{mockLoadCallback1, 1}}}
    };
    SystemAbilityManager::AbilityItem mockAbilityItem1 = {
        SystemAbilityManager::AbilityState::INIT, mockCallbackMap1
    };
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    saMgr->RemoveStartingAbilityCallbackForDevice(
        mockAbilityItem1, testAbility);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: DoUnloadSystemAbility002
 * @tc.desc: test DoUnloadSystemAbility with failed to unload system ability
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrTest, DoUnloadSystemAbility002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    ISystemAbilityManager::SAExtraProp saExtraProp(false, 0, u"", u"");
    int32_t systemAbilityId = DISTRIBUTED_SCHED_TEST_TT_ID;
    int32_t result = sm->AddSystemAbility(systemAbilityId, new TestTransactionService(), saExtraProp);
    EXPECT_EQ(result, ERR_OK);
    sptr<IRemoteObject> saObject = sm->CheckSystemAbility(systemAbilityId);
    SAInfo sAInfo;
    sAInfo.remoteObj = saObject;
    saMgr->abilityMap_.clear();
    saMgr->abilityMap_[SAID] = sAInfo;
    OnDemandEvent onDemandEvent;
    int32_t ret = saMgr->DoUnloadSystemAbility(SAID, PROCESS_NAME, onDemandEvent);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: SetDeviceName001
 * @tc.desc: test SetDeviceName
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrTest, SetDeviceName001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->SetDeviceName(DEVICE_NAME);
    EXPECT_EQ(saMgr->deviceName_, DEVICE_NAME);
}

/**
 * @tc.name: GetDeviceName001
 * @tc.desc: test GetDeviceName
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrTest, GetDeviceName001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->SetDeviceName(DEVICE_NAME);
    auto ret = saMgr->GetDeviceName();
    EXPECT_EQ(ret, DEVICE_NAME);
}

/**
 * @tc.name: OnAbilityCallbackDied001
 * @tc.desc: test OnAbilityCallbackDied with remoteObject is nullptr
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrTest, OnAbilityCallbackDied001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->startingAbilityMap_.clear();
    saMgr->OnAbilityCallbackDied(nullptr);
    EXPECT_TRUE(saMgr->startingAbilityMap_.empty());
}
} // namespace OHOS