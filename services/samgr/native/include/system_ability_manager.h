/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef SERVICES_SAMGR_NATIVE_INCLUDE_SYSTEM_ABILITY_MANAGER_H
#define SERVICES_SAMGR_NATIVE_INCLUDE_SYSTEM_ABILITY_MANAGER_H

#include <map>
#include <mutex>
#include <set>
#include <shared_mutex>
#include <string>
#include <utility>

#include "dbinder_service.h"
#include "dbinder_service_stub.h"
#include "device_status_collect_manager.h"
#include "dynamic_cache.h"
#include "ffrt_handler.h"
#include "rpc_callback_imp.h"
#include "thread_pool.h"
#include "timer.h"
#include "sa_profiles.h"
#include "system_ability_manager_stub.h"
#include "schedule/system_ability_state_scheduler.h"

namespace OHOS {
struct SAInfo {
    sptr<IRemoteObject> remoteObj;
    bool isDistributed = false;
    std::u16string capability;
    std::string permission;
};

enum {
    UUID = 0,
    NODE_ID,
    UNKNOWN,
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

class SystemAbilityManager : public DynamicCache, public SystemAbilityManagerStub {
public:
    virtual ~SystemAbilityManager();
    static sptr<SystemAbilityManager> GetInstance();

    int32_t RemoveSystemAbility(const sptr<IRemoteObject>& ability);
    std::vector<std::u16string> ListSystemAbilities(uint32_t dumpFlags) override;

    void SetDeviceName(const std::u16string &name)
    {
        deviceName_ = name;
    }

    const std::u16string& GetDeviceName() const
    {
        return deviceName_;
    }

    const sptr<DBinderService> GetDBinder() const
    {
        return dBinderService_;
    }

    sptr<IRemoteObject> GetSystemAbility(int32_t systemAbilityId) override;

    sptr<IRemoteObject> CheckSystemAbility(int32_t systemAbilityId) override;

    int32_t RemoveSystemAbility(int32_t systemAbilityId) override;

    int32_t SubscribeSystemAbility(int32_t systemAbilityId, const sptr<ISystemAbilityStatusChange>& listener) override;
    int32_t UnSubscribeSystemAbility(int32_t systemAbilityId,
        const sptr<ISystemAbilityStatusChange>& listener) override;
    void UnSubscribeSystemAbility(const sptr<IRemoteObject>& remoteObject);

    sptr<IRemoteObject> GetSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;

    sptr<IRemoteObject> CheckSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;

    int32_t AddOnDemandSystemAbilityInfo(int32_t systemAbilityId, const std::u16string& procName) override;

    sptr<IRemoteObject> CheckSystemAbility(int32_t systemAbilityId, bool& isExist) override;
    bool DoLoadOnDemandAbility(int32_t systemAbilityId, bool& isExist);

    int32_t RemoveDiedSystemAbility(int32_t systemAbilityId);

    void NotifyRemoteSaDied(const std::u16string& name);
    void NotifyRemoteDeviceOffline(const std::string& deviceId);
    int32_t AddSystemAbility(int32_t systemAbilityId, const sptr<IRemoteObject>& ability,
        const SAExtraProp& extraProp) override;
    std::string GetLocalNodeId()
    {
        return std::string();
    }
    void Init();
    void CleanFfrt();
    void SetFfrt();
    int32_t Dump(int32_t fd, const std::vector<std::u16string>& args) override;
    void AddSamgrToAbilityMap();

    int32_t AddSystemProcess(const std::u16string& procName, const sptr<IRemoteObject>& procObject) override;
    int32_t RemoveSystemProcess(const sptr<IRemoteObject>& procObject);
    int32_t GetSystemProcessInfo(int32_t systemAbilityId, SystemProcessInfo& systemProcessInfo) override;
    int32_t GetRunningSystemProcess(std::list<SystemProcessInfo>& systemProcessInfos) override;
    int32_t SubscribeSystemProcess(const sptr<ISystemProcessStatusChange>& listener) override;
    int32_t UnSubscribeSystemProcess(const sptr<ISystemProcessStatusChange>& listener) override;
    int32_t GetOnDemandReasonExtraData(int64_t extraDataId, MessageParcel& extraDataParcel) override;

    sptr<IRemoteObject> LoadSystemAbility(int32_t systemAbilityId, int32_t timeout) override
    {
        return nullptr;
    }

    int32_t LoadSystemAbility(int32_t systemAbilityId, const sptr<ISystemAbilityLoadCallback>& callback) override;
    int32_t DoLoadSystemAbility(int32_t systemAbilityId, const std::u16string& procName,
        const sptr<ISystemAbilityLoadCallback>& callback, int32_t callingPid, const OnDemandEvent& event);
    int32_t LoadSystemAbility(int32_t systemAbilityId, const std::string& deviceId,
        const sptr<ISystemAbilityLoadCallback>& callback) override;
    int32_t UnloadSystemAbility(int32_t systemAbilityId) override;
    int32_t DoUnloadSystemAbility(int32_t systemAbilityId, const std::u16string& procName, const OnDemandEvent& event);
    int32_t CancelUnloadSystemAbility(int32_t systemAbilityId) override;
    int32_t DoUnloadSystemAbility(int32_t systemAbilityId, const std::u16string& procName);
    int32_t UnloadAllIdleSystemAbility() override;
    bool IdleSystemAbility(int32_t systemAbilityId, const std::u16string& procName,
        const nlohmann::json& idleReason, int32_t& delayTime);
    bool ActiveSystemAbility(int32_t systemAbilityId, const std::u16string& procName,
        const nlohmann::json& activeReason);
    void OnAbilityCallbackDied(const sptr<IRemoteObject>& remoteObject);
    void OnRemoteCallbackDied(const sptr<IRemoteObject>& remoteObject);
    sptr<IRemoteObject> GetSystemAbilityFromRemote(int32_t systemAbilityId);
    bool LoadSystemAbilityFromRpc(const std::string& srcDeviceId, int32_t systemAbilityId,
        const sptr<ISystemAbilityLoadCallback>& callback);
    int32_t DoLoadSystemAbilityFromRpc(const std::string& srcDeviceId, int32_t systemAbilityId,
        const std::u16string& procName, const sptr<ISystemAbilityLoadCallback>& callback, const OnDemandEvent& event);
    void NotifyRpcLoadCompleted(const std::string& srcDeviceId, int32_t systemAbilityId,
        const sptr<IRemoteObject>& remoteObject);
    void StartDfxTimer();
    void DoLoadForPerf();
    void ProcessOnDemandEvent(const OnDemandEvent& event, const std::list<SaControlInfo>& saControlList);
    int32_t GetOnDemandPolicy(int32_t systemAbilityId, OnDemandPolicyType type,
        std::vector<SystemAbilityOnDemandEvent>& abilityOnDemandEvents) override;
    int32_t UpdateOnDemandPolicy(int32_t systemAbilityId, OnDemandPolicyType type,
        const std::vector<SystemAbilityOnDemandEvent>& abilityOnDemandEvents) override;
    int32_t GetOnDemandSystemAbilityIds(std::vector<int32_t>& systemAbilityIds) override;
    int32_t SendStrategy(int32_t type, std::vector<int32_t>& systemAbilityIds,
        int32_t level, std::string& action) override;
    bool CheckSaIsImmediatelyRecycle(int32_t systemAbilityId);
    bool IsDistributedSystemAbility(int32_t systemAbilityId);
    int32_t GetRunningSaExtensionInfoList(const std::string& extension,
        std::vector<SaExtensionInfo>& infoList) override;
    int32_t GetExtensionSaIds(const std::string& extension, std::vector<int32_t>& saIds) override;
    int32_t GetExtensionRunningSaList(const std::string& extension, std::vector<sptr<IRemoteObject>>& saList) override;
    int32_t GetCommonEventExtraDataIdlist(int32_t saId, std::vector<int64_t>& extraDataIdList,
        const std::string& eventName = "") override;
    sptr<IRemoteObject> GetSystemProcess(const std::u16string& procName);
    bool IsModuleUpdate(int32_t systemAbilityId);
    void RemoveWhiteCommonEvent();
private:
    enum class AbilityState {
        INIT,
        STARTING,
        STARTED,
    };

    using CallbackList = std::list<std::pair<sptr<ISystemAbilityLoadCallback>, int32_t>>;

    struct AbilityItem {
        AbilityState state = AbilityState::INIT;
        std::map<std::string, CallbackList> callbackMap; // key : networkid
        OnDemandEvent event;
    };

    SystemAbilityManager();
    std::string EventToJson(const OnDemandEvent& event);
    void DoInsertSaData(const std::u16string& name, const sptr<IRemoteObject>& ability, const SAExtraProp& extraProp);
    int32_t StartOnDemandAbility(int32_t systemAbilityId, bool& isExist);
    int32_t StartOnDemandAbilityLocked(int32_t systemAbilityId, bool& isExist);
    void RefreshListenerState(int32_t systemAbilityId);
    int32_t AddSystemAbility(const std::u16string& name, const sptr<IRemoteObject>& ability,
        const SAExtraProp& extraProp);
    int32_t FindSystemAbilityNotify(int32_t systemAbilityId, int32_t code);
    int32_t FindSystemAbilityNotify(int32_t systemAbilityId, const std::string& deviceId, int32_t code);

    void InitSaProfile();
    bool GetSaProfile(int32_t saId, SaProfile& saProfile);
    void CheckListenerNotify(int32_t systemAbilityId, const sptr<ISystemAbilityStatusChange>& listener);
    void NotifySystemAbilityChanged(int32_t systemAbilityId, const std::string& deviceId, int32_t code,
        const sptr<ISystemAbilityStatusChange>& listener);
    void UnSubscribeSystemAbilityLocked(std::list<SAListener>& listenerList,
        const sptr<IRemoteObject>& listener);

    void SendSystemAbilityAddedMsg(int32_t systemAbilityId, const sptr<IRemoteObject>& remoteObject);
    void SendSystemAbilityRemovedMsg(int32_t systemAbilityId);

    void NotifySystemAbilityLoaded(int32_t systemAbilityId, const sptr<IRemoteObject>& remoteObject);
    void NotifySystemAbilityLoaded(int32_t systemAbilityId, const sptr<IRemoteObject>& remoteObject,
        const sptr<ISystemAbilityLoadCallback>& callback);
    void NotifySystemAbilityLoadFail(int32_t systemAbilityId, const sptr<ISystemAbilityLoadCallback>& callback);
    int32_t StartingSystemProcess(const std::u16string& name, int32_t systemAbilityId, const OnDemandEvent& event);
    int32_t StartingSystemProcessLocked(const std::u16string& name, int32_t systemAbilityId,
        const OnDemandEvent& event);
    void StartOnDemandAbility(const std::u16string& name, int32_t systemAbilityId);
    void StartOnDemandAbilityLocked(const std::u16string& name, int32_t systemAbilityId);
    int32_t StartOnDemandAbilityInner(const std::u16string& name, int32_t systemAbilityId, AbilityItem& abilityItem);
    bool IsInitBootFinished();
    int32_t StartDynamicSystemProcess(const std::u16string& name, int32_t systemAbilityId, const OnDemandEvent& event);
    bool StopOnDemandAbility(const std::u16string& name, int32_t systemAbilityId, const OnDemandEvent& event);
    bool StopOnDemandAbilityInner(const std::u16string& name, int32_t systemAbilityId, const OnDemandEvent& event);
    void RemoveStartingAbilityCallback(CallbackList& callbackList, const sptr<IRemoteObject>& remoteObject);
    void RemoveStartingAbilityCallbackForDevice(AbilityItem& abilityItem, const sptr<IRemoteObject>& remoteObject);
    void RemoveStartingAbilityCallbackLocked(std::pair<sptr<ISystemAbilityLoadCallback>, int32_t>& itemPair);
    bool IsCacheCommonEvent(int32_t systemAbilityId);
    void SendCheckLoadedMsg(int32_t systemAbilityId, const std::u16string& name, const std::string& srcDeviceId,
        const sptr<ISystemAbilityLoadCallback>& callback);
    void RemoveCheckLoadedMsg(int32_t systemAbilityId);
    void SendLoadedSystemAbilityMsg(int32_t systemAbilityId, const sptr<IRemoteObject>& remoteObject,
        const sptr<ISystemAbilityLoadCallback>& callback);
    void DoLoadRemoteSystemAbility(int32_t systemAbilityId, int32_t callingPid,
        int32_t callingUid, const std::string& deviceId, const sptr<ISystemAbilityLoadCallback>& callback);
    sptr<DBinderServiceStub> DoMakeRemoteBinder(int32_t systemAbilityId, int32_t callingPid, int32_t callingUid,
        const std::string& deviceId);
    void RemoveRemoteCallbackLocked(std::list<sptr<ISystemAbilityLoadCallback>>& callbacks,
        const sptr<IRemoteObject>& remoteObject);
    void CleanCallbackForLoadFailed(int32_t systemAbilityId, const std::u16string& name,
        const std::string& srcDeviceId, const sptr<ISystemAbilityLoadCallback>& callback);
    int32_t UpdateSaFreMap(int32_t uid, int32_t saId);
    void ReportGetSAPeriodically();
    void OndemandLoad();
    void OndemandLoadForPerf();
    std::list<int32_t> GetAllOndemandSa();
    void SystemAbilityInvalidateCache(int32_t systemAbilityId);
#ifdef SUPPORT_DEVICE_MANAGER
    void DeviceIdToNetworkId(std::string& networkId);
#endif
    bool IpcStatSamgrProc(int32_t fd, int32_t cmd);
    void IpcDumpAllProcess(int32_t fd, int32_t cmd);
    void IpcDumpSamgrProcess(int32_t fd, int32_t cmd);
    void IpcDumpSingleProcess(int32_t fd, int32_t cmd, const std::string processName);
    int32_t IpcDumpProc(int32_t fd, const std::vector<std::string>& args);

    std::u16string deviceName_;
    static sptr<SystemAbilityManager> instance;
    static std::mutex instanceLock;
    sptr<IRemoteObject::DeathRecipient> abilityDeath_;
    sptr<IRemoteObject::DeathRecipient> systemProcessDeath_;
    sptr<IRemoteObject::DeathRecipient> abilityStatusDeath_;
    sptr<IRemoteObject::DeathRecipient> abilityCallbackDeath_;
    sptr<IRemoteObject::DeathRecipient> remoteCallbackDeath_;
    sptr<DBinderService> dBinderService_;
    sptr<DeviceStatusCollectManager> collectManager_;
    std::shared_ptr<RpcSystemAbilityCallback> rpcCallbackImp_;

    // must hold abilityMapLock_ never access other locks
    std::shared_mutex abilityMapLock_;
    std::map<int32_t, SAInfo> abilityMap_;

    // maybe hold listenerMapLock_ and then access onDemandLock_
    std::mutex listenerMapLock_;
    std::map<int32_t, std::list<SAListener>> listenerMap_;
    std::map<int32_t, int32_t> subscribeCountMap_;

    std::mutex onDemandLock_;
    std::map<int32_t, std::u16string> onDemandAbilityMap_;
    std::map<int32_t, AbilityItem> startingAbilityMap_;
    std::mutex systemProcessMapLock_;
    std::map<std::u16string, sptr<IRemoteObject>> systemProcessMap_;
    std::mutex startingProcessMapLock_;
    std::map<std::u16string, int64_t> startingProcessMap_;
    std::map<int32_t, int32_t> callbackCountMap_;

    std::shared_ptr<FFRTHandler> workHandler_;

    std::map<int32_t, SaProfile> saProfileMap_;
    std::set<int32_t> onDemandSaIdsSet_;
    std::mutex saProfileMapLock_;
    std::mutex loadRemoteLock_;
    std::map<std::string, std::list<sptr<ISystemAbilityLoadCallback>>> remoteCallbacks_; // key : said_deviceId

    std::mutex saFrequencyLock_;
    std::map<uint64_t, int32_t> saFrequencyMap_; // {pid_said, count}

    std::unique_ptr<Utils::Timer> reportEventTimer_;
    std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler_;
};
} // namespace OHOS

#endif // !defined(SERVICES_SAMGR_NATIVE_INCLUDE_SYSTEM_ABILITY_MANAGER_H)
