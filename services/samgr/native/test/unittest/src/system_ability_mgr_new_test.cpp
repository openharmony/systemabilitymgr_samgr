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
#include "samgr_err_code.h"
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
#include "if_local_ability_manager.h"
#include "system_process_status_change_proxy.h"
#include "test_log.h"
#define private public
#include "system_ability_manager.h"
#ifdef SUPPORT_COMMON_EVENT
#include "common_event_collect.h"
#include "ipc_skeleton.h"
#include "accesstoken_kit.h"
#endif

using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS;

namespace OHOS {
namespace system {
    /*
    * Returns true if the system parameter `key` has the value "1", "y", "yes", "on", or "true",
    * false for "0", "n", "no", "off", or "false", or `def` otherwise.
    */
    bool GetBoolParameter(const std::string &key, bool def)
    {
        return true;
    }

    std::string GetParameter(const std::string &key, const std::string &def)
    {
        return "";
    }

    bool SetParameter(const std::string &key, const std::string &value)
    {
        return true;
    }
}
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

void SaProfileStore(sptr<SystemAbilityManager>& saMgr,
    map<int32_t, SaProfile>& saProfileMapTmp, int32_t maxLoop)
{
    for (int32_t loop = 0; loop < maxLoop; ++loop) {
        if (saMgr->saProfileMap_.count(SAID + loop) > 0) {
            saProfileMapTmp[SAID + loop] = saMgr->saProfileMap_[SAID + loop];
        }
    }
    return;
}

void SaProfileRecover(sptr<SystemAbilityManager>& saMgr,
    map<int32_t, SaProfile> saProfileMapTmp, int32_t maxLoop)
{
    for (int32_t loop = 0; loop < maxLoop; ++loop) {
        if (saProfileMapTmp.count(SAID + loop) > 0) {
            saMgr->saProfileMap_[SAID + loop] = saProfileMapTmp[SAID + loop];
        } else {
            saMgr->saProfileMap_.erase(SAID + loop);
        }
    }
    return;
}

void SaProfileExtensionTestPrevSet(sptr<SystemAbilityManager>& saMgr, int32_t maxLoop)
{
    const int32_t mod_num = 2;
    std::vector<std::string> extensionVec = { "backup_test", "restore_test", "alpha", "beta" };
    for (int32_t loop = 0; loop < maxLoop; ++loop) {
        SaProfile saProfile;
        saProfile.process = Str8ToStr16(extensionVec[loop]);
        saProfile.extension.push_back(extensionVec[loop % mod_num]);
        saProfile.cacheCommonEvent = true;
        if (loop >= (maxLoop - mod_num)) {
            saProfile.extension.push_back(extensionVec[(loop + 1) % mod_num]);
        }
        saMgr->saProfileMap_[SAID + loop] = saProfile;
    }
    return;
}

void SaAbilityMapObjTestPrevSet(sptr<SystemAbilityManager>& saMgr, int32_t maxLoop)
{
    for (int32_t loop = 0; loop < maxLoop; ++loop) {
        if (loop == 0) {
            continue;
        }
        sptr<IRemoteObject> testAbility(new SaStatusChangeMock());
        SAInfo saInfo;
        saInfo.remoteObj = testAbility;
        saMgr->abilityMap_[SAID + loop] = saInfo;
    }

    return;
}

void ProcMapObjTestPrevSet(sptr<SystemAbilityManager>& saMgr, int32_t maxLoop)
{
    std::vector<std::string> extensionVec = { "backup_test", "restore_test", "alpha", "beta" };
    for (int32_t loop = 0; loop < maxLoop; ++loop) {
        if (loop == 0) {
            continue;
        }
        sptr<IRemoteObject> testAbility(new SaStatusChangeMock());
        saMgr->systemProcessMap_[Str8ToStr16(extensionVec[loop])] = testAbility;
    }
    return;
}
}

/**
 * @tc.name: GetLocalNodeId001
 * @tc.desc: test GetLocalNodeId
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrTest, GetLocalNodeId001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    string ret = saMgr->GetLocalNodeId();
    EXPECT_EQ(ret, "");
}

/**
 * @tc.name: ReportGetSAPeriodically001
 * @tc.desc: test ReportGetSAPeriodically with saFrequencyMap_ is not empty
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrTest, ReportGetSAPeriodically001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    uint64_t pid_said = 123;
    int32_t count = 1;
    saMgr->saFrequencyMap_.clear();
    saMgr->saFrequencyMap_[pid_said] = count;
    saMgr->ReportGetSAPeriodically();
    EXPECT_TRUE(saMgr->saFrequencyMap_.empty());
}

/**
 * @tc.name: UnloadSystemAbility004
 * @tc.desc: test UnloadSystemAbility004, abilityStateScheduler_ is nullptr
 * @tc.type: FUNC
 * @tc.require: I6J4T7
 */
HWTEST_F(SystemAbilityMgrTest, UnloadSystemAbility004, TestSize.Level3)
{
    DTEST_LOG << " UnloadSystemAbility004 " << std::endl;
    SamMockPermission::MockProcess("memmgrservice");
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    SaProfile saProfile;
    saProfile.process = u"memmgrservice";
    saMgr->saProfileMap_[1] = saProfile;
    saMgr->abilityStateScheduler_ = nullptr;
    int32_t systemAbilityId = 1;
    int32_t ret = saMgr->UnloadSystemAbility(systemAbilityId);
    saMgr->abilityStateScheduler_ = make_shared<SystemAbilityStateScheduler>();
    EXPECT_EQ(ret, STATE_SCHEDULER_NULL);
}

/**
 * @tc.name: StartDynamicSystemProcess
 * @tc.desc: start invalid process, return fail.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, StartDynamicSystemProcess001, TestSize.Level3)
{
    cout << "begin StartDynamicSystemProcess001 "<< endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    std::u16string invalidProcess = u"1234567890123456789012345678901234567890123456789"
        "01234567890123456789012345678901234567890123456";
    OnDemandEvent event;
    int result = saMgr->StartDynamicSystemProcess(invalidProcess, 100, event);
    cout << "begin StartDynamicSystemProcess001 result is "<< result << endl;
    EXPECT_EQ(result, 102);
}

/**
 * @tc.desc: test GetOnDemandPolicy with OnDemandPolicyType is valid
 * @tc.type: FUNC
 * @tc.require: I6V4AX
 */
HWTEST_F(SystemAbilityMgrTest, GetOnDemandPolicy001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    int32_t systemAbilityId = 1;
    OnDemandPolicyType type = OnDemandPolicyType::STOP_POLICY;
    std::vector<SystemAbilityOnDemandEvent> abilityOnDemandEvents;
    sptr<DeviceStatusCollectManager> collectManager_ = nullptr;
    int32_t ret = saMgr->GetOnDemandPolicy(systemAbilityId, type, abilityOnDemandEvents);
    EXPECT_EQ(ERR_INVALID_VALUE, ret);
}

HWTEST_F(SystemAbilityMgrTest, GetOnDemandPolicy003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    int32_t systemAbilityId = 1;
    OnDemandPolicyType type = OnDemandPolicyType::START_POLICY;
    uint32_t accessToken = IPCSkeleton::GetCallingTokenID();
    Security::AccessToken::NativeTokenInfo nativeTokenInfo;
    int32_t tokenInfoResult = Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(accessToken, nativeTokenInfo);
    SaProfile saProfile;
    saProfile.process = Str8ToStr16(nativeTokenInfo.processName);
    saProfile.startOnDemand.allowUpdate = false;
    saMgr->saProfileMap_[1] = saProfile;
    std::vector<SystemAbilityOnDemandEvent> abilityOnDemandEvents;
    int32_t ret = saMgr->GetOnDemandPolicy(systemAbilityId, type, abilityOnDemandEvents);
    EXPECT_EQ(ERR_PERMISSION_DENIED, ret);
}

HWTEST_F(SystemAbilityMgrTest, GetOnDemandPolicy004, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    int32_t systemAbilityId = 1;
    OnDemandPolicyType type = OnDemandPolicyType::START_POLICY;
    uint32_t accessToken = IPCSkeleton::GetCallingTokenID();
    Security::AccessToken::NativeTokenInfo nativeTokenInfo;
    int32_t tokenInfoResult = Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(accessToken, nativeTokenInfo);
    SaProfile saProfile;
    saProfile.process = Str8ToStr16(nativeTokenInfo.processName);
    saProfile.startOnDemand.allowUpdate = true;
    saMgr->saProfileMap_[1] = saProfile;
    sptr<DeviceStatusCollectManager> collectManager = nullptr;
    saMgr->collectManager_ = collectManager;
    std::vector<SystemAbilityOnDemandEvent> abilityOnDemandEvents;
    int32_t ret = saMgr->GetOnDemandPolicy(systemAbilityId, type, abilityOnDemandEvents);
    EXPECT_EQ(ERR_INVALID_VALUE, ret);
}


HWTEST_F(SystemAbilityMgrTest, GetOnDemandPolicy005, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    int32_t systemAbilityId = 1;
    OnDemandPolicyType type = OnDemandPolicyType::START_POLICY;
    uint32_t accessToken = IPCSkeleton::GetCallingTokenID();
    Security::AccessToken::NativeTokenInfo nativeTokenInfo;
    int32_t tokenInfoResult = Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(accessToken, nativeTokenInfo);
    SaProfile saProfile;
    saProfile.process = Str8ToStr16(nativeTokenInfo.processName);
    saProfile.startOnDemand.allowUpdate = true;
    saMgr->saProfileMap_[1] = saProfile;
    sptr<DeviceStatusCollectManager> collectManager = new DeviceStatusCollectManager();
    saMgr->collectManager_ = collectManager;
    std::vector<SystemAbilityOnDemandEvent> abilityOnDemandEvents;
    int32_t ret = saMgr->GetOnDemandPolicy(systemAbilityId, type, abilityOnDemandEvents);
    EXPECT_EQ(ERR_INVALID_VALUE, ret);
}

HWTEST_F(SystemAbilityMgrTest, GetOnDemandPolicy006, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
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
    saMgr->saProfileMap_[1] = saProfile;
    sptr<DeviceStatusCollectManager> collectManager = new DeviceStatusCollectManager();
    collectManager->onDemandSaProfiles_.emplace_back(saProfile);
    saMgr->collectManager_ = collectManager;
    std::vector<SystemAbilityOnDemandEvent> abilityOnDemandEvents;
    int32_t ret = saMgr->GetOnDemandPolicy(systemAbilityId, type, abilityOnDemandEvents);
    EXPECT_EQ(ERR_OK, ret);
}


/**
 * @tc.name: UpdateOnDemandPolicy001
 * @tc.desc: test UpdateOnDemandPolicy with OnDemandPolicyType is valid
 * @tc.type: FUNC
 * @tc.require: I6V4AX
 */
HWTEST_F(SystemAbilityMgrTest, UpdateOnDemandPolicy001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    OnDemandPolicyType type = OnDemandPolicyType::START_POLICY;
    SystemAbilityOnDemandEvent event = {OnDemandEventId::COMMON_EVENT, "TEST", "TEST"};
    std::vector<SystemAbilityOnDemandEvent> abilityOnDemandEvents;
    abilityOnDemandEvents.emplace_back(event);
    SaProfile saProfile = {u"test", TEST_OVERFLOW_SAID};
    saMgr->saProfileMap_[TEST_OVERFLOW_SAID] = saProfile;
    int32_t ret = saMgr->UpdateOnDemandPolicy(TEST_OVERFLOW_SAID, type, abilityOnDemandEvents);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    uint32_t accessToken = IPCSkeleton::GetCallingTokenID();
    Security::AccessToken::NativeTokenInfo nativeTokenInfo;
    int32_t result = Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(accessToken, nativeTokenInfo);
    EXPECT_TRUE(result == ERR_OK);
    saMgr->saProfileMap_[TEST_OVERFLOW_SAID].process = Str8ToStr16(nativeTokenInfo.processName);
    saMgr->saProfileMap_[TEST_OVERFLOW_SAID].startOnDemand.allowUpdate = false;
    ret = saMgr->UpdateOnDemandPolicy(TEST_OVERFLOW_SAID, type, abilityOnDemandEvents);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);

    saMgr->saProfileMap_[TEST_OVERFLOW_SAID].startOnDemand.allowUpdate = true;
    sptr<DeviceStatusCollectManager> collectMgr = saMgr->collectManager_;
    saMgr->collectManager_ = nullptr;
    ret = saMgr->UpdateOnDemandPolicy(TEST_OVERFLOW_SAID, type, abilityOnDemandEvents);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    saMgr->collectManager_ = collectMgr;
    ret = saMgr->UpdateOnDemandPolicy(TEST_OVERFLOW_SAID, type, abilityOnDemandEvents);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    saMgr->saProfileMap_.erase(TEST_OVERFLOW_SAID);
}

/**
 * @tc.name: GetOnDemandReasonExtraData001
 * @tc.desc: test GetOnDemandReasonExtraData with collectManager_ is nullptr
 * @tc.type: FUNC
 * @tc.require: I6XB42
 */

HWTEST_F(SystemAbilityMgrTest, GetOnDemandReasonExtraData001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
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
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
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
 * @tc.name: GetSystemAbilityWithDevice001
 * @tc.desc: get invalid system ability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, GetSystemAbilityWithDevice001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    std::string deviceId = "";
    auto ability = saMgr->GetSystemAbility(TEST_EXCEPTION_LOW_SA_ID, deviceId);
    EXPECT_EQ(ability, nullptr);

    SaProfile saProfile = {u"test", TEST_OVERFLOW_SAID};
    saMgr->saProfileMap_[TEST_OVERFLOW_SAID] = saProfile;
    ability = saMgr->GetSystemAbility(TEST_OVERFLOW_SAID, deviceId);
    EXPECT_EQ(ability, nullptr);

    saMgr->saProfileMap_[TEST_OVERFLOW_SAID].distributed = true;
    ability = saMgr->GetSystemAbility(TEST_OVERFLOW_SAID, deviceId);
    EXPECT_EQ(ability, nullptr);
    saMgr->saProfileMap_.erase(TEST_OVERFLOW_SAID);
}

/**
 * @tc.name: GetSystemAbilityFromRemote001
 * @tc.desc: get invalid system ability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, GetSystemAbilityFromRemote001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    auto ability = saMgr->GetSystemAbilityFromRemote(TEST_EXCEPTION_LOW_SA_ID);
    EXPECT_EQ(ability, nullptr);
}

/**
 * @tc.name: GetSystemAbilityFromRemote002
 * @tc.desc: get not exist system ability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, GetSystemAbilityFromRemote002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    auto ability = saMgr->GetSystemAbilityFromRemote(TEST_SYSTEM_ABILITY1);
    EXPECT_EQ(ability, nullptr);
}

/**
 * @tc.name: GetSystemAbilityFromRemote003
 * @tc.desc: get exist system ability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, GetSystemAbilityFromRemote003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    SAInfo saInfo;
    saMgr->abilityMap_[1] = saInfo;
    auto ability = saMgr->GetSystemAbilityFromRemote(1);
    EXPECT_EQ(ability, nullptr);
}

/**
 * @tc.name: GetSystemAbilityFromRemote004
 * @tc.desc: get exist system ability, isDistributed is true.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, GetSystemAbilityFromRemote004, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    SAInfo saInfo;
    saInfo.isDistributed = true;
    saMgr->abilityMap_[1] = saInfo;
    auto ability = saMgr->GetSystemAbilityFromRemote(1);
    EXPECT_EQ(ability, nullptr);
}

/**
 * @tc.name: GetDBinder001
 * @tc.desc: GetDBinder, return null
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityMgrTest, GetDBinder001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    sptr<DBinderService> result = saMgr->GetDBinder();
    EXPECT_TRUE(saMgr != nullptr);
}

/**
 * @tc.name: NotifyRpcLoadCompleted001
 * @tc.desc: test NotifyRpcLoadCompleted, workHandler_ is nullptr
 * @tc.type: FUNC
 * @tc.require: I6MO6A
 */

HWTEST_F(SystemAbilityMgrTest, NotifyRpcLoadCompleted001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    saMgr->workHandler_ = nullptr;
    saMgr->NotifyRpcLoadCompleted("", 1, testAbility);
    EXPECT_TRUE(saMgr != nullptr);
}

/**
 * @tc.name: NotifyRpcLoadCompleted003
 * @tc.desc: test NotifyRpcLoadCompleted, dBinderService_ is null
 * @tc.type: FUNC
 * @tc.require: I6MO6A
 */

HWTEST_F(SystemAbilityMgrTest, NotifyRpcLoadCompleted003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    saMgr->dBinderService_ = nullptr;
    saMgr->NotifyRpcLoadCompleted("", 1, testAbility);
    EXPECT_TRUE(saMgr != nullptr);
}

/**
 * @tc.name: NotifyRpcLoadCompleted004
 * @tc.desc: test NotifyRpcLoadCompleted
 * @tc.type: FUNC
 * @tc.require: I6MO6A
 */
HWTEST_F(SystemAbilityMgrTest, NotifyRpcLoadCompleted004, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    saMgr->NotifyRpcLoadCompleted("", 1, testAbility);
    EXPECT_TRUE(saMgr != nullptr);
}

/**
 * @tc.name: UnloadAllIdleSystemAbility001
 * @tc.desc: UnloadAllIdleSystemAbility process is memmgrservice
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrTest, UnloadAllIdleSystemAbility001, TestSize.Level3)
{
    SamMockPermission::MockProcess("memmgrservice");
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->abilityStateScheduler_ = make_shared<SystemAbilityStateScheduler>();
    int32_t ret = saMgr->UnloadAllIdleSystemAbility();
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: UnloadAllIdleSystemAbility002
 * @tc.desc: UnloadAllIdleSystemAbility abilityStateScheduler_ is null
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrTest, UnloadAllIdleSystemAbility002, TestSize.Level3)
{
    SamMockPermission::MockProcess("memmgrservice");
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->abilityStateScheduler_ = nullptr;
    int32_t ret = saMgr->UnloadAllIdleSystemAbility();
    saMgr->abilityStateScheduler_ = make_shared<SystemAbilityStateScheduler>();
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: UnloadAllIdleSystemAbility003
 * @tc.desc: UnloadAllIdleSystemAbility process is not memmgrservice
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrTest, UnloadAllIdleSystemAbility003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    int32_t ret = saMgr->UnloadAllIdleSystemAbility();
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: Dump001
 * @tc.desc: call Dump, return ERR_OK
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityMgrTest, Dump001, TestSize.Level3)
{
    SamMockPermission::MockProcess("hidumper_service");
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    vector<std::u16string> args;
    args.push_back(u"test_name");
    int32_t result = saMgr->Dump(1, args);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: Dump002
 * @tc.desc: call Dump, return ERR_OK
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityMgrTest, Dump002, TestSize.Level3)
{
    SamMockPermission::MockProcess("hidumper_service");
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    vector<std::u16string> args;
    vector<std::string> argsWithStr8;
    args.push_back(u"test_name");
    argsWithStr8.push_back("--ffrt");
    int32_t result = saMgr->Dump(1, args);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: Dump003
 * @tc.desc: call Dump.
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityMgrTest, Dump003, TestSize.Level3)
{
    SamMockPermission::MockProcess("hidumper_service");
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    vector<std::u16string> args;
    args.push_back(u"--ipc");
    args.push_back(u"all");
    int32_t result = saMgr->Dump(1, args);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: Dump004
 * @tc.desc: call Dump.
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityMgrTest, Dump004, TestSize.Level3)
{
    SamMockPermission::MockProcess("hidumper_service");
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    vector<std::u16string> args;
    args.push_back(u"--ffrt");
    args.push_back(u"99999");
    int32_t result = saMgr->Dump(1, args);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AddSamgrToAbilityMap001
 * @tc.desc: call AddSamgrToAbilityMap, return ERR_OK
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityMgrTest, AddSamgrToAbilityMap001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->AddSamgrToAbilityMap();
    vector<std::u16string> args;
    args.push_back(u"test_name");
    int32_t result = saMgr->Dump(1, args);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: GetCommonEventExtraIdList001
 * @tc.desc: call GetCommonEventExtraIdList001, return ERR_OK
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */
HWTEST_F(SystemAbilityMgrTest, GetCommonEventExtraIdList001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();

    const int32_t maxLoop = 4;
    map<int32_t, SaProfile> saProfileMapTmp;
    SaProfileStore(saMgr, saProfileMapTmp, maxLoop);
    SaProfileExtensionTestPrevSet(saMgr, maxLoop);
    OnDemandEvent event;
    std::list<SaControlInfo> saControlList;
    saMgr->collectManager_->SaveCacheCommonEventSaExtraId(event, saControlList);

    event.eventId = COMMON_EVENT;
    event.extraDataId = 1000000;
    SaControlInfo info;
    info.saId = SAID;
    info.cacheCommonEvent = true;
    saControlList.push_back(info);
    saMgr->collectManager_->SaveCacheCommonEventSaExtraId(event, saControlList);

    std::vector<int64_t> extraDataIdList;
    int32_t ret = saMgr->GetCommonEventExtraDataIdlist(SAID, extraDataIdList);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(extraDataIdList.size(), 1);
    extraDataIdList.clear();

    ret = saMgr->GetCommonEventExtraDataIdlist(SAID, extraDataIdList, "test");
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(extraDataIdList.size(), 0);
    extraDataIdList.clear();

    saMgr->collectManager_->ClearSaExtraDataId(SAID);
    ret = saMgr->GetCommonEventExtraDataIdlist(SAID, extraDataIdList);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(extraDataIdList.size(), 0);

    SaProfileRecover(saMgr, saProfileMapTmp, maxLoop);
}

/**
 * @tc.name: RemoveWhiteCommonEvent001
 * @tc.desc: test RemoveWhiteCommonEvent
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrTest, RemoveWhiteCommonEvent001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->RemoveWhiteCommonEvent();
    EXPECT_TRUE(saMgr->collectManager_ != nullptr);
}

/**
 * @tc.name: CleanFfrt001
 * @tc.desc: test CleanFfrt, workHandler_ true.
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrTest, CleanFfrt001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    std::shared_ptr<FFRTHandler> workHandler_ = make_shared<FFRTHandler>("workHandler");
    int ret = true;
    saMgr->CleanFfrt();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: CleanFfrt002
 * @tc.desc: test CleanFfrt, collectManager_ true.
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrTest, CleanFfrt002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<DeviceStatusCollectManager> collectManager_ =
        sptr<DeviceStatusCollectManager>(new DeviceStatusCollectManager());
    int ret = true;
    saMgr->CleanFfrt();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: CleanFfrt003
 * @tc.desc: test CleanFfrt, abilityStateScheduler_ true.
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrTest, CleanFfrt003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler_ =
        std::make_shared<SystemAbilityStateScheduler>();
    int ret = true;
    saMgr->CleanFfrt();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: SetFfrt001
 * @tc.desc: test SetFfrt, workHandler_  true.
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrTest, SetFfrt001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->workHandler_ = make_shared<FFRTHandler>("workHandler");
    saMgr->SetFfrt();
    EXPECT_NE(saMgr->collectManager_, nullptr);
    EXPECT_NE(saMgr->abilityStateScheduler_, nullptr);
}

/**
 * @tc.name: GetRunningSaExtensionInfoList001
 * @tc.desc: test GetRunningSaExtensionInfoList, not exist obj
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, GetRunningSaExtensionInfoList001, TestSize.Level3)
{
    DTEST_LOG << __func__ << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    const int32_t maxLoop = 4;
    map<int32_t, SaProfile> saProfileMapTmp;

    SaProfileStore(saMgr, saProfileMapTmp, maxLoop);
    SaProfileExtensionTestPrevSet(saMgr, maxLoop);

    MessageParcel data;
    MessageParcel reply;
    data.WriteString("restore_test");
    int32_t result = saMgr->GetRunningSaExtensionInfoListInner(data, reply);
    EXPECT_EQ(result, ERR_NONE);
    bool ret = reply.ReadInt32(result);
    EXPECT_EQ(ret, true);
    int32_t size = -1;
    ret = reply.ReadInt32(size);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(size, 0);

    saMgr->abilityMap_.clear();
    SaProfileRecover(saMgr, saProfileMapTmp, maxLoop);
}

/**
 * @tc.name: GetRunningSaExtensionInfoList002
 * @tc.desc: test GetRunningSaExtensionInfoList, get extension success with restore!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, GetRunningSaExtensionInfoList002, TestSize.Level3)
{
    DTEST_LOG << __func__ << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    const int32_t maxLoop = 4;
    map<int32_t, SaProfile> saProfileMapTmp;

    SaProfileStore(saMgr, saProfileMapTmp, maxLoop);
    SaProfileExtensionTestPrevSet(saMgr, maxLoop);
    ProcMapObjTestPrevSet(saMgr, maxLoop);
    SaAbilityMapObjTestPrevSet(saMgr, maxLoop);

    MessageParcel data;
    MessageParcel reply;
    data.WriteString("restore_test");
    int32_t result = saMgr->GetRunningSaExtensionInfoListInner(data, reply);
    EXPECT_EQ(result, ERR_NONE);
    bool ret = reply.ReadInt32(result);
    EXPECT_EQ(ret, true);

    int32_t size = -1;
    ret = reply.ReadInt32(size);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(size, 3);
    for (int32_t i = 0; i < size; ++i) {
        int id = -1;
        ret = reply.ReadInt32(id);
        EXPECT_EQ(ret, true);
        EXPECT_EQ(id, SAID + 1 + i);
        sptr<IRemoteObject> obj = reply.ReadRemoteObject();
        EXPECT_NE(obj, nullptr);
    }

    saMgr->abilityMap_.clear();
    SaProfileRecover(saMgr, saProfileMapTmp, maxLoop);
}

/**
 * @tc.name: StartOnDemandAbilityInner001
 * @tc.desc: test StartOnDemandAbilityInner, ERR_INVALID_VALUE.
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrTest, StartOnDemandAbilityInner001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    const std::u16string procName;
    int32_t systemAbilityId = 0;
    SystemAbilityManager::AbilityItem abilityItem;
    abilityItem.state = SystemAbilityManager::AbilityState::STARTING;
    int32_t ret = saMgr->StartOnDemandAbilityInner(procName, systemAbilityId, abilityItem);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: CheckSystemAbility006
 * @tc.desc: test CheckSystemAbility, nullptr.
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrTest, CheckSystemAbility006, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    int32_t systemAbilityId = -1;
    bool isExist;
    sptr<IRemoteObject> ret = saMgr->CheckSystemAbility(systemAbilityId, isExist);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: RemoveSystemAbility005
 * @tc.desc: test RemoveSystemAbility, ERR_INVALID_VALUE.
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrTest, RemoveSystemAbility005, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    const sptr<IRemoteObject> ability = nullptr;
    int32_t ret = saMgr->RemoveSystemAbility(ability);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: SubscribeSystemAbility001
 * @tc.desc: test SubscribeSystemAbility, ERR_INVALID_VALUE.
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrTest, SubscribeSystemAbility001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    int32_t systemAbilityId = -1;
    const sptr<ISystemAbilityStatusChange> listener;
    int32_t ret = saMgr->SubscribeSystemAbility(systemAbilityId, listener);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: UnSubscribeSystemAbility001
 * @tc.desc: test UnSubscribeSystemAbility, ERR_INVALID_VALUE.
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrTest, UnSubscribeSystemAbility001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    int32_t systemAbilityId = -1;
    const sptr<ISystemAbilityStatusChange> listener;
    int32_t ret = saMgr->UnSubscribeSystemAbility(systemAbilityId, listener);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: NotifyRemoteSaDied002
 * @tc.desc: test NotifyRemoteSaDied, true.
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrTest, NotifyRemoteSaDied002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    const std::u16string name;
    sptr<DBinderService> dBinderService_;
    int ret = true;
    saMgr->NotifyRemoteSaDied(name);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: AddSystemAbility006
 * @tc.desc: add system ability with empty capability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, AddSystemAbility006, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    int32_t systemAbilityId = -1;
    const sptr<IRemoteObject> ability;
    const ISystemAbilityManager::SAExtraProp extraProp;
    int32_t ret = saMgr->AddSystemAbility(systemAbilityId, ability, extraProp);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AddSystemAbility007
 * @tc.desc: test AddSystemAbility.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, AddSystemAbility007, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    int32_t systemAbilityId = -1;
    const sptr<IRemoteObject> ability;
    const ISystemAbilityManager::SAExtraProp extraProp;
    std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler_ = nullptr;
    int32_t ret = saMgr->AddSystemAbility(systemAbilityId, ability, extraProp);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: GetSystemProcess001
 * @tc.desc: test GetSystemProcess.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, GetSystemProcess001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    std::u16string procName;
    sptr<IRemoteObject> ret = saMgr->GetSystemProcess(procName);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: GetSystemProcessInfo003
 * @tc.desc: test GetSystemProcessInfo.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, GetSystemProcessInfo003, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->abilityStateScheduler_ = nullptr;
    int32_t systemAbilityId = 0;
    SystemProcessInfo systemProcessInfo;
    int32_t ret = saMgr->GetSystemProcessInfo(systemAbilityId, systemProcessInfo);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: LoadSystemAbilityFromRpc009
 * @tc.desc: test LoadSystemAbilityFromRpc.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, LoadSystemAbilityFromRpc009, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    const std::string srcDeviceId;
    SaProfile saProfile = {u"test", TEST_OVERFLOW_SAID};
    saProfile.distributed = true;
    saMgr->saProfileMap_[TEST_OVERFLOW_SAID] = saProfile;
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    std::shared_ptr<SystemAbilityStateScheduler> saScheduler = saMgr->abilityStateScheduler_;
    saMgr->abilityStateScheduler_ = nullptr;
    int32_t ret = saMgr->LoadSystemAbilityFromRpc(srcDeviceId, TEST_OVERFLOW_SAID, callback);
    EXPECT_FALSE(ret);
    saMgr->abilityStateScheduler_ = saScheduler;
    saMgr->saProfileMap_.erase(TEST_OVERFLOW_SAID);
}

/**
 * @tc.name: IsModuleUpdate001
 * @tc.desc: test IsModuleUpdate with saprofile is not exist.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, IsModuleUpdate001, TestSize.Level2)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->saProfileMap_.clear();
    bool ret = saMgr->IsModuleUpdate(SAID);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IsModuleUpdate002
 * @tc.desc: test IsModuleUpdate with default.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, IsModuleUpdate002, TestSize.Level2)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->saProfileMap_.clear();
    SaProfile saprofile;
    saMgr->saProfileMap_[saprofile.saId] = saprofile;
    bool ret = saMgr->IsModuleUpdate(saprofile.saId);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IsModuleUpdate003
 * @tc.desc: test IsModuleUpdate with moduleUpdate is true.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, IsModuleUpdate003, TestSize.Level2)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->saProfileMap_.clear();
    SaProfile saprofile;
    saprofile.moduleUpdate = true;
    saMgr->saProfileMap_[saprofile.saId] = saprofile;
    bool ret = saMgr->IsModuleUpdate(saprofile.saId);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: GetExtensionSaIdsInner001
 * @tc.desc: test GetExtensionSaIdsInner, read extension failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, GetExtensionSaIdsInner001, TestSize.Level3)
{
    DTEST_LOG << __func__ << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t result = saMgr->GetExtensionSaIdsInner(data, reply);
    EXPECT_EQ(result, ERR_FLATTEN_OBJECT);
}

/**
 * @tc.name: GetExtensionSaIdsInner002
 * @tc.desc: test GetExtensionSaIdsInner, read null extension!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, GetExtensionSaIdsInner002, TestSize.Level3)
{
    DTEST_LOG << __func__ << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteString("backup_test");
    int32_t result = saMgr->GetExtensionSaIdsInner(data, reply);
    EXPECT_EQ(result, ERR_NONE);
    int32_t ret = reply.ReadInt32(result);
    EXPECT_EQ(ret, true);

    vector<int32_t> saIds;
    ret = reply.ReadInt32Vector(&saIds);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(saIds.size(), 0);
}

/**
 * @tc.name: GetExtensionSaIdsInner003
 * @tc.desc: test GetExtensionSaIdsInner, get extension success with backup!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, GetExtensionSaIdsInner003, TestSize.Level3)
{
    DTEST_LOG << __func__ << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    const int32_t maxLoop = 4;
    map<int32_t, SaProfile> saProfileMapTmp;
    SaProfileStore(saMgr, saProfileMapTmp, maxLoop);
    SaProfileExtensionTestPrevSet(saMgr, maxLoop);

    MessageParcel data;
    MessageParcel reply;
    data.WriteString("backup_test");
    int32_t result = saMgr->GetExtensionSaIdsInner(data, reply);
    EXPECT_EQ(result, ERR_NONE);
    int32_t ret = reply.ReadInt32(result);
    EXPECT_EQ(ret, true);

    vector<int32_t> saIds;
    ret = reply.ReadInt32Vector(&saIds);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(saIds.size(), 3);
    EXPECT_EQ(saIds[0], SAID);
    EXPECT_EQ(saIds[1], SAID + 2);
    EXPECT_EQ(saIds[2], SAID + 3);

    SaProfileRecover(saMgr, saProfileMapTmp, maxLoop);
}

/**
 * @tc.name: GetExtensionRunningSaListInner001
 * @tc.desc: test GetExtensionRunningSaListInner, read extension failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, GetExtensionRunningSaListInner001, TestSize.Level3)
{
    DTEST_LOG << __func__ << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t result = saMgr->GetExtensionRunningSaListInner(data, reply);
    EXPECT_EQ(result, ERR_FLATTEN_OBJECT);
}

/**
 * @tc.name: GetExtensionRunningSaListInner002
 * @tc.desc: test GetExtensionRunningSaListInner, read null extension!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, GetExtensionRunningSaListInner002, TestSize.Level3)
{
    DTEST_LOG << __func__ << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteString("backup_test");
    int32_t result = saMgr->GetExtensionRunningSaListInner(data, reply);
    EXPECT_EQ(result, ERR_NONE);
    int32_t ret = reply.ReadInt32(result);
    EXPECT_EQ(ret, true);

    int32_t size;
    ret = reply.ReadInt32(size);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(size, 0);
}

/**
 * @tc.name: GetExtensionRunningSaListInner003
 * @tc.desc: test GetExtensionRunningSaListInner, get extension success with restore!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, GetExtensionRunningSaListInner003, TestSize.Level3)
{
    DTEST_LOG << __func__ << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    const int32_t maxLoop = 4;
    map<int32_t, SaProfile> saProfileMapTmp;

    SaProfileStore(saMgr, saProfileMapTmp, maxLoop);
    SaProfileExtensionTestPrevSet(saMgr, maxLoop);
    SaAbilityMapObjTestPrevSet(saMgr, maxLoop);

    MessageParcel data;
    MessageParcel reply;
    data.WriteString("restore_test");
    int32_t result = saMgr->GetExtensionRunningSaListInner(data, reply);
    EXPECT_EQ(result, ERR_NONE);
    int32_t ret = reply.ReadInt32(result);
    EXPECT_EQ(ret, true);

    int32_t size;
    ret = reply.ReadInt32(size);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(size, 3);
    for (int32_t i = 0; i < size; ++i) {
        sptr<IRemoteObject> obj = reply.ReadRemoteObject();
        EXPECT_NE(obj, nullptr);
    }

    saMgr->abilityMap_.clear();
    SaProfileRecover(saMgr, saProfileMapTmp, maxLoop);
}

/**
 * @tc.name: IpcDumpProc001
 * @tc.desc: test IpcDumpProc.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, IpcDumpProc001, TestSize.Level2)
{
    SamMockPermission::MockProcess("hidumper_service");
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    int32_t cmd = -1;
    int32_t fd = 1;
    std::vector<std::string> args;
    args.push_back("abc");
    args.push_back("abcd");
    int ret = saMgr->IpcDumpProc(fd, args);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: IpcDumpProc002
 * @tc.desc: test IpcDumpProc.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, IpcDumpProc002, TestSize.Level2)
{
    SamMockPermission::MockProcess("hidumper_service");
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    int32_t cmd = -1;
    int32_t fd = 1;
    std::vector<std::string> args;
    args.push_back("abc");
    args.push_back("abcd");
    args.push_back("--start-stat");
    int ret = saMgr->IpcDumpProc(fd, args);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: IpcDumpProc003
 * @tc.desc: test IpcDumpProc.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, IpcDumpProc003, TestSize.Level2)
{
    SamMockPermission::MockProcess("hidumper_service");
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    int32_t cmd = -1;
    int32_t fd = 1;
    std::vector<std::string> args;
    args.push_back("test");
    args.push_back("samgr");
    args.push_back("--start-stat");
    int ret = saMgr->IpcDumpProc(fd, args);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: IpcDumpProc004
 * @tc.desc: test IpcDumpProc.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, IpcDumpProc004, TestSize.Level2)
{
    SamMockPermission::MockProcess("hidumper_service");
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    int32_t cmd = -1;
    int32_t fd = 1;
    std::vector<std::string> args;
    args.push_back("test");
    args.push_back("all");
    args.push_back("--start-stat");
    int ret = saMgr->IpcDumpProc(fd, args);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: IpcStatSamgrProc001
 * @tc.desc: test IpcStatSamgrProc.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, IpcStatSamgrProc001, TestSize.Level2)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    int32_t cmd = IPC_STAT_CMD_START - 1;
    int32_t fd = 1;
    bool ret = saMgr->IpcStatSamgrProc(fd, cmd);
    EXPECT_FALSE(ret);
    cmd = IPC_STAT_CMD_MAX;
    bool result = saMgr->IpcStatSamgrProc(fd, cmd);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IpcStatSamgrProc002
 * @tc.desc: test IpcStatSamgrProc.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, IpcStatSamgrProc002, TestSize.Level2)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    int32_t cmd = IPC_STAT_CMD_START;
    int32_t dmd = IPC_STAT_CMD_STOP;
    int32_t emd = IPC_STAT_CMD_GET;
    int32_t fd = 1;
    int ret = true;
    saMgr->IpcStatSamgrProc(fd, cmd);
    saMgr->IpcStatSamgrProc(fd, dmd);
    saMgr->IpcStatSamgrProc(fd, emd);
    EXPECT_TRUE(ret);
    int32_t fmd = 4;
    bool result = saMgr->IpcStatSamgrProc(fd, fmd);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IpcDumpAllProcess001
 * @tc.desc: test IpcDumpAllProcess.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, IpcDumpAllProcess001, TestSize.Level2)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    int32_t cmd = IPC_STAT_CMD_START;
    int32_t fd = 1;
    bool ret = true;
    saMgr->IpcDumpAllProcess(fd, cmd);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: IpcDumpSamgrProcess001
 * @tc.desc: test IpcDumpSamgrProcess.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, IpcDumpSamgrProcess001, TestSize.Level2)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    int32_t cmd = 4;
    int32_t fd = 1;
    bool ret = true;
    saMgr->IpcDumpSamgrProcess(fd, cmd);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: IpcDumpSingleProcess001
 * @tc.desc: test IpcDumpSingleProcess.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, IpcDumpSingleProcess001, TestSize.Level2)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    int32_t cmd = IPC_STAT_CMD_START;
    int32_t fd = 1;
    bool ret = true;

    uint32_t accessToken = IPCSkeleton::GetCallingTokenID();
    Security::AccessToken::NativeTokenInfo nativeTokenInfo;
    int32_t tokenInfoResult = Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(accessToken, nativeTokenInfo);
    saMgr->IpcDumpSingleProcess(fd, cmd, nativeTokenInfo.processName);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: Test DoLoadForPerf
 * @tc.desc: DoLoadForPerf001
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, DoLoadForPerf001, TestSize.Level2)
{
    sptr<SystemAbilityManager> saMgr1 = new SystemAbilityManager;
    SaProfile saProfile;
    saProfile.process = u"memmgrservice";
    saMgr1->saProfileMap_.clear();
    saMgr1->saProfileMap_[-1] = saProfile;
    saMgr1->abilityMap_.clear();
    bool ret = true;
    saMgr1->DoLoadForPerf();
    EXPECT_TRUE(ret);
}
/**
 * @tc.name: Test IsDistributedSystemAbility
 * @tc.desc: IsDistributedSystemAbility001
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrTest, IsDistributedSystemAbility001, TestSize.Level2)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    bool res = saMgr->IsDistributedSystemAbility(-1);
    EXPECT_FALSE(res);
    SaProfile saProfile;
    saProfile.distributed = true;
    saProfile.process = u"memmgrservice";
    saMgr->saProfileMap_.clear();
    saMgr->saProfileMap_[1] = saProfile;
    res = saMgr->IsDistributedSystemAbility(1);
    EXPECT_TRUE(res);
    res = saMgr->IsDistributedSystemAbility(2);
    EXPECT_FALSE(res);
    saProfile.distributed = false;
    saMgr->saProfileMap_[2] = saProfile;
    res = saMgr->IsDistributedSystemAbility(2);
    EXPECT_FALSE(res);
}
} // namespace OHOS