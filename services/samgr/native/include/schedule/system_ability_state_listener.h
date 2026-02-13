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

#ifndef OHOS_SYSTEM_ABILITY_MANAGER_SYSTEM_ABILITY_STATE_LISTENER_H
#define OHOS_SYSTEM_ABILITY_MANAGER_SYSTEM_ABILITY_STATE_LISTENER_H

#include <string>

#include "sam_log.h"
#include "string_ex.h"

namespace OHOS {
class SystemAbilityStateListener {
public:
    virtual ~SystemAbilityStateListener() = default;
    virtual void OnAbilityNotLoadedLocked(int32_t systemAbilityId)
    {
        HILOGI("Scheduler SA:%{public}d not loaded", systemAbilityId);
    };
    virtual void OnAbilityLoadingLocked(int32_t systemAbilityId)
    {
        HILOGI("Scheduler SA:%{public}d loading", systemAbilityId);
    };
    virtual void OnAbilityLoadedLocked(int32_t systemAbilityId)
    {
        HILOGI("Scheduler SA:%{public}d loaded", systemAbilityId);
    };
    virtual void OnAbilityUnloadableLocked(int32_t systemAbilityId)
    {
        HILOGI("Scheduler SA:%{public}d unloadable", systemAbilityId);
    };
    virtual void OnAbilityUnloadingLocked(int32_t systemAbilityId)
    {
        HILOGI("Scheduler SA:%{public}d unloading", systemAbilityId);
    };
    virtual void OnProcessNotStartedLocked(const std::u16string& processName)
    {
        HILOGI("Scheduler proc:%{public}s stopped", Str16ToStr8(processName).c_str());
    };
    virtual void OnProcessStartedLocked(const std::u16string& processName)
    {
        HILOGI("Scheduler proc:%{public}s started", Str16ToStr8(processName).c_str());
    };
    virtual void OnProcessStoppingLocked(const std::u16string& processName)
    {
        HILOGI("Scheduler proc:%{public}s stopping", Str16ToStr8(processName).c_str());
    };
};
} // namespace OHOS

#endif // !defined(OHOS_SYSTEM_ABILITY_MANAGER_SYSTEM_ABILITY_STATE_LISTENER_H)