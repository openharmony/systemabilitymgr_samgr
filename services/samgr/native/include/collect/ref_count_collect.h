/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_SYSTEM_ABILITY_MANAGER_REF_COUNT_COLLECT_H
#define OHOS_SYSTEM_ABILITY_MANAGER_REF_COUNT_COLLECT_H

#include "system_ability_manager_proxy.h"
#include "system_ability_manager.h"
#include "iservice_registry.h"
#include "icollect_plugin.h"
#include "timer.h"
#include "common_timer_errors.h"
#include "hisysevent_adapter.h"

namespace OHOS {
class RefCountCollect : public ICollectPlugin {
public:
    explicit RefCountCollect(const sptr<IReport>& report);
    ~RefCountCollect() = default;

    int32_t OnStart() override;
    int32_t OnStop() override;
    void Init(const std::list<SaProfile>& saProfiles) override;

private:
    void IdentifyUnrefOndemand();
    void IdentifyUnrefResident();
    std::list<int32_t> residentSaList_;
    std::list<int32_t> unrefUnloadSaList_;
    std::unique_ptr<Utils::Timer> timer_;
};

} // namespace OHOS
#endif //OHOS_SYSTEM_ABILITY_MANAGER_REF_COUNT_COLLECT_H