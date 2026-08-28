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

#include "multi_system_ability_manager.h"
#include "ability_death_recipient.h"
#ifdef PREFERENCES_ENABLE
#include "device_timed_collect_tool.h"
#endif
#include "sam_log.h"
#include "samgr_xcollie.h"
#include "service_control.h"
#include "string_ex.h"
#include "system_ability_manager.h"
#include "system_ability_manager_util.h"
#include "ipc_skeleton.h"
#include "hisysevent_adapter.h"
#include "datetime_ex.h"

namespace OHOS {

MultiSystemAbilityManager::MultiSystemAbilityManager(int32_t userId)
    : MultiSystemAbilityManager(userId, GlobalSubscriptionInfo())
{}

MultiSystemAbilityManager::MultiSystemAbilityManager(
    int32_t userId, const GlobalSubscriptionInfo& globalSubscriptions)
    : userId_(userId), globalSubscriptions_(globalSubscriptions)
{
    logPrefix_ = "[U" + std::to_string(userId_) + "] ";
}

MultiSystemAbilityManager::~MultiSystemAbilityManager() {}

int32_t MultiSystemAbilityManager::Init(const std::list<SaProfile>& saProfiles)
{
    selfPtr_ = std::shared_ptr<BaseSystemAbilityManager>(this, [](BaseSystemAbilityManager*) {});
    abilityDeath_ = sptr<IRemoteObject::DeathRecipient>(new AbilityDeathRecipient(weak_from_this()));
    systemProcessDeath_ = sptr<IRemoteObject::DeathRecipient>(new SystemProcessDeathRecipient(weak_from_this()));
    abilityStatusDeath_ = sptr<IRemoteObject::DeathRecipient>(new AbilityStatusDeathRecipient(weak_from_this()));
    abilityCallbackDeath_ = sptr<IRemoteObject::DeathRecipient>(new AbilityCallbackDeathRecipient(weak_from_this()));
    remoteCallbackDeath_ = sptr<IRemoteObject::DeathRecipient>(new RemoteCallbackDeathRecipient(weak_from_this()));
    if (workHandler_ == nullptr) {
        workHandler_ = std::make_shared<FFRTHandler>("workHandler");
    }
    reportEventTimer_ = std::make_unique<Utils::Timer>("DfxReporter", -1);

    auto systemAbilityManager = SystemAbilityManager::GetInstance();
    if (systemAbilityManager == nullptr) {
        HILOGE("SystemAbilityManager is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::set<int32_t> multiInstanceSaIds = systemAbilityManager->GetMultiInstanceSaIds();

    std::list<SaProfile> filteredProfiles;
    InitSaProfiles(saProfiles, multiInstanceSaIds, filteredProfiles);

    abilityStateScheduler_ = std::make_shared<SystemAbilityStateScheduler>(weak_from_this());
    if (abilityStateScheduler_ != nullptr) {
        abilityStateScheduler_->Init(filteredProfiles);
    }

    collectManager_ = sptr<DeviceStatusCollectManager>(new DeviceStatusCollectManager(weak_from_this()));
    if (collectManager_ != nullptr) {
        collectManager_->Init(filteredProfiles);
    }

    int32_t result = InitGlobalSubscriptions();
    if (result != ERR_OK) {
        HILOGE("Init subscriptions failed, userId:%{public}d, result:%{public}d", userId_, result);
        return result;
    }

    HILOGI("MultiSAManager Init done, userId:%{public}d, saCount:%{public}zu", userId_, filteredProfiles.size());
    return ERR_OK;
}

void MultiSystemAbilityManager::InitSaProfiles(const std::list<SaProfile>& saProfiles,
    const std::set<int32_t>& multiInstanceSaIds, std::list<SaProfile>& filteredProfiles)
{
    for (const auto& saProfile : saProfiles) {
        if (multiInstanceSaIds.count(saProfile.saId) > 0) {
            filteredProfiles.push_back(saProfile);
        }
    }

    {
        std::lock_guard<samgr::mutex> autoLock(saProfileMapLock_);
        saProfileMap_.clear();
        onDemandSaIdsSet_.clear();
        for (const auto& saProfile : filteredProfiles) {
            SamgrUtil::FilterCommonSaProfile(saProfile, saProfileMap_[saProfile.saId]);
            if (!saProfile.runOnCreate) {
                onDemandSaIdsSet_.insert(saProfile.saId);
            }
        }
    }
}

void MultiSystemAbilityManager::Destroy()
{
    HILOGI("MultiSAManager Destroy for userId:%{public}d", userId_);
    BaseSystemAbilityManager::Destroy();
    {
        std::lock_guard<samgr::mutex> autoLock(subscriberInfoLock_);
        subscriberInfoMap_.clear();
    }
    HILOGI("MultiSAManager Destroy done for userId:%{public}d", userId_);
}

void MultiSystemAbilityManager::AddSubscriberInfo(int32_t systemAbilityId,
    const sptr<ISystemAbilityStatusChange>& listener, bool foregroundOnly)
{
    if (listener == nullptr || listener->AsObject() == nullptr) {
        return;
    }
    auto remoteObject = listener->AsObject();
    std::lock_guard<samgr::mutex> autoLock(subscriberInfoLock_);
    auto& subscriberInfos = subscriberInfoMap_[systemAbilityId];
    for (auto iter = subscriberInfos.begin(); iter != subscriberInfos.end();) {
        auto object = iter->remoteObject.promote();
        if (object == nullptr) {
            iter = subscriberInfos.erase(iter);
            continue;
        }
        if (object == remoteObject) {
            if (foregroundOnly) {
                iter->hasForegroundSubscription = true;
            } else {
                iter->hasDirectSubscription = true;
            }
            return;
        }
        ++iter;
    }
    SubscriberInfo subscriberInfo;
    subscriberInfo.remoteObject = remoteObject;
    subscriberInfo.hasDirectSubscription = !foregroundOnly;
    subscriberInfo.hasForegroundSubscription = foregroundOnly;
    subscriberInfos.push_back(subscriberInfo);
}

bool MultiSystemAbilityManager::RemoveSubscriberInfo(int32_t systemAbilityId,
    const sptr<ISystemAbilityStatusChange>& listener, bool foregroundOnly)
{
    if (listener == nullptr || listener->AsObject() == nullptr) {
        return false;
    }
    auto remoteObject = listener->AsObject();
    std::lock_guard<samgr::mutex> autoLock(subscriberInfoLock_);
    auto mapIter = subscriberInfoMap_.find(systemAbilityId);
    if (mapIter == subscriberInfoMap_.end()) {
        return true;
    }
    auto& subscriberInfos = mapIter->second;
    for (auto iter = subscriberInfos.begin(); iter != subscriberInfos.end();) {
        auto object = iter->remoteObject.promote();
        if (object == nullptr) {
            iter = subscriberInfos.erase(iter);
            continue;
        }
        if (object == remoteObject) {
            if (foregroundOnly) {
                iter->hasForegroundSubscription = false;
            } else {
                iter->hasDirectSubscription = false;
            }
            if (iter->hasDirectSubscription || iter->hasForegroundSubscription) {
                return false;
            }
            subscriberInfos.erase(iter);
            if (subscriberInfos.empty()) {
                subscriberInfoMap_.erase(mapIter);
            }
            return true;
        }
        ++iter;
    }
    if (subscriberInfos.empty()) {
        subscriberInfoMap_.erase(mapIter);
    }
    return true;
}

bool MultiSystemAbilityManager::ShouldNotifySubscriber(int32_t systemAbilityId,
    const sptr<ISystemAbilityStatusChange>& listener)
{
    if (listener == nullptr || listener->AsObject() == nullptr) {
        return false;
    }
    auto remoteObject = listener->AsObject();
    std::lock_guard<samgr::mutex> autoLock(subscriberInfoLock_);
    auto mapIter = subscriberInfoMap_.find(systemAbilityId);
    if (mapIter == subscriberInfoMap_.end()) {
        return false;
    }
    auto& subscriberInfos = mapIter->second;
    for (auto iter = subscriberInfos.begin(); iter != subscriberInfos.end();) {
        auto object = iter->remoteObject.promote();
        if (object == nullptr) {
            iter = subscriberInfos.erase(iter);
            continue;
        }
        if (object == remoteObject) {
            if (iter->hasDirectSubscription) {
                return true;
            }
            auto samgr = SystemAbilityManager::GetInstance();
            return !iter->hasForegroundSubscription ||
                (samgr != nullptr && userId_ == samgr->GetForegroundUserId());
        }
        ++iter;
    }
    if (subscriberInfos.empty()) {
        subscriberInfoMap_.erase(mapIter);
    }
    return false;
}

int32_t MultiSystemAbilityManager::SubscribeSystemAbility(int32_t systemAbilityId,
    const sptr<ISystemAbilityStatusChange>& listener)
{
    return SubscribeSystemAbility(systemAbilityId, listener, false);
}

int32_t MultiSystemAbilityManager::SubscribeSystemAbility(int32_t systemAbilityId,
    const sptr<ISystemAbilityStatusChange>& listener, bool foregroundOnly)
{
    return SubscribeSystemAbilityWithPid(
        systemAbilityId, listener, foregroundOnly, IPCSkeleton::GetCallingPid());
}

int32_t MultiSystemAbilityManager::SubscribeSystemAbilityWithPid(int32_t systemAbilityId,
    const sptr<ISystemAbilityStatusChange>& listener, bool foregroundOnly, int32_t callingPid)
{
    AddSubscriberInfo(systemAbilityId, listener, foregroundOnly);
    int32_t result = SubscribeSystemAbilityInner(systemAbilityId, listener, callingPid);
    if (result != ERR_OK) {
        RemoveSubscriberInfo(systemAbilityId, listener, foregroundOnly);
    }
    return result;
}

int32_t MultiSystemAbilityManager::UnSubscribeSystemAbility(int32_t systemAbilityId,
    const sptr<ISystemAbilityStatusChange>& listener)
{
    return UnSubscribeSystemAbility(systemAbilityId, listener, false);
}

int32_t MultiSystemAbilityManager::UnSubscribeSystemAbility(int32_t systemAbilityId,
    const sptr<ISystemAbilityStatusChange>& listener, bool foregroundOnly)
{
    if (!CheckInputSysAbilityId(systemAbilityId) || listener == nullptr || listener->AsObject() == nullptr) {
        HILOGW("UnSubscribeSystemAbility SAId or listener invalid");
        return ERR_INVALID_VALUE;
    }
    if (!RemoveSubscriberInfo(systemAbilityId, listener, foregroundOnly)) {
        return ERR_OK;
    }
    return BaseSystemAbilityManager::UnSubscribeSystemAbility(systemAbilityId, listener);
}

void MultiSystemAbilityManager::UnSubscribeSystemAbility(const sptr<IRemoteObject>& remoteObject)
{
    if (remoteObject == nullptr) {
        return;
    }
    BaseSystemAbilityManager::UnSubscribeSystemAbility(remoteObject);
    std::lock_guard<samgr::mutex> autoLock(subscriberInfoLock_);
    for (auto mapIter = subscriberInfoMap_.begin(); mapIter != subscriberInfoMap_.end();) {
        auto& subscriberInfos = mapIter->second;
        subscriberInfos.remove_if([&remoteObject](const SubscriberInfo& info) {
            auto object = info.remoteObject.promote();
            return object == nullptr || object == remoteObject;
        });
        if (subscriberInfos.empty()) {
            mapIter = subscriberInfoMap_.erase(mapIter);
        } else {
            ++mapIter;
        }
    }
}

int32_t MultiSystemAbilityManager::InitGlobalSubscriptions()
{
    GlobalSubscriptionInfo subscriptions = globalSubscriptions_;
    globalSubscriptions_ = {};
    std::vector<std::pair<int32_t, sptr<ISystemAbilityStatusChange>>> restoredSaSubscriptions;
    std::vector<sptr<ISystemProcessStatusChange>> restoredProcessSubscriptions;

    for (const auto& subscription : subscriptions.systemAbilitySubscriptions) {
        sptr<IRemoteObject> remoteObject = subscription.remoteObject;
        if (remoteObject == nullptr || remoteObject->IsObjectDead()) {
            continue;
        }
        sptr<ISystemAbilityStatusChange> listener = iface_cast<ISystemAbilityStatusChange>(remoteObject);
        if (listener == nullptr) {
            continue;
        }
        int32_t result = SubscribeSystemAbilityWithPid(
            subscription.systemAbilityId, listener, true, subscription.callingPid);
        if (result != ERR_OK) {
            RollbackGlobalSubscriptions(restoredSaSubscriptions, restoredProcessSubscriptions);
            return result;
        }
        restoredSaSubscriptions.emplace_back(subscription.systemAbilityId, listener);
    }
    for (const auto& subscription : subscriptions.systemProcessSubscriptions) {
        sptr<IRemoteObject> remoteObject = subscription;
        if (remoteObject == nullptr || remoteObject->IsObjectDead()) {
            continue;
        }
        sptr<ISystemProcessStatusChange> listener = iface_cast<ISystemProcessStatusChange>(remoteObject);
        if (listener == nullptr) {
            continue;
        }
        int32_t result = SubscribeSystemProcess(listener, true);
        if (result != ERR_OK) {
            RollbackGlobalSubscriptions(restoredSaSubscriptions, restoredProcessSubscriptions);
            return result;
        }
        restoredProcessSubscriptions.push_back(listener);
    }
    return ERR_OK;
}

void MultiSystemAbilityManager::RollbackGlobalSubscriptions(
    const std::vector<std::pair<int32_t, sptr<ISystemAbilityStatusChange>>>& restoredSaSubscriptions,
    const std::vector<sptr<ISystemProcessStatusChange>>& restoredProcessSubscriptions)
{
    for (auto iter = restoredProcessSubscriptions.rbegin(); iter != restoredProcessSubscriptions.rend(); ++iter) {
        int32_t result = UnSubscribeSystemProcess(*iter, true);
        if (result != ERR_OK) {
            HILOGE("InitGlobalSubscriptions rollback process subscription failed, result:%{public}d", result);
        }
    }
    for (auto iter = restoredSaSubscriptions.rbegin(); iter != restoredSaSubscriptions.rend(); ++iter) {
        int32_t result = UnSubscribeSystemAbility(iter->first, iter->second, true);
        if (result != ERR_OK) {
            HILOGE("Rollback SA:%{public}d failed, result:%{public}d", iter->first, result);
        }
    }
}

int32_t MultiSystemAbilityManager::FindSystemAbilityNotify(int32_t systemAbilityId,
    const std::string& deviceId, int32_t code)
{
    std::lock_guard<samgr::mutex> autoLock(listenerMapLock_);
    HILOGI("FindSaNotify SA:%{public}d,%{public}d_%{public}zu", systemAbilityId, code, listenerMap_.size());
    auto iter = listenerMap_.find(systemAbilityId);
    if (iter == listenerMap_.end()) {
        return ERR_OK;
    }
    auto& listeners = iter->second;
    if (code == static_cast<int32_t>(SamgrInterfaceCode::ADD_SYSTEM_ABILITY_TRANSACTION)) {
        for (auto& item : listeners) {
            if (item.state == ListenerState::INIT) {
                NotifySystemAbilityChanged(systemAbilityId, deviceId, code, item.listener);
                item.state = ListenerState::NOTIFIED;
            } else {
                HILOGI("FindSaNotify Listener has been notified,SA:%{public}d,callingPid:%{public}d",
                    systemAbilityId, item.callingPid);
            }
        }
    } else if (code == static_cast<int32_t>(SamgrInterfaceCode::REMOVE_SYSTEM_ABILITY_TRANSACTION)) {
        for (auto& item : listeners) {
            NotifySystemAbilityChanged(systemAbilityId, deviceId, code, item.listener);
            item.state = ListenerState::INIT;
        }
    }
    return ERR_OK;
}

void MultiSystemAbilityManager::NotifySystemAbilityChanged(int32_t systemAbilityId,
    const std::string& deviceId, int32_t code, const sptr<ISystemAbilityStatusChange>& listener)
{
    if (!ShouldNotifySubscriber(systemAbilityId, listener)) {
        HILOGD("NotifySystemAbilityChanged skip foreground-only listener, SA:%{public}d, userId:%{public}d",
            systemAbilityId, userId_);
        return;
    }
    BaseSystemAbilityManager::NotifySystemAbilityChanged(systemAbilityId, deviceId, code, listener);
}

int32_t MultiSystemAbilityManager::SubscribeSystemProcess(const sptr<ISystemProcessStatusChange>& listener)
{
    return SubscribeSystemProcess(listener, false);
}

int32_t MultiSystemAbilityManager::SubscribeSystemProcess(const sptr<ISystemProcessStatusChange>& listener,
    bool foregroundOnly)
{
    if (abilityStateScheduler_ == nullptr) {
        HILOGE("abilityStateScheduler is nullptr");
        return ERR_INVALID_VALUE;
    }
    return abilityStateScheduler_->SubscribeSystemProcess(listener, foregroundOnly);
}

int32_t MultiSystemAbilityManager::UnSubscribeSystemProcess(
    const sptr<ISystemProcessStatusChange>& listener)
{
    return UnSubscribeSystemProcess(listener, false);
}

int32_t MultiSystemAbilityManager::UnSubscribeSystemProcess(
    const sptr<ISystemProcessStatusChange>& listener, bool foregroundOnly)
{
    if (abilityStateScheduler_ == nullptr) {
        HILOGE("abilityStateScheduler is nullptr");
        return ERR_INVALID_VALUE;
    }
    return abilityStateScheduler_->UnSubscribeSystemProcess(listener, foregroundOnly);
}

void MultiSystemAbilityManager::NotifyAllSAsToStop()
{
    HILOGD("NotifyAllSAsToStop userId:%{public}d", userId_);

    std::vector<std::pair<int32_t, std::u16string>> saList;
    {
        std::shared_lock<samgr::shared_mutex> lock(abilityMapLock_);
        for (const auto& [saId, saInfo] : abilityMap_) {
            CommonSaProfile profile;
            if (GetSaProfile(saId, profile)) {
                saList.emplace_back(saId, profile.process);
            }
        }
    }

    OnDemandEvent event = {INTERFACE_CALL, "userStopping"};
    for (const auto& [saId, procName] : saList) {
        StopOnDemandAbility(procName, saId, event);
        HILOGD("NotifyAllSAsToStop SA:%{public}d notified, userId:%{public}d", saId, userId_);
    }

    HILOGI("NotifyAllSAsToStop done, userId:%{public}d, saCount:%{public}zu",
        userId_, saList.size());
}

void MultiSystemAbilityManager::StopEventCollection()
{
    HILOGD("MultiSAManager StopEventCollection for userId:%{public}d", userId_);
    if (collectManager_ != nullptr) {
        collectManager_->UnInit();
    }
    if (abilityStateScheduler_ != nullptr) {
        abilityStateScheduler_->CleanFfrt();
    }
#ifdef PREFERENCES_ENABLE
    int32_t result = PreferencesUtil::DeleteUserPreferences(userId_);
    if (result != ERR_OK) {
        HILOGE("MultiSAManager delete persistence failed, userId:%{public}d, result:%{public}d",
            userId_, result);
    }
#endif
    HILOGD("MultiSAManager StopEventCollection done for userId:%{public}d", userId_);
}

int32_t MultiSystemAbilityManager::StartDynamicSystemProcess(const std::u16string& name,
    int32_t systemAbilityId, const OnDemandEvent& event)
{
    std::string processName = Str16ToStr8(name);
    std::string eventStr = std::to_string(systemAbilityId) + "#" + std::to_string(event.eventId) + "#"
        + event.name + "#" + event.value + "#" + std::to_string(event.extraDataId) + "#";
    auto extraArgv = eventStr.c_str();
    if (abilityStateScheduler_ && !abilityStateScheduler_->IsSystemProcessNeverStartedLocked(name)) {
        int32_t ret = ERR_INVALID_VALUE;
        {
            SamgrXCollie samgrXCollie("samgr--WaitUserProcessStop_" + processName);
            ret = ServiceWaitForStatusByUserId(processName.c_str(), userId_, ServiceStatus::SERVICE_STOPPED, 1);
        }
        if (ret != ERR_OK) {
            HILOGE("ServiceWaitForStatusByUserId proc:%{public}s,SA:%{public}d timeout",
                processName.c_str(), systemAbilityId);
        }
    }
    int64_t begin = GetTickCount();
    int32_t result = ERR_INVALID_VALUE;
    if (!IsInitBootFinished()) {
        result = ServiceControlWithExtraByUserId(processName.c_str(),
            ServiceAction::START, userId_, &extraArgv, 1);
    } else {
        SamgrXCollie samgrXCollie("samgr--startProc_" + ToString(systemAbilityId));
        result = ServiceControlWithExtraByUserId(processName.c_str(),
            ServiceAction::START, userId_, &extraArgv, 1);
    }
    int64_t duration = GetTickCount() - begin;
    auto callingPid = IPCSkeleton::GetCallingPid();
    auto callingUid = IPCSkeleton::GetCallingUid();
    if (result != ERR_OK) {
        ReportProcessStartFail(processName, callingPid, callingUid, "err:" + ToString(result));
    }
    HILOGI("StartUserProc:%{public}s,SA:%{public}d,ret:%{public}d,%{public}" PRId64 "ms,userId:%{public}d",
        processName.c_str(), systemAbilityId, result, duration, userId_);
    return result;
}

} // namespace OHOS
