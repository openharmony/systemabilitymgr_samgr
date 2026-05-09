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

#include "system_ability_manager.h"

#include <cinttypes>
#include <thread>
#include <unistd.h>

#include "accesstoken_kit.h"
#include "datetime_ex.h"
#include "errors.h"
#include "file_ex.h"
#include "hisysevent_adapter.h"
#include "hitrace_meter.h"
#include "ipc_skeleton.h"
#include "memory_guard.h"
#include "parameter.h"
#include "parameters.h"
#include "sam_log.h"
#include "samgr_xcollie.h"
#include "string_ex.h"
#include "system_ability_manager_dumper.h"
#include "system_ability_manager_util.h"
#include "tools.h"

using namespace std;

namespace OHOS {
namespace {
constexpr const char* ONDEMAND_PERF_PARAM = "persist.samgr.perf.ondemand";
constexpr const char* ONDEMAND_WORKER = "OndemandLoader";
constexpr const char* ARGS_FFRT_PARAM = "--ffrt";
constexpr const char* ARGS_LISTENER_PARAM = "--listener";
constexpr const char* IPC_STAT_DUMP_PREFIX = "--ipc";
constexpr int32_t SOFTBUS_SERVER_SA_ID = 4700;
constexpr int32_t FIRST_DUMP_INDEX = 0;
constexpr const char* RESET_IPC_PRIOR = "resetIpcPrior";
constexpr int32_t RESET_IPC_PRIOR_TIMEOUT = 5 * 1000;
constexpr int64_t ONDEMAND_PERF_DELAY_TIME = 60 * 1000; // ms
constexpr uint32_t REPORT_GET_SA_INTERVAL = 24 * 60 * 60 * 1000; // ms and is one day
constexpr int32_t SHFIT_BIT = 32;
}

samgr::mutex SystemAbilityManager::instanceLock;
sptr<SystemAbilityManager> SystemAbilityManager::instance;

void SystemAbilityManager::OnSystemAbilityRegistered(int32_t systemAbilityId, bool isDistributed)
{
    RegisterDistribute(systemAbilityId, isDistributed);
}

void SystemAbilityManager::RegisterDistribute(int32_t systemAbilityId, bool isDistributed)
{
#ifdef SAMGR_ENABLE_DELAY_DBINDER
    if (isDistributed) {
        std::shared_lock<samgr::shared_mutex> readLock(dBinderServiceLock_);
        if (dBinderService_ != nullptr) {
            u16string strName = Str8ToStr16(to_string(systemAbilityId));
            dBinderService_->RegisterRemoteProxy(strName, systemAbilityId);
            HILOGI("AddSystemAbility RegisterRemoteProxy, SA:%{public}d", systemAbilityId);
        } else {
            if (!isDbinderServiceInit_) {
                distributedSaList_.push_back(systemAbilityId);
            }
        }
    }
    if (systemAbilityId == SOFTBUS_SERVER_SA_ID) {
        std::shared_lock<samgr::shared_mutex> readLock(dBinderServiceLock_);
        if (dBinderService_ != nullptr && rpcCallbackImp_ != nullptr) {
            bool ret = dBinderService_->StartDBinderService(rpcCallbackImp_);
            HILOGI("start result is %{public}s", ret ? "succeed" : "fail");
        }
    }
#else
    u16string strName = Str8ToStr16(to_string(systemAbilityId));
    if (isDistributed && dBinderService_ != nullptr) {
        dBinderService_->RegisterRemoteProxy(strName, systemAbilityId);
        HILOGI("AddSystemAbility RegisterRemoteProxy, SA:%{public}d", systemAbilityId);
    }
    if (systemAbilityId == SOFTBUS_SERVER_SA_ID) {
        if (dBinderService_ != nullptr && rpcCallbackImp_ != nullptr) {
            bool ret = dBinderService_->StartDBinderService(rpcCallbackImp_);
            HILOGI("start result is %{public}s", ret? "succeed" : "fail");
        }
    }
#endif
}

#ifdef SAMGR_ENABLE_DELAY_DBINDER
void SystemAbilityManager::InitDbinderService()
{
    std::unique_lock<samgr::shared_mutex> writeLock(dBinderServiceLock_);
    if (!isDbinderServiceInit_) {
        dBinderService_ = DBinderService::GetInstance();
        rpcCallbackImp_ = make_shared<RpcCallbackImp>();
        if (dBinderService_ != nullptr) {
            for (auto said : distributedSaList_) {
                u16string strName = Str8ToStr16(to_string(said));
                dBinderService_->RegisterRemoteProxy(strName, said);
                HILOGI("AddSystemAbility RegisterRemoteProxy, SA:%{public}d", said);
            }
            std::list<int32_t>().swap(distributedSaList_);
        }
        isDbinderServiceInit_ = true;
    }
    if (CheckSystemAbility(SOFTBUS_SERVER_SA_ID) != nullptr) {
        if (dBinderService_ != nullptr && rpcCallbackImp_ != nullptr) {
            bool ret = dBinderService_->StartDBinderService(rpcCallbackImp_);
            HILOGI("start result is %{public}s", ret ? "succeed" : "fail");
        }
    }
}
#endif

void SystemAbilityManager::Init()
{
    BaseSystemAbilityManager::Init();
#ifndef SAMGR_ENABLE_DELAY_DBINDER
    rpcCallbackImp_ = make_shared<RpcCallbackImp>();
#endif
    OndemandLoadForPerf();
    SamgrUtil::InvalidateSACache();
    SamgrUtil::RegisterSAListener();
}

bool SystemAbilityManager::IpcStatSamgrProc(int32_t fd, int32_t cmd)
{
    bool ret = false;
    std::string result;

    HILOGI("IpcStatSamgrProc:fd=%{public}d cmd=%{public}d", fd, cmd);
    if (cmd < IPC_STAT_CMD_START || cmd >= IPC_STAT_CMD_MAX) {
        HILOGW("para invalid, fd=%{public}d cmd=%{public}d", fd, cmd);
        return false;
    }

    switch (cmd) {
        case IPC_STAT_CMD_START: {
            ret = SystemAbilityManagerDumper::StartSamgrIpcStatistics(result);
            break;
        }
        case IPC_STAT_CMD_STOP: {
            ret = SystemAbilityManagerDumper::StopSamgrIpcStatistics(result);
            break;
        }
        case IPC_STAT_CMD_GET: {
            ret = SystemAbilityManagerDumper::GetSamgrIpcStatistics(result);
            break;
        }
        default:
            return false;
    }

    if (!SaveStringToFd(fd, result)) {
        HILOGW("save to fd failed");
        return false;
    }
    return ret;
}

void SystemAbilityManager::IpcDumpAllProcess(int32_t fd, int32_t cmd)
{
    lock_guard<samgr::mutex> autoLock(systemProcessMapLock_);
    for (auto iter = systemProcessMap_.begin(); iter != systemProcessMap_.end(); iter++) {
        sptr<ILocalAbilityManager> obj = iface_cast<ILocalAbilityManager>(iter->second);
        if (obj != nullptr) {
            obj->IpcStatCmdProc(fd, cmd);
        }
    }
}

void SystemAbilityManager::IpcDumpSamgrProcess(int32_t fd, int32_t cmd)
{
    if (!IpcStatSamgrProc(fd, cmd)) {
        HILOGE("IpcStatSamgrProc failed");
    }
}

void SystemAbilityManager::IpcDumpSingleProcess(int32_t fd, int32_t cmd, const std::string processName)
{
    sptr<ILocalAbilityManager> obj = iface_cast<ILocalAbilityManager>(GetSystemProcess(Str8ToStr16(processName)));
    if (obj != nullptr) {
        obj->IpcStatCmdProc(fd, cmd);
    }
}

int32_t SystemAbilityManager::IpcDumpProc(int32_t fd, const std::vector<std::string>& args)
{
    int32_t cmd;
    if (!SystemAbilityManagerDumper::IpcDumpCmdParser(cmd, args)) {
        HILOGE("IpcDumpCmdParser failed");
        return ERR_INVALID_VALUE;
    }

    HILOGI("IpcDumpProc:fd=%{public}d cmd=%{public}d request", fd, cmd);

    const std::string processName = args[IPC_STAT_PROCESS_INDEX];
    if (SystemAbilityManagerDumper::IpcDumpIsAllProcess(processName)) {
        IpcDumpAllProcess(fd, cmd);
        IpcDumpSamgrProcess(fd, cmd);
    } else if (SystemAbilityManagerDumper::IpcDumpIsSamgr(processName)) {
        IpcDumpSamgrProcess(fd, cmd);
    } else {
        IpcDumpSingleProcess(fd, cmd, processName);
    }
    return ERR_OK;
}

int32_t SystemAbilityManager::Dump(int32_t fd, const std::vector<std::u16string>& args)
{
    std::vector<std::string> argsWithStr8;
    for (const auto& arg : args) {
        argsWithStr8.emplace_back(Str16ToStr8(arg));
    }
    if ((argsWithStr8.size() > 0) && (argsWithStr8[FIRST_DUMP_INDEX] == ARGS_FFRT_PARAM)) {
        return SystemAbilityManagerDumper::FfrtDumpProc(abilityStateScheduler_, fd, argsWithStr8);
    }
    if ((argsWithStr8.size() > 0) && (argsWithStr8[FIRST_DUMP_INDEX] == ARGS_LISTENER_PARAM)) {
        std::map<int32_t, std::list<SAListener>> dumpListeners;
        {
            lock_guard<samgr::mutex> autoLock(listenerMapLock_);
            dumpListeners = listenerMap_;
        }
        return SystemAbilityManagerDumper::ListenerDumpProc(dumpListeners, fd, argsWithStr8);
    }
    if ((argsWithStr8.size() > 0) && (argsWithStr8[IPC_STAT_PREFIX_INDEX] == IPC_STAT_DUMP_PREFIX)) {
        return IpcDumpProc(fd, argsWithStr8);
    } else {
        std::string result;
        SystemAbilityManagerDumper::Dump(abilityStateScheduler_, argsWithStr8, result);
        if (!SaveStringToFd(fd, result)) {
            HILOGE("save to fd failed");
            return ERR_INVALID_VALUE;
        }
    }
    return ERR_OK;
}

void SystemAbilityManager::AddSamgrToAbilityMap()
{
    unique_lock<samgr::shared_mutex> writeLock(abilityMapLock_);
    int32_t systemAbilityId = 0;
    SAInfo saInfo;
    saInfo.remoteObj = this;
    saInfo.isDistributed = false;
    abilityMap_[systemAbilityId] = std::move(saInfo);
    if (abilityStateScheduler_ != nullptr) {
        abilityStateScheduler_->InitSamgrProcessContext();
    }
    HILOGD("samgr inserted");
}

void SystemAbilityManager::StartDfxTimer()
{
    reportEventTimer_->Setup();
    uint32_t timerId = reportEventTimer_->Register([this] {this->ReportGetSAPeriodically();},
        REPORT_GET_SA_INTERVAL);
    HILOGI("StartDfxTimer timerId : %{public}u!", timerId);
}

std::list<int32_t> SystemAbilityManager::GetAllOndemandSa()
{
    std::list<int32_t> ondemandSaids;
    {
        lock_guard<samgr::mutex> autoLock(saProfileMapLock_);
        for (const auto& [said, value] : saProfileMap_) {
            shared_lock<samgr::shared_mutex> readLock(abilityMapLock_);
            auto iter = abilityMap_.find(said);
            if (iter == abilityMap_.end()) {
                ondemandSaids.emplace_back(said);
            }
        }
    }
    return ondemandSaids;
}

void SystemAbilityManager::DoLoadForPerf()
{
    bool value = system::GetBoolParameter(ONDEMAND_PERF_PARAM, false);
    if (value) {
        std::list<int32_t> saids = GetAllOndemandSa();
        HILOGD("DoLoadForPerf ondemand size : %{public}zu.", saids.size());
        sptr<ISystemAbilityLoadCallback> callback(new SystemAbilityLoadCallbackStub());
        for (auto said : saids) {
            LoadSystemAbility(said, callback);
        }
    }
}

void SystemAbilityManager::OndemandLoadForPerf()
{
    if (workHandler_ == nullptr) {
        HILOGE("LoadForPerf workHandler_ not init!");
        return;
    }
    auto callback = [this] () {
        OndemandLoad();
    };
    workHandler_->PostTask(callback, ONDEMAND_PERF_DELAY_TIME);
}

void SystemAbilityManager::OndemandLoad()
{
    auto bootEventCallback = [](const char *key, const char *value, void *context) {
        int64_t begin = GetTickCount();
        SystemAbilityManager::GetInstance()->DoLoadForPerf();
        HILOGI("DoLoadForPerf spend %{public}" PRId64 "ms", GetTickCount() - begin);
    };

    int ret = WatchParameter(ONDEMAND_PERF_PARAM, bootEventCallback, nullptr);
    HILOGD("OndemandLoad ret %{public}d", ret);
}

sptr<IRemoteObject> SystemAbilityManager::GetSystemAbility(int32_t systemAbilityId)
{
    return CheckSystemAbility(systemAbilityId);
}

sptr<IRemoteObject> SystemAbilityManager::GetSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    return CheckSystemAbility(systemAbilityId, deviceId);
}

sptr<IRemoteObject> SystemAbilityManager::GetSystemAbilityFromRemote(int32_t systemAbilityId)
{
    HILOGD("%{public}s called, SA:%{public}d", __func__, systemAbilityId);
    if (!BaseSystemAbilityManager::CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("GetSystemAbilityFromRemote invalid!");
        return nullptr;
    }

    shared_lock<samgr::shared_mutex> readLock(abilityMapLock_);
    auto iter = abilityMap_.find(systemAbilityId);
    if (iter == abilityMap_.end()) {
        HILOGI("GetSystemAbilityFromRemote not found SA %{public}d.", systemAbilityId);
        return nullptr;
    }
    if (!(iter->second.isDistributed)) {
        HILOGW("GetSystemAbilityFromRemote SA:%{public}d not distributed", systemAbilityId);
        return nullptr;
    }
    HILOGI("GetSystemAbilityFromRemote found SA:%{public}d.", systemAbilityId);
    return iter->second.remoteObj;
}

sptr<IRemoteObject> SystemAbilityManager::CheckSystemAbility(int32_t systemAbilityId)
{
    return BaseSystemAbilityManager::CheckSystemAbility(systemAbilityId);
}

sptr<IRemoteObject> SystemAbilityManager::CheckSystemAbility(int32_t systemAbilityId,
    const std::string& deviceId)
{
    if (!IsDistributedSystemAbility(systemAbilityId)) {
        HILOGE("CheckSystemAbilityFromRpc SA:%{public}d not distributed!", systemAbilityId);
        return nullptr;
    }
    return DoMakeRemoteBinder(systemAbilityId, IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid(), deviceId);
}

int32_t SystemAbilityManager::AddSystemAbility(int32_t systemAbilityId, const sptr<IRemoteObject>& ability,
    const SAExtraProp& extraProp)
{
    return BaseSystemAbilityManager::AddSystemAbility(systemAbilityId, ability, extraProp);
}

int32_t SystemAbilityManager::RemoveSystemAbility(int32_t systemAbilityId)
{
    return BaseSystemAbilityManager::RemoveSystemAbility(systemAbilityId);
}

int32_t SystemAbilityManager::SubscribeSystemAbility(int32_t systemAbilityId,
    const sptr<ISystemAbilityStatusChange>& listener)
{
    return BaseSystemAbilityManager::SubscribeSystemAbility(systemAbilityId, listener);
}

int32_t SystemAbilityManager::UnSubscribeSystemAbility(int32_t systemAbilityId,
    const sptr<ISystemAbilityStatusChange>& listener)
{
    return BaseSystemAbilityManager::UnSubscribeSystemAbility(systemAbilityId, listener);
}

sptr<IRemoteObject> SystemAbilityManager::CheckSystemAbility(int32_t systemAbilityId, bool& isExist)
{
    return BaseSystemAbilityManager::CheckSystemAbility(systemAbilityId, isExist);
}

int32_t SystemAbilityManager::AddOnDemandSystemAbilityInfo(int32_t systemAbilityId,
    const std::u16string& procName)
{
    return BaseSystemAbilityManager::AddOnDemandSystemAbilityInfo(systemAbilityId, procName);
}

int32_t SystemAbilityManager::SubscribeSystemAbilityInImage(int32_t systemAbilityId,
    const sptr<ISystemAbilityStatusChange>& listener)
{
    return SubscribeSystemAbility(systemAbilityId, listener);
}

std::vector<std::u16string> SystemAbilityManager::ListSystemAbilities(uint32_t dumpFlags)
{
    return BaseSystemAbilityManager::ListSystemAbilities(dumpFlags);
}

int32_t SystemAbilityManager::AddSystemProcess(const std::u16string& procName,
    const sptr<IRemoteObject>& procObject)
{
    return BaseSystemAbilityManager::AddSystemProcess(procName, procObject);
}

int32_t SystemAbilityManager::GetSystemProcessInfo(int32_t systemAbilityId, SystemProcessInfo& systemProcessInfo)
{
    return BaseSystemAbilityManager::GetSystemProcessInfo(systemAbilityId, systemProcessInfo);
}

int32_t SystemAbilityManager::GetRunningSystemProcess(std::list<SystemProcessInfo>& systemProcessInfos)
{
    return BaseSystemAbilityManager::GetRunningSystemProcess(systemProcessInfos);
}

int32_t SystemAbilityManager::SubscribeSystemProcess(const sptr<ISystemProcessStatusChange>& listener)
{
    return BaseSystemAbilityManager::SubscribeSystemProcess(listener);
}

int32_t SystemAbilityManager::UnSubscribeSystemProcess(const sptr<ISystemProcessStatusChange>& listener)
{
    return BaseSystemAbilityManager::UnSubscribeSystemProcess(listener);
}

int32_t SystemAbilityManager::SubscribeLowMemSystemProcess(const sptr<ISystemProcessStatusChange>& listener)
{
    return BaseSystemAbilityManager::SubscribeLowMemSystemProcess(listener);
}

int32_t SystemAbilityManager::UnSubscribeLowMemSystemProcess(const sptr<ISystemProcessStatusChange>& listener)
{
    return BaseSystemAbilityManager::UnSubscribeLowMemSystemProcess(listener);
}

int32_t SystemAbilityManager::GetOnDemandReasonExtraData(int64_t extraDataId, MessageParcel& extraDataParcel)
{
    return BaseSystemAbilityManager::GetOnDemandReasonExtraData(extraDataId, extraDataParcel);
}

int32_t SystemAbilityManager::LoadSystemAbility(int32_t systemAbilityId,
    const sptr<ISystemAbilityLoadCallback>& callback)
{
    return BaseSystemAbilityManager::LoadSystemAbility(systemAbilityId, callback);
}

int32_t SystemAbilityManager::LoadSystemAbility(int32_t systemAbilityId, const std::string& deviceId,
    const sptr<ISystemAbilityLoadCallback>& callback)
{
    std::string key = ToString(systemAbilityId) + "_" + deviceId;
    {
        lock_guard<samgr::mutex> autoLock(loadRemoteLock_);
        auto& callbacks = remoteCallbacks_[key];
        auto iter = std::find_if(callbacks.begin(), callbacks.end(), [callback](auto itemCallback) {
            return callback->AsObject() == itemCallback->AsObject();
        });
        if (iter != callbacks.end()) {
            HILOGI("LoadSystemAbility already existed callback object SA:%{public}d", systemAbilityId);
            return ERR_OK;
        }
        if (remoteCallbackDeath_ != nullptr) {
            bool ret = callback->AsObject()->AddDeathRecipient(remoteCallbackDeath_);
            HILOGI("LoadSystemAbility SA:%{public}d AddDeathRecipient %{public}s",
                systemAbilityId, ret ? "succeed" : "failed");
        }
        callbacks.emplace_back(callback);
    }
    auto callingPid = IPCSkeleton::GetCallingPid();
    auto callingUid = IPCSkeleton::GetCallingUid();
    auto task = [this, systemAbilityId, callingPid, callingUid, deviceId, callback] {
        this->DoLoadRemoteSystemAbility(systemAbilityId, callingPid, callingUid, deviceId, callback);
    };
    std::thread thread(task);
    thread.detach();
    return ERR_OK;
}

void SystemAbilityManager::DoLoadRemoteSystemAbility(int32_t systemAbilityId, int32_t callingPid,
    int32_t callingUid, const std::string& deviceId, const sptr<ISystemAbilityLoadCallback>& callback)
{
    Samgr::MemoryGuard cacheGuard;
    pthread_setname_np(pthread_self(), ONDEMAND_WORKER);
    sptr<DBinderServiceStub> remoteBinder = DoMakeRemoteBinder(systemAbilityId, callingPid, callingUid, deviceId);

    if (callback == nullptr) {
        HILOGI("DoLoadRemoteSystemAbility callback is null, SA:%{public}d", systemAbilityId);
        return;
    }
    callback->OnLoadSACompleteForRemote(deviceId, systemAbilityId, remoteBinder);
    std::string key = ToString(systemAbilityId) + "_" + deviceId;
    {
        lock_guard<samgr::mutex> autoLock(loadRemoteLock_);
        if (remoteCallbackDeath_ != nullptr) {
            callback->AsObject()->RemoveDeathRecipient(remoteCallbackDeath_);
        }
        auto& callbacks = remoteCallbacks_[key];
        callbacks.remove(callback);
        if (callbacks.empty()) {
            remoteCallbacks_.erase(key);
        }
    }
}

bool SystemAbilityManager::LoadSystemAbilityFromRpc(const std::string& srcDeviceId, int32_t systemAbilityId,
    const sptr<ISystemAbilityLoadCallback>& callback)
{
    if (!BaseSystemAbilityManager::CheckInputSysAbilityId(systemAbilityId) || callback == nullptr) {
        HILOGW("LoadSystemAbility said or callback invalid!");
        return false;
    }
    if (!IsDistributedSystemAbility(systemAbilityId)) {
        HILOGE("LoadSystemAbilityFromRpc SA:%{public}d not distributed!", systemAbilityId);
        return false;
    }
    OnDemandEvent onDemandEvent = {INTERFACE_CALL, "loadFromRpc"};
    LoadRequestInfo loadRequestInfo = {srcDeviceId, callback, systemAbilityId, -1, onDemandEvent};
    if (abilityStateScheduler_ == nullptr) {
        HILOGE("abilityStateScheduler is nullptr");
        return false;
    }
    return abilityStateScheduler_->HandleLoadAbilityEvent(loadRequestInfo) == ERR_OK;
}

int32_t SystemAbilityManager::DoLoadSystemAbilityFromRpc(const std::string& srcDeviceId, int32_t systemAbilityId,
    const std::u16string& procName, const sptr<ISystemAbilityLoadCallback>& callback, const OnDemandEvent& event)
{
    sptr<IRemoteObject> targetObject = CheckSystemAbility(systemAbilityId);
    if (targetObject != nullptr) {
        SendLoadedSystemAbilityMsg(systemAbilityId, targetObject, callback);
        return ERR_OK;
    }
    {
        lock_guard<samgr::mutex> autoLock(onDemandLock_);
        auto& abilityItem = startingAbilityMap_[systemAbilityId];
        abilityItem.callbackMap[srcDeviceId].emplace_back(callback, 0);
        StartingSystemProcessLocked(procName, systemAbilityId, event);
    }
    SendCheckLoadedMsg(systemAbilityId, procName, srcDeviceId, callback);
    return ERR_OK;
}

sptr<DBinderServiceStub> SystemAbilityManager::DoMakeRemoteBinder(int32_t systemAbilityId, int32_t callingPid,
    int32_t callingUid, const std::string& deviceId)
{
    HILOGI("MakeRemoteBinder begin, SA:%{public}d", systemAbilityId);
    std::string networkId = deviceId;
#ifdef SUPPORT_DEVICE_MANAGER
    SamgrUtil::DeviceIdToNetworkId(networkId);
#endif
    sptr<DBinderServiceStub> remoteBinder = nullptr;
#ifdef SAMGR_ENABLE_DELAY_DBINDER
    std::shared_lock<samgr::shared_mutex> readLock(dBinderServiceLock_);
#endif
    if (dBinderService_ != nullptr) {
        string strName = to_string(systemAbilityId);
        {
            SamgrXCollie samgrXCollie("samgr--MakeRemoteBinder_" + strName);
            remoteBinder = dBinderService_->MakeRemoteBinder(Str8ToStr16(strName),
                networkId, systemAbilityId, callingPid, callingUid);
        }
    }
    HILOGI("MakeRemoteBinder end, result %{public}s, SA:%{public}d, networkId : %{public}s",
        remoteBinder == nullptr ? " failed" : "succeed", systemAbilityId, AnonymizeDeviceId(networkId).c_str());
    return remoteBinder;
}

void SystemAbilityManager::NotifyRpcLoadCompleted(const std::string& srcDeviceId, int32_t systemAbilityId,
    const sptr<IRemoteObject>& remoteObject)
{
    if (workHandler_ == nullptr) {
        HILOGE("NotifyRpcLoadCompleted work handler not initialized!");
        return;
    }
    auto notifyTask = [srcDeviceId, systemAbilityId, remoteObject, this]() {
#ifdef SAMGR_ENABLE_DELAY_DBINDER
        std::shared_lock<samgr::shared_mutex> readLock(dBinderServiceLock_);
#endif
        if (dBinderService_ != nullptr) {
            SamgrXCollie samgrXCollie("samgr--LoadSystemAbilityComplete_" + ToString(systemAbilityId));
            dBinderService_->LoadSystemAbilityComplete(srcDeviceId, systemAbilityId, remoteObject);
            return;
        }
        HILOGW("NotifyRpcLoadCompleted failed, SA:%{public}d, deviceId : %{public}s",
            systemAbilityId, AnonymizeDeviceId(srcDeviceId).c_str());
    };
    ffrt::submit(notifyTask);
}

int32_t SystemAbilityManager::UnloadSystemAbility(int32_t systemAbilityId)
{
    return BaseSystemAbilityManager::UnloadSystemAbility(systemAbilityId);
}

int32_t SystemAbilityManager::CancelUnloadSystemAbility(int32_t systemAbilityId)
{
    return BaseSystemAbilityManager::CancelUnloadSystemAbility(systemAbilityId);
}

int32_t SystemAbilityManager::UnloadAllIdleSystemAbility()
{
    return BaseSystemAbilityManager::UnloadAllIdleSystemAbility();
}

int32_t SystemAbilityManager::UnloadProcess(const std::vector<std::u16string>& processList)
{
    return BaseSystemAbilityManager::UnloadProcess(processList);
}

int32_t SystemAbilityManager::GetLruIdleSystemAbilityProc(std::vector<IdleProcessInfo>& processInfos)
{
    return BaseSystemAbilityManager::GetLruIdleSystemAbilityProc(processInfos);
}

int32_t SystemAbilityManager::OnStartSystemAbilityFail(int32_t systemAbilityId, int32_t errCode)
{
    return BaseSystemAbilityManager::OnStartSystemAbilityFail(systemAbilityId, errCode);
}

int32_t SystemAbilityManager::GetOnDemandSystemAbilityIds(std::vector<int32_t>& systemAbilityIds)
{
    return BaseSystemAbilityManager::GetOnDemandSystemAbilityIds(systemAbilityIds);
}

int32_t SystemAbilityManager::SendStrategy(int32_t type, std::vector<int32_t>& systemAbilityIds,
    int32_t level, std::string& action)
{
    return BaseSystemAbilityManager::SendStrategy(type, systemAbilityIds, level, action);
}

int32_t SystemAbilityManager::GetOnDemandPolicy(int32_t systemAbilityId, OnDemandPolicyType type,
    std::vector<SystemAbilityOnDemandEvent>& abilityOnDemandEvents)
{
    return BaseSystemAbilityManager::GetOnDemandPolicy(systemAbilityId, type, abilityOnDemandEvents);
}

int32_t SystemAbilityManager::UpdateOnDemandPolicy(int32_t systemAbilityId, OnDemandPolicyType type,
    const std::vector<SystemAbilityOnDemandEvent>& abilityOnDemandEvents)
{
    return BaseSystemAbilityManager::UpdateOnDemandPolicy(systemAbilityId, type, abilityOnDemandEvents);
}

int32_t SystemAbilityManager::GetExtensionSaIds(const std::string& extension, std::vector<int32_t>& saIds)
{
    return BaseSystemAbilityManager::GetExtensionSaIds(extension, saIds);
}

int32_t SystemAbilityManager::GetExtensionRunningSaList(const std::string& extension,
    std::vector<sptr<IRemoteObject>>& saList)
{
    return BaseSystemAbilityManager::GetExtensionRunningSaList(extension, saList);
}

int32_t SystemAbilityManager::GetRunningSaExtensionInfoList(const std::string& extension,
    std::vector<SaExtensionInfo>& infoList)
{
    return BaseSystemAbilityManager::GetRunningSaExtensionInfoList(extension, infoList);
}

int32_t SystemAbilityManager::GetCommonEventExtraDataIdlist(int32_t saId, std::vector<int64_t>& extraDataIdList,
    const std::string& eventName)
{
    return BaseSystemAbilityManager::GetCommonEventExtraDataIdlist(saId, extraDataIdList, eventName);
}

void SystemAbilityManager::ReportGetSAPeriodically()
{
    HILOGI("ReportGetSAPeriodically start!");
    lock_guard<samgr::mutex> autoLock(saFrequencyLock_);
    for (const auto& [key, count] : saFrequencyMap_) {
        uint32_t saId = static_cast<uint32_t>(key);
        uint32_t uid = key >> SHFIT_BIT;
        ReportGetSAFrequency(uid, saId, count);
    }
    saFrequencyMap_.clear();
}

void SystemAbilityManager::FlushResetPriorTask()
{
    if (workHandler_->HasInnerEvent(RESET_IPC_PRIOR)) {
        workHandler_->RemoveTask(RESET_IPC_PRIOR);
    }
    auto resetTimeoutTask = [this]() {
        std::lock_guard<std::mutex> lock(priorRefCntLock_);
        HILOGI("ResetPrior for time out");
        priorEnable_ = false;
        ResetIpcPrior();
        priorRefCnt_ = 0;
    };
    workHandler_->PostTask(resetTimeoutTask, RESET_IPC_PRIOR, RESET_IPC_PRIOR_TIMEOUT);
}

int32_t SystemAbilityManager::SetSamgrIpcPrior(bool enable)
{
    if (!isSupportSetPrior_) {
        HILOGD("SetSamgrIpcPrior is not support");
        return ERR_INVALID_OPERATION;
    }
    std::lock_guard<std::mutex> lock(priorRefCntLock_);
    if (enable) {
        if (!priorEnable_) {
            priorEnable_ = true;
            HILOGI("SetSamgrIpcPrior enable");
        }
        ++priorRefCnt_;
        FlushResetPriorTask();
    } else {
        if (!priorEnable_ || priorRefCnt_ <= 0) {
            HILOGD("SetSamgrIpcPrior disable invalid");
            return ERR_OK;
        }
        --priorRefCnt_;
        if (priorRefCnt_ == 0) {
            priorEnable_ = false;
            ResetIpcPrior();
            workHandler_->RemoveTask(RESET_IPC_PRIOR);
            HILOGI("SetSamgrIpcPrior disable");
        }
    }
    return ERR_OK;
}

#ifdef SUPPORT_MULTI_INSTANCE
int32_t SystemAbilityManager::OnUserStateChanged(int32_t userId, SamgrUserState userState)
{
    HILOGI("OnUserStateChanged userId:%{public}d, state:%{public}d", userId, userState);
    std::lock_guard<samgr::mutex> lock(userStateLock_);
    userStateMap_[userId] = userState;
    return ERR_OK;
}
#endif
} // namespace OHOS
