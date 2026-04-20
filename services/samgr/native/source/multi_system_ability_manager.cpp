/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "multi_system_ability_manager.h"
#include "sam_log.h"

namespace OHOS {

MultiSystemAbilityManager::MultiSystemAbilityManager(int32_t userId) : userId_(userId) {}

MultiSystemAbilityManager::~MultiSystemAbilityManager() {}

int32_t MultiSystemAbilityManager::Init(const std::list<SaProfile>& saProfiles)
{
    HILOGI("MultiSystemAbilityManager Init for userId:%{public}d", userId_);
    return ERR_OK;
}

int32_t MultiSystemAbilityManager::Destroy()
{
    HILOGI("MultiSystemAbilityManager Destroy for userId:%{public}d", userId_);
    return ERR_OK;
}

int32_t MultiSystemAbilityManager::AddSystemAbility(int32_t systemAbilityId,
    const sptr<IRemoteObject>& ability, const ISystemAbilityManager::SAExtraProp& extraProp)
{
    HILOGI("MultiSystemAbilityManager AddSystemAbility SA:%{public}d userId:%{public}d",
        systemAbilityId, userId_);
    return BaseSystemAbilityManager::AddSystemAbility(systemAbilityId, ability, extraProp);
}

sptr<IRemoteObject> MultiSystemAbilityManager::GetSystemAbility(int32_t systemAbilityId)
{
    HILOGD("MultiSystemAbilityManager GetSystemAbility SA:%{public}d userId:%{public}d",
        systemAbilityId, userId_);
    return BaseSystemAbilityManager::GetSystemAbility(systemAbilityId);
}

sptr<IRemoteObject> MultiSystemAbilityManager::CheckSystemAbility(int32_t systemAbilityId)
{
    HILOGD("MultiSystemAbilityManager CheckSystemAbility SA:%{public}d userId:%{public}d",
        systemAbilityId, userId_);
    return BaseSystemAbilityManager::CheckSystemAbility(systemAbilityId);
}

sptr<IRemoteObject> MultiSystemAbilityManager::CheckSystemAbility(int32_t systemAbilityId, bool& isExist)
{
    HILOGD("MultiSystemAbilityManager CheckSystemAbility SA:%{public}d userId:%{public}d",
        systemAbilityId, userId_);
    return BaseSystemAbilityManager::CheckSystemAbility(systemAbilityId, isExist);
}

int32_t MultiSystemAbilityManager::RemoveSystemAbility(int32_t systemAbilityId)
{
    HILOGI("MultiSystemAbilityManager RemoveSystemAbility SA:%{public}d userId:%{public}d",
        systemAbilityId, userId_);
    return BaseSystemAbilityManager::RemoveSystemAbility(systemAbilityId);
}

std::vector<std::u16string> MultiSystemAbilityManager::ListSystemAbilities(uint32_t dumpFlags)
{
    HILOGD("MultiSystemAbilityManager ListSystemAbilities userId:%{public}d", userId_);
    return BaseSystemAbilityManager::ListSystemAbilities(dumpFlags);
}

int32_t MultiSystemAbilityManager::SubscribeSystemAbility(int32_t systemAbilityId,
    const sptr<ISystemAbilityStatusChange>& listener)
{
    HILOGI("MultiSystemAbilityManager SubscribeSystemAbility SA:%{public}d userId:%{public}d",
        systemAbilityId, userId_);
    return BaseSystemAbilityManager::SubscribeSystemAbility(systemAbilityId, listener);
}

int32_t MultiSystemAbilityManager::UnSubscribeSystemAbility(int32_t systemAbilityId,
    const sptr<ISystemAbilityStatusChange>& listener)
{
    HILOGI("MultiSystemAbilityManager UnSubscribeSystemAbility SA:%{public}d userId:%{public}d",
        systemAbilityId, userId_);
    return BaseSystemAbilityManager::UnSubscribeSystemAbility(systemAbilityId, listener);
}

int32_t MultiSystemAbilityManager::AddSystemProcess(const std::u16string& procName,
    const sptr<IRemoteObject>& procObject)
{
    HILOGI("MultiSystemAbilityManager AddSystemProcess userId:%{public}d", userId_);
    return BaseSystemAbilityManager::AddSystemProcess(procName, procObject);
}

int32_t MultiSystemAbilityManager::AddOnDemandSystemAbilityInfo(int32_t systemAbilityId,
    const std::u16string& procName)
{
    HILOGI("MultiSystemAbilityManager AddOnDemandSystemAbilityInfo SA:%{public}d userId:%{public}d",
        systemAbilityId, userId_);
    return BaseSystemAbilityManager::AddOnDemandSystemAbilityInfo(systemAbilityId, procName);
}

void MultiSystemAbilityManager::Dump(int32_t fd)
{
    HILOGI("MultiSystemAbilityManager Dump userId:%{public}d", userId_);
}

void MultiSystemAbilityManager::GetAllSystemAbilityInfo(std::string& result)
{
    HILOGI("MultiSystemAbilityManager GetAllSystemAbilityInfo userId:%{public}d", userId_);
}

std::u16string MultiSystemAbilityManager::GetUserProcessName(const std::u16string& baseProcName) const
{
    return baseProcName;
}

bool MultiSystemAbilityManager::IsUserMultiInstanceSa(int32_t systemAbilityId) const
{
    return userMultiInstanceSaIds_.count(systemAbilityId) > 0;
}

} // namespace OHOS
