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
#include "itest_transaction_service.h"
#include "multi_system_ability_manager.h"
#include "sa_status_change_mock.h"
#include "system_ability_manager.h"
#include "system_process_status_change_stub.h"
#include "test_log.h"

using namespace std;
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace {
constexpr int32_t SAID = 1234;
constexpr int32_t OTHER_SAID = 1499;
const std::u16string PROCESS_NAME = u"test_process_name";

class CountingSaStatusChange final : public SystemAbilityStatusChangeStub {
public:
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override
    {
        ++addCount_;
    }

    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override
    {
        ++removeCount_;
    }

    int32_t addCount_ = 0;
    int32_t removeCount_ = 0;
};

class CountingProcessStatusChange final : public SystemProcessStatusChangeStub {
public:
    void OnSystemProcessStarted(SystemProcessInfo& systemProcessInfo) override
    {
        ++startedCount_;
    }

    void OnSystemProcessStopped(SystemProcessInfo& systemProcessInfo) override
    {
        ++stoppedCount_;
    }

    int32_t startedCount_ = 0;
    int32_t stoppedCount_ = 0;
};

class NullRemoteSaStatusChange final : public ISystemAbilityStatusChange {
public:
    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }

    void OnAddSystemAbility(int32_t, const std::string&) override {}
    void OnRemoveSystemAbility(int32_t, const std::string&) override {}
};
} // namespace

/**
 * @tc.name: MultiSystemAbilitySubscriberInfo001
 * @tc.desc: Verify direct and foreground subscriptions are tracked independently.
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, MultiSystemAbilitySubscriberInfo001, TestSize.Level1)
{
    constexpr int32_t USER_ID = 100;
    auto manager = std::make_shared<MultiSystemAbilityManager>(USER_ID);
    sptr<SaStatusChangeMock> listener = new SaStatusChangeMock();
    ASSERT_NE(manager, nullptr);
    ASSERT_NE(listener, nullptr);

    EXPECT_FALSE(manager->ShouldNotifySubscriber(SAID, listener));
    manager->AddSubscriberInfo(SAID, listener, false);
    EXPECT_TRUE(manager->ShouldNotifySubscriber(SAID, listener));

    manager->AddSubscriberInfo(SAID, listener, true);
    EXPECT_FALSE(manager->RemoveSubscriberInfo(SAID, listener, false));
    EXPECT_TRUE(manager->RemoveSubscriberInfo(SAID, listener, true));
    EXPECT_FALSE(manager->ShouldNotifySubscriber(SAID, listener));

    manager->AddSubscriberInfo(SAID, listener, false);
    manager->UnSubscribeSystemAbility(listener->AsObject());
    EXPECT_TRUE(manager->subscriberInfoMap_.empty());

    sptr<SystemAbilityManager> samgr = SystemAbilityManager::GetInstance();
    ASSERT_NE(samgr, nullptr);
    const int32_t previousUserId = samgr->userLifecycleManager_.GetForegroundUserId();
    samgr->userLifecycleManager_.foregroundUserId_.store(USER_ID);
    manager->listenerMap_[SAID].emplace_back(listener, 0);
    EXPECT_EQ(manager->BaseSystemAbilityManager::FindSystemAbilityNotify(
        SAID, "", static_cast<int32_t>(SamgrInterfaceCode::ADD_SYSTEM_ABILITY_TRANSACTION)), ERR_OK);
    EXPECT_EQ(manager->listenerMap_[SAID].front().state, ListenerState::NOTIFIED);
    EXPECT_EQ(manager->FindSystemAbilityNotify(
        SAID, "", static_cast<int32_t>(SamgrInterfaceCode::REMOVE_SYSTEM_ABILITY_TRANSACTION)), ERR_OK);
    samgr->userLifecycleManager_.foregroundUserId_.store(previousUserId);
    EXPECT_EQ(manager->listenerMap_[SAID].front().state, ListenerState::INIT);
}

/**
 * @tc.name: MultiSystemAbilitySubscriberInfo002
 * @tc.desc: Verify invalid listeners and an empty listener map are handled safely.
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, MultiSystemAbilitySubscriberInfo002, TestSize.Level1)
{
    constexpr int32_t USER_ID = 101;
    auto manager = std::make_shared<MultiSystemAbilityManager>(USER_ID);
    sptr<ISystemAbilityStatusChange> listener = nullptr;
    ASSERT_NE(manager, nullptr);

    manager->AddSubscriberInfo(SAID, listener, false);
    EXPECT_FALSE(manager->RemoveSubscriberInfo(SAID, listener, false));
    EXPECT_FALSE(manager->ShouldNotifySubscriber(SAID, listener));
    EXPECT_EQ(manager->UnSubscribeSystemAbility(SAID, listener, false), ERR_INVALID_VALUE);
    EXPECT_EQ(manager->FindSystemAbilityNotify(SAID, "", 0), ERR_OK);
    manager->UnSubscribeSystemAbility(sptr<IRemoteObject>(nullptr));
}

/**
 * @tc.name: MultiSystemAbilitySubscriberInfo009
 * @tc.desc: Verify listeners without a remote object are rejected consistently.
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, MultiSystemAbilitySubscriberInfo009, TestSize.Level1)
{
    constexpr int32_t userId = 117;
    auto manager = std::make_shared<MultiSystemAbilityManager>(userId);
    sptr<ISystemAbilityStatusChange> listener = new NullRemoteSaStatusChange();
    ASSERT_NE(manager, nullptr);
    ASSERT_NE(listener, nullptr);

    manager->AddSubscriberInfo(SAID, listener, false);
    EXPECT_TRUE(manager->subscriberInfoMap_.empty());
    EXPECT_FALSE(manager->RemoveSubscriberInfo(SAID, listener, false));
    EXPECT_FALSE(manager->ShouldNotifySubscriber(SAID, listener));
    EXPECT_EQ(manager->UnSubscribeSystemAbility(SAID, listener, false), ERR_INVALID_VALUE);
}

/**
 * @tc.name: MultiSystemAbilitySubscriberInfo010
 * @tc.desc: Verify removing a foreground subscription preserves a direct subscription.
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, MultiSystemAbilitySubscriberInfo010, TestSize.Level1)
{
    constexpr int32_t userId = 118;
    auto manager = std::make_shared<MultiSystemAbilityManager>(userId);
    sptr<SaStatusChangeMock> listener = new SaStatusChangeMock();
    ASSERT_NE(manager, nullptr);
    ASSERT_NE(listener, nullptr);

    manager->AddSubscriberInfo(SAID, listener, false);
    EXPECT_FALSE(manager->RemoveSubscriberInfo(SAID, listener, true));
    ASSERT_EQ(manager->subscriberInfoMap_[SAID].size(), 1U);
    EXPECT_TRUE(manager->subscriberInfoMap_[SAID].front().hasDirectSubscription);
}

/**
 * @tc.name: MultiSystemAbilitySubscriberCleanup011
 * @tc.desc: Verify expired and multi-listener subscription records are cleaned correctly.
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, MultiSystemAbilitySubscriberCleanup011, TestSize.Level1)
{
    constexpr int32_t userId = 123;
    auto manager = std::make_shared<MultiSystemAbilityManager>(userId);
    sptr<SaStatusChangeMock> expiredListener = new SaStatusChangeMock();
    sptr<SaStatusChangeMock> activeListener = new SaStatusChangeMock();
    sptr<SaStatusChangeMock> secondListener = new SaStatusChangeMock();
    ASSERT_NE(manager, nullptr);
    manager->AddSubscriberInfo(SAID, expiredListener, false);
    expiredListener = nullptr;
    EXPECT_TRUE(manager->RemoveSubscriberInfo(SAID, activeListener, false));
    EXPECT_TRUE(manager->subscriberInfoMap_.empty());
    manager->AddSubscriberInfo(SAID, activeListener, false);
    manager->AddSubscriberInfo(SAID, secondListener, false);
    EXPECT_TRUE(manager->RemoveSubscriberInfo(SAID, activeListener, false));
    ASSERT_EQ(manager->subscriberInfoMap_[SAID].size(), 1U);
    EXPECT_TRUE(manager->ShouldNotifySubscriber(SAID, secondListener));
}

/**
 * @tc.name: MultiSystemAbilitySubscriberNotify012
 * @tc.desc: Verify notification filtering handles non-target and foreground records.
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, MultiSystemAbilitySubscriberNotify012, TestSize.Level1)
{
    constexpr int32_t userId = 124;
    auto manager = std::make_shared<MultiSystemAbilityManager>(userId);
    sptr<SaStatusChangeMock> listener = new SaStatusChangeMock();
    sptr<SaStatusChangeMock> otherListener = new SaStatusChangeMock();
    sptr<SystemAbilityManager> samgr = SystemAbilityManager::GetInstance();
    ASSERT_NE(manager, nullptr);
    ASSERT_NE(samgr, nullptr);
    manager->AddSubscriberInfo(SAID, listener, true);
    EXPECT_FALSE(manager->ShouldNotifySubscriber(SAID, otherListener));
    auto& subscriber = manager->subscriberInfoMap_[SAID].front();
    subscriber.hasForegroundSubscription = false;
    EXPECT_TRUE(manager->ShouldNotifySubscriber(SAID, listener));
    subscriber.hasForegroundSubscription = true;
    const int32_t previousUserId = samgr->userLifecycleManager_.GetForegroundUserId();
    samgr->userLifecycleManager_.foregroundUserId_.store(userId);
    EXPECT_TRUE(manager->ShouldNotifySubscriber(SAID, listener));
    samgr->userLifecycleManager_.foregroundUserId_.store(previousUserId);
}

/**
 * @tc.name: MultiSystemAbilityProfileBranches013
 * @tc.desc: Verify existing handlers and both profile creation modes are retained.
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, MultiSystemAbilityProfileBranches013, TestSize.Level1)
{
    constexpr int32_t userId = 125;
    constexpr int32_t onDemandSaId = 2246;
    constexpr int32_t runOnCreateSaId = 2247;
    auto manager = std::make_shared<MultiSystemAbilityManager>(userId);
    ASSERT_NE(manager, nullptr);
    manager->workHandler_ = std::make_shared<FFRTHandler>("existingHandler");
    auto originalHandler = manager->workHandler_;
    SaProfile onDemandProfile = {u"ondemand_process", onDemandSaId};
    SaProfile runOnCreateProfile = {u"startup_process", runOnCreateSaId};
    onDemandProfile.runOnCreate = false;
    runOnCreateProfile.runOnCreate = true;
    std::list<SaProfile> profiles = {onDemandProfile, runOnCreateProfile};
    std::set<int32_t> saIds = {onDemandSaId, runOnCreateSaId};
    std::list<SaProfile> filteredProfiles;
    manager->InitSaProfiles(profiles, saIds, filteredProfiles);
    EXPECT_EQ(filteredProfiles.size(), 2U);
    EXPECT_EQ(manager->workHandler_, originalHandler);
    EXPECT_EQ(manager->onDemandSaIdsSet_.count(onDemandSaId), 1U);
    EXPECT_EQ(manager->onDemandSaIdsSet_.count(runOnCreateSaId), 0U);
}

/**
 * @tc.name: MultiSystemAbilityBackgroundNotify001
 * @tc.desc: Verify a background user's listener notification is skipped.
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, MultiSystemAbilityBackgroundNotify001, TestSize.Level1)
{
    constexpr int32_t userId = 122;
    constexpr int32_t foregroundUserId = 123;
    auto manager = std::make_shared<MultiSystemAbilityManager>(userId);
    sptr<SaStatusChangeMock> listener = new SaStatusChangeMock();
    sptr<SystemAbilityManager> samgr = SystemAbilityManager::GetInstance();
    ASSERT_NE(manager, nullptr);
    ASSERT_NE(listener, nullptr);
    ASSERT_NE(samgr, nullptr);
    const int32_t previousUserId = samgr->userLifecycleManager_.GetForegroundUserId();
    manager->listenerMap_[SAID].emplace_back(listener, 0);
    samgr->userLifecycleManager_.foregroundUserId_.store(foregroundUserId);

    EXPECT_EQ(manager->BaseSystemAbilityManager::FindSystemAbilityNotify(
        SAID, "", static_cast<int32_t>(SamgrInterfaceCode::ADD_SYSTEM_ABILITY_TRANSACTION)), ERR_OK);
    samgr->userLifecycleManager_.foregroundUserId_.store(previousUserId);
    EXPECT_EQ(manager->listenerMap_[SAID].front().state, ListenerState::INIT);
}

/**
 * @tc.name: MultiSystemAbilityProcessState001
 * @tc.desc: Verify process-name enumeration and null scheduler unsubscription paths.
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, MultiSystemAbilityProcessState001, TestSize.Level1)
{
    constexpr int32_t userId = 102;
    auto manager = std::make_shared<MultiSystemAbilityManager>(userId);
    ASSERT_NE(manager, nullptr);

    EXPECT_TRUE(manager->GetSystemProcessNames().empty());
    sptr<IRemoteObject> process = new TestTransactionService();
    manager->systemProcessMap_[PROCESS_NAME] = process;
    std::vector<std::u16string> processNames = manager->GetSystemProcessNames();
    ASSERT_EQ(processNames.size(), 1U);
    EXPECT_EQ(processNames.front(), PROCESS_NAME);

    manager->abilityStateScheduler_ = nullptr;
    void (BaseSystemAbilityManager::*unsubscribe)(const sptr<IRemoteObject>&) =
        &BaseSystemAbilityManager::UnSubscribeSystemProcess;
    EXPECT_NO_FATAL_FAILURE((manager.get()->*unsubscribe)(process));
}

/**
 * @tc.name: MultiSystemAbilityProcessStateNullScheduler001
 * @tc.desc: Verify process APIs reject a missing state scheduler.
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, MultiSystemAbilityProcessStateNullScheduler001, TestSize.Level1)
{
    auto manager = std::make_shared<MultiSystemAbilityManager>(120);
    ASSERT_NE(manager, nullptr);
    manager->abilityStateScheduler_ = nullptr;
    SystemProcessInfo processInfo;
    std::list<SystemProcessInfo> processInfos;
    sptr<ISystemProcessStatusChange> listener = nullptr;
    EXPECT_EQ(manager->GetSystemProcessInfo(SAID, processInfo), ERR_INVALID_VALUE);
    EXPECT_EQ(manager->GetRunningSystemProcess(processInfos), ERR_INVALID_VALUE);
    EXPECT_EQ(manager->SubscribeSystemProcess(listener), ERR_INVALID_VALUE);
    EXPECT_EQ(manager->UnSubscribeSystemProcess(listener), ERR_INVALID_VALUE);
}

/**
 * @tc.name: MultiSystemAbilityProcessNames001
 * @tc.desc: Verify process names are enumerated from the multi-user manager.
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, MultiSystemAbilityProcessNames001, TestSize.Level1)
{
    auto manager = std::make_shared<MultiSystemAbilityManager>(121);
    ASSERT_NE(manager, nullptr);
    manager->systemProcessMap_[u"samgr_test_process"] = new TestTransactionService();
    auto names = manager->GetSystemProcessNames();
    ASSERT_EQ(names.size(), 1U);
    EXPECT_EQ(names.front(), u"samgr_test_process");
}

/**
 * @tc.name: MultiSystemAbilityLifecycle001
 * @tc.desc: Verify multi-user manager lifecycle paths without initialized child components.
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, MultiSystemAbilityLifecycle001, TestSize.Level1)
{
    constexpr int32_t USER_ID = 102;
    auto manager = std::make_shared<MultiSystemAbilityManager>(USER_ID);
    sptr<ISystemProcessStatusChange> listener = nullptr;
    ASSERT_NE(manager, nullptr);

    EXPECT_EQ(manager->GetUserId(), USER_ID);
    EXPECT_EQ(manager->SubscribeSystemProcess(listener, true), ERR_INVALID_VALUE);
    EXPECT_EQ(manager->UnSubscribeSystemProcess(listener, true), ERR_INVALID_VALUE);
    manager->NotifyAllSAsToStop();
    manager->StopEventCollection();
    manager->Destroy();
    EXPECT_TRUE(manager->subscriberInfoMap_.empty());
}

/**
 * @tc.name: MultiSystemAbilityInit001
 * @tc.desc: Verify initialization restores only live global subscriptions.
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, MultiSystemAbilityInit001, TestSize.Level1)
{
    constexpr int32_t USER_ID = 103;
    GlobalSubscriptionInfo subscriptions;
    subscriptions.systemAbilitySubscriptions.push_back({SAID, nullptr, 0});
    subscriptions.systemProcessSubscriptions.push_back(nullptr);
    auto manager = std::make_shared<MultiSystemAbilityManager>(USER_ID, subscriptions);
    std::list<SaProfile> profiles;
    ASSERT_NE(manager, nullptr);

    EXPECT_EQ(manager->Init(profiles), ERR_OK);
    EXPECT_NE(manager->workHandler_, nullptr);
    EXPECT_NE(manager->abilityStateScheduler_, nullptr);
    EXPECT_NE(manager->collectManager_, nullptr);
    EXPECT_TRUE(manager->globalSubscriptions_.systemAbilitySubscriptions.empty());
    EXPECT_TRUE(manager->globalSubscriptions_.systemProcessSubscriptions.empty());
    manager->Destroy();
}

/**
 * @tc.name: MultiSystemAbilitySubscriberInfo003
 * @tc.desc: Verify foreground filtering and expired subscriber cleanup.
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, MultiSystemAbilitySubscriberInfo003, TestSize.Level1)
{
    constexpr int32_t USER_ID = 104;
    constexpr int32_t OTHER_USER_ID = 105;
    auto manager = std::make_shared<MultiSystemAbilityManager>(USER_ID);
    sptr<SaStatusChangeMock> foregroundListener = new SaStatusChangeMock();
    sptr<SaStatusChangeMock> expiredListener = new SaStatusChangeMock();
    sptr<SystemAbilityManager> samgr = SystemAbilityManager::GetInstance();
    ASSERT_NE(manager, nullptr);
    ASSERT_NE(foregroundListener, nullptr);
    ASSERT_NE(expiredListener, nullptr);
    ASSERT_NE(samgr, nullptr);

    int32_t previousUserId = samgr->userLifecycleManager_.GetForegroundUserId();
    manager->AddSubscriberInfo(SAID, foregroundListener, true);
    samgr->userLifecycleManager_.foregroundUserId_.store(OTHER_USER_ID);
    EXPECT_FALSE(manager->ShouldNotifySubscriber(SAID, foregroundListener));
    samgr->userLifecycleManager_.foregroundUserId_.store(USER_ID);
    EXPECT_TRUE(manager->ShouldNotifySubscriber(SAID, foregroundListener));

    manager->AddSubscriberInfo(SAID, expiredListener, false);
    expiredListener = nullptr;
    manager->AddSubscriberInfo(SAID, foregroundListener, false);
    EXPECT_TRUE(manager->ShouldNotifySubscriber(SAID, foregroundListener));
    samgr->userLifecycleManager_.foregroundUserId_.store(previousUserId);
}

/**
 * @tc.name: MultiSystemAbilitySubscriberNotify001
 * @tc.desc: Verify foreground-only and direct subscriptions dispatch notifications correctly.
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, MultiSystemAbilitySubscriberNotify001, TestSize.Level1)
{
    constexpr int32_t USER_ID = 106;
    constexpr int32_t OTHER_USER_ID = 107;
    auto manager = std::make_shared<MultiSystemAbilityManager>(USER_ID);
    sptr<CountingSaStatusChange> listener = new CountingSaStatusChange();
    sptr<SystemAbilityManager> samgr = SystemAbilityManager::GetInstance();
    ASSERT_NE(manager, nullptr);
    ASSERT_NE(listener, nullptr);
    ASSERT_NE(samgr, nullptr);

    int32_t previousUserId = samgr->userLifecycleManager_.GetForegroundUserId();
    manager->AddSubscriberInfo(SAID, listener, true);
    samgr->userLifecycleManager_.foregroundUserId_.store(OTHER_USER_ID);
    manager->NotifySystemAbilityChanged(
        SAID, "", static_cast<int32_t>(SamgrInterfaceCode::ADD_SYSTEM_ABILITY_TRANSACTION), listener);
    EXPECT_EQ(listener->addCount_, 0);

    samgr->userLifecycleManager_.foregroundUserId_.store(USER_ID);
    manager->NotifySystemAbilityChanged(
        SAID, "", static_cast<int32_t>(SamgrInterfaceCode::ADD_SYSTEM_ABILITY_TRANSACTION), listener);
    EXPECT_EQ(listener->addCount_, 1);

    manager->AddSubscriberInfo(SAID, listener, false);
    samgr->userLifecycleManager_.foregroundUserId_.store(OTHER_USER_ID);
    manager->NotifySystemAbilityChanged(
        SAID, "", static_cast<int32_t>(SamgrInterfaceCode::REMOVE_SYSTEM_ABILITY_TRANSACTION), listener);
    EXPECT_EQ(listener->removeCount_, 1);
    samgr->userLifecycleManager_.foregroundUserId_.store(previousUserId);
}

/**
 * @tc.name: MultiSystemAbilitySubscriberInfo004
 * @tc.desc: Verify expired and missing subscriber records are removed safely.
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, MultiSystemAbilitySubscriberInfo004, TestSize.Level1)
{
    constexpr int32_t USER_ID = 108;
    auto manager = std::make_shared<MultiSystemAbilityManager>(USER_ID);
    sptr<SaStatusChangeMock> expiredListener = new SaStatusChangeMock();
    sptr<SaStatusChangeMock> activeListener = new SaStatusChangeMock();
    ASSERT_NE(manager, nullptr);
    ASSERT_NE(expiredListener, nullptr);
    ASSERT_NE(activeListener, nullptr);

    manager->AddSubscriberInfo(SAID, expiredListener, false);
    expiredListener = nullptr;
    manager->AddSubscriberInfo(SAID, activeListener, false);
    ASSERT_EQ(manager->subscriberInfoMap_[SAID].size(), 1U);

    EXPECT_TRUE(manager->RemoveSubscriberInfo(SAID + 1, activeListener, false));
    EXPECT_TRUE(manager->RemoveSubscriberInfo(SAID, activeListener, false));
    EXPECT_TRUE(manager->subscriberInfoMap_.empty());

    manager->AddSubscriberInfo(SAID, activeListener, false);
    manager->listenerMap_[SAID].emplace_back(activeListener, 0);
    EXPECT_EQ(manager->FindSystemAbilityNotify(
        SAID, "", static_cast<int32_t>(SamgrInterfaceCode::ADD_SYSTEM_ABILITY_TRANSACTION)), ERR_OK);
    EXPECT_EQ(manager->FindSystemAbilityNotify(
        SAID, "", static_cast<int32_t>(SamgrInterfaceCode::ADD_SYSTEM_ABILITY_TRANSACTION)), ERR_OK);
}

/**
 * @tc.name: MultiSystemAbilitySubscriberInfo005
 * @tc.desc: Verify default unsubscribe keeps foreground subscriptions and remote removal keeps other entries.
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, MultiSystemAbilitySubscriberInfo005, TestSize.Level1)
{
    constexpr int32_t USER_ID = 110;
    constexpr int32_t SECOND_SAID = SAID + 2;
    auto manager = std::make_shared<MultiSystemAbilityManager>(USER_ID);
    sptr<SaStatusChangeMock> firstListener = new SaStatusChangeMock();
    sptr<SaStatusChangeMock> secondListener = new SaStatusChangeMock();
    ASSERT_NE(manager, nullptr);
    ASSERT_NE(firstListener, nullptr);
    ASSERT_NE(secondListener, nullptr);

    manager->AddSubscriberInfo(SAID, firstListener, false);
    manager->AddSubscriberInfo(SAID, firstListener, true);
    EXPECT_EQ(manager->UnSubscribeSystemAbility(SAID, firstListener), ERR_OK);
    ASSERT_EQ(manager->subscriberInfoMap_[SAID].size(), 1U);
    EXPECT_FALSE(manager->subscriberInfoMap_[SAID].front().hasDirectSubscription);
    EXPECT_TRUE(manager->subscriberInfoMap_[SAID].front().hasForegroundSubscription);

    manager->AddSubscriberInfo(SECOND_SAID, secondListener, false);
    manager->UnSubscribeSystemAbility(firstListener->AsObject());
    EXPECT_EQ(manager->subscriberInfoMap_.count(SAID), 0U);
    EXPECT_EQ(manager->subscriberInfoMap_.count(SECOND_SAID), 1U);
}

/**
 * @tc.name: MultiSystemAbilitySubscriberInfo006
 * @tc.desc: Verify a failed SA subscription rolls back subscriber metadata.
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, MultiSystemAbilitySubscriberInfo006, TestSize.Level1)
{
    constexpr int32_t USER_ID = 111;
    constexpr int32_t CALLING_PID = 12346;
    auto manager = std::make_shared<MultiSystemAbilityManager>(USER_ID);
    sptr<SaStatusChangeMock> listener = new SaStatusChangeMock();
    ASSERT_NE(manager, nullptr);
    ASSERT_NE(listener, nullptr);

    manager->subscribeCountMap_[CALLING_PID] = BaseSystemAbilityManager::MAX_SUBSCRIBE_COUNT;
    EXPECT_EQ(manager->SubscribeSystemAbilityWithPid(SAID, listener, false, CALLING_PID), ERR_PERMISSION_DENIED);
    EXPECT_TRUE(manager->subscriberInfoMap_.empty());
}

/**
 * @tc.name: MultiSystemAbilitySubscriberInfo007
 * @tc.desc: Verify an expired subscriber record is removed during notification filtering.
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, MultiSystemAbilitySubscriberInfo007, TestSize.Level1)
{
    constexpr int32_t USER_ID = 113;
    auto manager = std::make_shared<MultiSystemAbilityManager>(USER_ID);
    sptr<SaStatusChangeMock> expiredListener = new SaStatusChangeMock();
    sptr<SaStatusChangeMock> activeListener = new SaStatusChangeMock();
    ASSERT_NE(manager, nullptr);
    ASSERT_NE(expiredListener, nullptr);
    ASSERT_NE(activeListener, nullptr);

    manager->AddSubscriberInfo(SAID, expiredListener, false);
    expiredListener = nullptr;
    EXPECT_FALSE(manager->ShouldNotifySubscriber(SAID, activeListener));
    EXPECT_TRUE(manager->subscriberInfoMap_.empty());
}

/**
 * @tc.name: MultiSystemAbilitySubscriberInfo008
 * @tc.desc: Verify removing an unregistered listener preserves existing subscription metadata.
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, MultiSystemAbilitySubscriberInfo008, TestSize.Level1)
{
    constexpr int32_t USER_ID = 115;
    auto manager = std::make_shared<MultiSystemAbilityManager>(USER_ID);
    sptr<SaStatusChangeMock> registeredListener = new SaStatusChangeMock();
    sptr<SaStatusChangeMock> unregisteredListener = new SaStatusChangeMock();
    ASSERT_NE(manager, nullptr);
    ASSERT_NE(registeredListener, nullptr);
    ASSERT_NE(unregisteredListener, nullptr);

    manager->AddSubscriberInfo(SAID, registeredListener, false);
    EXPECT_TRUE(manager->RemoveSubscriberInfo(SAID, unregisteredListener, false));
    ASSERT_EQ(manager->subscriberInfoMap_[SAID].size(), 1U);
    EXPECT_TRUE(manager->ShouldNotifySubscriber(SAID, registeredListener));
}

/**
 * @tc.name: MultiSystemAbilityInit004
 * @tc.desc: Verify failed global subscription restoration rolls back prior subscriptions.
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, MultiSystemAbilityInit004, TestSize.Level1)
{
    constexpr int32_t USER_ID = 112;
    constexpr int32_t CALLING_PID = 12347;
    sptr<SaStatusChangeMock> firstListener = new SaStatusChangeMock();
    sptr<SaStatusChangeMock> secondListener = new SaStatusChangeMock();
    GlobalSubscriptionInfo subscriptions;
    subscriptions.systemAbilitySubscriptions.push_back({SAID, firstListener->AsObject(), CALLING_PID});
    subscriptions.systemAbilitySubscriptions.push_back({SAID + 1, secondListener->AsObject(), CALLING_PID});
    auto manager = std::make_shared<MultiSystemAbilityManager>(USER_ID, subscriptions);
    std::list<SaProfile> profiles;
    ASSERT_NE(firstListener, nullptr);
    ASSERT_NE(secondListener, nullptr);
    ASSERT_NE(manager, nullptr);

    manager->subscribeCountMap_[CALLING_PID] = BaseSystemAbilityManager::MAX_SUBSCRIBE_COUNT - 1;
    EXPECT_EQ(manager->Init(profiles), ERR_PERMISSION_DENIED);
    EXPECT_TRUE(manager->listenerMap_[SAID].empty());
    EXPECT_TRUE(manager->listenerMap_[SAID + 1].empty());
    manager->Destroy();
}

/**
 * @tc.name: MultiSystemAbilityStartDynamic001
 * @tc.desc: Verify an invalid user process start is reported as a failure.
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, MultiSystemAbilityStartDynamic001, TestSize.Level1)
{
    constexpr int32_t USER_ID = 106;
    const std::u16string invalidProcess = u"1234567890123456789012345678901234567890123456789"
        "01234567890123456789012345678901234567890123456";
    auto manager = std::make_shared<MultiSystemAbilityManager>(USER_ID);
    OnDemandEvent event;
    ASSERT_NE(manager, nullptr);

    EXPECT_NE(manager->StartDynamicSystemProcess(invalidProcess, SAID, event), ERR_OK);
}

/**
 * @tc.name: MultiSystemAbilityInit002
 * @tc.desc: Verify a valid global SA subscription is restored during initialization.
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, MultiSystemAbilityInit002, TestSize.Level1)
{
    constexpr int32_t USER_ID = 107;
    constexpr int32_t CALLING_PID = 12345;
    sptr<SaStatusChangeMock> listener = new SaStatusChangeMock();
    GlobalSubscriptionInfo subscriptions;
    subscriptions.systemAbilitySubscriptions.push_back({SAID, listener->AsObject(), CALLING_PID});
    auto manager = std::make_shared<MultiSystemAbilityManager>(USER_ID, subscriptions);
    std::list<SaProfile> profiles;
    ASSERT_NE(listener, nullptr);
    ASSERT_NE(manager, nullptr);

    EXPECT_EQ(manager->Init(profiles), ERR_OK);
    ASSERT_NE(manager->listenerMap_.find(SAID), manager->listenerMap_.end());
    ASSERT_EQ(manager->listenerMap_[SAID].size(), 1U);
    EXPECT_EQ(manager->listenerMap_[SAID].front().callingPid, CALLING_PID);
    manager->Destroy();
}

/**
 * @tc.name: MultiSystemAbilityInit005
 * @tc.desc: Verify a valid global process subscription is restored during initialization.
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, MultiSystemAbilityInit005, TestSize.Level1)
{
    constexpr int32_t USER_ID = 114;
    sptr<CountingProcessStatusChange> listener = new CountingProcessStatusChange();
    GlobalSubscriptionInfo subscriptions;
    subscriptions.systemProcessSubscriptions.push_back(listener->AsObject());
    auto manager = std::make_shared<MultiSystemAbilityManager>(USER_ID, subscriptions);
    std::list<SaProfile> profiles;
    ASSERT_NE(listener, nullptr);
    ASSERT_NE(manager, nullptr);

    EXPECT_EQ(manager->Init(profiles), ERR_OK);
    ASSERT_NE(manager->abilityStateScheduler_, nullptr);
    EXPECT_EQ(manager->abilityStateScheduler_->processListeners_.size(), 1U);
    manager->Destroy();
}

/**
 * @tc.name: MultiSystemAbilityInit006
 * @tc.desc: Verify a non-process remote object is ignored during global process subscription restoration.
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, MultiSystemAbilityInit006, TestSize.Level1)
{
    constexpr int32_t USER_ID = 116;
    GlobalSubscriptionInfo subscriptions;
    subscriptions.systemProcessSubscriptions.push_back(new TestTransactionService());
    auto manager = std::make_shared<MultiSystemAbilityManager>(USER_ID, subscriptions);
    std::list<SaProfile> profiles;
    ASSERT_NE(manager, nullptr);

    EXPECT_EQ(manager->Init(profiles), ERR_OK);
    ASSERT_NE(manager->abilityStateScheduler_, nullptr);
    EXPECT_TRUE(manager->abilityStateScheduler_->processListeners_.empty());
    manager->Destroy();
}

/**
 * @tc.name: MultiSystemAbilityInit007
 * @tc.desc: Verify an incompatible global SA subscription object is ignored.
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, MultiSystemAbilityInit007, TestSize.Level1)
{
    constexpr int32_t userId = 119;
    GlobalSubscriptionInfo subscriptions;
    subscriptions.systemAbilitySubscriptions.push_back({SAID, new TestTransactionService(), 0});
    auto manager = std::make_shared<MultiSystemAbilityManager>(userId, subscriptions);
    std::list<SaProfile> profiles;
    ASSERT_NE(manager, nullptr);

    ASSERT_EQ(manager->Init(profiles), ERR_OK);
    manager->Destroy();
    EXPECT_TRUE(manager->listenerMap_.empty());
}

/**
 * @tc.name: MultiSystemAbilityInit003
 * @tc.desc: Verify initialization keeps only configured multi-instance on-demand profiles.
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, MultiSystemAbilityInit003, TestSize.Level1)
{
    constexpr int32_t USER_ID = 109;
    constexpr int32_t NON_MULTI_INSTANCE_SAID = SAID + 1;
    sptr<SystemAbilityManager> samgr = SystemAbilityManager::GetInstance();
    ASSERT_NE(samgr, nullptr);
    const std::set<int32_t> originalSaIds = samgr->multiInstanceSaIds_;
    samgr->multiInstanceSaIds_.clear();
    samgr->multiInstanceSaIds_.insert(SAID);

    SaProfile multiInstanceProfile = {u"multi_instance_test", SAID};
    multiInstanceProfile.runOnCreate = false;
    SaProfile commonProfile = {u"common_test", NON_MULTI_INSTANCE_SAID};
    std::list<SaProfile> profiles = {multiInstanceProfile, commonProfile};
    auto manager = std::make_shared<MultiSystemAbilityManager>(USER_ID);
    ASSERT_NE(manager, nullptr);

    EXPECT_EQ(manager->Init(profiles), ERR_OK);
    EXPECT_NE(manager->saProfileMap_.find(SAID), manager->saProfileMap_.end());
    EXPECT_EQ(manager->saProfileMap_.find(NON_MULTI_INSTANCE_SAID), manager->saProfileMap_.end());
    EXPECT_TRUE(manager->onDemandSaIdsSet_.count(SAID) > 0);
    manager->Destroy();
    samgr->multiInstanceSaIds_ = originalSaIds;
}

/**
 * @tc.name: MultiSystemAbilityNotifyRemoval014
 * @tc.desc: Verify add and remove notifications update listener state and invoke both callbacks.
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, MultiSystemAbilityNotifyRemoval014, TestSize.Level1)
{
    constexpr int32_t userId = 127;
    auto manager = std::make_shared<MultiSystemAbilityManager>(userId);
    sptr<CountingSaStatusChange> listener = new CountingSaStatusChange();
    ASSERT_NE(manager, nullptr);
    ASSERT_NE(listener, nullptr);
    manager->AddSubscriberInfo(SAID, listener, false);
    manager->listenerMap_[SAID].emplace_back(listener, 0);

    EXPECT_EQ(manager->FindSystemAbilityNotify(SAID, "",
        static_cast<int32_t>(SamgrInterfaceCode::ADD_SYSTEM_ABILITY_TRANSACTION)), ERR_OK);
    EXPECT_EQ(manager->listenerMap_[SAID].front().state, ListenerState::NOTIFIED);
    EXPECT_EQ(manager->FindSystemAbilityNotify(SAID, "",
        static_cast<int32_t>(SamgrInterfaceCode::REMOVE_SYSTEM_ABILITY_TRANSACTION)), ERR_OK);
    EXPECT_EQ(listener->addCount_, 1);
    EXPECT_EQ(listener->removeCount_, 1);
    EXPECT_EQ(manager->listenerMap_[SAID].front().state, ListenerState::INIT);
}

/**
 * @tc.name: MultiSystemAbilityRemoteUnsubscribe015
 * @tc.desc: Verify remote-object unsubscription uses an initialized process scheduler.
 * @tc.type: FUNC
 */
HWTEST_F(BaseSystemAbilityMgrTest, MultiSystemAbilityRemoteUnsubscribe015, TestSize.Level1)
{
    constexpr int32_t userId = 128;
    auto manager = std::make_shared<MultiSystemAbilityManager>(userId);
    sptr<CountingProcessStatusChange> listener = new CountingProcessStatusChange();
    ASSERT_NE(manager, nullptr);
    ASSERT_NE(listener, nullptr);
    ASSERT_EQ(manager->Init({}), ERR_OK);
    ASSERT_EQ(manager->SubscribeSystemProcess(listener), ERR_OK);

    sptr<IRemoteObject> remoteObject = listener->AsObject();
    void (BaseSystemAbilityManager::*unsubscribe)(const sptr<IRemoteObject>&) =
        &BaseSystemAbilityManager::UnSubscribeSystemProcess;
    (manager.get()->*unsubscribe)(remoteObject);
    EXPECT_TRUE(manager->abilityStateScheduler_->processListeners_.empty());
    manager->Destroy();
}
} // namespace OHOS
