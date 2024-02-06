/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *e
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "device_param_collect.h"
#include "parameter.h"
#include "parameters.h"
#include "sa_profiles.h"
#include "sam_log.h"
#include "system_ability_manager.h"
#include "system_ability_status_change_stub.h"

using namespace std;

namespace OHOS {
namespace {
constexpr int32_t PARAM_WATCHER_DISTRIBUTED_SERVICE_ID = 3901;
}
static void DeviceParamCallback(const char* key, const char* value, void* context)
{
    HILOGI("key:%{public}s, value:%{public}s", key, value);
    OnDemandEvent event = {PARAM, key, value};
    DeviceParamCollect* deviceParamCollect = static_cast<DeviceParamCollect*>(context);
    if (deviceParamCollect == nullptr) {
        return;
    }
    deviceParamCollect->ReportEvent(event);
}

DeviceParamCollect::DeviceParamCollect(const sptr<IReport>& report)
    : ICollectPlugin(report)
{
}

bool DeviceParamCollect::CheckCondition(const OnDemandCondition& condition)
{
    std::string value = system::GetParameter(condition.name, "");
    return value == condition.value;
}

void DeviceParamCollect::Init(const std::list<SaProfile>& saProfiles)
{
    std::lock_guard<std::mutex> autoLock(paramLock_);
    HILOGI("DeviceParamCollect Init begin");
    for (auto saProfile : saProfiles) {
        for (auto onDemandEvent : saProfile.startOnDemand.onDemandEvents) {
            if (onDemandEvent.eventId == PARAM) {
                pendingParams_.insert(onDemandEvent.name);
            }
        }
        for (auto onDemandEvent : saProfile.stopOnDemand.onDemandEvents) {
            if (onDemandEvent.eventId == PARAM) {
                pendingParams_.insert(onDemandEvent.name);
            }
        }
    }
}

int32_t DeviceParamCollect::OnStart()
{
    HILOGI("DeviceParamCollect OnStart called");
    sptr<SystemAbilityStatusChange> statusChangeListener = new SystemAbilityStatusChange();
    statusChangeListener->Init(this);
    SystemAbilityManager::GetInstance()->SubscribeSystemAbility(PARAM_WATCHER_DISTRIBUTED_SERVICE_ID,
        statusChangeListener);
    return ERR_OK;
}

int32_t DeviceParamCollect::OnStop()
{
    HILOGI("DeviceParamCollect OnStop called");
    return ERR_OK;
}

void DeviceParamCollect::WatchParameters()
{
    std::lock_guard<std::mutex> autoLock(paramLock_);
    for (auto param : pendingParams_) {
        HILOGD("DeviceParamCollect watch param: %{puhlic}s", param.c_str());
        int32_t result = WatchParameter(param.c_str(), DeviceParamCallback, this);
        if (result != ERR_OK) {
            HILOGE("DeviceParamCollect watch events: %{public}s failed", param.c_str());
            continue;
        }
        params_.insert(param);
    }
    pendingParams_.clear();
}

int32_t DeviceParamCollect::AddCollectEvent(const OnDemandEvent& event)
{
    std::lock_guard<std::mutex> autoLock(paramLock_);
    auto iter = params_.find(event.name);
    if (iter != params_.end()) {
        return ERR_OK;
    }
    HILOGI("DeviceParamCollect add collect events: %{public}s", event.name.c_str());
    int32_t result = WatchParameter(event.name.c_str(), DeviceParamCallback, this);
    if (result == ERR_OK) {
        params_.insert(event.name);
    }
    return result;
}

int32_t DeviceParamCollect::RemoveUnusedEvent(const OnDemandEvent& event)
{
    std::lock_guard<std::mutex> autoLock(paramLock_);
    auto iter = params_.find(event.name);
    if (iter != params_.end()) {
        int32_t result = RemoveParameterWatcher(event.name.c_str(), nullptr, nullptr);
        if (result != ERR_OK) {
            HILOGE("DeviceParamCollect RemoveUnusedEvent failed");
            return result;
        }
        HILOGI("DeviceParamCollect remove event name: %{public}s", event.name.c_str());
        params_.erase(iter);
    }
    return ERR_OK;
}

void SystemAbilityStatusChange::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    HILOGI("OnAddSystemAbility SA:%{public}d", systemAbilityId);
    switch (systemAbilityId) {
        case PARAM_WATCHER_DISTRIBUTED_SERVICE_ID: {
            if (deviceParamCollect_ == nullptr) {
                HILOGE("DeviceParamCollect is nullptr");
                return;
            }
            auto task = [this] () {
                deviceParamCollect_->WatchParameters();
            };
            deviceParamCollect_->PostDelayTask(task, 0);
            break;
        }
        default:
            HILOGE("OnAddSystemAbility unhandled SA:%{public}d", systemAbilityId);
    }
}

void SystemAbilityStatusChange::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    HILOGI("OnRemoveSystemAbility: start!");
}

void SystemAbilityStatusChange::Init(const sptr<DeviceParamCollect>& deviceParamCollect)
{
    deviceParamCollect_ = deviceParamCollect;
}
}  // namespace OHOS
