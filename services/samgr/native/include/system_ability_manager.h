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

#ifndef SERVICES_SAMGR_NATIVE_INCLUDE_SYSTEM_ABILITY_MANAGER_H
#define SERVICES_SAMGR_NATIVE_INCLUDE_SYSTEM_ABILITY_MANAGER_H

#include <map>
#include <set>
#include <string>
#include <utility>

#include "base_system_ability_manager.h"
#include "dbinder_service.h"
#include "dbinder_service_stub.h"
#include "rpc_callback_imp.h"
#include "system_ability_manager_stub.h"

namespace OHOS {

enum {
    UUID = 0,
    NODE_ID,
    UNKNOWN,
};

class SystemAbilityManager : public BaseSystemAbilityManager, public SystemAbilityManagerStub {
public:
    ~SystemAbilityManager() override = default;

    using BaseSystemAbilityManager::UnSubscribeSystemAbility;
    using BaseSystemAbilityManager::RemoveSystemAbility;

    static sptr<SystemAbilityManager> GetInstance()
    {
        std::lock_guard<samgr::mutex> autoLock(instanceLock);
        if (instance == nullptr) {
            instance = new SystemAbilityManager;
        }
        return instance;
    }

    std::vector<std::u16string> ListSystemAbilities(uint32_t dumpFlags) override;

    sptr<IRemoteObject> GetSystemAbility(int32_t systemAbilityId) override;

    sptr<IRemoteObject> CheckSystemAbility(int32_t systemAbilityId) override;

    int32_t RemoveSystemAbility(int32_t systemAbilityId) override;

    int32_t SubscribeSystemAbility(int32_t systemAbilityId, const sptr<ISystemAbilityStatusChange>& listener) override;
    int32_t UnSubscribeSystemAbility(int32_t systemAbilityId,
        const sptr<ISystemAbilityStatusChange>& listener) override;

    sptr<IRemoteObject> GetSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;

    sptr<IRemoteObject> CheckSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;

    int32_t AddOnDemandSystemAbilityInfo(int32_t systemAbilityId, const std::u16string& procName) override;

    sptr<IRemoteObject> CheckSystemAbility(int32_t systemAbilityId, bool& isExist) override;

    int32_t AddSystemAbility(int32_t systemAbilityId, const sptr<IRemoteObject>& ability,
        const SAExtraProp& extraProp) override;
    void StartDfxTimer();
    void DoLoadForPerf();
    void Init();
    int32_t Dump(int32_t fd, const std::vector<std::u16string>& args) override;
    void AddSamgrToAbilityMap();

    int32_t AddSystemProcess(const std::u16string& procName, const sptr<IRemoteObject>& procObject) override;
    int32_t GetSystemProcessInfo(int32_t systemAbilityId, SystemProcessInfo& systemProcessInfo) override;
    int32_t GetRunningSystemProcess(std::list<SystemProcessInfo>& systemProcessInfos) override;
    int32_t SubscribeSystemProcess(const sptr<ISystemProcessStatusChange>& listener) override;
    int32_t UnSubscribeSystemProcess(const sptr<ISystemProcessStatusChange>& listener) override;
    int32_t SubscribeLowMemSystemProcess(const sptr<ISystemProcessStatusChange>& listener) override;
    int32_t UnSubscribeLowMemSystemProcess(const sptr<ISystemProcessStatusChange>& listener) override;
    int32_t GetOnDemandReasonExtraData(int64_t extraDataId, MessageParcel& extraDataParcel) override;

    sptr<IRemoteObject> LoadSystemAbility(int32_t systemAbilityId, int32_t timeout) override
    {
        return nullptr;
    }

    int32_t LoadSystemAbility(int32_t systemAbilityId, const sptr<ISystemAbilityLoadCallback>& callback) override;
    int32_t LoadSystemAbility(int32_t systemAbilityId, const std::string& deviceId,
        const sptr<ISystemAbilityLoadCallback>& callback) override;
    int32_t UnloadSystemAbility(int32_t systemAbilityId) override;
    int32_t CancelUnloadSystemAbility(int32_t systemAbilityId) override;
    int32_t UnloadAllIdleSystemAbility() override;
    int32_t UnloadProcess(const std::vector<std::u16string>& processList) override;
    int32_t GetLruIdleSystemAbilityProc(std::vector<IdleProcessInfo>& processInfos) override;
    int32_t OnStartSystemAbilityFail(int32_t systemAbilityId, int32_t errCode) override;
    sptr<IRemoteObject> GetSystemAbilityFromRemote(int32_t systemAbilityId);
    bool LoadSystemAbilityFromRpc(const std::string& srcDeviceId, int32_t systemAbilityId,
        const sptr<ISystemAbilityLoadCallback>& callback);
    int32_t DoLoadSystemAbilityFromRpc(const std::string& srcDeviceId, int32_t systemAbilityId,
        const std::u16string& procName, const sptr<ISystemAbilityLoadCallback>& callback, const OnDemandEvent& event);
    void NotifyRpcLoadCompleted(const std::string& srcDeviceId, int32_t systemAbilityId,
        const sptr<IRemoteObject>& remoteObject);
    int32_t GetOnDemandPolicy(int32_t systemAbilityId, OnDemandPolicyType type,
        std::vector<SystemAbilityOnDemandEvent>& abilityOnDemandEvents) override;
    int32_t UpdateOnDemandPolicy(int32_t systemAbilityId, OnDemandPolicyType type,
        const std::vector<SystemAbilityOnDemandEvent>& abilityOnDemandEvents) override;
    int32_t GetOnDemandSystemAbilityIds(std::vector<int32_t>& systemAbilityIds) override;
    int32_t SubscribeSystemAbilityInImage(int32_t systemAbilityId,
        const sptr<ISystemAbilityStatusChange>& listener) override;
    int32_t SendStrategy(int32_t type, std::vector<int32_t>& systemAbilityIds,
        int32_t level, std::string& action) override;
    int32_t SetSamgrIpcPrior(bool enable) override;
#ifdef SUPPORT_MULTI_INSTANCE
    int32_t OnUserStateChanged(int32_t userId, SamgrUserState userState) override;
#endif
    int32_t GetRunningSaExtensionInfoList(const std::string& extension,
        std::vector<SaExtensionInfo>& infoList) override;
    int32_t GetExtensionSaIds(const std::string& extension, std::vector<int32_t>& saIds) override;
    int32_t GetExtensionRunningSaList(const std::string& extension, std::vector<sptr<IRemoteObject>>& saList) override;
    int32_t GetCommonEventExtraDataIdlist(int32_t saId, std::vector<int64_t>& extraDataIdList,
        const std::string& eventName = "") override;
    sptr<IRemoteObject> GetLocalAbilityManagerProxy(int32_t systemAbilityId) override
    {
        CommonSaProfile saProfile;
        if (!GetSaProfile(systemAbilityId, saProfile)) {
            HILOGD("SA:%{public}d no profile!", systemAbilityId);
            return nullptr;
        }
        return GetSystemProcess(saProfile.process);
    }
    void RemoveWhiteCommonEvent()
    {
        if (collectManager_ != nullptr) {
            collectManager_->RemoveWhiteCommonEvent();
        }
    }
#ifdef SAMGR_ENABLE_DELAY_DBINDER
    void InitDbinderService();
#endif
private:
    using BaseSystemAbilityManager::AbilityState;
    using BaseSystemAbilityManager::CallbackList;
    using BaseSystemAbilityManager::AbilityItem;
    using BaseSystemAbilityManager::StartingProcessInfo;

    SystemAbilityManager()
    {
#ifndef SAMGR_ENABLE_DELAY_DBINDER
        dBinderService_ = DBinderService::GetInstance();
#endif
    }
    void DoLoadRemoteSystemAbility(int32_t systemAbilityId, int32_t callingPid,
        int32_t callingUid, const std::string& deviceId, const sptr<ISystemAbilityLoadCallback>& callback);
    sptr<DBinderServiceStub> DoMakeRemoteBinder(int32_t systemAbilityId, int32_t callingPid, int32_t callingUid,
        const std::string& deviceId);
    void ReportGetSAPeriodically();
    void OndemandLoad();
    void OndemandLoadForPerf();
    std::list<int32_t> GetAllOndemandSa();
    bool IpcStatSamgrProc(int32_t fd, int32_t cmd);
    void IpcDumpAllProcess(int32_t fd, int32_t cmd);
    void IpcDumpSamgrProcess(int32_t fd, int32_t cmd);
    void IpcDumpSingleProcess(int32_t fd, int32_t cmd, const std::string processName);
    int32_t IpcDumpProc(int32_t fd, const std::vector<std::string>& args);
    void RegisterDistribute(int32_t said, bool isDistributed);
    void OnSystemAbilityRegistered(int32_t systemAbilityId, bool isDistributed) override;
    void FlushResetPriorTask();

    static sptr<SystemAbilityManager> instance;
    static samgr::mutex instanceLock;
    sptr<DBinderService> dBinderService_;
    std::shared_ptr<RpcSystemAbilityCallback> rpcCallbackImp_;

#ifdef SAMGR_ENABLE_DELAY_DBINDER
    samgr::shared_mutex dBinderServiceLock_;
    std::list<int32_t> distributedSaList_;
    bool isDbinderServiceInit_ = false;
#endif

#ifdef SUPPORT_MULTI_INSTANCE
    samgr::mutex userStateLock_;
    std::map<int32_t, SamgrUserState> userStateMap_;
#endif
    std::mutex priorRefCntLock_;
    int32_t priorRefCnt_ = 0;
};
} // namespace OHOS

#endif // !defined(SERVICES_SAMGR_NATIVE_INCLUDE_SYSTEM_ABILITY_MANAGER_H)
