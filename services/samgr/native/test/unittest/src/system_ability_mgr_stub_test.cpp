/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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
#ifdef SUPPORT_ACCESS_TOKEN
#include "token_setproc.h"
#endif

#define private public
#define protected public
#include "sa_status_change_mock.h"
#include "system_ability_manager.h"
#include "system_ability_manager_util.h"

using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS;

namespace OHOS {
namespace {

const std::u16string SAMANAGER_INTERFACE_TOKEN = u"ohos.samgr.accessToken";
const string DEFAULT_LOAD_NAME = "loadevent";
constexpr uint32_t SAID = 1499;
constexpr int32_t INVALID_SAID = -1;
constexpr uint32_t INVALID_CODE = UINT32_MAX;
#ifdef SUPPORT_ACCESS_TOKEN
void RestoreDefaultToken()
{
    SetSelfTokenID(0);
}
#endif
#ifdef SUPPORT_MULTI_INSTANCE
/**
 * @tc.name: MultiInstanceFuncMap001
 * @tc.desc: test all multi-instance transaction handlers are registered
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, MultiInstanceFuncMap001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_NE(saMgr, nullptr);
    const uint32_t codes[] = {
        static_cast<uint32_t>(SamgrInterfaceCode::GET_SYSTEM_ABILITY_WITH_USERID_TRANSACTION),
        static_cast<uint32_t>(SamgrInterfaceCode::CHECK_SYSTEM_ABILITY_WITH_USERID_TRANSACTION),
        static_cast<uint32_t>(SamgrInterfaceCode::CHECK_SYSTEM_ABILITY_BY_USERID_TRANSACTION),
        static_cast<uint32_t>(SamgrInterfaceCode::GET_SYSTEM_PROCESS_INFO_WITH_USERID_TRANSACTION),
        static_cast<uint32_t>(SamgrInterfaceCode::GET_LOCAL_ABILITY_MANAGER_PROXY_WITH_USERID_TRANSACTION),
        static_cast<uint32_t>(SamgrInterfaceCode::LOAD_SYSTEM_ABILITY_WITH_USERID_TRANSACTION),
        static_cast<uint32_t>(SamgrInterfaceCode::SUBSCRIBE_SYSTEM_ABILITY_WITH_USERID_TRANSACTION),
        static_cast<uint32_t>(SamgrInterfaceCode::UNSUBSCRIBE_SYSTEM_ABILITY_WITH_USERID_TRANSACTION),
        static_cast<uint32_t>(SamgrInterfaceCode::SUBSCRIBE_SYSTEM_PROCESS_WITH_USERID_TRANSACTION),
        static_cast<uint32_t>(SamgrInterfaceCode::UNSUBSCRIBE_SYSTEM_PROCESS_WITH_USERID_TRANSACTION),
    };
    for (const auto code : codes) {
        EXPECT_NE(saMgr->memberFuncMap_.find(code), saMgr->memberFuncMap_.end());
    }
}

/**
 * @tc.name: GetSystemAbilityWithUserIdInner001
 * @tc.desc: Test multi-user get-system-ability parcel validation
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, GetSystemAbilityWithUserIdInner001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(saMgr->GetSystemAbilityWithUserIdInner(data, reply), ERR_NULL_OBJECT);

    EXPECT_TRUE(data.WriteInt32(INVALID_SAID));
    EXPECT_EQ(saMgr->GetSystemAbilityWithUserIdInner(data, reply), ERR_NULL_OBJECT);
}

/**
 * @tc.name: CheckSystemAbilityWithUserIdInner001
 * @tc.desc: Test multi-user check-system-ability parcel validation
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, CheckSystemAbilityWithUserIdInner001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(saMgr->CheckSystemAbilityWithUserIdInner(data, reply), ERR_NULL_OBJECT);

    EXPECT_TRUE(data.WriteInt32(INVALID_SAID));
    EXPECT_EQ(saMgr->CheckSystemAbilityWithUserIdInner(data, reply), ERR_NULL_OBJECT);
}

/**
 * @tc.name: CheckSystemAbilityByUserIdInner001
 * @tc.desc: Test multi-user check-system-ability-by-user parcel validation
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, CheckSystemAbilityByUserIdInner001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(saMgr->CheckSystemAbilityByUserIdInner(data, reply), ERR_NULL_OBJECT);

    EXPECT_TRUE(data.WriteInt32(INVALID_SAID));
    EXPECT_EQ(saMgr->CheckSystemAbilityByUserIdInner(data, reply), ERR_NULL_OBJECT);
}

/**
 * @tc.name: MultiUserParcelRead001
 * @tc.desc: Test multi-user query handlers reject truncated parcels after a valid SA ID
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, MultiUserParcelRead001, TestSize.Level3)
{
    SamMockPermission::MockPermission();
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel reply;
    MessageParcel getData;
    EXPECT_TRUE(getData.WriteInt32(SAID));
    EXPECT_EQ(saMgr->GetSystemAbilityWithUserIdInner(getData, reply), ERR_NULL_OBJECT);

    MessageParcel checkData;
    EXPECT_TRUE(checkData.WriteInt32(SAID));
    EXPECT_EQ(saMgr->CheckSystemAbilityWithUserIdInner(checkData, reply), ERR_NULL_OBJECT);

    MessageParcel existData;
    EXPECT_TRUE(existData.WriteInt32(SAID));
    EXPECT_EQ(saMgr->CheckSystemAbilityByUserIdInner(existData, reply), ERR_FLATTEN_OBJECT);
#ifdef SUPPORT_ACCESS_TOKEN
    RestoreDefaultToken();
#endif
}

/**
 * @tc.name: MultiUserSubscriptionParcel001
 * @tc.desc: Test multi-user SA subscription handlers reject a missing listener
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, MultiUserSubscriptionParcel001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel reply;
    MessageParcel subscribeData;
    EXPECT_TRUE(subscribeData.WriteInt32(SAID));
    EXPECT_EQ(saMgr->SubsSystemAbilityWithUserIdInner(subscribeData, reply), ERR_NULL_OBJECT);

    MessageParcel unsubscribeData;
    EXPECT_TRUE(unsubscribeData.WriteInt32(SAID));
    EXPECT_EQ(saMgr->UnSubsSystemAbilityWithUserIdInner(unsubscribeData, reply), ERR_NULL_OBJECT);
}

/**
 * @tc.name: LoadSystemAbilityWithUserIdInner001
 * @tc.desc: Test multi-user load-system-ability parcel validation
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, LoadSystemAbilityWithUserIdInner001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(saMgr->LoadSystemAbilityWithUserIdInner(data, reply), ERR_INVALID_VALUE);

    EXPECT_TRUE(data.WriteInt32(SAID));
    EXPECT_EQ(saMgr->LoadSystemAbilityWithUserIdInner(data, reply), ERR_INVALID_VALUE);
}

#ifdef SUPPORT_MULTI_INSTANCE
/**
 * @tc.name: LoadSystemAbilityWithUserIdInner002
 * @tc.desc: Test load-system-ability rejects an incompatible callback object and missing user ID.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, LoadSystemAbilityWithUserIdInner002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel reply;

    MessageParcel invalidCallback;
    ASSERT_TRUE(invalidCallback.WriteInt32(SAID));
    ASSERT_TRUE(invalidCallback.WriteRemoteObject(new TestTransactionService()));
    EXPECT_EQ(saMgr->LoadSystemAbilityWithUserIdInner(invalidCallback, reply), ERR_INVALID_VALUE);
}

/**
 * @tc.name: SubsSystemAbilityWithUserIdInner001
 * @tc.desc: Test multi-user subscribe-system-ability parcel validation
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, SubsSystemAbilityWithUserIdInner001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(saMgr->SubsSystemAbilityWithUserIdInner(data, reply), ERR_NULL_OBJECT);

    EXPECT_TRUE(data.WriteInt32(INVALID_SAID));
    EXPECT_EQ(saMgr->SubsSystemAbilityWithUserIdInner(data, reply), ERR_NULL_OBJECT);
}

/**
 * @tc.name: SubsSystemAbilityWithUserIdInner002
 * @tc.desc: Test subscribe-system-ability rejects an incompatible listener and missing user ID.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, SubsSystemAbilityWithUserIdInner002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel reply;

    MessageParcel invalidListener;
    ASSERT_TRUE(invalidListener.WriteInt32(SAID));
    ASSERT_TRUE(invalidListener.WriteRemoteObject(new TestTransactionService()));
    EXPECT_EQ(saMgr->SubsSystemAbilityWithUserIdInner(invalidListener, reply), ERR_NULL_OBJECT);
}

/**
 * @tc.name: MultiUserValidObjectTruncatedParcel001
 * @tc.desc: Test multi-user handlers reject parcels missing user ID after a valid remote object.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, MultiUserValidObjectTruncatedParcel001, TestSize.Level3)
{
    SamMockPermission::MockPermission();
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<SystemAbilityLoadCallbackMock> loadCallback = new SystemAbilityLoadCallbackMock();
    sptr<SaStatusChangeMock> abilityListener = new SaStatusChangeMock();
    sptr<SystemProcessStatusChange> processListener = new SystemProcessStatusChange();
    MessageParcel reply;

    MessageParcel loadData;
    ASSERT_TRUE(loadData.WriteInt32(SAID));
    ASSERT_TRUE(loadData.WriteRemoteObject(loadCallback->AsObject()));
    EXPECT_EQ(saMgr->LoadSystemAbilityWithUserIdInner(loadData, reply), ERR_INVALID_VALUE);

    MessageParcel subscribeAbilityData;
    ASSERT_TRUE(subscribeAbilityData.WriteInt32(SAID));
    ASSERT_TRUE(subscribeAbilityData.WriteRemoteObject(abilityListener->AsObject()));
    EXPECT_EQ(saMgr->SubsSystemAbilityWithUserIdInner(subscribeAbilityData, reply), ERR_NULL_OBJECT);

    MessageParcel unsubscribeAbilityData;
    ASSERT_TRUE(unsubscribeAbilityData.WriteInt32(SAID));
    ASSERT_TRUE(unsubscribeAbilityData.WriteRemoteObject(abilityListener->AsObject()));
    EXPECT_EQ(saMgr->UnSubsSystemAbilityWithUserIdInner(unsubscribeAbilityData, reply), ERR_NULL_OBJECT);

    MessageParcel subscribeProcessData;
    ASSERT_TRUE(subscribeProcessData.WriteRemoteObject(processListener->AsObject()));
    EXPECT_EQ(saMgr->SubscribeSystemProcessWithUserIdInner(subscribeProcessData, reply), ERR_NULL_OBJECT);

    MessageParcel unsubscribeProcessData;
    ASSERT_TRUE(unsubscribeProcessData.WriteRemoteObject(processListener->AsObject()));
    EXPECT_EQ(saMgr->UnSubscribeSystemProcessWithUserIdInner(unsubscribeProcessData, reply), ERR_NULL_OBJECT);

    MessageParcel checkExistData;
    ASSERT_TRUE(checkExistData.WriteInt32(SAID));
    ASSERT_TRUE(checkExistData.WriteBool(false));
    EXPECT_EQ(saMgr->CheckSystemAbilityByUserIdInner(checkExistData, reply), ERR_NULL_OBJECT);
#ifdef SUPPORT_ACCESS_TOKEN
    RestoreDefaultToken();
#endif
}

/**
 * @tc.name: MultiUserInvalidTargetUserQuery001
 * @tc.desc: Test complete multi-user query parcels propagate invalid target-user results.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, MultiUserInvalidTargetUserQuery001, TestSize.Level3)
{
    SamMockPermission::MockPermission();
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel reply;

    MessageParcel getData;
    ASSERT_TRUE(getData.WriteInt32(SAID));
    ASSERT_TRUE(getData.WriteInt32(SAMGR_INVALID_USER_ID));
    EXPECT_EQ(saMgr->GetSystemAbilityWithUserIdInner(getData, reply), ERR_NULL_OBJECT);

    MessageParcel checkData;
    ASSERT_TRUE(checkData.WriteInt32(SAID));
    ASSERT_TRUE(checkData.WriteInt32(SAMGR_INVALID_USER_ID));
    EXPECT_EQ(saMgr->CheckSystemAbilityWithUserIdInner(checkData, reply), ERR_NULL_OBJECT);

    MessageParcel checkExistData;
    ASSERT_TRUE(checkExistData.WriteInt32(SAID));
    ASSERT_TRUE(checkExistData.WriteBool(false));
    ASSERT_TRUE(checkExistData.WriteInt32(SAMGR_INVALID_USER_ID));
    EXPECT_EQ(saMgr->CheckSystemAbilityByUserIdInner(checkExistData, reply), ERR_NULL_OBJECT);

    MessageParcel processInfoData;
    ASSERT_TRUE(processInfoData.WriteInt32(SAID));
    ASSERT_TRUE(processInfoData.WriteInt32(SAMGR_INVALID_USER_ID));
    EXPECT_EQ(saMgr->GetSystemProcessInfoWithUserIdInner(processInfoData, reply), ERR_OK);

    MessageParcel localProxyData;
    ASSERT_TRUE(localProxyData.WriteInt32(SAID));
    ASSERT_TRUE(localProxyData.WriteInt32(SAMGR_INVALID_USER_ID));
    EXPECT_EQ(saMgr->GetLocalAbilityManagerProxyWithUserIdInner(localProxyData, reply), ERR_NULL_OBJECT);
#ifdef SUPPORT_ACCESS_TOKEN
    RestoreDefaultToken();
#endif
}

/**
 * @tc.name: MultiUserInvalidTargetUserOperation001
 * @tc.desc: Test complete multi-user operation parcels propagate invalid target-user results.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, MultiUserInvalidTargetUserOperation001, TestSize.Level3)
{
    SamMockPermission::MockPermission();
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<SystemAbilityLoadCallbackMock> loadCallback = new SystemAbilityLoadCallbackMock();
    sptr<SaStatusChangeMock> abilityListener = new SaStatusChangeMock();
    sptr<SystemProcessStatusChange> processListener = new SystemProcessStatusChange();
    MessageParcel reply;
    MessageParcel loadData;
    ASSERT_TRUE(loadData.WriteInt32(SAID));
    ASSERT_TRUE(loadData.WriteRemoteObject(loadCallback->AsObject()));
    ASSERT_TRUE(loadData.WriteInt32(SAMGR_INVALID_USER_ID));
    EXPECT_EQ(saMgr->LoadSystemAbilityWithUserIdInner(loadData, reply), INVALID_CALLING_USER_ID);

    MessageParcel subscribeAbilityData;
    ASSERT_TRUE(subscribeAbilityData.WriteInt32(SAID));
    ASSERT_TRUE(subscribeAbilityData.WriteRemoteObject(abilityListener->AsObject()));
    ASSERT_TRUE(subscribeAbilityData.WriteInt32(SAMGR_INVALID_USER_ID));
    EXPECT_EQ(saMgr->SubsSystemAbilityWithUserIdInner(subscribeAbilityData, reply), INVALID_CALLING_USER_ID);

    MessageParcel unsubscribeAbilityData;
    ASSERT_TRUE(unsubscribeAbilityData.WriteInt32(SAID));
    ASSERT_TRUE(unsubscribeAbilityData.WriteRemoteObject(abilityListener->AsObject()));
    ASSERT_TRUE(unsubscribeAbilityData.WriteInt32(SAMGR_INVALID_USER_ID));
    EXPECT_EQ(saMgr->UnSubsSystemAbilityWithUserIdInner(unsubscribeAbilityData, reply), INVALID_CALLING_USER_ID);

    MessageParcel subscribeProcessData;
    ASSERT_TRUE(subscribeProcessData.WriteRemoteObject(processListener->AsObject()));
    ASSERT_TRUE(subscribeProcessData.WriteInt32(SAMGR_INVALID_USER_ID));
    EXPECT_EQ(saMgr->SubscribeSystemProcessWithUserIdInner(subscribeProcessData, reply), INVALID_CALLING_USER_ID);

    MessageParcel unsubscribeProcessData;
    ASSERT_TRUE(unsubscribeProcessData.WriteRemoteObject(processListener->AsObject()));
    ASSERT_TRUE(unsubscribeProcessData.WriteInt32(SAMGR_INVALID_USER_ID));
    EXPECT_EQ(saMgr->UnSubscribeSystemProcessWithUserIdInner(unsubscribeProcessData, reply), INVALID_CALLING_USER_ID);
#ifdef SUPPORT_ACCESS_TOKEN
    RestoreDefaultToken();
#endif
}

/**
 * @tc.name: MultiUserQuerySuccess001
 * @tc.desc: Test multi-user query handlers serialize an existing ability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, MultiUserQuerySuccess001, TestSize.Level3)
{
    constexpr int32_t userId = 120;
    SamMockPermission::MockPermission();
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    ASSERT_NE(saMgr, nullptr);
    saMgr->userLifecycleManager_.SetSaProfiles(&saMgr->allSaProfiles_);
    ASSERT_EQ(saMgr->OnUserStateChanged(userId, USER_STATE_ACTIVATING), ERR_OK);
    auto manager = saMgr->GetMultiUserManager(userId);
    ASSERT_NE(manager, nullptr);
    sptr<IRemoteObject> ability = new TestTransactionService();
    manager->abilityMap_[SAID] = {ability, false};

    MessageParcel getData;
    MessageParcel getReply;
    ASSERT_TRUE(getData.WriteInt32(SAID));
    ASSERT_TRUE(getData.WriteInt32(userId));
    EXPECT_EQ(saMgr->GetSystemAbilityWithUserIdInner(getData, getReply), ERR_NONE);
    EXPECT_EQ(getReply.ReadRemoteObject(), ability);

    MessageParcel checkData;
    MessageParcel checkReply;
    ASSERT_TRUE(checkData.WriteInt32(SAID));
    ASSERT_TRUE(checkData.WriteInt32(userId));
    EXPECT_EQ(saMgr->CheckSystemAbilityWithUserIdInner(checkData, checkReply), ERR_NONE);
    EXPECT_EQ(checkReply.ReadRemoteObject(), ability);

    MessageParcel existData;
    MessageParcel existReply;
    ASSERT_TRUE(existData.WriteInt32(SAID));
    ASSERT_TRUE(existData.WriteBool(false));
    ASSERT_TRUE(existData.WriteInt32(userId));
    EXPECT_EQ(saMgr->CheckSystemAbilityByUserIdInner(existData, existReply), ERR_NONE);
    EXPECT_EQ(existReply.ReadRemoteObject(), ability);
    EXPECT_TRUE(existReply.ReadBool());
#ifdef SUPPORT_ACCESS_TOKEN
    RestoreDefaultToken();
#endif
    EXPECT_EQ(saMgr->OnUserStateChanged(userId, USER_STATE_STOPPING), ERR_OK);
}

/**
 * @tc.name: MultiUserProcessInfoSuccess001
 * @tc.desc: Test the multi-user process-info handler serializes process fields.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, MultiUserProcessInfoSuccess001, TestSize.Level3)
{
    constexpr int32_t userId = 121;
    constexpr int32_t processPid = 321;
    constexpr int32_t processUid = 654;
    const std::u16string processName = u"multi_user_process";
    SamMockPermission::MockPermission();
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    ASSERT_NE(saMgr, nullptr);
    saMgr->userLifecycleManager_.SetSaProfiles(&saMgr->allSaProfiles_);
    ASSERT_EQ(saMgr->OnUserStateChanged(userId, USER_STATE_ACTIVATING), ERR_OK);
    auto manager = saMgr->GetMultiUserManager(userId);
    ASSERT_NE(manager, nullptr);
    auto abilityContext = std::make_shared<SystemAbilityContext>();
    auto processContext = std::make_shared<SystemProcessContext>();
    processContext->processName = processName;
    processContext->pid = processPid;
    processContext->uid = processUid;
    abilityContext->ownProcessContext = processContext;
    manager->abilityStateScheduler_->abilityContextMap_[SAID] = abilityContext;

    MessageParcel data;
    MessageParcel reply;
    ASSERT_TRUE(data.WriteInt32(SAID));
    ASSERT_TRUE(data.WriteInt32(userId));
    EXPECT_EQ(saMgr->GetSystemProcessInfoWithUserIdInner(data, reply), ERR_OK);
    EXPECT_EQ(reply.ReadInt32(), ERR_OK);
    EXPECT_EQ(reply.ReadString(), Str16ToStr8(processName));
    EXPECT_EQ(reply.ReadInt32(), processPid);
    EXPECT_EQ(reply.ReadInt32(), processUid);
#ifdef SUPPORT_ACCESS_TOKEN
    RestoreDefaultToken();
#endif
    EXPECT_EQ(saMgr->OnUserStateChanged(userId, USER_STATE_STOPPING), ERR_OK);
}

/**
 * @tc.name: MultiUserLocalProxySuccess001
 * @tc.desc: Test the multi-user local-manager handler serializes the process object.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, MultiUserLocalProxySuccess001, TestSize.Level3)
{
    constexpr int32_t userId = 122;
    const std::u16string processName = u"multi_user_local_process";
    SamMockPermission::MockPermission();
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    ASSERT_NE(saMgr, nullptr);
    saMgr->userLifecycleManager_.SetSaProfiles(&saMgr->allSaProfiles_);
    ASSERT_EQ(saMgr->OnUserStateChanged(userId, USER_STATE_ACTIVATING), ERR_OK);
    auto manager = saMgr->GetMultiUserManager(userId);
    ASSERT_NE(manager, nullptr);
    CommonSaProfile profile;
    profile.process = processName;
    manager->saProfileMap_[SAID] = profile;
    sptr<IRemoteObject> process = new TestTransactionService();
    manager->systemProcessMap_[processName] = process;

    MessageParcel data;
    MessageParcel reply;
    ASSERT_TRUE(data.WriteInt32(SAID));
    ASSERT_TRUE(data.WriteInt32(userId));
    EXPECT_EQ(saMgr->GetLocalAbilityManagerProxyWithUserIdInner(data, reply), ERR_NONE);
    EXPECT_EQ(reply.ReadRemoteObject(), process);
#ifdef SUPPORT_ACCESS_TOKEN
    RestoreDefaultToken();
#endif
    EXPECT_EQ(saMgr->OnUserStateChanged(userId, USER_STATE_STOPPING), ERR_OK);
}

/**
 * @tc.name: MultiUserSubscriptionSuccess001
 * @tc.desc: Test multi-user ability and process subscriptions complete a round trip.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, MultiUserSubscriptionSuccess001, TestSize.Level3)
{
    constexpr int32_t userId = 123;
    SamMockPermission::MockPermission();
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    ASSERT_NE(saMgr, nullptr);
    saMgr->userLifecycleManager_.SetSaProfiles(&saMgr->allSaProfiles_);
    ASSERT_EQ(saMgr->OnUserStateChanged(userId, USER_STATE_ACTIVATING), ERR_OK);
    sptr<SaStatusChangeMock> abilityListener = new SaStatusChangeMock();
    sptr<SystemProcessStatusChange> processListener = new SystemProcessStatusChange();

    MessageParcel subscribeAbility;
    MessageParcel subscribeAbilityReply;
    ASSERT_TRUE(subscribeAbility.WriteInt32(SAID));
    ASSERT_TRUE(subscribeAbility.WriteRemoteObject(abilityListener->AsObject()));
    ASSERT_TRUE(subscribeAbility.WriteInt32(userId));
    EXPECT_EQ(saMgr->SubsSystemAbilityWithUserIdInner(subscribeAbility, subscribeAbilityReply), ERR_OK);

    MessageParcel unsubscribeAbility;
    MessageParcel unsubscribeAbilityReply;
    ASSERT_TRUE(unsubscribeAbility.WriteInt32(SAID));
    ASSERT_TRUE(unsubscribeAbility.WriteRemoteObject(abilityListener->AsObject()));
    ASSERT_TRUE(unsubscribeAbility.WriteInt32(userId));
    EXPECT_EQ(saMgr->UnSubsSystemAbilityWithUserIdInner(unsubscribeAbility, unsubscribeAbilityReply), ERR_OK);

    MessageParcel subscribeProcess;
    MessageParcel subscribeProcessReply;
    ASSERT_TRUE(subscribeProcess.WriteRemoteObject(processListener->AsObject()));
    ASSERT_TRUE(subscribeProcess.WriteInt32(userId));
    EXPECT_EQ(saMgr->SubscribeSystemProcessWithUserIdInner(subscribeProcess, subscribeProcessReply), ERR_OK);

    MessageParcel unsubscribeProcess;
    MessageParcel unsubscribeProcessReply;
    ASSERT_TRUE(unsubscribeProcess.WriteRemoteObject(processListener->AsObject()));
    ASSERT_TRUE(unsubscribeProcess.WriteInt32(userId));
    EXPECT_EQ(saMgr->UnSubscribeSystemProcessWithUserIdInner(unsubscribeProcess, unsubscribeProcessReply), ERR_OK);
#ifdef SUPPORT_ACCESS_TOKEN
    RestoreDefaultToken();
#endif
    EXPECT_EQ(saMgr->OnUserStateChanged(userId, USER_STATE_STOPPING), ERR_OK);
}

/**
 * @tc.name: MultiUserInvalidListenerType001
 * @tc.desc: Test multi-user handlers reject incompatible listener object types.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, MultiUserInvalidListenerType001, TestSize.Level3)
{
    SamMockPermission::MockPermission();
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    ASSERT_NE(saMgr, nullptr);
    sptr<IRemoteObject> invalidListener = new TestTransactionService();
    MessageParcel reply;

    MessageParcel unsubscribeAbility;
    ASSERT_TRUE(unsubscribeAbility.WriteInt32(SAID));
    ASSERT_TRUE(unsubscribeAbility.WriteRemoteObject(invalidListener));
    EXPECT_EQ(saMgr->UnSubsSystemAbilityWithUserIdInner(unsubscribeAbility, reply), ERR_NULL_OBJECT);

    MessageParcel subscribeProcess;
    ASSERT_TRUE(subscribeProcess.WriteRemoteObject(invalidListener));
    EXPECT_EQ(saMgr->SubscribeSystemProcessWithUserIdInner(subscribeProcess, reply), ERR_NULL_OBJECT);

    MessageParcel unsubscribeProcess;
    ASSERT_TRUE(unsubscribeProcess.WriteRemoteObject(invalidListener));
    int32_t result = saMgr->UnSubscribeSystemProcessWithUserIdInner(unsubscribeProcess, reply);
#ifdef SUPPORT_ACCESS_TOKEN
    RestoreDefaultToken();
#endif
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

#ifdef SUPPORT_ACCESS_TOKEN
/**
 * @tc.name: MultiUserNativeCallerGuard001
 * @tc.desc: Test process-related multi-user handlers reject a HAP caller.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, MultiUserNativeCallerGuard001, TestSize.Level3)
{
    constexpr uint32_t hapTokenId = (1U << 29U) | 1U;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    ASSERT_NE(saMgr, nullptr);
    SetSelfTokenID(hapTokenId);
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(saMgr->GetSystemProcessInfoWithUserIdInner(data, reply), ERR_PERMISSION_DENIED);
    EXPECT_EQ(saMgr->SubscribeSystemProcessWithUserIdInner(data, reply), ERR_PERMISSION_DENIED);
    int32_t result = saMgr->UnSubscribeSystemProcessWithUserIdInner(data, reply);
    RestoreDefaultToken();
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
}
#endif

/**
 * @tc.name: MultiUserMissingUserIdQuery001
 * @tc.desc: Test process-info and local-proxy handlers reject a missing user ID.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, MultiUserMissingUserIdQuery001, TestSize.Level3)
{
    SamMockPermission::MockPermission();
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    ASSERT_NE(saMgr, nullptr);
    MessageParcel reply;
    MessageParcel processInfoData;
    ASSERT_TRUE(processInfoData.WriteInt32(SAID));
    EXPECT_EQ(saMgr->GetSystemProcessInfoWithUserIdInner(processInfoData, reply), ERR_NULL_OBJECT);

    MessageParcel localProxyData;
    ASSERT_TRUE(localProxyData.WriteInt32(SAID));
    int32_t result = saMgr->GetLocalAbilityManagerProxyWithUserIdInner(localProxyData, reply);
#ifdef SUPPORT_ACCESS_TOKEN
    RestoreDefaultToken();
#endif
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: MultiUserLoadInvalidSaId001
 * @tc.desc: Test the multi-user load handler rejects an invalid system ability ID.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, MultiUserLoadInvalidSaId001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    ASSERT_NE(saMgr, nullptr);
    MessageParcel data;
    MessageParcel reply;
    ASSERT_TRUE(data.WriteInt32(INVALID_SAID));
    EXPECT_EQ(saMgr->LoadSystemAbilityWithUserIdInner(data, reply), ERR_INVALID_VALUE);
}

/**
 * @tc.name: MultiUserStateMissingUserId001
 * @tc.desc: Test the user-state handler rejects a parcel without a user ID.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, MultiUserStateMissingUserId001, TestSize.Level3)
{
    SamMockPermission::MockProcess("accountmgr");
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    ASSERT_NE(saMgr, nullptr);
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(saMgr->OnUserStateChangedInner(data, reply), ERR_FLATTEN_OBJECT);
}

/**
 * @tc.name: MultiUserLoadSuccess001
 * @tc.desc: Test the multi-user load handler returns success for an already loaded ability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, MultiUserLoadSuccess001, TestSize.Level3)
{
    constexpr int32_t userId = 125;
    const std::u16string processName = u"multi_user_loaded_process";
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    ASSERT_NE(saMgr, nullptr);
    saMgr->userLifecycleManager_.SetSaProfiles(&saMgr->allSaProfiles_);
    ASSERT_EQ(saMgr->OnUserStateChanged(userId, USER_STATE_ACTIVATING), ERR_OK);
    auto manager = saMgr->GetMultiUserManager(userId);
    ASSERT_NE(manager, nullptr);
    CommonSaProfile profile;
    profile.process = processName;
    manager->saProfileMap_[SAID] = profile;
    sptr<IRemoteObject> ability = new TestTransactionService();
    manager->abilityMap_[SAID] = {ability, false};
    auto abilityContext = std::make_shared<SystemAbilityContext>();
    auto processContext = std::make_shared<SystemProcessContext>();
    processContext->processName = processName;
    processContext->state = SystemProcessState::STARTED;
    abilityContext->systemAbilityId = SAID;
    abilityContext->state = SystemAbilityState::LOADED;
    abilityContext->ownProcessContext = processContext;
    manager->abilityStateScheduler_->abilityContextMap_[SAID] = abilityContext;

    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    MessageParcel data;
    MessageParcel reply;
    ASSERT_TRUE(data.WriteInt32(SAID));
    ASSERT_TRUE(data.WriteRemoteObject(callback->AsObject()));
    ASSERT_TRUE(data.WriteInt32(userId));
    EXPECT_EQ(saMgr->LoadSystemAbilityWithUserIdInner(data, reply), ERR_OK);
    EXPECT_EQ(reply.ReadInt32(), ERR_OK);
    EXPECT_EQ(saMgr->OnUserStateChanged(userId, USER_STATE_STOPPING), ERR_OK);
}
#endif

/**
 * @tc.name: UnSubsSystemAbilityWithUserIdInner001
 * @tc.desc: Test multi-user unsubscribe-system-ability parcel validation
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, UnSubsSystemAbilityWithUserIdInner001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(saMgr->UnSubsSystemAbilityWithUserIdInner(data, reply), ERR_NULL_OBJECT);

    EXPECT_TRUE(data.WriteInt32(INVALID_SAID));
    EXPECT_EQ(saMgr->UnSubsSystemAbilityWithUserIdInner(data, reply), ERR_NULL_OBJECT);
}

/**
 * @tc.name: GetSystemProcessInfoWithUserIdInner001
 * @tc.desc: Test multi-user process-info parcel validation
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, GetSystemProcessInfoWithUserIdInner001, TestSize.Level3)
{
    SamMockPermission::MockPermission();
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(saMgr->GetSystemProcessInfoWithUserIdInner(data, reply), ERR_NULL_OBJECT);

    EXPECT_TRUE(data.WriteInt32(INVALID_SAID));
    EXPECT_EQ(saMgr->GetSystemProcessInfoWithUserIdInner(data, reply), ERR_NULL_OBJECT);
#ifdef SUPPORT_ACCESS_TOKEN
    RestoreDefaultToken();
#endif
}

/**
 * @tc.name: GetLocalAbilityManagerProxyWithUserIdInner001
 * @tc.desc: Test multi-user local-manager proxy parcel validation
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, GetLocalAbilityManagerProxyWithUserIdInner001, TestSize.Level3)
{
    SamMockPermission::MockPermission();
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(saMgr->GetLocalAbilityManagerProxyWithUserIdInner(data, reply), ERR_NULL_OBJECT);

    EXPECT_TRUE(data.WriteInt32(INVALID_SAID));
    EXPECT_EQ(saMgr->GetLocalAbilityManagerProxyWithUserIdInner(data, reply), ERR_NULL_OBJECT);
#ifdef SUPPORT_ACCESS_TOKEN
    RestoreDefaultToken();
#endif
}

/**
 * @tc.name: SubscribeSystemProcessWithUserIdInner001
 * @tc.desc: Test multi-user process subscription parcel validation
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, SubscribeSystemProcessWithUserIdInner001, TestSize.Level3)
{
    SamMockPermission::MockPermission();
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(saMgr->SubscribeSystemProcessWithUserIdInner(data, reply), ERR_NULL_OBJECT);
#ifdef SUPPORT_ACCESS_TOKEN
    RestoreDefaultToken();
#endif
}

/**
 * @tc.name: UnSubscribeSystemProcessWithUserIdInner001
 * @tc.desc: Test multi-user process unsubscription parcel validation
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, UnSubscribeSystemProcessWithUserIdInner001, TestSize.Level3)
{
    SamMockPermission::MockPermission();
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(saMgr->UnSubscribeSystemProcessWithUserIdInner(data, reply), ERR_NULL_OBJECT);
#ifdef SUPPORT_ACCESS_TOKEN
    RestoreDefaultToken();
#endif
}

void InitUserLifecycleManager(const sptr<SystemAbilityManager>& saMgr)
{
    saMgr->userLifecycleManager_.SetSaProfiles(&saMgr->allSaProfiles_);
}
#endif
}
#ifdef SUPPORT_PENGLAI_MODE
bool g_permissionRet = false;
void* g_originHandle = SamgrUtil::penglaiFunc_;
bool MockIsLaunchAllowedByUid(const int32_t callingUid, const int32_t systemAbilityId)
{
    return g_permissionRet;
}

void SetPenglaiPerm(bool permission)
{
    SamgrUtil::penglaiFunc_ = (void*)MockIsLaunchAllowedByUid;
    g_permissionRet = permission;
}

void UnSetPenglaiPerm()
{
    SamgrUtil::penglaiFunc_ = g_originHandle;
}
#endif

void SystemProcessStatusChange::OnSystemProcessStarted(SystemProcessInfo& systemProcessInfo)
{
    cout << "OnSystemProcessStarted, processName: ";
}

void SystemProcessStatusChange::OnSystemProcessStopped(SystemProcessInfo& systemProcessInfo)
{
    cout << "OnSystemProcessStopped, processName: ";
}

void SystemAbilityMgrStubTest::SetUpTestCase()
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    saMgr->selfPtr_ = std::shared_ptr<BaseSystemAbilityManager>(
        saMgr.GetRefPtr(), [](BaseSystemAbilityManager*) {});
    saMgr->abilityStateScheduler_ = std::make_shared<SystemAbilityStateScheduler>(
        saMgr->weak_from_this());
    std::list<SaProfile> saProfiles;
    saMgr->abilityStateScheduler_->Init(saProfiles);
    DTEST_LOG << "SetUpTestCase" << std::endl;
}

void SystemAbilityMgrStubTest::TearDownTestCase()
{
    DTEST_LOG << "TearDownTestCase" << std::endl;
}

void SystemAbilityMgrStubTest::SetUp()
{
    DTEST_LOG << "SetUp" << std::endl;
}

void SystemAbilityMgrStubTest::TearDown()
{
    DTEST_LOG << "TearDown" << std::endl;
}

void SystemAbilityMgrStubTest::AddSystemAbilityContext(int32_t systemAbilityId, const std::u16string& processName)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    EXPECT_TRUE(saMgr->abilityStateScheduler_ != nullptr);
    std::unique_lock<samgr::shared_mutex> processWriteLock(saMgr->abilityStateScheduler_->processMapLock_);
    if (saMgr->abilityStateScheduler_->processContextMap_.count(processName) == 0) {
        auto processContext = std::make_shared<SystemProcessContext>();
        processContext->processName = processName;
        processContext->abilityStateCountMap[SystemAbilityState::NOT_LOADED] = 0;
        processContext->abilityStateCountMap[SystemAbilityState::LOADING] = 0;
        processContext->abilityStateCountMap[SystemAbilityState::LOADED] = 0;
        processContext->abilityStateCountMap[SystemAbilityState::UNLOADABLE] = 0;
        processContext->abilityStateCountMap[SystemAbilityState::UNLOADING] = 0;
        saMgr->abilityStateScheduler_->processContextMap_[processName] = processContext;
    }
    saMgr->abilityStateScheduler_->processContextMap_[processName]->saList.push_back(systemAbilityId);
    saMgr->abilityStateScheduler_->processContextMap_[processName]
        ->abilityStateCountMap[SystemAbilityState::NOT_LOADED]++;

    auto abilityContext = std::make_shared<SystemAbilityContext>();
    abilityContext->systemAbilityId = systemAbilityId;
    abilityContext->ownProcessContext = saMgr->abilityStateScheduler_->processContextMap_[processName];
    std::unique_lock<samgr::shared_mutex> abiltyWriteLock(saMgr->abilityStateScheduler_->abiltyMapLock_);
    saMgr->abilityStateScheduler_->abilityContextMap_[systemAbilityId] = abilityContext;
}

HWTEST_F(SystemAbilityMgrStubTest, OnRemoteRequest001, TestSize.Level4)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN);
    MessageParcel reply;
    MessageOption option;
    int32_t result = saMgr->OnRemoteRequest(INVALID_CODE, data, reply, option);
    EXPECT_EQ(result, IPC_STUB_UNKNOW_TRANS_ERR);
}

/**
 * @tc.name: OnRemoteRequestToken001
 * @tc.desc: Test request rejection before dispatch when the interface token is absent or invalid.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, OnRemoteRequestToken001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    ASSERT_NE(saMgr, nullptr);
    MessageParcel reply;
    MessageOption option;
    MessageParcel emptyTokenData;
    EXPECT_EQ(saMgr->OnRemoteRequest(INVALID_CODE, emptyTokenData, reply, option), ERR_PERMISSION_DENIED);

    MessageParcel invalidTokenData;
    ASSERT_TRUE(invalidTokenData.WriteInterfaceToken(u"ohos.samgr.invalid"));
    EXPECT_EQ(saMgr->OnRemoteRequest(INVALID_CODE, invalidTokenData, reply, option), ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: OnRemoteRequestDispatch001
 * @tc.desc: Test valid-token dispatch rejects truncated requests for SA query handlers.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, OnRemoteRequestDispatch001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    ASSERT_NE(saMgr, nullptr);
    MessageParcel reply;
    MessageOption option;
    MessageParcel getData;
    ASSERT_TRUE(getData.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN));
    EXPECT_EQ(saMgr->OnRemoteRequest(
        static_cast<uint32_t>(SamgrInterfaceCode::GET_SYSTEM_ABILITY_TRANSACTION), getData, reply, option),
        ERR_NULL_OBJECT);

    MessageParcel checkData;
    ASSERT_TRUE(checkData.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN));
    EXPECT_EQ(saMgr->OnRemoteRequest(
        static_cast<uint32_t>(SamgrInterfaceCode::CHECK_SYSTEM_ABILITY_TRANSACTION), checkData, reply, option),
        ERR_NULL_OBJECT);
}

#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityMgrStubTest, ListSystemAbilityInner001, TestSize.Level4)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t result = saMgr->ListSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
}
#endif

HWTEST_F(SystemAbilityMgrStubTest, SubsSystemAbilityInner001, TestSize.Level4)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t result = saMgr->SubsSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

HWTEST_F(SystemAbilityMgrStubTest, UnSubsSystemAbilityInner001, TestSize.Level4)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t result = saMgr->UnSubsSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityMgrStubTest, CheckRemtSystemAbilityInner001, TestSize.Level4)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t result = saMgr->CheckRemtSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
}
#endif

#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityMgrStubTest, AddOndemandSystemAbilityInner001, TestSize.Level4)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t result = saMgr->AddOndemandSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
}
#endif

#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityMgrStubTest, RemoveSystemAbilityInner001, TestSize.Level4)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t result = saMgr->RemoveSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
}
#endif

#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityMgrStubTest, GetSystemProcessInfoInner001, TestSize.Level3)
{
    DTEST_LOG << "GetSystemProcessInfoInner001" << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = saMgr->GetSystemProcessInfoInner(data, reply);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}
#endif

#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityMgrStubTest, GetRunningSystemProcessInner001, TestSize.Level3)
{
    DTEST_LOG << "GetRunningSystemProcessInner001" << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = saMgr->GetRunningSystemProcessInner(data, reply);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}
#endif

#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityMgrStubTest, UnSubscribeSystemProcessInner001, TestSize.Level3)
{
    DTEST_LOG << "UnSubscribeSystemProcessInner001" << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = saMgr->UnSubscribeSystemProcessInner(data, reply);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}
#endif

#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityMgrStubTest, GetOnDemandPolicyInner001, TestSize.Level3)
{
    DTEST_LOG << "GetOnDemandPolicyInner001" << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = saMgr->GetOnDemandPolicyInner(data, reply);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}
#endif

/**
 * @tc.name: Test UpdateOnDemandPolicyInner001
 * @tc.desc: UpdateOnDemandPolicyInner001
 * @tc.type: FUNC
 * @tc.require: I6T116
 */
#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityMgrStubTest, UpdateOnDemandPolicyInner001, TestSize.Level3)
{
    DTEST_LOG << "UpdateOnDemandPolicyInner001" << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = saMgr->UpdateOnDemandPolicyInner(data, reply);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}
#endif

/**
 * @tc.name: GetOnDemandReasonExtraDataInner001
 * @tc.desc: test GetOnDemandReasonExtraDataInner with permission is denied
 * @tc.type: FUNC
 * @tc.require: I6XB42
 */
#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityMgrStubTest, GetOnDemandReasonExtraDataInner001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = saMgr->GetOnDemandReasonExtraDataInner(data, reply);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}
#endif

/**
 * @tc.name: GetExtensionSaIdsInner001
 * @tc.desc: test GetExtensionSaIdsInner with permission is denied
 * @tc.type: FUNC
 * @tc.require: I6XB42
 */
#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityMgrStubTest, GetExtensionSaIdsInner001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = saMgr->GetExtensionSaIdsInner(data, reply);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}
#endif

/**
 * @tc.name: GetExtensionRunningSaListInner001
 * @tc.desc: test GetExtensionRunningSaListInner with permission is denied
 * @tc.type: FUNC
 * @tc.require: I6XB42
 */
#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityMgrStubTest, GetExtensionRunningSaListInner001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = saMgr->GetExtensionRunningSaListInner(data, reply);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}
#endif

/**
 * @tc.name: GetCommonEventExtraDataIdlistInner001
 * @tc.desc: test GetCommonEventExtraDataIdlistInner with permission is denied
 * @tc.type: FUNC
 * @tc.require: I6XB42
 */
#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityMgrStubTest, GetCommonEventExtraDataIdlistInner001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = saMgr->GetCommonEventExtraDataIdlistInner(data, reply);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}
#endif

/**
 * @tc.name: OnStartSystemAbilityFailInner004
 * @tc.desc: test OnStartSystemAbilityFailInner with permission is denied
 * @tc.type: FUNC
 */
#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityMgrStubTest, OnStartSystemAbilityFailInner004, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = saMgr->OnStartSystemAbilityFailInner(data, reply);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}
#endif

/**
 * @tc.name: ListSystemAbilityInner002
 * @tc.desc: test ListSystemAbilityInner, read dumpflag failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, ListSystemAbilityInner002, TestSize.Level3)
{
    SamMockPermission::MockPermission();
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t result = saMgr->ListSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_FLATTEN_OBJECT);
}

/**
 * @tc.name: ListSystemAbilityInner003
 * @tc.desc: test ListSystemAbilityInner, list empty!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, ListSystemAbilityInner003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t invalidDump = 1;
    data.WriteInt32(invalidDump);
    int32_t result = saMgr->ListSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: ListSystemAbilityInner004
 * @tc.desc: test ListSystemAbilityInner, list no empty!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, ListSystemAbilityInner004, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    SAInfo saInfo;
    saMgr->abilityMap_[SAID] = saInfo;
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t invalidDump = 1;
    data.WriteInt32(invalidDump);
    int32_t result = saMgr->ListSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: CheckRemtSystemAbilityInner002
 * @tc.desc: test CheckRemtSystemAbilityInner, read systemAbilityId failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, CheckRemtSystemAbilityInner002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(INVALID_SAID);
    int32_t result = saMgr->CheckRemtSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: CheckRemtSystemAbilityInner003
 * @tc.desc: test CheckRemtSystemAbilityInner, read deviceId failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, CheckRemtSystemAbilityInner003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    int32_t result = saMgr->CheckRemtSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_FLATTEN_OBJECT);
}

/**
 * @tc.name: CheckRemtSystemAbilityInner004
 * @tc.desc: test CheckRemtSystemAbilityInner!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, CheckRemtSystemAbilityInner004, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    std::string deviceId = "test";
    data.WriteString(deviceId);
    int32_t result = saMgr->CheckRemtSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: CheckRemtSystemAbilityInner005
 * @tc.desc: test CheckRemtSystemAbilityInner, penglai mode permission check failed!
 * @tc.type: FUNC
 */
#ifdef SUPPORT_PENGLAI_MODE
HWTEST_F(SystemAbilityMgrStubTest, CheckRemtSystemAbilityInner005, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    saMgr->SetPengLai(true);
    // set permission denied
    SetPenglaiPerm(false);
    int32_t result = saMgr->CheckRemtSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
    UnSetPenglaiPerm();
    saMgr->SetPengLai(false);
}
#endif

/**
 * @tc.name: CheckRemtSystemAbilityInner006
 * @tc.desc: test CheckSystemAbilityImmeInner, penglai mode permission check success!
 * @tc.type: FUNC
 */
#ifdef SUPPORT_PENGLAI_MODE
HWTEST_F(SystemAbilityMgrStubTest, CheckRemtSystemAbilityInner006, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    saMgr->SetPengLai(true);
    // set permission true
    SetPenglaiPerm(true);
    int32_t result = saMgr->CheckRemtSystemAbilityInner(data, reply);
    EXPECT_NE(result, ERR_PERMISSION_DENIED);
    UnSetPenglaiPerm();
    saMgr->SetPengLai(false);
}
#endif

/**
 * @tc.name: AddOndemandSystemAbilityInner002
 * @tc.desc: test AddOndemandSystemAbilityInner, read systemAbilityId failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddOndemandSystemAbilityInner002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(INVALID_SAID);
    int32_t result = saMgr->AddOndemandSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: AddOndemandSystemAbilityInner003
 * @tc.desc: test AddOndemandSystemAbilityInner, read localName failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddOndemandSystemAbilityInner003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    int32_t result = saMgr->AddOndemandSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: AddOndemandSystemAbilityInner004
 * @tc.desc: test AddOndemandSystemAbilityInner!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddOndemandSystemAbilityInner004, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    u16string localManagerName = u"test";
    data.WriteString16(localManagerName);
    int32_t result = saMgr->AddOndemandSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: UnmarshalingSaExtraProp001
 * @tc.desc: test UnmarshalingSaExtraProp, read isDistributed failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, UnmarshalingSaExtraProp001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    SystemAbilityManager::SAExtraProp extraProp;
    int32_t result = saMgr->UnmarshalingSaExtraProp(data, extraProp);
    EXPECT_EQ(result, ERR_FLATTEN_OBJECT);
}

/**
 * @tc.name: UnmarshalingSaExtraProp002
 * @tc.desc: test UnmarshalingSaExtraProp,  UnmarshalingSaExtraProp dumpFlags failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, UnmarshalingSaExtraProp002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    SystemAbilityManager::SAExtraProp extraProp;
    data.WriteBool(false);
    int32_t result = saMgr->UnmarshalingSaExtraProp(data, extraProp);
    EXPECT_EQ(result, ERR_FLATTEN_OBJECT);
}

/**
 * @tc.name: CheckSystemAbilityImmeInner001
 * @tc.desc: test CheckSystemAbilityImmeInner, read systemAbilityId failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, CheckSystemAbilityImmeInner001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(INVALID_SAID);
    int32_t result = saMgr->CheckSystemAbilityImmeInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: CheckSystemAbilityImmeInner002
 * @tc.desc: test CheckSystemAbilityImmeInner,  read isExist failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, CheckSystemAbilityImmeInner002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    int32_t result = saMgr->CheckSystemAbilityImmeInner(data, reply);
    EXPECT_EQ(result, ERR_FLATTEN_OBJECT);
}

/**
 * @tc.name: CheckSystemAbilityImmeInner003
 * @tc.desc: test CheckSystemAbilityImmeInner, WriteRemoteObject failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, CheckSystemAbilityImmeInner003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    data.WriteBool(false);
    int32_t result = saMgr->CheckSystemAbilityImmeInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: CheckSystemAbilityImmeInner004
 * @tc.desc: test CheckSystemAbilityImmeInner, penglai mode permission check failed!
 * @tc.type: FUNC
 */
#ifdef SUPPORT_PENGLAI_MODE
HWTEST_F(SystemAbilityMgrStubTest, CheckSystemAbilityImmeInner004, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    saMgr->SetPengLai(true);
    // set permission denied
    SetPenglaiPerm(false);
    int32_t result = saMgr->CheckSystemAbilityImmeInner(data, reply);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
    UnSetPenglaiPerm();
    saMgr->SetPengLai(false);
}
#endif

/**
 * @tc.name: CheckSystemAbilityImmeInner005
 * @tc.desc: test CheckSystemAbilityImmeInner, penglai mode permission check success!
 * @tc.type: FUNC
 */
#ifdef SUPPORT_PENGLAI_MODE
HWTEST_F(SystemAbilityMgrStubTest, CheckSystemAbilityImmeInner005, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    saMgr->SetPengLai(true);
    // set permission true
    SetPenglaiPerm(true);
    int32_t result = saMgr->CheckSystemAbilityImmeInner(data, reply);
    EXPECT_NE(result, ERR_PERMISSION_DENIED);
    UnSetPenglaiPerm();
    saMgr->SetPengLai(false);
}
#endif

/**
 * @tc.name: GetSystemAbilityInner001
 * @tc.desc: test GetSystemAbilityInner, read systemAbilityId failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, GetSystemAbilityInner001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t result = saMgr->GetSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: GetSystemAbilityInner002
 * @tc.desc: test GetSystemAbilityInner, penglai mode permission check failed!
 * @tc.type: FUNC
 */
#ifdef SUPPORT_PENGLAI_MODE
HWTEST_F(SystemAbilityMgrStubTest, GetSystemAbilityInner002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    saMgr->SetPengLai(true);
    // set permission denied
    SetPenglaiPerm(false);
    int32_t result = saMgr->GetSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
    UnSetPenglaiPerm();
    saMgr->SetPengLai(false);
}
#endif

/**
 * @tc.name: GetSystemAbilityInner003
 * @tc.desc: test GetSystemAbilityInner, penglai mode permission check success!
 * @tc.type: FUNC
 */
#ifdef SUPPORT_PENGLAI_MODE
HWTEST_F(SystemAbilityMgrStubTest, GetSystemAbilityInner003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    saMgr->SetPengLai(true);
    // set permission true
    SetPenglaiPerm(true);
    int32_t result = saMgr->GetSystemAbilityInner(data, reply);
    EXPECT_NE(result, ERR_PERMISSION_DENIED);
    UnSetPenglaiPerm();
    saMgr->SetPengLai(false);
}
#endif

/**
 * @tc.name: CheckSystemAbilityInner001
 * @tc.desc: test CheckSystemAbilityInner, read systemAbilityId failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, CheckSystemAbilityInner001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t result = saMgr->CheckSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: CheckSystemAbilityInner002
 * @tc.desc: test CheckSystemAbilityInner, penglai mode permission check failed!
 * @tc.type: FUNC
 */
#ifdef SUPPORT_PENGLAI_MODE
HWTEST_F(SystemAbilityMgrStubTest, CheckSystemAbilityInner002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    saMgr->SetPengLai(true);
    // set permission denied
    SetPenglaiPerm(false);
    int32_t result = saMgr->CheckSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
    UnSetPenglaiPerm();
    saMgr->SetPengLai(false);
}
#endif

/**
 * @tc.name: CheckSystemAbilityInner003
 * @tc.desc: test CheckSystemAbilityInner, penglai mode permission check succes!
 * @tc.type: FUNC
 */
#ifdef SUPPORT_PENGLAI_MODE
HWTEST_F(SystemAbilityMgrStubTest, CheckSystemAbilityInner003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    saMgr->SetPengLai(true);
    // set permission true
    SetPenglaiPerm(true);
    int32_t result = saMgr->CheckSystemAbilityInner(data, reply);
    EXPECT_NE(result, ERR_PERMISSION_DENIED);
    UnSetPenglaiPerm();
    saMgr->SetPengLai(false);
}
#endif

/**
 * @tc.name: AddSystemAbilityInner002
 * @tc.desc: test AddSystemAbilityInner, read systemAbilityId failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddSystemAbilityInner002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(INVALID_SAID);
    int32_t result = saMgr->AddSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: AddSystemAbilityInner003
 * @tc.desc: test AddSystemAbilityInner, readParcelable failed!!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddSystemAbilityInner003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    int32_t result = saMgr->AddSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: AddSystemAbilityInner004
 * @tc.desc: test AddSystemAbilityInner, UnmarshalingSaExtraProp failed!!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddSystemAbilityInner004, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<IRemoteObject> testAbility(new SaStatusChangeMock());
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    data.WriteRemoteObject(testAbility);
    int32_t result = saMgr->AddSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_FLATTEN_OBJECT);
}

/**
 * @tc.name: AddSystemAbilityInner005
 * @tc.desc: test AddSystemAbilityInner, ERR_OK.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddSystemAbilityInner005, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->abilityStateScheduler_ = std::make_shared<SystemAbilityStateScheduler>(
    std::weak_ptr<BaseSystemAbilityManager>{});
    sptr<IRemoteObject> testAbility(new SaStatusChangeMock());
    EXPECT_TRUE(saMgr != nullptr);
    SystemAbilityManager::SAExtraProp extraProp;
    bool isDistributed = true;
    int32_t dumpFlags = 0;
    std::u16string capability = u"capability";
    std::u16string permission = u"permission";
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    data.WriteRemoteObject(testAbility);
    data.WriteBool(isDistributed);
    data.WriteInt32(dumpFlags);
    data.WriteString16(capability);
    data.WriteString16(permission);
    CommonSaProfile saProfile;
    saProfile.process = u"test";
    saProfile.distributed = true;
    saProfile.saId = SAID;
    saMgr->saProfileMap_[SAID] = saProfile;
    int32_t result = saMgr->AddSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_OK);
    saMgr->saProfileMap_.erase(SAID);
}

/**
 * @tc.name: Test GetSystemProcessInfoInner002
 * @tc.desc: GetSystemProcessInfoInner002, permission denied!
 * @tc.type: FUNC
 * @tc.require: I7VQQG
 */
HWTEST_F(SystemAbilityMgrStubTest, GetSystemProcessInfoInner002, TestSize.Level3)
{
    DTEST_LOG << "GetSystemProcessInfoInner002" << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(INVALID_SAID);
    int32_t ret = saMgr->GetSystemProcessInfoInner(data, reply);
    EXPECT_EQ(ret, ERR_NULL_OBJECT);
}

/**
 * @tc.name: Test GetSystemProcessInfoInner003
 * @tc.desc: GetSystemProcessInfoInner003, permission denied!
 * @tc.type: FUNC
 * @tc.require: I7VQQG
 */
HWTEST_F(SystemAbilityMgrStubTest, GetSystemProcessInfoInner003, TestSize.Level3)
{
    DTEST_LOG << "GetSystemProcessInfoInner003" << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = saMgr->GetSystemProcessInfoInner(data, reply);
    EXPECT_EQ(ret, ERR_NULL_OBJECT);
}

/**
 * @tc.name: AddSystemProcessInner002
 * @tc.desc: test AddSystemProcessInner, read process name failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddSystemProcessInner002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t result = saMgr->AddSystemProcessInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: AddSystemProcessInner003
 * @tc.desc: test AddSystemProcessInner readParcelable failed!!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddSystemProcessInner003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    std::u16string procName = u"test";
    data.WriteString16(procName);
    int32_t result = saMgr->AddSystemProcessInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: AddSystemProcessInner004
 * @tc.desc: test AddSystemProcessInner!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddSystemProcessInner004, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    sptr<IRemoteObject> testAbility(new SaStatusChangeMock());
    MessageParcel data;
    MessageParcel reply;
    std::u16string procName = u"test";
    data.WriteString16(procName);
    data.WriteRemoteObject(testAbility);
    int32_t result = saMgr->AddSystemProcessInner(data, reply);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: Test GetRunningSystemProcessInner002
 * @tc.desc: GetRunningSystemProcessInner002
 * @tc.type: FUNC
 * @tc.require: I6H10P
 */
HWTEST_F(SystemAbilityMgrStubTest, GetRunningSystemProcessInner002, TestSize.Level3)
{
    DTEST_LOG << "GetRunningSystemProcessInner002" << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = saMgr->GetRunningSystemProcessInner(data, reply);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: Test SubscribeSystemProcessInner002
 * @tc.desc: SubscribeSystemProcessInner002
 * @tc.type: FUNC
 * @tc.require: I6H10P
 */
HWTEST_F(SystemAbilityMgrStubTest, SubscribeSystemProcessInner002, TestSize.Level3)
{
    DTEST_LOG << "SubscribeSystemProcessInner002" << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    data.WriteRemoteObject(nullptr);
    MessageParcel reply;
    int32_t ret = saMgr->SubscribeSystemProcessInner(data, reply);
    EXPECT_EQ(ret, ERR_NULL_OBJECT);
}

/**
 * @tc.name: Test SubscribeSystemProcessInner003
 * @tc.desc: SubscribeSystemProcessInner003
 * @tc.type: FUNC
 * @tc.require: I6H10P
 */
HWTEST_F(SystemAbilityMgrStubTest, SubscribeSystemProcessInner003, TestSize.Level3)
{
    DTEST_LOG << "SubscribeSystemProcessInner003" << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    sptr<IRemoteObject> ptr = new SystemProcessStatusChange();
    data.WriteRemoteObject(ptr);
    MessageParcel reply;
    int32_t ret = saMgr->SubscribeSystemProcessInner(data, reply);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: SendStrategyInner001
 * @tc.desc: test SendStrategyInner, read systemAbilityId failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, SendStrategyInner001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t result = saMgr->SendStrategyInner(data, reply);
    EXPECT_EQ(result, ERR_FLATTEN_OBJECT);
}

/**
 * @tc.name: GetLocalAbilityManagerProxyInner001
 * @tc.desc: test GetLocalAbilityManagerProxyInner
 * @tc.type: FUNC
 * @tc.require: I6XB42
 */
HWTEST_F(SystemAbilityMgrStubTest, GetLocalAbilityManagerProxyInner001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = saMgr->GetLocalAbilityManagerProxyInner(data, reply);
    EXPECT_EQ(ret, ERR_NULL_OBJECT);
}

/**
 * @tc.name: GetLocalAbilityManagerProxyInner002
 * @tc.desc: test GetLocalAbilityManagerProxyInner
 * @tc.type: FUNC
 * @tc.require: I6XB42
 */
HWTEST_F(SystemAbilityMgrStubTest, GetLocalAbilityManagerProxyInner002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    EXPECT_TRUE(data.WriteInt32(INVALID_SAID));
    int32_t ret = saMgr->GetLocalAbilityManagerProxyInner(data, reply);
    EXPECT_EQ(ret, ERR_NULL_OBJECT);
}

/**
 * @tc.name: GetLocalAbilityManagerProxyInner003
 * @tc.desc: test GetLocalAbilityManagerProxyInner
 * @tc.type: FUNC
 * @tc.require: I6XB42
 */
HWTEST_F(SystemAbilityMgrStubTest, GetLocalAbilityManagerProxyInner003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    EXPECT_TRUE(data.WriteInt32(SAID));
    int32_t ret = saMgr->GetLocalAbilityManagerProxyInner(data, reply);
    EXPECT_EQ(ret, ERR_NULL_OBJECT);
}

/**
 * @tc.name: SetSamgrIpcPriorInner001
 * @tc.desc: test SetSamgrIpcPriorInner with other process
 * @tc.type: FUNC
 * @tc.require: I6XB42
 */
HWTEST_F(SystemAbilityMgrStubTest, SetSamgrIpcPriorInner001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    EXPECT_TRUE(data.WriteBool(true));
    int32_t ret = saMgr->SetSamgrIpcPriorInner(data, reply);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: SetSamgrIpcPriorInner002
 * @tc.desc: test SetSamgrIpcPriorInner with rss process
 * @tc.type: FUNC
 * @tc.require: I6XB42
 */
HWTEST_F(SystemAbilityMgrStubTest, SetSamgrIpcPriorInner002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    SamMockPermission::MockProcess("resource_schedule_service");
    EXPECT_TRUE(data.WriteBool(true));
    int32_t ret = saMgr->SetSamgrIpcPriorInner(data, reply);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: SubscribeLowMemSystemProcessInner001
 * @tc.desc: test SubscribeLowMemSystemProcessInner with empty data
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, SubscribeLowMemSystemProcessInner001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = saMgr->SubscribeLowMemSystemProcessInner(data, reply);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: SubscribeLowMemSystemProcessInner002
 * @tc.desc: test SubscribeLowMemSystemProcessInner with iface_cast failure
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, SubscribeLowMemSystemProcessInner002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    sptr<IPCObjectStub> invalidListener = sptr<IPCObjectStub>::MakeSptr(u"invalid listener");
    data.WriteRemoteObject(invalidListener);
    int32_t ret = saMgr->SubscribeLowMemSystemProcessInner(data, reply);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: SubscribeLowMemSystemProcessInner003
 * @tc.desc: test SubscribeLowMemSystemProcessInner with null remote object
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, SubscribeLowMemSystemProcessInner003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    sptr<IRemoteObject> ptr = new SystemProcessStatusChange();
    data.WriteRemoteObject(ptr);
    reply.ParseFrom(0, 0); // eventually makes reply non writable
    int32_t ret = saMgr->SubscribeLowMemSystemProcessInner(data, reply);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: SubscribeLowMemSystemProcessInner004
 * @tc.desc: test SubscribeLowMemSystemProcessInner succeeded
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, SubscribeLowMemSystemProcessInner004, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    sptr<IRemoteObject> ptr = new SystemProcessStatusChange();
    data.WriteRemoteObject(ptr);
    int32_t ret = saMgr->SubscribeLowMemSystemProcessInner(data, reply);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: UnSubscribeLowMemSystemProcessInner001
 * @tc.desc: test UnSubscribeLowMemSystemProcessInner with empty data
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, UnSubscribeLowMemSystemProcessInner001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = saMgr->UnSubscribeLowMemSystemProcessInner(data, reply);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: UnSubscribeLowMemSystemProcessInner002
 * @tc.desc: test UnSubscribeLowMemSystemProcessInner with iface_cast failure
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, UnSubscribeLowMemSystemProcessInner002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    sptr<IPCObjectStub> invalidListener = sptr<IPCObjectStub>::MakeSptr(u"invalid listener");
    data.WriteRemoteObject(invalidListener);
    int32_t ret = saMgr->UnSubscribeLowMemSystemProcessInner(data, reply);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: UnSubscribeLowMemSystemProcessInner003
 * @tc.desc: test UnSubscribeLowMemSystemProcessInner with null remote object
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, UnSubscribeLowMemSystemProcessInner003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    sptr<IRemoteObject> ptr = new SystemProcessStatusChange();
    data.WriteRemoteObject(ptr);
    reply.ParseFrom(0, 0); // eventually makes reply non writable
    int32_t ret = saMgr->UnSubscribeLowMemSystemProcessInner(data, reply);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: UnSubscribeLowMemSystemProcessInner004
 * @tc.desc: test UnSubscribeLowMemSystemProcessInner succeeded
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, UnSubscribeLowMemSystemProcessInner004, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    sptr<IRemoteObject> ptr = new SystemProcessStatusChange();
    data.WriteRemoteObject(ptr);
    int32_t ret = saMgr->UnSubscribeLowMemSystemProcessInner(data, reply);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: OnStartSystemAbilityFailInner001
 * @tc.desc: test OnStartSystemAbilityFailInner, read failed
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, OnStartSystemAbilityFailInner001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    {
        MessageParcel data;
        MessageParcel reply;
        int32_t ret = saMgr->OnStartSystemAbilityFailInner(data, reply);
        EXPECT_EQ(ret, ERR_INVALID_VALUE);
    }
    {
        MessageParcel data;
        MessageParcel reply;
        EXPECT_TRUE(data.WriteInt32(SAID));
        int32_t ret = saMgr->OnStartSystemAbilityFailInner(data, reply);
        EXPECT_EQ(ret, ERR_INVALID_VALUE);
    }
}

/**
 * @tc.name: OnStartSystemAbilityFailInner002
 * @tc.desc: test OnStartSystemAbilityFailInner, Check said failed
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, OnStartSystemAbilityFailInner002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    EXPECT_TRUE(data.WriteInt32(-1));
    EXPECT_TRUE(data.WriteInt32(-1));
    int32_t ret = saMgr->OnStartSystemAbilityFailInner(data, reply);
    EXPECT_EQ(ret, ERR_NULL_OBJECT);
}

/**
 * @tc.name: OnStartSystemAbilityFailInner003
 * @tc.desc: test OnStartSystemAbilityFailInner, result failed
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, OnStartSystemAbilityFailInner003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    EXPECT_TRUE(data.WriteInt32(SAID));
    EXPECT_TRUE(data.WriteInt32(-1));
    int32_t ret = saMgr->OnStartSystemAbilityFailInner(data, reply);
    EXPECT_EQ(ret, ERR_OK);
}

#ifdef SUPPORT_MULTI_INSTANCE
/**
 * @tc.name: OnUserStateChangedInner001
 * @tc.desc: test OnUserStateChangedInner, no interface token
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, OnUserStateChangedInner001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = saMgr->OnUserStateChangedInner(data, reply);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: OnUserStateChangedInner002
 * @tc.desc: test OnUserStateChangedInner, invalid userState
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, OnUserStateChangedInner002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    SamMockPermission::MockProcess("accountmgr");
    MessageParcel data;
    MessageParcel reply;
    EXPECT_TRUE(data.WriteInt32(100));
    EXPECT_TRUE(data.WriteInt32(-1));
    int32_t ret = saMgr->OnUserStateChangedInner(data, reply);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: OnUserStateChangedInner003
 * @tc.desc: test OnUserStateChangedInner, userState out of range
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, OnUserStateChangedInner003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    SamMockPermission::MockProcess("accountmgr");
    MessageParcel data;
    MessageParcel reply;
    EXPECT_TRUE(data.WriteInt32(100));
    EXPECT_TRUE(data.WriteInt32(3));
    int32_t ret = saMgr->OnUserStateChangedInner(data, reply);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: OnUserStateChangedInner004
 * @tc.desc: test OnUserStateChangedInner with accountmgr process and valid ACTIVATING
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, OnUserStateChangedInner004, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    SamMockPermission::MockProcess("accountmgr");
    InitUserLifecycleManager(saMgr);
    MessageParcel data;
    MessageParcel reply;
    constexpr int32_t userId = 104;
    EXPECT_TRUE(data.WriteInt32(userId));
    EXPECT_TRUE(data.WriteInt32(static_cast<int32_t>(USER_STATE_ACTIVATING)));
    int32_t ret = saMgr->OnUserStateChangedInner(data, reply);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(saMgr->OnUserStateChanged(userId, USER_STATE_STOPPING), ERR_OK);
}

/**
 * @tc.name: OnUserStateChangedInner005
 * @tc.desc: test OnUserStateChangedInner with accountmgr process and valid SWITCHING
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, OnUserStateChangedInner005, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    SamMockPermission::MockProcess("accountmgr");
    InitUserLifecycleManager(saMgr);
    constexpr int32_t userId = 105;
    ASSERT_EQ(saMgr->OnUserStateChanged(userId, USER_STATE_ACTIVATING), ERR_OK);
    MessageParcel data;
    MessageParcel reply;
    EXPECT_TRUE(data.WriteInt32(userId));
    EXPECT_TRUE(data.WriteInt32(static_cast<int32_t>(USER_STATE_SWITCHING)));
    int32_t ret = saMgr->OnUserStateChangedInner(data, reply);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(saMgr->OnUserStateChanged(userId, USER_STATE_STOPPING), ERR_OK);
}

/**
 * @tc.name: OnUserStateChangedInner006
 * @tc.desc: test OnUserStateChangedInner with accountmgr process and valid STOPPING
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, OnUserStateChangedInner006, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    SamMockPermission::MockProcess("accountmgr");
    InitUserLifecycleManager(saMgr);
    constexpr int32_t userId = 106;
    ASSERT_EQ(saMgr->OnUserStateChanged(userId, USER_STATE_ACTIVATING), ERR_OK);
    MessageParcel data;
    MessageParcel reply;
    EXPECT_TRUE(data.WriteInt32(userId));
    EXPECT_TRUE(data.WriteInt32(static_cast<int32_t>(USER_STATE_STOPPING)));
    int32_t ret = saMgr->OnUserStateChangedInner(data, reply);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: OnUserStateChangedInner007
 * @tc.desc: test OnUserStateChangedInner with invalid caller process
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, OnUserStateChangedInner007, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    SamMockPermission::MockProcess("invalid_process");
    MessageParcel data;
    MessageParcel reply;
    EXPECT_TRUE(data.WriteInt32(100));
    EXPECT_TRUE(data.WriteInt32(static_cast<int32_t>(USER_STATE_ACTIVATING)));
    int32_t ret = saMgr->OnUserStateChangedInner(data, reply);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: OnUserStateChangedInner008
 * @tc.desc: test OnUserStateChangedInner with accountmgr process but missing userState
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, OnUserStateChangedInner008, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    SamMockPermission::MockProcess("accountmgr");
    MessageParcel data;
    MessageParcel reply;
    EXPECT_TRUE(data.WriteInt32(100));
    int32_t ret = saMgr->OnUserStateChangedInner(data, reply);
    EXPECT_EQ(ret, ERR_FLATTEN_OBJECT);
}

/**
 * @tc.name: OnUserStateChangedInner009
 * @tc.desc: test OnUserStateChangedInner forwards the user ID sentinel to service validation
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, OnUserStateChangedInner009, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    SamMockPermission::MockProcess("accountmgr");
    MessageParcel data;
    MessageParcel reply;
    EXPECT_TRUE(data.WriteInt32(SAMGR_INVALID_USER_ID));
    EXPECT_TRUE(data.WriteInt32(static_cast<int32_t>(USER_STATE_ACTIVATING)));
    int32_t ret = saMgr->OnUserStateChangedInner(data, reply);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(reply.ReadInt32(), ERR_INVALID_VALUE);
}

/**
 * @tc.name: OnUserStateChanged001
 * @tc.desc: test OnUserStateChanged, save and verify user state
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, OnUserStateChanged001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitUserLifecycleManager(saMgr);
    constexpr int32_t userId = 110;
    int32_t ret = saMgr->OnUserStateChanged(userId, USER_STATE_ACTIVATING);
    EXPECT_EQ(ret, ERR_OK);
    {
        std::shared_lock<samgr::shared_mutex> lock(saMgr->userLifecycleManager_.userStateLock_);
        auto it = saMgr->userLifecycleManager_.userStateMap_.find(userId);
        EXPECT_TRUE(it != saMgr->userLifecycleManager_.userStateMap_.end());
        EXPECT_EQ(it->second, USER_STATE_ACTIVATING);
    }
    ret = saMgr->OnUserStateChanged(userId, USER_STATE_SWITCHING);
    EXPECT_EQ(ret, ERR_OK);
    {
        std::shared_lock<samgr::shared_mutex> lock(saMgr->userLifecycleManager_.userStateLock_);
        auto it = saMgr->userLifecycleManager_.userStateMap_.find(userId);
        EXPECT_TRUE(it != saMgr->userLifecycleManager_.userStateMap_.end());
        EXPECT_EQ(it->second, USER_STATE_SWITCHING);
    }
    ret = saMgr->OnUserStateChanged(userId, USER_STATE_STOPPING);
    EXPECT_EQ(ret, ERR_OK);
    {
        std::shared_lock<samgr::shared_mutex> lock(saMgr->userLifecycleManager_.userStateLock_);
        auto it = saMgr->userLifecycleManager_.userStateMap_.find(userId);
        EXPECT_EQ(it, saMgr->userLifecycleManager_.userStateMap_.end());
    }
}

/**
 * @tc.name: OnUserStateChanged002
 * @tc.desc: test repeated user activation is idempotent
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, OnUserStateChanged002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitUserLifecycleManager(saMgr);
    constexpr int32_t userId = 111;
    ASSERT_EQ(saMgr->OnUserStateChanged(userId, USER_STATE_ACTIVATING), ERR_OK);
    EXPECT_EQ(saMgr->OnUserStateChanged(userId, USER_STATE_ACTIVATING), ERR_OK);
    {
        std::shared_lock<samgr::shared_mutex> lock(saMgr->userLifecycleManager_.userStateLock_);
        auto it = saMgr->userLifecycleManager_.userStateMap_.find(userId);
        EXPECT_TRUE(it != saMgr->userLifecycleManager_.userStateMap_.end());
        EXPECT_EQ(it->second, USER_STATE_ACTIVATING);
    }
    EXPECT_EQ(saMgr->OnUserStateChanged(userId, USER_STATE_STOPPING), ERR_OK);
}

/**
 * @tc.name: MultiUserMissingFields001
 * @tc.desc: Verify user-aware handlers reject parcels missing required fields.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, MultiUserMissingFields001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    ASSERT_NE(saMgr, nullptr);
    MessageParcel reply;
    MessageParcel data;
    EXPECT_EQ(saMgr->GetSystemAbilityWithUserIdInner(data, reply), ERR_NULL_OBJECT);
    EXPECT_EQ(saMgr->CheckSystemAbilityByUserIdInner(data, reply), ERR_NULL_OBJECT);
    EXPECT_EQ(saMgr->LoadSystemAbilityWithUserIdInner(data, reply), ERR_INVALID_VALUE);
}
#endif
}