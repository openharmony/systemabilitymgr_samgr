/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "system_ability_mgr_load_test.h"
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
constexpr int32_t SAID = 1234;
constexpr int32_t OVERFLOW_TIME = 257;
constexpr int32_t TEST_OVERFLOW_SAID = 99999;
constexpr int32_t TEST_EXCEPTION_HIGH_SA_ID = LAST_SYS_ABILITY_ID + 1;
constexpr int32_t TEST_EXCEPTION_LOW_SA_ID = -1;

const std::u16string PROCESS_NAME = u"test_process_name";
const std::u16string SAMANAGER_INTERFACE_TOKEN = u"ohos.samgr.accessToken";

void InitSaMgr(sptr<SystemAbilityManager>& saMgr)
{
    saMgr->abilityDeath_ = sptr<IRemoteObject::DeathRecipient>(new AbilityDeathRecipient());
    saMgr->systemProcessDeath_ = sptr<IRemoteObject::DeathRecipient>(new SystemProcessDeathRecipient());
    saMgr->abilityStatusDeath_ = sptr<IRemoteObject::DeathRecipient>(new AbilityStatusDeathRecipient());
    saMgr->abilityCallbackDeath_ = sptr<IRemoteObject::DeathRecipient>(new AbilityCallbackDeathRecipient());
    saMgr->remoteCallbackDeath_ = sptr<IRemoteObject::DeathRecipient>(new RemoteCallbackDeathRecipient());
    saMgr->workHandler_ = make_shared<FFRTHandler>("workHandler");
    saMgr->collectManager_ = sptr<DeviceStatusCollectManager>(new DeviceStatusCollectManager());
    saMgr->abilityStateScheduler_ = std::make_shared<SystemAbilityStateScheduler>();
}
}

void SystemAbilityMgrLoadTest::SetUpTestCase()
{
    DTEST_LOG << "SetUpTestCase" << std::endl;
}

void SystemAbilityMgrLoadTest::TearDownTestCase()
{
    DTEST_LOG << "TearDownTestCase" << std::endl;
}

void SystemAbilityMgrLoadTest::SetUp()
{
    SamMockPermission::MockPermission();
    DTEST_LOG << "SetUp" << std::endl;
}

void SystemAbilityMgrLoadTest::TearDown()
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->CleanFfrt();
    DTEST_LOG << "TearDown" << std::endl;
}

/**
 * @tc.name: LoadSystemAbility001
 * @tc.desc: load system ability with invalid systemAbilityId.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrLoadTest, LoadSystemAbility001, TestSize.Level0)
{
    DTEST_LOG << "LoadSystemAbility001 begin" << std::endl;
    int32_t systemAbilityId = TEST_EXCEPTION_LOW_SA_ID;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    int32_t result = saMgr->LoadSystemAbility(systemAbilityId, nullptr);
    EXPECT_TRUE(result != ERR_OK);
    DTEST_LOG << "LoadSystemAbility001 end" << std::endl;
}

/**
 * @tc.name: LoadSystemAbility002
 * @tc.desc: load system ability with invalid systemAbilityId.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrLoadTest, LoadSystemAbility002, TestSize.Level0)
{
    DTEST_LOG << "LoadSystemAbility002 begin" << std::endl;
    int32_t systemAbilityId = TEST_EXCEPTION_HIGH_SA_ID;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    int32_t result = saMgr->LoadSystemAbility(systemAbilityId, nullptr);
    EXPECT_TRUE(result != ERR_OK);
    DTEST_LOG << "LoadSystemAbility002 end" << std::endl;
}

/**
 * @tc.name: LoadSystemAbility003
 * @tc.desc: load system ability with invalid callback.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrLoadTest, LoadSystemAbility003, TestSize.Level0)
{
    DTEST_LOG << "LoadSystemAbility003 begin" << std::endl;
    int32_t systemAbilityId = DISTRIBUTED_SCHED_TEST_SO_ID;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    int32_t result = saMgr->LoadSystemAbility(systemAbilityId, nullptr);
    EXPECT_TRUE(result != ERR_OK);
    DTEST_LOG << "LoadSystemAbility003 end" << std::endl;
}

/**
 * @tc.name: LoadSystemAbility004
 * @tc.desc: load system ability with not exist systemAbilityId.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrLoadTest, LoadSystemAbility004, TestSize.Level0)
{
    DTEST_LOG << "LoadSystemAbility004 begin" << std::endl;
    int32_t systemAbilityId = DISTRIBUTED_SCHED_TEST_SO_ID;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    sptr<ISystemAbilityLoadCallback> callback = new SystemAbilityLoadCallbackMock();
    int32_t result = saMgr->LoadSystemAbility(systemAbilityId, callback);
    EXPECT_TRUE(result != ERR_OK);
    DTEST_LOG << "LoadSystemAbility004 end" << std::endl;
}

/**
 * @tc.name: LoadSystemAbility005
 * @tc.desc: test OnRemoteRequest, invalid interface token.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrLoadTest, LoadSystemAbility005, TestSize.Level1)
{
    DTEST_LOG << "LoadSystemAbility005 begin" << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    int32_t result = saMgr->OnRemoteRequest(static_cast<uint32_t>(SamgrInterfaceCode::LOAD_SYSTEM_ABILITY_TRANSACTION),
        data, reply, option);
    EXPECT_TRUE(result != ERR_NONE);
    DTEST_LOG << "LoadSystemAbility005 end" << std::endl;
}

/**
 * @tc.name: LoadSystemAbility006
 * @tc.desc: test OnRemoteRequest, invalid systemAbilityId.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrLoadTest, LoadSystemAbility006, TestSize.Level1)
{
    DTEST_LOG << "LoadSystemAbility006 begin" << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    MessageParcel data;
    data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN);
    MessageParcel reply;
    MessageOption option;
    int32_t result = saMgr->OnRemoteRequest(static_cast<uint32_t>(SamgrInterfaceCode::LOAD_SYSTEM_ABILITY_TRANSACTION),
        data, reply, option);
    EXPECT_TRUE(result != ERR_NONE);
    DTEST_LOG << "LoadSystemAbility006 end" << std::endl;
}

/**
 * @tc.name: LoadSystemAbility007
 * @tc.desc: test OnRemoteRequest, invalid systemAbilityId.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrLoadTest, LoadSystemAbility007, TestSize.Level1)
{
    DTEST_LOG << "LoadSystemAbility007 begin" << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    MessageParcel data;
    data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN);
    data.WriteInt32(TEST_EXCEPTION_HIGH_SA_ID);
    MessageParcel reply;
    MessageOption option;
    int32_t result = saMgr->OnRemoteRequest(static_cast<uint32_t>(SamgrInterfaceCode::LOAD_SYSTEM_ABILITY_TRANSACTION),
        data, reply, option);
    EXPECT_TRUE(result != ERR_NONE);
    DTEST_LOG << "LoadSystemAbility007 end" << std::endl;
}

/**
 * @tc.name: LoadSystemAbility008
 * @tc.desc: test OnRemoteRequest, null callback.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrLoadTest, LoadSystemAbility008, TestSize.Level1)
{
    DTEST_LOG << "LoadSystemAbility008 begin" << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    MessageParcel data;
    data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN);
    data.WriteInt32(DISTRIBUTED_SCHED_TEST_SO_ID);
    MessageParcel reply;
    MessageOption option;
    int32_t result = saMgr->OnRemoteRequest(static_cast<uint32_t>(SamgrInterfaceCode::LOAD_SYSTEM_ABILITY_TRANSACTION),
        data, reply, option);
    EXPECT_TRUE(result != ERR_NONE);
    DTEST_LOG << "LoadSystemAbility008 end" << std::endl;
}

/**
 * @tc.name: LoadSystemAbility009
 * @tc.desc: test OnRemoteRequest, not exist systemAbilityId.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrLoadTest, LoadSystemAbility009, TestSize.Level1)
{
    DTEST_LOG << "LoadSystemAbility009 begin" << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
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
    DTEST_LOG << "LoadSystemAbility009 end" << std::endl;
}

/**
 * @tc.name: LoadSystemAbility010
 * @tc.desc: test LoadSystemAbility with saProfileMap_ is empty
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrLoadTest, LoadSystemAbility010, TestSize.Level3)
{
    DTEST_LOG << "LoadSystemAbility010 begin" << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    sptr<ISystemAbilityLoadCallback> callback = new SystemAbilityLoadCallbackMock();
    int32_t ret = saMgr->LoadSystemAbility(SAID, callback);
    EXPECT_EQ(ret, PROFILE_NOT_EXIST);
    DTEST_LOG << "LoadSystemAbility010 end" << std::endl;
}

/**
 * @tc.name: LoadSystemAbility011
 * @tc.desc: test LoadSystemAbility with invalid said
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrLoadTest, LoadSystemAbility011, TestSize.Level3)
{
    DTEST_LOG << "LoadSystemAbility011 begin" << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    sptr<ISystemAbilityLoadCallback> callback = new SystemAbilityLoadCallbackMock();
    int32_t ret = saMgr->LoadSystemAbility(-1, callback);
    EXPECT_EQ(ret, INVALID_INPUT_PARA);
    DTEST_LOG << "LoadSystemAbility011 end" << std::endl;
}

/**
 * @tc.name: OnLoadSystemAbilitySuccess001
 * @tc.desc: test OnLoadSystemAbilitySuccess, null callback.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrLoadTest, OnLoadSystemAbilitySuccess001, TestSize.Level1)
{
    DTEST_LOG << "OnLoadSystemAbilitySuccess001 begin" << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    saMgr->NotifySystemAbilityLoaded(DISTRIBUTED_SCHED_TEST_SO_ID, nullptr, nullptr);
    EXPECT_TRUE(callback->GetSystemAbilityId() == 0);
    DTEST_LOG << "OnLoadSystemAbilitySuccess001 end" << std::endl;
}

/**
 * @tc.name: OnLoadSystemAbilitySuccess002
 * @tc.desc: test OnLoadSystemAbilitySuccess, null IRemoteObject.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrLoadTest, OnLoadSystemAbilitySuccess002, TestSize.Level1)
{
    DTEST_LOG << "OnLoadSystemAbilitySuccess002 begin" << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    saMgr->Init();
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    saMgr->NotifySystemAbilityLoaded(DISTRIBUTED_SCHED_TEST_SO_ID, nullptr, callback);
    EXPECT_TRUE(callback->GetSystemAbilityId() == DISTRIBUTED_SCHED_TEST_SO_ID);
    EXPECT_TRUE(callback->GetRemoteObject() == nullptr);
    DTEST_LOG << "OnLoadSystemAbilitySuccess002 end" << std::endl;
}

/**
 * @tc.name: OnLoadSystemAbilitySuccess003
 * @tc.desc: test OnLoadSystemAbilitySuccess.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrLoadTest, OnLoadSystemAbilitySuccess003, TestSize.Level1)
{
    DTEST_LOG << "OnLoadSystemAbilitySuccess003 begin" << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    sptr<IRemoteObject> remoteObject = new TestTransactionService();
    saMgr->NotifySystemAbilityLoaded(DISTRIBUTED_SCHED_TEST_SO_ID, remoteObject, callback);
    EXPECT_TRUE(callback->GetSystemAbilityId() == DISTRIBUTED_SCHED_TEST_SO_ID);
    EXPECT_TRUE(callback->GetRemoteObject() == remoteObject);
    DTEST_LOG << "OnLoadSystemAbilitySuccess003 end" << std::endl;
}

/**
 * @tc.name: OnLoadSystemAbilitySuccess004
 * @tc.desc: test OnLoadSystemAbilitySuccess, null callback.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrLoadTest, OnLoadSystemAbilitySuccess004, TestSize.Level1)
{
    DTEST_LOG << " OnLoadSystemAbilitySuccess004 begin" << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    saMgr->NotifySystemAbilityLoaded(DISTRIBUTED_SCHED_TEST_SO_ID, nullptr, nullptr);
    EXPECT_TRUE(callback->GetSystemAbilityId() == 0);
    DTEST_LOG << "OnLoadSystemAbilitySuccess004 end" << std::endl;
}

/**
 * @tc.name: ReportLoadSAOverflow001
 * @tc.desc: ReportLoadSAOverflow001
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrLoadTest, ReportLoadSAOverflow001, TestSize.Level1)
{
    DTEST_LOG << "ReportLoadSAOverflow001 begin" << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    for (int i = 0; i < OVERFLOW_TIME; ++i) {
        sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
        saMgr->LoadSystemAbility(TEST_OVERFLOW_SAID, callback);
    }
    DTEST_LOG << "ReportLoadSAOverflow001 end" << std::endl;
}

/**
 * @tc.name: LoadRemoteSystemAbility001
 * @tc.desc: load system ability with invalid systemAbilityId.
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrLoadTest, LoadRemoteSystemAbility001, TestSize.Level2)
{
    DTEST_LOG << "LoadRemoteSystemAbility001 begin" << std::endl;
    sptr<ISystemAbilityManager> saMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(saMgr != nullptr);
    int32_t systemAbilityId = TEST_EXCEPTION_LOW_SA_ID;
    std::string deviceId = "";
    int32_t result = saMgr->LoadSystemAbility(systemAbilityId, deviceId, nullptr);
    EXPECT_TRUE(result != ERR_OK);
    DTEST_LOG << "LoadRemoteSystemAbility001 end" << std::endl;
}

/**
 * @tc.name: LoadRemoteSystemAbility002
 * @tc.desc: load system ability with invalid systemAbilityId.
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrLoadTest, LoadRemoteSystemAbility002, TestSize.Level2)
{
    DTEST_LOG << "LoadRemoteSystemAbility002 begin" << std::endl;
    sptr<ISystemAbilityManager> saMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(saMgr != nullptr);
    int32_t systemAbilityId = TEST_EXCEPTION_LOW_SA_ID;
    std::string deviceId = "123456789";
    int32_t result = saMgr->LoadSystemAbility(systemAbilityId, deviceId, nullptr);
    EXPECT_TRUE(result != ERR_OK);
    DTEST_LOG << "LoadRemoteSystemAbility002 end" << std::endl;
}

/**
 * @tc.name: LoadRemoteSystemAbility002
 * @tc.desc: load system ability with invalid systemAbilityId.
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrLoadTest, LoadRemoteSystemAbility003, TestSize.Level2)
{
    DTEST_LOG << "LoadRemoteSystemAbility003 begin" << std::endl;
    sptr<ISystemAbilityManager> saMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(saMgr != nullptr);
    int32_t systemAbilityId = -1;
    std::string deviceId = "123456789";
    int32_t result = saMgr->LoadSystemAbility(systemAbilityId, deviceId, nullptr);
    EXPECT_TRUE(result != ERR_OK);
    DTEST_LOG << "LoadRemoteSystemAbility003 end" << std::endl;
}

/**
 * @tc.name: LoadRemoteSystemAbility004
 * @tc.desc: load system ability with invalid systemAbilityId.
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrLoadTest, LoadRemoteSystemAbility004, TestSize.Level2)
{
    DTEST_LOG << "LoadRemoteSystemAbility004 begin" << std::endl;
    sptr<ISystemAbilityManager> saMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(saMgr != nullptr);
    int32_t systemAbilityId = 0;
    std::string deviceId = "123456789";
    int32_t result = saMgr->LoadSystemAbility(systemAbilityId, deviceId, nullptr);
    EXPECT_TRUE(result != ERR_OK);
    DTEST_LOG << "LoadRemoteSystemAbility004 end" << std::endl;
}

/**
 * @tc.name: LoadRemoteSystemAbility004
 * @tc.desc: load system ability with invalid systemAbilityId.
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrLoadTest, LoadRemoteSystemAbility005, TestSize.Level2)
{
    DTEST_LOG << "LoadRemoteSystemAbility005 begin" << std::endl;
    sptr<ISystemAbilityManager> saMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(saMgr != nullptr);
    int32_t systemAbilityId = 0;
    std::string deviceId = "";
    int32_t result = saMgr->LoadSystemAbility(systemAbilityId, deviceId, nullptr);
    EXPECT_TRUE(result != ERR_OK);
    DTEST_LOG << "LoadRemoteSystemAbility005 end" << std::endl;
}

/**
 * @tc.name: LoadRemoteSystemAbility004
 * @tc.desc: load system ability with invalid systemAbilityId.
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrLoadTest, LoadRemoteSystemAbility006, TestSize.Level2)
{
    DTEST_LOG << "LoadRemoteSystemAbility006 begin" << std::endl;
    sptr<ISystemAbilityManager> saMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(saMgr != nullptr);
    int32_t systemAbilityId = -1;
    std::string deviceId = "";
    int32_t result = saMgr->LoadSystemAbility(systemAbilityId, deviceId, nullptr);
    EXPECT_TRUE(result != ERR_OK);
    DTEST_LOG << "LoadRemoteSystemAbility006 end" << std::endl;
}

/**
 * @tc.name: LoadRemoteSystemAbility007
 * @tc.desc: load system ability with invalid systemAbilityId.
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrLoadTest, LoadRemoteSystemAbility007, TestSize.Level2)
{
    DTEST_LOG << " LoadRemoteSystemAbility007 begin" << std::endl;
    sptr<ISystemAbilityManager> saMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(saMgr != nullptr);
    int32_t systemAbilityId = -1;
    std::string deviceId = "1234567890";
    int32_t result = saMgr->LoadSystemAbility(systemAbilityId, deviceId, nullptr);
    EXPECT_TRUE(result != ERR_OK);
    DTEST_LOG << "LoadRemoteSystemAbility007 end" << std::endl;
}

/**
 * @tc.name: LoadSystemAbilityFromRpc001
 * @tc.desc: load system ability with invalid systemAbilityId.
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrLoadTest, LoadSystemAbilityFromRpc001, TestSize.Level2)
{
    DTEST_LOG << "LoadSystemAbilityFromRpc001 begin" << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    std::string deviceId = "";
    int32_t systemAbilityId = -1;
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    bool ret = saMgr->LoadSystemAbilityFromRpc(deviceId, systemAbilityId, callback);
    EXPECT_FALSE(ret);
    DTEST_LOG << "LoadSystemAbilityFromRpc001 end" << std::endl;
}

/**
 * @tc.name: LoadSystemAbilityFromRpc002
 * @tc.desc: load system ability with invalid systemAbilityId.
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrLoadTest, LoadSystemAbilityFromRpc002, TestSize.Level2)
{
    DTEST_LOG << "LoadSystemAbilityFromRpc002 begin" << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    std::string deviceId = "";
    int32_t systemAbilityId = 0;
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    bool ret = saMgr->LoadSystemAbilityFromRpc(deviceId, systemAbilityId, callback);
    EXPECT_FALSE(ret);
    DTEST_LOG << "LoadSystemAbilityFromRpc002 end" << std::endl;
}

/**
 * @tc.name: LoadSystemAbilityFromRpc003
 * @tc.desc: load system ability with invalid systemAbilityId.
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrLoadTest, LoadSystemAbilityFromRpc003, TestSize.Level2)
{
    DTEST_LOG << "LoadSystemAbilityFromRpc003 begin" << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    std::string deviceId = "";
    int32_t systemAbilityId = 0;
    bool ret = saMgr->LoadSystemAbilityFromRpc(deviceId, systemAbilityId, nullptr);
    EXPECT_FALSE(ret);
    DTEST_LOG << "LoadSystemAbilityFromRpc003 end" << std::endl;
}

/**
 * @tc.name: LoadSystemAbilityFromRpc004
 * @tc.desc: load system ability with invalid systemAbilityId.
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrLoadTest, LoadSystemAbilityFromRpc004, TestSize.Level2)
{
    DTEST_LOG << "LoadSystemAbilityFromRpc004 begin" << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    std::string deviceId = "1111111";
    int32_t systemAbilityId = 0;
    bool ret = saMgr->LoadSystemAbilityFromRpc(deviceId, systemAbilityId, nullptr);
    EXPECT_FALSE(ret);
    DTEST_LOG << "LoadSystemAbilityFromRpc004 end" << std::endl;
}

/**
 * @tc.name: LoadSystemAbilityFromRpc005
 * @tc.desc: load system ability with callback is nullptr.
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrLoadTest, LoadSystemAbilityFromRpc005, TestSize.Level2)
{
    DTEST_LOG << " LoadSystemAbilityFromRpc005  begin" << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    std::string deviceId = "2222222";
    int32_t systemAbilityId = 1;
    bool ret = saMgr->LoadSystemAbilityFromRpc(deviceId, systemAbilityId, nullptr);
    EXPECT_FALSE(ret);
    DTEST_LOG << "LoadSystemAbilityFromRpc005 end" << std::endl;
}

/**
 * @tc.name: LoadSystemAbilityFromRpc006
 * @tc.desc: load system ability with sa profile distributed false.
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrLoadTest, LoadSystemAbilityFromRpc006, TestSize.Level2)
{
    DTEST_LOG << "LoadSystemAbilityFromRpc006 begin" << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    std::string deviceId = "2222222";
    int32_t systemAbilityId = 1;
    CommonSaProfile saProfile;
    saMgr->saProfileMap_[1] = saProfile;
    bool ret = saMgr->LoadSystemAbilityFromRpc(deviceId, systemAbilityId, nullptr);
    EXPECT_FALSE(ret);
    DTEST_LOG << "LoadSystemAbilityFromRpc006 end" << std::endl;
}

/**
 * @tc.name: LoadSystemAbilityFromRpc007
 * @tc.desc: load system ability with abilityStateScheduler_ nullptr
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrLoadTest, LoadSystemAbilityFromRpc007, TestSize.Level2)
{
    DTEST_LOG << "LoadSystemAbilityFromRpc007 begin" << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    std::string deviceId = "2222222";
    int32_t systemAbilityId = 1;
    CommonSaProfile saProfile;
    saProfile.distributed = true;
    saMgr->saProfileMap_[1] = saProfile;
    saMgr->abilityStateScheduler_ = nullptr;
    bool ret = saMgr->LoadSystemAbilityFromRpc(deviceId, systemAbilityId, nullptr);
    EXPECT_FALSE(ret);
    DTEST_LOG << "LoadSystemAbilityFromRpc007 end" << std::endl;
}

/**
 * @tc.name: LoadSystemAbilityFromRpc008
 * @tc.desc: load system ability with distributed true
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrLoadTest, LoadSystemAbilityFromRpc008, TestSize.Level2)
{
    DTEST_LOG << "LoadSystemAbilityFromRpc008 begin" << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    std::string deviceId = "2222222";
    int32_t systemAbilityId = 1;
    CommonSaProfile saProfile;
    saProfile.distributed = true;
    saMgr->saProfileMap_[1] = saProfile;
    bool ret = saMgr->LoadSystemAbilityFromRpc(deviceId, systemAbilityId, nullptr);
    EXPECT_FALSE(ret);
    DTEST_LOG << "LoadSystemAbilityFromRpc008 end" << std::endl;
}

/**
 * @tc.name: LoadSystemAbilityFromRpc009
 * @tc.desc: test LoadSystemAbilityFromRpc.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrLoadTest, LoadSystemAbilityFromRpc009, TestSize.Level1)
{
    DTEST_LOG << "LoadSystemAbilityFromRpc009 begin" << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    const std::string srcDeviceId;
    CommonSaProfile saProfile = {u"test", TEST_OVERFLOW_SAID};
    saProfile.distributed = true;
    saMgr->saProfileMap_[TEST_OVERFLOW_SAID] = saProfile;
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    saMgr->abilityStateScheduler_ = nullptr;
    int32_t ret = saMgr->LoadSystemAbilityFromRpc(srcDeviceId, TEST_OVERFLOW_SAID, callback);
    EXPECT_FALSE(ret);
    DTEST_LOG << "LoadSystemAbilityFromRpc009 end" << std::endl;
}

/**
 * @tc.name: UnloadSystemAbility001
 * @tc.desc: UnloadSystemAbility sa not exist
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrLoadTest, UnloadSystemAbility001, TestSize.Level3)
{
    DTEST_LOG << "UnloadSystemAbility001 begin" << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    int32_t systemAbilityId = 1;
    int32_t result = saMgr->UnloadSystemAbility(systemAbilityId);
    EXPECT_EQ(result, PROFILE_NOT_EXIST);
    DTEST_LOG << "UnloadSystemAbility001 end" << std::endl;
}

/**
 * @tc.name: UnloadSystemAbility002
 * @tc.desc: UnloadSystemAbility, caller invalid
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityMgrLoadTest, UnloadSystemAbility002, TestSize.Level3)
{
    DTEST_LOG << "UnloadSystemAbility002 begin" << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    int32_t systemAbilityId = 1;
    CommonSaProfile saProfile;
    saMgr->saProfileMap_[1] = saProfile;
    saMgr->abilityStateScheduler_ = nullptr;
    int32_t result = saMgr->UnloadSystemAbility(systemAbilityId);
    EXPECT_EQ(result, INVALID_CALL_PROC);
    DTEST_LOG << "UnloadSystemAbility002 end" << std::endl;
}
#endif

/**
 * @tc.name: UnloadSystemAbility003
 * @tc.desc: test UnloadSystemAbility004, abilityStateScheduler_ is nullptr
 * @tc.type: FUNC
 * @tc.require: I6J4T7
 */
HWTEST_F(SystemAbilityMgrLoadTest, UnloadSystemAbility003, TestSize.Level3)
{
    DTEST_LOG << " UnloadSystemAbility003 begin" << std::endl;
    SamMockPermission::MockProcess("memmgrservice");
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    CommonSaProfile saProfile;
    saProfile.process = u"memmgrservice";
    saMgr->saProfileMap_[1] = saProfile;
    saMgr->abilityStateScheduler_ = nullptr;
    int32_t systemAbilityId = 1;
    int32_t ret = saMgr->UnloadSystemAbility(systemAbilityId);
    EXPECT_EQ(ret, STATE_SCHEDULER_NULL);
    DTEST_LOG << "UnloadSystemAbility003 end" << std::endl;
}

/**
 * @tc.name: CancelUnloadSystemAbility001
 * @tc.desc: test CancelUnloadSystemAbility, said is invalid
 * @tc.type: FUNC
 * @tc.require: I6J4T7
 */
HWTEST_F(SystemAbilityMgrLoadTest, CancelUnloadSystemAbility001, TestSize.Level3)
{
    DTEST_LOG << " CancelUnloadSystemAbility001 begin" << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    int32_t systemAbilityId = -1;
    int32_t ret = saMgr->CancelUnloadSystemAbility(systemAbilityId);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    DTEST_LOG << "CancelUnloadSystemAbility001 end" << std::endl;
}

/**
 * @tc.name: CancelUnloadSystemAbility002
 * @tc.desc: test CancelUnloadSystemAbility, said is invalid
 * @tc.type: FUNC
 * @tc.require: I6J4T7
 */
HWTEST_F(SystemAbilityMgrLoadTest, CancelUnloadSystemAbility002, TestSize.Level3)
{
    DTEST_LOG << " CancelUnloadSystemAbility002 begin" << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    int32_t systemAbilityId = 1;
    int32_t ret = saMgr->CancelUnloadSystemAbility(systemAbilityId);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    DTEST_LOG << "CancelUnloadSystemAbility002 end" << std::endl;
}

/**
 * @tc.name: CancelUnloadSystemAbility003
 * @tc.desc: test CancelUnloadSystemAbility, caller process is invalid
 * @tc.type: FUNC
 * @tc.require: I6J4T7
 */
HWTEST_F(SystemAbilityMgrLoadTest, CancelUnloadSystemAbility003, TestSize.Level3)
{
    DTEST_LOG << " CancelUnloadSystemAbility003 begin" << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    CommonSaProfile saProfile;
    saMgr->saProfileMap_[1] = saProfile;
    int32_t systemAbilityId = 1;
    int32_t ret = saMgr->CancelUnloadSystemAbility(systemAbilityId);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    DTEST_LOG << "CancelUnloadSystemAbility003 end" << std::endl;
}

/**
 * @tc.name: CancelUnloadSystemAbility004
 * @tc.desc: test CancelUnloadSystemAbility, caller process is valid
 * @tc.type: FUNC
 * @tc.require: I6J4T7
 */
HWTEST_F(SystemAbilityMgrLoadTest, CancelUnloadSystemAbility004, TestSize.Level3)
{
    DTEST_LOG << " CancelUnloadSystemAbility004 begin" << std::endl;
    SamMockPermission::MockProcess("mockProcess");
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    CommonSaProfile saProfile;
    saProfile.process = u"mockProcess";
    saMgr->saProfileMap_[1] = saProfile;
    int32_t systemAbilityId = 1;
    int32_t ret = saMgr->CancelUnloadSystemAbility(systemAbilityId);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    DTEST_LOG << "CancelUnloadSystemAbility004 end" << std::endl;
}

/**
 * @tc.name: CancelUnloadSystemAbility005
 * @tc.desc: test CancelUnloadSystemAbility, abilityStateScheduler_ is nullptr
 * @tc.type: FUNC
 * @tc.require: I6J4T7
 */
HWTEST_F(SystemAbilityMgrLoadTest, CancelUnloadSystemAbility005, TestSize.Level3)
{
    DTEST_LOG << " CancelUnloadSystemAbility005 begin" << std::endl;
    SamMockPermission::MockProcess("mockProcess");
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    CommonSaProfile saProfile;
    saProfile.process = u"mockProcess";
    saMgr->saProfileMap_[1] = saProfile;
    int32_t systemAbilityId = 1;
    saMgr->abilityStateScheduler_ = nullptr;
    int32_t ret = saMgr->CancelUnloadSystemAbility(systemAbilityId);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    DTEST_LOG << "CancelUnloadSystemAbility005 end" << std::endl;
}

/**
 * @tc.name: CancelUnloadSystemAbility006
 * @tc.desc: test CancelUnloadSystemAbility, abilityStateScheduler_ is nullptr
 * @tc.type: FUNC
 * @tc.require: I6J4T7
 */
#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityMgrLoadTest, CancelUnloadSystemAbility006, TestSize.Level3)
{
    DTEST_LOG << " CancelUnloadSystemAbility006 begin" << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    uint32_t accessToken = IPCSkeleton::GetCallingTokenID();
    Security::AccessToken::NativeTokenInfo nativeTokenInfo;
    int32_t result = Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(accessToken, nativeTokenInfo);
    EXPECT_TRUE(result == ERR_OK);
    CommonSaProfile saProfile;
    saProfile.saId = TEST_OVERFLOW_SAID;
    saProfile.process = Str8ToStr16(nativeTokenInfo.processName);
    saMgr->saProfileMap_[TEST_OVERFLOW_SAID] = saProfile;
    saMgr->abilityStateScheduler_ = nullptr;
    int32_t ret = saMgr->CancelUnloadSystemAbility(TEST_OVERFLOW_SAID);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    DTEST_LOG << "CancelUnloadSystemAbility006 end" << std::endl;
}
#endif

/**
 * @tc.name: DoUnloadSystemAbility001
 * @tc.desc: test DoUnloadSystemAbility, targetObject is no nullptr
 * @tc.type: FUNC
 * @tc.require: I6MO6A
 */
HWTEST_F(SystemAbilityMgrLoadTest, DoUnloadSystemAbility001, TestSize.Level3)
{
    DTEST_LOG << "DoUnloadSystemAbility001 begin" << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
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
    DTEST_LOG << "DoUnloadSystemAbility001 end" << std::endl;
}

/**
 * @tc.name: DoUnloadSystemAbility002
 * @tc.desc: test DoUnloadSystemAbility with failed to unload system ability
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrLoadTest, DoUnloadSystemAbility002, TestSize.Level3)
{
    DTEST_LOG << "DoUnloadSystemAbility002 begin" << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    ISystemAbilityManager::SAExtraProp saExtraProp(false, 0, u"", u"");
    int32_t systemAbilityId = DISTRIBUTED_SCHED_TEST_TT_ID;
    int32_t result = saMgr->AddSystemAbility(systemAbilityId, new TestTransactionService(), saExtraProp);
    EXPECT_EQ(result, ERR_OK);
    sptr<IRemoteObject> saObject = saMgr->CheckSystemAbility(systemAbilityId);
    SAInfo sAInfo;
    sAInfo.remoteObj = saObject;
    saMgr->abilityMap_[SAID] = sAInfo;
    OnDemandEvent onDemandEvent;
    int32_t ret = saMgr->DoUnloadSystemAbility(SAID, PROCESS_NAME, onDemandEvent);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    DTEST_LOG << "DoUnloadSystemAbility002 end" << std::endl;
}

/**
 * @tc.name: UnloadAllIdleSystemAbility001
 * @tc.desc: UnloadAllIdleSystemAbility process is memmgrservice
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrLoadTest, UnloadAllIdleSystemAbility001, TestSize.Level3)
{
    DTEST_LOG << "UnloadAllIdleSystemAbility001 begin" << std::endl;
    SamMockPermission::MockProcess("memmgrservice");
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    int32_t ret = saMgr->UnloadAllIdleSystemAbility();
    EXPECT_EQ(ret, ERR_OK);
    DTEST_LOG << "UnloadAllIdleSystemAbility001 end" << std::endl;
}

/**
 * @tc.name: UnloadAllIdleSystemAbility002
 * @tc.desc: UnloadAllIdleSystemAbility abilityStateScheduler_ is null
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrLoadTest, UnloadAllIdleSystemAbility002, TestSize.Level3)
{
    DTEST_LOG << "UnloadAllIdleSystemAbility002 begin" << std::endl;
    SamMockPermission::MockProcess("memmgrservice");
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    saMgr->abilityStateScheduler_ = nullptr;
    int32_t ret = saMgr->UnloadAllIdleSystemAbility();
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    DTEST_LOG << "UnloadAllIdleSystemAbility002 end" << std::endl;
}

/**
 * @tc.name: UnloadAllIdleSystemAbility003
 * @tc.desc: UnloadAllIdleSystemAbility process is not memmgrservice
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityMgrLoadTest, UnloadAllIdleSystemAbility003, TestSize.Level3)
{
    DTEST_LOG << "UnloadAllIdleSystemAbility003 begin" << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    int32_t ret = saMgr->UnloadAllIdleSystemAbility();
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
    DTEST_LOG << "UnloadAllIdleSystemAbility003 end" << std::endl;
}
#endif

/**
 * @tc.name: UnloadProcess001
 * @tc.desc: UnloadProcess
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrLoadTest, UnloadProcess001, TestSize.Level3)
{
    DTEST_LOG << "UnloadProcess001 begin" << std::endl;
    SamMockPermission::MockProcess("memmgrservice");
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    std::vector<std::u16string> processList;
    processList.push_back(u"test");
    int32_t ret = saMgr->UnloadProcess(processList);
    EXPECT_NE(ret, ERR_INVALID_VALUE);
    DTEST_LOG << "UnloadProcess001 end" << std::endl;
}

/**
 * @tc.name: UnloadProcess002
 * @tc.desc: UnloadProcess abilityStateScheduler_ is null
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrLoadTest, UnloadProcess002, TestSize.Level3)
{
    DTEST_LOG << "UnloadProcess002 begin" << std::endl;
    SamMockPermission::MockProcess("memmgrservice");
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    saMgr->abilityStateScheduler_ = nullptr;
    std::vector<std::u16string> processList;
    processList.push_back(u"test");
    int32_t ret = saMgr->UnloadProcess(processList);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    DTEST_LOG << "UnloadProcess002 end" << std::endl;
}

} // namespace OHOS