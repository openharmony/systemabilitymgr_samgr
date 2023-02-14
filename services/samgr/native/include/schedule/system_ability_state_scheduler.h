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

#ifndef OHOS_SYSTEM_ABILITY_MANAGER_SYSTEM_ABILITY_STATE_SCHEDULER_H
#define OHOS_SYSTEM_ABILITY_MANAGER_SYSTEM_ABILITY_STATE_SCHEDULER_H

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <shared_mutex>

#include "event_handler.h"
#include "sa_profiles.h"
#include "schedule/system_ability_event_handler.h"

namespace OHOS {
struct ProcessInfo {
    std::u16string processName;
    int32_t pid = -1;
    int32_t uid = -1;
};

class SystemAbilityStateScheduler : public SystemAbilityStateListener,
    public std::enable_shared_from_this<SystemAbilityStateScheduler> {
public:
    SystemAbilityStateScheduler() = default;
    virtual ~SystemAbilityStateScheduler() = default;
    void Init(const std::list<SaProfile>& saProfiles);
    
    int32_t HandleLoadAbilityEvent(const LoadRequestInfo& loadRequestInfo);
    int32_t HandleLoadAbilityEvent(int32_t systemAbilityId, bool& isExist);
    int32_t HandleUnloadAbilityEvent(int32_t systemAbilityId, UnloadReason unloadReason);
    int32_t SendAbilityStateEvent(int32_t systemAbilityId, AbilityStateEvent event);
    int32_t SendProcessStateEvent(const ProcessInfo& processInfo, ProcessStateEvent event);
    bool IsSystemAbilityUnloading(int32_t systemAbilityId);
private:
    void InitStateContext(const std::list<SaProfile>& saProfiles);

    bool GetSystemAbilityContext(int32_t systemAbilityId,
        std::shared_ptr<SystemAbilityContext>& abilityContext);
    bool GetSystemProcessContext(const std::u16string& processName,
        std::shared_ptr<SystemProcessContext>& processContext);

    int32_t HandleLoadAbilityEventLocked(const std::shared_ptr<SystemAbilityContext>& abilityContext,
        const LoadRequestInfo& loadRequestInfo);
    int32_t HandleUnloadAbilityEventLocked(const std::shared_ptr<SystemAbilityContext>& abilityContext,
        UnloadReason unloadReason);

    int32_t SendDelayUnloadEventLocked(uint32_t systemAbilityId);
    int32_t RemoveDelayUnloadEventLocked(uint32_t systemAbilityId);
    int32_t ProcessDelayUnloadEvent(int32_t systemAbilityId);

    int32_t PendLoadEventLocked(const std::shared_ptr<SystemAbilityContext>& abilityContext,
        const LoadRequestInfo& loadRequestInfo);
    int32_t PendUnloadEventLocked(const std::shared_ptr<SystemAbilityContext>& abilityContext,
        UnloadReason unloadReason);
    int32_t RemovePendingUnloadEventLocked(const std::shared_ptr<SystemAbilityContext>& abilityContext);
    int32_t HandlePendingLoadEventLocked(const std::shared_ptr<SystemAbilityContext>& abilityContext);
    int32_t HandlePendingUnloadEventLocked(const std::shared_ptr<SystemAbilityContext>& abilityContext);

    int32_t DoLoadSystemAbilityLocked(const std::shared_ptr<SystemAbilityContext>& abilityContext,
        const LoadRequestInfo& loadRequestInfo);
    int32_t DoUnloadSystemAbilityLocked(const std::shared_ptr<SystemAbilityContext>& abilityContext);
    int32_t TryUnloadAllSystemAbility(const std::shared_ptr<SystemProcessContext>& processContext);
    bool CanUnloadAllSystemAbility(const std::shared_ptr<SystemProcessContext>& processContext);
    int32_t UnloadAllSystemAbilityLocked(const std::shared_ptr<SystemProcessContext>& processContext);

    int32_t TryKillSystemProcess(const std::shared_ptr<SystemProcessContext>& processContext);
    bool CanKillSystemProcess(const std::shared_ptr<SystemProcessContext>& processContext);
    int32_t KillSystemProcessLocked(const std::shared_ptr<SystemProcessContext>& processContext);

    void OnAbilityNotLoadedLocked(int32_t systemAbilityId) override;
    void OnAbilityLoadedLocked(int32_t systemAbilityId) override;
    void OnAbilityUnloadableLocked(int32_t systemAbilityId) override;
    void OnProcessNotStartedLocked(const std::u16string& processName) override;

    class UnloadEventHandler : public AppExecFwk::EventHandler {
    public:
        UnloadEventHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner,
            const std::weak_ptr<SystemAbilityStateScheduler>& stateScheduler)
            : AppExecFwk::EventHandler(runner), stateScheduler_(stateScheduler) {}
        ~UnloadEventHandler() = default;
        void ProcessEvent(const OHOS::AppExecFwk::InnerEvent::Pointer& event) override;
    private:
        std::weak_ptr<SystemAbilityStateScheduler> stateScheduler_;
    };

    std::shared_ptr<SystemAbilityStateMachine> stateMachine_;
    std::shared_ptr<SystemAbilityEventHandler> stateEventHandler_;
    std::shared_mutex abiltyMapLock_;
    std::shared_mutex processMapLock_;
    std::map<int32_t, std::shared_ptr<SystemAbilityContext>> abilityContextMap_;
    std::map<std::u16string, std::shared_ptr<SystemProcessContext>> processContextMap_;
    std::shared_ptr<UnloadEventHandler> unloadEventHandler_;
    std::shared_ptr<AppExecFwk::EventHandler> processHandler_;
};
} // namespace OHOS

#endif // !defined(OHOS_SYSTEM_ABILITY_MANAGER_SYSTEM_ABILITY_STATE_SCHEDULER_H)