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

#include "nlohmann/json.hpp"
#include "system_ability_manager_util.h"
#include "ipc_skeleton.h"
#include "string_ex.h"
#include "tools.h"
#include "sam_log.h"

namespace OHOS {
using namespace std;
constexpr int32_t MAX_NAME_SIZE = 200;
constexpr int32_t SPLIT_NAME_VECTOR_SIZE = 2;
constexpr int32_t UID_ROOT = 0;
constexpr int32_t UID_SYSTEM = 1000;

const string EVENT_TYPE = "eventId";
const string EVENT_NAME = "name";
const string EVENT_VALUE = "value";
const string EVENT_EXTRA_DATA_ID = "extraDataId";

bool SamgrUtil::IsNameInValid(const std::u16string& name)
{
    HILOGD("%{public}s called:name = %{public}s", __func__, Str16ToStr8(name).c_str());
    bool ret = false;
    if (name.empty() || name.size() > MAX_NAME_SIZE || DeleteBlank(name).empty()) {
        ret = true;
    }

    return ret;
}

void SamgrUtil::ParseRemoteSaName(const std::u16string& name, std::string& deviceId,
    std::u16string& saName)
{
    vector<string> strVector;
    SplitStr(Str16ToStr8(name), "_", strVector);
    if (strVector.size() == SPLIT_NAME_VECTOR_SIZE) {
        deviceId = strVector[0];
        saName = Str8ToStr16(strVector[1]);
    }
}

bool SamgrUtil::CheckDistributedPermission()
{
    auto callingUid = IPCSkeleton::GetCallingUid();
    if (callingUid != UID_ROOT && callingUid != UID_SYSTEM) {
        return false;
    }
    return true;
}

bool SamgrUtil::IsSameEvent(const OnDemandEvent& event, std::list<OnDemandEvent>& enableOnceList)
{
    for (auto iter = enableOnceList.begin(); iter != enableOnceList.end(); iter++) {
        if (event.eventId == iter->eventId && event.name == iter->name && event.value == iter->value) {
            HILOGI("event already exits in enable-once list");
            return true;
        }
    }
    return false;
}

std::string SamgrUtil::EventToStr(const OnDemandEvent& event)
{
    nlohmann::json eventJson;
    eventJson[EVENT_TYPE] = event.eventId;
    eventJson[EVENT_NAME] = event.name;
    eventJson[EVENT_VALUE] = event.value;
    eventJson[EVENT_EXTRA_DATA_ID] = event.extraDataId;
    std::string eventStr = eventJson.dump();
    return eventStr;
}

std::string SamgrUtil::TransformDeviceId(const std::string& deviceId, int32_t type, bool isPrivate)
{
    return isPrivate ? std::string() : deviceId;
}
}