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

#include "system_ability_manager.h"

#include <cinttypes>
#include <thread>
#include <unistd.h>

#include "ability_death_recipient.h"
#include "accesstoken_kit.h"
#include "datetime_ex.h"
#include "directory_ex.h"
#include "errors.h"
#include "file_ex.h"
#include "hisysevent_adapter.h"
#include "hitrace_meter.h"
#include "samgr_err_code.h"
#include "if_local_ability_manager.h"
#include "ipc_skeleton.h"
#include "local_ability_manager_proxy.h"
#include "memory_guard.h"
#include "parse_util.h"
#include "parameter.h"
#include "parameters.h"
#include "sam_log.h"
#include "service_control.h"
#include "string_ex.h"
#include "system_ability_manager_util.h"
#include "system_ability_manager_dumper.h"
#include "tools.h"
#include "samgr_xcollie.h"

#ifdef SUPPORT_DEVICE_MANAGER
#include "device_manager.h"
using namespace OHOS::DistributedHardware;
#endif

using namespace std;

namespace OHOS {
namespace {
constexpr const char* START_SAID = "said";
constexpr const char* PKG_NAME = "Samgr_Networking";
constexpr const char* PREFIX = "/system/profile/";
constexpr const char* LOCAL_DEVICE = "local";
constexpr const char* ONDEMAND_PARAM = "persist.samgr.perf.ondemand";
constexpr const char* DYNAMIC_CACHE_PARAM = "persist.samgr.cache.sa";
constexpr const char* RESOURCE_SCHEDULE_PROCESS_NAME = "resource_schedule_service";
constexpr const char* IPC_STAT_DUMP_PREFIX = "--ipc";
constexpr const char* ONDEMAND_PERF_PARAM = "persist.samgr.perf.ondemand";
constexpr const char* ONDEMAND_WORKER = "OndemandLoader";
constexpr const char* ARGS_FFRT_PARAM = "--ffrt";
constexpr const char* BOOT_INIT_TIME_PARAM = "ohos.boot.time.init";
constexpr const char* DEFAULT_BOOT_INIT_TIME = "0";

constexpr uint32_t REPORT_GET_SA_INTERVAL = 24 * 60 * 60 * 1000; // ms and is one day
constexpr int32_t MAX_SUBSCRIBE_COUNT = 256;
constexpr int32_t MAX_SA_FREQUENCY_COUNT = INT32_MAX - 1000000;
constexpr int32_t SHFIT_BIT = 32;
constexpr int32_t DEVICE_INFO_SERVICE_SA = 3902;
constexpr int32_t HIDUMPER_SERVICE_SA = 1212;
constexpr int32_t MEDIA_ANALYSIS_SERVICE_SA = 10120;
constexpr int64_t ONDEMAND_PERF_DELAY_TIME = 60 * 1000; // ms
constexpr int64_t CHECK_LOADED_DELAY_TIME = 4 * 1000; // ms
constexpr int32_t SOFTBUS_SERVER_SA_ID = 4700;
constexpr int32_t FFRT_DUMP_INDEX = 0;
}

std::mutex SystemAbilityManager::instanceLock;
sptr<SystemAbilityManager> SystemAbilityManager::instance;

SystemAbilityManager::SystemAbilityManager()
{
    dBinderService_ = DBinderService::GetInstance();
}

SystemAbilityManager::~SystemAbilityManager()
{
    if (reportEventTimer_ != nullptr) {
        reportEventTimer_->Shutdown();
    }
}

void SystemAbilityManager::Init()
{
    abilityDeath_ = sptr<IRemoteObject::DeathRecipient>(new AbilityDeathRecipient());
    systemProcessDeath_ = sptr<IRemoteObject::DeathRecipient>(new SystemProcessDeathRecipient());
    abilityStatusDeath_ = sptr<IRemoteObject::DeathRecipient>(new AbilityStatusDeathRecipient());
    abilityCallbackDeath_ = sptr<IRemoteObject::DeathRecipient>(new AbilityCallbackDeathRecipient());
    remoteCallbackDeath_ = sptr<IRemoteObject::DeathRecipient>(new RemoteCallbackDeathRecipient());

    rpcCallbackImp_ = make_shared<RpcCallbackImp>();
    if (workHandler_ == nullptr) {
        workHandler_ = make_shared<FFRTHandler>("workHandler");
    }
    collectManager_ = sptr<DeviceStatusCollectManager>(new DeviceStatusCollectManager());
    abilityStateScheduler_ = std::make_shared<SystemAbilityStateScheduler>();
    InitSaProfile();
    reportEventTimer_ = std::make_unique<Utils::Timer>("DfxReporter");
    OndemandLoadForPerf();
    SetKey(DYNAMIC_CACHE_PARAM);
}

void SystemAbilityManager::RemoveWhiteCommonEvent()
{
    if (collectManager_ != nullptr) {
        collectManager_->RemoveWhiteCommonEvent();
    }
}

void SystemAbilityManager::CleanFfrt()
{
    if (workHandler_ != nullptr) {
        workHandler_->CleanFfrt();
    }
    if (collectManager_ != nullptr) {
        collectManager_->CleanFfrt();
    }
    if (abilityStateScheduler_ != nullptr) {
        abilityStateScheduler_->CleanFfrt();
    }
}

void SystemAbilityManager::SetFfrt()
{
    if (workHandler_ != nullptr) {
        workHandler_->SetFfrt("workHandler");
    }
    if (collectManager_ != nullptr) {
        collectManager_->SetFfrt();
    }
    if (abilityStateScheduler_ != nullptr) {
        abilityStateScheduler_->SetFfrt();
    }
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
    lock_guard<mutex> autoLock(systemProcessMapLock_);
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
    if ((argsWithStr8.size() > 0) && (argsWithStr8[FFRT_DUMP_INDEX] == ARGS_FFRT_PARAM)) {
        return SystemAbilityManagerDumper::FfrtDumpProc(abilityStateScheduler_, fd, argsWithStr8);
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
    unique_lock<shared_mutex> writeLock(abilityMapLock_);
    int32_t systemAbilityId = 0;
    SAInfo saInfo;
    saInfo.remoteObj = this;
    saInfo.isDistributed = false;
    saInfo.capability = u"";
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

sptr<SystemAbilityManager> SystemAbilityManager::GetInstance()
{
    std::lock_guard<std::mutex> autoLock(instanceLock);
    if (instance == nullptr) {
        instance = new SystemAbilityManager;
    }
    return instance;
}

void SystemAbilityManager::InitSaProfile()
{
    int64_t begin = GetTickCount();
    std::vector<std::string> fileNames;
    GetDirFiles(PREFIX, fileNames);
    auto parser = std::make_shared<ParseUtil>();
    for (const auto& file : fileNames) {
        if (file.empty() || file.find(".json") == std::string::npos ||
            file.find("_trust.json") != std::string::npos) {
            continue;
        }
        parser->ParseSaProfiles(file);
    }
    std::list<SaProfile> saInfos = parser->GetAllSaProfiles();
    if (abilityStateScheduler_ != nullptr) {
        abilityStateScheduler_->Init(saInfos);
    }
    if (collectManager_ != nullptr) {
        collectManager_->Init(saInfos);
    }
    lock_guard<mutex> autoLock(saProfileMapLock_);
    onDemandSaIdsSet_.insert(DEVICE_INFO_SERVICE_SA);
    onDemandSaIdsSet_.insert(HIDUMPER_SERVICE_SA);
    onDemandSaIdsSet_.insert(MEDIA_ANALYSIS_SERVICE_SA);
    for (const auto& saInfo : saInfos) {
        saProfileMap_[saInfo.saId] = saInfo;
        if (!saInfo.runOnCreate) {
            HILOGI("InitProfile saId %{public}d", saInfo.saId);
            onDemandSaIdsSet_.insert(saInfo.saId);
        }
    }
    HILOGI("InitProfile spend %{public}" PRId64 "ms", GetTickCount() - begin);
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

std::list<int32_t> SystemAbilityManager::GetAllOndemandSa()
{
    std::list<int32_t> ondemandSaids;
    {
        lock_guard<mutex> autoLock(saProfileMapLock_);
        for (const auto& [said, value] : saProfileMap_) {
            shared_lock<shared_mutex> readLock(abilityMapLock_);
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
    bool value = system::GetBoolParameter(ONDEMAND_PARAM, false);
    if (value) {
        std::list<int32_t> saids = GetAllOndemandSa();
        HILOGD("DoLoadForPerf ondemand size : %{public}zu.", saids.size());
        sptr<ISystemAbilityLoadCallback> callback(new SystemAbilityLoadCallbackStub());
        for (auto said : saids) {
            LoadSystemAbility(said, callback);
        }
    }
}

bool SystemAbilityManager::GetSaProfile(int32_t saId, SaProfile& saProfile)
{
    lock_guard<mutex> autoLock(saProfileMapLock_);
    auto iter = saProfileMap_.find(saId);
    if (iter == saProfileMap_.end()) {
        return false;
    } else {
        saProfile = iter->second;
    }
    return true;
}

int32_t SystemAbilityManager::GetOnDemandPolicy(int32_t systemAbilityId, OnDemandPolicyType type,
    std::vector<SystemAbilityOnDemandEvent>& abilityOnDemandEvents)
{
    SaProfile saProfile;
    if (!GetSaProfile(systemAbilityId, saProfile)) {
        HILOGE("GetOnDemandPolicy invalid SA:%{public}d", systemAbilityId);
        return ERR_INVALID_VALUE;
    }
    if (!SamgrUtil::CheckCallerProcess(saProfile)) {
        HILOGE("GetOnDemandPolicy invalid caller SA:%{public}d", systemAbilityId);
        return ERR_INVALID_VALUE;
    }
    if (!SamgrUtil::CheckAllowUpdate(type, saProfile)) {
        HILOGE("GetOnDemandPolicy not allow get SA:%{public}d", systemAbilityId);
        return ERR_PERMISSION_DENIED;
    }

    if (collectManager_ == nullptr) {
        HILOGE("GetOnDemandPolicy collectManager is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::vector<OnDemandEvent> onDemandEvents;
    int32_t result = ERR_INVALID_VALUE;
    result = collectManager_->GetOnDemandEvents(systemAbilityId, type, onDemandEvents);
    if (result != ERR_OK) {
        HILOGE("GetOnDemandPolicy add collect event failed");
        return result;
    }
    for (auto& item : onDemandEvents) {
        SystemAbilityOnDemandEvent eventOuter;
        SamgrUtil::ConvertToSystemAbilityOnDemandEvent(item, eventOuter);
        abilityOnDemandEvents.push_back(eventOuter);
    }
    HILOGI("GetOnDemandPolicy policy size : %{public}zu.", abilityOnDemandEvents.size());
    return ERR_OK;
}

int32_t SystemAbilityManager::UpdateOnDemandPolicy(int32_t systemAbilityId, OnDemandPolicyType type,
    const std::vector<SystemAbilityOnDemandEvent>& abilityOnDemandEvents)
{
    SaProfile saProfile;
    if (!GetSaProfile(systemAbilityId, saProfile)) {
        HILOGE("UpdateOnDemandPolicy invalid SA:%{public}d", systemAbilityId);
        return ERR_INVALID_VALUE;
    }
    if (!SamgrUtil::CheckCallerProcess(saProfile)) {
        HILOGE("UpdateOnDemandPolicy invalid caller SA:%{public}d", systemAbilityId);
        return ERR_INVALID_VALUE;
    }
    if (!SamgrUtil::CheckAllowUpdate(type, saProfile)) {
        HILOGE("UpdateOnDemandPolicy not allow get SA:%{public}d", systemAbilityId);
        return ERR_PERMISSION_DENIED;
    }

    if (collectManager_ == nullptr) {
        HILOGE("UpdateOnDemandPolicy collectManager is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::vector<OnDemandEvent> onDemandEvents;
    for (auto& item : abilityOnDemandEvents) {
        OnDemandEvent event;
        SamgrUtil::ConvertToOnDemandEvent(item, event);
        onDemandEvents.push_back(event);
    }
    int32_t result = ERR_INVALID_VALUE;
    result = collectManager_->UpdateOnDemandEvents(systemAbilityId, type, onDemandEvents);
    if (result != ERR_OK) {
        HILOGE("UpdateOnDemandPolicy add collect event failed");
        return result;
    }
    HILOGI("UpdateOnDemandPolicy policy size : %{public}zu.", onDemandEvents.size());
    return ERR_OK;
}

void SystemAbilityManager::ProcessOnDemandEvent(const OnDemandEvent& event,
    const std::list<SaControlInfo>& saControlList)
{
    HILOGI("DoEvent:%{public}d name:%{public}s value:%{public}s",
        event.eventId, event.name.c_str(), event.value.c_str());
    if (collectManager_ != nullptr) {
        collectManager_->SaveCacheCommonEventSaExtraId(event, saControlList);
    }
    if (abilityStateScheduler_ == nullptr) {
        HILOGE("abilityStateScheduler is nullptr");
        return;
    }
    abilityStateScheduler_->CheckEnableOnce(event, saControlList);
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
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("GetSystemAbilityFromRemote invalid!");
        return nullptr;
    }

    shared_lock<shared_mutex> readLock(abilityMapLock_);
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
    HILOGD("%{public}s called, SA:%{public}d", __func__, systemAbilityId);

    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("CheckSystemAbility CheckSystemAbility invalid!");
        return nullptr;
    }
    int32_t count = UpdateSaFreMap(IPCSkeleton::GetCallingUid(), systemAbilityId);
    shared_lock<shared_mutex> readLock(abilityMapLock_);
    auto iter = abilityMap_.find(systemAbilityId);
    if (iter != abilityMap_.end()) {
        HILOGD("found SA:%{public}d,callpid:%{public}d", systemAbilityId, IPCSkeleton::GetCallingPid());
        return iter->second.remoteObj;
    }
    HILOGI("NF SA:%{public}d,pid:%{public}d,n:%{public}d",
        systemAbilityId, IPCSkeleton::GetCallingPid(), count);
    return nullptr;
}

sptr<IRemoteObject> SystemAbilityManager::CheckSystemAbility(int32_t systemAbilityId,
    const std::string& deviceId)
{
    SaProfile saProfile;
    bool ret = GetSaProfile(systemAbilityId, saProfile);
    if (!ret) {
        HILOGE("CheckSystemAbilityFromRpc SA:%{public}d not supported!", systemAbilityId);
        return nullptr;
    }
    if (!saProfile.distributed) {
        HILOGE("CheckSystemAbilityFromRpc SA:%{public}d not distributed!", systemAbilityId);
        return nullptr;
    }
    return DoMakeRemoteBinder(systemAbilityId, IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid(), deviceId);
}

int32_t SystemAbilityManager::FindSystemAbilityNotify(int32_t systemAbilityId, int32_t code)
{
    return FindSystemAbilityNotify(systemAbilityId, "", code);
}

void SystemAbilityManager::NotifySystemAbilityChanged(int32_t systemAbilityId, const std::string& deviceId,
    int32_t code, const sptr<ISystemAbilityStatusChange>& listener)
{
    HILOGD("NotifySystemAbilityChanged, SA:%{public}d", systemAbilityId);
    if (listener == nullptr) {
        HILOGE("%{public}s listener null pointer!", __func__);
        return;
    }

    switch (code) {
        case static_cast<uint32_t>(SamgrInterfaceCode::ADD_SYSTEM_ABILITY_TRANSACTION): {
            listener->OnAddSystemAbility(systemAbilityId, deviceId);
            break;
        }
        case static_cast<uint32_t>(SamgrInterfaceCode::REMOVE_SYSTEM_ABILITY_TRANSACTION): {
            listener->OnRemoveSystemAbility(systemAbilityId, deviceId);
            break;
        }
        default:
            break;
    }
}

int32_t SystemAbilityManager::FindSystemAbilityNotify(int32_t systemAbilityId, const std::string& deviceId,
    int32_t code)
{
    HILOGI("FindSaNotify called:SA:%{public}d,code:%{public}d,size:%{public}zu",
        systemAbilityId, code, listenerMap_.size());
    lock_guard<mutex> autoLock(listenerMapLock_);
    auto iter = listenerMap_.find(systemAbilityId);
    if (iter == listenerMap_.end()) {
        return ERR_OK;
    }
    auto& listeners = iter->second;
    if (code == static_cast<int32_t>(SamgrInterfaceCode::ADD_SYSTEM_ABILITY_TRANSACTION)) {
        for (auto& item : listeners) {
            if (item.state == ListenerState::INIT) {
                NotifySystemAbilityChanged(systemAbilityId, deviceId, code, item.listener);
                item.state = ListenerState::NOTIFIED;
            } else {
                HILOGI("FindSaNotify Listener has been notified,SA:%{public}d,callingPid:%{public}d",
                    systemAbilityId, item.callingPid);
            }
        }
    } else if (code == static_cast<int32_t>(SamgrInterfaceCode::REMOVE_SYSTEM_ABILITY_TRANSACTION)) {
        for (auto& item : listeners) {
            NotifySystemAbilityChanged(systemAbilityId, deviceId, code, item.listener);
            item.state = ListenerState::INIT;
        }
    }
    return ERR_OK;
}

void SystemAbilityManager::StartOnDemandAbility(const std::u16string& procName, int32_t systemAbilityId)
{
    lock_guard<mutex> autoLock(onDemandLock_);
    StartOnDemandAbilityLocked(procName, systemAbilityId);
}

void SystemAbilityManager::StartOnDemandAbilityLocked(const std::u16string& procName, int32_t systemAbilityId)
{
    auto iter = startingAbilityMap_.find(systemAbilityId);
    if (iter == startingAbilityMap_.end()) {
        return;
    }
    auto& abilityItem = iter->second;
    StartOnDemandAbilityInner(procName, systemAbilityId, abilityItem);
}

int32_t SystemAbilityManager::StartOnDemandAbilityInner(const std::u16string& procName, int32_t systemAbilityId,
    AbilityItem& abilityItem)
{
    if (abilityItem.state != AbilityState::INIT) {
        HILOGW("StartSaInner SA:%{public}d,state:%{public}d,proc:%{public}s",
            systemAbilityId, abilityItem.state, Str16ToStr8(procName).c_str());
        return ERR_INVALID_VALUE;
    }
    sptr<ILocalAbilityManager> procObject =
        iface_cast<ILocalAbilityManager>(GetSystemProcess(procName));
    if (procObject == nullptr) {
        HILOGI("get process:%{public}s fail", Str16ToStr8(procName).c_str());
        return ERR_INVALID_VALUE;
    }
    auto event = abilityItem.event;
    auto eventStr = SamgrUtil::EventToStr(event);
    HILOGI("StartSA:%{public}d", systemAbilityId);
    procObject->StartAbility(systemAbilityId, eventStr);
    abilityItem.state = AbilityState::STARTING;
    return ERR_OK;
}

bool SystemAbilityManager::StopOnDemandAbility(const std::u16string& procName,
    int32_t systemAbilityId, const OnDemandEvent& event)
{
    lock_guard<mutex> autoLock(onDemandLock_);
    return StopOnDemandAbilityInner(procName, systemAbilityId, event);
}

bool SystemAbilityManager::StopOnDemandAbilityInner(const std::u16string& procName,
    int32_t systemAbilityId, const OnDemandEvent& event)
{
    sptr<ILocalAbilityManager> procObject =
        iface_cast<ILocalAbilityManager>(GetSystemProcess(procName));
    if (procObject == nullptr) {
        HILOGI("get process:%{public}s fail", Str16ToStr8(procName).c_str());
        return false;
    }
    auto eventStr = SamgrUtil::EventToStr(event);
    HILOGI("StopSA:%{public}d", systemAbilityId);
    return procObject->StopAbility(systemAbilityId, eventStr);
}

int32_t SystemAbilityManager::AddOnDemandSystemAbilityInfo(int32_t systemAbilityId,
    const std::u16string& procName)
{
    HILOGD("%{public}s called", __func__);
    if (!CheckInputSysAbilityId(systemAbilityId) || SamgrUtil::IsNameInValid(procName)) {
        HILOGW("AddOnDemandSystemAbilityInfo SAId or procName invalid.");
        return ERR_INVALID_VALUE;
    }

    lock_guard<mutex> autoLock(onDemandLock_);
    auto onDemandSaSize = onDemandAbilityMap_.size();
    if (onDemandSaSize >= MAX_SERVICES) {
        HILOGE("map size error, (Has been greater than %{public}zu)",
            onDemandAbilityMap_.size());
        return ERR_INVALID_VALUE;
    }
    {
        lock_guard<mutex> autoLock(systemProcessMapLock_);
        if (systemProcessMap_.count(procName) == 0) {
            HILOGW("AddOnDemandSystemAbilityInfo procName:%{public}s not exist.", Str16ToStr8(procName).c_str());
            return ERR_INVALID_VALUE;
        }
    }
    onDemandAbilityMap_[systemAbilityId] = procName;
    HILOGI("insert onDemand SA:%{public}d. size:%{public}zu", systemAbilityId,
        onDemandAbilityMap_.size());
    if (startingAbilityMap_.count(systemAbilityId) != 0) {
        if (workHandler_ != nullptr) {
            auto pendingTask = [procName, systemAbilityId, this] () {
                StartOnDemandAbility(procName, systemAbilityId);
            };
            bool ret = workHandler_->PostTask(pendingTask);
            if (!ret) {
                HILOGW("AddOnDemandSystemAbilityInfo PostTask failed!");
            }
        }
    }
    return ERR_OK;
}

int32_t SystemAbilityManager::StartOnDemandAbility(int32_t systemAbilityId, bool& isExist)
{
    lock_guard<mutex> onDemandAbilityLock(onDemandLock_);
    return StartOnDemandAbilityLocked(systemAbilityId, isExist);
}

int32_t SystemAbilityManager::StartOnDemandAbilityLocked(int32_t systemAbilityId, bool& isExist)
{
    auto iter = onDemandAbilityMap_.find(systemAbilityId);
    if (iter == onDemandAbilityMap_.end()) {
        isExist = false;
        return ERR_INVALID_VALUE;
    }
    HILOGI("found onDemand SA:%{public}d.", systemAbilityId);
    isExist = true;
    AbilityItem& abilityItem = startingAbilityMap_[systemAbilityId];
    return StartOnDemandAbilityInner(iter->second, systemAbilityId, abilityItem);
}

sptr<IRemoteObject> SystemAbilityManager::CheckSystemAbility(int32_t systemAbilityId, bool& isExist)
{
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        return nullptr;
    }
    if (abilityStateScheduler_ == nullptr) {
        HILOGE("abilityStateScheduler is nullptr");
        return nullptr;
    }
    if (abilityStateScheduler_->IsSystemAbilityUnloading(systemAbilityId)) {
        HILOGW("SA:%{public}d is unloading", systemAbilityId);
        return nullptr;
    }
    sptr<IRemoteObject> abilityProxy = CheckSystemAbility(systemAbilityId);
    if (abilityProxy == nullptr) {
        abilityStateScheduler_->HandleLoadAbilityEvent(systemAbilityId, isExist);
        return nullptr;
    }
    isExist = true;
    return abilityProxy;
}

bool SystemAbilityManager::DoLoadOnDemandAbility(int32_t systemAbilityId, bool& isExist)
{
    lock_guard<mutex> autoLock(onDemandLock_);
    sptr<IRemoteObject> abilityProxy = CheckSystemAbility(systemAbilityId);
    if (abilityProxy != nullptr) {
        isExist = true;
        return true;
    }
    auto iter = startingAbilityMap_.find(systemAbilityId);
    if (iter != startingAbilityMap_.end() && iter->second.state == AbilityState::STARTING) {
        isExist = true;
        return true;
    }
    auto onDemandIter = onDemandAbilityMap_.find(systemAbilityId);
    if (onDemandIter == onDemandAbilityMap_.end()) {
        isExist = false;
        return false;
    }
    auto& abilityItem = startingAbilityMap_[systemAbilityId];
    abilityItem.event = {INTERFACE_CALL, "get", ""};
    return StartOnDemandAbilityLocked(systemAbilityId, isExist) == ERR_OK;
}

int32_t SystemAbilityManager::RemoveSystemAbility(int32_t systemAbilityId)
{
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("RemoveSystemAbility SA:%{public}d", systemAbilityId);
        return ERR_INVALID_VALUE;
    }
    {
        unique_lock<shared_mutex> writeLock(abilityMapLock_);
        auto itSystemAbility = abilityMap_.find(systemAbilityId);
        if (itSystemAbility == abilityMap_.end()) {
            HILOGI("RemoveSystemAbility not found!");
            return ERR_INVALID_VALUE;
        }
        sptr<IRemoteObject> ability = itSystemAbility->second.remoteObj;
        if (ability != nullptr && abilityDeath_ != nullptr) {
            ability->RemoveDeathRecipient(abilityDeath_);
        }
        (void)abilityMap_.erase(itSystemAbility);
        KHILOGI("%{public}s called, SA:%{public}d, size:%{public}zu", __func__, systemAbilityId, abilityMap_.size());
    }
    if (abilityStateScheduler_ == nullptr) {
        HILOGE("abilityStateScheduler is nullptr");
        return ERR_INVALID_VALUE;
    }
    SystemAbilityInvalidateCache(systemAbilityId);
    abilityStateScheduler_->SendAbilityStateEvent(systemAbilityId, AbilityStateEvent::ABILITY_UNLOAD_SUCCESS_EVENT);
    SendSystemAbilityRemovedMsg(systemAbilityId);
    if (IsCacheCommonEvent(systemAbilityId) && collectManager_ != nullptr) {
        collectManager_->ClearSaExtraDataId(systemAbilityId);
    }
    return ERR_OK;
}

int32_t SystemAbilityManager::RemoveSystemAbility(const sptr<IRemoteObject>& ability)
{
    HILOGD("%{public}s called, (ability)", __func__);
    if (ability == nullptr) {
        HILOGW("ability is nullptr ");
        return ERR_INVALID_VALUE;
    }

    int32_t saId = 0;
    {
        unique_lock<shared_mutex> writeLock(abilityMapLock_);
        for (auto iter = abilityMap_.begin(); iter != abilityMap_.end(); ++iter) {
            if (iter->second.remoteObj == ability) {
                saId = iter->first;
                (void)abilityMap_.erase(iter);
                if (abilityDeath_ != nullptr) {
                    ability->RemoveDeathRecipient(abilityDeath_);
                }
                KHILOGI("%{public}s called, SA:%{public}d removed, size:%{public}zu", __func__, saId,
                    abilityMap_.size());
                break;
            }
        }
    }

    if (saId != 0) {
        SystemAbilityInvalidateCache(saId);
        if (IsCacheCommonEvent(saId) && collectManager_ != nullptr) {
            collectManager_->ClearSaExtraDataId(saId);
        }
        ReportSaCrash(saId);
        if (abilityStateScheduler_ == nullptr) {
            HILOGE("abilityStateScheduler is nullptr");
            return ERR_INVALID_VALUE;
        }
        abilityStateScheduler_->HandleAbilityDiedEvent(saId);
        SendSystemAbilityRemovedMsg(saId);
    }
    return ERR_OK;
}

int32_t SystemAbilityManager::RemoveDiedSystemAbility(int32_t systemAbilityId)
{
    {
        unique_lock<shared_mutex> writeLock(abilityMapLock_);
        auto itSystemAbility = abilityMap_.find(systemAbilityId);
        if (itSystemAbility == abilityMap_.end()) {
            return ERR_OK;
        }
        sptr<IRemoteObject> ability = itSystemAbility->second.remoteObj;
        if (ability != nullptr && abilityDeath_ != nullptr) {
            ability->RemoveDeathRecipient(abilityDeath_);
        }
        (void)abilityMap_.erase(itSystemAbility);
        ReportSaCrash(systemAbilityId);
        KHILOGI("%{public}s called, SA:%{public}d removed, size:%{public}zu", __func__, systemAbilityId,
            abilityMap_.size());
    }
    SendSystemAbilityRemovedMsg(systemAbilityId);
    return ERR_OK;
}

vector<u16string> SystemAbilityManager::ListSystemAbilities(uint32_t dumpFlags)
{
    vector<u16string> list;
    shared_lock<shared_mutex> readLock(abilityMapLock_);
    for (auto iter = abilityMap_.begin(); iter != abilityMap_.end(); iter++) {
        list.emplace_back(Str8ToStr16(to_string(iter->first)));
    }
    return list;
}

void SystemAbilityManager::CheckListenerNotify(int32_t systemAbilityId,
    const sptr<ISystemAbilityStatusChange>& listener)
{
    sptr<IRemoteObject> targetObject = CheckSystemAbility(systemAbilityId);
    if (targetObject == nullptr) {
        return;
    }
    lock_guard<mutex> autoLock(listenerMapLock_);
    auto& listeners = listenerMap_[systemAbilityId];
    for (auto& itemListener : listeners) {
        if (listener->AsObject() == itemListener.listener->AsObject()) {
            int32_t callingPid = itemListener.callingPid;
            if (itemListener.state == ListenerState::INIT) {
                HILOGI("NotifySaChanged add SA:%{public}d,cnt:%{public}d,callpid:%{public}d",
                    systemAbilityId, subscribeCountMap_[callingPid], callingPid);
                NotifySystemAbilityChanged(systemAbilityId, "",
                    static_cast<uint32_t>(SamgrInterfaceCode::ADD_SYSTEM_ABILITY_TRANSACTION), listener);
                itemListener.state = ListenerState::NOTIFIED;
            } else {
                HILOGI("Subscribe Listener has been notified,SA:%{public}d,callpid:%{public}d",
                    systemAbilityId, callingPid);
            }
            break;
        }
    }
}

int32_t SystemAbilityManager::SubscribeSystemAbility(int32_t systemAbilityId,
    const sptr<ISystemAbilityStatusChange>& listener)
{
    if (!CheckInputSysAbilityId(systemAbilityId) || listener == nullptr) {
        HILOGW("SubscribeSystemAbility SAId or listener invalid!");
        return ERR_INVALID_VALUE;
    }

    auto callingPid = IPCSkeleton::GetCallingPid();
    {
        lock_guard<mutex> autoLock(listenerMapLock_);
        auto& listeners = listenerMap_[systemAbilityId];
        for (const auto& itemListener : listeners) {
            if (listener->AsObject() == itemListener.listener->AsObject()) {
                HILOGI("already exist listener object SA:%{public}d", systemAbilityId);
                return ERR_OK;
            }
        }
        auto& count = subscribeCountMap_[callingPid];
        if (count >= MAX_SUBSCRIBE_COUNT) {
            HILOGE("SubscribeSystemAbility pid:%{public}d overflow max subscribe count!", callingPid);
            return ERR_PERMISSION_DENIED;
        }
        ++count;
        if (abilityStatusDeath_ != nullptr) {
            bool ret = listener->AsObject()->AddDeathRecipient(abilityStatusDeath_);
            listeners.emplace_back(listener, callingPid);
            if (!ret) {
                HILOGW("Subscribe SA:%{public}d AddDeath fail,cnt:%{public}d,"
                    "callpid:%{public}d", systemAbilityId, count, callingPid);
            }
        }
        HILOGI("Subscribe SA:%{public}d,size:%{public}zu,cnt:%{public}d,callpid:%{public}d",
            systemAbilityId, listeners.size(), count, callingPid);
    }
    CheckListenerNotify(systemAbilityId, listener);
    return ERR_OK;
}

void SystemAbilityManager::UnSubscribeSystemAbilityLocked(
    std::list<SAListener>& listenerList, const sptr<IRemoteObject>& listener)
{
    auto item = listenerList.begin();
    for (; item != listenerList.end(); item++) {
        if (item->listener == nullptr) {
            HILOGE("listener is null");
            return;
        }
        if (item->listener->AsObject() == listener) {
            break;
        }
    }
    if (item == listenerList.end()) {
        return;
    }
    int32_t callpid = item->callingPid;
    auto iterPair = subscribeCountMap_.find(callpid);
    if (iterPair != subscribeCountMap_.end()) {
        --(iterPair->second);
        if (iterPair->second == 0) {
            subscribeCountMap_.erase(iterPair);
        }
    }
    listenerList.erase(item);
    HILOGI("rm SA listener callPid:%{public}d,size:%{public}zu", callpid, listenerList.size());
}

int32_t SystemAbilityManager::UnSubscribeSystemAbility(int32_t systemAbilityId,
    const sptr<ISystemAbilityStatusChange>& listener)
{
    if (!CheckInputSysAbilityId(systemAbilityId) || listener == nullptr) {
        HILOGW("UnSubscribeSa saId or listener invalid");
        return ERR_INVALID_VALUE;
    }

    lock_guard<mutex> autoLock(listenerMapLock_);
    auto& listeners = listenerMap_[systemAbilityId];
    UnSubscribeSystemAbilityLocked(listeners, listener->AsObject());
    if (abilityStatusDeath_ != nullptr) {
        listener->AsObject()->RemoveDeathRecipient(abilityStatusDeath_);
    }
    HILOGI("UnSubscribeSa:%{public}d,size:%{public}zu", systemAbilityId, listeners.size());
    return ERR_OK;
}

void SystemAbilityManager::UnSubscribeSystemAbility(const sptr<IRemoteObject>& remoteObject)
{
    lock_guard<mutex> autoLock(listenerMapLock_);
    HILOGD("UnSubscribeSa remote object dead! size:%{public}zu", listenerMap_.size());
    for (auto& item : listenerMap_) {
        auto& listeners = item.second;
        UnSubscribeSystemAbilityLocked(listeners, remoteObject);
    }
    if (abilityStatusDeath_ != nullptr) {
        remoteObject->RemoveDeathRecipient(abilityStatusDeath_);
    }
}

void SystemAbilityManager::NotifyRemoteSaDied(const std::u16string& name)
{
    std::u16string saName;
    std::string deviceId;
    SamgrUtil::ParseRemoteSaName(name, deviceId, saName);
    if (dBinderService_ != nullptr) {
        std::string nodeId = SamgrUtil::TransformDeviceId(deviceId, NODE_ID, false);
        dBinderService_->NoticeServiceDie(saName, nodeId);
        HILOGI("NotifyRemoteSaDied, serviceName:%{public}s, deviceId:%{public}s",
            Str16ToStr8(saName).c_str(), AnonymizeDeviceId(nodeId).c_str());
    }
}

void SystemAbilityManager::NotifyRemoteDeviceOffline(const std::string& deviceId)
{
    if (dBinderService_ != nullptr) {
        dBinderService_->NoticeDeviceDie(deviceId);
        HILOGI("NotifyRemoteDeviceOffline, deviceId:%{public}s", AnonymizeDeviceId(deviceId).c_str());
    }
}

void SystemAbilityManager::RefreshListenerState(int32_t systemAbilityId)
{
    lock_guard<mutex> autoLock(listenerMapLock_);
    auto iter = listenerMap_.find(systemAbilityId);
    if (iter != listenerMap_.end()) {
        auto& listeners = iter->second;
        for (auto& item : listeners) {
            item.state = ListenerState::INIT;
        }
    }
}

int32_t SystemAbilityManager::AddSystemAbility(int32_t systemAbilityId, const sptr<IRemoteObject>& ability,
    const SAExtraProp& extraProp)
{
    if (!CheckInputSysAbilityId(systemAbilityId) || ability == nullptr) {
        HILOGE("AddSystemAbilityExtra input params is invalid.");
        return ERR_INVALID_VALUE;
    }
    RefreshListenerState(systemAbilityId);
    {
        unique_lock<shared_mutex> writeLock(abilityMapLock_);
        auto saSize = abilityMap_.size();
        if (saSize >= MAX_SERVICES) {
            HILOGE("map size error, (Has been greater than %zu)", saSize);
            return ERR_INVALID_VALUE;
        }
        SAInfo saInfo = { ability, extraProp.isDistributed, extraProp.capability, Str16ToStr8(extraProp.permission) };
        if (abilityMap_.count(systemAbilityId) > 0) {
            auto callingPid = IPCSkeleton::GetCallingPid();
            auto callingUid = IPCSkeleton::GetCallingUid();
            HILOGW("SA:%{public}d is being covered, callPid:%{public}d, callUid:%{public}d",
                systemAbilityId, callingPid, callingUid);
        }
        abilityMap_[systemAbilityId] = std::move(saInfo);
        KHILOGI("insert %{public}d. size:%{public}zu", systemAbilityId, abilityMap_.size());
    }
    RemoveCheckLoadedMsg(systemAbilityId);
    if (abilityDeath_ != nullptr) {
        ability->AddDeathRecipient(abilityDeath_);
    }
    u16string strName = Str8ToStr16(to_string(systemAbilityId));
    if (extraProp.isDistributed && dBinderService_ != nullptr) {
        dBinderService_->RegisterRemoteProxy(strName, systemAbilityId);
        HILOGI("AddSystemAbility RegisterRemoteProxy, SA:%{public}d", systemAbilityId);
    }
    if (systemAbilityId == SOFTBUS_SERVER_SA_ID) {
        if (dBinderService_ != nullptr && rpcCallbackImp_ != nullptr) {
            bool ret = dBinderService_->StartDBinderService(rpcCallbackImp_);
            HILOGI("start result is %{public}s", ret ? "succeed" : "fail");
        }
    }
    if (abilityStateScheduler_ == nullptr) {
        HILOGE("abilityStateScheduler is nullptr");
        return ERR_INVALID_VALUE;
    }
    abilityStateScheduler_->UpdateLimitDelayUnloadTime(systemAbilityId);
    SystemAbilityInvalidateCache(systemAbilityId);
    abilityStateScheduler_->SendAbilityStateEvent(systemAbilityId, AbilityStateEvent::ABILITY_LOAD_SUCCESS_EVENT);
    SendSystemAbilityAddedMsg(systemAbilityId, ability);
    return ERR_OK;
}

void SystemAbilityManager::SystemAbilityInvalidateCache(int32_t systemAbilityId)
{
    auto pos = onDemandSaIdsSet_.find(systemAbilityId);
    if (pos != onDemandSaIdsSet_.end()) {
        HILOGD("SystemAbilityInvalidateCache SA:%{public}d.", systemAbilityId);
        return;
    }
    SamgrUtil::InvalidateSACache();
}

int32_t SystemAbilityManager::AddSystemProcess(const u16string& procName,
    const sptr<IRemoteObject>& procObject)
{
    if (procName.empty() || procObject == nullptr) {
        HILOGE("AddSystemProcess empty name or null object!");
        return ERR_INVALID_VALUE;
    }
    {
        lock_guard<mutex> autoLock(systemProcessMapLock_);
        size_t procNum = systemProcessMap_.size();
        if (procNum >= MAX_SERVICES) {
            HILOGE("AddSystemProcess map size reach MAX_SERVICES already");
            return ERR_INVALID_VALUE;
        }
        systemProcessMap_[procName] = procObject;
        HILOGI("AddProc:%{public}s. size:%{public}zu", Str16ToStr8(procName).c_str(),
            systemProcessMap_.size());
    }
    if (systemProcessDeath_ != nullptr) {
        bool ret = procObject->AddDeathRecipient(systemProcessDeath_);
        if (!ret) {
            HILOGW("AddProc:%{public}s,size:%{public}zu,AddDeath fail",
                Str16ToStr8(procName).c_str(), systemProcessMap_.size());
        }
    }
    {
        lock_guard<mutex> autoLock(startingProcessMapLock_);
        auto iterStarting = startingProcessMap_.find(procName);
        if (iterStarting != startingProcessMap_.end()) {
            int64_t end = GetTickCount();
            HILOGI("AddProc start proc:%{public}s spend %{public}" PRId64 "ms",
                Str16ToStr8(procName).c_str(), (end - iterStarting->second));
            startingProcessMap_.erase(iterStarting);
        }
    }
    if (abilityStateScheduler_ == nullptr) {
        HILOGE("abilityStateScheduler is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto callingPid = IPCSkeleton::GetCallingPid();
    auto callingUid = IPCSkeleton::GetCallingUid();
    ProcessInfo processInfo = {procName, callingPid, callingUid};
    abilityStateScheduler_->SendProcessStateEvent(processInfo, ProcessStateEvent::PROCESS_STARTED_EVENT);
    return ERR_OK;
}

int32_t SystemAbilityManager::RemoveSystemProcess(const sptr<IRemoteObject>& procObject)
{
    if (procObject == nullptr) {
        HILOGW("RemoveSystemProcess null object!");
        return ERR_INVALID_VALUE;
    }

    int32_t result = ERR_INVALID_VALUE;
    std::u16string processName;
    if (systemProcessDeath_ != nullptr) {
        procObject->RemoveDeathRecipient(systemProcessDeath_);
    }
    {
        lock_guard<mutex> autoLock(systemProcessMapLock_);
        for (const auto& [procName, object] : systemProcessMap_) {
            if (object != procObject) {
                continue;
            }
            std::string name = Str16ToStr8(procName);
            processName = procName;
            (void)systemProcessMap_.erase(procName);
            HILOGI("RemoveProc:%{public}s dead, size:%{public}zu", name.c_str(),
                systemProcessMap_.size());
            result = ERR_OK;
            break;
        }
    }
    if (result == ERR_OK) {
        if (abilityStateScheduler_ == nullptr) {
            HILOGE("abilityStateScheduler is nullptr");
            return ERR_INVALID_VALUE;
        }
        ProcessInfo processInfo = {processName};
        abilityStateScheduler_->SendProcessStateEvent(processInfo, ProcessStateEvent::PROCESS_STOPPED_EVENT);
    } else {
        HILOGW("RemoveSystemProcess called and not found process.");
    }
    return result;
}

sptr<IRemoteObject> SystemAbilityManager::GetSystemProcess(const u16string& procName)
{
    if (procName.empty()) {
        HILOGE("GetSystemProcess empty name!");
        return nullptr;
    }

    lock_guard<mutex> autoLock(systemProcessMapLock_);
    auto iter = systemProcessMap_.find(procName);
    if (iter != systemProcessMap_.end()) {
        HILOGD("process:%{public}s found", Str16ToStr8(procName).c_str());
        return iter->second;
    }
    HILOGE("process:%{public}s not exist", Str16ToStr8(procName).c_str());
    return nullptr;
}

int32_t SystemAbilityManager::GetSystemProcessInfo(int32_t systemAbilityId, SystemProcessInfo& systemProcessInfo)
{
    if (abilityStateScheduler_ == nullptr) {
        HILOGE("abilityStateScheduler is nullptr");
        return ERR_INVALID_VALUE;
    }
    return abilityStateScheduler_->GetSystemProcessInfo(systemAbilityId, systemProcessInfo);
}

bool SystemAbilityManager::IsDistributedSystemAbility(int32_t systemAbilityId)
{
    SaProfile saProfile;
    bool ret = GetSaProfile(systemAbilityId, saProfile);
    if (!ret) {
        HILOGE("IsDistributedSa SA:%{public}d no Profile!", systemAbilityId);
        return false;
    }
    return saProfile.distributed;
}

int32_t SystemAbilityManager::GetRunningSystemProcess(std::list<SystemProcessInfo>& systemProcessInfos)
{
    if (abilityStateScheduler_ == nullptr) {
        HILOGE("abilityStateScheduler is nullptr");
        return ERR_INVALID_VALUE;
    }
    return abilityStateScheduler_->GetRunningSystemProcess(systemProcessInfos);
}

int32_t SystemAbilityManager::SubscribeSystemProcess(const sptr<ISystemProcessStatusChange>& listener)
{
    if (abilityStateScheduler_ == nullptr) {
        HILOGE("abilityStateScheduler is nullptr");
        return ERR_INVALID_VALUE;
    }
    return abilityStateScheduler_->SubscribeSystemProcess(listener);
}

int32_t SystemAbilityManager::UnSubscribeSystemProcess(const sptr<ISystemProcessStatusChange>& listener)
{
    if (abilityStateScheduler_ == nullptr) {
        HILOGE("abilityStateScheduler is nullptr");
        return ERR_INVALID_VALUE;
    }
    return abilityStateScheduler_->UnSubscribeSystemProcess(listener);
}

int32_t SystemAbilityManager::GetOnDemandReasonExtraData(int64_t extraDataId, MessageParcel& extraDataParcel)
{
    if (collectManager_ == nullptr) {
        HILOGE("collectManager is nullptr");
        return ERR_INVALID_VALUE;
    }
    OnDemandReasonExtraData extraData;
    if (collectManager_->GetOnDemandReasonExtraData(extraDataId, extraData) != ERR_OK) {
        HILOGE("get extra data failed");
        return ERR_INVALID_VALUE;
    }
    if (!extraDataParcel.WriteParcelable(&extraData)) {
        HILOGE("write extra data failed");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

void SystemAbilityManager::SendSystemAbilityAddedMsg(int32_t systemAbilityId, const sptr<IRemoteObject>& remoteObject)
{
    if (workHandler_ == nullptr) {
        HILOGE("SendSaAddedMsg work handler not init");
        return;
    }
    auto notifyAddedTask = [systemAbilityId, remoteObject, this]() {
        FindSystemAbilityNotify(systemAbilityId,
            static_cast<uint32_t>(SamgrInterfaceCode::ADD_SYSTEM_ABILITY_TRANSACTION));
        HILOGI("SendSaAddedMsg notify SA:%{public}d", systemAbilityId);
        NotifySystemAbilityLoaded(systemAbilityId, remoteObject);
    };
    bool ret = workHandler_->PostTask(notifyAddedTask);
    if (!ret) {
        HILOGW("SendSaAddedMsg PostTask fail");
    }
}

void SystemAbilityManager::SendSystemAbilityRemovedMsg(int32_t systemAbilityId)
{
    if (workHandler_ == nullptr) {
        HILOGE("SendSaRemovedMsg work handler not init");
        return;
    }
    auto notifyRemovedTask = [systemAbilityId, this]() {
        FindSystemAbilityNotify(systemAbilityId,
            static_cast<uint32_t>(SamgrInterfaceCode::REMOVE_SYSTEM_ABILITY_TRANSACTION));
    };
    bool ret = workHandler_->PostTask(notifyRemovedTask);
    if (!ret) {
        HILOGW("SendSaRemovedMsg PostTask fail");
    }
}

bool SystemAbilityManager::IsModuleUpdate(int32_t systemAbilityId)
{
    SaProfile saProfile;
    bool ret = GetSaProfile(systemAbilityId, saProfile);
    if (!ret) {
        HILOGE("IsModuleUpdate SA:%{public}d not exist!", systemAbilityId);
        return false;
    }
    return saProfile.moduleUpdate;
}

void SystemAbilityManager::SendCheckLoadedMsg(int32_t systemAbilityId, const std::u16string& name,
    const std::string& srcDeviceId, const sptr<ISystemAbilityLoadCallback>& callback)
{
    if (workHandler_ == nullptr) {
        HILOGE("SendCheckLoadedMsg work handler not initialized!");
        return;
    }

    auto delayTask = [systemAbilityId, name, srcDeviceId, callback, this]() {
        if (workHandler_ != nullptr) {
            HILOGD("SendCheckLoadedMsg deltask SA:%{public}d", systemAbilityId);
            workHandler_->DelTask(ToString(systemAbilityId));
        } else {
            HILOGE("SendCheckLoadedMsg workHandler_ is null");
        }
        if (CheckSystemAbility(systemAbilityId) != nullptr) {
            HILOGI("SendCheckLoadedMsg SA:%{public}d loaded.", systemAbilityId);
            return;
        }
        HILOGI("SendCheckLoadedMsg handle for SA:%{public}d.", systemAbilityId);
        CleanCallbackForLoadFailed(systemAbilityId, name, srcDeviceId, callback);
        if (abilityStateScheduler_ == nullptr) {
            HILOGE("abilityStateScheduler is nullptr");
            return;
        }
        HILOGI("SendCheckLoadedMsg SA:%{public}d, load timeout.", systemAbilityId);
        ReportSamgrSaLoadFail(systemAbilityId, IPCSkeleton::GetCallingPid(),
            IPCSkeleton::GetCallingUid(), "time out");
        SamgrUtil::SendUpdateSaState(systemAbilityId, "loadfail");
        if (IsCacheCommonEvent(systemAbilityId) && collectManager_ != nullptr) {
            collectManager_->ClearSaExtraDataId(systemAbilityId);
        }
        abilityStateScheduler_->SendAbilityStateEvent(systemAbilityId, AbilityStateEvent::ABILITY_LOAD_FAILED_EVENT);
        (void)GetSystemProcess(name);
    };
    bool ret = workHandler_->PostTask(delayTask, ToString(systemAbilityId), CHECK_LOADED_DELAY_TIME);
    if (!ret) {
        HILOGI("SendCheckLoadedMsg PostTask SA:%{public}d! failed", systemAbilityId);
    }
}

void SystemAbilityManager::CleanCallbackForLoadFailed(int32_t systemAbilityId, const std::u16string& name,
    const std::string& srcDeviceId, const sptr<ISystemAbilityLoadCallback>& callback)
{
    {
        lock_guard<mutex> autoLock(startingProcessMapLock_);
        auto iterStarting = startingProcessMap_.find(name);
        if (iterStarting != startingProcessMap_.end()) {
            HILOGI("CleanCallback clean process:%{public}s", Str16ToStr8(name).c_str());
            startingProcessMap_.erase(iterStarting);
        }
    }
    lock_guard<mutex> autoLock(onDemandLock_);
    auto iter = startingAbilityMap_.find(systemAbilityId);
    if (iter == startingAbilityMap_.end()) {
        HILOGI("CleanCallback SA:%{public}d not in startingAbilityMap.", systemAbilityId);
        return;
    }
    auto& abilityItem = iter->second;
    for (auto& callbackItem : abilityItem.callbackMap[srcDeviceId]) {
        if (callback->AsObject() == callbackItem.first->AsObject()) {
            NotifySystemAbilityLoadFail(systemAbilityId, callbackItem.first);
            RemoveStartingAbilityCallbackLocked(callbackItem);
            abilityItem.callbackMap[srcDeviceId].remove(callbackItem);
            break;
        }
    }
    if (abilityItem.callbackMap[srcDeviceId].empty()) {
        HILOGI("CleanCallback startingAbilityMap remove SA:%{public}d. with deviceId", systemAbilityId);
        abilityItem.callbackMap.erase(srcDeviceId);
    }

    if (abilityItem.callbackMap.empty()) {
        HILOGI("CleanCallback startingAbilityMap remove SA:%{public}d.", systemAbilityId);
        startingAbilityMap_.erase(iter);
    }
}

bool SystemAbilityManager::IsCacheCommonEvent(int32_t systemAbilityId)
{
    SaProfile saProfile;
    if (!GetSaProfile(systemAbilityId, saProfile)) {
        HILOGD("SA:%{public}d no profile!", systemAbilityId);
        return false;
    }
    return saProfile.cacheCommonEvent;
}

void SystemAbilityManager::RemoveCheckLoadedMsg(int32_t systemAbilityId)
{
    if (workHandler_ == nullptr) {
        HILOGE("RemoveCheckLoadedMsg work handler not init");
        return;
    }
    workHandler_->RemoveTask(ToString(systemAbilityId));
}

void SystemAbilityManager::SendLoadedSystemAbilityMsg(int32_t systemAbilityId, const sptr<IRemoteObject>& remoteObject,
    const sptr<ISystemAbilityLoadCallback>& callback)
{
    if (workHandler_ == nullptr) {
        HILOGE("SendLoadedSaMsg work handler not init");
        return;
    }
    auto notifyLoadedTask = [systemAbilityId, remoteObject, callback, this]() {
        HILOGI("SendLoadedSaMsg notify SA:%{public}d", systemAbilityId);
        NotifySystemAbilityLoaded(systemAbilityId, remoteObject, callback);
    };
    bool ret = workHandler_->PostTask(notifyLoadedTask);
    if (!ret) {
        HILOGW("SendLoadedSaMsg PostTask fail");
    }
}

void SystemAbilityManager::NotifySystemAbilityLoaded(int32_t systemAbilityId, const sptr<IRemoteObject>& remoteObject,
    const sptr<ISystemAbilityLoadCallback>& callback)
{
    if (callback == nullptr) {
        HILOGE("NotifySystemAbilityLoaded callback null!");
        return;
    }
    HILOGD("NotifySaLoaded SA:%{public}d,SaSize:%{public}zu,ProcSize:%{public}zu,"
        "startingSaSize:%{public}zu", systemAbilityId, abilityMap_.size(), systemProcessMap_.size(),
        startingAbilityMap_.size());
    callback->OnLoadSystemAbilitySuccess(systemAbilityId, remoteObject);
}

void SystemAbilityManager::NotifySystemAbilityLoaded(int32_t systemAbilityId, const sptr<IRemoteObject>& remoteObject)
{
    lock_guard<mutex> autoLock(onDemandLock_);
    auto iter = startingAbilityMap_.find(systemAbilityId);
    if (iter == startingAbilityMap_.end()) {
        return;
    }
    auto& abilityItem = iter->second;
    for (auto& [deviceId, callbackList] : abilityItem.callbackMap) {
        for (auto& callbackItem : callbackList) {
            HILOGI("notify SA:%{public}d,SaSize:%{public}zu,ProcSize:%{public}zu,CbSize:%{public}d",
                systemAbilityId, abilityMap_.size(), systemProcessMap_.size(), callbackItem.second);
            NotifySystemAbilityLoaded(systemAbilityId, remoteObject, callbackItem.first);
            RemoveStartingAbilityCallbackLocked(callbackItem);
        }
    }
    startingAbilityMap_.erase(iter);
}

void SystemAbilityManager::NotifySystemAbilityLoadFail(int32_t systemAbilityId,
    const sptr<ISystemAbilityLoadCallback>& callback)
{
    if (callback == nullptr) {
        HILOGE("NotifySaLoadFail callback null");
        return;
    }
    HILOGI("NotifySaLoadFail SA:%{public}d", systemAbilityId);
    callback->OnLoadSystemAbilityFail(systemAbilityId);
}

bool SystemAbilityManager::IsInitBootFinished()
{
    std::string initTime = system::GetParameter(BOOT_INIT_TIME_PARAM, DEFAULT_BOOT_INIT_TIME);
    return initTime != DEFAULT_BOOT_INIT_TIME;
}

int32_t SystemAbilityManager::StartDynamicSystemProcess(const std::u16string& name,
    int32_t systemAbilityId, const OnDemandEvent& event)
{
    std::string eventStr = std::to_string(systemAbilityId) + "#" + std::to_string(event.eventId) + "#"
        + event.name + "#" + event.value + "#" + std::to_string(event.extraDataId) + "#";
    auto extraArgv = eventStr.c_str();
    if (abilityStateScheduler_ && !abilityStateScheduler_->IsSystemProcessNeverStartedLocked(name)) {
        // Waiting for the init subsystem to perceive process death
        int ret = ServiceWaitForStatus(Str16ToStr8(name).c_str(), ServiceStatus::SERVICE_STOPPED, 1);
        if (ret != 0) {
            HILOGE("ServiceWaitForStatus proc:%{public}s,SA:%{public}d timeout",
                Str16ToStr8(name).c_str(), systemAbilityId);
        }
    }
    int64_t begin = GetTickCount();
    int result = ERR_INVALID_VALUE;
    if (!IsInitBootFinished()) {
        result = ServiceControlWithExtra(Str16ToStr8(name).c_str(), ServiceAction::START, &extraArgv, 1);
    } else {
        SamgrXCollie samgrXCollie("samgr--startProccess_" + ToString(systemAbilityId));
        result = ServiceControlWithExtra(Str16ToStr8(name).c_str(), ServiceAction::START, &extraArgv, 1);
    }

    int64_t duration = GetTickCount() - begin;
    auto callingPid = IPCSkeleton::GetCallingPid();
    auto callingUid = IPCSkeleton::GetCallingUid();
    if (result != 0) {
        ReportProcessStartFail(Str16ToStr8(name), callingPid, callingUid, "err:" + ToString(result));
    }
    ReportProcessStartDuration(Str16ToStr8(name), callingPid, callingUid, duration);
    KHILOGI("Start dynamic proc:%{public}s,SA:%{public}d,ret:%{public}d,spend %{public}" PRId64 "ms",
        Str16ToStr8(name).c_str(), systemAbilityId, result, duration);
    return result;
}

int32_t SystemAbilityManager::StartingSystemProcessLocked(const std::u16string& procName,
    int32_t systemAbilityId, const OnDemandEvent& event)
{
    bool isProcessStarted = false;
    {
        lock_guard<mutex> autoLock(systemProcessMapLock_);
        isProcessStarted = (systemProcessMap_.count(procName) != 0);
    }
    if (isProcessStarted) {
        bool isExist = false;
        StartOnDemandAbilityLocked(systemAbilityId, isExist);
        if (!isExist) {
            HILOGE("no find onDemandSA:%{public}d", systemAbilityId);
        }
        return ERR_OK;
    }
    // call init start process
    {
        lock_guard<mutex> autoLock(startingProcessMapLock_);
        if (startingProcessMap_.count(procName) != 0) {
            HILOGI("StartingProc:%{public}s already starting", Str16ToStr8(procName).c_str());
            return ERR_OK;
        } else {
            int64_t begin = GetTickCount();
            startingProcessMap_.emplace(procName, begin);
        }
    }
    int32_t result = StartDynamicSystemProcess(procName, systemAbilityId, event);
    if (result != ERR_OK) {
        lock_guard<mutex> autoLock(startingProcessMapLock_);
        auto iterStarting = startingProcessMap_.find(procName);
        if (iterStarting != startingProcessMap_.end()) {
            startingProcessMap_.erase(iterStarting);
        }
    }
    return result;
}

int32_t SystemAbilityManager::StartingSystemProcess(const std::u16string& procName,
    int32_t systemAbilityId, const OnDemandEvent& event)
{
    bool isProcessStarted = false;
    {
        lock_guard<mutex> autoLock(systemProcessMapLock_);
        isProcessStarted = (systemProcessMap_.count(procName) != 0);
    }
    if (isProcessStarted) {
        bool isExist = false;
        StartOnDemandAbility(systemAbilityId, isExist);
        if (!isExist) {
            HILOGE("no find onDemandSA:%{public}d.", systemAbilityId);
        }
        return ERR_OK;
    }
    // call init start process
    {
        lock_guard<mutex> autoLock(startingProcessMapLock_);
        if (startingProcessMap_.count(procName) != 0) {
            HILOGI("StartingProc:%{public}s already starting", Str16ToStr8(procName).c_str());
            return ERR_OK;
        } else {
            int64_t begin = GetTickCount();
            startingProcessMap_.emplace(procName, begin);
        }
    }
    int32_t result = StartDynamicSystemProcess(procName, systemAbilityId, event);
    if (result != ERR_OK) {
        lock_guard<mutex> autoLock(startingProcessMapLock_);
        auto iterStarting = startingProcessMap_.find(procName);
        if (iterStarting != startingProcessMap_.end()) {
            startingProcessMap_.erase(iterStarting);
        }
    }
    return result;
}

int32_t SystemAbilityManager::DoLoadSystemAbility(int32_t systemAbilityId, const std::u16string& procName,
    const sptr<ISystemAbilityLoadCallback>& callback, int32_t callingPid, const OnDemandEvent& event)
{
    sptr<IRemoteObject> targetObject = CheckSystemAbility(systemAbilityId);
    if (targetObject != nullptr) {
        HILOGI("DoLoadSystemAbility notify SA:%{public}d callpid:%{public}d!", systemAbilityId, callingPid);
        NotifySystemAbilityLoaded(systemAbilityId, targetObject, callback);
        return ERR_OK;
    }
    int32_t result = ERR_INVALID_VALUE;
    {
        lock_guard<mutex> autoLock(onDemandLock_);
        auto& abilityItem = startingAbilityMap_[systemAbilityId];
        for (const auto& itemCallback : abilityItem.callbackMap[LOCAL_DEVICE]) {
            if (callback->AsObject() == itemCallback.first->AsObject()) {
                HILOGI("LoadSystemAbility already existed callback object SA:%{public}d", systemAbilityId);
                return ERR_OK;
            }
        }
        auto& count = callbackCountMap_[callingPid];
        if (count >= MAX_SUBSCRIBE_COUNT) {
            HILOGE("LoadSystemAbility pid:%{public}d overflow max callback count!", callingPid);
            return CALLBACK_MAP_SIZE_LIMIT;
        }
        ++count;
        abilityItem.callbackMap[LOCAL_DEVICE].emplace_back(callback, callingPid);
        abilityItem.event = event;
        if (abilityCallbackDeath_ != nullptr) {
            bool ret = callback->AsObject()->AddDeathRecipient(abilityCallbackDeath_);
            if (!ret) {
                HILOGW("LoadSa SA:%{public}d AddDeath fail,cnt:%{public}d,callpid:%{public}d",
                    systemAbilityId, count, callingPid);
            }
        }
        ReportSamgrSaLoad(systemAbilityId, IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid(), event.eventId);
        HILOGI("LoadSa SA:%{public}d size:%{public}zu,count:%{public}d",
            systemAbilityId, abilityItem.callbackMap[LOCAL_DEVICE].size(), count);
    }
    result = StartingSystemProcess(procName, systemAbilityId, event);
    SendCheckLoadedMsg(systemAbilityId, procName, LOCAL_DEVICE, callback);
    return result;
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
        lock_guard<mutex> autoLock(onDemandLock_);
        auto& abilityItem = startingAbilityMap_[systemAbilityId];
        abilityItem.callbackMap[srcDeviceId].emplace_back(callback, 0);
        StartingSystemProcessLocked(procName, systemAbilityId, event);
    }
    SendCheckLoadedMsg(systemAbilityId, procName, srcDeviceId, callback);
    return ERR_OK;
}

int32_t SystemAbilityManager::LoadSystemAbility(int32_t systemAbilityId,
    const sptr<ISystemAbilityLoadCallback>& callback)
{
    if (!CheckInputSysAbilityId(systemAbilityId) || callback == nullptr) {
        HILOGW("LoadSystemAbility SAId or callback invalid!");
        return INVALID_INPUT_PARA;
    }
    SaProfile saProfile;
    bool ret = GetSaProfile(systemAbilityId, saProfile);
    if (!ret) {
        HILOGE("LoadSystemAbility SA:%{public}d not supported!", systemAbilityId);
        return PROFILE_NOT_EXIST;
    }
    auto callingPid = IPCSkeleton::GetCallingPid();
    OnDemandEvent onDemandEvent = {INTERFACE_CALL, "load"};
    LoadRequestInfo loadRequestInfo = {LOCAL_DEVICE, callback, systemAbilityId, callingPid, onDemandEvent};
    return abilityStateScheduler_->HandleLoadAbilityEvent(loadRequestInfo);
}

bool SystemAbilityManager::LoadSystemAbilityFromRpc(const std::string& srcDeviceId, int32_t systemAbilityId,
    const sptr<ISystemAbilityLoadCallback>& callback)
{
    if (!CheckInputSysAbilityId(systemAbilityId) || callback == nullptr) {
        HILOGW("LoadSystemAbility said or callback invalid!");
        return false;
    }
    SaProfile saProfile;
    bool ret = GetSaProfile(systemAbilityId, saProfile);
    if (!ret) {
        HILOGE("LoadSystemAbilityFromRpc SA:%{public}d not supported!", systemAbilityId);
        return false;
    }

    if (!saProfile.distributed) {
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

int32_t SystemAbilityManager::UnloadSystemAbility(int32_t systemAbilityId)
{
    SaProfile saProfile;
    bool ret = GetSaProfile(systemAbilityId, saProfile);
    if (!ret) {
        HILOGE("UnloadSystemAbility SA:%{public}d not supported!", systemAbilityId);
        return PROFILE_NOT_EXIST;
    }
    if (!SamgrUtil::CheckCallerProcess(saProfile)) {
        HILOGE("UnloadSystemAbility invalid caller process, SA:%{public}d", systemAbilityId);
        return INVALID_CALL_PROC;
    }
    if (abilityStateScheduler_ == nullptr) {
        HILOGE("abilityStateScheduler is nullptr");
        return STATE_SCHEDULER_NULL;
    }
    OnDemandEvent onDemandEvent = {INTERFACE_CALL, "unload"};
    auto callingPid = IPCSkeleton::GetCallingPid();
    std::shared_ptr<UnloadRequestInfo> unloadRequestInfo =
        std::make_shared<UnloadRequestInfo>(onDemandEvent, systemAbilityId, callingPid);
    return abilityStateScheduler_->HandleUnloadAbilityEvent(unloadRequestInfo);
}

bool SystemAbilityManager::CheckSaIsImmediatelyRecycle(int32_t systemAbilityId)
{
    SaProfile saProfile;
    bool ret = GetSaProfile(systemAbilityId, saProfile);
    if (!ret) {
        HILOGE("UnloadSystemAbility SA:%{public}d not supported!", systemAbilityId);
        return ERR_INVALID_VALUE;
    }
    return saProfile.recycleStrategy == IMMEDIATELY;
}

int32_t SystemAbilityManager::CancelUnloadSystemAbility(int32_t systemAbilityId)
{
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("CancelUnloadSystemAbility SAId or callback invalid!");
        return ERR_INVALID_VALUE;
    }
    SaProfile saProfile;
    bool ret = GetSaProfile(systemAbilityId, saProfile);
    if (!ret) {
        HILOGE("CancelUnloadSystemAbility SA:%{public}d not supported!", systemAbilityId);
        return ERR_INVALID_VALUE;
    }
    if (!SamgrUtil::CheckCallerProcess(saProfile)) {
        HILOGE("CancelUnloadSystemAbility invalid caller process, SA:%{public}d", systemAbilityId);
        return ERR_INVALID_VALUE;
    }
    if (abilityStateScheduler_ == nullptr) {
        HILOGE("abilityStateScheduler is nullptr");
        return ERR_INVALID_VALUE;
    }
    return abilityStateScheduler_->HandleCancelUnloadAbilityEvent(systemAbilityId);
}

int32_t SystemAbilityManager::DoUnloadSystemAbility(int32_t systemAbilityId,
    const std::u16string& procName, const OnDemandEvent& event)
{
    sptr<IRemoteObject> targetObject = CheckSystemAbility(systemAbilityId);
    if (targetObject == nullptr) {
        return ERR_OK;
    }
    {
        lock_guard<mutex> autoLock(onDemandLock_);
        bool result = StopOnDemandAbilityInner(procName, systemAbilityId, event);
        if (!result) {
            HILOGE("unload system ability failed, SA:%{public}d", systemAbilityId);
            return ERR_INVALID_VALUE;
        }
    }
    ReportSamgrSaUnload(systemAbilityId, IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid(), event.eventId);
    SamgrUtil::SendUpdateSaState(systemAbilityId, "unload");
    return ERR_OK;
}

int32_t SystemAbilityManager::UnloadAllIdleSystemAbility()
{
    if (!SamgrUtil::CheckCallerProcess("memmgrservice")) {
        HILOGE("UnloadAllIdleSystemAbility invalid caller process, only support for memmgrservice");
        return ERR_PERMISSION_DENIED;
    }
    if (abilityStateScheduler_ == nullptr) {
        HILOGE("abilityStateScheduler is nullptr");
        return ERR_INVALID_VALUE;
    }
    return abilityStateScheduler_->UnloadAllIdleSystemAbility();
}

bool SystemAbilityManager::IdleSystemAbility(int32_t systemAbilityId, const std::u16string& procName,
    const nlohmann::json& idleReason, int32_t& delayTime)
{
    sptr<IRemoteObject> targetObject = CheckSystemAbility(systemAbilityId);
    if (targetObject == nullptr) {
        HILOGE("IdleSystemAbility SA:%{public}d not loaded", systemAbilityId);
        return false;
    }
    sptr<ILocalAbilityManager> procObject =
        iface_cast<ILocalAbilityManager>(GetSystemProcess(procName));
    if (procObject == nullptr) {
        HILOGE("get process:%{public}s fail", Str16ToStr8(procName).c_str());
        return false;
    }
    HILOGI("IdleSA:%{public}d", systemAbilityId);
    SamgrXCollie samgrXCollie("samgr--IdleSa_" + ToString(systemAbilityId));
    return procObject->IdleAbility(systemAbilityId, idleReason, delayTime);
}

bool SystemAbilityManager::ActiveSystemAbility(int32_t systemAbilityId, const std::u16string& procName,
    const nlohmann::json& activeReason)
{
    sptr<IRemoteObject> targetObject = CheckSystemAbility(systemAbilityId);
    if (targetObject == nullptr) {
        HILOGE("ActiveSystemAbility SA:%{public}d not loaded", systemAbilityId);
        return false;
    }
    sptr<ILocalAbilityManager> procObject =
        iface_cast<ILocalAbilityManager>(GetSystemProcess(procName));
    if (procObject == nullptr) {
        HILOGE("get process:%{public}s fail", Str16ToStr8(procName).c_str());
        return false;
    }
    HILOGI("ActiveSA:%{public}d", systemAbilityId);
    SamgrXCollie samgrXCollie("samgr--ActiveSa_" + ToString(systemAbilityId));
    return procObject->ActiveAbility(systemAbilityId, activeReason);
}

int32_t SystemAbilityManager::LoadSystemAbility(int32_t systemAbilityId, const std::string& deviceId,
    const sptr<ISystemAbilityLoadCallback>& callback)
{
    std::string key = ToString(systemAbilityId) + "_" + deviceId;
    {
        lock_guard<mutex> autoLock(loadRemoteLock_);
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
        lock_guard<mutex> autoLock(loadRemoteLock_);
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

#ifdef SUPPORT_DEVICE_MANAGER
void SystemAbilityManager::DeviceIdToNetworkId(std::string& networkId)
{
    std::vector<DmDeviceInfo> devList;
    if (DeviceManager::GetInstance().GetTrustedDeviceList(PKG_NAME, "", devList) == ERR_OK) {
        for (const DmDeviceInfo& devInfo : devList) {
            if (networkId == devInfo.deviceId) {
                networkId = devInfo.networkId;
                break;
            }
        }
    }
}
#endif

sptr<DBinderServiceStub> SystemAbilityManager::DoMakeRemoteBinder(int32_t systemAbilityId, int32_t callingPid,
    int32_t callingUid, const std::string& deviceId)
{
    HILOGI("MakeRemoteBinder begin, SA:%{public}d", systemAbilityId);
    std::string networkId = deviceId;
#ifdef SUPPORT_DEVICE_MANAGER
    DeviceIdToNetworkId(networkId);
#endif
    sptr<DBinderServiceStub> remoteBinder = nullptr;
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

void SystemAbilityManager::RemoveStartingAbilityCallbackLocked(
    std::pair<sptr<ISystemAbilityLoadCallback>, int32_t>& itemPair)
{
    if (abilityCallbackDeath_ != nullptr) {
        itemPair.first->AsObject()->RemoveDeathRecipient(abilityCallbackDeath_);
    }
    auto iterCount = callbackCountMap_.find(itemPair.second);
    if (iterCount != callbackCountMap_.end()) {
        --iterCount->second;
        if (iterCount->second == 0) {
            callbackCountMap_.erase(iterCount);
        }
    }
}

void SystemAbilityManager::RemoveStartingAbilityCallbackForDevice(AbilityItem& abilityItem,
    const sptr<IRemoteObject>& remoteObject)
{
    auto& callbacks = abilityItem.callbackMap;
    auto iter = callbacks.begin();
    while (iter != callbacks.end()) {
        CallbackList& callbackList = iter->second;
        RemoveStartingAbilityCallback(callbackList, remoteObject);
        if (callbackList.empty()) {
            callbacks.erase(iter++);
        } else {
            ++iter;
        }
    }
}

void SystemAbilityManager::RemoveStartingAbilityCallback(CallbackList& callbackList,
    const sptr<IRemoteObject>& remoteObject)
{
    auto iterCallback = callbackList.begin();
    while (iterCallback != callbackList.end()) {
        auto& callbackPair = *iterCallback;
        if (callbackPair.first->AsObject() == remoteObject) {
            RemoveStartingAbilityCallbackLocked(callbackPair);
            iterCallback = callbackList.erase(iterCallback);
            break;
        } else {
            ++iterCallback;
        }
    }
}

void SystemAbilityManager::OnAbilityCallbackDied(const sptr<IRemoteObject>& remoteObject)
{
    HILOGI("OnAbilityCallbackDied received remoteObject died message!");
    if (remoteObject == nullptr) {
        return;
    }
    lock_guard<mutex> autoLock(onDemandLock_);
    auto iter = startingAbilityMap_.begin();
    while (iter != startingAbilityMap_.end()) {
        AbilityItem& abilityItem = iter->second;
        RemoveStartingAbilityCallbackForDevice(abilityItem, remoteObject);
        if (abilityItem.callbackMap.empty()) {
            startingAbilityMap_.erase(iter++);
        } else {
            ++iter;
        }
    }
}

void SystemAbilityManager::OnRemoteCallbackDied(const sptr<IRemoteObject>& remoteObject)
{
    HILOGI("OnRemoteCallbackDied received remoteObject died message!");
    if (remoteObject == nullptr) {
        return;
    }
    lock_guard<mutex> autoLock(loadRemoteLock_);
    auto iter = remoteCallbacks_.begin();
    while (iter != remoteCallbacks_.end()) {
        auto& callbacks = iter->second;
        RemoveRemoteCallbackLocked(callbacks, remoteObject);
        if (callbacks.empty()) {
            remoteCallbacks_.erase(iter++);
        } else {
            ++iter;
        }
    }
}

void SystemAbilityManager::RemoveRemoteCallbackLocked(std::list<sptr<ISystemAbilityLoadCallback>>& callbacks,
    const sptr<IRemoteObject>& remoteObject)
{
    for (const auto& callback : callbacks) {
        if (callback->AsObject() == remoteObject) {
            if (remoteCallbackDeath_ != nullptr) {
                callback->AsObject()->RemoveDeathRecipient(remoteCallbackDeath_);
            }
            callbacks.remove(callback);
            break;
        }
    }
}

int32_t SystemAbilityManager::UpdateSaFreMap(int32_t uid, int32_t saId)
{
    if (uid < 0) {
        HILOGW("UpdateSaFreMap return, uid not valid!");
        return -1;
    }

    uint64_t key = SamgrUtil::GenerateFreKey(uid, saId);
    lock_guard<mutex> autoLock(saFrequencyLock_);
    auto& count = saFrequencyMap_[key];
    if (count < MAX_SA_FREQUENCY_COUNT) {
        count++;
    }
    return count;
}

void SystemAbilityManager::ReportGetSAPeriodically()
{
    HILOGI("ReportGetSAPeriodically start!");
    lock_guard<mutex> autoLock(saFrequencyLock_);
    for (const auto& [key, count] : saFrequencyMap_) {
        uint32_t saId = static_cast<uint32_t>(key);
        uint32_t uid = key >> SHFIT_BIT;
        ReportGetSAFrequency(uid, saId, count);
    }
    saFrequencyMap_.clear();
}

int32_t SystemAbilityManager::GetOnDemandSystemAbilityIds(std::vector<int32_t>& systemAbilityIds)
{
    HILOGD("GetOnDemandSystemAbilityIds start!");
    if (onDemandSaIdsSet_.empty()) {
        HILOGD("GetOnDemandSystemAbilityIds error!");
        return ERR_INVALID_VALUE;
    }
    for (int32_t onDemandSaId : onDemandSaIdsSet_) {
        systemAbilityIds.emplace_back(onDemandSaId);
    }
    return ERR_OK;
}

int32_t SystemAbilityManager::SendStrategy(int32_t type, std::vector<int32_t>& systemAbilityIds,
    int32_t level, std::string& action)
{
    HILOGD("SendStrategy begin");
    uint32_t accessToken = IPCSkeleton::GetCallingTokenID();
    Security::AccessToken::NativeTokenInfo nativeTokenInfo;
    int32_t result = Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(accessToken, nativeTokenInfo);
    if (result != ERR_OK || nativeTokenInfo.processName != RESOURCE_SCHEDULE_PROCESS_NAME) {
        HILOGW("SendStrategy reject used by %{public}s", nativeTokenInfo.processName.c_str());
        return ERR_PERMISSION_DENIED;
    }

    for (auto saId : systemAbilityIds) {
        SaProfile saProfile;
        if (!GetSaProfile(saId, saProfile)) {
            HILOGW("not found SA: %{public}d.", saId);
            return ERR_INVALID_VALUE;
        }
        auto procName = saProfile.process;
        sptr<ILocalAbilityManager> procObject =
            iface_cast<ILocalAbilityManager>(GetSystemProcess(procName));
        if (procObject == nullptr) {
            HILOGW("get process:%{public}s fail", Str16ToStr8(procName).c_str());
            return ERR_INVALID_VALUE;
        }
        procObject->SendStrategyToSA(type, saId, level, action);
    }
    return ERR_OK;
}

int32_t SystemAbilityManager::GetExtensionSaIds(const std::string& extension, std::vector<int32_t>& saIds)
{
    lock_guard<mutex> autoLock(saProfileMapLock_);
    for (const auto& [saId, value] : saProfileMap_) {
        if (std::find(value.extension.begin(), value.extension.end(), extension) !=
            value.extension.end()) {
            saIds.push_back(saId);
        }
    }
    return ERR_OK;
}

int32_t SystemAbilityManager::GetExtensionRunningSaList(const std::string& extension,
    std::vector<sptr<IRemoteObject>>& saList)
{
    lock_guard<mutex> autoLock(saProfileMapLock_);
    for (const auto& [saId, value] : saProfileMap_) {
        if (std::find(value.extension.begin(), value.extension.end(), extension)
            != value.extension.end()) {
            shared_lock<shared_mutex> readLock(abilityMapLock_);
            auto iter = abilityMap_.find(saId);
            if (iter != abilityMap_.end() && iter->second.remoteObj != nullptr) {
                saList.push_back(iter->second.remoteObj);
                HILOGD("%{public}s get extension(%{public}s) saId(%{public}d)", __func__, extension.c_str(), saId);
            }
        }
    }
    return ERR_OK;
}

int32_t SystemAbilityManager::GetRunningSaExtensionInfoList(const std::string& extension,
    std::vector<SaExtensionInfo>& infoList)
{
    lock_guard<mutex> autoLock(saProfileMapLock_);
    for (const auto& [saId, value] : saProfileMap_) {
        if (std::find(value.extension.begin(), value.extension.end(), extension)
            != value.extension.end()) {
            auto obj = GetSystemProcess(value.process);
            if (obj == nullptr) {
                HILOGD("get SaExtInfoList sa not load,ext:%{public}s SA:%{public}d", extension.c_str(), saId);
                continue;
            }
            shared_lock<shared_mutex> readLock(abilityMapLock_);
            auto iter = abilityMap_.find(saId);
            if (iter == abilityMap_.end() || iter->second.remoteObj == nullptr) {
                HILOGD("getRunningSaExtInfoList SA:%{public}d not load,ext:%{public}s", saId, extension.c_str());
                continue;
            }
            SaExtensionInfo tmp{saId, obj};
            infoList.emplace_back(tmp);
            HILOGD("get SaExtInfoList suc,ext:%{public}s,SA:%{public}d,proc:%{public}s",
                extension.c_str(), saId, Str16ToStr8(value.process).c_str());
        }
    }
    return ERR_OK;
}

int32_t SystemAbilityManager::GetCommonEventExtraDataIdlist(int32_t saId, std::vector<int64_t>& extraDataIdList,
    const std::string& eventName)
{
    if (!IsCacheCommonEvent(saId)) {
        HILOGI("SA:%{public}d no cache event", saId);
        return ERR_OK;
    }
    if (collectManager_ == nullptr) {
        HILOGE("collectManager is nullptr");
        return ERR_INVALID_VALUE;
    }
    return collectManager_->GetSaExtraDataIdList(saId, extraDataIdList, eventName);
}
} // namespace OHOS
