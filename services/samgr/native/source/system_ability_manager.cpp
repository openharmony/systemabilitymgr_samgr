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
#include "hicollie_helper.h"
#include "hisysevent_adapter.h"
#include "hitrace_meter.h"
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
#include "system_ability_manager_dumper.h"
#include "tools.h"

#ifdef SUPPORT_DEVICE_MANAGER
#include "device_manager.h"
using namespace OHOS::DistributedHardware;
#endif

using namespace std;

namespace OHOS {
namespace {
const string START_SAID = "said";
const string EVENT_TYPE = "eventId";
const string EVENT_NAME = "name";
const string EVENT_VALUE = "value";
const string EVENT_EXTRA_DATA_ID = "extraDataId";
const string PKG_NAME = "Samgr_Networking";
const string PREFIX = "/system/profile/";
const string LOCAL_DEVICE = "local";
const string ONDEMAND_PARAM = "persist.samgr.perf.ondemand";
constexpr const char* ONDEMAND_PERF_PARAM = "persist.samgr.perf.ondemand";
constexpr const char* ONDEMAND_WORKER = "OndemandLoader";

constexpr uint32_t REPORT_GET_SA_INTERVAL = 24 * 60 * 60 * 1000; // ms and is one day
constexpr int32_t MAX_NAME_SIZE = 200;
constexpr int32_t SPLIT_NAME_VECTOR_SIZE = 2;
constexpr int32_t WAITTING = 1;
constexpr int32_t UID_ROOT = 0;
constexpr int32_t UID_SYSTEM = 1000;
constexpr int32_t MAX_SUBSCRIBE_COUNT = 256;
constexpr int32_t MAX_SA_FREQUENCY_COUNT = INT32_MAX - 1000000;
constexpr int32_t SHFIT_BIT = 32;
constexpr int64_t ONDEMAND_PERF_DELAY_TIME = 60 * 1000; // ms
constexpr int64_t CHECK_LOADED_DELAY_TIME = 4 * 1000; // ms
constexpr int32_t SOFTBUS_SERVER_SA_ID = 4700;
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
        auto runner = AppExecFwk::EventRunner::Create("workHandler");
        workHandler_ = make_shared<AppExecFwk::EventHandler>(runner);
        workHandler_->PostTask([]() { Samgr::MemoryGuard cacheGuard; });
    }
    collectManager_ = sptr<DeviceStatusCollectManager>(new DeviceStatusCollectManager());
    abilityStateScheduler_ = std::make_shared<SystemAbilityStateScheduler>();
    InitSaProfile();
    WatchDogInit();
    reportEventTimer_ = std::make_unique<Utils::Timer>("DfxReporter");
    OndemandLoadForPerf();
}

void SystemAbilityManager::WatchDogInit()
{
    constexpr int CHECK_PERIOD = 10000;
    auto timeOutCallback = [this](const std::string& name, int waitState) {
        int32_t pid = getpid();
        uint32_t uid = getuid();
        time_t curTime = time(nullptr);
        std::string sendMsg = std::string((ctime(&curTime) == nullptr) ? "" : ctime(&curTime)) + "\n";
        if (waitState == WAITTING) {
            WatchDogSendEvent(pid, uid, sendMsg, "SERVICE_BLOCK");
        }
    };
    int result = HicollieHelper::AddThread("SamgrTask", workHandler_, timeOutCallback, CHECK_PERIOD);
    if (!result) {
        HILOGE("Watchdog start failed");
    }
}

int32_t SystemAbilityManager::Dump(int32_t fd, const std::vector<std::u16string>& args)
{
    std::vector<std::string> argsWithStr8;
    for (const auto& arg : args) {
        argsWithStr8.emplace_back(Str16ToStr8(arg));
    }
    std::string result;
    SystemAbilityManagerDumper::Dump(abilityStateScheduler_, argsWithStr8, result);
    if (!SaveStringToFd(fd, result)) {
        HILOGE("save to fd failed");
        return ERR_INVALID_VALUE;
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
    HILOGD("samgr inserted");
}

const sptr<DBinderService> SystemAbilityManager::GetDBinder() const
{
    return dBinderService_;
}

void SystemAbilityManager::StartDfxTimer()
{
    reportEventTimer_->Setup();
    uint32_t timerId = reportEventTimer_->Register(std::bind(&SystemAbilityManager::ReportGetSAPeriodically, this),
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
    if (collectManager_ != nullptr) {
        collectManager_->Init(saInfos);
    }
    if (abilityStateScheduler_ != nullptr) {
        abilityStateScheduler_->Init(saInfos);
    }
    lock_guard<mutex> autoLock(saProfileMapLock_);
    for (const auto& saInfo : saInfos) {
        saProfileMap_[saInfo.saId] = saInfo;
    }
    HILOGI("[PerformanceTest] InitSaProfile spend %{public}" PRId64 " ms", GetTickCount() - begin);
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
        HILOGI("[PerformanceTest] DoLoadForPerf spend %{public}" PRId64 " ms", GetTickCount() - begin);
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

bool SystemAbilityManager::CheckCallerProcess(SaProfile& saProfile)
{
    uint32_t accessToken = IPCSkeleton::GetCallingTokenID();
    Security::AccessToken::NativeTokenInfo nativeTokenInfo;
    int32_t tokenInfoResult = Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(accessToken, nativeTokenInfo);
    if (tokenInfoResult != ERR_OK) {
        HILOGE("get token info failed");
        return false;
    }
    std::string callProcess = Str16ToStr8(saProfile.process);
    if (nativeTokenInfo.processName!= callProcess) {
        HILOGE("cannot operate SA: %{public}d by process: %{public}s",
            saProfile.saId, nativeTokenInfo.processName.c_str());
        return false;
    }
    return true;
}

bool SystemAbilityManager::CheckAllowUpdate(OnDemandPolicyType type, SaProfile& saProfile)
{
    if (type == OnDemandPolicyType::START_POLICY && saProfile.startOnDemand.allowUpdate) {
        return true;
    } else if (type == OnDemandPolicyType::STOP_POLICY && saProfile.stopOnDemand.allowUpdate) {
        return true;
    }
    return false;
}

void SystemAbilityManager::ConvertToOnDemandEvent(const SystemAbilityOnDemandEvent& from, OnDemandEvent& to)
{
    to.eventId = static_cast<int32_t>(from.eventId);
    to.name = from.name;
    to.value = from.value;
    for (auto& item : from.conditions) {
        OnDemandCondition condition;
        condition.eventId = static_cast<int32_t>(item.eventId);
        condition.name = item.name;
        condition.value = item.value;
        to.conditions.push_back(condition);
    }
    to.enableOnce = from.enableOnce;
}

void SystemAbilityManager::ConvertToSystemAbilityOnDemandEvent(const OnDemandEvent& from,
    SystemAbilityOnDemandEvent& to)
{
    to.eventId = static_cast<OnDemandEventId>(from.eventId);
    to.name = from.name;
    to.value = from.value;
    for (auto& item : from.conditions) {
        SystemAbilityOnDemandCondition condition;
        condition.eventId = static_cast<OnDemandEventId>(item.eventId);
        condition.name = item.name;
        condition.value = item.value;
        to.conditions.push_back(condition);
    }
    to.enableOnce = from.enableOnce;
}

int32_t SystemAbilityManager::GetOnDemandPolicy(int32_t systemAbilityId, OnDemandPolicyType type,
    std::vector<SystemAbilityOnDemandEvent>& abilityOnDemandEvents)
{
    SaProfile saProfile;
    if (!GetSaProfile(systemAbilityId, saProfile)) {
        HILOGE("GetOnDemandPolicy invalid saId: %{public}d", systemAbilityId);
        return ERR_INVALID_VALUE;
    }
    if (!CheckCallerProcess(saProfile)) {
        HILOGE("GetOnDemandPolicy invalid caller saId: %{public}d", systemAbilityId);
        return ERR_INVALID_VALUE;
    }
    if (!CheckAllowUpdate(type, saProfile)) {
        HILOGE("GetOnDemandPolicy not allow get saId: %{public}d", systemAbilityId);
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
        ConvertToSystemAbilityOnDemandEvent(item, eventOuter);
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
        HILOGE("UpdateOnDemandPolicy invalid saId: %{public}d", systemAbilityId);
        return ERR_INVALID_VALUE;
    }
    if (!CheckCallerProcess(saProfile)) {
        HILOGE("UpdateOnDemandPolicy invalid caller saId: %{public}d", systemAbilityId);
        return ERR_INVALID_VALUE;
    }
    if (!CheckAllowUpdate(type, saProfile)) {
        HILOGE("UpdateOnDemandPolicy not allow get saId: %{public}d", systemAbilityId);
        return ERR_PERMISSION_DENIED;
    }

    if (collectManager_ == nullptr) {
        HILOGE("UpdateOnDemandPolicy collectManager is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::vector<OnDemandEvent> onDemandEvents;
    for (auto& item : abilityOnDemandEvents) {
        OnDemandEvent event;
        ConvertToOnDemandEvent(item, event);
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
    HILOGI("SystemAbilityManager::ProcessEvent eventId:%{public}d name:%{public}s value:%{public}s",
        event.eventId, event.name.c_str(), event.value.c_str());
    sptr<ISystemAbilityLoadCallback> callback(new SystemAbilityLoadCallbackStub());
    if (abilityStateScheduler_ == nullptr) {
        HILOGE("abilityStateScheduler is nullptr");
        return;
    }
    for (auto& saControl : saControlList) {
        int32_t result = ERR_INVALID_VALUE;
        if (saControl.ondemandId == START_ON_DEMAND) {
            result = CheckStartEnableOnce(event, saControl, callback);
        } else if (saControl.ondemandId == STOP_ON_DEMAND) {
            result = CheckStopEnableOnce(event, saControl);
        } else {
            HILOGE("ondemandId error");
        }
        if (result != ERR_OK) {
            HILOGE("process ondemand event failed, ondemandId:%{public}d, saId:%{public}d",
                saControl.ondemandId, saControl.saId);
        }
    }
}

int32_t SystemAbilityManager::CheckStartEnableOnce(const OnDemandEvent& event,
    const SaControlInfo& saControl, sptr<ISystemAbilityLoadCallback> callback)
{
    int32_t result = ERR_INVALID_VALUE;
    if (saControl.enableOnce) {
        lock_guard<mutex> autoLock(startEnableOnceLock_);
        auto iter = startEnableOnceMap_.find(saControl.saId);
        if (iter != startEnableOnceMap_.end() && IsSameEvent(event, startEnableOnceMap_[saControl.saId])) {
            HILOGI("ondemand canceled for enable-once, ondemandId:%{public}d, saId:%{public}d",
                saControl.ondemandId, saControl.saId);
            return result;
        }
        startEnableOnceMap_[saControl.saId].emplace_back(event);
        HILOGI("startEnableOnceMap_ add saId:%{public}d, eventId:%{public}d",
            saControl.saId, event.eventId);
    }
    auto callingPid = IPCSkeleton::GetCallingPid();
    LoadRequestInfo loadRequestInfo = {saControl.saId, LOCAL_DEVICE, callback, callingPid, event};
    result = abilityStateScheduler_->HandleLoadAbilityEvent(loadRequestInfo);
    if (saControl.enableOnce && result != ERR_OK) {
        lock_guard<mutex> autoLock(startEnableOnceLock_);
        auto& events = startEnableOnceMap_[saControl.saId];
        events.remove(event);
        if (events.empty()) {
            startEnableOnceMap_.erase(saControl.saId);
        }
        HILOGI("startEnableOnceMap_remove saId:%{public}d, eventId:%{public}d",
            saControl.saId, event.eventId);
    }
    return result;
}

int32_t SystemAbilityManager::CheckStopEnableOnce(const OnDemandEvent& event,
    const SaControlInfo& saControl)
{
    int32_t result = ERR_INVALID_VALUE;
    if (saControl.enableOnce) {
        lock_guard<mutex> autoLock(stopEnableOnceLock_);
        auto iter = stopEnableOnceMap_.find(saControl.saId);
        if (iter != stopEnableOnceMap_.end() && IsSameEvent(event, stopEnableOnceMap_[saControl.saId])) {
            HILOGI("ondemand canceled for enable-once, ondemandId:%{public}d, saId:%{public}d",
                saControl.ondemandId, saControl.saId);
            return result;
        }
        stopEnableOnceMap_[saControl.saId].emplace_back(event);
        HILOGI("stopEnableOnceMap_ add saId:%{public}d, eventId:%{public}d",
            saControl.saId, event.eventId);
    }
    UnloadRequestInfo unloadRequestInfo = {saControl.saId, event};
    result = abilityStateScheduler_->HandleUnloadAbilityEvent(unloadRequestInfo);
    if (saControl.enableOnce && result != ERR_OK) {
        lock_guard<mutex> autoLock(stopEnableOnceLock_);
        auto& events = stopEnableOnceMap_[saControl.saId];
        events.remove(event);
        if (events.empty()) {
            stopEnableOnceMap_.erase(saControl.saId);
        }
        HILOGI("stopEnableOnceMap_ remove saId:%{public}d, eventId:%{public}d",
            saControl.saId, event.eventId);
    }
    return result;
}

bool SystemAbilityManager::IsSameEvent(const OnDemandEvent& event, std::list<OnDemandEvent>& enableOnceList)
{
    for (auto iter = enableOnceList.begin(); iter != enableOnceList.end(); iter++) {
        if (event.eventId == iter->eventId && event.name == iter->name && event.value == iter->value) {
            HILOGI("event already exits in enable-once list");
            return true;
        }
    }
    return false;
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
    HILOGD("%{public}s called, systemAbilityId = %{public}d", __func__, systemAbilityId);
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("GetSystemAbilityFromRemote invalid!");
        return nullptr;
    }

    shared_lock<shared_mutex> readLock(abilityMapLock_);
    auto iter = abilityMap_.find(systemAbilityId);
    if (iter == abilityMap_.end()) {
        HILOGI("GetSystemAbilityFromRemote not found service : %{public}d.", systemAbilityId);
        return nullptr;
    }
    if (!(iter->second.isDistributed)) {
        HILOGW("GetSystemAbilityFromRemote service : %{public}d not distributed", systemAbilityId);
        return nullptr;
    }
    HILOGI("GetSystemAbilityFromRemote found service : %{public}d.", systemAbilityId);
    return iter->second.remoteObj;
}

sptr<IRemoteObject> SystemAbilityManager::CheckSystemAbility(int32_t systemAbilityId)
{
    HILOGD("%{public}s called, systemAbilityId = %{public}d", __func__, systemAbilityId);
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("CheckSystemAbility CheckSystemAbility invalid!");
        return nullptr;
    }
    UpdateSaFreMap(IPCSkeleton::GetCallingUid(), systemAbilityId);
    shared_lock<shared_mutex> readLock(abilityMapLock_);
    auto iter = abilityMap_.find(systemAbilityId);
    if (iter != abilityMap_.end()) {
        HILOGD("found service : %{public}d.", systemAbilityId);
        return iter->second.remoteObj;
    }
    HILOGW("NOT found service : %{public}d", systemAbilityId);
    return nullptr;
}

bool SystemAbilityManager::CheckDistributedPermission()
{
    auto callingUid = IPCSkeleton::GetCallingUid();
    if (callingUid != UID_ROOT && callingUid != UID_SYSTEM) {
        return false;
    }
    return true;
}

sptr<IRemoteObject> SystemAbilityManager::CheckSystemAbility(int32_t systemAbilityId,
    const std::string& deviceId)
{
    return DoMakeRemoteBinder(systemAbilityId, IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid(), deviceId);
}

int32_t SystemAbilityManager::FindSystemAbilityNotify(int32_t systemAbilityId, int32_t code)
{
    return FindSystemAbilityNotify(systemAbilityId, "", code);
}

void SystemAbilityManager::NotifySystemAbilityChanged(int32_t systemAbilityId, const std::string& deviceId,
    int32_t code, const sptr<ISystemAbilityStatusChange>& listener)
{
    HILOGD("NotifySystemAbilityChanged, systemAbilityId = %{public}d", systemAbilityId);
    if (listener == nullptr) {
        HILOGE("%s listener null pointer!", __func__);
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
    HILOGI("%{public}s called:systemAbilityId = %{public}d, code = %{public}d", __func__, systemAbilityId, code);
    lock_guard<recursive_mutex> autoLock(listenerMapLock_);
    auto iter = listenerMap_.find(systemAbilityId);
    if (iter != listenerMap_.end()) {
        auto& listeners = iter->second;
        for (const auto& item : listeners) {
            NotifySystemAbilityChanged(systemAbilityId, deviceId, code, item.first);
        }
    }

    return ERR_OK;
}

bool SystemAbilityManager::IsNameInValid(const std::u16string& name)
{
    HILOGI("%{public}s called:name = %{public}s", __func__, Str16ToStr8(name).c_str());
    bool ret = false;
    if (name.empty() || name.size() > MAX_NAME_SIZE || DeleteBlank(name).empty()) {
        ret = true;
    }

    return ret;
}

void SystemAbilityManager::StartOnDemandAbility(const std::u16string& procName, int32_t systemAbilityId)
{
    lock_guard<recursive_mutex> autoLock(onDemandLock_);
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
        return ERR_INVALID_VALUE;
    }
    sptr<ILocalAbilityManager> procObject =
        iface_cast<ILocalAbilityManager>(GetSystemProcess(procName));
    if (procObject == nullptr) {
        HILOGI("get process:%{public}s fail", Str16ToStr8(procName).c_str());
        return ERR_INVALID_VALUE;
    }
    auto event = abilityItem.event;
    auto eventStr = EventToStr(event);
    procObject->StartAbility(systemAbilityId, eventStr);
    abilityItem.state = AbilityState::STARTING;
    return ERR_OK;
}

bool SystemAbilityManager::StopOnDemandAbility(const std::u16string& procName,
    int32_t systemAbilityId, const OnDemandEvent& event)
{
    lock_guard<recursive_mutex> autoLock(onDemandLock_);
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
    auto eventStr = EventToStr(event);
    return procObject->StopAbility(systemAbilityId, eventStr);
}

int32_t SystemAbilityManager::AddOnDemandSystemAbilityInfo(int32_t systemAbilityId,
    const std::u16string& procName)
{
    HILOGD("%{public}s called", __func__);
    if (!CheckInputSysAbilityId(systemAbilityId) || IsNameInValid(procName)) {
        HILOGW("AddOnDemandSystemAbilityInfo systemAbilityId or procName invalid.");
        return ERR_INVALID_VALUE;
    }

    lock_guard<recursive_mutex> autoLock(onDemandLock_);
    auto onDemandSaSize = onDemandAbilityMap_.size();
    if (onDemandSaSize >= MAX_SERVICES) {
        HILOGE("map size error, (Has been greater than %{public}zu)",
            onDemandAbilityMap_.size());
        return ERR_INVALID_VALUE;
    }

    if (systemProcessMap_.count(procName) == 0) {
        HILOGW("AddOnDemandSystemAbilityInfo procName:%{public}s not exist.", Str16ToStr8(procName).c_str());
        return ERR_INVALID_VALUE;
    }
    onDemandAbilityMap_[systemAbilityId] = procName;
    HILOGI("insert onDemand systemAbilityId:%{public}d. size : %{public}zu", systemAbilityId,
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
    lock_guard<recursive_mutex> onDemandAbilityLock(onDemandLock_);
    auto iter = onDemandAbilityMap_.find(systemAbilityId);
    if (iter == onDemandAbilityMap_.end()) {
        isExist = false;
        return ERR_INVALID_VALUE;
    }
    HILOGI("found onDemandAbility: %{public}d.", systemAbilityId);
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
        HILOGW("SA: %{public}d is unloading", systemAbilityId);
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
    lock_guard<recursive_mutex> autoLock(onDemandLock_);
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
    return StartOnDemandAbility(systemAbilityId, isExist) == ERR_OK;
}

int32_t SystemAbilityManager::RemoveSystemAbility(int32_t systemAbilityId)
{
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("RemoveSystemAbility systemAbilityId:%{public}d", systemAbilityId);
        return ERR_INVALID_VALUE;
    }
    {
        unique_lock<shared_mutex> writeLock(abilityMapLock_);
        auto itSystemAbility = abilityMap_.find(systemAbilityId);
        if (itSystemAbility == abilityMap_.end()) {
            HILOGI("SystemAbilityManager::RemoveSystemAbility not found!");
            return ERR_INVALID_VALUE;
        }
        sptr<IRemoteObject> ability = itSystemAbility->second.remoteObj;
        if (ability != nullptr && abilityDeath_ != nullptr) {
            ability->RemoveDeathRecipient(abilityDeath_);
        }
        (void)abilityMap_.erase(itSystemAbility);
        HILOGI("%s called, systemAbilityId : %{public}d, size : %{public}zu", __func__, systemAbilityId,
            abilityMap_.size());
    }
    if (abilityStateScheduler_ == nullptr) {
        HILOGE("abilityStateScheduler is nullptr");
        return ERR_INVALID_VALUE;
    }
    abilityStateScheduler_->SendAbilityStateEvent(systemAbilityId, AbilityStateEvent::ABILITY_UNLOAD_SUCCESS_EVENT);
    SendSystemAbilityRemovedMsg(systemAbilityId);
    return ERR_OK;
}

int32_t SystemAbilityManager::RemoveSystemAbility(const sptr<IRemoteObject>& ability)
{
    HILOGI("%s called, (ability)", __func__);
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
                HILOGI("%s called, systemAbilityId:%{public}d removed, size : %{public}zu", __func__, saId,
                    abilityMap_.size());
                break;
            }
        }
    }

    if (saId != 0) {
        if (abilityStateScheduler_ == nullptr) {
            HILOGE("abilityStateScheduler is nullptr");
            return ERR_INVALID_VALUE;
        }
        abilityStateScheduler_->HandleAbilityDiedEvent(saId);
        SendSystemAbilityRemovedMsg(saId);
    }
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

int32_t SystemAbilityManager::SubscribeSystemAbility(int32_t systemAbilityId,
    const sptr<ISystemAbilityStatusChange>& listener)
{
    if (!CheckInputSysAbilityId(systemAbilityId) || listener == nullptr) {
        HILOGW("SubscribeSystemAbility systemAbilityId or listener invalid!");
        return ERR_INVALID_VALUE;
    }

    auto callingPid = IPCSkeleton::GetCallingPid();
    {
        lock_guard<recursive_mutex> autoLock(listenerMapLock_);
        auto& listeners = listenerMap_[systemAbilityId];
        for (const auto& itemListener : listeners) {
            if (listener->AsObject() == itemListener.first->AsObject()) {
                HILOGI("already exist listener object systemAbilityId = %{public}d", systemAbilityId);
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
            HILOGI("SubscribeSystemAbility systemAbilityId = %{public}d AddDeathRecipient %{public}s",
                systemAbilityId, ret ? "succeed" : "failed");
        }
        HILOGI("SubscribeSystemAbility systemAbilityId = %{public}d, size = %{public}zu", systemAbilityId,
            listeners.size());
    }
    sptr<IRemoteObject> targetObject = CheckSystemAbility(systemAbilityId);
    if (targetObject != nullptr) {
        NotifySystemAbilityChanged(systemAbilityId, "",
            static_cast<uint32_t>(SamgrInterfaceCode::ADD_SYSTEM_ABILITY_TRANSACTION), listener);
    }
    return ERR_OK;
}

void SystemAbilityManager::UnSubscribeSystemAbilityLocked(
    std::list<std::pair<sptr<ISystemAbilityStatusChange>, int32_t>>& listenerList,
    const sptr<IRemoteObject>& listener)
{
    auto iter = listenerList.begin();
    while (iter != listenerList.end()) {
        auto& item = *iter;
        if (item.first->AsObject() != listener) {
            ++iter;
            continue;
        }

        if (abilityStatusDeath_ != nullptr) {
            listener->RemoveDeathRecipient(abilityStatusDeath_);
        }
        auto iterPair = subscribeCountMap_.find(item.second);
        if (iterPair != subscribeCountMap_.end()) {
            --iterPair->second;
            if (iterPair->second == 0) {
                subscribeCountMap_.erase(iterPair);
            }
        }
        HILOGI("Remove the systemAbilityStatus listener added by callingPid: %{public}d", item.second);
        iter = listenerList.erase(iter);
        break;
    }
}

int32_t SystemAbilityManager::UnSubscribeSystemAbility(int32_t systemAbilityId,
    const sptr<ISystemAbilityStatusChange>& listener)
{
    if (!CheckInputSysAbilityId(systemAbilityId) || listener == nullptr) {
        HILOGW("UnSubscribeSystemAbility systemAbilityId or listener invalid!");
        return ERR_INVALID_VALUE;
    }

    lock_guard<recursive_mutex> autoLock(listenerMapLock_);
    auto& listeners = listenerMap_[systemAbilityId];
    UnSubscribeSystemAbilityLocked(listeners, listener->AsObject());
    HILOGI("UnSubscribeSystemAbility systemAbilityId = %{public}d, size = %{public}zu", systemAbilityId,
        listeners.size());
    return ERR_OK;
}

void SystemAbilityManager::UnSubscribeSystemAbility(const sptr<IRemoteObject>& remoteObject)
{
    lock_guard<recursive_mutex> autoLock(listenerMapLock_);
    for (auto& item : listenerMap_) {
        auto& listeners = item.second;
        UnSubscribeSystemAbilityLocked(listeners, remoteObject);
    }
    HILOGI("UnSubscribeSystemAbility remote object dead!");
}

void SystemAbilityManager::SetDeviceName(const u16string &name)
{
    deviceName_ = name;
}

const u16string& SystemAbilityManager::GetDeviceName() const
{
    return deviceName_;
}

void SystemAbilityManager::NotifyRemoteSaDied(const std::u16string& name)
{
    std::u16string saName;
    std::string deviceId;
    ParseRemoteSaName(name, deviceId, saName);
    if (dBinderService_ != nullptr) {
        std::string nodeId = TransformDeviceId(deviceId, NODE_ID, false);
        dBinderService_->NoticeServiceDie(saName, nodeId);
        HILOGI("NotifyRemoteSaDied, serviceName is %s, deviceId is %s",
            Str16ToStr8(saName).c_str(), nodeId.c_str());
    }
}

void SystemAbilityManager::NotifyRemoteDeviceOffline(const std::string& deviceId)
{
    if (dBinderService_ != nullptr) {
        dBinderService_->NoticeDeviceDie(deviceId);
        HILOGI("NotifyRemoteDeviceOffline, deviceId is %s", deviceId.c_str());
    }
}

void SystemAbilityManager::ParseRemoteSaName(const std::u16string& name, std::string& deviceId, std::u16string& saName)
{
    vector<string> strVector;
    SplitStr(Str16ToStr8(name), "_", strVector);
    if (strVector.size() == SPLIT_NAME_VECTOR_SIZE) {
        deviceId = strVector[0];
        saName = Str8ToStr16(strVector[1]);
    }
}

int32_t SystemAbilityManager::AddSystemAbility(int32_t systemAbilityId, const sptr<IRemoteObject>& ability,
    const SAExtraProp& extraProp)
{
    if (!CheckInputSysAbilityId(systemAbilityId) || ability == nullptr) {
        HILOGE("AddSystemAbilityExtra input params is invalid.");
        return ERR_INVALID_VALUE;
    }
    {
        unique_lock<shared_mutex> writeLock(abilityMapLock_);
        auto saSize = abilityMap_.size();
        if (saSize >= MAX_SERVICES) {
            HILOGE("map size error, (Has been greater than %zu)", saSize);
            return ERR_INVALID_VALUE;
        }
        SAInfo saInfo;
        saInfo.remoteObj = ability;
        saInfo.isDistributed = extraProp.isDistributed;
        saInfo.capability = extraProp.capability;
        saInfo.permission = Str16ToStr8(extraProp.permission);
        if (abilityMap_.count(systemAbilityId) > 0) {
            auto callingPid = IPCSkeleton::GetCallingPid();
            auto callingUid = IPCSkeleton::GetCallingUid();
            HILOGW("systemAbility: %{public}d is being covered, callingPid is %{public}d, callingUid is %{public}d",
                systemAbilityId, callingPid, callingUid);
        }
        abilityMap_[systemAbilityId] = std::move(saInfo);
        HILOGI("insert %{public}d. size : %{public}zu", systemAbilityId, abilityMap_.size());
    }
    RemoveCheckLoadedMsg(systemAbilityId);
    if (abilityDeath_ != nullptr) {
        ability->AddDeathRecipient(abilityDeath_);
    }

    u16string strName = Str8ToStr16(to_string(systemAbilityId));
    if (extraProp.isDistributed && dBinderService_ != nullptr) {
        dBinderService_->RegisterRemoteProxy(strName, systemAbilityId);
        HILOGI("AddSystemAbility RegisterRemoteProxy, serviceId is %{public}d", systemAbilityId);
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
    abilityStateScheduler_->SendAbilityStateEvent(systemAbilityId, AbilityStateEvent::ABILITY_LOAD_SUCCESS_EVENT);
    SendSystemAbilityAddedMsg(systemAbilityId, ability);
    return ERR_OK;
}

int32_t SystemAbilityManager::AddSystemProcess(const u16string& procName,
    const sptr<IRemoteObject>& procObject)
{
    if (procName.empty() || procObject == nullptr) {
        HILOGE("AddSystemProcess empty name or null object!");
        return ERR_INVALID_VALUE;
    }

    {
        lock_guard<recursive_mutex> autoLock(onDemandLock_);
        size_t procNum = systemProcessMap_.size();
        if (procNum >= MAX_SERVICES) {
            HILOGE("AddSystemProcess map size reach MAX_SERVICES already");
            return ERR_INVALID_VALUE;
        }
        systemProcessMap_[procName] = procObject;
        if (systemProcessDeath_ != nullptr) {
            bool ret = procObject->AddDeathRecipient(systemProcessDeath_);
            HILOGW("AddSystemProcess AddDeathRecipient %{public}s!", ret ? "succeed" : "failed");
        }
        HILOGI("AddSystemProcess insert %{public}s. size : %{public}zu", Str16ToStr8(procName).c_str(),
            systemProcessMap_.size());
        auto iterStarting = startingProcessMap_.find(procName);
        if (iterStarting != startingProcessMap_.end()) {
            int64_t end = GetTickCount();
            HILOGI("[PerformanceTest] AddSystemProcess start process:%{public}s spend %{public}" PRId64 " ms",
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
    {
        lock_guard<recursive_mutex> autoLock(onDemandLock_);
        for (const auto& [procName, object] : systemProcessMap_) {
            if (object == procObject) {
                if (systemProcessDeath_ != nullptr) {
                    procObject->RemoveDeathRecipient(systemProcessDeath_);
                }
                std::string name = Str16ToStr8(procName);
                processName = procName;
                (void)systemProcessMap_.erase(procName);
                HILOGI("RemoveSystemProcess process:%{public}s dead, size : %{public}zu", name.c_str(),
                    systemProcessMap_.size());
                result = ERR_OK;
                break;
            }
        }
    }
    if (result == ERR_OK) {
        // Waiting for the init subsystem to perceive process death
        ServiceWaitForStatus(Str16ToStr8(processName).c_str(), ServiceStatus::SERVICE_STOPPED, 1);

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

    lock_guard<recursive_mutex> autoLock(onDemandLock_);
    auto iter = systemProcessMap_.find(procName);
    if (iter != systemProcessMap_.end()) {
        HILOGD("process:%{public}s found", Str16ToStr8(procName).c_str());
        return iter->second;
    }
    HILOGE("process:%{public}s not exist", Str16ToStr8(procName).c_str());
    return nullptr;
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
        HILOGE("SendSystemAbilityAddedMsg work handler not initialized!");
        return;
    }
    auto notifyAddedTask = [systemAbilityId, remoteObject, this]() {
        FindSystemAbilityNotify(systemAbilityId,
            static_cast<uint32_t>(SamgrInterfaceCode::ADD_SYSTEM_ABILITY_TRANSACTION));
        NotifySystemAbilityLoaded(systemAbilityId, remoteObject);
    };
    bool ret = workHandler_->PostTask(notifyAddedTask);
    if (!ret) {
        HILOGW("SendSystemAbilityAddedMsg PostTask failed!");
    }
}

void SystemAbilityManager::SendSystemAbilityRemovedMsg(int32_t systemAbilityId)
{
    if (workHandler_ == nullptr) {
        HILOGE("SendSystemAbilityRemovedMsg work handler not initialized!");
        return;
    }
    auto notifyRemovedTask = [systemAbilityId, this]() {
        FindSystemAbilityNotify(systemAbilityId,
            static_cast<uint32_t>(SamgrInterfaceCode::REMOVE_SYSTEM_ABILITY_TRANSACTION));
    };
    bool ret = workHandler_->PostTask(notifyRemovedTask);
    if (!ret) {
        HILOGW("SendSystemAbilityRemovedMsg PostTask failed!");
    }
}

void SystemAbilityManager::SendCheckLoadedMsg(int32_t systemAbilityId, const std::u16string& name,
    const std::string& srcDeviceId, const sptr<ISystemAbilityLoadCallback>& callback)
{
    if (workHandler_ == nullptr) {
        HILOGE("SendCheckLoadedMsg work handler not initialized!");
        return;
    }

    auto delayTask = [systemAbilityId, name, srcDeviceId, callback, this]() {
        HILOGI("SendCheckLoadedMsg handle for SA : %{public}d.", systemAbilityId);
        if (CheckSystemAbility(systemAbilityId) != nullptr) {
            HILOGI("SendCheckLoadedMsg SA : %{public}d loaded.", systemAbilityId);
            return;
        }
        CleanCallbackForLoadFailed(systemAbilityId, name, srcDeviceId, callback);
        if (abilityStateScheduler_ == nullptr) {
            HILOGE("abilityStateScheduler is nullptr");
            return;
        }
        abilityStateScheduler_->SendAbilityStateEvent(systemAbilityId, AbilityStateEvent::ABILITY_LOAD_FAILED_EVENT);
        (void)GetSystemProcess(name);
    };
    bool ret = workHandler_->PostTask(delayTask, ToString(systemAbilityId), CHECK_LOADED_DELAY_TIME);
    HILOGI("SendCheckLoadedMsg PostTask name : %{public}d!, ret : %{public}s",
        systemAbilityId, ret ? "success" : "failed");
}

void SystemAbilityManager::CleanCallbackForLoadFailed(int32_t systemAbilityId, const std::u16string& name,
    const std::string& srcDeviceId, const sptr<ISystemAbilityLoadCallback>& callback)
{
    lock_guard<recursive_mutex> autoLock(onDemandLock_);
    auto iterStarting = startingProcessMap_.find(name);
    if (iterStarting != startingProcessMap_.end()) {
        HILOGI("CleanCallback clean process:%{public}s", Str16ToStr8(name).c_str());
        startingProcessMap_.erase(iterStarting);
    }
    auto iter = startingAbilityMap_.find(systemAbilityId);
    if (iter == startingAbilityMap_.end()) {
        HILOGI("CleanCallback SA : %{public}d not in startingAbilityMap.", systemAbilityId);
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
        HILOGI("CleanCallback startingAbilityMap remove SA : %{public}d. with deviceId", systemAbilityId);
        abilityItem.callbackMap.erase(srcDeviceId);
    }

    if (abilityItem.callbackMap.empty()) {
        HILOGI("CleanCallback startingAbilityMap remove SA : %{public}d.", systemAbilityId);
        startingAbilityMap_.erase(iter);
    }
}

void SystemAbilityManager::RemoveCheckLoadedMsg(int32_t systemAbilityId)
{
    if (workHandler_ == nullptr) {
        HILOGE("RemoveCheckLoadedMsg work handler not initialized!");
        return;
    }
    workHandler_->RemoveTask(ToString(systemAbilityId));
}

void SystemAbilityManager::SendLoadedSystemAbilityMsg(int32_t systemAbilityId, const sptr<IRemoteObject>& remoteObject,
    const sptr<ISystemAbilityLoadCallback>& callback)
{
    if (workHandler_ == nullptr) {
        HILOGE("SendLoadedSystemAbilityMsg work handler not initialized!");
        return;
    }
    auto notifyLoadedTask = [systemAbilityId, remoteObject, callback, this]() {
        NotifySystemAbilityLoaded(systemAbilityId, remoteObject, callback);
    };
    bool ret = workHandler_->PostTask(notifyLoadedTask);
    if (!ret) {
        HILOGW("SendLoadedSystemAbilityMsg PostTask failed!");
    }
}

void SystemAbilityManager::NotifySystemAbilityLoaded(int32_t systemAbilityId, const sptr<IRemoteObject>& remoteObject,
    const sptr<ISystemAbilityLoadCallback>& callback)
{
    if (callback == nullptr) {
        HILOGE("NotifySystemAbilityLoaded callback null!");
        return;
    }
    HILOGI("NotifySystemAbilityLoaded systemAbilityId : %{public}d", systemAbilityId);
    callback->OnLoadSystemAbilitySuccess(systemAbilityId, remoteObject);
}

void SystemAbilityManager::NotifySystemAbilityLoaded(int32_t systemAbilityId, const sptr<IRemoteObject>& remoteObject)
{
    lock_guard<recursive_mutex> autoLock(onDemandLock_);
    auto iter = startingAbilityMap_.find(systemAbilityId);
    if (iter == startingAbilityMap_.end()) {
        return;
    }
    auto& abilityItem = iter->second;
    for (auto& [deviceId, callbackList] : abilityItem.callbackMap) {
        for (auto& callbackItem : callbackList) {
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
        HILOGE("NotifySystemAbilityLoadFail callback null!");
        return;
    }
    HILOGI("NotifySystemAbilityLoadFailed systemAbilityId : %{public}d", systemAbilityId);
    callback->OnLoadSystemAbilityFail(systemAbilityId);
}

int32_t SystemAbilityManager::StartDynamicSystemProcess(const std::u16string& name,
    int32_t systemAbilityId, const OnDemandEvent& event)
{
    std::string eventStr = std::to_string(systemAbilityId) + "#" + std::to_string(event.eventId) + "#"
        + event.name + "#" + event.value + "#" + std::to_string(event.extraDataId) + "#";
    auto extraArgv = eventStr.c_str();
    auto result = ServiceControlWithExtra(Str16ToStr8(name).c_str(), ServiceAction::START, &extraArgv, 1);
    HILOGI("StartDynamicSystemProcess call ServiceControlWithExtra result:%{public}d!", result);
    return (result == 0) ? ERR_OK : ERR_INVALID_VALUE;
}

int32_t SystemAbilityManager::StartingSystemProcess(const std::u16string& procName,
    int32_t systemAbilityId, const OnDemandEvent& event)
{
    lock_guard<recursive_mutex> autoLock(onDemandLock_);
    if (startingProcessMap_.count(procName) != 0) {
        HILOGI("StartingSystemProcess process:%{public}s already starting!", Str16ToStr8(procName).c_str());
        return ERR_OK;
    }
    auto iter = systemProcessMap_.find(procName);
    if (iter != systemProcessMap_.end()) {
        bool isExist = false;
        StartOnDemandAbility(systemAbilityId, isExist);
        if (!isExist) {
            HILOGE("not found onDemandAbility: %{public}d.", systemAbilityId);
        }
        return ERR_OK;
    }
    // call init start process
    int64_t begin = GetTickCount();
    int32_t result = StartDynamicSystemProcess(procName, systemAbilityId, event);
    if (result == ERR_OK) {
        startingProcessMap_.emplace(procName, begin);
    }
    return result;
}

int32_t SystemAbilityManager::DoLoadSystemAbility(int32_t systemAbilityId, const std::u16string& procName,
    const sptr<ISystemAbilityLoadCallback>& callback, int32_t callingPid, const OnDemandEvent& event)
{
    int32_t result = ERR_INVALID_VALUE;
    {
        lock_guard<recursive_mutex> autoLock(onDemandLock_);
        sptr<IRemoteObject> targetObject = CheckSystemAbility(systemAbilityId);
        if (targetObject != nullptr) {
            NotifySystemAbilityLoaded(systemAbilityId, targetObject, callback);
            return ERR_OK;
        }
        auto& abilityItem = startingAbilityMap_[systemAbilityId];
        for (const auto& itemCallback : abilityItem.callbackMap[LOCAL_DEVICE]) {
            if (callback->AsObject() == itemCallback.first->AsObject()) {
                HILOGI("LoadSystemAbility already existed callback object systemAbilityId:%{public}d", systemAbilityId);
                return ERR_OK;
            }
        }
        auto& count = callbackCountMap_[callingPid];
        if (count >= MAX_SUBSCRIBE_COUNT) {
            HILOGE("LoadSystemAbility pid:%{public}d overflow max callback count!", callingPid);
            return ERR_PERMISSION_DENIED;
        }
        ++count;
        abilityItem.callbackMap[LOCAL_DEVICE].emplace_back(callback, callingPid);
        abilityItem.event = event;
        if (abilityCallbackDeath_ != nullptr) {
            bool ret = callback->AsObject()->AddDeathRecipient(abilityCallbackDeath_);
            HILOGI("LoadSystemAbility systemAbilityId:%{public}d AddDeathRecipient %{public}s",
                systemAbilityId, ret ? "succeed" : "failed");
        }
        result = StartingSystemProcess(procName, systemAbilityId, event);
        HILOGI("LoadSystemAbility systemAbilityId:%{public}d size : %{public}zu",
            systemAbilityId, abilityItem.callbackMap[LOCAL_DEVICE].size());
    }
    SendCheckLoadedMsg(systemAbilityId, procName, LOCAL_DEVICE, callback);
    return result;
}

int32_t SystemAbilityManager::DoLoadSystemAbilityFromRpc(const std::string& srcDeviceId, int32_t systemAbilityId,
    const std::u16string& procName, const sptr<ISystemAbilityLoadCallback>& callback, const OnDemandEvent& event)
{
    {
        lock_guard<recursive_mutex> autoLock(onDemandLock_);
        sptr<IRemoteObject> targetObject = CheckSystemAbility(systemAbilityId);
        if (targetObject != nullptr) {
            SendLoadedSystemAbilityMsg(systemAbilityId, targetObject, callback);
            return ERR_OK;
        }
        auto& abilityItem = startingAbilityMap_[systemAbilityId];
        abilityItem.callbackMap[srcDeviceId].emplace_back(callback, 0);
        StartingSystemProcess(procName, systemAbilityId, event);
    }
    SendCheckLoadedMsg(systemAbilityId, procName, srcDeviceId, callback);
    return ERR_OK;
}

int32_t SystemAbilityManager::LoadSystemAbility(int32_t systemAbilityId,
    const sptr<ISystemAbilityLoadCallback>& callback)
{
    if (!CheckInputSysAbilityId(systemAbilityId) || callback == nullptr) {
        HILOGW("LoadSystemAbility systemAbilityId or callback invalid!");
        return ERR_INVALID_VALUE;
    }
    SaProfile saProfile;
    bool ret = GetSaProfile(systemAbilityId, saProfile);
    if (!ret) {
        HILOGE("LoadSystemAbility systemAbilityId:%{public}d not supported!", systemAbilityId);
        return ERR_INVALID_VALUE;
    }
    auto callingPid = IPCSkeleton::GetCallingPid();
    OnDemandEvent onDemandEvent = {INTERFACE_CALL, "load"};
    LoadRequestInfo loadRequestInfo = {systemAbilityId, LOCAL_DEVICE, callback, callingPid, onDemandEvent};
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
        HILOGE("LoadSystemAbilityFromRpc said:%{public}d not supported!", systemAbilityId);
        return false;
    }

    if (!saProfile.distributed) {
        HILOGE("LoadSystemAbilityFromRpc said:%{public}d not distributed!", systemAbilityId);
        return false;
    }
    OnDemandEvent onDemandEvent = {INTERFACE_CALL, "loadFromRpc"};
    LoadRequestInfo loadRequestInfo = {systemAbilityId, srcDeviceId, callback, -1, onDemandEvent};
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
        HILOGE("UnloadSystemAbility systemAbilityId:%{public}d not supported!", systemAbilityId);
        return ERR_INVALID_VALUE;
    }
    if (!CheckCallerProcess(saProfile)) {
        HILOGE("UnloadSystemAbility invalid caller process, saId: %{public}d", systemAbilityId);
        return ERR_INVALID_VALUE;
    }
    if (abilityStateScheduler_ == nullptr) {
        HILOGE("abilityStateScheduler is nullptr");
        return ERR_INVALID_VALUE;
    }
    OnDemandEvent onDemandEvent = {INTERFACE_CALL, "unload"};
    UnloadRequestInfo unloadRequestInfo = {systemAbilityId, onDemandEvent};
    return abilityStateScheduler_->HandleUnloadAbilityEvent(unloadRequestInfo);
}

int32_t SystemAbilityManager::CancelUnloadSystemAbility(int32_t systemAbilityId)
{
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("CancelUnloadSystemAbility systemAbilityId or callback invalid!");
        return ERR_INVALID_VALUE;
    }
    SaProfile saProfile;
    bool ret = GetSaProfile(systemAbilityId, saProfile);
    if (!ret) {
        HILOGE("CancelUnloadSystemAbility systemAbilityId:%{public}d not supported!", systemAbilityId);
        return ERR_INVALID_VALUE;
    }
    if (!CheckCallerProcess(saProfile)) {
        HILOGE("CancelUnloadSystemAbility invalid caller process, saId: %{public}d", systemAbilityId);
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
    lock_guard<recursive_mutex> autoLock(onDemandLock_);
    sptr<IRemoteObject> targetObject = CheckSystemAbility(systemAbilityId);
    if (targetObject == nullptr) {
        return ERR_OK;
    }
    bool result = StopOnDemandAbility(procName, systemAbilityId, event);
    if (!result) {
        HILOGE("unload system ability failed, systemAbilityId: %{public}d", systemAbilityId);
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

bool SystemAbilityManager::IdleSystemAbility(int32_t systemAbilityId, const std::u16string& procName,
    const nlohmann::json& idleReason, int32_t& delayTime)
{
    sptr<IRemoteObject> targetObject = CheckSystemAbility(systemAbilityId);
    if (targetObject == nullptr) {
        HILOGE("systemAbility :%{public}d not loaded", systemAbilityId);
        return false;
    }
    sptr<ILocalAbilityManager> procObject =
        iface_cast<ILocalAbilityManager>(GetSystemProcess(procName));
    if (procObject == nullptr) {
        HILOGE("get process:%{public}s fail", Str16ToStr8(procName).c_str());
        return false;
    }
    return procObject->IdleAbility(systemAbilityId, idleReason, delayTime);
}

bool SystemAbilityManager::ActiveSystemAbility(int32_t systemAbilityId, const std::u16string& procName,
    const nlohmann::json& activeReason)
{
    sptr<IRemoteObject> targetObject = CheckSystemAbility(systemAbilityId);
    if (targetObject == nullptr) {
        HILOGE("systemAbility :%{public}d not loaded", systemAbilityId);
        return false;
    }
    sptr<ILocalAbilityManager> procObject =
        iface_cast<ILocalAbilityManager>(GetSystemProcess(procName));
    if (procObject == nullptr) {
        HILOGE("get process:%{public}s fail", Str16ToStr8(procName).c_str());
        return false;
    }
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
            HILOGI("LoadSystemAbility already existed callback object systemAbilityId:%{public}d", systemAbilityId);
            return ERR_OK;
        }
        if (remoteCallbackDeath_ != nullptr) {
            bool ret = callback->AsObject()->AddDeathRecipient(remoteCallbackDeath_);
            HILOGI("LoadSystemAbility systemAbilityId:%{public}d AddDeathRecipient %{public}s",
                systemAbilityId, ret ? "succeed" : "failed");
        }
        callbacks.emplace_back(callback);
    }
    auto callingPid = IPCSkeleton::GetCallingPid();
    auto callingUid = IPCSkeleton::GetCallingUid();
    auto task = std::bind(&SystemAbilityManager::DoLoadRemoteSystemAbility, this,
        systemAbilityId, callingPid, callingUid, deviceId, callback);
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
        HILOGI("DoLoadRemoteSystemAbility return, callback is nullptr, said : %{public}d", systemAbilityId);
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
    HILOGI("MakeRemoteBinder begin, said : %{public}d", systemAbilityId);
    std::string networkId = deviceId;
#ifdef SUPPORT_DEVICE_MANAGER
    DeviceIdToNetworkId(networkId);
#endif
    sptr<DBinderServiceStub> remoteBinder = nullptr;
    if (dBinderService_ != nullptr) {
        string strName = to_string(systemAbilityId);
        remoteBinder = dBinderService_->MakeRemoteBinder(Str8ToStr16(strName),
            networkId, systemAbilityId, callingPid, callingUid);
    }
    HILOGI("MakeRemoteBinder end, result %{public}s, said : %{public}d, networkId : %{public}s",
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
            dBinderService_->LoadSystemAbilityComplete(srcDeviceId, systemAbilityId, remoteObject);
            return;
        }
        HILOGW("NotifyRpcLoadCompleted failed, said: %{public}d, deviceId : %{public}s",
            systemAbilityId, AnonymizeDeviceId(srcDeviceId).c_str());
    };
    bool ret = workHandler_->PostTask(notifyTask);
    if (!ret) {
        HILOGW("NotifyRpcLoadCompleted PostTask failed!");
    }
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
    lock_guard<recursive_mutex> autoLock(onDemandLock_);
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

std::string SystemAbilityManager::TransformDeviceId(const std::string& deviceId, int32_t type, bool isPrivate)
{
    return isPrivate ? std::string() : deviceId;
}

std::string SystemAbilityManager::GetLocalNodeId()
{
    return std::string();
}

std::string SystemAbilityManager::EventToStr(const OnDemandEvent& event)
{
    nlohmann::json eventJson;
    eventJson[EVENT_TYPE] = event.eventId;
    eventJson[EVENT_NAME] = event.name;
    eventJson[EVENT_VALUE] = event.value;
    eventJson[EVENT_EXTRA_DATA_ID] = event.extraDataId;
    std::string eventStr = eventJson.dump();
    return eventStr;
}

void SystemAbilityManager::UpdateSaFreMap(int32_t uid, int32_t saId)
{
    if (uid < 0) {
        HILOGW("UpdateSaFreMap return, uid not valid!");
        return;
    }

    uint64_t key = GenerateFreKey(uid, saId);
    lock_guard<mutex> autoLock(saFrequencyLock_);
    auto& count = saFrequencyMap_[key];
    if (count < MAX_SA_FREQUENCY_COUNT) {
        count++;
    }
}

uint64_t SystemAbilityManager::GenerateFreKey(int32_t uid, int32_t saId) const
{
    uint32_t uSaid = static_cast<uint32_t>(saId);
    uint64_t key = static_cast<uint64_t>(uid);
    return (key << SHFIT_BIT) | uSaid;
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
} // namespace OHOS
