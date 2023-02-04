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

#include "sam_log.h"
#include "string_ex.h"
#include "schedule/system_ability_event_handler.h"

namespace OHOS {
SystemAbilityEventHandler::SystemAbilityEventHandler(const std::shared_ptr<SystemAbilityStateMachine>& stateMachine)
{
    stateMachine_ = stateMachine;
    InitEventHandlerMap();
}

void SystemAbilityEventHandler::InitEventHandlerMap()
{
    abilityEventHandlerMap_[AbilityStateEvent::ABILITY_LOAD_FAILED_EVENT] =
        &SystemAbilityEventHandler::HandleAbilityLoadFailedEventLocked;
    abilityEventHandlerMap_[AbilityStateEvent::ABILITY_LOAD_SUCCESS_EVENT] =
        &SystemAbilityEventHandler::HandleAbilityLoadSuccessEventLocked;
    abilityEventHandlerMap_[AbilityStateEvent::ABILITY_UNLOAD_SUCCESS_EVENT] =
        &SystemAbilityEventHandler::HandleAbilityUnLoadSuccessEventLocked;
    processEventHandlerMap_[ProcessStateEvent::PROCESS_STARTED_EVENT] =
        &SystemAbilityEventHandler::HandleProcessStartedEventLocked;
    processEventHandlerMap_[ProcessStateEvent::PROCESS_STOPPED_EVENT] =
        &SystemAbilityEventHandler::HandleProcessStoppedEventLocked;
}

int32_t SystemAbilityEventHandler::HandleAbilityEventLocked(const std::shared_ptr<SystemAbilityContext>& context,
    AbilityStateEvent event)
{
    if (context == nullptr) {
        HILOGE("[SA Scheduler] context is nullptr");
        return ERR_INVALID_VALUE;
    }
    HILOGI("[SA Scheduler][SA: %{public}d] handle ability event %{public}d start",
        context->systemAbilityId, event);
    auto iter = abilityEventHandlerMap_.find(event);
    if (iter != abilityEventHandlerMap_.end()) {
        auto func = iter->second;
        if (func != nullptr) {
            return (this->*func)(context);
        }
    }
    HILOGE("[SA Scheduler][SA: %{public}d] invalid ability event %{public}d", context->systemAbilityId, event);
    return ERR_INVALID_VALUE;
}

int32_t SystemAbilityEventHandler::HandleProcessEventLocked(const std::shared_ptr<SystemProcessContext>& context,
    ProcessStateEvent event)
{
    if (context == nullptr) {
        HILOGE("[SA Scheduler] context is nullptr");
        return ERR_INVALID_VALUE;
    }
    HILOGI("[SA Scheduler][process: %{public}s] handle process event %{public}d start",
        Str16ToStr8(context->processName).c_str(), event);
    auto iter = processEventHandlerMap_.find(event);
    if (iter != processEventHandlerMap_.end()) {
        auto func = iter->second;
        if (func != nullptr) {
            return (this->*func)(context);
        }
    }
    HILOGE("[SA Scheduler][process: %{public}s] invalid process event %{public}d",
        Str16ToStr8(context->processName).c_str(), event);
    return ERR_INVALID_VALUE;
}

int32_t SystemAbilityEventHandler::HandleAbilityLoadFailedEventLocked(
    const std::shared_ptr<SystemAbilityContext>& context)
{
    HILOGI("[SA Scheduler][SA: %{public}d] handle load failed event", context->systemAbilityId);
    int32_t result = ERR_OK;
    switch (context->state) {
        case SystemAbilityState::LOADING:
            if (context->pendingEvent == PendingEvent::UNLOAD_ABILITY_EVENT) {
                HILOGI("[SA Scheduler][SA: %{public}d] remove pending unload event", context->systemAbilityId);
                context->pendingEvent = PendingEvent::NO_EVENT;
            }
            result = stateMachine_->AbilityStateTransitionLocked(context, SystemAbilityState::NOT_LOADED);
            break;
        default:
            result = ERR_INVALID_VALUE;
            HILOGE("[SA Scheduler][SA: %{public}d] in state %{public}d, cannot handle load failed event",
                context->systemAbilityId, context->state);
            break;
    }
    return result;
}

int32_t SystemAbilityEventHandler::HandleAbilityLoadSuccessEventLocked(
    const std::shared_ptr<SystemAbilityContext>& context)
{
    HILOGI("[SA Scheduler][SA: %{public}d] handle load success event", context->systemAbilityId);
    int32_t result = ERR_OK;
    switch (context->state) {
        case SystemAbilityState::NOT_LOADED:
        case SystemAbilityState::LOADING:
            result = stateMachine_->AbilityStateTransitionLocked(context, SystemAbilityState::LOADED);
            break;
        default:
            result = ERR_INVALID_VALUE;
            HILOGE("[SA Scheduler][SA: %{public}d] in state %{public}d, cannot handle load success event",
                context->systemAbilityId, context->state);
            break;
    }
    return result;
}

int32_t SystemAbilityEventHandler::HandleAbilityUnLoadSuccessEventLocked(
    const std::shared_ptr<SystemAbilityContext>& context)
{
    HILOGI("[SA Scheduler][SA: %{public}d] handle unload success event", context->systemAbilityId);
    int32_t result = ERR_OK;
    switch (context->state) {
        case SystemAbilityState::LOADING:
            if (context->pendingEvent == PendingEvent::UNLOAD_ABILITY_EVENT) {
                HILOGI("[SA Scheduler][SA: %{public}d] remove pending unload event", context->systemAbilityId);
                context->pendingEvent = PendingEvent::NO_EVENT;
            }
            result = stateMachine_->AbilityStateTransitionLocked(context, SystemAbilityState::NOT_LOADED);
        case SystemAbilityState::LOADED:
        case SystemAbilityState::UNLOADABLE:
        case SystemAbilityState::UNLOADING:
            result = stateMachine_->AbilityStateTransitionLocked(context, SystemAbilityState::NOT_LOADED);
            break;
        default:
            result = ERR_INVALID_VALUE;
            HILOGE("[SA Scheduler][SA: %{public}d] in state %{public}d, not need handle unload success event",
                context->systemAbilityId, context->state);
            break;
    }
    return result;
}

int32_t SystemAbilityEventHandler::HandleProcessStartedEventLocked(
    const std::shared_ptr<SystemProcessContext>& context)
{
    HILOGI("[SA Scheduler][process: %{public}s] handle started event", Str16ToStr8(context->processName).c_str());
    int32_t result = ERR_OK;
    switch (context->state) {
        case SystemProcessState::NOT_STARTED:
            result = stateMachine_->ProcessStateTransitionLocked(context, SystemProcessState::STARTED);
            break;
        default:
            result = ERR_INVALID_VALUE;
            HILOGE("[SA Scheduler][process: %{public}s] in state %{public}d, not need handle started event",
                Str16ToStr8(context->processName).c_str(), context->state);
            break;
    }
    return result;
}

int32_t SystemAbilityEventHandler::HandleProcessStoppedEventLocked(
    const std::shared_ptr<SystemProcessContext>& context)
{
    HILOGI("[SA Scheduler][process: %{public}s] handle stopped event", Str16ToStr8(context->processName).c_str());
    int32_t result = ERR_OK;
    switch (context->state) {
        case SystemProcessState::STARTED:
        case SystemProcessState::STOPPING:
            result = stateMachine_->ProcessStateTransitionLocked(context, SystemProcessState::NOT_STARTED);
            break;
        default:
            result = ERR_INVALID_VALUE;
            HILOGE("[SA Scheduler][process: %{public}s] in state %{public}d, not need handle stopped event",
                Str16ToStr8(context->processName).c_str(), context->state);
            break;
    }
    return result;
}
}