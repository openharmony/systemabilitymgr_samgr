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

#ifndef SAMGR_TEST_UNITTEST_INCLUDE_SYSTEM_ABILITY_MANAGER_MOCK_H
#define SAMGR_TEST_UNITTEST_INCLUDE_SYSTEM_ABILITY_MANAGER_MOCK_H

#include "if_system_ability_manager.h"

namespace OHOS {
class ISystemAbilityManagerMock : public ISystemAbilityManager {
public:

    std::vector<std::u16string> ListSystemAbilities(unsigned int dumpFlags = DUMP_FLAG_PRIORITY_ALL) override;

    sptr<IRemoteObject> GetSystemAbility(int32_t systemAbilityId) override;

    sptr<IRemoteObject> CheckSystemAbility(int32_t systemAbilityId) override;

    int32_t RemoveSystemAbility(int32_t systemAbilityId) override;

    int32_t SubscribeSystemAbility(int32_t systemAbilityId,
        const sptr<ISystemAbilityStatusChange>& listener) override;

    int32_t UnSubscribeSystemAbility(int32_t systemAbilityId,
        const sptr<ISystemAbilityStatusChange>& listener) override;

    sptr<IRemoteObject> GetSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;

    sptr<IRemoteObject> CheckSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;

    int32_t AddOnDemandSystemAbilityInfo(int32_t systemAbilityId,
        const std::u16string& localAbilityManagerName) override;

    sptr<IRemoteObject> CheckSystemAbility(int32_t systemAbilityId, bool& isExist) override;

    int32_t AddSystemAbility(int32_t systemAbilityId, const sptr<IRemoteObject>& ability,
        const SAExtraProp& extraProp = SAExtraProp(false, DUMP_FLAG_PRIORITY_DEFAULT, u"", u"")) override;

    int32_t AddSystemProcess(const std::u16string& procName, const sptr<IRemoteObject>& procObject) override;

    sptr<IRemoteObject> LoadSystemAbility(int32_t systemAbilityId, int32_t timeout) override;

    int32_t LoadSystemAbility(int32_t systemAbilityId, const sptr<ISystemAbilityLoadCallback>& callback) override;

    int32_t LoadSystemAbility(int32_t systemAbilityId, const std::string& deviceId,
        const sptr<ISystemAbilityLoadCallback>& callback) override;

    int32_t UnloadSystemAbility(int32_t systemAbilityId) override;

    int32_t CancelUnloadSystemAbility(int32_t systemAbilityId) override;

    int32_t UnloadAllIdleSystemAbility() override;

    int32_t GetSystemProcessInfo(int32_t systemAbilityId, SystemProcessInfo& systemProcessInfo) override;

    int32_t GetRunningSystemProcess(std::list<SystemProcessInfo>& systemProcessInfos) override;

    int32_t SubscribeSystemProcess(const sptr<ISystemProcessStatusChange>& listener) override;

    int32_t SendStrategy(int32_t type, std::vector<int32_t>& systemAbilityIds,
        int32_t level, std::string& action) override;

    int32_t UnSubscribeSystemProcess(const sptr<ISystemProcessStatusChange>& listener) override;

    int32_t GetExtensionSaIds(const std::string& extension, std::vector<int32_t> &saIds) override;

    int32_t GetExtensionRunningSaList(const std::string& extension,
        std::vector<sptr<IRemoteObject>>& saList) override;

    int32_t GetRunningSaExtensionInfoList(const std::string& extension,
        std::vector<SaExtensionInfo>& infoList) override;

    int32_t GetCommonEventExtraDataIdlist(int32_t saId, std::vector<int64_t>& extraDataIdList,
        const std::string& eventName = "") override;

    int32_t GetOnDemandReasonExtraData(int64_t extraDataId, MessageParcel& extraDataParcel) override;

    int32_t GetOnDemandPolicy(int32_t systemAbilityId, OnDemandPolicyType type,
        std::vector<SystemAbilityOnDemandEvent>& abilityOnDemandEvents) override;

    int32_t UpdateOnDemandPolicy(int32_t systemAbilityId, OnDemandPolicyType type,
        const std::vector<SystemAbilityOnDemandEvent>& abilityOnDemandEvents) override;

    int32_t GetOnDemandSystemAbilityIds(std::vector<int32_t>& systemAbilityIds) override;

    sptr<IRemoteObject> AsObject() override;
};
}
#endif /* SAMGR_TEST_UNITTEST_INCLUDE_SYSTEM_ABILITY_MANAGER_MOCK_H */
