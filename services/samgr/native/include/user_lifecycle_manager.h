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

#ifndef SERVICES_SAMGR_NATIVE_INCLUDE_USER_LIFECYCLE_MANAGER_H
#define SERVICES_SAMGR_NATIVE_INCLUDE_USER_LIFECYCLE_MANAGER_H

#ifdef SUPPORT_MULTI_INSTANCE

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>

#include "ffrt_handler.h"
#include "if_system_ability_manager.h"
#include "multi_system_ability_manager.h"
#include "samgr_ffrt_api.h"

namespace OHOS {

struct SaProfile;

class UserLifecycleManager {
public:
    UserLifecycleManager() = default;
    ~UserLifecycleManager();

    int32_t OnUserStateChanged(int32_t userId, SamgrUserState userState);

    int32_t GetForegroundUserId() const
    {
        return foregroundUserId_.load();
    }

    bool IsValidUser(int32_t userId) const;

    std::shared_ptr<MultiSystemAbilityManager> GetMultiUserManager(int32_t userId) const;
    std::shared_ptr<MultiSystemAbilityManager> GetStoppingMultiUserManager(int32_t userId) const;

    bool IsUserStopping(int32_t userId) const;

    void SetSaProfiles(const std::list<SaProfile>* profiles)
    {
        allSaProfiles_ = profiles;
    }

    size_t GetActiveUserCount() const;

    std::set<int32_t> GetValidUserIds() const;

    int32_t SubscribeSystemAbilityForAllUsers(int32_t systemAbilityId,
        const sptr<ISystemAbilityStatusChange>& listener, int32_t callingPid);
    int32_t UnSubscribeSystemAbilityForAllUsers(int32_t systemAbilityId,
        const sptr<ISystemAbilityStatusChange>& listener);
    int32_t SubscribeSystemProcessForAllUsers(
        const sptr<ISystemProcessStatusChange>& listener, bool& subscriptionAdded);
    int32_t UnSubscribeSystemProcessForAllUsers(const sptr<ISystemProcessStatusChange>& listener);

private:
    struct LifecycleTaskState {
        std::mutex lock;
        std::condition_variable condition;
        bool alive = true;
        uint32_t activeTasks = 0;
    };

    struct UserStopping {
        std::shared_ptr<MultiSystemAbilityManager> manager;
        std::vector<std::u16string> processNames;
        std::atomic<bool> finalizing { false };
    };

    std::shared_ptr<samgr::mutex> GetUserLifecycleLock(int32_t userId);
    int32_t HandleUserActivating(int32_t userId);
    int32_t HandleUserSwitching(int32_t userId);
    int32_t HandleUserStopping(int32_t userId);
    void StartUserStopping(int32_t userId, const std::shared_ptr<MultiSystemAbilityManager>& manager);
    void PostUserStopTimeoutTask(int32_t userId, const std::shared_ptr<UserStopping>& stoppingContext,
        const std::shared_ptr<FFRTHandler>& stopHandler);
    void CompleteUserStopping(int32_t userId, const std::shared_ptr<UserStopping>& stoppingContext);
    void CancelPendingStopTasks();
    void RemoveExpiredGlobalSubscriptionsLocked();
    void RollbackSaSubscribe(const std::vector<std::shared_ptr<MultiSystemAbilityManager>>& managers,
        int32_t systemAbilityId, const sptr<ISystemAbilityStatusChange>& listener);
    void RollbackSaUnsubscribe(const std::vector<std::shared_ptr<MultiSystemAbilityManager>>& managers,
        int32_t systemAbilityId, const sptr<ISystemAbilityStatusChange>& listener, int32_t callingPid);
    void RollbackProcessSubscribe(
        const std::vector<std::shared_ptr<MultiSystemAbilityManager>>& managers,
        const sptr<ISystemProcessStatusChange>& listener);
    void RollbackProcessUnsubscribe(
        const std::vector<std::shared_ptr<MultiSystemAbilityManager>>& managers,
        const sptr<ISystemProcessStatusChange>& listener);

    samgr::mutex lifecycleLocksLock_;
    std::map<int32_t, std::shared_ptr<samgr::mutex>> lifecycleLocks_;
    mutable samgr::shared_mutex userStateLock_;
    std::map<int32_t, SamgrUserState> userStateMap_;
    std::map<int32_t, std::shared_ptr<MultiSystemAbilityManager>> multiUserManagers_;
    std::map<int32_t, std::shared_ptr<UserStopping>> userStoppingContexts_;
    // Keep each user's queue alive after its stop task returns to avoid destroying a queue from its own task.
    std::map<int32_t, std::shared_ptr<FFRTHandler>> stopTaskHandlers_;
    std::set<int32_t> validUserIds_;
    std::atomic<int32_t> foregroundUserId_{SAMGR_INVALID_USER_ID};
    std::shared_ptr<LifecycleTaskState> taskState_ = std::make_shared<LifecycleTaskState>();

    samgr::mutex globalSubscriptionLock_;
    GlobalSubscriptionInfo globalSubscriptions_;

    const std::list<SaProfile>* allSaProfiles_ = nullptr;
};

} // namespace OHOS

#endif // SUPPORT_MULTI_INSTANCE

#endif // SERVICES_SAMGR_NATIVE_INCLUDE_USER_LIFECYCLE_MANAGER_H
