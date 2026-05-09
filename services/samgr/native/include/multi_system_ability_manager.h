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

#ifndef SERVICES_SAMGR_NATIVE_INCLUDE_MULTI_SYSTEM_ABILITY_MANAGER_H
#define SERVICES_SAMGR_NATIVE_INCLUDE_MULTI_SYSTEM_ABILITY_MANAGER_H

#include "base_system_ability_manager.h"

namespace OHOS {

class MultiSystemAbilityManager : public BaseSystemAbilityManager {
public:
    explicit MultiSystemAbilityManager(int32_t userId);
    ~MultiSystemAbilityManager();

    int32_t GetUserId() const { return userId_; }

    int32_t Init(const std::list<SaProfile>& saProfiles);
    int32_t Destroy();

    int32_t AddSystemAbility(int32_t systemAbilityId, const sptr<IRemoteObject>& ability,
        const ISystemAbilityManager::SAExtraProp& extraProp) override;
    sptr<IRemoteObject> GetSystemAbility(int32_t systemAbilityId) override;
    sptr<IRemoteObject> CheckSystemAbility(int32_t systemAbilityId) override;
    sptr<IRemoteObject> CheckSystemAbility(int32_t systemAbilityId, bool& isExist) override;
    int32_t RemoveSystemAbility(int32_t systemAbilityId) override;
    std::vector<std::u16string> ListSystemAbilities(uint32_t dumpFlags) override;

    int32_t SubscribeSystemAbility(int32_t systemAbilityId,
        const sptr<ISystemAbilityStatusChange>& listener) override;
    int32_t UnSubscribeSystemAbility(int32_t systemAbilityId,
        const sptr<ISystemAbilityStatusChange>& listener) override;

    int32_t AddSystemProcess(const std::u16string& procName,
        const sptr<IRemoteObject>& procObject) override;
    int32_t AddOnDemandSystemAbilityInfo(int32_t systemAbilityId,
        const std::u16string& procName);

    void Dump(int32_t fd);
    void GetAllSystemAbilityInfo(std::string& result);

private:
    std::u16string GetUserProcessName(const std::u16string& baseProcName) const;
    bool IsUserMultiInstanceSa(int32_t systemAbilityId) const;

    int32_t userId_;
    std::set<int32_t> userMultiInstanceSaIds_;
};

} // namespace OHOS

#endif // !defined(SERVICES_SAMGR_NATIVE_INCLUDE_MULTI_SYSTEM_ABILITY_MANAGER_H)
