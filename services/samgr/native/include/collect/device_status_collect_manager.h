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

#ifndef OHOS_SYSTEM_ABILITY_MANAGER_DEVICE_STATUS_COLLECT_MANAGER_H
#define OHOS_SYSTEM_ABILITY_MANAGER_DEVICE_STATUS_COLLECT_MANAGER_H

#include <list>
#include <memory>

#include "event_handler.h"
#include "icollect_plugin.h"

namespace OHOS {
class DeviceStatusCollectManager : public IReport {
public:
    DeviceStatusCollectManager() = default;
    ~DeviceStatusCollectManager() = default;
    void Init(const std::list<SaProfile>& saProfiles);
    void UnInit();
    void ReportEvent(const OnDemandEvent& event) override;
    void StartCollect();
private:
    void FilterOnDemandSaProfiles(const std::list<SaProfile>& saProfiles);
    void GetSaControlListByEvent(const OnDemandEvent& event, std::list<SaControlInfo>& saControlList);
    static bool IsSameEvent(const OnDemandEvent& ev1, const OnDemandEvent& ev2);
    std::map<int32_t, sptr<ICollectPlugin>> collectPluginMap_;
    std::shared_ptr<AppExecFwk::EventHandler> collectHandler_;
    std::list<SaProfile> onDemandSaProfiles;
};
} // namespace OHOS
#endif // OHOS_SYSTEM_ABILITY_MANAGER_DEVICE_STATUS_COLLECT_MANAGER_H