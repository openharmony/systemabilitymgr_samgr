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

void DeviceTimedCollect::Init(const std::list<SaProfile>& saProfiles)
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

void DeviceTimedCollect::PostLoopTaskLocked(int32_t interval)
{
    if (loopTasks_.count(interval) > 0) {
        HILOGE("DeviceTimedCollect interval has been post");
        return;
    }
    loopTasks_[interval] = [this, interval] () {
        OnDemandEvent event = { TIMED_EVENT, LOOP_EVENT, to_string(interval) };
        lock_guard<mutex> autoLock(taskLock_);
        if (timedSet_.find(interval) != timedSet_.end()) {
            HILOGI("DeviceTimedCollect ReportEvent interval: %{public}d", interval);
            ReportEvent(event);
            PostDelayTask(loopTasks_[interval], interval);
        } else {
            HILOGD("DeviceTimedCollect interval %{public}d has been remove", interval);
        }
    };
    PostDelayTask(loopTasks_[interval], interval);
}

int32_t DeviceTimedCollect::OnStart()
{
    HILOGI("DeviceTimedCollect OnStart called");
    lock_guard<mutex> autoLock(taskLock_);
    for (std::set<int32_t>::iterator it = timedSet_.begin(); it != timedSet_.end(); ++it) {
        int32_t interval = *it;
        HILOGI("DeviceTimedCollect send task: %{public}d", interval);
        PostLoopTaskLocked(interval);
    }
    return ERR_OK;
}

int32_t DeviceTimedCollect::OnStop()
{
    HILOGI("DeviceTimedCollect OnStop called");
    return ERR_OK;
}

int32_t DeviceTimedCollect::AddCollectEvent(const OnDemandEvent& event)
{
    if (event.name != LOOP_EVENT) {
        HILOGE("DeviceTimedCollect invalid event name: %{public}s", event.name.c_str());
        return ERR_INVALID_VALUE;
    }
    int32_t interval = atoi(event.value.c_str());
    if (interval < MIN_INTERVAL) {
        HILOGE("DeviceTimedCollect invalid interval: %{public}d", interval);
        return ERR_INVALID_VALUE;
    }
    std::lock_guard<std::mutex> autoLock(taskLock_);
    auto iter = timedSet_.find(interval);
    if (iter != timedSet_.end()) {
        return ERR_OK;
    }
    HILOGI("DeviceTimedCollect add collect events: %{public}d", interval);
    timedSet_.insert(interval);
    PostLoopTaskLocked(interval);
    return ERR_OK;
}

int32_t DeviceTimedCollect::RemoveUnusedEvent(const OnDemandEvent& event)
{
    if (event.name != LOOP_EVENT) {
        HILOGE("DeviceTimedCollect invalid event name: %{public}s", event.name.c_str());
        return ERR_INVALID_VALUE;
    }
    int32_t interval = atoi(event.value.c_str());
    std::lock_guard<std::mutex> autoLock(taskLock_);
    auto iter = timedSet_.find(interval);
    if (iter != timedSet_.end()) {
        HILOGI("DeviceTimedCollect remove interval: %{public}d", interval);
        timedSet_.erase(iter);
        loopTasks_.erase(interval);
    }
    return ERR_OK;
}
}
