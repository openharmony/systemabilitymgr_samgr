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
        HILOGE("[SA Scheduler] context is nullptr");
        return ERR_INVALID_VALUE;
    }
    if (abilityStateHandlerMap_.count(nextState) == 0) {
        HILOGE("[SA Scheduler] invalid next state: %{public}d", nextState);
        return ERR_INVALID_VALUE;
    }
    std::shared_ptr<SystemAbilityStateHandler> handler = abilityStateHandlerMap_[nextState];
    if (handler == nullptr) {
        HILOGE("[SA Scheduler] next state: %{public}d handler is nullptr", nextState);
        return ERR_INVALID_VALUE;
    }
    SystemAbilityState currentState = context->state;
    if (currentState == nextState) {
        HILOGI("[SA Scheduler][SA: %{public}d] current state %{public}d is same as next state %{public}d",
            context->systemAbilityId, currentState, nextState);
        return ERR_OK;
    }
    if (!handler->CanEnter(currentState)) {
        HILOGE("[SA Scheduler][SA: %{public}d] cannot transiton from state %{public}d to state %{public}d",
            context->systemAbilityId, currentState, nextState);
        return ERR_INVALID_VALUE;
    }
    if (!UpdateStateCount(context->ownProcessContext, currentState, nextState)) {
        HILOGE("[SA Scheduler][SA: %{public}d] update state count failed", context->systemAbilityId);
        return ERR_INVALID_VALUE;
    }
    context->state = nextState;
    HILOGD("[SA Scheduler][SA: %{public}d] transiton from state %{public}d to state %{public}d",
        context->systemAbilityId, currentState, nextState);
    handler->OnEnter(context);
    return ERR_OK;
}

bool SystemAbilityStateMachine::UpdateStateCount(const std::shared_ptr<SystemProcessContext>& context,
    SystemAbilityState fromState, SystemAbilityState toState)
{
    if (context == nullptr) {
        HILOGE("[SA Scheduler] process context is nullptr");
        return false;
    }
    std::unique_lock<std::shared_mutex> uniqueLock(context->stateCountLock);
    if (!context->abilityStateCountMap.count(fromState) || !context->abilityStateCountMap.count(toState)) {
        HILOGE("[SA Scheduler][process: %{public}s] invalid state",
            Str16ToStr8(context->processName).c_str());
        return false;
    }
    if (context->abilityStateCountMap[fromState] <= 0) {
        HILOGE("[SA Scheduler][process: %{public}s] invalid current state count",
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
        HILOGE("[SA Scheduler] process context is nullptr");
        return ERR_INVALID_VALUE;
    }
    if (processStateHandlerMap_.count(nextState) == 0) {
        HILOGE("[SA Scheduler] invalid next state: %{public}d", nextState);
        return ERR_INVALID_VALUE;
    }
    std::shared_ptr<SystemProcessStateHandler> handler = processStateHandlerMap_[nextState];
    if (handler == nullptr) {
        HILOGE("[SA Scheduler] next state: %{public}d handler is nullptr", nextState);
        return ERR_INVALID_VALUE;
    }
    SystemProcessState currentState = context->state;
    if (currentState == nextState) {
        HILOGI("[SA Scheduler][process: %{public}s] current state %{public}d is same as next state %{public}d",
            Str16ToStr8(context->processName).c_str(), currentState, nextState);
        return ERR_OK;
    }
    if (!handler->CanEnter(currentState)) {
        HILOGI("[SA Scheduler][process: %{public}s] cannot transiton from state %{public}d to state %{public}d",
            Str16ToStr8(context->processName).c_str(), currentState, nextState);
        return ERR_INVALID_VALUE;
    }
    context->state = nextState;
    HILOGI("[SA Scheduler][process: %{public}s] transiton from state %{public}d to state %{public}d",
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
        HILOGE("[SA Scheduler] listener is nullptr");
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
        HILOGE("[SA Scheduler] listener is nullptr");
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
        HILOGE("[SA Scheduler] listener is nullptr");
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
        HILOGE("[SA Scheduler] listener is nullptr");
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
        HILOGE("[SA Scheduler] listener is nullptr");
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
        HILOGE("[SA Scheduler] listener is nullptr");
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
        HILOGE("[SA Scheduler] listener is nullptr");
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
        HILOGE("[SA Scheduler] listener is nullptr");
        return;
    }
    listener->OnProcessStoppingLocked(context->processName);
}
}  // namespace OHOS