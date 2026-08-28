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

#ifndef SERVICES_SAMGR_NATIVE_INCLUDE_MULTI_SYSTEM_ABILITY_MANAGER_H
#define SERVICES_SAMGR_NATIVE_INCLUDE_MULTI_SYSTEM_ABILITY_MANAGER_H

#include "base_system_ability_manager.h"

namespace OHOS {

// Keep listeners alive while no user manager exists so the next activation can replay subscriptions.
struct GlobalSystemAbilitySubscription {
    int32_t systemAbilityId;
    sptr<IRemoteObject> remoteObject;
    int32_t callingPid;
};

struct GlobalSubscriptionInfo {
    std::list<GlobalSystemAbilitySubscription> systemAbilitySubscriptions;
    std::list<sptr<IRemoteObject>> systemProcessSubscriptions;
};

class MultiSystemAbilityManager : public BaseSystemAbilityManager {
public:
    explicit MultiSystemAbilityManager(int32_t userId);
    MultiSystemAbilityManager(int32_t userId, const GlobalSubscriptionInfo& globalSubscriptions);
    ~MultiSystemAbilityManager();

    int32_t GetUserId() const override { return userId_; }

    int32_t Init(const std::list<SaProfile>& saProfiles);
    void Destroy() override;

    void StopEventCollection();

    void NotifyAllSAsToStop();

    int32_t SubscribeSystemAbility(int32_t systemAbilityId,
        const sptr<ISystemAbilityStatusChange>& listener) override;
    int32_t SubscribeSystemAbility(int32_t systemAbilityId,
        const sptr<ISystemAbilityStatusChange>& listener, bool foregroundOnly);
    int32_t UnSubscribeSystemAbility(int32_t systemAbilityId,
        const sptr<ISystemAbilityStatusChange>& listener) override;
    void UnSubscribeSystemAbility(const sptr<IRemoteObject>& remoteObject) override;
    int32_t UnSubscribeSystemAbility(int32_t systemAbilityId,
        const sptr<ISystemAbilityStatusChange>& listener, bool foregroundOnly);
    int32_t SubscribeSystemProcess(const sptr<ISystemProcessStatusChange>& listener) override;
    int32_t SubscribeSystemProcess(const sptr<ISystemProcessStatusChange>& listener, bool foregroundOnly);
    int32_t UnSubscribeSystemProcess(const sptr<ISystemProcessStatusChange>& listener) override;
    int32_t UnSubscribeSystemProcess(const sptr<ISystemProcessStatusChange>& listener, bool foregroundOnly);

    int32_t StartDynamicSystemProcess(const std::u16string& name, int32_t systemAbilityId,
        const OnDemandEvent& event) override;

protected:
    int32_t FindSystemAbilityNotify(int32_t systemAbilityId, const std::string& deviceId, int32_t code) override;
    void NotifySystemAbilityChanged(int32_t systemAbilityId, const std::string& deviceId, int32_t code,
        const sptr<ISystemAbilityStatusChange>& listener) override;

private:
    friend class UserLifecycleManager;

    struct SubscriberInfo {
        wptr<IRemoteObject> remoteObject;
        bool hasDirectSubscription = false;
        bool hasForegroundSubscription = false;
    };

    void AddSubscriberInfo(int32_t systemAbilityId, const sptr<ISystemAbilityStatusChange>& listener,
        bool foregroundOnly);
    bool RemoveSubscriberInfo(int32_t systemAbilityId, const sptr<ISystemAbilityStatusChange>& listener,
        bool foregroundOnly);
    bool ShouldNotifySubscriber(int32_t systemAbilityId, const sptr<ISystemAbilityStatusChange>& listener);
    int32_t SubscribeSystemAbilityWithPid(int32_t systemAbilityId,
        const sptr<ISystemAbilityStatusChange>& listener, bool foregroundOnly, int32_t callingPid);
    void InitSaProfiles(const std::list<SaProfile>& saProfiles, const std::set<int32_t>& multiInstanceSaIds,
        std::list<SaProfile>& filteredProfiles);
    int32_t InitGlobalSubscriptions();
    void RollbackGlobalSubscriptions(
        const std::vector<std::pair<int32_t, sptr<ISystemAbilityStatusChange>>>& restoredSaSubscriptions,
        const std::vector<sptr<ISystemProcessStatusChange>>& restoredProcessSubscriptions);

    int32_t userId_;
    GlobalSubscriptionInfo globalSubscriptions_;
    samgr::mutex subscriberInfoLock_;
    std::map<int32_t, std::list<SubscriberInfo>> subscriberInfoMap_;
};

} // namespace OHOS

#endif // !defined(SERVICES_SAMGR_NATIVE_INCLUDE_MULTI_SYSTEM_ABILITY_MANAGER_H)
