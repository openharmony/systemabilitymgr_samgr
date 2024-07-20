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

#ifndef OHOS_SYSTEM_ABILITY_MANAGER_DEVICE_TIMED_COLLECT_H
#define OHOS_SYSTEM_ABILITY_MANAGER_DEVICE_TIMED_COLLECT_H

#ifdef PREFERENCES_ENABLE
#include "device_timed_collect_tool.h"
#endif
#include "icollect_plugin.h"

#include <mutex>
#include <set>

namespace OHOS {
struct TimeInfo {
    bool normal = false;
    bool awake = false;
};
class DeviceTimedCollect : public ICollectPlugin {
public:
    explicit DeviceTimedCollect(const sptr<IReport>& report);
    ~DeviceTimedCollect() = default;

    int32_t OnStart() override;
    int32_t OnStop() override;
    void Init(const std::list<SaProfile>& saProfiles) override;
    int32_t AddCollectEvent(const OnDemandEvent& event) override;
    int32_t RemoveUnusedEvent(const OnDemandEvent& event) override;
private:
    void SaveTimedEvent(const OnDemandEvent& onDemandEvent);
    void SaveTimedInfos(const OnDemandEvent& onDemandEvent, int32_t interval);
    void ReportEventByTimeInfo(int32_t interval, bool persistence);
    void PostDelayTaskByTimeInfo(std::function<void()> callback, int32_t interval, int32_t disTime);
    int64_t CalculateDelayTime(const std::string& timeString);
    void PostPersistenceLoopTasks();
    void PostNonPersistenceLoopTasks();

    void PostPersistenceLoopTaskLocked(int32_t interval);
    void PostNonPersistenceLoopTaskLocked(int32_t interval);
    void PostPersistenceTimedTaskLocked(std::string timeString, int64_t timeGap);
    void PostNonPersistenceTimedTaskLocked(std::string timeString, int64_t timeGap);

    void ProcessPersistenceTasks();
    void ProcessPersistenceLoopTask(int64_t disTime, int64_t triggerTime, std::string strInterval);
    void ProcessPersistenceTimedTask(int64_t disTime, std::string timeString);

    void PostPersistenceDelayTask(std::function<void()> loopTask,
        int32_t interval, int32_t disTime);

    void RemoveNonPersistenceLoopTask(int32_t interval);
    void RemovePersistenceLoopTask(int32_t interval);
    void RemoveTimesInfo(const OnDemandEvent& onDemandEvent, int32_t interval);
    std::set<int32_t> nonPersitenceLoopEventSet_;
    std::set<int32_t> persitenceLoopEventSet_;
    std::mutex nonPersitenceLoopEventSetLock_;
    std::mutex nonPersitenceTimedEventSetLock;
    std::mutex persitenceLoopEventSetLock_;
    std::mutex persitenceTimedEventSetLock_;
    std::map<int32_t, std::function<void()>> nonPersitenceLoopTasks_;
    std::map<int32_t, std::function<void()>> persitenceLoopTasks_;
    std::mutex timeInfosLock_;
    std::map<int32_t, TimeInfo> timeInfos_;
#ifdef PREFERENCES_ENABLE
    std::shared_ptr<PreferencesUtil> preferencesUtil_;
#endif
};
} // namespace OHOS
#endif // OHOS_SYSTEM_ABILITY_MANAGER_DEVICE_TIMED_COLLECT_H
