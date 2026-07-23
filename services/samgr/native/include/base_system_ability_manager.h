/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#ifndef SERVICES_SAMGR_NATIVE_INCLUDE_BASE_SYSTEM_ABILITY_MANAGER_H
#define SERVICES_SAMGR_NATIVE_INCLUDE_BASE_SYSTEM_ABILITY_MANAGER_H

#include <cstdint>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>

#include "ability_death_recipient.h"
#include "device_status_collect_manager.h"
#include "ffrt_handler.h"
#include "if_local_ability_manager.h"
#include "if_system_ability_manager.h"
#include "isystem_ability_load_callback.h"
#include "isystem_ability_status_change.h"
#include "isystem_process_status_change.h"
#include "rpc_callback_imp.h"
#include "sa_profiles.h"
#include "schedule/system_ability_state_scheduler.h"
#include "samgr_ffrt_api.h"
#include "timer.h"

namespace OHOS {

struct SAInfo {
    sptr<IRemoteObject> remoteObj;
    bool isDistributed = false;
};

enum ListenerState {
    INIT = 0,
    NOTIFIED,
};

struct SAListener {
    sptr<ISystemAbilityStatusChange> listener;
    int32_t callingPid;
    ListenerState state = ListenerState::INIT;
    SAListener(sptr<ISystemAbilityStatusChange> lst, int32_t cpid, ListenerState sta = ListenerState::INIT)
        :listener(lst), callingPid(cpid), state(sta) {}
};

class BaseSystemAbilityManager {
public:
    virtual ~BaseSystemAbilityManager();

    virtual int32_t AddSystemAbility(int32_t systemAbilityId, const sptr<IRemoteObject>& ability,
        const ISystemAbilityManager::SAExtraProp& extraProp);
    virtual int32_t RemoveSystemAbility(int32_t systemAbilityId);
    int32_t RemoveSystemAbility(const sptr<IRemoteObject>& ability);
    int32_t RemoveDiedSystemAbility(int32_t systemAbilityId);
    virtual std::vector<std::u16string> ListSystemAbilities(uint32_t dumpFlags);

    virtual sptr<IRemoteObject> GetSystemAbility(int32_t systemAbilityId);
    virtual sptr<IRemoteObject> CheckSystemAbility(int32_t systemAbilityId);
    virtual sptr<IRemoteObject> CheckSystemAbility(int32_t systemAbilityId, bool& isExist);

    virtual int32_t SubscribeSystemAbility(int32_t systemAbilityId,
        const sptr<ISystemAbilityStatusChange>& listener);
    virtual int32_t UnSubscribeSystemAbility(int32_t systemAbilityId,
        const sptr<ISystemAbilityStatusChange>& listener);
    void UnSubscribeSystemAbility(const sptr<IRemoteObject>& remoteObject);

    int32_t AddOnDemandSystemAbilityInfo(int32_t systemAbilityId, const std::u16string& procName);
    bool DoLoadOnDemandAbility(int32_t systemAbilityId, bool& isExist);
    int32_t DoLoadSystemAbility(int32_t systemAbilityId, const std::u16string& procName,
        const sptr<ISystemAbilityLoadCallback>& callback, int32_t callingPid, const OnDemandEvent& event);
    virtual int32_t LoadSystemAbility(int32_t systemAbilityId, const sptr<ISystemAbilityLoadCallback>& callback);
    virtual int32_t UnloadSystemAbility(int32_t systemAbilityId);
    int32_t DoUnloadSystemAbility(int32_t systemAbilityId, const std::u16string& procName,
        const OnDemandEvent& event);
    virtual int32_t CancelUnloadSystemAbility(int32_t systemAbilityId);
    virtual int32_t UnloadAllIdleSystemAbility();
    virtual int32_t UnloadProcess(const std::vector<std::u16string>& processList);
    virtual int32_t GetLruIdleSystemAbilityProc(std::vector<IdleProcessInfo>& processInfos);
    virtual int32_t OnStartSystemAbilityFail(int32_t systemAbilityId, int32_t errCode);
    bool IdleSystemAbility(int32_t systemAbilityId, const std::u16string& procName,
        const nlohmann::json& idleReason, int32_t& delayTime);
    bool ActiveSystemAbility(int32_t systemAbilityId, const std::u16string& procName,
        const nlohmann::json& activeReason);

    virtual int32_t AddSystemProcess(const std::u16string& procName, const sptr<IRemoteObject>& procObject);
    int32_t RemoveSystemProcess(const sptr<IRemoteObject>& procObject);
    sptr<IRemoteObject> GetSystemProcess(const std::u16string& procName);
    virtual int32_t GetSystemProcessInfo(int32_t systemAbilityId, SystemProcessInfo& systemProcessInfo);
    virtual int32_t GetRunningSystemProcess(std::list<SystemProcessInfo>& systemProcessInfos);
    virtual int32_t SubscribeSystemProcess(const sptr<ISystemProcessStatusChange>& listener);
    virtual int32_t UnSubscribeSystemProcess(const sptr<ISystemProcessStatusChange>& listener);
    virtual int32_t SubscribeLowMemSystemProcess(const sptr<ISystemProcessStatusChange>& listener);
    virtual int32_t UnSubscribeLowMemSystemProcess(const sptr<ISystemProcessStatusChange>& listener);

    virtual int32_t GetOnDemandPolicy(int32_t systemAbilityId, OnDemandPolicyType type,
        std::vector<SystemAbilityOnDemandEvent>& abilityOnDemandEvents);
    virtual int32_t UpdateOnDemandPolicy(int32_t systemAbilityId, OnDemandPolicyType type,
        const std::vector<SystemAbilityOnDemandEvent>& abilityOnDemandEvents);
    virtual int32_t GetOnDemandSystemAbilityIds(std::vector<int32_t>& systemAbilityIds);
    virtual int32_t SendStrategy(int32_t type, std::vector<int32_t>& systemAbilityIds,
        int32_t level, std::string& action);
    virtual int32_t GetOnDemandReasonExtraData(int64_t extraDataId, MessageParcel& extraDataParcel);
    virtual void ProcessOnDemandEvent(const OnDemandEvent& event, const std::list<SaControlInfo>& saControlList);

    virtual int32_t GetExtensionSaIds(const std::string& extension, std::vector<int32_t>& saIds);
    virtual int32_t GetExtensionRunningSaList(const std::string& extension,
        std::vector<sptr<IRemoteObject>>& saList);
    virtual int32_t GetRunningSaExtensionInfoList(const std::string& extension,
        std::vector<ISystemAbilityManager::SaExtensionInfo>& infoList);
    virtual int32_t GetCommonEventExtraDataIdlist(int32_t saId, std::vector<int64_t>& extraDataIdList,
        const std::string& eventName = "");

    void Init();
    void CleanFfrt();
    void SetFfrt();
    bool GetSaProfile(int32_t saId, CommonSaProfile& saProfile);
    bool IsDistributedSystemAbility(int32_t systemAbilityId);
    bool CheckSaIsImmediatelyRecycle(int32_t systemAbilityId);
    bool IsCacheCommonEvent(int32_t systemAbilityId);
    bool IsModuleUpdate(int32_t systemAbilityId);
    void RemoveOnDemandSaInDiedProc(std::shared_ptr<SystemProcessContext>& processContext);
#ifdef SUPPORT_MULTI_INSTANCE
    std::set<int32_t> GetMultiInstanceSaIds();
#endif

    void OnAbilityCallbackDied(const sptr<IRemoteObject>& remoteObject);
    void OnRemoteCallbackDied(const sptr<IRemoteObject>& remoteObject);

protected:
    BaseSystemAbilityManager() = default;
    virtual void OnSystemAbilityRegistered(int32_t systemAbilityId, bool isDistributed) {}

    static bool CheckInputSysAbilityId(int32_t sysAbilityId)
    {
        if (sysAbilityId >= 0 && sysAbilityId <= 0x00ffffff) {
            return true;
        }
        return false;
    }

    enum class AbilityState {
        INIT,
        STARTING,
        STARTED,
    };

    using CallbackList = std::list<std::pair<sptr<ISystemAbilityLoadCallback>, int32_t>>;

    struct AbilityItem {
        AbilityState state = AbilityState::INIT;
        std::map<std::string, CallbackList> callbackMap;
        OnDemandEvent event;
    };

    struct StartingProcessInfo {
        std::u16string procName;
        int32_t callingPid = -1;
        int32_t callUid = -1;
        std::string callPname;
        int32_t calleeSaId = -1;
        int64_t begin;
    };

    int32_t StartOnDemandAbility(int32_t systemAbilityId, bool& isExist);
    int32_t StartOnDemandAbilityLocked(int32_t systemAbilityId, bool& isExist);
    void StartOnDemandAbility(const std::u16string& name, int32_t systemAbilityId);
    void StartOnDemandAbilityLocked(const std::u16string& name, int32_t systemAbilityId);
    int32_t StartOnDemandAbilityInner(const std::u16string& name, int32_t systemAbilityId,
        AbilityItem& abilityItem);
    bool StopOnDemandAbility(const std::u16string& name, int32_t systemAbilityId, const OnDemandEvent& event);
    bool StopOnDemandAbilityInner(const std::u16string& name, int32_t systemAbilityId,
        const OnDemandEvent& event);

    int32_t StartingSystemProcess(const std::u16string& name, int32_t systemAbilityId,
        const OnDemandEvent& event);
    int32_t StartingSystemProcessLocked(const std::u16string& name, int32_t systemAbilityId,
        const OnDemandEvent& event);
    virtual int32_t StartDynamicSystemProcess(const std::u16string& name, int32_t systemAbilityId,
        const OnDemandEvent& event);
    bool IsInitBootFinished();

    void RefreshListenerState(int32_t systemAbilityId);
    int32_t FindSystemAbilityNotify(int32_t systemAbilityId, int32_t code);
    int32_t FindSystemAbilityNotify(int32_t systemAbilityId, const std::string& deviceId, int32_t code);
    void CheckListenerNotify(int32_t systemAbilityId,
        const sptr<ISystemAbilityStatusChange>& listener);
    void NotifySystemAbilityChanged(int32_t systemAbilityId, const std::string& deviceId, int32_t code,
        const sptr<ISystemAbilityStatusChange>& listener);
    void NotifySystemAbilityAddedByAsync(int32_t systemAbilityId,
        const sptr<ISystemAbilityStatusChange>& listener);
    void UnSubscribeSystemAbilityLocked(std::list<SAListener>& listenerList,
        const sptr<IRemoteObject>& listener);

    void SendSystemAbilityAddedMsg(int32_t systemAbilityId, const sptr<IRemoteObject>& remoteObject);
    void SendSystemAbilityRemovedMsg(int32_t systemAbilityId);
    void SendCheckLoadedMsg(int32_t systemAbilityId, const std::u16string& name,
        const std::string& srcDeviceId, const sptr<ISystemAbilityLoadCallback>& callback);
    void RemoveCheckLoadedMsg(int32_t systemAbilityId);
    void SendLoadedSystemAbilityMsg(int32_t systemAbilityId, const sptr<IRemoteObject>& remoteObject,
        const sptr<ISystemAbilityLoadCallback>& callback);
    void NotifySystemAbilityLoaded(int32_t systemAbilityId, const sptr<IRemoteObject>& remoteObject);
    void NotifySystemAbilityLoaded(int32_t systemAbilityId, const sptr<IRemoteObject>& remoteObject,
        const sptr<ISystemAbilityLoadCallback>& callback);
    void NotifySystemAbilityLoadFail(int32_t systemAbilityId,
        const sptr<ISystemAbilityLoadCallback>& callback, int32_t errCode);
    void CleanCallbackForLoadFailed(int32_t systemAbilityId, const std::u16string& name,
        const std::string& srcDeviceId, const sptr<ISystemAbilityLoadCallback>& callback);

    void RemoveStartingAbilityCallback(CallbackList& callbackList,
        const sptr<IRemoteObject>& remoteObject);
    void RemoveStartingAbilityCallbackForDevice(AbilityItem& abilityItem,
        const sptr<IRemoteObject>& remoteObject);
    void RemoveStartingAbilityCallbackLocked(
        std::pair<sptr<ISystemAbilityLoadCallback>, int32_t>& itemPair);
    void RemoveRemoteCallbackLocked(std::list<sptr<ISystemAbilityLoadCallback>>& callbacks,
        const sptr<IRemoteObject>& remoteObject);

    void InitSaProfile();
    void SystemAbilityInvalidateCache(int32_t systemAbilityId);

    int32_t UpdateSaFreMap(int32_t uid, int32_t saId);

    samgr::shared_mutex abilityMapLock_;
    std::map<int32_t, SAInfo> abilityMap_;

    samgr::mutex listenerMapLock_;
    std::map<int32_t, std::list<SAListener>> listenerMap_;
    std::map<int32_t, int32_t> subscribeCountMap_;

    samgr::mutex onDemandLock_;
    std::map<int32_t, std::u16string> onDemandAbilityMap_;
    std::map<int32_t, AbilityItem> startingAbilityMap_;

    samgr::mutex systemProcessMapLock_;
    std::map<std::u16string, sptr<IRemoteObject>> systemProcessMap_;

    samgr::mutex startingProcessMapLock_;
    std::map<std::u16string, StartingProcessInfo> startingProcessMap_;
    std::map<int32_t, int32_t> callbackCountMap_;

    std::map<int32_t, CommonSaProfile> saProfileMap_;
    std::set<int32_t> onDemandSaIdsSet_;
    samgr::mutex saProfileMapLock_;

    samgr::mutex loadRemoteLock_;
    std::map<std::string, std::list<sptr<ISystemAbilityLoadCallback>>> remoteCallbacks_;

    samgr::mutex saFrequencyLock_;
    std::map<uint64_t, int32_t> saFrequencyMap_; // {pid_said, count}
#ifdef SUPPORT_MULTI_INSTANCE
    samgr::mutex multiInstanceSaIdsLock_;
    std::set<int32_t> multiInstanceSaIds_;
#endif

    std::shared_ptr<FFRTHandler> workHandler_;

    std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler_;

    sptr<DeviceStatusCollectManager> collectManager_;

    std::unique_ptr<Utils::Timer> reportEventTimer_;

    sptr<IRemoteObject::DeathRecipient> abilityDeath_;
    sptr<IRemoteObject::DeathRecipient> systemProcessDeath_;
    sptr<IRemoteObject::DeathRecipient> abilityStatusDeath_;
    sptr<IRemoteObject::DeathRecipient> abilityCallbackDeath_;
    sptr<IRemoteObject::DeathRecipient> remoteCallbackDeath_;
};

} // namespace OHOS

#endif // !defined(SERVICES_SAMGR_NATIVE_INCLUDE_BASE_SYSTEM_ABILITY_MANAGER_H)
