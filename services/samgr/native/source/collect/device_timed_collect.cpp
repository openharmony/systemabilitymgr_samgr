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

#ifdef PREFERENCES_ENABLE
#include "preferences_errno.h"
#include "preferences_helper.h"
#include "preferences_value.h"
#endif
#include "sam_log.h"
#include "sa_profiles.h"
#include "system_ability_manager.h"

using namespace std;

namespace OHOS {
namespace {
constexpr const char* LOOP_EVENT = "loopevent";
constexpr const char* ORDER_TIMED_EVENT = "timedevent";
constexpr int32_t MIN_INTERVAL = 30;
}

DeviceTimedCollect::DeviceTimedCollect(const sptr<IReport>& report)
    : ICollectPlugin(report)
{
}

void DeviceTimedCollect::Init(const std::list<SaProfile>& saProfiles)
{
    for (auto& saProfile : saProfiles) {
        for (auto& onDemandEvent : saProfile.startOnDemand.onDemandEvents) {
            SaveTimedEvent(onDemandEvent);
        }
        for (auto& onDemandEvent : saProfile.stopOnDemand.onDemandEvents) {
            SaveTimedEvent(onDemandEvent);
        }
    }
    HILOGD("DeviceTimedCollect timedSet count: %{public}zu", nonPersitenceLoopEventSet_.size());
}

void DeviceTimedCollect::ProcessPersistenceTasks()
{
#ifdef PREFERENCES_ENABLE
    preferencesUtil_ = PreferencesUtil::GetInstance();
    std::map<std::string, NativePreferences::PreferencesValue> allData = preferencesUtil_->ObtainAll();
    int64_t currentTime = TimeUtils::GetTimestamp();
    for (const auto& [strInterval, triggerTime] : allData) {
        if (strInterval.find("_") != std::string::npos) {
            continue;
        }
        int64_t disTime = static_cast<int64_t>(triggerTime) - currentTime;
        if (strInterval.find(':') != string::npos) {
            ProcessPersistenceTimedTask(disTime, strInterval);
            return;
        }
        ProcessPersistenceLoopTask(disTime, static_cast<int64_t>(triggerTime), strInterval);
    }
#endif
}

void DeviceTimedCollect::ProcessPersistenceTimedTask(int64_t disTime, std::string timeString)
{
#ifdef PREFERENCES_ENABLE
    if (disTime <= 0) {
        OnDemandEvent event = { TIMED_EVENT, ORDER_TIMED_EVENT, timeString, -1, true };
        ReportEvent(event);
        preferencesUtil_->Remove(timeString);
        return;
    }
    auto timedTask = [this, timeString] () {
        OnDemandEvent event = { TIMED_EVENT, ORDER_TIMED_EVENT, timeString, -1, true };
        ReportEvent(event);
        preferencesUtil_->Remove(timeString);
    };
    PostDelayTask(timedTask, disTime);
#endif
}

void DeviceTimedCollect::ProcessPersistenceLoopTask(int64_t disTime, int64_t triggerTime, std::string strInterval)
{
    int64_t interval = atoi(strInterval.c_str());
    if (persitenceLoopTasks_.count(interval) > 0) {
        return;
    }
#ifdef PREFERENCES_ENABLE
    int64_t currentTime = TimeUtils::GetTimestamp();
    if (static_cast<int64_t>(triggerTime) - interval > currentTime) {
        HILOGW("currentTime is not true");
        return;
    }
#endif
    if (interval < MIN_INTERVAL) {
        HILOGW("interval is not true");
        return;
    }
    persitenceLoopTasks_[interval] = [this, interval] () {
        OnDemandEvent event = { TIMED_EVENT, LOOP_EVENT, to_string(interval), -1, true };
        ReportEvent(event);
        PostPersistenceDelayTask(persitenceLoopTasks_[interval], interval, interval);
    };
    if (disTime <= 0) {
        OnDemandEvent event = { TIMED_EVENT, LOOP_EVENT, strInterval, -1, true };
        ReportEvent(event);
        // In order to enable the timer to start on time next time and make up for the missing time
        disTime = interval - abs(disTime) % interval;
        PostPersistenceDelayTask(persitenceLoopTasks_[interval], interval, disTime);
    } else {
        PostDelayTask(persitenceLoopTasks_[interval], disTime);
    }
}

void DeviceTimedCollect::SaveTimedEvent(const OnDemandEvent& onDemandEvent)
{
    if (onDemandEvent.eventId == TIMED_EVENT && onDemandEvent.name == LOOP_EVENT) {
        HILOGI("DeviceTimedCollect save timed task: %{public}s", onDemandEvent.value.c_str());
        int32_t interval = atoi(onDemandEvent.value.c_str());
        if (interval < MIN_INTERVAL) {
            HILOGE("DeviceTimedCollect invalid interval %{public}s", onDemandEvent.value.c_str());
            return;
        }
        if (onDemandEvent.persistence) {
            lock_guard<mutex> autoLock(persitenceLoopEventSetLock_);
            persitenceLoopEventSet_.insert(interval);
        } else {
            lock_guard<mutex> autoLock(nonPersitenceLoopEventSetLock_);
            nonPersitenceLoopEventSet_.insert(interval);
        }
    }
}

void DeviceTimedCollect::PostPersistenceLoopTaskLocked(int32_t interval)
{
    if (persitenceLoopTasks_.count(interval) > 0) {
        return;
    }
    persitenceLoopTasks_[interval] = [this, interval] () {
        OnDemandEvent event = { TIMED_EVENT, LOOP_EVENT, to_string(interval), -1, true };
        ReportEvent(event);
        PostPersistenceDelayTask(persitenceLoopTasks_[interval], interval, interval);
    };
    PostPersistenceDelayTask(persitenceLoopTasks_[interval], interval, interval);
}

void DeviceTimedCollect::PostNonPersistenceLoopTaskLocked(int32_t interval)
{
    if (nonPersitenceLoopTasks_.count(interval) > 0) {
        HILOGE("DeviceTimedCollect interval has been post");
        return;
    }
    nonPersitenceLoopTasks_[interval] = [this, interval] () {
        OnDemandEvent event = { TIMED_EVENT, LOOP_EVENT, to_string(interval) };
        lock_guard<mutex> autoLock(nonPersitenceLoopEventSetLock_);
        if (nonPersitenceLoopEventSet_.find(interval) != nonPersitenceLoopEventSet_.end()) {
            HILOGI("DeviceTimedCollect ReportEvent interval: %{public}d", interval);
            ReportEvent(event);
            PostDelayTask(nonPersitenceLoopTasks_[interval], interval);
        } else {
            HILOGD("DeviceTimedCollect interval %{public}d has been remove", interval);
        }
    };
    PostDelayTask(nonPersitenceLoopTasks_[interval], interval);
}

void DeviceTimedCollect::PostPersistenceDelayTask(std::function<void()> postTask,
    int32_t interval, int32_t disTime)
{
#ifdef PREFERENCES_ENABLE
    int64_t currentTime = TimeUtils::GetTimestamp();
    int64_t upgradeTime = currentTime + static_cast<int64_t>(disTime);
    preferencesUtil_->SaveLong(to_string(interval), upgradeTime);
    PostDelayTask(postTask, disTime);
    HILOGI("save persistence time %{public}d, interval time %{public}d", static_cast<int32_t>(upgradeTime), interval);
#endif
}

int32_t DeviceTimedCollect::OnStart()
{
    HILOGI("DeviceTimedCollect OnStart called");
    ProcessPersistenceTasks();
    PostNonPersistenceLoopTasks();
    PostPersistenceLoopTasks();
    return ERR_OK;
}

void DeviceTimedCollect::PostNonPersistenceLoopTasks()
{
    lock_guard<mutex> autoLock(nonPersitenceLoopEventSetLock_);
    for (auto it = nonPersitenceLoopEventSet_.begin(); it != nonPersitenceLoopEventSet_.end(); ++it) {
        HILOGI("DeviceTimedCollect send task: %{public}d", *it);
        PostNonPersistenceLoopTaskLocked(*it);
    }
}

void DeviceTimedCollect::PostPersistenceLoopTasks()
{
    lock_guard<mutex> autoLock(persitenceLoopEventSetLock_);
    for (auto it = persitenceLoopEventSet_.begin(); it != persitenceLoopEventSet_.end(); ++it) {
        HILOGI("DeviceTimedCollect send persitence task: %{public}d", *it);
        PostPersistenceLoopTaskLocked(*it);
    }
}

int32_t DeviceTimedCollect::OnStop()
{
    HILOGI("DeviceTimedCollect OnStop called");
    return ERR_OK;
}

int64_t DeviceTimedCollect::CalculateDelayTime(const std::string& timeString)
{
    std::tm inputTime;
    strptime(const_cast<char*>(timeString.c_str()), "%Y-%m-%d-%H:%M:%S", &inputTime);
    std::time_t orderTime = mktime(&inputTime);
    if (orderTime == -1) {
        return -1;
    }
    int64_t timeGap = orderTime - time(nullptr);
    return timeGap;
}

void DeviceTimedCollect::PostPersistenceTimedTaskLocked(std::string timeString, int64_t timeGap)
{
#ifdef PREFERENCES_ENABLE
    auto timedTask = [this, timeString] () {
        OnDemandEvent event = { TIMED_EVENT, ORDER_TIMED_EVENT, timeString, -1, true };
        ReportEvent(event);
        preferencesUtil_->Remove(timeString);
    };
    int64_t currentTime = TimeUtils::GetTimestamp();
    int64_t upgradeTime = currentTime + timeGap;
    preferencesUtil_->SaveLong(timeString, upgradeTime);
    PostDelayTask(timedTask, timeGap);
#endif
}

void DeviceTimedCollect::PostNonPersistenceTimedTaskLocked(std::string timeString, int64_t timeGap)
{
    auto timedTask = [this, timeString] () {
        OnDemandEvent event = { TIMED_EVENT, ORDER_TIMED_EVENT, timeString };
        ReportEvent(event);
    };
    PostDelayTask(timedTask, timeGap);
}

int32_t DeviceTimedCollect::AddCollectEvent(const OnDemandEvent& event)
{
    if (event.name != LOOP_EVENT && event.name != ORDER_TIMED_EVENT) {
        HILOGE("DeviceTimedCollect invalid event name: %{public}s", event.name.c_str());
        return ERR_INVALID_VALUE;
    }
    if (event.name == ORDER_TIMED_EVENT) {
        int64_t timeGap = CalculateDelayTime(event.value);
#ifdef PREFERENCES_ENABLE
        if (event.persistence) {
            std::lock_guard<std::mutex> autoLock(persitenceTimedEventSetLock_);
            PostPersistenceTimedTaskLocked(event.value, timeGap);
            return ERR_OK;
        }
#endif
        std::lock_guard<std::mutex> autoLock(nonPersitenceTimedEventSetLock);
        PostNonPersistenceTimedTaskLocked(event.value, timeGap);
        return ERR_OK;
    }
    if (event.persistence) {
        HILOGE("invalid event persistence, loopevent is not support persistence");
        return ERR_INVALID_VALUE;
    }
    int32_t interval = atoi(event.value.c_str());
    if (interval < MIN_INTERVAL) {
        HILOGE("DeviceTimedCollect invalid interval: %{public}d", interval);
        return ERR_INVALID_VALUE;
    }
    std::lock_guard<std::mutex> autoLock(nonPersitenceLoopEventSetLock_);
    auto iter = nonPersitenceLoopEventSet_.find(interval);
    if (iter != nonPersitenceLoopEventSet_.end()) {
        return ERR_OK;
    }
    HILOGI("DeviceTimedCollect add collect events: %{public}d", interval);
    nonPersitenceLoopEventSet_.insert(interval);
    PostNonPersistenceLoopTaskLocked(interval);
    return ERR_OK;
}

int32_t DeviceTimedCollect::RemoveUnusedEvent(const OnDemandEvent& event)
{
    if (event.name != LOOP_EVENT) {
        HILOGE("DeviceTimedCollect invalid event name: %{public}s", event.name.c_str());
        return ERR_INVALID_VALUE;
    }
    int32_t interval = atoi(event.value.c_str());
    if (event.persistence) {
        RemovePersistenceLoopTask(interval);
    } else {
        RemoveNonPersistenceLoopTask(interval);
    }
    return ERR_OK;
}

void DeviceTimedCollect::RemoveNonPersistenceLoopTask(int32_t interval)
{
    std::lock_guard<std::mutex> autoLock(nonPersitenceLoopEventSetLock_);
    auto iter = nonPersitenceLoopEventSet_.find(interval);
    if (iter != nonPersitenceLoopEventSet_.end()) {
        nonPersitenceLoopEventSet_.erase(iter);
        nonPersitenceLoopTasks_.erase(interval);
    }
}

void DeviceTimedCollect::RemovePersistenceLoopTask(int32_t interval)
{
    std::lock_guard<std::mutex> autoLock(persitenceLoopEventSetLock_);
    auto iter = persitenceLoopEventSet_.find(interval);
    if (iter != persitenceLoopEventSet_.end()) {
        persitenceLoopEventSet_.erase(iter);
        persitenceLoopTasks_.erase(interval);
    }
}
}
