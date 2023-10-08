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

#include "system_ability_on_demand_event.h"

#include "sam_log.h"

namespace OHOS {
bool OnDemandEventToParcel::WriteOnDemandEventsToParcel(
    const std::vector<SystemAbilityOnDemandEvent>& abilityOnDemandEvents, MessageParcel& data)
{
    size_t size = abilityOnDemandEvents.size();
    if (!data.WriteInt32(size)) {
        HILOGW("WriteOnDemandEvents write list size failed!");
        return false;
    }
    for (auto& event : abilityOnDemandEvents) {
        if (!WriteOnDemandEventToParcel(event, data)) {
            HILOGW("WriteOnDemandEvents write event failed!");
            return false;
        }
        if (!data.WriteBool(event.persistence)) {
            HILOGW("WriteOnDemandEvents write persistence failed!");
            return false;
        }
        if (!data.WriteInt32(event.conditions.size())) {
            HILOGW("WriteOnDemandEvents write conditions size failed!");
            return false;
        }
        for (auto& condition : event.conditions) {
            if (!WriteOnDemandConditionToParcel(condition, data)) {
                HILOGW("WriteOnDemandEvents write condition failed!");
                return false;
            }
        }
        if (!data.WriteBool(event.enableOnce)) {
            HILOGW("WriteOnDemandEvents write enableOnce failed!");
            return false;
        }
    }
    return true;
}

bool OnDemandEventToParcel::WriteOnDemandEventToParcel(const SystemAbilityOnDemandEvent& event, MessageParcel& data)
{
    if (!data.WriteInt32(static_cast<int32_t>(event.eventId))) {
        HILOGW("WriteOnDemandEvent write eventId failed!");
        return false;
    }
    if (!data.WriteString(event.name)) {
        HILOGW("WriteOnDemandEvent write name failed!");
        return false;
    }
    if (!data.WriteString(event.value)) {
        HILOGW("WriteOnDemandEvent write value failed!");
        return false;
    }
    return true;
}

bool OnDemandEventToParcel::WriteOnDemandConditionToParcel(const SystemAbilityOnDemandCondition& condition,
    MessageParcel& data)
{
    if (!data.WriteInt32(static_cast<int32_t>(condition.eventId))) {
        HILOGW("WriteOnDemandCondition write eventId failed!");
        return false;
    }
    if (!data.WriteString(condition.name)) {
        HILOGW("WriteOnDemandCondition write name failed!");
        return false;
    }
    if (!data.WriteString(condition.value)) {
        HILOGW("WriteOnDemandCondition write value failed!");
        return false;
    }
    return true;
}

bool OnDemandEventToParcel::ReadOnDemandEventsFromParcel(
    std::vector<SystemAbilityOnDemandEvent>& abilityOnDemandEvents, MessageParcel& reply)
{
    int32_t size = 0;
    if (!reply.ReadInt32(size)) {
        HILOGW("ReadOnDemandEvents read list size failed!");
        return false;
    }
    if (static_cast<size_t>(size) > reply.GetReadableBytes() || size < 0) {
        HILOGW("invalid list size: %{public}d", size);
        return false;
    }
    for (int32_t i = 0; i < size; i++) {
        SystemAbilityOnDemandEvent event;
        if (!ReadOnDemandEventFromParcel(event, reply)) {
            HILOGW("ReadOnDemandEvents read event failed!");
            return false;
        }
        if (!reply.ReadBool(event.persistence)) {
            HILOGW("ReadOnDemandEvents read persistence failed!");
            return false;
        }
        int32_t conditionsSize = 0;
        if (!reply.ReadInt32(conditionsSize)) {
            HILOGW("ReadOnDemandEvents read conditions size failed!");
            return false;
        }
        if (static_cast<size_t>(conditionsSize) > reply.GetReadableBytes() || conditionsSize < 0) {
            HILOGW("invalid condition list size: %{public}d", conditionsSize);
            return false;
        }
        for (int32_t j = 0; j < conditionsSize; j++) {
            SystemAbilityOnDemandCondition condition;
            if (!ReadOnDemandConditionFromParcel(condition, reply)) {
                HILOGW("ReadOnDemandEvents read condition failed!");
                return false;
            }
            event.conditions.push_back(condition);
        }
        if (!reply.ReadBool(event.enableOnce)) {
            HILOGW("ReadOnDemandEvents read enableOnce failed!");
            return false;
        }
        abilityOnDemandEvents.push_back(event);
    }
    return true;
}

bool OnDemandEventToParcel::ReadOnDemandEventFromParcel(SystemAbilityOnDemandEvent& event, MessageParcel& reply)
{
    int32_t eventId = 0;
    if (!reply.ReadInt32(eventId)) {
        HILOGW("ReadOnDemandEvent read eventId failed!");
        return false;
    }
    event.eventId = static_cast<OnDemandEventId>(eventId);
    if (!reply.ReadString(event.name)) {
        HILOGW("ReadOnDemandEvent read name failed!");
        return false;
    }
    if (!reply.ReadString(event.value)) {
        HILOGW("ReadOnDemandEvent read value failed!");
        return false;
    }
    return true;
}

bool OnDemandEventToParcel::ReadOnDemandConditionFromParcel(SystemAbilityOnDemandCondition& condition,
    MessageParcel& reply)
{
    int32_t eventId = 0;
    if (!reply.ReadInt32(eventId)) {
        HILOGW("ReadOnDemandCondition read eventId failed!");
        return false;
    }
    condition.eventId = static_cast<OnDemandEventId>(eventId);
    if (!reply.ReadString(condition.name)) {
        HILOGW("ReadOnDemandCondition read name failed!");
        return false;
    }
    if (!reply.ReadString(condition.value)) {
        HILOGW("ReadOnDemandCondition read value failed!");
        return false;
    }
    return true;
}
}