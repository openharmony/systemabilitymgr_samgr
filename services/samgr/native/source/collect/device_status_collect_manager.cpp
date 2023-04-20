/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "device_status_collect_manager.h"

#include "datetime_ex.h"
#include "device_timed_collect.h"
#ifdef SUPPORT_DEVICE_MANAGER
#include "device_networking_collect.h"
#endif
#ifdef SUPPORT_COMMON_EVENT
#include "common_event_collect.h"
#endif
#ifdef SUPPORT_SWITCH_COLLECT
#include "device_switch_collect.h"
#endif
#include "device_param_collect.h"
#include "memory_guard.h"
#include "sam_log.h"
#include "system_ability_manager.h"

namespace OHOS {
namespace {
constexpr int32_t SECOND = 1000;
}
void DeviceStatusCollectManager::Init(const std::list<SaProfile>& saProfiles)
{
    HILOGI("DeviceStatusCollectManager Init begin");
    FilterOnDemandSaProfiles(saProfiles);
    auto runner = AppExecFwk::EventRunner::Create("collect");
    collectHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    collectHandler_->PostTask([]() { Samgr::MemoryGuard cacheGuard; });
    sptr<ICollectPlugin> deviceParamCollect = new DeviceParamCollect(this);
    deviceParamCollect->Init(saProfiles);
    collectPluginMap_[PARAM] = deviceParamCollect;
#ifdef SUPPORT_DEVICE_MANAGER
    sptr<ICollectPlugin> networkingCollect = new DeviceNetworkingCollect(this);
    collectPluginMap_[DEVICE_ONLINE] = networkingCollect;
#endif
#ifdef SUPPORT_COMMON_EVENT
    sptr<ICollectPlugin> eventStatuscollect = new CommonEventCollect(this);
    eventStatuscollect->Init(onDemandSaProfiles_);
    collectPluginMap_[COMMON_EVENT] = eventStatuscollect;
#endif
#ifdef SUPPORT_SWITCH_COLLECT
    sptr<ICollectPlugin> deviceSwitchCollect = new DeviceSwitchCollect(this);
    deviceSwitchCollect->Init(saProfiles);
    collectPluginMap_[SETTING_SWITCH] = deviceSwitchCollect;
#endif
    sptr<ICollectPlugin> timedCollect = new DeviceTimedCollect(this);
    timedCollect->Init(onDemandSaProfiles_);
    collectPluginMap_[TIMED_EVENT] = timedCollect;
    StartCollect();
    HILOGI("DeviceStatusCollectManager Init end");
}

void DeviceStatusCollectManager::FilterOnDemandSaProfiles(const std::list<SaProfile>& saProfiles)
{
    std::unique_lock<std::shared_mutex> writeLock(saProfilesLock_);
    for (auto& saProfile : saProfiles) {
        if (saProfile.startOnDemand.onDemandEvents.empty() && saProfile.stopOnDemand.onDemandEvents.empty()) {
            continue;
        }
        onDemandSaProfiles_.emplace_back(saProfile);
    }
}

void DeviceStatusCollectManager::GetSaControlListByEvent(const OnDemandEvent& event,
    std::list<SaControlInfo>& saControlList)
{
    std::shared_lock<std::shared_mutex> readLock(saProfilesLock_);
    for (auto& profile : onDemandSaProfiles_) {
        // start on demand
        for (auto iterStart = profile.startOnDemand.onDemandEvents.begin();
            iterStart != profile.startOnDemand.onDemandEvents.end(); iterStart++) {
            if (IsSameEvent(event, *iterStart) && CheckConditions(*iterStart)) {
                // maybe the process is being killed, let samgr make decisions.
                SaControlInfo control = { START_ON_DEMAND, profile.saId, iterStart->enableOnce };
                saControlList.emplace_back(control);
                break;
            }
        }
        // stop on demand
        for (auto iterStop = profile.stopOnDemand.onDemandEvents.begin();
            iterStop != profile.stopOnDemand.onDemandEvents.end(); iterStop++) {
            if (IsSameEvent(event, *iterStop) && CheckConditions(*iterStop)) {
                // maybe the process is starting, let samgr make decisions.
                SaControlInfo control = { STOP_ON_DEMAND, profile.saId, iterStop->enableOnce };
                saControlList.emplace_back(control);
                break;
            }
        }
    }
    HILOGD("DeviceStatusCollectManager saControlList size %{public}zu", saControlList.size());
}

bool DeviceStatusCollectManager::IsSameEvent(const OnDemandEvent& ev1, const OnDemandEvent& ev2)
{
    return (ev1.eventId == ev2.eventId && ev1.name == ev2.name && (ev1.value == ev2.value || "" == ev2.value));
}

bool DeviceStatusCollectManager::CheckConditions(const OnDemandEvent& onDemandEvent)
{
    if (onDemandEvent.conditions.empty()) {
        return true;
    }
    for (auto& condition : onDemandEvent.conditions) {
        if (collectPluginMap_.count(condition.eventId) == 0) {
            HILOGE("not support condition: %{public}d", condition.eventId);
            return false;
        }
        if (collectPluginMap_[condition.eventId] == nullptr) {
            HILOGE("not support condition: %{public}d", condition.eventId);
            return false;
        }
        bool ret = collectPluginMap_[condition.eventId]->CheckCondition(condition);
        HILOGI("CheckCondition condition: %{public}s, value: %{public}s, ret: %{public}s",
            condition.name.c_str(), condition.value.c_str(), ret ? "pass" : "not pass");
        if (!ret) {
            return false;
        }
    }
    return true;
}

void DeviceStatusCollectManager::UnInit()
{
    for (auto& iter : collectPluginMap_) {
        iter.second->OnStop();
    }
    collectPluginMap_.clear();

    if (collectHandler_ != nullptr) {
        collectHandler_->SetEventRunner(nullptr);
        collectHandler_ = nullptr;
    }
}

void DeviceStatusCollectManager::StartCollect()
{
    HILOGI("DeviceStatusCollectManager OnStart begin");
    if (collectHandler_ == nullptr) {
        return;
    }
    auto callback = [this] () {
        for (auto& iter : collectPluginMap_) {
            iter.second->OnStart();
        }
    };
    collectHandler_->PostTask(callback);
}

void DeviceStatusCollectManager::ReportEvent(const OnDemandEvent& event)
{
    if (collectHandler_ == nullptr) {
        HILOGW("DeviceStatusCollectManager collectHandler_ is nullptr");
        return;
    }
    std::list<SaControlInfo> saControlList;
    GetSaControlListByEvent(event, saControlList);
    if (saControlList.empty()) {
        HILOGW("DeviceStatusCollectManager no matched event");
        return;
    }
    auto callback = [event, saControlList = std::move(saControlList)] () {
        SystemAbilityManager::GetInstance()->ProcessOnDemandEvent(event, saControlList);
    };
    collectHandler_->PostTask(callback);
}

void DeviceStatusCollectManager::PostDelayTask(std::function<void()> callback, int32_t delayTime)
{
    HILOGI("DeviceStatusCollectManager PostDelayTask begin");
    collectHandler_->PostTask(callback, delayTime * SECOND);
}

int32_t DeviceStatusCollectManager::GetOnDemandReasonExtraData(int64_t extraDataId, OnDemandReasonExtraData& extraData)
{
    HILOGI("DeviceStatusCollectManager GetOnDemandReasonExtraData begin");
    if (collectPluginMap_.count(COMMON_EVENT) == 0) {
        HILOGE("not support get extra data");
        return ERR_INVALID_VALUE;
    }
    if (collectPluginMap_[COMMON_EVENT] == nullptr) {
        HILOGE("CommonEventCollect is nullptr");
        return ERR_INVALID_VALUE;
    }
    if (!collectPluginMap_[COMMON_EVENT]->GetOnDemandReasonExtraData(extraDataId, extraData)) {
        HILOGE("get extra data failed");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

int32_t DeviceStatusCollectManager::AddCollectEvents(const std::vector<OnDemandEvent>& events)
{
    if (events.size() == 0) {
        return ERR_OK;
    }
    for (auto& event : events) {
        if (collectPluginMap_.count(event.eventId) == 0) {
            HILOGE("not support eventId: %{public}d", event.eventId);
            return ERR_INVALID_VALUE;
        }
        if (collectPluginMap_[event.eventId] == nullptr) {
            HILOGE("not support eventId: %{public}d", event.eventId);
            return ERR_INVALID_VALUE;
        }
        int32_t ret = collectPluginMap_[event.eventId]->AddCollectEvent(event);
        if (ret != ERR_OK) {
            HILOGE("add collect event failed, eventId: %{public}d", event.eventId);
            return ret;
        }
    }
    return ERR_OK;
}

int32_t DeviceStatusCollectManager::GetOnDemandEvents(int32_t systemAbilityId, OnDemandPolicyType type,
    std::vector<OnDemandEvent>& events)
{
    HILOGI("DeviceStatusCollectManager GetOnDemandEvents begin");
    std::shared_lock<std::shared_mutex> readLock(saProfilesLock_);
    auto iter = std::find_if(onDemandSaProfiles_.begin(), onDemandSaProfiles_.end(), [systemAbilityId](auto saProfile) {
        return saProfile.saId == systemAbilityId;
    });
    if (iter == onDemandSaProfiles_.end()) {
        HILOGI("DeviceStatusCollectManager GetOnDemandEvents invalid saId:%{public}d", systemAbilityId);
        return ERR_INVALID_VALUE;
    }
    if (type == OnDemandPolicyType::START_POLICY) {
        events = (*iter).startOnDemand.onDemandEvents;
    } else if (type == OnDemandPolicyType::STOP_POLICY) {
        events = (*iter).stopOnDemand.onDemandEvents;
    } else {
        HILOGE("DeviceStatusCollectManager GetOnDemandEvents invalid policy types");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

int32_t DeviceStatusCollectManager::UpdateOnDemandEvents(int32_t systemAbilityId, OnDemandPolicyType type,
    const std::vector<OnDemandEvent>& events)
{
    HILOGI("DeviceStatusCollectManager UpdateOnDemandEvents begin");
    std::unique_lock<std::shared_mutex> writeLock(saProfilesLock_);
    auto iter = std::find_if(onDemandSaProfiles_.begin(), onDemandSaProfiles_.end(), [systemAbilityId](auto saProfile) {
        return saProfile.saId == systemAbilityId;
    });
    if (iter == onDemandSaProfiles_.end()) {
        HILOGI("DeviceStatusCollectManager UpdateOnDemandEvents invalid saId:%{public}d", systemAbilityId);
        return ERR_INVALID_VALUE;
    }
    if (AddCollectEvents(events) != ERR_OK) {
        HILOGI("DeviceStatusCollectManager AddCollectEvents failed saId:%{public}d", systemAbilityId);
        return ERR_INVALID_VALUE;
    }

    if (type == OnDemandPolicyType::START_POLICY) {
        (*iter).startOnDemand.onDemandEvents = events;
    } else if (type == OnDemandPolicyType::STOP_POLICY) {
        (*iter).stopOnDemand.onDemandEvents = events;
    } else {
        HILOGE("DeviceStatusCollectManager UpdateOnDemandEvents policy types");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}
}  // namespace OHOS
