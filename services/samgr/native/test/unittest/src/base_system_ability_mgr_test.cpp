/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "base_system_ability_mgr_test.h"
#include "ability_death_recipient.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "if_local_ability_manager.h"
#include "itest_transaction_service.h"
#include "sa_profiles.h"
#include "sa_status_change_mock.h"
#include "sam_mock_permission.h"
#include "samgr_err_code.h"
#include "string_ex.h"
#include "system_ability_definition.h"
#include "test_log.h"
#define private public
#define protected public
#include "ipc_skeleton.h"
#ifdef SUPPORT_ACCESS_TOKEN
#include "accesstoken_kit.h"
#endif
#include "system_ability_manager.h"
#include "system_ability_manager_util.h"
#include "tools.h"

using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS;

namespace OHOS {
namespace {
constexpr int32_t SAID = 1234;
constexpr int32_t OTHER_SAID = 1499;
constexpr int32_t TEST_EXCEPTION_HIGH_SA_ID = LAST_SYS_ABILITY_ID + 1;
const std::u16string PROCESS_NAME = u"test_process_name";
constexpr int32_t MAX_SUBSCRIBE_COUNT = 256;

class MockLocalAbilityManager : public IRemoteStub<ILocalAbilityManager> {
public:
    bool StartAbility(int32_t systemAbilityId, const std::string& eventStr) override { return true; }
    bool StopAbility(int32_t systemAbilityId, const std::string& eventStr) override { return true; }
    bool ActiveAbility(int32_t systemAbilityId, const nlohmann::json& activeReason) override { return true; }
    bool IdleAbility(int32_t systemAbilityId, const nlohmann::json& idleReason, int32_t& delayTime) override
    { return true; }
    bool SendStrategyToSA(int32_t type, int32_t systemAbilityId, int32_t level, std::string& action) override
    { return true; }
    bool IpcStatCmdProc(int32_t fd, int32_t cmd) override { return true; }
    bool FfrtStatCmdProc(int32_t fd, int32_t cmd) override { return true; }
    bool FfrtDumperProc(std::string& result) override { return true; }
    int32_t SystemAbilityExtProc(const std::string& extension, int32_t said,
        SystemAbilityExtensionPara* callback, bool isAsync) override { return ERR_OK; }
    int32_t ServiceControlCmd(int32_t fd, int32_t systemAbilityId,
        const std::vector<std::u16string>& args) override { return ERR_OK; }
};

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
} // namespace

void BaseSystemAbilityMgrTest::SetUpTestCase()
{
    DTEST_LOG << "SetUpTestCase" << std::endl;
}

void BaseSystemAbilityMgrTest::TearDownTestCase()
{
    DTEST_LOG << "TearDownTestCase" << std::endl;
}

void BaseSystemAbilityMgrTest::SetUp()
{
    SamMockPermission::MockPermission();
    DTEST_LOG << "SetUp" << std::endl;
}

void BaseSystemAbilityMgrTest::TearDown()
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->CleanFfrt();
    DTEST_LOG << "TearDown" << std::endl;
}

/**
 * @tc.name: RemoveSystemAbilityByObj001
 * @tc.desc: remove system ability by object, ability is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, RemoveSystemAbilityByObj001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    int32_t result = saMgr->RemoveSystemAbility(static_cast<sptr<IRemoteObject>>(nullptr));
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: RemoveSystemAbilityByObj002
 * @tc.desc: remove system ability by object, ability not in abilityMap
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, RemoveSystemAbilityByObj002, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    int32_t result = saMgr->RemoveSystemAbility(testAbility);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: RemoveSystemAbilityByObj003
 * @tc.desc: add SA then remove by object, verify abilityMap empty
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, RemoveSystemAbilityByObj003, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    CommonSaProfile saProfile;
    saProfile.process = u"test";
    saProfile.distributed = false;
    saProfile.saId = SAID;
    saMgr->saProfileMap_[SAID] = saProfile;
    ISystemAbilityManager::SAExtraProp extraProp(false, ISystemAbilityManager::DUMP_FLAG_PRIORITY_DEFAULT, u"", u"");
    int32_t addResult = saMgr->AddSystemAbility(SAID, testAbility, extraProp);
    EXPECT_EQ(addResult, ERR_OK);
    int32_t removeResult = saMgr->RemoveSystemAbility(testAbility);
    EXPECT_EQ(removeResult, ERR_OK);
    EXPECT_TRUE(saMgr->abilityMap_.find(SAID) == saMgr->abilityMap_.end());
    saMgr->saProfileMap_.erase(SAID);
}

/**
 * @tc.name: RemoveSystemAbilityByObj004
 * @tc.desc: add SA, set abilityDeath_=nullptr, remove by object
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, RemoveSystemAbilityByObj004, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    CommonSaProfile saProfile;
    saProfile.process = u"test";
    saProfile.distributed = false;
    saProfile.saId = SAID;
    saMgr->saProfileMap_[SAID] = saProfile;
    ISystemAbilityManager::SAExtraProp extraProp(false, ISystemAbilityManager::DUMP_FLAG_PRIORITY_DEFAULT, u"", u"");
    int32_t addResult = saMgr->AddSystemAbility(SAID, testAbility, extraProp);
    EXPECT_EQ(addResult, ERR_OK);
    saMgr->abilityDeath_ = nullptr;
    int32_t removeResult = saMgr->RemoveSystemAbility(testAbility);
    EXPECT_EQ(removeResult, ERR_OK);
    saMgr->saProfileMap_.erase(SAID);
}

/**
 * @tc.name: RemoveSystemAbilityByObj005
 * @tc.desc: add SA, set abilityStateScheduler_=nullptr, remove by object
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, RemoveSystemAbilityByObj005, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    CommonSaProfile saProfile;
    saProfile.process = u"test";
    saProfile.distributed = false;
    saProfile.saId = SAID;
    saMgr->saProfileMap_[SAID] = saProfile;
    ISystemAbilityManager::SAExtraProp extraProp(false, ISystemAbilityManager::DUMP_FLAG_PRIORITY_DEFAULT, u"", u"");
    int32_t addResult = saMgr->AddSystemAbility(SAID, testAbility, extraProp);
    EXPECT_EQ(addResult, ERR_OK);
    saMgr->abilityStateScheduler_ = nullptr;
    int32_t removeResult = saMgr->RemoveSystemAbility(testAbility);
    EXPECT_EQ(removeResult, ERR_INVALID_VALUE);
    saMgr->saProfileMap_.erase(SAID);
}

/**
 * @tc.name: RemoveDiedSystemAbility001
 * @tc.desc: remove died system ability, SA not in map
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, RemoveDiedSystemAbility001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    int32_t result = saMgr->RemoveDiedSystemAbility(SAID);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: RemoveDiedSystemAbility002
 * @tc.desc: add SA with null ability, remove died
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, RemoveDiedSystemAbility002, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    SAInfo saInfo;
    saInfo.remoteObj = nullptr;
    saInfo.isDistributed = false;
    saMgr->abilityMap_[SAID] = saInfo;
    int32_t result = saMgr->RemoveDiedSystemAbility(SAID);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_TRUE(saMgr->abilityMap_.find(SAID) == saMgr->abilityMap_.end());
}

/**
 * @tc.name: RemoveDiedSystemAbility003
 * @tc.desc: add SA with real ability, remove died
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, RemoveDiedSystemAbility003, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    SAInfo saInfo;
    saInfo.remoteObj = testAbility;
    saInfo.isDistributed = false;
    saMgr->abilityMap_[SAID] = saInfo;
    int32_t result = saMgr->RemoveDiedSystemAbility(SAID);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_TRUE(saMgr->abilityMap_.find(SAID) == saMgr->abilityMap_.end());
}

/**
 * @tc.name: SubscribeMaxCount001
 * @tc.desc: subscribe 256 times, 257th returns ERR_PERMISSION_DENIED
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, SubscribeMaxCount001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    for (int32_t i = 0; i < MAX_SUBSCRIBE_COUNT; i++) {
        sptr<ISystemAbilityStatusChange> listener = new SaStatusChangeMock();
        int32_t result = saMgr->SubscribeSystemAbility(SAID, listener);
        EXPECT_EQ(result, ERR_OK);
    }
    sptr<ISystemAbilityStatusChange> overflowListener = new SaStatusChangeMock();
    int32_t overflowResult = saMgr->SubscribeSystemAbility(SAID, overflowListener);
    EXPECT_EQ(overflowResult, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: SubscribeDeathNull001
 * @tc.desc: set abilityStatusDeath_=nullptr, subscribe still ERR_OK
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, SubscribeDeathNull001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    saMgr->abilityStatusDeath_ = nullptr;
    sptr<ISystemAbilityStatusChange> listener = new SaStatusChangeMock();
    int32_t result = saMgr->SubscribeSystemAbility(SAID, listener);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: UnSubscribeLockedNull001
 * @tc.desc: manually add listener with nullptr, call UnSubscribeSystemAbilityLocked
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, UnSubscribeLockedNull001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    SAListener nullListener(nullptr, 0);
    saMgr->listenerMap_[SAID].emplace_back(nullListener);
    size_t sizeBefore = saMgr->listenerMap_[SAID].size();
    sptr<ISystemAbilityStatusChange> testListener = new SaStatusChangeMock();
    saMgr->UnSubscribeSystemAbilityLocked(saMgr->listenerMap_[SAID], testListener->AsObject());
    EXPECT_EQ(saMgr->listenerMap_[SAID].size(), sizeBefore);
}

/**
 * @tc.name: UnSubscribeLockedNotFound001
 * @tc.desc: call UnSubscribeSystemAbilityLocked with listener not in list
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, UnSubscribeLockedNotFound001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    sptr<ISystemAbilityStatusChange> listener1 = new SaStatusChangeMock();
    saMgr->listenerMap_[SAID].emplace_back(listener1, 1);
    size_t sizeBefore = saMgr->listenerMap_[SAID].size();
    sptr<ISystemAbilityStatusChange> listener2 = new SaStatusChangeMock();
    saMgr->UnSubscribeSystemAbilityLocked(saMgr->listenerMap_[SAID], listener2->AsObject());
    EXPECT_EQ(saMgr->listenerMap_[SAID].size(), sizeBefore);
}

/**
 * @tc.name: UnSubscribeLockedCountZero001
 * @tc.desc: subscribe then unsubscribe same listener, subscribeCountMap_ entry erased
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, UnSubscribeLockedCountZero001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    sptr<ISystemAbilityStatusChange> listener = new SaStatusChangeMock();
    int32_t result = saMgr->SubscribeSystemAbility(SAID, listener);
    EXPECT_EQ(result, ERR_OK);
    int32_t callingPid = IPCSkeleton::GetCallingPid();
    EXPECT_TRUE(saMgr->subscribeCountMap_.find(callingPid) != saMgr->subscribeCountMap_.end());
    int32_t unsubResult = saMgr->UnSubscribeSystemAbility(SAID, listener);
    EXPECT_EQ(unsubResult, ERR_OK);
    EXPECT_TRUE(saMgr->subscribeCountMap_.find(callingPid) == saMgr->subscribeCountMap_.end());
}

/**
 * @tc.name: AddSystemProcessMax001
 * @tc.desc: pre-populate systemProcessMap_ with 1000 entries, add one more fails
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, AddSystemProcessMax001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    for (uint32_t i = 0; i < MAX_SERVICES; i++) {
        std::u16string name = u"proc_" + Str8ToStr16(std::to_string(i));
        sptr<IRemoteObject> obj = new TestTransactionService();
        saMgr->systemProcessMap_[name] = obj;
    }
    sptr<IRemoteObject> extraObj = new TestTransactionService();
    int32_t result = saMgr->AddSystemProcess(u"extra_process", extraObj);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AddSystemProcessDeathNull001
 * @tc.desc: set systemProcessDeath_=nullptr, add process returns ERR_OK
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, AddSystemProcessDeathNull001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    saMgr->systemProcessDeath_ = nullptr;
    sptr<IRemoteObject> procObject = new TestTransactionService();
    int32_t result = saMgr->AddSystemProcess(PROCESS_NAME, procObject);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: RemoveSystemProcessNull001
 * @tc.desc: remove system process, procObject is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, RemoveSystemProcessNull001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    int32_t result = saMgr->RemoveSystemProcess(nullptr);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: RemoveSystemProcessNotFound001
 * @tc.desc: add process then remove with different object
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, RemoveSystemProcessNotFound001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    sptr<IRemoteObject> procObject = new TestTransactionService();
    saMgr->systemProcessMap_[PROCESS_NAME] = procObject;
    sptr<IRemoteObject> differentObj = new TestTransactionService();
    int32_t result = saMgr->RemoveSystemProcess(differentObj);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: RemoveSystemProcessFound001
 * @tc.desc: add process then remove with same object
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, RemoveSystemProcessFound001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    sptr<IRemoteObject> procObject = new TestTransactionService();
    saMgr->systemProcessMap_[PROCESS_NAME] = procObject;
    int32_t result = saMgr->RemoveSystemProcess(procObject);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_TRUE(saMgr->systemProcessMap_.find(PROCESS_NAME) == saMgr->systemProcessMap_.end());
}

/**
 * @tc.name: AddSystemAbilityMax001
 * @tc.desc: pre-populate abilityMap_ with 1000 entries, add one more fails
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, AddSystemAbilityMax001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    for (uint32_t i = 0; i < MAX_SERVICES; i++) {
        int32_t saId = static_cast<int32_t>(i);
        SAInfo saInfo;
        saInfo.remoteObj = new TestTransactionService();
        saInfo.isDistributed = false;
        saMgr->abilityMap_[saId] = saInfo;
        CommonSaProfile saProfile;
        saProfile.process = u"test";
        saProfile.distributed = false;
        saProfile.saId = saId;
        saMgr->saProfileMap_[saId] = saProfile;
    }
    sptr<IRemoteObject> extraAbility = new TestTransactionService();
    ISystemAbilityManager::SAExtraProp extraProp(false, ISystemAbilityManager::DUMP_FLAG_PRIORITY_DEFAULT, u"", u"");
    int32_t result = saMgr->AddSystemAbility(OTHER_SAID, extraAbility, extraProp);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: CheckSystemAbilityBoolInvalid001
 * @tc.desc: check system ability with invalid SA id returns nullptr
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, CheckSystemAbilityBoolInvalid001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    bool isExist = false;
    sptr<IRemoteObject> result = saMgr->CheckSystemAbility(TEST_EXCEPTION_HIGH_SA_ID, isExist);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: CheckSystemAbilityBoolSchedulerNull001
 * @tc.desc: set abilityStateScheduler_=nullptr, valid SA id returns nullptr
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, CheckSystemAbilityBoolSchedulerNull001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    saMgr->abilityStateScheduler_ = nullptr;
    bool isExist = false;
    sptr<IRemoteObject> result = saMgr->CheckSystemAbility(SAID, isExist);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: CheckSystemAbilityBoolNotLoaded001
 * @tc.desc: valid SA not in abilityMap returns nullptr, isExist=false
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, CheckSystemAbilityBoolNotLoaded001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    bool isExist = false;
    sptr<IRemoteObject> result = saMgr->CheckSystemAbility(SAID, isExist);
    EXPECT_EQ(result, nullptr);
    EXPECT_EQ(isExist, false);
}

/**
 * @tc.name: AddOnDemandMax001
 * @tc.desc: pre-populate onDemandAbilityMap_ with 1000 entries, add fails
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, AddOnDemandMax001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    for (uint32_t i = 0; i < MAX_SERVICES; i++) {
        int32_t saId = static_cast<int32_t>(i);
        std::u16string procName = u"proc_" + Str8ToStr16(std::to_string(i));
        saMgr->onDemandAbilityMap_[saId] = procName;
    }
    sptr<IRemoteObject> procObject = new TestTransactionService();
    saMgr->systemProcessMap_[PROCESS_NAME] = procObject;
    int32_t result = saMgr->AddOnDemandSystemAbilityInfo(OTHER_SAID, PROCESS_NAME);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AddOnDemandStarting001
 * @tc.desc: add SA to startingAbilityMap_, add ondemand triggers PostTask
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, AddOnDemandStarting001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    BaseSystemAbilityManager::AbilityItem abilityItem;
    abilityItem.state = BaseSystemAbilityManager::AbilityState::STARTING;
    saMgr->startingAbilityMap_[SAID] = abilityItem;
    sptr<IRemoteObject> procObject = new TestTransactionService();
    saMgr->systemProcessMap_[PROCESS_NAME] = procObject;
    int32_t result = saMgr->AddOnDemandSystemAbilityInfo(SAID, PROCESS_NAME);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: LoadSystemAbilityInvalid001
 * @tc.desc: load system ability with invalid SA id returns INVALID_INPUT_PARA
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, LoadSystemAbilityInvalid001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    sptr<ISystemAbilityLoadCallback> callback = new SystemAbilityLoadCallbackMock();
    int32_t result = saMgr->LoadSystemAbility(TEST_EXCEPTION_HIGH_SA_ID, callback);
    EXPECT_EQ(result, INVALID_INPUT_PARA);
}

/**
 * @tc.name: LoadSystemAbilityNoProfile001
 * @tc.desc: load system ability with no profile returns PROFILE_NOT_EXIST
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, LoadSystemAbilityNoProfile001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    sptr<ISystemAbilityLoadCallback> callback = new SystemAbilityLoadCallbackMock();
    int32_t result = saMgr->LoadSystemAbility(SAID, callback);
    EXPECT_EQ(result, PROFILE_NOT_EXIST);
}

/**
 * @tc.name: UnloadSystemAbilityNoProfile001
 * @tc.desc: unload system ability with no profile returns PROFILE_NOT_EXIST
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, UnloadSystemAbilityNoProfile001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    int32_t result = saMgr->UnloadSystemAbility(SAID);
    EXPECT_EQ(result, PROFILE_NOT_EXIST);
}

/**
 * @tc.name: CancelUnloadInvalid001
 * @tc.desc: cancel unload with invalid SA id returns ERR_INVALID_VALUE
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, CancelUnloadInvalid001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    int32_t result = saMgr->CancelUnloadSystemAbility(TEST_EXCEPTION_HIGH_SA_ID);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: CancelUnloadNoProfile001
 * @tc.desc: cancel unload with no profile returns ERR_INVALID_VALUE
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, CancelUnloadNoProfile001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    int32_t result = saMgr->CancelUnloadSystemAbility(SAID);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: CancelUnloadSchedulerNull001
 * @tc.desc: add profile, set scheduler null returns ERR_INVALID_VALUE
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, CancelUnloadSchedulerNull001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    CommonSaProfile saProfile;
    saProfile.process = u"test_process";
    saProfile.distributed = false;
    saProfile.saId = SAID;
    saMgr->saProfileMap_[SAID] = saProfile;
    saMgr->abilityStateScheduler_ = nullptr;
    int32_t result = saMgr->CancelUnloadSystemAbility(SAID);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    saMgr->saProfileMap_.erase(SAID);
}

/**
 * @tc.name: GetOnDemandReasonExtraDataNull001
 * @tc.desc: set collectManager_=nullptr returns ERR_INVALID_VALUE
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, GetOnDemandReasonExtraDataNull001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    saMgr->collectManager_ = nullptr;
    MessageParcel extraDataParcel;
    int32_t result = saMgr->GetOnDemandReasonExtraData(1, extraDataParcel);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: DoLoadOnDemandLoaded001
 * @tc.desc: add SA to abilityMap_, DoLoadOnDemandAbility returns isExist=true
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, DoLoadOnDemandLoaded001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    SAInfo saInfo;
    saInfo.remoteObj = testAbility;
    saInfo.isDistributed = false;
    saMgr->abilityMap_[SAID] = saInfo;
    bool isExist = false;
    bool result = saMgr->DoLoadOnDemandAbility(SAID, isExist);
    EXPECT_EQ(result, true);
    EXPECT_EQ(isExist, true);
}

/**
 * @tc.name: DoLoadOnDemandStarting001
 * @tc.desc: add SA to startingAbilityMap_ with STARTING state, isExist=true
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, DoLoadOnDemandStarting001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    BaseSystemAbilityManager::AbilityItem abilityItem;
    abilityItem.state = BaseSystemAbilityManager::AbilityState::STARTING;
    saMgr->startingAbilityMap_[SAID] = abilityItem;
    bool isExist = false;
    bool result = saMgr->DoLoadOnDemandAbility(SAID, isExist);
    EXPECT_EQ(result, true);
    EXPECT_EQ(isExist, true);
}

/**
 * @tc.name: DoLoadOnDemandNotOnDemand001
 * @tc.desc: SA not in onDemandAbilityMap_, DoLoadOnDemandAbility returns isExist=false
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, DoLoadOnDemandNotOnDemand001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);
    bool isExist = false;
    bool result = saMgr->DoLoadOnDemandAbility(SAID, isExist);
    EXPECT_EQ(result, false);
    EXPECT_EQ(isExist, false);
}

/**
 * @tc.name: GetExtensionSaIdsEmpty001
 * @tc.desc: empty saProfileMap_ returns empty result
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, GetExtensionSaIdsEmpty001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitSaMgr(saMgr);
    saMgr->saProfileMap_.clear();

    std::vector<int32_t> saIds;
    int32_t ret = saMgr->GetExtensionSaIds("test_ext", saIds);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_TRUE(saIds.empty());
}

/**
 * @tc.name: GetExtensionSaIdsMatch001
 * @tc.desc: add CommonSaProfile with extension="test_ext", SA found in result
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, GetExtensionSaIdsMatch001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitSaMgr(saMgr);
    saMgr->saProfileMap_.clear();

    CommonSaProfile profile;
    profile.process = PROCESS_NAME;
    profile.extension.push_back("test_ext");
    saMgr->saProfileMap_[SAID] = profile;

    std::vector<int32_t> saIds;
    int32_t ret = saMgr->GetExtensionSaIds("test_ext", saIds);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(saIds.size(), 1u);
    EXPECT_EQ(saIds[0], SAID);

    saMgr->saProfileMap_.clear();
}

/**
 * @tc.name: GetExtensionRunningSaListEmpty001
 * @tc.desc: no matching extension returns empty list
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, GetExtensionRunningSaListEmpty001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitSaMgr(saMgr);
    saMgr->saProfileMap_.clear();
    saMgr->abilityMap_.clear();

    CommonSaProfile profile;
    profile.process = PROCESS_NAME;
    profile.extension.push_back("no_match_ext");
    saMgr->saProfileMap_[SAID] = profile;

    std::vector<sptr<IRemoteObject>> saList;
    int32_t ret = saMgr->GetExtensionRunningSaList("test_ext", saList);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_TRUE(saList.empty());

    saMgr->saProfileMap_.clear();
    saMgr->abilityMap_.clear();
}

/**
 * @tc.name: GetExtensionRunningSaListMatch001
 * @tc.desc: matching extension with SA loaded in abilityMap_ returns found
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, GetExtensionRunningSaListMatch001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitSaMgr(saMgr);
    saMgr->saProfileMap_.clear();
    saMgr->abilityMap_.clear();

    CommonSaProfile profile;
    profile.process = PROCESS_NAME;
    profile.extension.push_back("test_ext");
    saMgr->saProfileMap_[SAID] = profile;

    sptr<IRemoteObject> ability = new (std::nothrow) TestTransactionService();
    ASSERT_NE(ability, nullptr);
    SAInfo saInfo;
    saInfo.remoteObj = ability;
    saMgr->abilityMap_[SAID] = saInfo;

    std::vector<sptr<IRemoteObject>> saList;
    int32_t ret = saMgr->GetExtensionRunningSaList("test_ext", saList);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(saList.size(), 1u);
    EXPECT_EQ(saList[0], ability);

    saMgr->saProfileMap_.clear();
    saMgr->abilityMap_.clear();
}

/**
 * @tc.name: GetRunningSaExtInfoNull001
 * @tc.desc: matching extension but GetSystemProcess returns null, skip
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, GetRunningSaExtInfoNull001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitSaMgr(saMgr);
    saMgr->saProfileMap_.clear();
    saMgr->abilityMap_.clear();
    saMgr->systemProcessMap_.clear();

    CommonSaProfile profile;
    profile.process = PROCESS_NAME;
    profile.extension.push_back("test_ext");
    saMgr->saProfileMap_[SAID] = profile;

    std::vector<ISystemAbilityManager::SaExtensionInfo> infoList;
    int32_t ret = saMgr->GetRunningSaExtensionInfoList("test_ext", infoList);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_TRUE(infoList.empty());

    saMgr->saProfileMap_.clear();
}

/**
 * @tc.name: GetRunningSaExtInfoMatch001
 * @tc.desc: matching extension with SA loaded returns found
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, GetRunningSaExtInfoMatch001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitSaMgr(saMgr);
    saMgr->saProfileMap_.clear();
    saMgr->abilityMap_.clear();
    saMgr->systemProcessMap_.clear();

    CommonSaProfile profile;
    profile.process = PROCESS_NAME;
    profile.extension.push_back("test_ext");
    saMgr->saProfileMap_[SAID] = profile;

    sptr<IRemoteObject> procObj = new (std::nothrow) TestTransactionService();
    ASSERT_NE(procObj, nullptr);
    saMgr->systemProcessMap_[PROCESS_NAME] = procObj;

    sptr<IRemoteObject> ability = new (std::nothrow) TestTransactionService();
    ASSERT_NE(ability, nullptr);
    SAInfo saInfo;
    saInfo.remoteObj = ability;
    saMgr->abilityMap_[SAID] = saInfo;

    std::vector<ISystemAbilityManager::SaExtensionInfo> infoList;
    int32_t ret = saMgr->GetRunningSaExtensionInfoList("test_ext", infoList);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(infoList.size(), 1u);
    EXPECT_EQ(infoList[0].saId, SAID);

    saMgr->saProfileMap_.clear();
    saMgr->abilityMap_.clear();
    saMgr->systemProcessMap_.clear();
}

/**
 * @tc.name: GetOnDemandSystemAbilityIdsEmpty001
 * @tc.desc: onDemandSaIdsSet_ empty returns ERR_INVALID_VALUE
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, GetOnDemandSystemAbilityIdsEmpty001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitSaMgr(saMgr);
    saMgr->onDemandSaIdsSet_.clear();

    std::vector<int32_t> ids;
    int32_t ret = saMgr->GetOnDemandSystemAbilityIds(ids);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    EXPECT_TRUE(ids.empty());
}

/**
 * @tc.name: GetOnDemandSystemAbilityIdsOk001
 * @tc.desc: insert SAID into set returns ERR_OK with 1 element
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, GetOnDemandSystemAbilityIdsOk001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitSaMgr(saMgr);
    saMgr->onDemandSaIdsSet_.clear();
    saMgr->onDemandSaIdsSet_.insert(SAID);

    std::vector<int32_t> ids;
    int32_t ret = saMgr->GetOnDemandSystemAbilityIds(ids);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(ids.size(), 1u);
    EXPECT_EQ(ids[0], SAID);
}

/**
 * @tc.name: UpdateSaFreMapInvalid001
 * @tc.desc: uid < 0 returns -1
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, UpdateSaFreMapInvalid001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitSaMgr(saMgr);
    saMgr->saFrequencyMap_.clear();

    int32_t ret = saMgr->UpdateSaFreMap(-1, SAID);
    EXPECT_EQ(ret, -1);
}

/**
 * @tc.name: UpdateSaFreMapOk001
 * @tc.desc: valid uid, call twice, count increments
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, UpdateSaFreMapOk001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitSaMgr(saMgr);
    saMgr->saFrequencyMap_.clear();

    int32_t ret1 = saMgr->UpdateSaFreMap(100, SAID);
    EXPECT_EQ(ret1, 1);

    int32_t ret2 = saMgr->UpdateSaFreMap(100, SAID);
    EXPECT_EQ(ret2, 2);
}

/**
 * @tc.name: NotifySaLoadedCallbackNull001
 * @tc.desc: callback==nullptr returns early
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, NotifySaLoadedCallbackNull001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitSaMgr(saMgr);

    sptr<IRemoteObject> ability = new (std::nothrow) TestTransactionService();
    ASSERT_NE(ability, nullptr);

    saMgr->NotifySystemAbilityLoaded(SAID, ability, nullptr);
}

/**
 * @tc.name: NotifySaLoadFailNull001
 * @tc.desc: callback==nullptr returns early
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, NotifySaLoadFailNull001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitSaMgr(saMgr);

    saMgr->NotifySystemAbilityLoadFail(SAID, nullptr, LOAD_SA_TIMEOUT);
    EXPECT_TRUE(saMgr->startingAbilityMap_.empty());
}

/**
 * @tc.name: IsCacheCommonEvent001
 * @tc.desc: no profile returns false
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, IsCacheCommonEvent001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitSaMgr(saMgr);
    saMgr->saProfileMap_.clear();

    bool ret = saMgr->IsCacheCommonEvent(SAID);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IsModuleUpdate001
 * @tc.desc: no profile returns false
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, IsModuleUpdate001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitSaMgr(saMgr);
    saMgr->saProfileMap_.clear();

    bool ret = saMgr->IsModuleUpdate(SAID);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: NotifyChangedNull001
 * @tc.desc: listener==nullptr returns early
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, NotifyChangedNull001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitSaMgr(saMgr);

    saMgr->NotifySystemAbilityChanged(SAID, "", 0, nullptr);
    EXPECT_TRUE(saMgr->listenerMap_.empty());
}

/**
 * @tc.name: NotifyChangedAdd001
 * @tc.desc: code=0 (ADD) calls OnAddSystemAbility on SaStatusChangeMock
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, NotifyChangedAdd001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitSaMgr(saMgr);

    sptr<SaStatusChangeMock> listener = new (std::nothrow) SaStatusChangeMock();
    ASSERT_NE(listener, nullptr);

    uint32_t addCode = static_cast<uint32_t>(SamgrInterfaceCode::ADD_SYSTEM_ABILITY_TRANSACTION);
    saMgr->NotifySystemAbilityChanged(SAID, "", addCode, listener);
}

/**
 * @tc.name: NotifyChangedRemove001
 * @tc.desc: code=1 (REMOVE) calls OnRemoveSystemAbility
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, NotifyChangedRemove001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitSaMgr(saMgr);

    sptr<SaStatusChangeMock> listener = new (std::nothrow) SaStatusChangeMock();
    ASSERT_NE(listener, nullptr);

    uint32_t removeCode = static_cast<uint32_t>(SamgrInterfaceCode::REMOVE_SYSTEM_ABILITY_TRANSACTION);
    saMgr->NotifySystemAbilityChanged(SAID, "", removeCode, listener);
}

/**
 * @tc.name: NotifyChangedDefault001
 * @tc.desc: code=999 hits default branch, no listener call
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, NotifyChangedDefault001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitSaMgr(saMgr);

    sptr<SaStatusChangeMock> listener = new (std::nothrow) SaStatusChangeMock();
    ASSERT_NE(listener, nullptr);

    saMgr->NotifySystemAbilityChanged(SAID, "", 999, listener);
}

/**
 * @tc.name: FindNotifyNoListeners001
 * @tc.desc: no listeners for SA returns ERR_OK
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, FindNotifyNoListeners001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitSaMgr(saMgr);
    saMgr->listenerMap_.clear();

    int32_t ret = saMgr->FindSystemAbilityNotify(SAID, "", 0);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: FindNotifyAdd001
 * @tc.desc: subscribe listener, call FindSystemAbilityNotify with ADD code, listener becomes NOTIFIED
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, FindNotifyAdd001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitSaMgr(saMgr);
    saMgr->listenerMap_.clear();

    sptr<SaStatusChangeMock> listener = new (std::nothrow) SaStatusChangeMock();
    ASSERT_NE(listener, nullptr);

    SAListener saListener(listener, IPCSkeleton::GetCallingPid(), ListenerState::INIT);
    saMgr->listenerMap_[SAID].push_back(saListener);

    uint32_t addCode = static_cast<uint32_t>(SamgrInterfaceCode::ADD_SYSTEM_ABILITY_TRANSACTION);
    int32_t ret = saMgr->FindSystemAbilityNotify(SAID, "", addCode);
    EXPECT_EQ(ret, ERR_OK);

    for (auto& item : saMgr->listenerMap_[SAID]) {
        EXPECT_EQ(item.state, ListenerState::NOTIFIED);
    }

    saMgr->listenerMap_.clear();
}

/**
 * @tc.name: FindNotifyRemove001
 * @tc.desc: call with REMOVE code resets listener state to INIT
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, FindNotifyRemove001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitSaMgr(saMgr);
    saMgr->listenerMap_.clear();

    sptr<SaStatusChangeMock> listener = new (std::nothrow) SaStatusChangeMock();
    ASSERT_NE(listener, nullptr);

    SAListener saListener(listener, IPCSkeleton::GetCallingPid(), ListenerState::NOTIFIED);
    saMgr->listenerMap_[SAID].push_back(saListener);

    uint32_t removeCode = static_cast<uint32_t>(SamgrInterfaceCode::REMOVE_SYSTEM_ABILITY_TRANSACTION);
    int32_t ret = saMgr->FindSystemAbilityNotify(SAID, "", removeCode);
    EXPECT_EQ(ret, ERR_OK);

    for (auto& item : saMgr->listenerMap_[SAID]) {
        EXPECT_EQ(item.state, ListenerState::INIT);
    }

    saMgr->listenerMap_.clear();
}

/**
 * @tc.name: SystemAbilityInvalidateCache001
 * @tc.desc: SAID in onDemandSaIdsSet_ returns early
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, SystemAbilityInvalidateCache001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitSaMgr(saMgr);
    saMgr->onDemandSaIdsSet_.clear();
    saMgr->onDemandSaIdsSet_.insert(SAID);

    saMgr->SystemAbilityInvalidateCache(SAID);
    EXPECT_NE(saMgr->onDemandSaIdsSet_.find(SAID), saMgr->onDemandSaIdsSet_.end());
}

/**
 * @tc.name: SystemAbilityInvalidateCache002
 * @tc.desc: OTHER_SAID not in set calls SamgrUtil::InvalidateSACache
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, SystemAbilityInvalidateCache002, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitSaMgr(saMgr);
    saMgr->onDemandSaIdsSet_.clear();
    saMgr->onDemandSaIdsSet_.insert(SAID);

    saMgr->SystemAbilityInvalidateCache(OTHER_SAID);
    EXPECT_EQ(saMgr->onDemandSaIdsSet_.find(OTHER_SAID), saMgr->onDemandSaIdsSet_.end());
}

/**
 * @tc.name: NotifySaAddedByAsyncNull001
 * @tc.desc: workHandler_==nullptr returns early
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, NotifySaAddedByAsyncNull001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitSaMgr(saMgr);
    saMgr->workHandler_ = nullptr;

    sptr<SaStatusChangeMock> listener = new (std::nothrow) SaStatusChangeMock();
    ASSERT_NE(listener, nullptr);

    saMgr->NotifySystemAbilityAddedByAsync(SAID, listener);
}

/**
 * @tc.name: SendSaAddedMsgNull001
 * @tc.desc: workHandler_==nullptr returns early
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, SendSaAddedMsgNull001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitSaMgr(saMgr);
    saMgr->workHandler_ = nullptr;

    sptr<IRemoteObject> ability = new (std::nothrow) TestTransactionService();
    ASSERT_NE(ability, nullptr);

    saMgr->SendSystemAbilityAddedMsg(SAID, ability);
}

/**
 * @tc.name: SendSaRemovedMsgNull001
 * @tc.desc: workHandler_==nullptr returns early
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, SendSaRemovedMsgNull001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitSaMgr(saMgr);
    saMgr->workHandler_ = nullptr;

    saMgr->SendSystemAbilityRemovedMsg(SAID);
    EXPECT_EQ(saMgr->workHandler_, nullptr);
}

/**
 * @tc.name: RemoveCheckLoadedMsgNull001
 * @tc.desc: workHandler_==nullptr returns early
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, RemoveCheckLoadedMsgNull001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitSaMgr(saMgr);
    saMgr->workHandler_ = nullptr;

    saMgr->RemoveCheckLoadedMsg(SAID);
    EXPECT_EQ(saMgr->workHandler_, nullptr);
}

/**
 * @tc.name: SendLoadedSaMsgNull001
 * @tc.desc: workHandler_==nullptr returns early
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, SendLoadedSaMsgNull001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitSaMgr(saMgr);
    saMgr->workHandler_ = nullptr;

    sptr<IRemoteObject> ability = new (std::nothrow) TestTransactionService();
    ASSERT_NE(ability, nullptr);
    sptr<SystemAbilityLoadCallbackMock> cb = new (std::nothrow) SystemAbilityLoadCallbackMock();
    ASSERT_NE(cb, nullptr);

    saMgr->SendLoadedSystemAbilityMsg(SAID, ability, cb);
}

/**
 * @tc.name: RefreshListenerState001
 * @tc.desc: SA not in listenerMap_ is no-op
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, RefreshListenerState001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitSaMgr(saMgr);
    saMgr->listenerMap_.clear();

    saMgr->RefreshListenerState(SAID);
    EXPECT_TRUE(saMgr->listenerMap_.find(SAID) == saMgr->listenerMap_.end());
}

/**
 * @tc.name: OnAbilityCallbackDiedNull001
 * @tc.desc: remoteObject==nullptr returns early
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, OnAbilityCallbackDiedNull001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitSaMgr(saMgr);

    saMgr->OnAbilityCallbackDied(nullptr);
    EXPECT_TRUE(saMgr->startingAbilityMap_.empty());
}

/**
 * @tc.name: OnAbilityCallbackDiedEmpty001
 * @tc.desc: empty startingAbilityMap_ is no-op
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, OnAbilityCallbackDiedEmpty001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitSaMgr(saMgr);
    saMgr->startingAbilityMap_.clear();

    sptr<IRemoteObject> obj = new (std::nothrow) TestTransactionService();
    ASSERT_NE(obj, nullptr);

    saMgr->OnAbilityCallbackDied(obj);
    EXPECT_TRUE(saMgr->startingAbilityMap_.empty());
}

/**
 * @tc.name: OnRemoteCallbackDiedNull001
 * @tc.desc: remoteObject==nullptr returns early
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, OnRemoteCallbackDiedNull001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitSaMgr(saMgr);

    saMgr->OnRemoteCallbackDied(nullptr);
    EXPECT_TRUE(saMgr->remoteCallbacks_.empty());
}

/**
 * @tc.name: OnRemoteCallbackDiedEmpty001
 * @tc.desc: empty remoteCallbacks_ is no-op
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, OnRemoteCallbackDiedEmpty001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitSaMgr(saMgr);
    saMgr->remoteCallbacks_.clear();

    sptr<IRemoteObject> obj = new (std::nothrow) TestTransactionService();
    ASSERT_NE(obj, nullptr);

    saMgr->OnRemoteCallbackDied(obj);
    EXPECT_TRUE(saMgr->remoteCallbacks_.empty());
}

/**
 * @tc.name: GetCommonEventExtraNotCache001
 * @tc.desc: SA not cache event returns ERR_OK
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, GetCommonEventExtraNotCache001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitSaMgr(saMgr);
    saMgr->saProfileMap_.clear();

    std::vector<int64_t> extraDataIdList;
    int32_t ret = saMgr->GetCommonEventExtraDataIdlist(SAID, extraDataIdList, "test_event");
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: GetCommonEventExtraNull001
 * @tc.desc: collectManager_==nullptr returns ERR_INVALID_VALUE
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, GetCommonEventExtraNull001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitSaMgr(saMgr);
    saMgr->saProfileMap_.clear();
    saMgr->collectManager_ = nullptr;

    CommonSaProfile profile;
    profile.process = PROCESS_NAME;
    profile.cacheCommonEvent = true;
    saMgr->saProfileMap_[SAID] = profile;

    std::vector<int64_t> extraDataIdList;
    int32_t ret = saMgr->GetCommonEventExtraDataIdlist(SAID, extraDataIdList, "test_event");
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    saMgr->saProfileMap_.clear();
}

/**
 * @tc.name: NotifySaLoadedObjNotStarting001
 * @tc.desc: SA not in startingAbilityMap_ returns early
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, NotifySaLoadedObjNotStarting001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitSaMgr(saMgr);
    saMgr->startingAbilityMap_.clear();

    sptr<IRemoteObject> ability = new (std::nothrow) TestTransactionService();
    ASSERT_NE(ability, nullptr);

    saMgr->NotifySystemAbilityLoaded(SAID, ability);
    EXPECT_TRUE(saMgr->startingAbilityMap_.empty());
}

/**
 * @tc.name: NotifySaLoadedObjHasCallbacks001
 * @tc.desc: SA in startingAbilityMap_ with callbacks, callback called and map cleared
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, NotifySaLoadedObjHasCallbacks001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitSaMgr(saMgr);
    saMgr->startingAbilityMap_.clear();
    saMgr->callbackCountMap_.clear();

    sptr<SystemAbilityLoadCallbackMock> cb = new (std::nothrow) SystemAbilityLoadCallbackMock();
    ASSERT_NE(cb, nullptr);

    if (saMgr->abilityCallbackDeath_ != nullptr) {
        cb->AsObject()->AddDeathRecipient(saMgr->abilityCallbackDeath_);
    }

    BaseSystemAbilityManager::AbilityItem abilityItem;
    abilityItem.callbackMap["local"].push_back({cb, SAID});
    abilityItem.callbackMap["local"].push_back({cb, SAID});
    saMgr->callbackCountMap_[SAID] = 2;
    saMgr->startingAbilityMap_[SAID] = abilityItem;

    sptr<IRemoteObject> ability = new (std::nothrow) TestTransactionService();
    ASSERT_NE(ability, nullptr);

    saMgr->NotifySystemAbilityLoaded(SAID, ability);

    EXPECT_EQ(cb->GetSystemAbilityId(), SAID);
    EXPECT_EQ(cb->GetRemoteObject(), ability);
    EXPECT_TRUE(saMgr->startingAbilityMap_.empty());
}

/**
 * @tc.name: RemoveStartingCbNull001
 * @tc.desc: abilityCallbackDeath_==nullptr skips RemoveDeathRecipient
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, RemoveStartingCbNull001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitSaMgr(saMgr);
    saMgr->abilityCallbackDeath_ = nullptr;
    saMgr->callbackCountMap_.clear();

    sptr<SystemAbilityLoadCallbackMock> cb = new (std::nothrow) SystemAbilityLoadCallbackMock();
    ASSERT_NE(cb, nullptr);

    std::pair<sptr<ISystemAbilityLoadCallback>, int32_t> itemPair(cb, SAID);
    saMgr->callbackCountMap_[SAID] = 1;

    saMgr->RemoveStartingAbilityCallbackLocked(itemPair);

    EXPECT_TRUE(saMgr->callbackCountMap_.find(SAID) == saMgr->callbackCountMap_.end());
}

/**
 * @tc.name: RemoveStartingCbCountZero001
 * @tc.desc: add callback then remove, count reaches 0, erased from map
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, RemoveStartingCbCountZero001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitSaMgr(saMgr);
    saMgr->callbackCountMap_.clear();

    sptr<SystemAbilityLoadCallbackMock> cb = new (std::nothrow) SystemAbilityLoadCallbackMock();
    ASSERT_NE(cb, nullptr);

    if (saMgr->abilityCallbackDeath_ != nullptr) {
        cb->AsObject()->AddDeathRecipient(saMgr->abilityCallbackDeath_);
    }

    std::pair<sptr<ISystemAbilityLoadCallback>, int32_t> itemPair(cb, SAID);
    saMgr->callbackCountMap_[SAID] = 1;

    saMgr->RemoveStartingAbilityCallbackLocked(itemPair);

    EXPECT_TRUE(saMgr->callbackCountMap_.find(SAID) == saMgr->callbackCountMap_.end());
}

/**
 * @tc.name: CheckListenerNotifyNull001
 * @tc.desc: SA not loaded returns early (line 643)
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, CheckListenerNotifyNull001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitSaMgr(saMgr);
    saMgr->abilityMap_.clear();
    saMgr->listenerMap_.clear();

    sptr<SaStatusChangeMock> listener = new (std::nothrow) SaStatusChangeMock();
    ASSERT_NE(listener, nullptr);

    saMgr->CheckListenerNotify(SAID, listener);
}

/**
 * @tc.name: CheckListenerNotifyInit001
 * @tc.desc: SA loaded, listener in INIT state calls NotifySystemAbilityAddedByAsync
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, CheckListenerNotifyInit001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitSaMgr(saMgr);
    saMgr->abilityMap_.clear();
    saMgr->listenerMap_.clear();

    sptr<IRemoteObject> ability = new (std::nothrow) TestTransactionService();
    ASSERT_NE(ability, nullptr);
    SAInfo saInfo;
    saInfo.remoteObj = ability;
    saMgr->abilityMap_[SAID] = saInfo;

    sptr<SaStatusChangeMock> listener = new (std::nothrow) SaStatusChangeMock();
    ASSERT_NE(listener, nullptr);

    SAListener saListener(listener, IPCSkeleton::GetCallingPid(), ListenerState::INIT);
    saMgr->listenerMap_[SAID].push_back(saListener);
    saMgr->subscribeCountMap_[IPCSkeleton::GetCallingPid()] = 1;

    saMgr->CheckListenerNotify(SAID, listener);

    for (auto& item : saMgr->listenerMap_[SAID]) {
        EXPECT_EQ(item.state, ListenerState::NOTIFIED);
    }

    saMgr->abilityMap_.clear();
    saMgr->listenerMap_.clear();
}

/**
 * @tc.name: CheckListenerNotifyNotified001
 * @tc.desc: SA loaded, listener already NOTIFIED skips notification
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, CheckListenerNotifyNotified001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    InitSaMgr(saMgr);
    saMgr->abilityMap_.clear();
    saMgr->listenerMap_.clear();

    sptr<IRemoteObject> ability = new (std::nothrow) TestTransactionService();
    ASSERT_NE(ability, nullptr);
    SAInfo saInfo;
    saInfo.remoteObj = ability;
    saMgr->abilityMap_[SAID] = saInfo;

    sptr<SaStatusChangeMock> listener = new (std::nothrow) SaStatusChangeMock();
    ASSERT_NE(listener, nullptr);

    SAListener saListener(listener, IPCSkeleton::GetCallingPid(), ListenerState::NOTIFIED);
    saMgr->listenerMap_[SAID].push_back(saListener);
    saMgr->subscribeCountMap_[IPCSkeleton::GetCallingPid()] = 1;

    saMgr->CheckListenerNotify(SAID, listener);

    for (auto& item : saMgr->listenerMap_[SAID]) {
        EXPECT_EQ(item.state, ListenerState::NOTIFIED);
    }

    saMgr->abilityMap_.clear();
    saMgr->listenerMap_.clear();
}

/**
 * @tc.name: SendStrategy001
 * @tc.desc: test SendStrategy, caller processName is not resource_schedule_service
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, SendStrategy001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);

    std::vector<int32_t> systemAbilityIds = {SAID};
    int32_t level = 0;
    std::string action = "test";

    int32_t result = saMgr->SendStrategy(0, systemAbilityIds, level, action);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: SendStrategy002
 * @tc.desc: test SendStrategy, SA profile not found
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, SendStrategy002, TestSize.Level1)
{
    SamMockPermission::MockProcess("resource_schedule_service");
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);

    std::vector<int32_t> systemAbilityIds = {SAID};
    int32_t level = 0;
    std::string action = "test";

    int32_t result = saMgr->SendStrategy(0, systemAbilityIds, level, action);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: SendStrategy003
 * @tc.desc: test SendStrategy, SA profile found but process not registered
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, SendStrategy003, TestSize.Level1)
{
    SamMockPermission::MockProcess("resource_schedule_service");
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);

    CommonSaProfile profile;
    profile.process = PROCESS_NAME;
    saMgr->saProfileMap_[SAID] = profile;

    std::vector<int32_t> systemAbilityIds = {SAID};
    int32_t level = 0;
    std::string action = "test";

    int32_t result = saMgr->SendStrategy(0, systemAbilityIds, level, action);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    saMgr->saProfileMap_.clear();
}

/**
 * @tc.name: SendStrategy004
 * @tc.desc: test SendStrategy, empty SA list returns OK
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, SendStrategy004, TestSize.Level1)
{
    SamMockPermission::MockProcess("resource_schedule_service");
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);

    std::vector<int32_t> systemAbilityIds;
    int32_t level = 0;
    std::string action = "test";

    int32_t result = saMgr->SendStrategy(0, systemAbilityIds, level, action);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: SendStrategy005
 * @tc.desc: test SendStrategy, valid profile and process, SendStrategyToSA called
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, SendStrategy005, TestSize.Level1)
{
    SamMockPermission::MockProcess("resource_schedule_service");
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);

    CommonSaProfile profile;
    profile.process = PROCESS_NAME;
    saMgr->saProfileMap_[SAID] = profile;

    sptr<MockLocalAbilityManager> procObject = new MockLocalAbilityManager;
    saMgr->systemProcessMap_[PROCESS_NAME] = procObject;

    std::vector<int32_t> systemAbilityIds = {SAID};
    int32_t level = 1;
    std::string action = "test_action";

    int32_t result = saMgr->SendStrategy(0, systemAbilityIds, level, action);
    EXPECT_EQ(result, ERR_OK);

    saMgr->saProfileMap_.clear();
    saMgr->systemProcessMap_.clear();
}

/**
 * @tc.name: SendStrategy006
 * @tc.desc: test SendStrategy, second SA in list has no profile
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, SendStrategy006, TestSize.Level1)
{
    SamMockPermission::MockProcess("resource_schedule_service");
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);

    CommonSaProfile profile;
    profile.process = PROCESS_NAME;
    saMgr->saProfileMap_[SAID] = profile;

    sptr<MockLocalAbilityManager> procObject = new MockLocalAbilityManager;
    saMgr->systemProcessMap_[PROCESS_NAME] = procObject;

    std::vector<int32_t> systemAbilityIds = {SAID, OTHER_SAID};
    int32_t level = 0;
    std::string action = "test";

    int32_t result = saMgr->SendStrategy(0, systemAbilityIds, level, action);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    saMgr->saProfileMap_.clear();
    saMgr->systemProcessMap_.clear();
}

/**
 * @tc.name: SendStrategy007
 * @tc.desc: test SendStrategy, multiple valid SAs all succeed
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, SendStrategy007, TestSize.Level1)
{
    SamMockPermission::MockProcess("resource_schedule_service");
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    InitSaMgr(saMgr);

    CommonSaProfile profile;
    profile.process = PROCESS_NAME;
    saMgr->saProfileMap_[SAID] = profile;
    saMgr->saProfileMap_[OTHER_SAID] = profile;

    sptr<MockLocalAbilityManager> procObject = new MockLocalAbilityManager;
    saMgr->systemProcessMap_[PROCESS_NAME] = procObject;

    std::vector<int32_t> systemAbilityIds = {SAID, OTHER_SAID};
    int32_t level = 2;
    std::string action = "multi_test";

    int32_t result = saMgr->SendStrategy(0, systemAbilityIds, level, action);
    EXPECT_EQ(result, ERR_OK);

    saMgr->saProfileMap_.clear();
    saMgr->systemProcessMap_.clear();
}
} // namespace OHOS
