/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "system_ability_manager_mock.h"

namespace OHOS {
std::vector<std::u16string> ISystemAbilityManagerMock::ListSystemAbilities(unsigned int dumpFlags)
{
    std::vector<std::u16string> vecSystemAbility;
    return vecSystemAbility;
}

sptr<IRemoteObject> ISystemAbilityManagerMock::GetSystemAbility(int32_t systemAbilityId)
{
    return nullptr;
}

sptr<IRemoteObject> ISystemAbilityManagerMock::CheckSystemAbility(int32_t systemAbilityId)
{
    return nullptr;
}

int32_t ISystemAbilityManagerMock::RemoveSystemAbility(int32_t systemAbilityId)
{
    return 0;
}

int32_t ISystemAbilityManagerMock::SubscribeSystemAbility(int32_t systemAbilityId,
    const sptr<ISystemAbilityStatusChange>& listener)
{
    return 0;
}

int32_t ISystemAbilityManagerMock::UnSubscribeSystemAbility(int32_t systemAbilityId,
    const sptr<ISystemAbilityStatusChange>& listener)
{
    return 0;
}

sptr<IRemoteObject> ISystemAbilityManagerMock::GetSystemAbility(int32_t systemAbilityId,
    const std::string& deviceId)
{
    return nullptr;
}

sptr<IRemoteObject> ISystemAbilityManagerMock::CheckSystemAbility(int32_t systemAbilityId,
    const std::string& deviceId)
{
    return nullptr;
}

int32_t ISystemAbilityManagerMock::AddOnDemandSystemAbilityInfo(int32_t systemAbilityId,
    const std::u16string& localAbilityManagerName)
{
    return 0;
}

sptr<IRemoteObject> ISystemAbilityManagerMock::CheckSystemAbility(int32_t systemAbilityId, bool& isExist)
{
    return nullptr;
}

int32_t ISystemAbilityManagerMock::AddSystemAbility(int32_t systemAbilityId, const sptr<IRemoteObject>& ability,
    const SAExtraProp& extraProp)
{
    return 0;
}

int32_t ISystemAbilityManagerMock::AddSystemProcess(const std::u16string& procName,
    const sptr<IRemoteObject>& procObject)
{
    return 0;
}


sptr<IRemoteObject> ISystemAbilityManagerMock::LoadSystemAbility(int32_t systemAbilityId, int32_t timeout)
{
    return nullptr;
}


int32_t ISystemAbilityManagerMock::LoadSystemAbility(int32_t systemAbilityId,
    const sptr<ISystemAbilityLoadCallback>& callback)
{
    return 0;
}

int32_t ISystemAbilityManagerMock::LoadSystemAbility(int32_t systemAbilityId, const std::string& deviceId,
    const sptr<ISystemAbilityLoadCallback>& callback)
{
    return 0;
}

int32_t ISystemAbilityManagerMock::UnloadSystemAbility(int32_t systemAbilityId)
{
    return 0;
}

int32_t ISystemAbilityManagerMock::CancelUnloadSystemAbility(int32_t systemAbilityId)
{
    return 0;
}

int32_t ISystemAbilityManagerMock::UnloadAllIdleSystemAbility()
{
    return 0;
}

int32_t ISystemAbilityManagerMock::UnloadProcess(const std::vector<std::u16string>& processList)
{
    return 0;
}

int32_t ISystemAbilityManagerMock::GetLruIdleSystemAbilityProc(std::vector<IdleProcessInfo>& processInfos)
{
    return 0;
}

int32_t ISystemAbilityManagerMock::GetSystemProcessInfo(int32_t systemAbilityId, SystemProcessInfo& systemProcessInfo)
{
    return 0;
}

int32_t ISystemAbilityManagerMock::GetRunningSystemProcess(std::list<SystemProcessInfo>& systemProcessInfos)
{
    return 0;
}

int32_t ISystemAbilityManagerMock::SubscribeSystemProcess(const sptr<ISystemProcessStatusChange>& listener)
{
    return 0;
}

int32_t ISystemAbilityManagerMock::SendStrategy(int32_t type, std::vector<int32_t>& systemAbilityIds,
    int32_t level, std::string& action)
{
    return 0;
}

int32_t ISystemAbilityManagerMock::UnSubscribeSystemProcess(const sptr<ISystemProcessStatusChange>& listener)
{
    return 0;
}

int32_t ISystemAbilityManagerMock::GetExtensionSaIds(const std::string& extension, std::vector<int32_t> &saIds)
{
    return 0;
}

int32_t ISystemAbilityManagerMock::GetExtensionRunningSaList(const std::string& extension,
    std::vector<sptr<IRemoteObject>>& saList)
{
    return 0;
}

int32_t ISystemAbilityManagerMock::GetRunningSaExtensionInfoList(const std::string& extension,
    std::vector<SaExtensionInfo>& infoList)
{
    return 0;
}

int32_t ISystemAbilityManagerMock::GetCommonEventExtraDataIdlist(int32_t saId, std::vector<int64_t>& extraDataIdList,
    const std::string& eventName)
{
    return 0;
}

int32_t ISystemAbilityManagerMock::GetOnDemandReasonExtraData(int64_t extraDataId, MessageParcel& extraDataParcel)
{
    return 0;
}

int32_t ISystemAbilityManagerMock::GetOnDemandPolicy(int32_t systemAbilityId, OnDemandPolicyType type,
    std::vector<SystemAbilityOnDemandEvent>& abilityOnDemandEvents)
{
    return 0;
}

int32_t ISystemAbilityManagerMock::UpdateOnDemandPolicy(int32_t systemAbilityId, OnDemandPolicyType type,
    const std::vector<SystemAbilityOnDemandEvent>& abilityOnDemandEvents)
{
    return 0;
}

int32_t ISystemAbilityManagerMock::GetOnDemandSystemAbilityIds(std::vector<int32_t>& systemAbilityIds)
{
    return 0;
}

sptr<IRemoteObject> ISystemAbilityManagerMock::AsObject()
{
    return nullptr;
}

}
