/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "device_timed_collect.h"

#include <algorithm>

#include "sam_log.h"
#include "sa_profiles.h"
#include "system_ability_definition.h"
#include "system_ability_manager.h"

using namespace std;

namespace OHOS {
namespace {
const std::string LOOP_EVENT = "loopevent";
constexpr int32_t MIN_INTERVAL = 30;
}

DeviceTimedCollect::DeviceTimedCollect(const sptr<IReport>& report)
    : ICollectPlugin(report)
{
}

int32_t DeviceTimedCollect::Init(const std::list<SaProfile>& saProfiles)
{
    lock_guard<mutex> autoLock(taskLock_);
    for (auto& saProfile : saProfiles) {
        for (auto& onDemandEvent : saProfile.startOnDemand.onDemandEvents) {
            SaveTimedEvent(onDemandEvent);
        }
        for (auto& onDemandEvent : saProfile.stopOnDemand.onDemandEvents) {
            SaveTimedEvent(onDemandEvent);
        }
    }
    HILOGD("DeviceTimedCollect timedSet count: %{public}zu", timedSet_.size());
    return ERR_OK;
}

void DeviceTimedCollect::SaveTimedEvent(const OnDemandEvent& onDemandEvent)
{
    if (onDemandEvent.eventId == TIMED_EVENT && onDemandEvent.name == LOOP_EVENT) {
        HILOGI("DeviceTimedCollect save timed task: %{public}s", onDemandEvent.value.c_str());
        int32_t interval = atoi(onDemandEvent.value.c_str());
        if (interval >= MIN_INTERVAL) {
            timedSet_.insert(interval);
        } else {
            HILOGE("DeviceTimedCollect invalid interval %{public}s", onDemandEvent.value.c_str());
        }
    }
}

int32_t DeviceTimedCollect::OnStart()
{
    HILOGI("DeviceTimedCollect OnStart called");
    lock_guard<mutex> autoLock(taskLock_);
    for (std::set<int32_t>::iterator it = timedSet_.begin(); it != timedSet_.end(); ++it) {
        int32_t interval = *it;
        HILOGI("DeviceTimedCollect send task: %{public}d", interval);
        loopTask_ = [this, interval] () {
            OnDemandEvent event = { TIMED_EVENT, LOOP_EVENT, to_string(interval) };
            HILOGI("DeviceTimedCollect ReportEvent interval: %{public}d", interval);
            ReportEvent(event);
            lock_guard<mutex> autoLock(taskLock_);
            PostDelayTask(loopTask_, interval);
        };
        PostDelayTask(loopTask_, interval);
    }
    return ERR_OK;
}

int32_t DeviceTimedCollect::OnStop()
{
    HILOGI("DeviceTimedCollect OnStop called");
    return ERR_OK;
}
}
