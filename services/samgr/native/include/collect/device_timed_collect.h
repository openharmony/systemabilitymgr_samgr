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
#include "icollect_plugin.h"

#include <list>
#include <map>
#include <mutex>
#include <set>

namespace OHOS {
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
    void PostLoopTaskLocked(int32_t interval);
    std::set<int32_t> timedSet_;
    std::mutex taskLock_;
    std::map<int32_t, std::function<void()>> loopTasks_;
};
} // namespace OHOS
#endif // OHOS_SYSTEM_ABILITY_MANAGER_DEVICE_TIMED_COLLECT_H
