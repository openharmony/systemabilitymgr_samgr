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

#ifndef SERVICES_SAMGR_NATIVE_INCLUDE_SYSTEM_ABILITY_MANAGER_UTIL_H
#define SERVICES_SAMGR_NATIVE_INCLUDE_SYSTEM_ABILITY_MANAGER_UTIL_H

#include <string>
#include <list>

#include "sa_profiles.h"
#include "system_ability_on_demand_event.h"
#include "ffrt_handler.h"

namespace OHOS {
class SamgrUtil {
public:
    ~SamgrUtil();
    static bool IsNameInValid(const std::u16string& name);
    static void ParseRemoteSaName(const std::u16string& name, std::string& deviceId, std::u16string& saName);
    static bool CheckDistributedPermission();
    static bool IsSameEvent(const OnDemandEvent& event, std::list<OnDemandEvent>& enableOnceList);
    static std::string EventToStr(const OnDemandEvent& event);
    static std::string TransformDeviceId(const std::string& deviceId, int32_t type, bool isPrivate);
    static bool CheckCallerProcess(const SaProfile& saProfile);
    static bool CheckCallerProcess(const std::string& callProcess);
    static bool CheckAllowUpdate(OnDemandPolicyType type, const SaProfile& saProfile);
    static void ConvertToOnDemandEvent(const SystemAbilityOnDemandEvent& from, OnDemandEvent& to);
    static void ConvertToSystemAbilityOnDemandEvent(const OnDemandEvent& from, SystemAbilityOnDemandEvent& to);
    static uint64_t GenerateFreKey(int32_t uid, int32_t saId);
    static std::list<int32_t> GetCacheCommonEventSa(const OnDemandEvent& event,
        const std::list<SaControlInfo>& saControlList);
    static void SetModuleUpdateParam(const std::string& key, const std::string& value);
    static void SendUpdateSaState(int32_t systemAbilityId, const std::string& updateSaState);
    static void InvalidateSACache();
private:
    static std::shared_ptr<FFRTHandler> setParmHandler_;
};
} // namespace OHOS

#endif // !defined(SERVICES_SAMGR_NATIVE_INCLUDE_SYSTEM_ABILITY_MANAGER_UTIL_H)
