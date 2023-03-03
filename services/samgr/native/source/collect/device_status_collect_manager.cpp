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
#ifdef SUPPORT_DEVICE_MANAGER
    sptr<ICollectPlugin> networkingCollect = new DeviceNetworkingCollect(this);
    collectPluginMap_[DEVICE_ONLINE] = networkingCollect;
#endif
#ifdef SUPPORT_COMMON_EVENT
    sptr<CommonEventCollect> eventStatuscollect = new CommonEventCollect(this);
    eventStatuscollect->Init(onDemandSaProfiles_);
    collectPluginMap_[COMMON_EVENT] = eventStatuscollect;
#endif
    sptr<DeviceTimedCollect> timedCollect = new DeviceTimedCollect(this);
    timedCollect->Init(onDemandSaProfiles_);
    collectPluginMap_[TIMED_EVENT] = timedCollect;
    StartCollect();
    HILOGI("DeviceStatusCollectManager Init end");
}

void DeviceStatusCollectManager::FilterOnDemandSaProfiles(const std::list<SaProfile>& saProfiles)
{
    for (auto& saProfile : saProfiles) {
        if (saProfile.startOnDemand.empty() && saProfile.stopOnDemand.empty()) {
            continue;
        }
        onDemandSaProfiles_.emplace_back(saProfile);
    }
}

void DeviceStatusCollectManager::GetSaControlListByEvent(const OnDemandEvent& event,
    std::list<SaControlInfo>& saControlList)
{
    for (auto& profile : onDemandSaProfiles_) {
        // start on demand
        for (auto iterStart = profile.startOnDemand.begin(); iterStart != profile.startOnDemand.end(); iterStart++) {
            if (IsSameEvent(event, *iterStart) && CheckConditions(*iterStart)) {
                // maybe the process is being killed, let samgr make decisions.
                SaControlInfo control = { START_ON_DEMAND, profile.saId, iterStart->enableOnce };
                saControlList.emplace_back(control);
                break;
            }
        }
        // stop on demand
        for (auto iterStop = profile.stopOnDemand.begin(); iterStop != profile.stopOnDemand.end(); iterStop++) {
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
    return (ev1.eventId == ev2.eventId && ev1.name == ev2.name && ev1.value == ev2.value);
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
}  // namespace OHOS
