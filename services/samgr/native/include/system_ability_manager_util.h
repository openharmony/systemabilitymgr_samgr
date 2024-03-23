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
};
} // namespace OHOS

#endif // !defined(SERVICES_SAMGR_NATIVE_INCLUDE_SYSTEM_ABILITY_MANAGER_UTIL_H)
