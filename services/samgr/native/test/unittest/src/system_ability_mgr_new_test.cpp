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

#include "system_ability_mgr_new_test.h"
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
#ifdef SUPPORT_ACCESS_TOKEN
#include "accesstoken_kit.h"
#endif
#include "ability_death_recipient.h"
#endif

using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS;

namespace OHOS {
const int32_t softBusServerSaID = 4700;
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
constexpr int32_t TEST_OVERFLOW_SAID = 99999;
constexpr int32_t TEST_EXCEPTION_LOW_SA_ID = -1;
constexpr int32_t TEST_SYSTEM_ABILITY1 = 1491;

void SaProfileStore(sptr<SystemAbilityManager>& saMgr,
    map<int32_t, CommonSaProfile>& saProfileMapTmp, int32_t maxLoop)
{
    for (int32_t loop = 0; loop < maxLoop; ++loop) {
        if (saMgr->saProfileMap_.count(SAID + loop) > 0) {
            saProfileMapTmp[SAID + loop] = saMgr->saProfileMap_[SAID + loop];
        }
    }
    return;
}

void SaProfileRecover(sptr<SystemAbilityManager>& saMgr,
    map<int32_t, CommonSaProfile> saProfileMapTmp, int32_t maxLoop)
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
        CommonSaProfile saProfile;
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

void SystemAbilityMgrNewTest::SetUpTestCase()
{
    DTEST_LOG << "SetUpTestCase" << std::endl;
}

void SystemAbilityMgrNewTest::TearDownTestCase()
{
    DTEST_LOG << "TearDownTestCase" << std::endl;
}

void SystemAbilityMgrNewTest::SetUp()
{
    SamMockPermission::MockPermission();
    DTEST_LOG << "SetUp" << std::endl;
}

void SystemAbilityMgrNewTest::TearDown()
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->CleanFfrt();
    DTEST_LOG << "TearDown" << std::endl;
}

/**
 * @tc.name: GetLocalNodeId001
 * @tc.desc: test GetLocalNodeId
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrNewTest, GetLocalNodeId001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);

    string ret = saMgr->GetLocalNodeId();
    EXPECT_EQ(ret, "");
}

/**
 * @tc.name: ReportGetSAPeriodically001
 * @tc.desc: test ReportGetSAPeriodically with saFrequencyMap_ is not empty
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrNewTest, ReportGetSAPeriodically001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);

    uint64_t pid_said = 123;
    int32_t count = 1;
    saMgr->saFrequencyMap_[pid_said] = count;
    saMgr->ReportGetSAPeriodically();
    EXPECT_TRUE(saMgr->saFrequencyMap_.empty());
}

/**
 * @tc.name: StartDynamicSystemProcess
 * @tc.desc: start invalid process, return fail.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrNewTest, StartDynamicSystemProcess001, TestSize.Level3)
{
    cout << "begin StartDynamicSystemProcess001 "<< endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);

    std::u16string invalidProcess = u"1234567890123456789012345678901234567890123456789"
        "01234567890123456789012345678901234567890123456";
    OnDemandEvent event;
    int result = saMgr->StartDynamicSystemProcess(invalidProcess, 100, event);
    cout << "begin StartDynamicSystemProcess001 result is "<< result << endl;
    EXPECT_EQ(result, 102);
}

/**
 * @tc.name: GetSystemAbilityWithDevice001
 * @tc.desc: get invalid system ability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrNewTest, GetSystemAbilityWithDevice001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);

    std::string deviceId = "";
    auto ability = saMgr->GetSystemAbility(TEST_EXCEPTION_LOW_SA_ID, deviceId);
    EXPECT_EQ(ability, nullptr);

    CommonSaProfile saProfile = {u"test", TEST_OVERFLOW_SAID};
    saMgr->saProfileMap_[TEST_OVERFLOW_SAID] = saProfile;
    ability = saMgr->GetSystemAbility(TEST_OVERFLOW_SAID, deviceId);
    EXPECT_EQ(ability, nullptr);

    saMgr->saProfileMap_[TEST_OVERFLOW_SAID].distributed = true;
    ability = saMgr->GetSystemAbility(TEST_OVERFLOW_SAID, deviceId);
    EXPECT_EQ(ability, nullptr);
}

/**
 * @tc.name: GetSystemAbilityFromRemote001
 * @tc.desc: get invalid system ability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrNewTest, GetSystemAbilityFromRemote001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    saMgr->Init();
    auto ability = saMgr->GetSystemAbilityFromRemote(TEST_EXCEPTION_LOW_SA_ID);
    EXPECT_EQ(ability, nullptr);
}

/**
 * @tc.name: GetSystemAbilityFromRemote002
 * @tc.desc: get not exist system ability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrNewTest, GetSystemAbilityFromRemote002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);

    auto ability = saMgr->GetSystemAbilityFromRemote(TEST_SYSTEM_ABILITY1);
    EXPECT_EQ(ability, nullptr);
}

/**
 * @tc.name: GetSystemAbilityFromRemote003
 * @tc.desc: get exist system ability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrNewTest, GetSystemAbilityFromRemote003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);
    SAInfo saInfo;
    saMgr->abilityMap_[1] = saInfo;
    int systemAbilityId = 1;
    auto ability = saMgr->GetSystemAbilityFromRemote(systemAbilityId);
    EXPECT_EQ(ability, nullptr);
}

/**
 * @tc.name: GetSystemAbilityFromRemote004
 * @tc.desc: get exist system ability, isDistributed is true.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrNewTest, GetSystemAbilityFromRemote004, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);
    SAInfo saInfo;
    saInfo.isDistributed = true;
    saMgr->abilityMap_[1] = saInfo;
    int systemAbilityId = 1;
    auto ability = saMgr->GetSystemAbilityFromRemote(systemAbilityId);
    EXPECT_EQ(ability, nullptr);
}

/**
 * @tc.name: GetDBinder001
 * @tc.desc: GetDBinder, return null
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityMgrNewTest, GetDBinder001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);
    sptr<DBinderService> result = saMgr->GetDBinder();
    EXPECT_TRUE(result == nullptr);
}

/**
 * @tc.name: NotifyRpcLoadCompleted001
 * @tc.desc: test NotifyRpcLoadCompleted, workHandler_ is nullptr
 * @tc.type: FUNC
 * @tc.require: I6MO6A
 */

HWTEST_F(SystemAbilityMgrNewTest, NotifyRpcLoadCompleted001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);
#ifdef SAMGR_ENABLE_DELAY_DBINDER
    saMgr->InitDbinderService();
#endif
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    saMgr->workHandler_ = nullptr;
    saMgr->NotifyRpcLoadCompleted("", 1, testAbility);
    EXPECT_TRUE(saMgr != nullptr);
}

/**
 * @tc.name: NotifyRpcLoadCompleted002
 * @tc.desc: test NotifyRpcLoadCompleted, dBinderService_ is null
 * @tc.type: FUNC
 * @tc.require: I6MO6A
 */

HWTEST_F(SystemAbilityMgrNewTest, NotifyRpcLoadCompleted002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    saMgr->NotifyRpcLoadCompleted("", 1, testAbility);
}

/**
 * @tc.name: NotifyRpcLoadCompleted003
 * @tc.desc: test NotifyRpcLoadCompleted
 * @tc.type: FUNC
 * @tc.require: I6MO6A
 */
HWTEST_F(SystemAbilityMgrNewTest, NotifyRpcLoadCompleted003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);
#ifdef SAMGR_ENABLE_DELAY_DBINDER
    saMgr->InitDbinderService();
#endif
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    saMgr->NotifyRpcLoadCompleted("", 1, testAbility);
    EXPECT_TRUE(saMgr != nullptr);
}

/**
 * @tc.name: Dump001
 * @tc.desc: call Dump, return ERR_OK
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityMgrNewTest, Dump001, TestSize.Level3)
{
    SamMockPermission::MockProcess("hidumper_service");
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);
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

HWTEST_F(SystemAbilityMgrNewTest, Dump002, TestSize.Level3)
{
    SamMockPermission::MockProcess("hidumper_service");
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);
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

HWTEST_F(SystemAbilityMgrNewTest, Dump003, TestSize.Level3)
{
    SamMockPermission::MockProcess("hidumper_service");
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);
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

HWTEST_F(SystemAbilityMgrNewTest, Dump004, TestSize.Level3)
{
    SamMockPermission::MockProcess("hidumper_service");
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);
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

HWTEST_F(SystemAbilityMgrNewTest, AddSamgrToAbilityMap001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);
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
HWTEST_F(SystemAbilityMgrNewTest, GetCommonEventExtraIdList001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);
    sptr<CommonEventCollect> commonEventCollect = new CommonEventCollect(saMgr->collectManager_);
    commonEventCollect->workHandler_ = std::make_shared<CommonHandler>(commonEventCollect);
    saMgr->collectManager_->collectPluginMap_[COMMON_EVENT] = commonEventCollect;

    const int32_t maxLoop = 4;
    map<int32_t, CommonSaProfile> saProfileMapTmp;
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
}

/**
 * @tc.name: RemoveWhiteCommonEvent001
 * @tc.desc: test RemoveWhiteCommonEvent
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrNewTest, RemoveWhiteCommonEvent001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);
    saMgr->workHandler_ = make_shared<FFRTHandler>("workHandler");
    saMgr->CleanFfrt();
    saMgr->collectManager_ = sptr<DeviceStatusCollectManager>(new DeviceStatusCollectManager());
    saMgr->CleanFfrt();
    saMgr->abilityStateScheduler_ = std::make_shared<SystemAbilityStateScheduler>();
    saMgr->CleanFfrt();
    const std::u16string name;
    saMgr->NotifyRemoteSaDied(name);
    saMgr->RemoveWhiteCommonEvent();
    EXPECT_TRUE(saMgr->collectManager_ != nullptr);
}

/**
 * @tc.name: SetFfrt001
 * @tc.desc: test SetFfrt, workHandler_  true.
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrNewTest, SetFfrt001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);
    saMgr->SetFfrt();
    EXPECT_NE(saMgr->collectManager_, nullptr);
    EXPECT_NE(saMgr->abilityStateScheduler_, nullptr);
}

/**
 * @tc.name: GetRunningSaExtensionInfoList001
 * @tc.desc: test GetRunningSaExtensionInfoList, not exist obj
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrNewTest, GetRunningSaExtensionInfoList001, TestSize.Level3)
{
    DTEST_LOG << __func__ << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);
    const int32_t maxLoop = 4;
    map<int32_t, CommonSaProfile> saProfileMapTmp;

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
}

/**
 * @tc.name: GetRunningSaExtensionInfoList002
 * @tc.desc: test GetRunningSaExtensionInfoList, get extension success with restore!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrNewTest, GetRunningSaExtensionInfoList002, TestSize.Level3)
{
    DTEST_LOG << __func__ << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);
    const int32_t maxLoop = 4;
    map<int32_t, CommonSaProfile> saProfileMapTmp;

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
}

/**
 * @tc.name: CheckSystemAbility006
 * @tc.desc: test CheckSystemAbility, nullptr.
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrNewTest, CheckSystemAbility006, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);
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
HWTEST_F(SystemAbilityMgrNewTest, RemoveSystemAbility005, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);
    const sptr<IRemoteObject> ability = nullptr;
    int32_t ret = saMgr->RemoveSystemAbility(ability);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AddSystemAbility006
 * @tc.desc: add system ability with empty capability.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrNewTest, AddSystemAbility006, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);
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
HWTEST_F(SystemAbilityMgrNewTest, AddSystemAbility007, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);
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
HWTEST_F(SystemAbilityMgrNewTest, GetSystemProcess001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);
    std::u16string procName;
    sptr<IRemoteObject> ret = saMgr->GetSystemProcess(procName);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: GetSystemProcessInfo003
 * @tc.desc: test GetSystemProcessInfo.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrNewTest, GetSystemProcessInfo003, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    saMgr->abilityStateScheduler_ = nullptr;
    int32_t systemAbilityId = 0;
    SystemProcessInfo systemProcessInfo;
    int32_t ret = saMgr->GetSystemProcessInfo(systemAbilityId, systemProcessInfo);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: IsModuleUpdate001
 * @tc.desc: test IsModuleUpdate with saprofile is not exist.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrNewTest, IsModuleUpdate001, TestSize.Level2)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);
    saMgr->saProfileMap_.clear();
    bool ret = saMgr->IsModuleUpdate(SAID);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IsModuleUpdate002
 * @tc.desc: test IsModuleUpdate with default.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrNewTest, IsModuleUpdate002, TestSize.Level2)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);
    saMgr->saProfileMap_.clear();
    CommonSaProfile saprofile;
    saMgr->saProfileMap_[saprofile.saId] = saprofile;
    bool ret = saMgr->IsModuleUpdate(saprofile.saId);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IsModuleUpdate003
 * @tc.desc: test IsModuleUpdate with moduleUpdate is true.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrNewTest, IsModuleUpdate003, TestSize.Level2)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);
    saMgr->saProfileMap_.clear();
    CommonSaProfile saprofile;
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
HWTEST_F(SystemAbilityMgrNewTest, GetExtensionSaIdsInner001, TestSize.Level3)
{
    DTEST_LOG << __func__ << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);
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
HWTEST_F(SystemAbilityMgrNewTest, GetExtensionSaIdsInner002, TestSize.Level3)
{
    DTEST_LOG << __func__ << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);
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
HWTEST_F(SystemAbilityMgrNewTest, GetExtensionSaIdsInner003, TestSize.Level3)
{
    DTEST_LOG << __func__ << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);
    const int32_t maxLoop = 4;
    map<int32_t, CommonSaProfile> saProfileMapTmp;
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
}

/**
 * @tc.name: GetExtensionRunningSaListInner001
 * @tc.desc: test GetExtensionRunningSaListInner, read extension failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrNewTest, GetExtensionRunningSaListInner001, TestSize.Level3)
{
    DTEST_LOG << __func__ << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);
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
HWTEST_F(SystemAbilityMgrNewTest, GetExtensionRunningSaListInner002, TestSize.Level3)
{
    DTEST_LOG << __func__ << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);
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
HWTEST_F(SystemAbilityMgrNewTest, GetExtensionRunningSaListInner003, TestSize.Level3)
{
    DTEST_LOG << __func__ << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);
    const int32_t maxLoop = 4;
    map<int32_t, CommonSaProfile> saProfileMapTmp;

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
}

/**
 * @tc.name: IpcDumpProc001
 * @tc.desc: test IpcDumpProc.
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrNewTest, IpcDumpProc001, TestSize.Level2)
{
    SamMockPermission::MockProcess("hidumper_service");
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);
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
HWTEST_F(SystemAbilityMgrNewTest, IpcDumpProc002, TestSize.Level2)
{
    SamMockPermission::MockProcess("hidumper_service");
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);
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
HWTEST_F(SystemAbilityMgrNewTest, IpcDumpProc003, TestSize.Level2)
{
    SamMockPermission::MockProcess("hidumper_service");
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);
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
HWTEST_F(SystemAbilityMgrNewTest, IpcDumpProc004, TestSize.Level2)
{
    SamMockPermission::MockProcess("hidumper_service");
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);
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
HWTEST_F(SystemAbilityMgrNewTest, IpcStatSamgrProc001, TestSize.Level2)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);
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
HWTEST_F(SystemAbilityMgrNewTest, IpcStatSamgrProc002, TestSize.Level2)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);
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
HWTEST_F(SystemAbilityMgrNewTest, IpcDumpAllProcess001, TestSize.Level2)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);
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
HWTEST_F(SystemAbilityMgrNewTest, IpcDumpSamgrProcess001, TestSize.Level2)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);
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
#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityMgrNewTest, IpcDumpSingleProcess001, TestSize.Level2)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);
    int32_t cmd = IPC_STAT_CMD_START;
    int32_t fd = 1;
    bool ret = true;

    uint32_t accessToken = IPCSkeleton::GetCallingTokenID();
    Security::AccessToken::NativeTokenInfo nativeTokenInfo;
    int32_t tokenInfoResult = Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(accessToken, nativeTokenInfo);
    saMgr->IpcDumpSingleProcess(fd, cmd, nativeTokenInfo.processName);
    EXPECT_TRUE(ret);
}
#endif

/**
 * @tc.name: Test DoLoadForPerf
 * @tc.desc: DoLoadForPerf001
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrNewTest, DoLoadForPerf001, TestSize.Level2)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);
    CommonSaProfile saProfile;
    saProfile.process = u"memmgrservice";
    saMgr->saProfileMap_[-1] = saProfile;
    bool ret = true;
    saMgr->DoLoadForPerf();
    EXPECT_TRUE(ret);
}
/**
 * @tc.name: Test IsDistributedSystemAbility
 * @tc.desc: IsDistributedSystemAbility001
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrNewTest, IsDistributedSystemAbility001, TestSize.Level2)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_TRUE(saMgr != nullptr);
    InitSaMgr(saMgr);
    bool res = saMgr->IsDistributedSystemAbility(-1);
    EXPECT_FALSE(res);
    CommonSaProfile saProfile;
    saProfile.distributed = true;
    saProfile.process = u"memmgrservice";
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

#ifdef SAMGR_ENABLE_DELAY_DBINDER
HWTEST_F(SystemAbilityMgrNewTest, RegisterDistribute001, TestSize.Level2)
{
    DTEST_LOG<<"RegisterDistribute001 BEGIN"<<std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager();
    EXPECT_FALSE(saMgr == nullptr);
    InitSaMgr(saMgr);
    saMgr->isDbinderServiceInit_ = false;
    saMgr->distributedSaList_.push_back(softBusServerSaID);
    saMgr->InitDbinderService();
    saMgr->RegisterDistribute(softBusServerSaID, true);
    EXPECT_FALSE(saMgr->dBinderService_ = nullptr);
    DTEST_LOG<<"RegisterDistribute001 END"<<std::endl;
}
#endif

} // namespace OHOS