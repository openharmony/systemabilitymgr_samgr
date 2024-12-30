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
#include "ability_death_recipient.h"
#include "test_log.h"
#define private public
#include "ipc_skeleton.h"
#ifdef SUPPORT_ACCESS_TOKEN
#include "accesstoken_kit.h"
#endif
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
constexpr int32_t TEST_OVERFLOW_SAID = 99999;
constexpr int32_t TEST_EXCEPTION_HIGH_SA_ID = LAST_SYS_ABILITY_ID + 1;
constexpr int32_t TEST_EXCEPTION_LOW_SA_ID = -1;
constexpr int32_t ONDEMAND_SLEEP_TIME = 600 * 1000; // us
constexpr int64_t ONDEMAND_EXTRA_DATA_ID = 1;

const string ONDEMAND_PARAM = "persist.samgr.perf.ondemand";

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
 * @tc.name: StartOnDemandAbility001
 * @tc.desc: test StartOnDemandAbility, invalid systemAbilityId.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, StartOnDemandAbility001, TestSize.Level0)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
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
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
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
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
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
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
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
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
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
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
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
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
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
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
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
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
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
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
    int32_t said = -1;
    int32_t result = saMgr->AddOnDemandSystemAbilityInfo(said, u"");
    EXPECT_EQ(result, ERR_INVALID_VALUE);
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
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
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
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
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
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
    CommonSaProfile saProfile;
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
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
    CommonSaProfile saProfile;
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
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
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
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
    CommonSaProfile saProfile;
    saMgr->saProfileMap_.clear();
    saMgr->saProfileMap_[SAID] = saProfile;
    auto ret = saMgr->GetAllOndemandSa();
    EXPECT_FALSE(ret.empty());
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
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
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
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
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
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
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
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    ISystemAbilityManager::SAExtraProp saExtraProp;
    saMgr->abilityStateScheduler_->processHandler_ = nullptr;
    saMgr->AddSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID, testAbility, saExtraProp);
    bool isExist = true;
    bool result = saMgr->DoLoadOnDemandAbility(DISTRIBUTED_SCHED_TEST_TT_ID, isExist);
    EXPECT_EQ(result, true);
    CommonSaProfile saProfile = {u"test", DISTRIBUTED_SCHED_TEST_TT_ID};
    saProfile.cacheCommonEvent = true;
    saMgr->saProfileMap_[DISTRIBUTED_SCHED_TEST_TT_ID] = saProfile;
    int32_t ret = saMgr->RemoveSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID);
    EXPECT_EQ(ret, ERR_OK);
    saMgr->saProfileMap_.erase(DISTRIBUTED_SCHED_TEST_TT_ID);
}

/**
 * @tc.name: StartOnDemandAbilityInner001
 * @tc.desc: test StartOnDemandAbilityInner, ERR_INVALID_VALUE.
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrTest, StartOnDemandAbilityInner001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
    const std::u16string procName;
    int32_t systemAbilityId = 0;
    SystemAbilityManager::AbilityItem abilityItem;
    abilityItem.state = SystemAbilityManager::AbilityState::STARTING;
    int32_t ret = saMgr->StartOnDemandAbilityInner(procName, systemAbilityId, abilityItem);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.desc: test GetOnDemandPolicy with OnDemandPolicyType is valid
 * @tc.type: FUNC
 * @tc.require: I6V4AX
 */
HWTEST_F(SystemAbilityMgrTest, GetOnDemandPolicy001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
    int32_t systemAbilityId = 1;
    OnDemandPolicyType type = OnDemandPolicyType::START_POLICY;
    std::vector<SystemAbilityOnDemandEvent> abilityOnDemandEvents;
    int32_t ret = saMgr->GetOnDemandPolicy(systemAbilityId, type, abilityOnDemandEvents);
    EXPECT_EQ(ERR_INVALID_VALUE, ret);
}

/**
 * @tc.desc: test GetOnDemandPolicy with OnDemandPolicyType is valid
 * @tc.type: FUNC
 * @tc.require: I6V4AX
 */
HWTEST_F(SystemAbilityMgrTest, GetOnDemandPolicy002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
    int32_t systemAbilityId = 1;
    OnDemandPolicyType type = OnDemandPolicyType::STOP_POLICY;
    std::vector<SystemAbilityOnDemandEvent> abilityOnDemandEvents;
    sptr<DeviceStatusCollectManager> collectManager_ = nullptr;
    int32_t ret = saMgr->GetOnDemandPolicy(systemAbilityId, type, abilityOnDemandEvents);
    EXPECT_EQ(ERR_INVALID_VALUE, ret);
}

#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityMgrTest, GetOnDemandPolicy003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
    int32_t systemAbilityId = 1;
    OnDemandPolicyType type = OnDemandPolicyType::START_POLICY;
    uint32_t accessToken = IPCSkeleton::GetCallingTokenID();
    Security::AccessToken::NativeTokenInfo nativeTokenInfo;
    int32_t tokenInfoResult = Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(accessToken, nativeTokenInfo);
    CommonSaProfile saProfile;
    saProfile.process = Str8ToStr16(nativeTokenInfo.processName);
    saProfile.startAllowUpdate= false;
    saMgr->saProfileMap_[1] = saProfile;
    std::vector<SystemAbilityOnDemandEvent> abilityOnDemandEvents;
    int32_t ret = saMgr->GetOnDemandPolicy(systemAbilityId, type, abilityOnDemandEvents);
    EXPECT_EQ(ERR_PERMISSION_DENIED, ret);
}
#endif

#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityMgrTest, GetOnDemandPolicy004, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
    int32_t systemAbilityId = 1;
    OnDemandPolicyType type = OnDemandPolicyType::START_POLICY;
    uint32_t accessToken = IPCSkeleton::GetCallingTokenID();
    Security::AccessToken::NativeTokenInfo nativeTokenInfo;
    int32_t tokenInfoResult = Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(accessToken, nativeTokenInfo);
    CommonSaProfile saProfile;
    saProfile.process = Str8ToStr16(nativeTokenInfo.processName);
    saProfile.startAllowUpdate = true;
    saMgr->saProfileMap_[1] = saProfile;
    sptr<DeviceStatusCollectManager> collectManager = nullptr;
    saMgr->collectManager_ = collectManager;
    std::vector<SystemAbilityOnDemandEvent> abilityOnDemandEvents;
    int32_t ret = saMgr->GetOnDemandPolicy(systemAbilityId, type, abilityOnDemandEvents);
    EXPECT_EQ(ERR_INVALID_VALUE, ret);
}
#endif

#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityMgrTest, GetOnDemandPolicy005, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
    int32_t systemAbilityId = 1;
    OnDemandPolicyType type = OnDemandPolicyType::START_POLICY;
    uint32_t accessToken = IPCSkeleton::GetCallingTokenID();
    Security::AccessToken::NativeTokenInfo nativeTokenInfo;
    int32_t tokenInfoResult = Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(accessToken, nativeTokenInfo);
    CommonSaProfile saProfile;
    saProfile.process = Str8ToStr16(nativeTokenInfo.processName);
    saProfile.startAllowUpdate = true;
    saMgr->saProfileMap_[1] = saProfile;
    sptr<DeviceStatusCollectManager> collectManager = new DeviceStatusCollectManager();
    saMgr->collectManager_ = collectManager;
    std::vector<SystemAbilityOnDemandEvent> abilityOnDemandEvents;
    int32_t ret = saMgr->GetOnDemandPolicy(systemAbilityId, type, abilityOnDemandEvents);
    EXPECT_EQ(ERR_INVALID_VALUE, ret);
}
#endif

#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityMgrTest, GetOnDemandPolicy006, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
    int32_t systemAbilityId = 1;
    OnDemandPolicyType type = OnDemandPolicyType::START_POLICY;
    uint32_t accessToken = IPCSkeleton::GetCallingTokenID();
    Security::AccessToken::NativeTokenInfo nativeTokenInfo;
    int32_t tokenInfoResult = Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(accessToken, nativeTokenInfo);
    SaProfile saProfile;
    saProfile.process = Str8ToStr16(nativeTokenInfo.processName);
    saProfile.saId = 1;
    saProfile.startOnDemand.allowUpdate = true;
    vector<OnDemandEvent> onDemandEvents;
    onDemandEvents.push_back({ 1, "test" });
    saProfile.startOnDemand.onDemandEvents = onDemandEvents;
    SamgrUtil::FilterCommonSaProfile(saProfile,  saMgr->saProfileMap_[1]);
    sptr<DeviceStatusCollectManager> collectManager = new DeviceStatusCollectManager();
    std::list<SaProfile> saProfiles;
    saProfiles.emplace_back(saProfile);
    collectManager->FilterOnDemandSaProfiles(saProfiles);
    saMgr->collectManager_ = collectManager;
    std::vector<SystemAbilityOnDemandEvent> abilityOnDemandEvents;
    int32_t ret = saMgr->GetOnDemandPolicy(systemAbilityId, type, abilityOnDemandEvents);
    EXPECT_EQ(ERR_OK, ret);
}
#endif

/**
 * @tc.name: UpdateOnDemandPolicy001
 * @tc.desc: test UpdateOnDemandPolicy with OnDemandPolicyType is valid
 * @tc.type: FUNC
 * @tc.require: I6V4AX
 */
#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityMgrTest, UpdateOnDemandPolicy001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
    OnDemandPolicyType type = OnDemandPolicyType::START_POLICY;
    SystemAbilityOnDemandEvent event = {OnDemandEventId::COMMON_EVENT, "TEST", "TEST"};
    std::vector<SystemAbilityOnDemandEvent> abilityOnDemandEvents;
    abilityOnDemandEvents.emplace_back(event);
    CommonSaProfile saProfile = {u"test", TEST_OVERFLOW_SAID};
    saMgr->saProfileMap_[TEST_OVERFLOW_SAID] = saProfile;
    int32_t ret = saMgr->UpdateOnDemandPolicy(TEST_OVERFLOW_SAID, type, abilityOnDemandEvents);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    uint32_t accessToken = IPCSkeleton::GetCallingTokenID();
    Security::AccessToken::NativeTokenInfo nativeTokenInfo;
    int32_t result = Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(accessToken, nativeTokenInfo);
    EXPECT_TRUE(result == ERR_OK);
    saMgr->saProfileMap_[TEST_OVERFLOW_SAID].process = Str8ToStr16(nativeTokenInfo.processName);
    saMgr->saProfileMap_[TEST_OVERFLOW_SAID].startAllowUpdate = false;
    ret = saMgr->UpdateOnDemandPolicy(TEST_OVERFLOW_SAID, type, abilityOnDemandEvents);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);

    saMgr->saProfileMap_[TEST_OVERFLOW_SAID].startAllowUpdate = true;
    sptr<DeviceStatusCollectManager> collectMgr = saMgr->collectManager_;
    saMgr->collectManager_ = nullptr;
    ret = saMgr->UpdateOnDemandPolicy(TEST_OVERFLOW_SAID, type, abilityOnDemandEvents);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    saMgr->collectManager_ = collectMgr;
    ret = saMgr->UpdateOnDemandPolicy(TEST_OVERFLOW_SAID, type, abilityOnDemandEvents);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    saMgr->saProfileMap_.erase(TEST_OVERFLOW_SAID);
}
#endif

/**
 * @tc.name: GetOnDemandReasonExtraData001
 * @tc.desc: test GetOnDemandReasonExtraData with collectManager_ is nullptr
 * @tc.type: FUNC
 * @tc.require: I6XB42
 */

HWTEST_F(SystemAbilityMgrTest, GetOnDemandReasonExtraData001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
    MessageParcel messageParcel;
    sptr<DeviceStatusCollectManager> collectMgr = saMgr->collectManager_;
    saMgr->collectManager_ = nullptr;
    int32_t ret = saMgr->GetOnDemandReasonExtraData(ONDEMAND_EXTRA_DATA_ID, messageParcel);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    
    saMgr->collectManager_ = collectMgr;
    ret = saMgr->GetOnDemandReasonExtraData(ONDEMAND_EXTRA_DATA_ID, messageParcel);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: GetOnDemandReasonExtraData002
 * @tc.desc: test GetOnDemandReasonExtraData with extraDataId is not exist
 * @tc.type: FUNC
 * @tc.require: I6XB42
 */

HWTEST_F(SystemAbilityMgrTest, GetOnDemandReasonExtraData002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
    sptr<DeviceStatusCollectManager> collectManager = new DeviceStatusCollectManager();
    collectManager->collectPluginMap_.clear();
    saMgr->collectManager_ = collectManager;
    MessageParcel messageParcel;
    int32_t ret = saMgr->GetOnDemandReasonExtraData(ONDEMAND_EXTRA_DATA_ID, messageParcel);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: GetOnDemandReasonExtraData003
 * @tc.desc: call GetOnDemandReasonExtraData, get extraData
 * @tc.type: FUNC
 * @tc.require: I6XB42
 */

HWTEST_F(SystemAbilityMgrTest, GetOnDemandReasonExtraData003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
    sptr<DeviceStatusCollectManager> collectManager = new DeviceStatusCollectManager();
    saMgr->collectManager_ = collectManager;
    sptr<CommonEventCollect> commonEventCollect = new CommonEventCollect(collectManager);
    commonEventCollect->workHandler_ = std::make_shared<CommonHandler>(commonEventCollect);
    collectManager->collectPluginMap_.clear();
    collectManager->collectPluginMap_[COMMON_EVENT] = commonEventCollect;
    EventFwk::CommonEventData eventData;
    commonEventCollect->SaveOnDemandReasonExtraData(eventData);
    MessageParcel messageParcel;
    int32_t ret = saMgr->GetOnDemandReasonExtraData(ONDEMAND_EXTRA_DATA_ID, messageParcel);
    EXPECT_EQ(ret, ERR_OK);
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
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
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
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
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
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
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
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
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
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
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
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
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
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
    saMgr->abilityStateScheduler_ = std::make_shared<SystemAbilityStateScheduler>();
    saMgr->ProcessOnDemandEvent(event, saControlList);
    EXPECT_NE(saMgr, nullptr);
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
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
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
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
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
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
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
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
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
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
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
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
    sptr<IRemoteObject> testAbility(new SaStatusChangeMock());
    SAInfo saInfo;
    saInfo.remoteObj = testAbility;
    saMgr->abilityMap_[TEST_OVERFLOW_SAID] = saInfo;
    nlohmann::json activeReason;
    bool ret = saMgr->ActiveSystemAbility(TEST_OVERFLOW_SAID, u"test", activeReason);
    EXPECT_FALSE(ret);
    saMgr->abilityMap_.erase(TEST_OVERFLOW_SAID);
}

} // namespace OHOS
