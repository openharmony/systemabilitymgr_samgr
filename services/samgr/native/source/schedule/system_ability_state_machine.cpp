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

#include <shared_mutex>

#include "sam_log.h"
#include "string_ex.h"
#include "samgr_err_code.h"
#include "schedule/system_ability_state_machine.h"

namespace OHOS {
SystemAbilityStateMachine::SystemAbilityStateMachine(const std::shared_ptr<SystemAbilityStateListener>& listener)
{
    InitStateHandlerMap(listener);
}

void SystemAbilityStateMachine::InitStateHandlerMap(const std::shared_ptr<SystemAbilityStateListener>& listener)
{
    abilityStateHandlerMap_[SystemAbilityState::NOT_LOADED] = std::make_shared<NotLoadedStateHandler>(listener);
    abilityStateHandlerMap_[SystemAbilityState::LOADING] = std::make_shared<LoadingStateHandler>(listener);
    abilityStateHandlerMap_[SystemAbilityState::LOADED] = std::make_shared<LoadedStateHandler>(listener);
    abilityStateHandlerMap_[SystemAbilityState::UNLOADABLE] = std::make_shared<UnloadableStateHandler>(listener);
    abilityStateHandlerMap_[SystemAbilityState::UNLOADING] = std::make_shared<UnloadingStateHandler>(listener);

    processStateHandlerMap_[SystemProcessState::NOT_STARTED] = std::make_shared<NotStartedStateHandler>(listener);
    processStateHandlerMap_[SystemProcessState::STARTED] = std::make_shared<StartedStateHandler>(listener);
    processStateHandlerMap_[SystemProcessState::STOPPING] = std::make_shared<StoppingStateHandler>(listener);
}

int32_t SystemAbilityStateMachine::AbilityStateTransitionLocked(const std::shared_ptr<SystemAbilityContext>& context,
    const SystemAbilityState nextState)
{
    if (context == nullptr) {
        HILOGE("Scheduler:context is null");
        return SA_CONTEXT_NULL;
    }
    if (abilityStateHandlerMap_.count(nextState) == 0) {
        HILOGE("Scheduler:invalid next state:%{public}d", nextState);
        return INVALID_SA_NEXT_STATE;
    }
    std::shared_ptr<SystemAbilityStateHandler> handler = abilityStateHandlerMap_[nextState];
    if (handler == nullptr) {
        HILOGE("Scheduler:next state:%{public}d handler is null", nextState);
        return SA_STATE_HANDLER_NULL;
    }
    SystemAbilityState currentState = context->state;
    if (currentState == nextState) {
        HILOGI("Scheduler SA:%{public}d state current %{public}d is same as next %{public}d",
            context->systemAbilityId, currentState, nextState);
        return ERR_OK;
    }
    if (!handler->CanEnter(currentState)) {
        HILOGE("Scheduler SA:%{public}d can't transiton from state %{public}d to %{public}d",
            context->systemAbilityId, currentState, nextState);
        return TRANSIT_SA_STATE_FAIL;
    }
    if (!UpdateStateCount(context->ownProcessContext, currentState, nextState)) {
        HILOGE("Scheduler SA:%{public}d update state count fail", context->systemAbilityId);
        return UPDATE_STATE_COUNT_FAIL;
    }
    context->state = nextState;
    HILOGD("Scheduler SA:%{public}d transiton from state %{public}d to %{public}d",
        context->systemAbilityId, currentState, nextState);
    handler->OnEnter(context);
    return ERR_OK;
}

bool SystemAbilityStateMachine::UpdateStateCount(const std::shared_ptr<SystemProcessContext>& context,
    SystemAbilityState fromState, SystemAbilityState toState)
{
    if (context == nullptr) {
        HILOGE("Scheduler:proc context is null");
        return false;
    }
    std::lock_guard<std::mutex> autoLock(context->stateCountLock);
    if (!context->abilityStateCountMap.count(fromState) || !context->abilityStateCountMap.count(toState)) {
        HILOGE("Scheduler proc:%{public}s invalid state",
            Str16ToStr8(context->processName).c_str());
        return false;
    }
    if (context->abilityStateCountMap[fromState] <= 0) {
        HILOGE("Scheduler proc:%{public}s invalid current state count",
            Str16ToStr8(context->processName).c_str());
        return false;
    }
    context->abilityStateCountMap[fromState]--;
    context->abilityStateCountMap[toState]++;
    return true;
}

int32_t SystemAbilityStateMachine::ProcessStateTransitionLocked(const std::shared_ptr<SystemProcessContext>& context,
    SystemProcessState nextState)
{
    if (context == nullptr) {
        HILOGE("Scheduler:proc context is null");
        return PROC_CONTEXT_NULL;
    }
    if (processStateHandlerMap_.count(nextState) == 0) {
        HILOGE("Scheduler:invalid next state:%{public}d", nextState);
        return INVALID_PROC_NEXT_STATE;
    }
    std::shared_ptr<SystemProcessStateHandler> handler = processStateHandlerMap_[nextState];
    if (handler == nullptr) {
        HILOGE("Scheduler:next state:%{public}d handler is null", nextState);
        return PROC_STATE_HANDLER_NULL;
    }
    SystemProcessState currentState = context->state;
    if (currentState == nextState) {
        HILOGI("Scheduler proc:%{public}s state current %{public}d is same as next %{public}d",
            Str16ToStr8(context->processName).c_str(), currentState, nextState);
        return ERR_OK;
    }
    if (!handler->CanEnter(currentState)) {
        HILOGI("Scheduler proc:%{public}s can't transiton from state %{public}d to %{public}d",
            Str16ToStr8(context->processName).c_str(), currentState, nextState);
        return TRANSIT_PROC_STATE_FAIL;
    }
    context->state = nextState;
    HILOGI("Scheduler proc:%{public}s transiton from state %{public}d to %{public}d",
        Str16ToStr8(context->processName).c_str(), currentState, nextState);
    handler->OnEnter(context);
    return ERR_OK;
}

bool NotLoadedStateHandler::CanEnter(SystemAbilityState fromState)
{
    return fromState != SystemAbilityState::NOT_LOADED;
}

void NotLoadedStateHandler::OnEnter(const std::shared_ptr<SystemAbilityContext>& context)
{
    auto listener = listener_.lock();
    if (listener == nullptr) {
        HILOGE("Scheduler:listener is null");
        return;
    }
    listener->OnAbilityNotLoadedLocked(context->systemAbilityId);
}

bool LoadingStateHandler::CanEnter(SystemAbilityState fromState)
{
    return fromState == SystemAbilityState::NOT_LOADED;
}

void LoadingStateHandler::OnEnter(const std::shared_ptr<SystemAbilityContext>& context)
{
    auto listener = listener_.lock();
    if (listener == nullptr) {
        HILOGE("Scheduler:listener is null");
        return;
    }
    listener->OnAbilityLoadingLocked(context->systemAbilityId);
}

bool LoadedStateHandler::CanEnter(SystemAbilityState fromState)
{
    if (fromState == SystemAbilityState::NOT_LOADED
        || fromState == SystemAbilityState::LOADING
        || fromState == SystemAbilityState::UNLOADABLE) {
        return true;
    }
    return false;
}

void LoadedStateHandler::OnEnter(const std::shared_ptr<SystemAbilityContext>& context)
{
    auto listener = listener_.lock();
    if (listener == nullptr) {
        HILOGE("Scheduler:listener is null");
        return;
    }
    listener->OnAbilityLoadedLocked(context->systemAbilityId);
}

bool UnloadableStateHandler::CanEnter(SystemAbilityState fromState)
{
    return fromState == SystemAbilityState::LOADED;
}

void UnloadableStateHandler::OnEnter(const std::shared_ptr<SystemAbilityContext>& context)
{
    auto listener = listener_.lock();
    if (listener == nullptr) {
        HILOGE("Scheduler:listener is null");
        return;
    }
    listener->OnAbilityUnloadableLocked(context->systemAbilityId);
}

bool UnloadingStateHandler::CanEnter(SystemAbilityState fromState)
{
    return fromState == SystemAbilityState::UNLOADABLE;
}

void UnloadingStateHandler::OnEnter(const std::shared_ptr<SystemAbilityContext>& context)
{
    auto listener = listener_.lock();
    if (listener == nullptr) {
        HILOGE("Scheduler:listener is null");
        return;
    }
    listener->OnAbilityUnloadingLocked(context->systemAbilityId);
}

bool NotStartedStateHandler::CanEnter(SystemProcessState fromState)
{
    return fromState != SystemProcessState::NOT_STARTED;
}

void NotStartedStateHandler::OnEnter(const std::shared_ptr<SystemProcessContext>& context)
{
    auto listener = listener_.lock();
    if (listener == nullptr) {
        HILOGE("Scheduler:listener is null");
        return;
    }
    listener->OnProcessNotStartedLocked(context->processName);
}

bool StartedStateHandler::CanEnter(SystemProcessState fromState)
{
    return fromState == SystemProcessState::NOT_STARTED;
}

void StartedStateHandler::OnEnter(const std::shared_ptr<SystemProcessContext>& context)
{
    auto listener = listener_.lock();
    if (listener == nullptr) {
        HILOGE("Scheduler:listener is null");
        return;
    }
    listener->OnProcessStartedLocked(context->processName);
}

bool StoppingStateHandler::CanEnter(SystemProcessState fromState)
{
    return fromState == SystemProcessState::STARTED;
}

void StoppingStateHandler::OnEnter(const std::shared_ptr<SystemProcessContext>& context)
{
    auto listener = listener_.lock();
    if (listener == nullptr) {
        HILOGE("Scheduler:listener is null");
        return;
    }
    listener->OnProcessStoppingLocked(context->processName);
}
}  // namespace OHOS