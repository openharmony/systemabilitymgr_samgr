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

#ifndef OHOS_SYSTEM_ABILITY_MANAGER_SYSTEM_ABILITY_STATE_MACHINE_H
#define OHOS_SYSTEM_ABILITY_MANAGER_SYSTEM_ABILITY_STATE_MACHINE_H

#include <map>
#include <memory>
#include <string>

#include "schedule/system_ability_state_context.h"
#include "schedule/system_ability_state_listener.h"

namespace OHOS {
class SystemAbilityStateHandler {
public:
    explicit SystemAbilityStateHandler(const std::shared_ptr<SystemAbilityStateListener>& listener)
        : listener_(listener) {};
    virtual ~SystemAbilityStateHandler() {};
    virtual bool CanEnter(SystemAbilityState fromState) = 0;
    virtual void OnEnter(const std::shared_ptr<SystemAbilityContext>& context) = 0;
protected:
    std::weak_ptr<SystemAbilityStateListener> listener_;
};

class NotLoadedStateHandler : public SystemAbilityStateHandler {
public:
    explicit NotLoadedStateHandler(const std::shared_ptr<SystemAbilityStateListener>& listener)
        : SystemAbilityStateHandler(listener) {};
    bool CanEnter(SystemAbilityState fromState) override;
    void OnEnter(const std::shared_ptr<SystemAbilityContext>& context) override;
};

class LoadingStateHandler : public SystemAbilityStateHandler {
public:
    explicit LoadingStateHandler(const std::shared_ptr<SystemAbilityStateListener>& listener)
        : SystemAbilityStateHandler(listener) {};
    bool CanEnter(SystemAbilityState fromState) override;
    void OnEnter(const std::shared_ptr<SystemAbilityContext>& context) override;
};

class LoadedStateHandler : public SystemAbilityStateHandler {
public:
    explicit LoadedStateHandler(const std::shared_ptr<SystemAbilityStateListener>& listener)
        : SystemAbilityStateHandler(listener) {};
    bool CanEnter(SystemAbilityState fromState) override;
    void OnEnter(const std::shared_ptr<SystemAbilityContext>& context) override;
};

class UnloadableStateHandler : public SystemAbilityStateHandler {
public:
    explicit UnloadableStateHandler(const std::shared_ptr<SystemAbilityStateListener>& listener)
        : SystemAbilityStateHandler(listener) {};
    bool CanEnter(SystemAbilityState fromState) override;
    void OnEnter(const std::shared_ptr<SystemAbilityContext>& context) override;
};

class UnloadingStateHandler : public SystemAbilityStateHandler {
public:
    explicit UnloadingStateHandler(const std::shared_ptr<SystemAbilityStateListener>& listener)
        : SystemAbilityStateHandler(listener) {};
    bool CanEnter(SystemAbilityState fromState) override;
    void OnEnter(const std::shared_ptr<SystemAbilityContext>& context) override;
};

class SystemProcessStateHandler {
public:
    explicit SystemProcessStateHandler(const std::shared_ptr<SystemAbilityStateListener>& listener)
        : listener_(listener) {};
    virtual ~SystemProcessStateHandler() {};
    virtual bool CanEnter(SystemProcessState fromState) = 0;
    virtual void OnEnter(const std::shared_ptr<SystemProcessContext>& context) = 0;
protected:
    std::weak_ptr<SystemAbilityStateListener> listener_;
};

class NotStartedStateHandler : public SystemProcessStateHandler {
public:
    explicit NotStartedStateHandler(const std::shared_ptr<SystemAbilityStateListener>& listener)
        : SystemProcessStateHandler(listener) {};
    bool CanEnter(SystemProcessState fromState) override;
    void OnEnter(const std::shared_ptr<SystemProcessContext>& context) override;
};

class StartedStateHandler : public SystemProcessStateHandler {
public:
    explicit StartedStateHandler(const std::shared_ptr<SystemAbilityStateListener>& listener)
        : SystemProcessStateHandler(listener) {};
    bool CanEnter(SystemProcessState fromState) override;
    void OnEnter(const std::shared_ptr<SystemProcessContext>& context) override;
};

class StoppingStateHandler : public SystemProcessStateHandler {
public:
    explicit StoppingStateHandler(const std::shared_ptr<SystemAbilityStateListener>& listener)
        : SystemProcessStateHandler(listener) {};
    bool CanEnter(SystemProcessState fromState) override;
    void OnEnter(const std::shared_ptr<SystemProcessContext>& context) override;
};

class SystemAbilityStateMachine {
public:
    explicit SystemAbilityStateMachine(const std::shared_ptr<SystemAbilityStateListener>& listener);
    int32_t AbilityStateTransitionLocked(const std::shared_ptr<SystemAbilityContext>& context,
        SystemAbilityState state);
    int32_t ProcessStateTransitionLocked(const std::shared_ptr<SystemProcessContext>& context,
        SystemProcessState state);
private:
    void InitStateHandlerMap(const std::shared_ptr<SystemAbilityStateListener>& listener);
    bool UpdateStateCount(const std::shared_ptr<SystemProcessContext>& context,
        SystemAbilityState fromState, SystemAbilityState toState);
    std::map<SystemAbilityState, std::shared_ptr<SystemAbilityStateHandler>> abilityStateHandlerMap_;
    std::map<SystemProcessState, std::shared_ptr<SystemProcessStateHandler>> processStateHandlerMap_;
};
} // namespace OHOS

#endif // !defined(OHOS_SYSTEM_ABILITY_MANAGER_SYSTEM_ABILITY_STATE_MACHINE_H)