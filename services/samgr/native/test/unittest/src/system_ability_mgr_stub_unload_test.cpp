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

#include "system_ability_mgr_stub_unload_test.h"
#include "samgr_err_code.h"
#include "ability_death_recipient.h"
#include "itest_transaction_service.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "sa_profiles.h"
#include "sam_mock_permission.h"
#include "string_ex.h"
#include "system_ability_definition.h"
#include "test_log.h"

#define private public
#include "sa_status_change_mock.h"
#include "system_ability_manager.h"
#include "device_param_collect.h"

using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS;

namespace OHOS {
namespace {
constexpr uint32_t SAID = 1499;
constexpr int32_t INVALID_SAID = -1;
}

void SystemAbilityMgrStubUnLoadTest::SetUpTestCase()
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    saMgr->abilityStateScheduler_ = std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    saMgr->abilityStateScheduler_->Init(saProfiles);
    DTEST_LOG << "SetUpTestCase" << std::endl;
}

void SystemAbilityMgrStubUnLoadTest::TearDownTestCase()
{
    DTEST_LOG << "TearDownTestCase" << std::endl;
}

void SystemAbilityMgrStubUnLoadTest::SetUp()
{
    DTEST_LOG << "SetUp" << std::endl;
}

void SystemAbilityMgrStubUnLoadTest::TearDown()
{
    DTEST_LOG << "TearDown" << std::endl;
}

HWTEST_F(SystemAbilityMgrStubUnLoadTest, UnSubsSystemAbilityInner002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(INVALID_SAID);
    int32_t result = saMgr->UnSubsSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

HWTEST_F(SystemAbilityMgrStubUnLoadTest, UnSubsSystemAbilityInner003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    int32_t result = saMgr->UnSubsSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

HWTEST_F(SystemAbilityMgrStubUnLoadTest, UnSubsSystemAbilityInner004, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    sptr<IRemoteObject> testAbility(new SaStatusChangeMock());
    SAInfo saInfo;
    saMgr->abilityMap_[SAID] = saInfo;
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    data.WriteRemoteObject(testAbility);
    int32_t result = saMgr->UnSubsSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_OK);
}

HWTEST_F(SystemAbilityMgrStubUnLoadTest, RemoveSystemAbilityInner002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(INVALID_SAID);
    int32_t result = saMgr->RemoveSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

HWTEST_F(SystemAbilityMgrStubUnLoadTest, RemoveSystemAbilityInner003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    saMgr->abilityMap_.clear();
    int32_t result = saMgr->RemoveSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

HWTEST_F(SystemAbilityMgrStubUnLoadTest, RemoveSystemAbility001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    int32_t res = saMgr->RemoveSystemAbility(INVALID_SAID);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

HWTEST_F(SystemAbilityMgrStubUnLoadTest, RemoveSystemAbility002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    int32_t res = saMgr->RemoveSystemAbility(SAID);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

HWTEST_F(SystemAbilityMgrStubUnLoadTest, RemoveSystemAbility003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    SAInfo saInfo;
    saInfo.remoteObj = nullptr;
    saMgr->abilityMap_[SAID] = saInfo;
    int32_t res = saMgr->RemoveSystemAbility(SAID);
    EXPECT_EQ(res, ERR_OK);
}

HWTEST_F(SystemAbilityMgrStubUnLoadTest, RemoveSystemAbility004, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    SAInfo saInfo;
    saInfo.remoteObj = saMgr;
    saMgr->abilityMap_[SAID] = saInfo;
    saMgr->abilityDeath_ = nullptr;
    int32_t res = saMgr->RemoveSystemAbility(SAID);
    saMgr->abilityDeath_.clear();
    EXPECT_EQ(res, ERR_OK);
}

HWTEST_F(SystemAbilityMgrStubUnLoadTest, RemoveSystemAbility005, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    int32_t res = saMgr->RemoveSystemAbility(nullptr);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

HWTEST_F(SystemAbilityMgrStubUnLoadTest, RemoveSystemAbility006, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    SAInfo saInfo;
    saInfo.remoteObj = saMgr;
    saMgr->abilityMap_[SAID] = saInfo;
    saMgr->abilityDeath_ = nullptr;
    int32_t res = saMgr->RemoveSystemAbility(saMgr);
    EXPECT_EQ(res, ERR_OK);
}

HWTEST_F(SystemAbilityMgrStubUnLoadTest, RemoveSystemAbility007, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    SAInfo saInfo;
    saInfo.remoteObj = saMgr;
    uint32_t saId = 0;
    saMgr->abilityMap_[saId] = saInfo;
    saMgr->abilityDeath_ = nullptr;
    int32_t res = saMgr->RemoveSystemAbility(saMgr);
    EXPECT_EQ(res, ERR_OK);
}

HWTEST_F(SystemAbilityMgrStubUnLoadTest, UnSubscribeSystemAbility001, TestSize.Level0)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    sptr<SaStatusChangeMock> listener(new SaStatusChangeMock());
    int32_t res = saMgr->UnSubscribeSystemAbility(INVALID_SAID, listener);
    u16string name = u"device_saname";
    saMgr->NotifyRemoteSaDied(name);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

HWTEST_F(SystemAbilityMgrStubUnLoadTest, UnSubscribeSystemAbility002, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    sptr<SaStatusChangeMock> listener(nullptr);
    int32_t res = saMgr->UnSubscribeSystemAbility(SAID, listener);
    u16string name = u"deviceSaname";
    saMgr->dBinderService_ = nullptr;
    saMgr->NotifyRemoteSaDied(name);
    saMgr->dBinderService_ = DBinderService::GetInstance();
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

HWTEST_F(SystemAbilityMgrStubUnLoadTest, UnSubscribeSystemAbility003, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    sptr<SaStatusChangeMock> listener(nullptr);
    int32_t res = saMgr->UnSubscribeSystemAbility(INVALID_SAID, listener);
    string deviceId = "device";
    saMgr->NotifyRemoteDeviceOffline(deviceId);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

HWTEST_F(SystemAbilityMgrStubUnLoadTest, UnSubscribeSystemAbility004, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    sptr<SaStatusChangeMock> listener(new SaStatusChangeMock());
    saMgr->listenerMap_[SAID].push_back({listener, SAID});
    saMgr->abilityStatusDeath_ = nullptr;
    saMgr->subscribeCountMap_.clear();
    int32_t res = saMgr->UnSubscribeSystemAbility(SAID, listener);
    saMgr->dBinderService_ = nullptr;
    string deviceId = "device";
    saMgr->NotifyRemoteDeviceOffline(deviceId);
    saMgr->dBinderService_ = DBinderService::GetInstance();
    EXPECT_EQ(res, ERR_OK);
}

HWTEST_F(SystemAbilityMgrStubUnLoadTest, UnSubscribeSystemAbility005, TestSize.Level0)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    sptr<SaStatusChangeMock> listener(new SaStatusChangeMock());
    saMgr->listenerMap_[SAID].push_back({listener, SAID});
    saMgr->abilityStatusDeath_ = nullptr;
    int countNum = 2;
    saMgr->subscribeCountMap_[SAID] = countNum;
    int32_t res = saMgr->UnSubscribeSystemAbility(SAID, listener);
    EXPECT_EQ(res, ERR_OK);
}

HWTEST_F(SystemAbilityMgrStubUnLoadTest, RemoveSystemProcess001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    saMgr->workHandler_ = make_shared<FFRTHandler>("workHandler");
    sptr<IRemoteObject> testAbility(nullptr);
    int32_t res = saMgr->RemoveSystemProcess(testAbility);
    saMgr->NotifySystemAbilityLoadFail(SAID, nullptr);
    u16string name = u"test";
    string srcDeviceId = "srcDeviceId";
    saMgr->startingProcessMap_.clear();
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    SystemAbilityManager::AbilityItem abilityItem;
    saMgr->startingAbilityMap_[SAID] = abilityItem;
    saMgr->CleanCallbackForLoadFailed(SAID, name, srcDeviceId, callback);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: RemoveSystemProcess002
 * @tc.desc: test RemoveSystemProcess, return ERR_OK
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubUnLoadTest, RemoveSystemProcess002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    sptr<IRemoteObject> testAbility(new SaStatusChangeMock());
    u16string procName = u"proname";
    int32_t res = saMgr->AddSystemProcess(procName, testAbility);
    EXPECT_EQ(res, ERR_OK);
    saMgr->systemProcessDeath_ = nullptr;
    res = saMgr->RemoveSystemProcess(testAbility);
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    saMgr->NotifySystemAbilityLoadFail(SAID, callback);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: RemoveSystemProcess003
 * @tc.desc: test RemoveSystemProcess, return ERR_INVALID_VALUE
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubUnLoadTest, RemoveSystemProcess003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    sptr<IRemoteObject> testAbility(new SaStatusChangeMock());
    saMgr->systemProcessMap_.clear();
    int32_t res = saMgr->RemoveSystemProcess(testAbility);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: OnRemoveSystemAbilityInner001
 * @tc.desc: test SystemAbilityStatusChangeStub::OnRemoveSystemAbilityInner001
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubUnLoadTest, OnRemoveSystemAbilityInner001, TestSize.Level3)
{
    sptr<SaStatusChangeMock> testAbility(new SaStatusChangeMock());
    EXPECT_TRUE(testAbility != nullptr);
    MessageParcel data;
    data.WriteInt32(INVALID_SAID);
    MessageParcel reply;
    int32_t result = testAbility->OnRemoveSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: OnRemoveSystemAbilityInner002
 * @tc.desc: test SystemAbilityStatusChangeStub::OnRemoveSystemAbilityInner002
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubUnLoadTest, OnRemoveSystemAbilityInner002, TestSize.Level3)
{
    sptr<SaStatusChangeMock> testAbility(new SaStatusChangeMock());
    EXPECT_TRUE(testAbility != nullptr);
    MessageParcel data;
    data.WriteInt32(SAID);
    std::string deviceId = "";
    data.WriteString(deviceId);
    MessageParcel reply;
    int32_t result = testAbility->OnRemoveSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: UnloadSystemAbilityInner001
 * @tc.desc: call UnloadSystemAbility with invalid said
 * @tc.type: FUNC
 * @tc.require: I6AJ3S
 */
HWTEST_F(SystemAbilityMgrStubUnLoadTest, UnloadSystemAbilityInner001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(INVALID_SAID);
    int32_t result = saMgr->UnloadSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: UnloadSystemAbilityInner002
 * @tc.desc: call UnloadSystemAbility
 * @tc.type: FUNC
 * @tc.require: I6AJ3S
 */
#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityMgrStubUnLoadTest, UnloadSystemAbilityInner002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    CommonSaProfile saProfile;
    saProfile.process = u"test";
    saProfile.distributed = true;
    saProfile.saId = SAID;
    saMgr->saProfileMap_[SAID] = saProfile;
    int32_t result = saMgr->UnloadSystemAbilityInner(data, reply);
    EXPECT_EQ(result, INVALID_CALL_PROC);
    saMgr->saProfileMap_.erase(SAID);
}
#endif

/**
 * @tc.name: UnloadSystemAbility001
 * @tc.desc: call UnloadSystemAbility with invalid said
 * @tc.type: FUNC
 * @tc.require: I6AJ3S
 */
HWTEST_F(SystemAbilityMgrStubUnLoadTest, UnloadSystemAbility001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    int32_t result = saMgr->UnloadSystemAbility(INVALID_SAID);
    EXPECT_EQ(result, PROFILE_NOT_EXIST);
}

/**
 * @tc.name: UnloadSystemAbility002
 * @tc.desc: call UnloadSystemAbility with sa profile is not exist
 * @tc.type: FUNC
 * @tc.require: I6AJ3S
 */
#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityMgrStubUnLoadTest, UnloadSystemAbility002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    CommonSaProfile saProfile;
    saProfile.process = u"test";
    saProfile.distributed = true;
    saProfile.saId = SAID;
    saMgr->saProfileMap_[SAID] = saProfile;
    int32_t result = saMgr->UnloadSystemAbility(SAID);
    EXPECT_EQ(result, INVALID_CALL_PROC);
    saMgr->saProfileMap_.erase(SAID);
}
#endif

/**
 * @tc.name: UnloadSystemAbility003
 * @tc.desc: call UnloadSystemAbility not from native process
 * @tc.type: FUNC
 * @tc.require: I6AJ3S
 */
#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityMgrStubUnLoadTest, UnloadSystemAbility003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    CommonSaProfile saProfile;
    saMgr->saProfileMap_[SAID] = saProfile;
    int32_t result = saMgr->UnloadSystemAbility(SAID);
    EXPECT_EQ(result, INVALID_CALL_PROC);
}
#endif

/**
 * @tc.name: UnloadSystemAbility004
 * @tc.desc: call UnloadSystemAbility from invalid process
 * @tc.type: FUNC
 * @tc.require: I6AJ3S
 */
#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityMgrStubUnLoadTest, UnloadSystemAbility004, TestSize.Level3)
{
    SamMockPermission::MockProcess("invalidProcess");
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    CommonSaProfile saProfile;
    saProfile.process = u"testProcess";
    saMgr->saProfileMap_[SAID] = saProfile;
    int32_t result = saMgr->UnloadSystemAbility(SAID);
    EXPECT_EQ(result, INVALID_CALL_PROC);
}
#endif

/**
 * @tc.name: UnloadSystemAbility005
 * @tc.desc: call UnloadSystemAbility
 * @tc.type: FUNC
 * @tc.require: I6AJ3S
 */
HWTEST_F(SystemAbilityMgrStubUnLoadTest, UnloadSystemAbility005, TestSize.Level3)
{
    SamMockPermission::MockProcess("memmgrservice");
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    saMgr->abilityStateScheduler_->processContextMap_.clear();
    saMgr->abilityStateScheduler_->abilityContextMap_.clear();
    CommonSaProfile saProfile;
    saProfile.process = u"memmgrservice";
    saMgr->saProfileMap_[SAID] = saProfile;
    int32_t result = saMgr->UnloadSystemAbility(SAID);
    EXPECT_EQ(result, GET_SA_CONTEXT_FAIL);
}

/**
 * @tc.name: Test UnSubscribeSystemProcessInner002
 * @tc.desc: UnSubscribeSystemProcessInner002
 * @tc.type: FUNC
 * @tc.require: I6H10P
 */
HWTEST_F(SystemAbilityMgrStubUnLoadTest, UnSubscribeSystemProcessInner002, TestSize.Level3)
{
    DTEST_LOG << "UnSubscribeSystemProcessInner002" << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    data.WriteRemoteObject(nullptr);
    MessageParcel reply;
    int32_t ret = saMgr->UnSubscribeSystemProcessInner(data, reply);
    EXPECT_EQ(ret, ERR_NULL_OBJECT);
}

HWTEST_F(SystemAbilityMgrStubUnLoadTest, UnSubscribeSystemProcessInner003, TestSize.Level3)
{
    DTEST_LOG << "UnSubscribeSystemProcessInner003" << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    sptr<IRemoteObject> ptr = new SystemProcessStatusChange();
    data.WriteRemoteObject(ptr);
    MessageParcel reply;
    int32_t ret = saMgr->UnSubscribeSystemProcessInner(data, reply);
    EXPECT_EQ(ret, ERR_OK);
}

HWTEST_F(SystemAbilityMgrStubUnLoadTest, UnloadSystemAbilityInner003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    int32_t result = saMgr->UnloadSystemAbilityInner(data, reply);
    EXPECT_EQ(result, GET_SA_CONTEXT_FAIL);
}

/**
 * @tc.name: UnloadProcessInner001
 * @tc.desc: Test UnloadProcess
 * @tc.type: FUNC
 * @tc.require: I6H10P
 */
HWTEST_F(SystemAbilityMgrStubUnLoadTest, UnloadProcessInner001, TestSize.Level3)
{
    DTEST_LOG << "UnloadProcessInner001 begin" << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    SamMockPermission::MockProcess("memmgrservice");
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t result = saMgr->UnloadProcessInner(data, reply);
    EXPECT_EQ(result, ERR_OK);
    string deviceId = "memmgrservice";
    data.WriteString(deviceId);
    result = saMgr->UnloadProcessInner(data, reply);
    EXPECT_EQ(result, ERR_OK);
    DTEST_LOG << "UnloadProcessInner001 end" << std::endl;
}

/**
 * @tc.name: GetLruIdleSystemAbilityProcInner001
 * @tc.desc: Test GetLruIdleSystemAbilityProcInner
 * @tc.type: FUNC
 * @tc.require: I6H10P
 */
HWTEST_F(SystemAbilityMgrStubUnLoadTest, GetLruIdleSystemAbilityProcInner001, TestSize.Level3)
{
    DTEST_LOG << "GetLruIdleSystemAbilityProcInner001 begin" << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<DeviceStatusCollectManager> pCollect = new DeviceStatusCollectManager();
    saMgr->collectManager_ = pCollect;
    sptr<DeviceParamCollect> deviceParamCollect = new DeviceParamCollect(pCollect);
    deviceParamCollect->lowMemPrepareList_.push_back(INVALID_SAID);
    deviceParamCollect->lowMemPrepareList_.push_back(SAID);
    saMgr->collectManager_->collectPluginMap_[PARAM] = deviceParamCollect;
    MessageParcel data;
    MessageParcel reply;
    int32_t result = saMgr->GetLruIdleSystemAbilityProcInner(data, reply);
    EXPECT_EQ(result, ERR_OK);
    DTEST_LOG << "GetLruIdleSystemAbilityProcInner001 end" << std::endl;
}
}