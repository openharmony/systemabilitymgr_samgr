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

#include "ffrt_handler.h"
#include "isystem_process_status_change.h"
#include "nlohmann/json.hpp"
#include "sa_profiles.h"
#include "schedule/system_ability_event_handler.h"

namespace OHOS {
constexpr int32_t UNLOAD_DELAY_TIME = 20 * 1000;

class SystemAbilityStateScheduler : public SystemAbilityStateListener,
    public std::enable_shared_from_this<SystemAbilityStateScheduler> {
public:
    SystemAbilityStateScheduler() = default;
    virtual ~SystemAbilityStateScheduler() = default;
    void Init(const std::list<SaProfile>& saProfiles);
    void CleanFfrt();
    void SetFfrt();

    int32_t HandleAbilityDiedEvent(int32_t systemAbilityId);
    int32_t HandleLoadAbilityEvent(const LoadRequestInfo& loadRequestInfo);
    int32_t HandleLoadAbilityEvent(int32_t systemAbilityId, bool& isExist);
    int32_t HandleUnloadAbilityEvent(const std::shared_ptr<UnloadRequestInfo> unloadRequestInfo);
    int32_t HandleCancelUnloadAbilityEvent(int32_t systemAbilityId);
    int32_t UnloadAllIdleSystemAbility();
    int32_t SendAbilityStateEvent(int32_t systemAbilityId, AbilityStateEvent event);
    int32_t SendProcessStateEvent(const ProcessInfo& processInfo, ProcessStateEvent event);
    bool IsSystemAbilityUnloading(int32_t systemAbilityId);

    int32_t GetSystemProcessInfo(int32_t systemAbilityId, SystemProcessInfo& systemProcessInfo);
    int32_t GetRunningSystemProcess(std::list<SystemProcessInfo>& systemProcessInfos);
    int32_t GetProcessNameByProcessId(int32_t pid, std::u16string& processName);
    void GetAllSystemAbilityInfo(std::string& result);
    void GetSystemAbilityInfo(int32_t said, std::string& result);
    void GetProcessInfo(const std::string& processName, std::string& result);
    void GetAllSystemAbilityInfoByState(const std::string& state, std::string& result);
    int32_t SubscribeSystemProcess(const sptr<ISystemProcessStatusChange>& listener);
    int32_t UnSubscribeSystemProcess(const sptr<ISystemProcessStatusChange>& listener);
    bool IsSystemProcessNeverStartedLocked(const std::u16string& processName);
    void InitSamgrProcessContext();
    void CheckEnableOnce(const OnDemandEvent& event, const std::list<SaControlInfo>& saControlList);
    int32_t CheckStartEnableOnce(const OnDemandEvent& event, const SaControlInfo& saControl,
        sptr<ISystemAbilityLoadCallback> callback);
    int32_t CheckStopEnableOnce(const OnDemandEvent& event, const SaControlInfo& saControl);
    void UpdateLimitDelayUnloadTime(int32_t systemAbilityId);
    void UpdateLimitDelayUnloadTimeTask(int32_t systemAbilityId);
private:
    void InitStateContext(const std::list<SaProfile>& saProfiles);

    int32_t LimitDelayUnloadTime(int32_t delayUnloadTime);
    bool GetSystemAbilityContext(int32_t systemAbilityId,
        std::shared_ptr<SystemAbilityContext>& abilityContext);
    bool GetSystemProcessContext(const std::u16string& processName,
        std::shared_ptr<SystemProcessContext>& processContext);

    int32_t HandleLoadAbilityEventLocked(const std::shared_ptr<SystemAbilityContext>& abilityContext,
        const LoadRequestInfo& loadRequestInfo);
    int32_t HandleUnloadAbilityEventLocked(const std::shared_ptr<SystemAbilityContext>& abilityContext,
        const std::shared_ptr<UnloadRequestInfo> unloadRequestInfo);

    int32_t SendDelayUnloadEventLocked(uint32_t systemAbilityId, int32_t delayTime = UNLOAD_DELAY_TIME);
    int32_t RemoveDelayUnloadEventLocked(uint32_t systemAbilityId);
    int32_t ProcessDelayUnloadEvent(int32_t systemAbilityId);
    int32_t ProcessDelayUnloadEventLocked(int32_t systemAbilityId);

    int32_t PendLoadEventLocked(const std::shared_ptr<SystemAbilityContext>& abilityContext,
        const LoadRequestInfo& loadRequestInfo);
    int32_t PendUnloadEventLocked(const std::shared_ptr<SystemAbilityContext>& abilityContext,
        const std::shared_ptr<UnloadRequestInfo> unloadRequestInfo);
    int32_t RemovePendingUnloadEventLocked(const std::shared_ptr<SystemAbilityContext>& abilityContext);
    int32_t HandlePendingLoadEventLocked(const std::shared_ptr<SystemAbilityContext>& abilityContext);
    int32_t HandlePendingUnloadEventLocked(const std::shared_ptr<SystemAbilityContext>& abilityContext);

    int32_t DoLoadSystemAbilityLocked(const std::shared_ptr<SystemAbilityContext>& abilityContext,
        const LoadRequestInfo& loadRequestInfo);
    int32_t DoUnloadSystemAbilityLocked(const std::shared_ptr<SystemAbilityContext>& abilityContext);

    int32_t PostTryUnloadAllAbilityTask(const std::shared_ptr<SystemProcessContext>& processContext);
    int32_t PostUnloadTimeoutTask(const std::shared_ptr<SystemProcessContext>& processContext);
    void RemoveUnloadTimeoutTask(const std::shared_ptr<SystemProcessContext>& processContext);
    int32_t PostTryKillProcessTask(const std::shared_ptr<SystemProcessContext>& processContext);

    int32_t TryUnloadAllSystemAbility(const std::shared_ptr<SystemProcessContext>& processContext);
    bool CanUnloadAllSystemAbility(const std::shared_ptr<SystemProcessContext>& processContext);
    bool CanUnloadAllSystemAbilityLocked(const std::shared_ptr<SystemProcessContext>& processContext,
        bool isNeedCheckRecycleStrategy = false);
    bool CheckSaIsImmediatelyRecycle(const std::shared_ptr<SystemProcessContext>& processContext);
    int32_t UnloadAllSystemAbilityLocked(const std::shared_ptr<SystemProcessContext>& processContext);

    int32_t TryKillSystemProcess(const std::shared_ptr<SystemProcessContext>& processContext);
    bool CanKillSystemProcess(const std::shared_ptr<SystemProcessContext>& processContext);
    bool CanKillSystemProcessLocked(const std::shared_ptr<SystemProcessContext>& processContext);
    int32_t KillSystemProcessLocked(const std::shared_ptr<SystemProcessContext>& processContext);

    bool CanRestartProcessLocked(const std::shared_ptr<SystemProcessContext>& processContext);
    int32_t GetAbnormallyDiedAbilityLocked(std::shared_ptr<SystemProcessContext>& processContext,
        std::list<std::shared_ptr<SystemAbilityContext>>& abnormallyDiedAbilityList);
    int32_t HandleAbnormallyDiedAbilityLocked(std::shared_ptr<SystemProcessContext>& processContext,
        std::list<std::shared_ptr<SystemAbilityContext>>& abnormallyDiedAbilityList);

    void NotifyProcessStarted(const std::shared_ptr<SystemProcessContext>& processContext);
    void NotifyProcessStopped(const std::shared_ptr<SystemProcessContext>& processContext);
    void OnAbilityNotLoadedLocked(int32_t systemAbilityId) override;
    void OnAbilityLoadedLocked(int32_t systemAbilityId) override;
    void OnAbilityUnloadableLocked(int32_t systemAbilityId) override;
    void OnProcessNotStartedLocked(const std::u16string& processName) override;
    void OnProcessStartedLocked(const std::u16string& processName) override;

    int32_t ActiveSystemAbilityLocked(const std::shared_ptr<SystemAbilityContext>& abilityContext,
        const nlohmann::json& activeReason);

    class UnloadEventHandler {
    public:
        UnloadEventHandler(const std::weak_ptr<SystemAbilityStateScheduler>& stateScheduler)
            : stateScheduler_(stateScheduler)
        {
            handler_ = std::make_shared<FFRTHandler>("UnloadEventHandler");
        }
        ~UnloadEventHandler() = default;
        void ProcessEvent(uint32_t eventId);
        bool SendEvent(uint32_t eventId, int64_t extraDataId, uint64_t delayTime);
        void RemoveEvent(uint32_t eventId);
        bool HasInnerEvent(uint32_t eventId);
        void CleanFfrt();
        void SetFfrt();

    private:
        std::weak_ptr<SystemAbilityStateScheduler> stateScheduler_;
        std::shared_ptr<FFRTHandler> handler_;
    };
    std::shared_ptr<SystemAbilityStateMachine> stateMachine_;
    std::shared_ptr<SystemAbilityEventHandler> stateEventHandler_;
    std::shared_mutex abiltyMapLock_;
    std::shared_mutex processMapLock_;
    std::map<int32_t, std::shared_ptr<SystemAbilityContext>> abilityContextMap_;
    std::map<std::u16string, std::shared_ptr<SystemProcessContext>> processContextMap_;
    std::shared_ptr<UnloadEventHandler> unloadEventHandler_;
    std::shared_ptr<FFRTHandler> processHandler_;
    std::shared_mutex listenerSetLock_;
    std::list<sptr<ISystemProcessStatusChange>> processListeners_;
    sptr<IRemoteObject::DeathRecipient> processListenerDeath_;
    std::mutex startEnableOnceLock_;
    std::map<int32_t, std::list<OnDemandEvent>> startEnableOnceMap_;
    std::mutex stopEnableOnceLock_;
    std::map<int32_t, std::list<OnDemandEvent>> stopEnableOnceMap_;
};
} // namespace OHOS

#endif // !defined(OHOS_SYSTEM_ABILITY_MANAGER_SYSTEM_ABILITY_STATE_SCHEDULER_H)