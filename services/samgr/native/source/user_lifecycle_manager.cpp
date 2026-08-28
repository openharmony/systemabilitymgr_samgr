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

#include "user_lifecycle_manager.h"

#include <algorithm>

#include "datetime_ex.h"
#include "errors.h"
#include "multi_system_ability_manager.h"
#include "sa_profiles.h"
#include "sam_log.h"
#include "samgr_xcollie.h"
#include "service_control.h"
#include "string_ex.h"

namespace OHOS {
namespace {
constexpr uint64_t USER_STOP_TIMEOUT = 5 * 1000;
// ServiceWaitForStatusByUserId treats timeout <= 0 as the init default (30s).
// Keep this bounded so the fallback can issue the stop request promptly.
constexpr int32_t USER_STOP_STATUS_CHECK_TIMEOUT = 1;
constexpr const char* USER_STOP_HANDLER_PREFIX = "UserStopHandler_";
}

UserLifecycleManager::~UserLifecycleManager()
{
    CancelPendingStopTasks();
}

int32_t UserLifecycleManager::OnUserStateChanged(int32_t userId, SamgrUserState userState)
{
    HILOGD("OnUserStateChanged userId:%{public}d, state:%{public}d", userId, userState);

    auto lifecycleLock = GetUserLifecycleLock(userId);
    std::lock_guard<samgr::mutex> lock(*lifecycleLock);
    switch (userState) {
        case USER_STATE_ACTIVATING:
            return HandleUserActivating(userId);
        case USER_STATE_SWITCHING:
            return HandleUserSwitching(userId);
        case USER_STATE_STOPPING:
            return HandleUserStopping(userId);
        default:
            HILOGE("OnUserStateChanged invalid state:%{public}d", userState);
            return ERR_INVALID_VALUE;
    }
}

std::shared_ptr<samgr::mutex> UserLifecycleManager::GetUserLifecycleLock(int32_t userId)
{
    std::lock_guard<samgr::mutex> lock(lifecycleLocksLock_);
    auto& lifecycleLock = lifecycleLocks_[userId];
    if (lifecycleLock == nullptr) {
        lifecycleLock = std::make_shared<samgr::mutex>();
    }
    return lifecycleLock;
}

int32_t UserLifecycleManager::HandleUserActivating(int32_t userId)
{
    HILOGD("HandleUserActivating userId:%{public}d", userId);

    {
        std::unique_lock<samgr::shared_mutex> lock(userStateLock_);
        auto stateIt = userStateMap_.find(userId);
        if (stateIt != userStateMap_.end() && stateIt->second == USER_STATE_STOPPING) {
            HILOGW("HandleUserActivating reject, userId:%{public}d is stopping", userId);
            return ERR_INVALID_OPERATION;
        }
        if (multiUserManagers_.count(userId) > 0) {
            HILOGD("HandleUserActivating userId:%{public}d already exists, skip", userId);
            return ERR_OK;
        }
    }

    // Serialize creation with global subscription updates so the snapshot and insertion are atomic.
    std::lock_guard<samgr::mutex> subscriptionLock(globalSubscriptionLock_);
    RemoveExpiredGlobalSubscriptionsLocked();

    if (allSaProfiles_ == nullptr) {
        HILOGE("HandleUserActivating allSaProfiles_ not initialized, userId:%{public}d", userId);
        return ERR_INVALID_VALUE;
    }

    auto mgr = std::make_shared<MultiSystemAbilityManager>(userId, globalSubscriptions_);
    int32_t ret = mgr->Init(*allSaProfiles_);
    if (ret != ERR_OK) {
        HILOGE("HandleUserActivating Init MultiSAMgr failed, userId:%{public}d, ret:%{public}d",
            userId, ret);
        mgr->Destroy();
        return ret;
    }

    {
        std::unique_lock<samgr::shared_mutex> lock(userStateLock_);
        if (multiUserManagers_.count(userId) > 0) {
            HILOGW("HandleUserActivating userId:%{public}d created by another thread, "
                "destroy current", userId);
            lock.unlock();
            mgr->Destroy();
            return ERR_OK;
        }
        multiUserManagers_[userId] = mgr;
        validUserIds_.insert(userId);
        userStateMap_[userId] = USER_STATE_ACTIVATING;
    }

    HILOGI("HandleUserActivating userId:%{public}d done", userId);
    return ERR_OK;
}

int32_t UserLifecycleManager::HandleUserSwitching(int32_t userId)
{
    HILOGD("HandleUserSwitching userId:%{public}d", userId);

    {
        std::unique_lock<samgr::shared_mutex> lock(userStateLock_);

        auto stateIt = userStateMap_.find(userId);
        if (stateIt != userStateMap_.end() && stateIt->second == USER_STATE_STOPPING) {
            HILOGW("HandleUserSwitching reject, userId:%{public}d is stopping", userId);
            return ERR_INVALID_VALUE;
        }

        if (userId == foregroundUserId_.load()) {
            HILOGD("HandleUserSwitching userId:%{public}d already foreground, skip", userId);
            return ERR_OK;
        }

        if (multiUserManagers_.count(userId) == 0) {
            HILOGE("HandleUserSwitching Manager not exist for userId:%{public}d", userId);
            return ERR_INVALID_VALUE;
        }

        userStateMap_[userId] = USER_STATE_SWITCHING;
        foregroundUserId_.store(userId);
    }

    HILOGI("HandleUserSwitching userId:%{public}d set as foreground", userId);
    return ERR_OK;
}

int32_t UserLifecycleManager::HandleUserStopping(int32_t userId)
{
    HILOGD("HandleUserStopping userId:%{public}d", userId);
    std::shared_ptr<MultiSystemAbilityManager> mgr;
    {
        std::unique_lock<samgr::shared_mutex> lock(userStateLock_);
        if (userStoppingContexts_.count(userId) > 0) {
            HILOGD("HandleUserStopping userId:%{public}d already stopping, skip", userId);
            return ERR_OK;
        }
        userStateMap_[userId] = USER_STATE_STOPPING;
        validUserIds_.erase(userId);

        auto it = multiUserManagers_.find(userId);
        if (it == multiUserManagers_.end()) {
            HILOGW("HandleUserStopping Manager not found for userId:%{public}d", userId);
            userStateMap_.erase(userId);
            return ERR_INVALID_VALUE;
        }
        mgr = it->second;
        multiUserManagers_.erase(it);

        if (userId == foregroundUserId_.load()) {
            foregroundUserId_.store(SAMGR_INVALID_USER_ID);
            HILOGI("HandleUserStopping foreground userId:%{public}d logged out, fallback to INVALID", userId);
        }
    }

    StartUserStopping(userId, mgr);
    return ERR_OK;
}

void UserLifecycleManager::StartUserStopping(
    int32_t userId, const std::shared_ptr<MultiSystemAbilityManager>& manager)
{
    auto stoppingContext = std::make_shared<UserStopping>();
    stoppingContext->manager = manager;
    stoppingContext->processNames = manager->GetSystemProcessNames();
    std::shared_ptr<FFRTHandler> stopHandler;
    {
        std::unique_lock<samgr::shared_mutex> lock(userStateLock_);
        userStoppingContexts_[userId] = stoppingContext;
        auto& handler = stopTaskHandlers_[userId];
        if (handler == nullptr) {
            handler = std::make_shared<FFRTHandler>(USER_STOP_HANDLER_PREFIX + std::to_string(userId));
        }
        stopHandler = handler;
    }

    manager->NotifyAllSAsToStop();
    if (stoppingContext->processNames.empty()) {
        CompleteUserStopping(userId, stoppingContext);
        return;
    }
    PostUserStopTimeoutTask(userId, stoppingContext, stopHandler);
}

void UserLifecycleManager::PostUserStopTimeoutTask(int32_t userId,
    const std::shared_ptr<UserStopping>& stoppingContext, const std::shared_ptr<FFRTHandler>& stopHandler)
{
    HILOGD("HandleUserStopping userId:%{public}d, asynchronously waiting %{public}zu processes to exit",
        userId, stoppingContext->processNames.size());
    auto taskState = taskState_;
    auto timeoutTask = [this, taskState, userId, stoppingContext]() {
        {
            std::lock_guard<std::mutex> lock(taskState->lock);
            if (!taskState->alive) {
                return;
            }
            ++taskState->activeTasks;
        }

        CompleteUserStopping(userId, stoppingContext);

        {
            std::lock_guard<std::mutex> lock(taskState->lock);
            --taskState->activeTasks;
            if (!taskState->alive && taskState->activeTasks == 0) {
                taskState->condition.notify_all();
            }
        }
    };
    if (!stopHandler->PostTask(timeoutTask, USER_STOP_TIMEOUT)) {
        HILOGE("HandleUserStopping post timeout task failed, userId:%{public}d", userId);
        CompleteUserStopping(userId, stoppingContext);
    }
}

void UserLifecycleManager::CompleteUserStopping(
    int32_t userId, const std::shared_ptr<UserStopping>& stoppingContext)
{
    if (stoppingContext == nullptr || stoppingContext->manager == nullptr ||
        stoppingContext->finalizing.exchange(true)) {
        return;
    }

    for (const auto& procName : stoppingContext->processNames) {
        std::string procNameStr = Str16ToStr8(procName);
        int32_t waitResult = ERR_INVALID_VALUE;
        {
            SamgrXCollie samgrXCollie("samgr--WaitUserProcessStop_" + procNameStr);
            waitResult = ServiceWaitForStatusByUserId(procNameStr.c_str(), userId,
                ServiceStatus::SERVICE_STOPPED, USER_STOP_STATUS_CHECK_TIMEOUT);
        }
        if (waitResult == ERR_OK) {
            HILOGD("Process stopped, proc:%{public}s, userId:%{public}d", procNameStr.c_str(), userId);
            continue;
        }

        int32_t stopRet = ERR_INVALID_VALUE;
        {
            SamgrXCollie samgrXCollie("samgr--ForceStopUserProcess_" + procNameStr);
            stopRet = ServiceControlWithExtraByUserId(procNameStr.c_str(), ServiceAction::STOP,
                userId, nullptr, 0);
        }
        if (stopRet != ERR_OK) {
            HILOGE("CompleteUserStopping status check failed, proc:%{public}s, userId:%{public}d, "
                "waitResult:%{public}d, forceStopResult:%{public}d", procNameStr.c_str(), userId,
                waitResult, stopRet);
        } else {
            HILOGW("CompleteUserStopping force stopped proc:%{public}s, userId:%{public}d, "
                "waitResult:%{public}d, forceStopResult:%{public}d", procNameStr.c_str(), userId,
                waitResult, stopRet);
        }
    }

    stoppingContext->manager->StopEventCollection();
    stoppingContext->manager->Destroy();

    {
        std::unique_lock<samgr::shared_mutex> lock(userStateLock_);
        auto contextIt = userStoppingContexts_.find(userId);
        if (contextIt != userStoppingContexts_.end() && contextIt->second == stoppingContext) {
            userStoppingContexts_.erase(contextIt);
            auto stateIt = userStateMap_.find(userId);
            if (stateIt != userStateMap_.end() && stateIt->second == USER_STATE_STOPPING) {
                userStateMap_.erase(stateIt);
            }
        }
    }

    HILOGI("CompleteUserStopping userId:%{public}d done", userId);
}

void UserLifecycleManager::CancelPendingStopTasks()
{
    {
        SamgrXCollie samgrXCollie("samgr--CancelPendingUserStopTasks");
        std::unique_lock<std::mutex> lock(taskState_->lock);
        taskState_->alive = false;
        taskState_->condition.wait(lock, [state = taskState_]() {
            return state->activeTasks == 0;
        });
    }

    std::vector<std::shared_ptr<FFRTHandler>> stopHandlers;
    {
        std::unique_lock<samgr::shared_mutex> lock(userStateLock_);
        for (const auto& item : stopTaskHandlers_) {
            stopHandlers.push_back(item.second);
        }
        stopTaskHandlers_.clear();
        userStoppingContexts_.clear();
    }

    for (const auto& stopHandler : stopHandlers) {
        if (stopHandler != nullptr) {
            stopHandler->CleanFfrt();
        }
    }
}

bool UserLifecycleManager::IsValidUser(int32_t userId) const
{
    std::shared_lock<samgr::shared_mutex> lock(userStateLock_);
    return validUserIds_.count(userId) > 0;
}

std::shared_ptr<MultiSystemAbilityManager> UserLifecycleManager::GetMultiUserManager(int32_t userId) const
{
    std::shared_lock<samgr::shared_mutex> lock(userStateLock_);
    auto it = multiUserManagers_.find(userId);
    if (it == multiUserManagers_.end()) {
        return nullptr;
    }
    return it->second;
}

std::shared_ptr<MultiSystemAbilityManager> UserLifecycleManager::GetStoppingMultiUserManager(int32_t userId) const
{
    std::shared_lock<samgr::shared_mutex> lock(userStateLock_);
    auto contextIt = userStoppingContexts_.find(userId);
    if (contextIt == userStoppingContexts_.end() || contextIt->second == nullptr) {
        return nullptr;
    }
    return contextIt->second->manager;
}

bool UserLifecycleManager::IsUserStopping(int32_t userId) const
{
    std::shared_lock<samgr::shared_mutex> lock(userStateLock_);
    return userStoppingContexts_.count(userId) > 0;
}

size_t UserLifecycleManager::GetActiveUserCount() const
{
    std::shared_lock<samgr::shared_mutex> lock(userStateLock_);
    return multiUserManagers_.size();
}

std::set<int32_t> UserLifecycleManager::GetValidUserIds() const
{
    std::shared_lock<samgr::shared_mutex> lock(userStateLock_);
    return validUserIds_;
}

void UserLifecycleManager::RemoveExpiredGlobalSubscriptionsLocked()
{
    globalSubscriptions_.systemAbilitySubscriptions.remove_if([](const auto& subscription) {
        sptr<IRemoteObject> remoteObject = subscription.remoteObject;
        return remoteObject == nullptr || remoteObject->IsObjectDead();
    });
    globalSubscriptions_.systemProcessSubscriptions.remove_if([](const auto& subscription) {
        sptr<IRemoteObject> remoteObject = subscription;
        return remoteObject == nullptr || remoteObject->IsObjectDead();
    });
}

void UserLifecycleManager::RollbackSaSubscribe(
    const std::vector<std::shared_ptr<MultiSystemAbilityManager>>& managers, int32_t systemAbilityId,
    const sptr<ISystemAbilityStatusChange>& listener)
{
    for (auto iter = managers.rbegin(); iter != managers.rend(); ++iter) {
        int32_t rollbackResult = (*iter)->UnSubscribeSystemAbility(systemAbilityId, listener, true);
        if (rollbackResult != ERR_OK) {
            HILOGE("Subscription rollback failed, result:%{public}d", rollbackResult);
        }
    }
}

void UserLifecycleManager::RollbackSaUnsubscribe(
    const std::vector<std::shared_ptr<MultiSystemAbilityManager>>& managers, int32_t systemAbilityId,
    const sptr<ISystemAbilityStatusChange>& listener, int32_t callingPid)
{
    for (auto iter = managers.rbegin(); iter != managers.rend(); ++iter) {
        int32_t rollbackResult = (*iter)->SubscribeSystemAbilityWithPid(systemAbilityId, listener, true, callingPid);
        if (rollbackResult != ERR_OK) {
            HILOGE("Subscription rollback failed, result:%{public}d", rollbackResult);
        }
    }
}

void UserLifecycleManager::RollbackProcessSubscribe(
    const std::vector<std::shared_ptr<MultiSystemAbilityManager>>& managers,
    const sptr<ISystemProcessStatusChange>& listener)
{
    for (auto iter = managers.rbegin(); iter != managers.rend(); ++iter) {
        int32_t rollbackResult = (*iter)->UnSubscribeSystemProcess(listener, true);
        if (rollbackResult != ERR_OK) {
            HILOGE("System process subscription rollback failed, result:%{public}d", rollbackResult);
        }
    }
}

void UserLifecycleManager::RollbackProcessUnsubscribe(
    const std::vector<std::shared_ptr<MultiSystemAbilityManager>>& managers,
    const sptr<ISystemProcessStatusChange>& listener)
{
    for (auto iter = managers.rbegin(); iter != managers.rend(); ++iter) {
        int32_t rollbackResult = (*iter)->SubscribeSystemProcess(listener, true);
        if (rollbackResult != ERR_OK) {
            HILOGE("System process subscription rollback failed, result:%{public}d", rollbackResult);
        }
    }
}

int32_t UserLifecycleManager::SubscribeSystemAbilityForAllUsers(int32_t systemAbilityId,
    const sptr<ISystemAbilityStatusChange>& listener, int32_t callingPid)
{
    if (listener == nullptr || listener->AsObject() == nullptr) {
        return ERR_INVALID_VALUE;
    }

    std::lock_guard<samgr::mutex> subscriptionLock(globalSubscriptionLock_);
    RemoveExpiredGlobalSubscriptionsLocked();
    sptr<IRemoteObject> remoteObject = listener->AsObject();
    auto existing = std::find_if(globalSubscriptions_.systemAbilitySubscriptions.begin(),
        globalSubscriptions_.systemAbilitySubscriptions.end(),
        [systemAbilityId, remoteObject](const auto& subscription) {
            return subscription.systemAbilityId == systemAbilityId &&
                subscription.remoteObject == remoteObject;
        });
    if (existing != globalSubscriptions_.systemAbilitySubscriptions.end()) {
        return ERR_OK;
    }
    size_t subscriptionCount = std::count_if(globalSubscriptions_.systemAbilitySubscriptions.begin(),
        globalSubscriptions_.systemAbilitySubscriptions.end(), [callingPid](const auto& subscription) {
            return subscription.callingPid == callingPid;
        });
    if (subscriptionCount >= static_cast<size_t>(BaseSystemAbilityManager::MAX_SUBSCRIBE_COUNT)) {
        HILOGE("SubscribeSystemAbilityForAllUsers pid:%{public}d overflow max subscribe count", callingPid);
        return ERR_PERMISSION_DENIED;
    }

    std::vector<std::shared_ptr<MultiSystemAbilityManager>> subscribedManagers;
    std::shared_lock<samgr::shared_mutex> userLock(userStateLock_);
    for (const auto& [userId, manager] : multiUserManagers_) {
        if (manager == nullptr) {
            continue;
        }
        int32_t result = manager->SubscribeSystemAbilityWithPid(
            systemAbilityId, listener, true, callingPid);
        if (result != ERR_OK) {
            HILOGE("Subscribe failed, userId:%{public}d, result:%{public}d", userId, result);
            RollbackSaSubscribe(subscribedManagers, systemAbilityId, listener);
            return result;
        }
        subscribedManagers.push_back(manager);
    }

    globalSubscriptions_.systemAbilitySubscriptions.push_back(
        {systemAbilityId, remoteObject, callingPid});
    return ERR_OK;
}

int32_t UserLifecycleManager::UnSubscribeSystemAbilityForAllUsers(int32_t systemAbilityId,
    const sptr<ISystemAbilityStatusChange>& listener)
{
    if (listener == nullptr || listener->AsObject() == nullptr) {
        return ERR_INVALID_VALUE;
    }

    std::lock_guard<samgr::mutex> subscriptionLock(globalSubscriptionLock_);
    RemoveExpiredGlobalSubscriptionsLocked();
    sptr<IRemoteObject> remoteObject = listener->AsObject();
    auto subscription = std::find_if(globalSubscriptions_.systemAbilitySubscriptions.begin(),
        globalSubscriptions_.systemAbilitySubscriptions.end(),
        [systemAbilityId, remoteObject](const auto& item) {
            return item.systemAbilityId == systemAbilityId && item.remoteObject == remoteObject;
        });
    if (subscription == globalSubscriptions_.systemAbilitySubscriptions.end()) {
        return ERR_OK;
    }

    std::vector<std::shared_ptr<MultiSystemAbilityManager>> unsubscribedManagers;
    std::shared_lock<samgr::shared_mutex> userLock(userStateLock_);
    for (const auto& [userId, manager] : multiUserManagers_) {
        if (manager == nullptr) {
            continue;
        }
        int32_t result = manager->UnSubscribeSystemAbility(systemAbilityId, listener, true);
        if (result != ERR_OK) {
            HILOGE("UnSubscribeSystemAbilityForAllUsers failed, userId:%{public}d, result:%{public}d",
                userId, result);
            RollbackSaUnsubscribe(unsubscribedManagers, systemAbilityId, listener, subscription->callingPid);
            return result;
        }
        unsubscribedManagers.push_back(manager);
    }
    globalSubscriptions_.systemAbilitySubscriptions.erase(subscription);
    return ERR_OK;
}

int32_t UserLifecycleManager::SubscribeSystemProcessForAllUsers(
    const sptr<ISystemProcessStatusChange>& listener, bool& subscriptionAdded)
{
    subscriptionAdded = false;
    if (listener == nullptr || listener->AsObject() == nullptr) {
        return ERR_INVALID_VALUE;
    }

    std::lock_guard<samgr::mutex> subscriptionLock(globalSubscriptionLock_);
    RemoveExpiredGlobalSubscriptionsLocked();
    sptr<IRemoteObject> remoteObject = listener->AsObject();
    auto existing = std::find_if(globalSubscriptions_.systemProcessSubscriptions.begin(),
        globalSubscriptions_.systemProcessSubscriptions.end(), [remoteObject](const auto& subscription) {
            return subscription == remoteObject;
        });
    if (existing != globalSubscriptions_.systemProcessSubscriptions.end()) {
        return ERR_OK;
    }

    std::vector<std::shared_ptr<MultiSystemAbilityManager>> subscribedManagers;
    std::shared_lock<samgr::shared_mutex> userLock(userStateLock_);
    for (const auto& [userId, manager] : multiUserManagers_) {
        if (manager == nullptr) {
            continue;
        }
        int32_t result = manager->SubscribeSystemProcess(listener, true);
        if (result != ERR_OK) {
            HILOGE("SubscribeSystemProcessForAllUsers failed, userId:%{public}d, result:%{public}d",
                userId, result);
            RollbackProcessSubscribe(subscribedManagers, listener);
            return result;
        }
        subscribedManagers.push_back(manager);
    }

    globalSubscriptions_.systemProcessSubscriptions.push_back(remoteObject);
    subscriptionAdded = true;
    return ERR_OK;
}

int32_t UserLifecycleManager::UnSubscribeSystemProcessForAllUsers(
    const sptr<ISystemProcessStatusChange>& listener)
{
    if (listener == nullptr || listener->AsObject() == nullptr) {
        return ERR_INVALID_VALUE;
    }

    std::lock_guard<samgr::mutex> subscriptionLock(globalSubscriptionLock_);
    RemoveExpiredGlobalSubscriptionsLocked();
    sptr<IRemoteObject> remoteObject = listener->AsObject();
    auto subscription = std::find_if(globalSubscriptions_.systemProcessSubscriptions.begin(),
        globalSubscriptions_.systemProcessSubscriptions.end(), [remoteObject](const auto& item) {
            return item == remoteObject;
        });
    if (subscription == globalSubscriptions_.systemProcessSubscriptions.end()) {
        return ERR_OK;
    }

    std::vector<std::shared_ptr<MultiSystemAbilityManager>> unsubscribedManagers;
    std::shared_lock<samgr::shared_mutex> userLock(userStateLock_);
    for (const auto& [userId, manager] : multiUserManagers_) {
        if (manager == nullptr) {
            continue;
        }
        int32_t result = manager->UnSubscribeSystemProcess(listener, true);
        if (result != ERR_OK) {
            HILOGE("UnSubscribeSystemProcessForAllUsers failed, userId:%{public}d, result:%{public}d",
                userId, result);
            RollbackProcessUnsubscribe(unsubscribedManagers, listener);
            return result;
        }
        unsubscribedManagers.push_back(manager);
    }
    globalSubscriptions_.systemProcessSubscriptions.erase(subscription);
    return ERR_OK;
}

} // namespace OHOS
