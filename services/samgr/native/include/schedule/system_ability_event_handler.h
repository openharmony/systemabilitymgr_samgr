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

#ifndef OHOS_SYSTEM_ABILITY_MANAGER_SYSTEM_ABILITY_EVENT_HANDLER_H
#define OHOS_SYSTEM_ABILITY_MANAGER_SYSTEM_ABILITY_EVENT_HANDLER_H

#include "schedule/system_ability_state_machine.h"

namespace OHOS {
struct ProcessInfo {
    std::u16string processName;
    int32_t pid = -1;
    int32_t uid = -1;
};

enum class AbilityStateEvent {
    ABILITY_LOAD_SUCCESS_EVENT = 0,
    ABILITY_LOAD_FAILED_EVENT,
    ABILITY_UNLOAD_SUCCESS_EVENT,
};

enum class ProcessStateEvent {
    PROCESS_STARTED_EVENT = 0,
    PROCESS_STOPPED_EVENT,
};

class SystemAbilityEventHandler {
public:
    explicit SystemAbilityEventHandler(const std::shared_ptr<SystemAbilityStateMachine>& stateMachine);
    int32_t HandleAbilityEventLocked(const std::shared_ptr<SystemAbilityContext>& context, AbilityStateEvent event);
    int32_t HandleProcessEventLocked(const std::shared_ptr<SystemProcessContext>& context,
        const ProcessInfo& processInfo, ProcessStateEvent event);
private:
    void InitEventHandlerMap();
    int32_t HandleAbilityLoadFailedEventLocked(const std::shared_ptr<SystemAbilityContext>& context);
    int32_t HandleAbilityLoadSuccessEventLocked(const std::shared_ptr<SystemAbilityContext>& context);
    int32_t HandleAbilityUnLoadSuccessEventLocked(const std::shared_ptr<SystemAbilityContext>& context);
    int32_t HandleProcessStartedEventLocked(const std::shared_ptr<SystemProcessContext>& context,
        const ProcessInfo& processInfo);
    int32_t HandleProcessStoppedEventLocked(const std::shared_ptr<SystemProcessContext>& context,
        const ProcessInfo& processInfo);
    using AbilityEventHandlerFunc =
        int32_t(SystemAbilityEventHandler::*)(const std::shared_ptr<SystemAbilityContext>& context);
    using ProcessEventHandlerFunc =
        int32_t(SystemAbilityEventHandler::*)(const std::shared_ptr<SystemProcessContext>& context,
        const ProcessInfo& processInfo);
    std::shared_ptr<SystemAbilityStateMachine> stateMachine_;
    std::map<AbilityStateEvent, AbilityEventHandlerFunc> abilityEventHandlerMap_;
    std::map<ProcessStateEvent, ProcessEventHandlerFunc> processEventHandlerMap_;
};
} // namespace OHOS

#endif // !defined(OHOS_SYSTEM_ABILITY_MANAGER_SYSTEM_ABILITY_EVENT_HANDLER_H)