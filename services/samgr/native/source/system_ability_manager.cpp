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
#include "samgr_err_code.h"
#include "samgr_xcollie.h"
#include "string_ex.h"
#include "system_ability_manager_dumper.h"
#include "system_ability_manager_util.h"
#include "tools.h"
#ifdef SUPPORT_MULTI_INSTANCE
#include "multi_system_ability_manager.h"
#endif

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
constexpr uint32_t BASE_CALLING_USER = 0xffffffffU;
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
#ifdef SUPPORT_MULTI_INSTANCE
    userLifecycleManager_.SetSaProfiles(&allSaProfiles_);
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
#ifdef SUPPORT_MULTI_INSTANCE
    const int32_t caller = GetCallingUserId();
    if (!IsValidCallingUserId(caller)) {
        HILOGD("GetSystemAbility: invalid calling userId:%{public}d", caller);
        return nullptr;
    }
    int32_t target = RouteForUser(systemAbilityId, caller);
    if (target != BASE_USER) {
        auto mgr = GetMultiUserManager(target);
        if (mgr == nullptr) {
            HILOGD("GetSystemAbility: multiUserManager[%{public}d] not found", target);
            return nullptr;
        }
        return mgr->CheckSystemAbility(systemAbilityId);
    }
#endif
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
#ifdef SUPPORT_MULTI_INSTANCE
    const int32_t caller = GetCallingUserId();
    if (!IsValidCallingUserId(caller)) {
        HILOGD("CheckSystemAbility: invalid calling userId:%{public}d", caller);
        return nullptr;
    }
    int32_t target = RouteForUser(systemAbilityId, caller);
    if (target != BASE_USER) {
        auto mgr = GetMultiUserManager(target);
        if (mgr == nullptr) {
            HILOGD("CheckSystemAbility: multiUserManager[%{public}d] not found", target);
            return nullptr;
        }
        return mgr->CheckSystemAbility(systemAbilityId);
    }
#endif
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
#ifdef SUPPORT_MULTI_INSTANCE
    const int32_t caller = GetCallingUserId();
    if (!IsValidCallingUserId(caller)) {
        HILOGE("AddSystemAbility: invalid calling userId:%{public}d, SA:%{public}d", caller, systemAbilityId);
        return INVALID_CALLING_USER_ID;
    }
    int32_t routeResult = RouteForSa(systemAbilityId, caller);
    if (routeResult != SAMGR_OK) {
        return routeResult;
    }
    if (caller != BASE_USER) {
        auto mgr = GetMultiUserManager(caller);
        if (mgr == nullptr) {
            HILOGE("AddSystemAbility: multiUserManager[%{public}d] not found, SA:%{public}d",
                caller, systemAbilityId);
            return ERR_INVALID_VALUE;
        }
        return mgr->AddSystemAbility(systemAbilityId, ability, extraProp);
    }
#endif
    return BaseSystemAbilityManager::AddSystemAbility(systemAbilityId, ability, extraProp);
}

int32_t SystemAbilityManager::RemoveSystemAbility(int32_t systemAbilityId)
{
#ifdef SUPPORT_MULTI_INSTANCE
    const int32_t caller = GetCallingUserId();
    if (!IsValidCallingUserId(caller) && !userLifecycleManager_.IsUserStopping(caller)) {
        HILOGD("RemoveSystemAbility: invalid calling userId:%{public}d", caller);
        return INVALID_CALLING_USER_ID;
    }
    int32_t routeResult = RouteForSa(systemAbilityId, caller);
    if (routeResult != SAMGR_OK) {
        return routeResult;
    }
    if (caller != BASE_USER) {
        auto mgr = userLifecycleManager_.IsUserStopping(caller) ?
            GetStoppingMultiUserManager(caller) : GetMultiUserManager(caller);
        if (mgr == nullptr) {
            return ERR_INVALID_VALUE;
        }
        return mgr->RemoveSystemAbility(systemAbilityId);
    }
#endif
    return BaseSystemAbilityManager::RemoveSystemAbility(systemAbilityId);
}

int32_t SystemAbilityManager::SubscribeSystemAbility(int32_t systemAbilityId,
    const sptr<ISystemAbilityStatusChange>& listener)
{
#ifdef SUPPORT_MULTI_INSTANCE
    const int32_t caller = GetCallingUserId();
    if (!IsValidCallingUserId(caller)) {
        HILOGD("SubscribeSystemAbility: invalid calling userId:%{public}d", caller);
        return INVALID_CALLING_USER_ID;
    }

    if (caller == BASE_USER && IsMultiInstanceSaId(systemAbilityId)) {
        if (!BaseSystemAbilityManager::CheckInputSysAbilityId(systemAbilityId) || listener == nullptr ||
            listener->AsObject() == nullptr) {
            return ERR_INVALID_VALUE;
        }
        return userLifecycleManager_.SubscribeSystemAbilityForAllUsers(
            systemAbilityId, listener, IPCSkeleton::GetCallingPid());
    } else {
        int32_t target = RouteForUser(systemAbilityId, caller);
        if (target != BASE_USER) {
            auto mgr = GetMultiUserManager(target);
            if (mgr == nullptr) {
                return ERR_INVALID_VALUE;
            }
            return mgr->SubscribeSystemAbility(systemAbilityId, listener);
        }
    }
#endif
    return BaseSystemAbilityManager::SubscribeSystemAbility(systemAbilityId, listener);
}

int32_t SystemAbilityManager::UnSubscribeSystemAbility(int32_t systemAbilityId,
    const sptr<ISystemAbilityStatusChange>& listener)
{
#ifdef SUPPORT_MULTI_INSTANCE
    const int32_t caller = GetCallingUserId();
    if (!IsValidCallingUserId(caller)) {
        HILOGD("UnSubscribeSystemAbility: invalid calling userId:%{public}d", caller);
        return INVALID_CALLING_USER_ID;
    }

    if (caller == BASE_USER && IsMultiInstanceSaId(systemAbilityId)) {
        return userLifecycleManager_.UnSubscribeSystemAbilityForAllUsers(systemAbilityId, listener);
    } else {
        int32_t target = RouteForUser(systemAbilityId, caller);
        if (target != BASE_USER) {
            auto mgr = GetMultiUserManager(target);
            if (mgr == nullptr) {
                return ERR_INVALID_VALUE;
            }
            return mgr->UnSubscribeSystemAbility(systemAbilityId, listener);
        }
    }
#endif
    return BaseSystemAbilityManager::UnSubscribeSystemAbility(systemAbilityId, listener);
}

sptr<IRemoteObject> SystemAbilityManager::CheckSystemAbility(int32_t systemAbilityId, bool& isExist)
{
#ifdef SUPPORT_MULTI_INSTANCE
    const int32_t caller = GetCallingUserId();
    if (!IsValidCallingUserId(caller)) {
        HILOGD("CheckSystemAbility(isExist): invalid calling userId:%{public}d", caller);
        return nullptr;
    }
    int32_t target = RouteForUser(systemAbilityId, caller);
    if (target != BASE_USER) {
        auto mgr = GetMultiUserManager(target);
        if (mgr == nullptr) {
            HILOGD("CheckSystemAbility(isExist): multiUserManager[%{public}d] not found", target);
            return nullptr;
        }
        return mgr->CheckSystemAbility(systemAbilityId, isExist);
    }
#endif
    return BaseSystemAbilityManager::CheckSystemAbility(systemAbilityId, isExist);
}

int32_t SystemAbilityManager::AddOnDemandSystemAbilityInfo(int32_t systemAbilityId,
    const std::u16string& procName)
{
#ifdef SUPPORT_MULTI_INSTANCE
    const int32_t caller = GetCallingUserId();
    if (!IsValidCallingUserId(caller)) {
        HILOGD("AddOnDemandSystemAbilityInfo: invalid calling userId:%{public}d", caller);
        return INVALID_CALLING_USER_ID;
    }
    int32_t routeResult = RouteForSa(systemAbilityId, caller);
    if (routeResult != SAMGR_OK) {
        return routeResult;
    }
    if (caller != BASE_USER) {
        auto mgr = GetMultiUserManager(caller);
        if (mgr == nullptr) {
            return ERR_INVALID_VALUE;
        }
        return mgr->AddOnDemandSystemAbilityInfo(systemAbilityId, procName);
    }
#endif
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
#ifdef SUPPORT_MULTI_INSTANCE
    const int32_t caller = GetCallingUserId();
    if (!IsValidCallingUserId(caller)) {
        HILOGD("AddSystemProcess: invalid calling userId:%{public}d", caller);
        return INVALID_CALLING_USER_ID;
    }
    if (caller != BASE_USER) {
        auto mgr = GetMultiUserManager(caller);
        if (mgr == nullptr) {
            HILOGE("AddSystemProcess: multiUserManager[%{public}d] not found, proc:%{public}s",
                caller, Str16ToStr8(procName).c_str());
            return ERR_INVALID_VALUE;
        }
        return mgr->AddSystemProcess(procName, procObject);
    }
#endif
    return BaseSystemAbilityManager::AddSystemProcess(procName, procObject);
}

int32_t SystemAbilityManager::GetSystemProcessInfo(int32_t systemAbilityId, SystemProcessInfo& systemProcessInfo)
{
#ifdef SUPPORT_MULTI_INSTANCE
    const int32_t caller = GetCallingUserId();
    if (!IsValidCallingUserId(caller)) {
        HILOGD("GetSystemProcessInfo: invalid calling userId:%{public}d", caller);
        return INVALID_CALLING_USER_ID;
    }
    int32_t target = RouteForUser(systemAbilityId, caller);
    if (target != BASE_USER) {
        auto mgr = GetMultiUserManager(target);
        if (mgr == nullptr) {
            HILOGD("GetSystemProcessInfo: multiUserManager[%{public}d] not found", target);
            return ERR_INVALID_VALUE;
        }
        return mgr->GetSystemProcessInfo(systemAbilityId, systemProcessInfo);
    }
#endif
    return BaseSystemAbilityManager::GetSystemProcessInfo(systemAbilityId, systemProcessInfo);
}

int32_t SystemAbilityManager::GetRunningSystemProcess(std::list<SystemProcessInfo>& systemProcessInfos)
{
#ifdef SUPPORT_MULTI_INSTANCE
    const int32_t caller = GetCallingUserId();
    if (!IsValidCallingUserId(caller)) {
        HILOGD("GetRunningSystemProcess: invalid calling userId:%{public}d", caller);
        return INVALID_CALLING_USER_ID;
    }
#endif
    int32_t result = BaseSystemAbilityManager::GetRunningSystemProcess(systemProcessInfos);
#ifdef SUPPORT_MULTI_INSTANCE
    for (int32_t userId : userLifecycleManager_.GetValidUserIds()) {
        auto mgr = GetMultiUserManager(userId);
        if (mgr != nullptr) {
            std::list<SystemProcessInfo> userProcessInfos;
            int32_t userResult = mgr->GetRunningSystemProcess(userProcessInfos);
            if (userResult == ERR_OK) {
                systemProcessInfos.insert(systemProcessInfos.end(), userProcessInfos.begin(), userProcessInfos.end());
            } else if (result == ERR_OK) {
                result = userResult;
            }
        }
    }
#endif
    return result;
}

int32_t SystemAbilityManager::SubscribeSystemProcess(const sptr<ISystemProcessStatusChange>& listener)
{
#ifdef SUPPORT_MULTI_INSTANCE
    const int32_t caller = GetCallingUserId();
    if (!IsValidCallingUserId(caller)) {
        HILOGD("SubscribeSystemProcess: invalid calling userId:%{public}d", caller);
        return INVALID_CALLING_USER_ID;
    }

    if (caller == BASE_USER) {
        bool subscriptionAdded = false;
        int32_t result = userLifecycleManager_.SubscribeSystemProcessForAllUsers(listener, subscriptionAdded);
        if (result != ERR_OK) {
            return result;
        }
        result = BaseSystemAbilityManager::SubscribeSystemProcess(listener);
        if (result != ERR_OK && subscriptionAdded) {
            int32_t rollbackResult = userLifecycleManager_.UnSubscribeSystemProcessForAllUsers(listener);
            if (rollbackResult != ERR_OK) {
                HILOGE("SubscribeSystemProcess rollback failed, result:%{public}d", rollbackResult);
            }
        }
        return result;
    } else {
        auto mgr = GetMultiUserManager(caller);
        if (mgr == nullptr) {
            HILOGD("SubscribeSystemProcess: multiUserManager[%{public}d] not found", caller);
            return ERR_INVALID_VALUE;
        }
        return mgr->SubscribeSystemProcess(listener);
    }
#endif
    return BaseSystemAbilityManager::SubscribeSystemProcess(listener);
}

int32_t SystemAbilityManager::UnSubscribeSystemProcess(const sptr<ISystemProcessStatusChange>& listener)
{
#ifdef SUPPORT_MULTI_INSTANCE
    const int32_t caller = GetCallingUserId();
    if (!IsValidCallingUserId(caller)) {
        HILOGD("UnSubscribeSystemProcess: invalid calling userId:%{public}d", caller);
        return INVALID_CALLING_USER_ID;
    }

    if (caller == BASE_USER) {
        int32_t result = userLifecycleManager_.UnSubscribeSystemProcessForAllUsers(listener);
        if (result != ERR_OK) {
            return result;
        }
        result = BaseSystemAbilityManager::UnSubscribeSystemProcess(listener);
        if (result != ERR_OK) {
            bool subscriptionAdded = false;
            int32_t rollbackResult =
                userLifecycleManager_.SubscribeSystemProcessForAllUsers(listener, subscriptionAdded);
            if (rollbackResult != ERR_OK) {
                HILOGE("UnSubscribeSystemProcess rollback failed, result:%{public}d", rollbackResult);
            }
        }
        return result;
    } else {
        auto mgr = GetMultiUserManager(caller);
        if (mgr == nullptr) {
            HILOGD("UnSubscribeSystemProcess: multiUserManager[%{public}d] not found", caller);
            return ERR_INVALID_VALUE;
        }
        return mgr->UnSubscribeSystemProcess(listener);
    }
#endif
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
#ifdef SUPPORT_MULTI_INSTANCE
    const int32_t caller = GetCallingUserId();
    if (!IsValidCallingUserId(caller)) {
        HILOGE("LoadSystemAbility(callback): invalid calling userId:%{public}d", caller);
        return INVALID_CALLING_USER_ID;
    }
    int32_t target = RouteForUser(systemAbilityId, caller);
    if (target != BASE_USER) {
        auto mgr = GetMultiUserManager(target);
        if (mgr == nullptr) {
            HILOGE("LoadSystemAbility(callback): multiUserManager[%{public}d] not found", target);
            return ERR_INVALID_VALUE;
        }
        return mgr->LoadSystemAbility(systemAbilityId, callback);
    }
#endif
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
#ifdef SUPPORT_MULTI_INSTANCE
    const int32_t caller = GetCallingUserId();
    if (!IsValidCallingUserId(caller)) {
        HILOGD("UnloadSystemAbility: invalid calling userId:%{public}d", caller);
        return INVALID_CALLING_USER_ID;
    }
    int32_t routeResult = RouteForSa(systemAbilityId, caller);
    if (routeResult != SAMGR_OK) {
        return routeResult;
    }
    if (caller != BASE_USER) {
        auto mgr = GetMultiUserManager(caller);
        if (mgr == nullptr) {
            return ERR_INVALID_VALUE;
        }
        return mgr->UnloadSystemAbility(systemAbilityId);
    }
#endif
    return BaseSystemAbilityManager::UnloadSystemAbility(systemAbilityId);
}

int32_t SystemAbilityManager::CancelUnloadSystemAbility(int32_t systemAbilityId)
{
#ifdef SUPPORT_MULTI_INSTANCE
    const int32_t caller = GetCallingUserId();
    if (!IsValidCallingUserId(caller)) {
        HILOGD("CancelUnloadSystemAbility: invalid calling userId:%{public}d", caller);
        return INVALID_CALLING_USER_ID;
    }
    int32_t routeResult = RouteForSa(systemAbilityId, caller);
    if (routeResult != SAMGR_OK) {
        return routeResult;
    }
    if (caller != BASE_USER) {
        auto mgr = GetMultiUserManager(caller);
        if (mgr == nullptr) {
            return ERR_INVALID_VALUE;
        }
        return mgr->CancelUnloadSystemAbility(systemAbilityId);
    }
#endif
    return BaseSystemAbilityManager::CancelUnloadSystemAbility(systemAbilityId);
}

int32_t SystemAbilityManager::UnloadAllIdleSystemAbility()
{
#ifdef SUPPORT_MULTI_INSTANCE
    const int32_t caller = GetCallingUserId();
    if (!IsValidCallingUserId(caller)) {
        HILOGD("UnloadAllIdleSystemAbility: invalid calling userId:%{public}d", caller);
        return INVALID_CALLING_USER_ID;
    }
#endif
    int32_t result = BaseSystemAbilityManager::UnloadAllIdleSystemAbility();
#ifdef SUPPORT_MULTI_INSTANCE
    for (int32_t userId : userLifecycleManager_.GetValidUserIds()) {
        auto mgr = GetMultiUserManager(userId);
        if (mgr != nullptr) {
            int32_t userResult = mgr->UnloadAllIdleSystemAbility();
            if (result == ERR_OK && userResult != ERR_OK) {
                result = userResult;
            }
        }
    }
#endif
    return result;
}

int32_t SystemAbilityManager::UnloadProcess(const std::vector<std::u16string>& processList)
{
#ifdef SUPPORT_MULTI_INSTANCE
    const int32_t caller = GetCallingUserId();
    if (!IsValidCallingUserId(caller)) {
        HILOGD("UnloadProcess: invalid calling userId:%{public}d", caller);
        return INVALID_CALLING_USER_ID;
    }
#endif
    int32_t result = BaseSystemAbilityManager::UnloadProcess(processList);
#ifdef SUPPORT_MULTI_INSTANCE
    for (int32_t userId : userLifecycleManager_.GetValidUserIds()) {
        auto mgr = GetMultiUserManager(userId);
        if (mgr != nullptr) {
            int32_t userResult = mgr->UnloadProcess(processList);
            if (result == ERR_OK && userResult != ERR_OK) {
                result = userResult;
            }
        }
    }
#endif
    return result;
}

int32_t SystemAbilityManager::GetLruIdleSystemAbilityProc(std::vector<IdleProcessInfo>& processInfos)
{
#ifdef SUPPORT_MULTI_INSTANCE
    const int32_t caller = GetCallingUserId();
    if (!IsValidCallingUserId(caller)) {
        HILOGD("GetLruIdleSystemAbilityProc: invalid calling userId:%{public}d", caller);
        return INVALID_CALLING_USER_ID;
    }
#endif
    int32_t result = BaseSystemAbilityManager::GetLruIdleSystemAbilityProc(processInfos);
#ifdef SUPPORT_MULTI_INSTANCE
    for (int32_t userId : userLifecycleManager_.GetValidUserIds()) {
        auto mgr = GetMultiUserManager(userId);
        if (mgr != nullptr) {
            std::vector<IdleProcessInfo> userProcessInfos;
            int32_t userResult = mgr->GetLruIdleSystemAbilityProc(userProcessInfos);
            if (userResult == ERR_OK) {
                processInfos.insert(processInfos.end(), userProcessInfos.begin(), userProcessInfos.end());
            } else if (result == ERR_OK) {
                result = userResult;
            }
        }
    }
#endif
    return result;
}

int32_t SystemAbilityManager::OnStartSystemAbilityFail(int32_t systemAbilityId, int32_t errCode)
{
    return BaseSystemAbilityManager::OnStartSystemAbilityFail(systemAbilityId, errCode);
}

sptr<IRemoteObject> SystemAbilityManager::GetLocalAbilityManagerProxy(int32_t systemAbilityId)
{
#ifdef SUPPORT_MULTI_INSTANCE
    const int32_t caller = GetCallingUserId();
    if (!IsValidCallingUserId(caller)) {
        HILOGD("GetLocalAbilityManagerProxy: invalid calling userId:%{public}d", caller);
        return nullptr;
    }
    int32_t target = RouteForUser(systemAbilityId, caller);
    if (target != BASE_USER) {
        auto mgr = GetMultiUserManager(target);
        if (mgr == nullptr) {
            HILOGD("GetLocalAbilityManagerProxy: multiUserManager[%{public}d] not found", target);
            return nullptr;
        }
        return mgr->GetLocalAbilityManagerProxy(systemAbilityId);
    }
#endif
    return BaseSystemAbilityManager::GetLocalAbilityManagerProxy(systemAbilityId);
}

int32_t SystemAbilityManager::GetOnDemandSystemAbilityIds(std::vector<int32_t>& systemAbilityIds)
{
    return BaseSystemAbilityManager::GetOnDemandSystemAbilityIds(systemAbilityIds);
}

int32_t SystemAbilityManager::SendStrategy(int32_t type, std::vector<int32_t>& systemAbilityIds,
    int32_t level, std::string& action)
{
#ifdef SUPPORT_MULTI_INSTANCE
    const int32_t caller = GetCallingUserId();
    if (!IsValidCallingUserId(caller)) {
        HILOGD("SendStrategy: invalid calling userId:%{public}d", caller);
        return INVALID_CALLING_USER_ID;
    }

    if (systemAbilityIds.empty()) {
        return BaseSystemAbilityManager::SendStrategy(type, systemAbilityIds, level, action);
    }

    bool dispatched = false;
    int32_t result = ERR_OK;
    std::vector<int32_t> baseSaIds;
    std::vector<int32_t> multiSaIds;
    for (int32_t saId : systemAbilityIds) {
        if (IsMultiInstanceSaId(saId)) {
            multiSaIds.emplace_back(saId);
        } else {
            baseSaIds.emplace_back(saId);
        }
    }
    if (!baseSaIds.empty()) {
        dispatched = true;
        int32_t baseResult = BaseSystemAbilityManager::SendStrategy(type, baseSaIds, level, action);
        if (baseResult != ERR_OK) {
            result = baseResult;
        }
    }

    if (!multiSaIds.empty()) {
        int32_t multiResult = SendStrategyToUsers(type, multiSaIds, level, action, dispatched);
        if (result == ERR_OK) {
            result = multiResult;
        }
    }
    return dispatched ? result : ERR_INVALID_VALUE;
#endif
    return BaseSystemAbilityManager::SendStrategy(type, systemAbilityIds, level, action);
}

#ifdef SUPPORT_MULTI_INSTANCE
int32_t SystemAbilityManager::SendStrategyToUsers(int32_t type,
    std::vector<int32_t>& systemAbilityIds, int32_t level, std::string& action, bool& dispatched)
{
    int32_t result = ERR_OK;
    for (int32_t userId : userLifecycleManager_.GetValidUserIds()) {
        auto mgr = GetMultiUserManager(userId);
        if (mgr == nullptr) {
            continue;
        }
        dispatched = true;
        int32_t userResult = mgr->SendStrategy(type, systemAbilityIds, level, action);
        if (result == ERR_OK && userResult != ERR_OK) {
            result = userResult;
        }
    }
    return result;
}
#endif

int32_t SystemAbilityManager::GetOnDemandPolicy(int32_t systemAbilityId, OnDemandPolicyType type,
    std::vector<SystemAbilityOnDemandEvent>& abilityOnDemandEvents)
{
#ifdef SUPPORT_MULTI_INSTANCE
    const int32_t caller = GetCallingUserId();
    if (!IsValidCallingUserId(caller)) {
        HILOGD("GetOnDemandPolicy: invalid calling userId:%{public}d", caller);
        return INVALID_CALLING_USER_ID;
    }
    int32_t routeResult = RouteForSa(systemAbilityId, caller);
    if (routeResult != SAMGR_OK) {
        return routeResult;
    }
    if (caller != BASE_USER) {
        auto mgr = GetMultiUserManager(caller);
        if (mgr == nullptr) {
            return ERR_INVALID_VALUE;
        }
        return mgr->GetOnDemandPolicy(systemAbilityId, type, abilityOnDemandEvents);
    }
#endif
    return BaseSystemAbilityManager::GetOnDemandPolicy(systemAbilityId, type, abilityOnDemandEvents);
}

int32_t SystemAbilityManager::UpdateOnDemandPolicy(int32_t systemAbilityId, OnDemandPolicyType type,
    const std::vector<SystemAbilityOnDemandEvent>& abilityOnDemandEvents)
{
#ifdef SUPPORT_MULTI_INSTANCE
    const int32_t caller = GetCallingUserId();
    if (!IsValidCallingUserId(caller)) {
        HILOGD("UpdateOnDemandPolicy: invalid calling userId:%{public}d", caller);
        return INVALID_CALLING_USER_ID;
    }
    int32_t routeResult = RouteForSa(systemAbilityId, caller);
    if (routeResult != SAMGR_OK) {
        return routeResult;
    }
    if (caller != BASE_USER) {
        auto mgr = GetMultiUserManager(caller);
        if (mgr == nullptr) {
            return ERR_INVALID_VALUE;
        }
        return mgr->UpdateOnDemandPolicy(systemAbilityId, type, abilityOnDemandEvents);
    }
#endif
    return BaseSystemAbilityManager::UpdateOnDemandPolicy(systemAbilityId, type, abilityOnDemandEvents);
}

int32_t SystemAbilityManager::GetExtensionSaIds(const std::string& extension, std::vector<int32_t>& saIds)
{
    return BaseSystemAbilityManager::GetExtensionSaIds(extension, saIds);
}

int32_t SystemAbilityManager::GetExtensionRunningSaList(const std::string& extension,
    std::vector<sptr<IRemoteObject>>& saList)
{
#ifdef SUPPORT_MULTI_INSTANCE
    const int32_t caller = GetCallingUserId();
    if (!IsValidCallingUserId(caller)) {
        HILOGD("GetExtensionRunningSaList: invalid calling userId:%{public}d", caller);
        return INVALID_CALLING_USER_ID;
    }
#endif
    int32_t result = BaseSystemAbilityManager::GetExtensionRunningSaList(extension, saList);
#ifdef SUPPORT_MULTI_INSTANCE
    for (int32_t userId : userLifecycleManager_.GetValidUserIds()) {
        auto mgr = GetMultiUserManager(userId);
        if (mgr != nullptr) {
            std::vector<sptr<IRemoteObject>> userSaList;
            int32_t userResult = mgr->GetExtensionRunningSaList(extension, userSaList);
            if (userResult == ERR_OK) {
                saList.insert(saList.end(), userSaList.begin(), userSaList.end());
            } else if (result == ERR_OK) {
                result = userResult;
            }
        }
    }
#endif
    return result;
}

int32_t SystemAbilityManager::GetRunningSaExtensionInfoList(const std::string& extension,
    std::vector<SaExtensionInfo>& infoList)
{
#ifdef SUPPORT_MULTI_INSTANCE
    const int32_t caller = GetCallingUserId();
    if (!IsValidCallingUserId(caller)) {
        HILOGD("GetRunningSaExtensionInfoList: invalid calling userId:%{public}d", caller);
        return INVALID_CALLING_USER_ID;
    }
#endif
    int32_t result = BaseSystemAbilityManager::GetRunningSaExtensionInfoList(extension, infoList);
#ifdef SUPPORT_MULTI_INSTANCE
    for (int32_t userId : userLifecycleManager_.GetValidUserIds()) {
        auto mgr = GetMultiUserManager(userId);
        if (mgr != nullptr) {
            std::vector<SaExtensionInfo> userInfoList;
            int32_t userResult = mgr->GetRunningSaExtensionInfoList(extension, userInfoList);
            if (userResult == ERR_OK) {
                infoList.insert(infoList.end(), userInfoList.begin(), userInfoList.end());
            } else if (result == ERR_OK) {
                result = userResult;
            }
        }
    }
#endif
    return result;
}

int32_t SystemAbilityManager::GetCommonEventExtraDataIdlist(int32_t saId, std::vector<int64_t>& extraDataIdList,
    const std::string& eventName)
{
#ifdef SUPPORT_MULTI_INSTANCE
    const int32_t caller = GetCallingUserId();
    if (!IsValidCallingUserId(caller)) {
        HILOGD("GetCommonEventExtraDataIdlist: invalid calling userId:%{public}d", caller);
        return INVALID_CALLING_USER_ID;
    }
    int32_t routeResult = RouteForSa(saId, caller);
    if (routeResult != SAMGR_OK) {
        return routeResult;
    }
    if (caller != BASE_USER) {
        auto mgr = GetMultiUserManager(caller);
        if (mgr == nullptr) {
            return ERR_INVALID_VALUE;
        }
        return mgr->GetCommonEventExtraDataIdlist(saId, extraDataIdList, eventName);
    }
#endif
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
    if (userId == SAMGR_INVALID_USER_ID || userId == BASE_USER) {
        HILOGW("OnUserStateChanged invalid userId:%{public}d", userId);
        return ERR_INVALID_VALUE;
    }
    return userLifecycleManager_.OnUserStateChanged(userId, userState);
}

bool SystemAbilityManager::IsMultiInstanceSaId(int32_t saId)
{
    std::set<int32_t> multiInstanceSaIds = BaseSystemAbilityManager::GetMultiInstanceSaIds();
    return multiInstanceSaIds.find(saId) != multiInstanceSaIds.end();
}

int32_t SystemAbilityManager::GetCallingUserId() const
{
    const uint32_t callingUserId = static_cast<uint32_t>(IPCSkeleton::GetCallingUserID() >> SHFIT_BIT);
    if (callingUserId == BASE_CALLING_USER) {
        return BASE_USER;
    }

    return static_cast<int32_t>(callingUserId);
}

bool SystemAbilityManager::IsValidCallingUserId(int32_t userId) const
{
    return userId == BASE_USER || IsValidUser(userId);
}

int32_t SystemAbilityManager::RouteForUser(int32_t saId, int32_t caller)
{
    if (!IsMultiInstanceSaId(saId)) {
        return BASE_USER;
    }

    return caller == BASE_USER ? GetForegroundUserId() : caller;
}

int32_t SystemAbilityManager::RouteForSa(int32_t saId, int32_t caller)
{
    const bool isMultiInstance = IsMultiInstanceSaId(saId);
    if (caller == BASE_USER) {
        return isMultiInstance ? SA_OPERATION_NOT_ALLOWED : SAMGR_OK;
    }

    return isMultiInstance ? SAMGR_OK : SA_OPERATION_NOT_ALLOWED;
}

sptr<IRemoteObject> SystemAbilityManager::GetSystemAbility(int32_t systemAbilityId, int32_t userId)
{
    const int32_t caller = GetCallingUserId();
    if (caller != BASE_USER) {
        HILOGD("GetSystemAbility(userId): caller is not base, caller:%{public}d", caller);
        return nullptr;
    }
    if (!IsValidUser(userId)) {
        HILOGD("GetSystemAbility(userId): invalid target userId:%{public}d", userId);
        return nullptr;
    }
    auto mgr = GetMultiUserManager(userId);
    if (mgr == nullptr) {
        HILOGW("GetSystemAbility(userId) reject: manager not found, userId:%{public}d", userId);
        return nullptr;
    }
    return mgr->CheckSystemAbility(systemAbilityId);
}

sptr<IRemoteObject> SystemAbilityManager::CheckSystemAbility(int32_t systemAbilityId, int32_t userId)
{
    const int32_t caller = GetCallingUserId();
    if (caller != BASE_USER) {
        HILOGD("CheckSystemAbility(userId): caller is not base, caller:%{public}d", caller);
        return nullptr;
    }
    if (!IsValidUser(userId)) {
        HILOGD("CheckSystemAbility(userId): invalid target userId:%{public}d", userId);
        return nullptr;
    }
    auto mgr = GetMultiUserManager(userId);
    if (mgr == nullptr) {
        HILOGW("CheckSystemAbility(userId) reject: manager not found, userId:%{public}d", userId);
        return nullptr;
    }
    return mgr->CheckSystemAbility(systemAbilityId);
}

sptr<IRemoteObject> SystemAbilityManager::CheckSystemAbilityByUserId(
    int32_t systemAbilityId, bool& isExist, int32_t userId)
{
    isExist = false;
    const int32_t caller = GetCallingUserId();
    if (caller != BASE_USER) {
        HILOGD("CheckSystemAbilityByUserId: caller is not base, caller:%{public}d", caller);
        return nullptr;
    }
    if (!IsValidUser(userId)) {
        HILOGD("CheckSystemAbilityByUserId: invalid target userId:%{public}d", userId);
        return nullptr;
    }
    auto mgr = GetMultiUserManager(userId);
    if (mgr == nullptr) {
        HILOGW("CheckSystemAbilityByUserId(bool) reject: manager not found, userId:%{public}d", userId);
        return nullptr;
    }
    return mgr->CheckSystemAbility(systemAbilityId, isExist);
}

int32_t SystemAbilityManager::GetSystemProcessInfo(int32_t systemAbilityId,
    SystemProcessInfo& systemProcessInfo, int32_t userId)
{
    const int32_t caller = GetCallingUserId();
    if (caller != BASE_USER) {
        HILOGD("GetSystemProcessInfo(userId): caller is not base, caller:%{public}d", caller);
        return ERR_PERMISSION_DENIED;
    }
    if (!IsValidUser(userId)) {
        HILOGD("GetSystemProcessInfo(userId): invalid target userId:%{public}d", userId);
        return INVALID_CALLING_USER_ID;
    }
    auto mgr = GetMultiUserManager(userId);
    if (mgr == nullptr) {
        HILOGW("GetSystemProcessInfoByUserId reject: manager not found, userId:%{public}d", userId);
        return ERR_INVALID_VALUE;
    }
    return mgr->GetSystemProcessInfo(systemAbilityId, systemProcessInfo);
}

sptr<IRemoteObject> SystemAbilityManager::GetLocalAbilityManagerProxy(int32_t systemAbilityId, int32_t userId)
{
    const int32_t caller = GetCallingUserId();
    if (caller != BASE_USER) {
        HILOGD("GetLocalAbilityManagerProxy(userId): caller is not base, caller:%{public}d", caller);
        return nullptr;
    }
    if (!IsValidUser(userId)) {
        HILOGD("GetLocalAbilityManagerProxy(userId): invalid target userId:%{public}d", userId);
        return nullptr;
    }
    auto mgr = GetMultiUserManager(userId);
    if (mgr == nullptr) {
        HILOGW("GetLocalAbilityManagerProxyByUserId reject: manager not found, userId:%{public}d", userId);
        return nullptr;
    }
    return mgr->GetLocalAbilityManagerProxy(systemAbilityId);
}

int32_t SystemAbilityManager::LoadSystemAbility(int32_t systemAbilityId,
    const sptr<ISystemAbilityLoadCallback>& callback, int32_t userId)
{
    const int32_t caller = GetCallingUserId();
    if (caller != BASE_USER) {
        HILOGD("LoadSystemAbility(userId): caller is not base, caller:%{public}d", caller);
        return ERR_PERMISSION_DENIED;
    }
    if (!IsValidUser(userId)) {
        HILOGD("LoadSystemAbility(userId): invalid target userId:%{public}d", userId);
        return INVALID_CALLING_USER_ID;
    }
    auto mgr = GetMultiUserManager(userId);
    if (mgr == nullptr) {
        HILOGW("LoadSystemAbilityByUserId reject: manager not found, userId:%{public}d", userId);
        return ERR_INVALID_VALUE;
    }
    return mgr->LoadSystemAbility(systemAbilityId, callback);
}

int32_t SystemAbilityManager::SubscribeSystemAbility(int32_t systemAbilityId,
    const sptr<ISystemAbilityStatusChange>& listener, int32_t userId)
{
    const int32_t caller = GetCallingUserId();
    if (caller != BASE_USER) {
        HILOGD("SubscribeSystemAbility(userId): caller is not base, caller:%{public}d", caller);
        return ERR_PERMISSION_DENIED;
    }
    if (!IsValidUser(userId)) {
        HILOGD("SubscribeSystemAbility(userId): invalid target userId:%{public}d", userId);
        return INVALID_CALLING_USER_ID;
    }
    auto mgr = GetMultiUserManager(userId);
    if (mgr == nullptr) {
        HILOGW("SubscribeSystemAbilityByUserId reject: manager not found, userId:%{public}d", userId);
        return ERR_INVALID_VALUE;
    }
    return mgr->SubscribeSystemAbility(systemAbilityId, listener, false);
}

int32_t SystemAbilityManager::UnSubscribeSystemAbility(int32_t systemAbilityId,
    const sptr<ISystemAbilityStatusChange>& listener, int32_t userId)
{
    const int32_t caller = GetCallingUserId();
    if (caller != BASE_USER) {
        HILOGD("UnSubscribeSystemAbility(userId): caller is not base, caller:%{public}d", caller);
        return ERR_PERMISSION_DENIED;
    }
    if (!IsValidUser(userId)) {
        HILOGD("UnSubscribeSystemAbility(userId): invalid target userId:%{public}d", userId);
        return INVALID_CALLING_USER_ID;
    }
    auto mgr = GetMultiUserManager(userId);
    if (mgr == nullptr) {
        HILOGW("UnSubscribeSystemAbilityByUserId reject: manager not found, userId:%{public}d", userId);
        return ERR_INVALID_VALUE;
    }
    return mgr->UnSubscribeSystemAbility(systemAbilityId, listener, false);
}

int32_t SystemAbilityManager::SubscribeSystemProcess(const sptr<ISystemProcessStatusChange>& listener, int32_t userId)
{
    const int32_t caller = GetCallingUserId();
    if (caller != BASE_USER) {
        HILOGD("SubscribeSystemProcess(userId): caller is not base, caller:%{public}d", caller);
        return ERR_PERMISSION_DENIED;
    }
    if (!IsValidUser(userId)) {
        HILOGD("SubscribeSystemProcess(userId): invalid target userId:%{public}d", userId);
        return INVALID_CALLING_USER_ID;
    }
    auto mgr = GetMultiUserManager(userId);
    if (mgr == nullptr) {
        HILOGW("SubscribeSystemProcessByUserId reject: manager not found, userId:%{public}d", userId);
        return ERR_INVALID_VALUE;
    }
    return mgr->SubscribeSystemProcess(listener, false);
}

int32_t SystemAbilityManager::UnSubscribeSystemProcess(const sptr<ISystemProcessStatusChange>& listener, int32_t userId)
{
    const int32_t caller = GetCallingUserId();
    if (caller != BASE_USER) {
        HILOGD("UnSubscribeSystemProcess(userId): caller is not base, caller:%{public}d", caller);
        return ERR_PERMISSION_DENIED;
    }
    if (!IsValidUser(userId)) {
        HILOGD("UnSubscribeSystemProcess(userId): invalid target userId:%{public}d", userId);
        return INVALID_CALLING_USER_ID;
    }
    auto mgr = GetMultiUserManager(userId);
    if (mgr == nullptr) {
        HILOGW("UnSubscribeSystemProcessByUserId reject: manager not found, userId:%{public}d", userId);
        return ERR_INVALID_VALUE;
    }
    return mgr->UnSubscribeSystemProcess(listener, false);
}

#endif // SUPPORT_MULTI_INSTANCE
} // namespace OHOS
