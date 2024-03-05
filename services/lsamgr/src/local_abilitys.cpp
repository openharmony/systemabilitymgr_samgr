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

#include "local_abilitys.h"

using namespace std;
using namespace OHOS::HiviewDFX;

namespace OHOS {
#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD001800
#undef LOG_TAG
#define LOG_TAG "SA"
LocalAbilitys& LocalAbilitys::GetInstance()
{
    static auto instance = new LocalAbilitys();
    return *instance;
}

void LocalAbilitys::AddAbility(int32_t systemAbilityId, const sptr<IRemoteObject>& ability)
{
    HILOG_DEBUG(LOG_CORE, "AddAbility to cache %{public}d", systemAbilityId);
    std::lock_guard<std::mutex> autoLock(localSALock_);
    localSAMap_[systemAbilityId] = ability;
}

sptr<IRemoteObject> LocalAbilitys::GetAbility(int32_t systemAbilityId)
{
    std::lock_guard<std::mutex> autoLock(localSALock_);
    auto it = localSAMap_.find(systemAbilityId);
    if (it != localSAMap_.end()) {
        return it->second;
    }
    return nullptr;
}

void LocalAbilitys::RemoveAbility(int32_t systemAbilityId)
{
    HILOG_DEBUG(LOG_CORE, "RemoveAbility from cache %{public}d", systemAbilityId);
    std::lock_guard<std::mutex> autoLock(localSALock_);
    auto it = localSAMap_.find(systemAbilityId);
    if (it != localSAMap_.end()) {
        localSAMap_.erase(systemAbilityId);
    }
}
}
