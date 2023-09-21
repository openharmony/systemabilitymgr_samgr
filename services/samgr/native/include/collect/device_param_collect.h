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

#ifndef OHOS_SYSTEM_ABILITY_MANAGER_DEVICE_PARAM_COLLECT_H
#define OHOS_SYSTEM_ABILITY_MANAGER_DEVICE_PARAM_COLLECT_H

#include "icollect_plugin.h"
#include "system_ability_status_change_stub.h"
#include <set>
#include <list>

namespace OHOS {
class DeviceParamCollect : public ICollectPlugin {
public:
    explicit DeviceParamCollect(const sptr<IReport>& report);
    ~DeviceParamCollect() = default;
    void Init(const std::list<SaProfile>& saProfiles) override;
    void WatchParameters();
    bool CheckCondition(const OnDemandCondition& condition) override;
    int32_t AddCollectEvent(const OnDemandEvent& event) override;
    int32_t RemoveUnusedEvent(const OnDemandEvent& event) override;
    int32_t OnStart() override;
    int32_t OnStop() override;
private:
    std::mutex paramLock_;
    std::set<std::string> pendingParams_;
    std::set<std::string> params_;
};

class SystemAbilityStatusChange : public SystemAbilityStatusChangeStub {
public:
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    void Init(const sptr<DeviceParamCollect>& deviceParamCollect);
private:
    sptr<DeviceParamCollect> deviceParamCollect_;
};
} // namespace OHOS
#endif // OHOS_SYSTEM_ABILITY_MANAGER_DEVICE_PARAM_COLLECT_H