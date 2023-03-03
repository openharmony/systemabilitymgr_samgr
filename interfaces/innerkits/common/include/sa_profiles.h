/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef SAMGR_INTERFACE_INNERKITS_COMMOM_INCLUDE_SAPROFILE_H
#define SAMGR_INTERFACE_INNERKITS_COMMOM_INCLUDE_SAPROFILE_H

#include <string>
#include <vector>

namespace OHOS {
using DlHandle = void*;

enum {
    INTERFACE_CALL = 0,
    DEVICE_ONLINE,
    SETTING_SWITCH,
    PARAM,
    COMMON_EVENT,
};

enum {
    START_ON_DEMAND = 1,
    STOP_ON_DEMAND,
};

enum {
    START = 1,
    KILL,
    FREEZE,
};

struct OnDemandEvent {
    int32_t eventId;
    std::string name;
    std::string value;
    bool enableOnce = false;

    bool operator==(const OnDemandEvent& event) const
    {
        return this->eventId == event.eventId && this->name == event.name && this->value == event.value;
    }
};

struct SaControlInfo {
    int32_t ondemandId;
    int32_t saId;
    bool enableOnce = false;
};

struct SaProfile {
    std::u16string process;
    int32_t saId = 0;
    std::string libPath;
    std::vector<int32_t> dependSa;
    int32_t dependTimeout = 0;
    bool runOnCreate = false;
    bool distributed = false;
    int32_t dumpLevel = 0;
    std::u16string capability;
    std::u16string permission;
    std::string bootPhase;
    std::vector<OnDemandEvent> startOnDemand;
    std::vector<OnDemandEvent> stopOnDemand;
    DlHandle handle = nullptr;
};
}
#endif // SAMGR_INTERFACE_INNERKITS_COMMOM_INCLUDE_SAPROFILE_H
