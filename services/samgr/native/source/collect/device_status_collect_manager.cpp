/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "device_networking_collect.h"
#include "sam_log.h"
#include "system_ability_manager.h"

namespace OHOS {
void DeviceStatusCollectManager::Init(const std::list<SaProfile>& saProfiles)
{
    HILOGI("DeviceStatusCollectManager Init begin");
    FilterOnDemandSaProfiles(saProfiles);
    auto runner = AppExecFwk::EventRunner::Create("collect");
    collectHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    sptr<ICollectPlugin> networkingCollect = new DeviceNetworkingCollect(this);
    collectPluginMap_[DEVICE_ONLINE] = networkingCollect;
    StartCollect();
    HILOGI("DeviceStatusCollectManager Init end");
}

void DeviceStatusCollectManager::FilterOnDemandSaProfiles(const std::list<SaProfile>& saProfiles)
{
    for (auto& saProfile : saProfiles) {
        if (saProfile.startOnDemand.empty() || saProfile.stopOnDemand.empty()) {
            continue;
        }
        onDemandSaProfiles.push_back(saProfile);
    }
}

void DeviceStatusCollectManager::GetSaControlListByEvent(const OnDemandEvent& event,
    std::list<SaControlInfo>& saControlList)
{
    for (auto& profile : onDemandSaProfiles) {
        // start on demand
        for (auto iterStart = profile.startOnDemand.begin(); iterStart != profile.startOnDemand.end(); iterStart++) {
            if (IsSameEvent(event, *iterStart)) {
                // maybe the process is being killed, let samgr make decisions.
                SaControlInfo control = { START_ON_DEMAND, profile.saId };
                saControlList.push_back(control);
                break;
            }
        }
        // stop on demand
        for (auto iterStop = profile.stopOnDemand.begin(); iterStop != profile.stopOnDemand.end(); iterStop++) {
            if (IsSameEvent(event, *iterStop)) {
                // maybe the process is starting, let samgr make decisions.
                SaControlInfo control = { STOP_ON_DEMAND, profile.saId };
                saControlList.emplace_back(control);
                break;
            }
        }
    }
    HILOGI("DeviceStatusCollectManager saControlList size %{public}zu", saControlList.size());
}

bool DeviceStatusCollectManager::IsSameEvent(const OnDemandEvent& ev1, const OnDemandEvent& ev2)
{
    if (ev1.eventId == ev2.eventId && ev1.name == ev2.name && ev1.value == ev2.value) {
        return true;
    }
    return false;
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
}  // namespace OHOS
