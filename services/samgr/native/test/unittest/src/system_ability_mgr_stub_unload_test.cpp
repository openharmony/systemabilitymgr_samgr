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

#include "system_ability_mgr_stub_test.h"
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

using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS;

namespace OHOS {
namespace {
constexpr uint32_t SAID = 1499;
constexpr int32_t INVALID_SAID = -1;
}

HWTEST_F(SystemAbilityMgrStubTest, UnSubsSystemAbilityInner002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(INVALID_SAID);
    int32_t result = saMgr->UnSubsSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

HWTEST_F(SystemAbilityMgrStubTest, UnSubsSystemAbilityInner003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    int32_t result = saMgr->UnSubsSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

HWTEST_F(SystemAbilityMgrStubTest, UnSubsSystemAbilityInner004, TestSize.Level1)
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

HWTEST_F(SystemAbilityMgrStubTest, RemoveSystemAbilityInner002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(INVALID_SAID);
    int32_t result = saMgr->RemoveSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

HWTEST_F(SystemAbilityMgrStubTest, RemoveSystemAbilityInner003, TestSize.Level3)
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

HWTEST_F(SystemAbilityMgrStubTest, RemoveSystemAbility001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    int32_t res = saMgr->RemoveSystemAbility(INVALID_SAID);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

HWTEST_F(SystemAbilityMgrStubTest, RemoveSystemAbility002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    int32_t res = saMgr->RemoveSystemAbility(SAID);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

HWTEST_F(SystemAbilityMgrStubTest, RemoveSystemAbility003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    SAInfo saInfo;
    saInfo.remoteObj = nullptr;
    saMgr->abilityMap_[SAID] = saInfo;
    int32_t res = saMgr->RemoveSystemAbility(SAID);
    EXPECT_EQ(res, ERR_OK);
}

HWTEST_F(SystemAbilityMgrStubTest, RemoveSystemAbility004, TestSize.Level3)
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

HWTEST_F(SystemAbilityMgrStubTest, RemoveSystemAbility005, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    int32_t res = saMgr->RemoveSystemAbility(nullptr);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

HWTEST_F(SystemAbilityMgrStubTest, RemoveSystemAbility006, TestSize.Level3)
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

HWTEST_F(SystemAbilityMgrStubTest, RemoveSystemAbility007, TestSize.Level3)
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

HWTEST_F(SystemAbilityMgrStubTest, UnSubscribeSystemAbility001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    sptr<SaStatusChangeMock> listener(new SaStatusChangeMock());
    int32_t res = saMgr->UnSubscribeSystemAbility(INVALID_SAID, listener);
    u16string name = u"device_saname";
    saMgr->NotifyRemoteSaDied(name);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

HWTEST_F(SystemAbilityMgrStubTest, UnSubscribeSystemAbility002, TestSize.Level1)
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

HWTEST_F(SystemAbilityMgrStubTest, UnSubscribeSystemAbility003, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    sptr<SaStatusChangeMock> listener(nullptr);
    int32_t res = saMgr->UnSubscribeSystemAbility(INVALID_SAID, listener);
    string deviceId = "device";
    saMgr->NotifyRemoteDeviceOffline(deviceId);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

HWTEST_F(SystemAbilityMgrStubTest, UnSubscribeSystemAbility004, TestSize.Level1)
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

HWTEST_F(SystemAbilityMgrStubTest, UnSubscribeSystemAbility005, TestSize.Level3)
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

HWTEST_F(SystemAbilityMgrStubTest, RemoveSystemProcess001, TestSize.Level1)
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
HWTEST_F(SystemAbilityMgrStubTest, RemoveSystemProcess002, TestSize.Level3)
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
HWTEST_F(SystemAbilityMgrStubTest, RemoveSystemProcess003, TestSize.Level3)
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
HWTEST_F(SystemAbilityMgrStubTest, OnRemoveSystemAbilityInner001, TestSize.Level3)
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
HWTEST_F(SystemAbilityMgrStubTest, OnRemoveSystemAbilityInner002, TestSize.Level3)
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
HWTEST_F(SystemAbilityMgrStubTest, UnloadSystemAbilityInner001, TestSize.Level3)
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
HWTEST_F(SystemAbilityMgrStubTest, UnloadSystemAbilityInner002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    int32_t result = saMgr->UnloadSystemAbilityInner(data, reply);
    EXPECT_EQ(result, INVALID_CALL_PROC);
}

/**
 * @tc.name: UnloadSystemAbility001
 * @tc.desc: call UnloadSystemAbility with invalid said
 * @tc.type: FUNC
 * @tc.require: I6AJ3S
 */
HWTEST_F(SystemAbilityMgrStubTest, UnloadSystemAbility001, TestSize.Level3)
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
HWTEST_F(SystemAbilityMgrStubTest, UnloadSystemAbility002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    int32_t result = saMgr->UnloadSystemAbility(SAID);
    EXPECT_EQ(result, INVALID_CALL_PROC);
}

/**
 * @tc.name: UnloadSystemAbility003
 * @tc.desc: call UnloadSystemAbility not from native process
 * @tc.type: FUNC
 * @tc.require: I6AJ3S
 */
HWTEST_F(SystemAbilityMgrStubTest, UnloadSystemAbility003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    CommonSaProfile saProfile;
    saMgr->saProfileMap_[SAID] = saProfile;
    int32_t result = saMgr->UnloadSystemAbility(SAID);
    EXPECT_EQ(result, INVALID_CALL_PROC);
}

/**
 * @tc.name: UnloadSystemAbility004
 * @tc.desc: call UnloadSystemAbility from invalid process
 * @tc.type: FUNC
 * @tc.require: I6AJ3S
 */
HWTEST_F(SystemAbilityMgrStubTest, UnloadSystemAbility004, TestSize.Level3)
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

/**
 * @tc.name: UnloadSystemAbility005
 * @tc.desc: call UnloadSystemAbility
 * @tc.type: FUNC
 * @tc.require: I6AJ3S
 */
HWTEST_F(SystemAbilityMgrStubTest, UnloadSystemAbility005, TestSize.Level3)
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
HWTEST_F(SystemAbilityMgrStubTest, UnSubscribeSystemProcessInner002, TestSize.Level3)
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

HWTEST_F(SystemAbilityMgrStubTest, UnSubscribeSystemProcessInner003, TestSize.Level3)
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

HWTEST_F(SystemAbilityMgrStubTest, UnloadSystemAbilityInner003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    int32_t result = saMgr->UnloadSystemAbilityInner(data, reply);
    EXPECT_EQ(result, GET_SA_CONTEXT_FAIL);
}
}