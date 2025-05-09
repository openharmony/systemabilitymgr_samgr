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

#ifndef OHOS_SYSTEM_ABILITY_MANAGER_REPORT_INTERFACE_H
#define OHOS_SYSTEM_ABILITY_MANAGER_REPORT_INTERFACE_H

#include "refbase.h"
#include "sa_profiles.h"

namespace OHOS {
class IReport : public virtual RefBase {
public:
    IReport() = default;
    virtual ~IReport() = default;
    virtual void ReportEvent(const OnDemandEvent& event) = 0;
    virtual void PostTask(std::function<void()> callback) = 0;
    virtual void PostDelayTask(std::function<void()> callback, int32_t delayTime) = 0;
};
} // namespace OHOS
#endif // OHOS_SYSTEM_ABILITY_MANAGER_REPORT_INTERFACE_H