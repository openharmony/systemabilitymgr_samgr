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

#include <algorithm>

#include "ability_death_recipient.h"
#include "datetime_ex.h"
#include "ipc_skeleton.h"
#include "memory_guard.h"
#include "sam_log.h"
#include "hisysevent_adapter.h"
#include "schedule/system_ability_state_scheduler.h"
#include "service_control.h"
#include "samgr_err_code.h"
#include "string_ex.h"
#include "system_ability_manager.h"
#include "samgr_xcollie.h"
#include "parameters.h"
#include "system_ability_manager_util.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace {
constexpr int64_t RESTART_TIME_INTERVAL_LIMIT = 20 * 1000;
constexpr int32_t RESTART_TIMES_LIMIT = 4;
constexpr int32_t MAX_SUBSCRIBE_COUNT = 256;
constexpr int32_t UNLOAD_TIMEOUT_TIME = 5 * 1000;
constexpr const char* LOCAL_DEVICE = "local";
constexpr int32_t MAX_DELAY_TIME = 5 * 60 * 1000;
constexpr int32_t MAX_DURATION = 10 * 60 * 1000; // ms
constexpr int32_t ONCE_DELAY_TIME = 10 * 1000; // ms
constexpr const char* CANCEL_UNLOAD = "cancelUnload";
constexpr const char* KEY_EVENT_ID = "eventId";
constexpr const char* KEY_NAME = "name";
constexpr const char* KEY_VALUE = "value";
constexpr const char* KEY_EXTRA_DATA_ID = "extraDataId";
constexpr const char* KEY_UNLOAD_TIMEOUT = "unloadTimeout";
const std::u16string SAMGR_PROCESS_NAME = u"samgr";
constexpr const char *SA_STATE_ENUM_STR[] = {
    "NOT_LOADED", "LOADING", "LOADED", "UNLOADABLE", "UNLOADING" };
constexpr const char *PROCESS_STATE_ENUM_STR[] = {
    "NOT_STARTED", "STARTED", "STOPPING" };
constexpr const char *PENDINGEVENT_ENUM_STR[] = {
    "NO_EVENT", "LOAD_ABILITY_EVENT", "UNLOAD_ABILITY_EVENT" };
}
void SystemAbilityStateScheduler::Init(const std::list<SaProfile>& saProfiles)
{
    HILOGI("Scheduler:init start");
    InitStateContext(saProfiles);
    processListenerDeath_ = sptr<IRemoteObject::DeathRecipient>(new SystemProcessListenerDeathRecipient());
    unloadEventHandler_ = std::make_shared<UnloadEventHandler>(weak_from_this());

    auto listener =  std::dynamic_pointer_cast<SystemAbilityStateListener>(shared_from_this());
    stateMachine_ = std::make_shared<SystemAbilityStateMachine>(listener);
    stateEventHandler_ = std::make_shared<SystemAbilityEventHandler>(stateMachine_);

    processHandler_ = std::make_shared<FFRTHandler>("ProcessHandler");
    HILOGI("Scheduler:init end");
}

void SystemAbilityStateScheduler::CleanFfrt()
{
    if (processHandler_ != nullptr) {
        processHandler_->CleanFfrt();
    }
    if (unloadEventHandler_ != nullptr) {
        unloadEventHandler_->CleanFfrt();
    }
}

void SystemAbilityStateScheduler::SetFfrt()
{
    if (processHandler_ != nullptr) {
        processHandler_->SetFfrt("ProcessHandler");
    }
    if (unloadEventHandler_ != nullptr) {
        unloadEventHandler_->SetFfrt();
    }
}

void SystemAbilityStateScheduler::InitStateContext(const std::list<SaProfile>& saProfiles)
{
    for (auto& saProfile : saProfiles) {
        if (saProfile.process.empty()) {
            continue;
        }
        std::unique_lock<std::shared_mutex> processWriteLock(processMapLock_);
        if (processContextMap_.count(saProfile.process) == 0) {
            auto processContext = std::make_shared<SystemProcessContext>();
            processContext->processName = saProfile.process;
            processContext->abilityStateCountMap[SystemAbilityState::NOT_LOADED] = 0;
            processContext->abilityStateCountMap[SystemAbilityState::LOADING] = 0;
            processContext->abilityStateCountMap[SystemAbilityState::LOADED] = 0;
            processContext->abilityStateCountMap[SystemAbilityState::UNLOADABLE] = 0;
            processContext->abilityStateCountMap[SystemAbilityState::UNLOADING] = 0;
            processContextMap_[saProfile.process] = processContext;
        }
        processContextMap_[saProfile.process]->saList.push_back(saProfile.saId);
        processContextMap_[saProfile.process]->abilityStateCountMap[SystemAbilityState::NOT_LOADED]++;
        auto abilityContext = std::make_shared<SystemAbilityContext>();
        abilityContext->systemAbilityId = saProfile.saId;
        abilityContext->isAutoRestart = saProfile.autoRestart;
        int32_t delayUnloadTime = LimitDelayUnloadTime(saProfile.stopOnDemand.delayTime);
        abilityContext->delayUnloadTime = delayUnloadTime;
        abilityContext->ownProcessContext = processContextMap_[saProfile.process];
        std::unique_lock<std::shared_mutex> abiltyWriteLock(abiltyMapLock_);
        abilityContextMap_[saProfile.saId] = abilityContext;
    }
}

void SystemAbilityStateScheduler::InitSamgrProcessContext()
{
    auto processContext = std::make_shared<SystemProcessContext>();
    processContext->processName = SAMGR_PROCESS_NAME;
    processContext->abilityStateCountMap[SystemAbilityState::NOT_LOADED] = 0;
    processContext->abilityStateCountMap[SystemAbilityState::LOADING] = 0;
    processContext->abilityStateCountMap[SystemAbilityState::LOADED] = 1;
    processContext->abilityStateCountMap[SystemAbilityState::UNLOADABLE] = 0;
    processContext->abilityStateCountMap[SystemAbilityState::UNLOADING] = 0;
    processContext->pid = getpid();
    processContext->uid = static_cast<int32_t>(getuid());
    processContextMap_[SAMGR_PROCESS_NAME] = processContext;
    processContextMap_[SAMGR_PROCESS_NAME]->saList.push_back(0);
    processContext->state = SystemProcessState::STARTED;

    auto abilityContext = std::make_shared<SystemAbilityContext>();
    abilityContext->systemAbilityId = 0;
    abilityContext->isAutoRestart = false;
    abilityContext->delayUnloadTime = MAX_DELAY_TIME;
    abilityContext->ownProcessContext = processContextMap_[SAMGR_PROCESS_NAME];
    std::unique_lock<std::shared_mutex> abiltyWriteLock(abiltyMapLock_);
    abilityContextMap_[0] = abilityContext;
}

int32_t SystemAbilityStateScheduler::LimitDelayUnloadTime(int32_t delayUnloadTime)
{
    if (delayUnloadTime < 0) {
        return 0;
    }
    if (delayUnloadTime > MAX_DELAY_TIME) {
        return MAX_DELAY_TIME;
    }
    return delayUnloadTime;
}

bool SystemAbilityStateScheduler::GetSystemAbilityContext(int32_t systemAbilityId,
    std::shared_ptr<SystemAbilityContext>& abilityContext)
{
    std::shared_lock<std::shared_mutex> readLock(abiltyMapLock_);
    if (abilityContextMap_.count(systemAbilityId) == 0) {
        HILOGD("Scheduler SA:%{public}d not in SA profiles", systemAbilityId);
        return false;
    }
    abilityContext = abilityContextMap_[systemAbilityId];
    if (abilityContext == nullptr) {
        HILOGE("Scheduler SA:%{public}d context is null", systemAbilityId);
        return false;
    }
    if (abilityContext->ownProcessContext == nullptr) {
        HILOGE("Scheduler SA:%{public}d not in any proc", systemAbilityId);
        return false;
    }
    return true;
}

void SystemAbilityStateScheduler::UpdateLimitDelayUnloadTime(int32_t systemAbilityId)
{
    if (processHandler_ == nullptr) {
        HILOGE("UpdateLimitDelayUnloadTime process handler not init");
        return;
    }
    auto UpdateDelayUnloadTimeTask = [systemAbilityId, this]() {
        UpdateLimitDelayUnloadTimeTask(systemAbilityId);
    };
    bool ret = processHandler_->PostTask(UpdateDelayUnloadTimeTask);
    if (!ret) {
        HILOGW("UpdateLimitDelayUnloadTime PostTask fail");
    }
}

void SystemAbilityStateScheduler::UpdateLimitDelayUnloadTimeTask(int32_t systemAbilityId)
{
    std::shared_ptr<SystemAbilityContext> abilityContext;
    if (!GetSystemAbilityContext(systemAbilityId, abilityContext)) {
        return;
    }
    std::lock_guard<std::mutex> autoLock(abilityContext->ownProcessContext->processLock);
    if (abilityContext->lastStartTime != 0) {
        int64_t begin = abilityContext->lastStartTime;
        int64_t end = GetTickCount();
        if (end - begin <= MAX_DURATION) {
            int64_t onceDelayTime = abilityContext->delayUnloadTime;
            onceDelayTime += ONCE_DELAY_TIME;
            abilityContext->delayUnloadTime = LimitDelayUnloadTime(onceDelayTime);
            HILOGI("DelayUnloadTime is %{public}d, SA:%{public}d",
                abilityContext->delayUnloadTime, systemAbilityId);
        }
    }
    abilityContext->lastStartTime = GetTickCount();
}

bool SystemAbilityStateScheduler::GetSystemProcessContext(const std::u16string& processName,
    std::shared_ptr<SystemProcessContext>& processContext)
{
    std::shared_lock<std::shared_mutex> readLock(processMapLock_);
    if (processContextMap_.count(processName) == 0) {
        HILOGE("Scheduler proc:%{public}s invalid", Str16ToStr8(processName).c_str());
        return false;
    }
    processContext = processContextMap_[processName];
    if (processContext == nullptr) {
        HILOGE("Scheduler proc:%{public}s context is null", Str16ToStr8(processName).c_str());
        return false;
    }
    return true;
}

bool SystemAbilityStateScheduler::IsSystemAbilityUnloading(int32_t systemAbilityId)
{
    std::shared_ptr<SystemAbilityContext> abilityContext;
    if (!GetSystemAbilityContext(systemAbilityId, abilityContext)) {
        return false;
    }
    std::lock_guard<std::mutex> autoLock(abilityContext->ownProcessContext->processLock);
    if (abilityContext->state ==SystemAbilityState::UNLOADING
        || abilityContext->ownProcessContext->state == SystemProcessState::STOPPING) {
        return true;
    }
    return false;
}

int32_t SystemAbilityStateScheduler::HandleLoadAbilityEvent(int32_t systemAbilityId, bool& isExist)
{
    std::shared_ptr<SystemAbilityContext> abilityContext;
    if (!GetSystemAbilityContext(systemAbilityId, abilityContext)) {
        isExist = false;
        return ERR_INVALID_VALUE;
    }
    std::lock_guard<std::mutex> autoLock(abilityContext->ownProcessContext->processLock);
    if (abilityContext->ownProcessContext->state == SystemProcessState::NOT_STARTED) {
        isExist = false;
        return ERR_INVALID_VALUE;
    }
    if (abilityContext->ownProcessContext->state == SystemProcessState::STARTED
        && abilityContext->state ==SystemAbilityState::NOT_LOADED) {
        HILOGD("Scheduler SA:%{public}d handle load event by check start", systemAbilityId);
        bool result = SystemAbilityManager::GetInstance()->DoLoadOnDemandAbility(systemAbilityId, isExist);
        if (result) {
            return stateMachine_->AbilityStateTransitionLocked(abilityContext, SystemAbilityState::LOADING);
        }
        return ERR_INVALID_VALUE;
    }
    isExist = true;
    return ERR_OK;
}

int32_t SystemAbilityStateScheduler::HandleLoadAbilityEvent(const LoadRequestInfo& loadRequestInfo)
{
    std::shared_ptr<SystemAbilityContext> abilityContext;
    if (!GetSystemAbilityContext(loadRequestInfo.systemAbilityId, abilityContext)) {
        return GET_SA_CONTEXT_FAIL;
    }
    HILOGI("Scheduler SA:%{public}d load event start,callpid:%{public}d,evtid:%{public}d,"
        "ProcSta:%{public}d,SASta:%{public}d", loadRequestInfo.systemAbilityId, loadRequestInfo.callingPid,
        loadRequestInfo.loadEvent.eventId, abilityContext->ownProcessContext->state, abilityContext->state);
    std::lock_guard<std::mutex> autoLock(abilityContext->ownProcessContext->processLock);
    int32_t result = HandleLoadAbilityEventLocked(abilityContext, loadRequestInfo);
    if (result != ERR_OK) {
        HILOGE("Scheduler SA:%{public}d handle load fail,ret:%{public}d",
            loadRequestInfo.systemAbilityId, result);
    }
    return result;
}

int32_t SystemAbilityStateScheduler::HandleLoadAbilityEventLocked(
    const std::shared_ptr<SystemAbilityContext>& abilityContext, const LoadRequestInfo& loadRequestInfo)
{
    if (abilityContext->state ==SystemAbilityState::UNLOADING
        || abilityContext->ownProcessContext->state == SystemProcessState::STOPPING) {
        return PendLoadEventLocked(abilityContext, loadRequestInfo);
    }
    nlohmann::json activeReason;
    activeReason[KEY_EVENT_ID] = loadRequestInfo.loadEvent.eventId;
    activeReason[KEY_NAME] = loadRequestInfo.loadEvent.name;
    activeReason[KEY_VALUE] = loadRequestInfo.loadEvent.value;
    activeReason[KEY_EXTRA_DATA_ID] = loadRequestInfo.loadEvent.extraDataId;
    int32_t result = INVALID_SA_STATE;
    switch (abilityContext->state) {
        case SystemAbilityState::LOADING:
            result = RemovePendingUnloadEventLocked(abilityContext);
            break;
        case SystemAbilityState::LOADED:
            result = RemoveDelayUnloadEventLocked(abilityContext->systemAbilityId);
            break;
        case SystemAbilityState::UNLOADABLE:
            result = ActiveSystemAbilityLocked(abilityContext, activeReason);
            break;
        case SystemAbilityState::NOT_LOADED:
            result = ERR_OK;
            break;
        default:
            result = INVALID_SA_STATE;
            HILOGI("Scheduler SA:%{public}d in state %{public}d, can't load SA",
                loadRequestInfo.systemAbilityId, abilityContext->state);
            break;
    }
    if (result == ERR_OK) {
        return DoLoadSystemAbilityLocked(abilityContext, loadRequestInfo);
    }
    return result;
}

int32_t SystemAbilityStateScheduler::HandleUnloadAbilityEvent(
    const std::shared_ptr<UnloadRequestInfo> unloadRequestInfo)
{
    if (unloadRequestInfo == nullptr) {
        HILOGE("Scheduler:HandleUnloadSaEvent unloadRequestInfo is null");
        return UNLOAD_REQUEST_NULL;
    }
    std::shared_ptr<SystemAbilityContext> abilityContext;
    if (!GetSystemAbilityContext(unloadRequestInfo->systemAbilityId, abilityContext)) {
        return GET_SA_CONTEXT_FAIL;
    }
    HILOGI("Scheduler SA:%{public}d handle unload start callPid:%{public}d,evtId:%{public}d,"
        "ProcSta:%{public}d,SASta:%{public}d", unloadRequestInfo->systemAbilityId, unloadRequestInfo->callingPid,
        unloadRequestInfo->unloadEvent.eventId, abilityContext->ownProcessContext->state, abilityContext->state);
    std::lock_guard<std::mutex> autoLock(abilityContext->ownProcessContext->processLock);
    int32_t result = HandleUnloadAbilityEventLocked(abilityContext, unloadRequestInfo);
    if (result != ERR_OK) {
        HILOGE("Scheduler SA:%{public}d handle unload fail,ret:%{public}d",
            unloadRequestInfo->systemAbilityId, result);
    }
    return result;
}

int32_t SystemAbilityStateScheduler::HandleUnloadAbilityEventLocked(
    const std::shared_ptr<SystemAbilityContext>& abilityContext,
    const std::shared_ptr<UnloadRequestInfo> unloadRequestInfo)
{
    if (unloadRequestInfo == nullptr) {
        HILOGE("Scheduler SA:%{public}d unloadRequestInfo is null", abilityContext->systemAbilityId);
        return UNLOAD_REQUEST_NULL;
    }
    abilityContext->unloadRequest = unloadRequestInfo;
    int32_t result = INVALID_SA_STATE;
    switch (abilityContext->state) {
        case SystemAbilityState::LOADING:
            result = PendUnloadEventLocked(abilityContext, unloadRequestInfo);
            break;
        case SystemAbilityState::LOADED:
            if (unloadRequestInfo->unloadEvent.eventId == INTERFACE_CALL) {
                result = ProcessDelayUnloadEventLocked(abilityContext->systemAbilityId);
            } else {
                result = SendDelayUnloadEventLocked(abilityContext->systemAbilityId, abilityContext->delayUnloadTime);
            }
            break;
        default:
            result = ERR_OK;
            HILOGI("Scheduler SA:%{public}d in state %{public}d,not need unload SA,callPid:%{public}d",
                abilityContext->systemAbilityId, abilityContext->state, unloadRequestInfo->callingPid);
            break;
    }
    return result;
}

int32_t SystemAbilityStateScheduler::HandleCancelUnloadAbilityEvent(int32_t systemAbilityId)
{
    HILOGI("Scheduler SA:%{public}d cancel unload start", systemAbilityId);
    std::shared_ptr<SystemAbilityContext> abilityContext;
    if (!GetSystemAbilityContext(systemAbilityId, abilityContext)) {
        return ERR_INVALID_VALUE;
    }
    nlohmann::json activeReason;
    activeReason[KEY_EVENT_ID] = INTERFACE_CALL;
    activeReason[KEY_NAME] = CANCEL_UNLOAD;
    activeReason[KEY_VALUE] = "";
    activeReason[KEY_EXTRA_DATA_ID] = -1;
    int32_t result = ERR_INVALID_VALUE;
    std::lock_guard<std::mutex> autoLock(abilityContext->ownProcessContext->processLock);
    switch (abilityContext->state) {
        case SystemAbilityState::UNLOADABLE:
            result = ActiveSystemAbilityLocked(abilityContext, activeReason);
            break;
        default:
            result = ERR_OK;
            HILOGI("Scheduler SA:%{public}d in state %{public}d,not need cancel unload",
                systemAbilityId, abilityContext->state);
            break;
    }
    return result;
}

int32_t SystemAbilityStateScheduler::ActiveSystemAbilityLocked(
    const std::shared_ptr<SystemAbilityContext>& abilityContext,
    const nlohmann::json& activeReason)
{
    bool result = SystemAbilityManager::GetInstance()->ActiveSystemAbility(abilityContext->systemAbilityId,
        abilityContext->ownProcessContext->processName, activeReason);
    if (!result) {
        HILOGE("Scheduler SA:%{public}d active fail", abilityContext->systemAbilityId);
        return ACTIVE_SA_FAIL;
    }
    return stateMachine_->AbilityStateTransitionLocked(abilityContext, SystemAbilityState::LOADED);
}

int32_t SystemAbilityStateScheduler::SendAbilityStateEvent(int32_t systemAbilityId, AbilityStateEvent event)
{
    HILOGD("Scheduler SA:%{public}d recv state event", systemAbilityId);
    std::shared_ptr<SystemAbilityContext> abilityContext;
    if (!GetSystemAbilityContext(systemAbilityId, abilityContext)) {
        return ERR_INVALID_VALUE;
    }
    std::lock_guard<std::mutex> autoLock(abilityContext->ownProcessContext->processLock);
    return stateEventHandler_->HandleAbilityEventLocked(abilityContext, event);
}

int32_t SystemAbilityStateScheduler::SendProcessStateEvent(const ProcessInfo& processInfo, ProcessStateEvent event)
{
    HILOGD("Scheduler proc:%{public}s receive state event",
        Str16ToStr8(processInfo.processName).c_str());
    std::shared_ptr<SystemProcessContext> processContext;
    if (!GetSystemProcessContext(processInfo.processName, processContext)) {
        return ERR_INVALID_VALUE;
    }
    std::lock_guard<std::mutex> autoLock(processContext->processLock);
    return stateEventHandler_->HandleProcessEventLocked(processContext, processInfo, event);
}

int32_t SystemAbilityStateScheduler::SendDelayUnloadEventLocked(uint32_t systemAbilityId, int32_t delayTime)
{
    if (unloadEventHandler_ == nullptr) {
        HILOGE("Scheduler:unload handler not init");
        return UNLOAD_EVENT_HANDLER_NULL;
    }
    if (unloadEventHandler_->HasInnerEvent(systemAbilityId)) {
        return ERR_OK;
    }
    HILOGI("Scheduler SA:%{public}d send delay unload event", systemAbilityId);
    bool ret = unloadEventHandler_->SendEvent(systemAbilityId, 0, delayTime);
    if (!ret) {
        HILOGE("Scheduler:send event fail");
        return SEND_EVENT_FAIL;
    }
    return ERR_OK;
}

int32_t SystemAbilityStateScheduler::RemoveDelayUnloadEventLocked(uint32_t systemAbilityId)
{
    if (unloadEventHandler_ == nullptr) {
        HILOGE("Scheduler:unload handler not init");
        return UNLOAD_EVENT_HANDLER_NULL;
    }
    if (!unloadEventHandler_->HasInnerEvent(systemAbilityId)) {
        return ERR_OK;
    }
    HILOGI("Scheduler SA:%{public}d rm delay unload event", systemAbilityId);
    unloadEventHandler_->RemoveEvent(systemAbilityId);
    return ERR_OK;
}

int32_t SystemAbilityStateScheduler::PendLoadEventLocked(const std::shared_ptr<SystemAbilityContext>& abilityContext,
    const LoadRequestInfo& loadRequestInfo)
{
    HILOGI("Scheduler SA:%{public}d save load event", abilityContext->systemAbilityId);
    if (loadRequestInfo.callback == nullptr) {
        HILOGW("Scheduler:callback invalid!");
        return CALLBACK_NULL;
    }
    bool isExist = std::any_of(abilityContext->pendingLoadEventList.begin(),
        abilityContext->pendingLoadEventList.end(), [&loadRequestInfo](const auto& loadEventItem) {
            return loadRequestInfo.callback->AsObject() == loadEventItem.callback->AsObject();
        });
    if (isExist) {
        HILOGI("Scheduler SA:%{public}d existed callback", abilityContext->systemAbilityId);
        return ERR_OK;
    }
    auto& count = abilityContext->pendingLoadEventCountMap[loadRequestInfo.callingPid];
    if (count >= MAX_SUBSCRIBE_COUNT) {
        HILOGE("Scheduler SA:%{public}d pid:%{public}d overflow max callback count!",
            abilityContext->systemAbilityId, loadRequestInfo.callingPid);
        return PEND_LOAD_EVENT_SIZE_LIMIT;
    }
    ++count;
    abilityContext->pendingLoadEventList.emplace_back(loadRequestInfo);
    abilityContext->pendingEvent = PendingEvent::LOAD_ABILITY_EVENT;
    return ERR_OK;
}

int32_t SystemAbilityStateScheduler::PendUnloadEventLocked(
    const std::shared_ptr<SystemAbilityContext>& abilityContext,
    const std::shared_ptr<UnloadRequestInfo> unloadRequestInfo)
{
    HILOGI("Scheduler SA:%{public}d save unload event", abilityContext->systemAbilityId);
    abilityContext->pendingEvent = PendingEvent::UNLOAD_ABILITY_EVENT;
    abilityContext->pendingUnloadEvent = unloadRequestInfo;
    return ERR_OK;
}

int32_t SystemAbilityStateScheduler::RemovePendingUnloadEventLocked(
    const std::shared_ptr<SystemAbilityContext>& abilityContext)
{
    if (abilityContext->pendingEvent == PendingEvent::UNLOAD_ABILITY_EVENT) {
        HILOGI("Scheduler SA:%{public}d rm pending unload event", abilityContext->systemAbilityId);
        abilityContext->pendingEvent = PendingEvent::NO_EVENT;
    }
    return ERR_OK;
}

int32_t SystemAbilityStateScheduler::HandlePendingLoadEventLocked(
    const std::shared_ptr<SystemAbilityContext>& abilityContext)
{
    if (abilityContext->pendingEvent != PendingEvent::LOAD_ABILITY_EVENT) {
        HILOGD("Scheduler SA:%{public}d no pending load event", abilityContext->systemAbilityId);
        return ERR_OK;
    }
    HILOGI("Scheduler SA:%{public}d handle pending load event start", abilityContext->systemAbilityId);
    abilityContext->pendingEvent = PendingEvent::NO_EVENT;
    for (auto& loadRequestInfo : abilityContext->pendingLoadEventList) {
        int32_t result = HandleLoadAbilityEventLocked(abilityContext, loadRequestInfo);
        if (result != ERR_OK) {
            HILOGE("Scheduler SA:%{public}d handle pending load event fail,callPid:%{public}d",
                abilityContext->systemAbilityId, loadRequestInfo.callingPid);
        }
    }
    abilityContext->pendingLoadEventList.clear();
    abilityContext->pendingLoadEventCountMap.clear();
    return ERR_OK;
}

int32_t SystemAbilityStateScheduler::HandlePendingUnloadEventLocked(
    const std::shared_ptr<SystemAbilityContext>& abilityContext)
{
    if (abilityContext->pendingEvent != PendingEvent::UNLOAD_ABILITY_EVENT) {
        HILOGD("Scheduler SA:%{public}d no pending unload event", abilityContext->systemAbilityId);
        return ERR_OK;
    }
    HILOGI("Scheduler SA:%{public}d handle pending unload event start", abilityContext->systemAbilityId);
    abilityContext->pendingEvent = PendingEvent::NO_EVENT;
    return HandleUnloadAbilityEventLocked(abilityContext, abilityContext->pendingUnloadEvent);
}

int32_t SystemAbilityStateScheduler::DoLoadSystemAbilityLocked(
    const std::shared_ptr<SystemAbilityContext>& abilityContext, const LoadRequestInfo& loadRequestInfo)
{
    int32_t result = ERR_OK;
    if (loadRequestInfo.deviceId == LOCAL_DEVICE) {
        HILOGD("Scheduler SA:%{public}d load ability from local start", abilityContext->systemAbilityId);
        result = SystemAbilityManager::GetInstance()->DoLoadSystemAbility(abilityContext->systemAbilityId,
            abilityContext->ownProcessContext->processName, loadRequestInfo.callback, loadRequestInfo.callingPid,
            loadRequestInfo.loadEvent);
    } else {
        HILOGD("Scheduler SA:%{public}d load ability from remote start", abilityContext->systemAbilityId);
        result = SystemAbilityManager::GetInstance()->DoLoadSystemAbilityFromRpc(loadRequestInfo.deviceId,
            abilityContext->systemAbilityId, abilityContext->ownProcessContext->processName, loadRequestInfo.callback,
            loadRequestInfo.loadEvent);
    }
    if (result == ERR_OK && abilityContext->state == SystemAbilityState::NOT_LOADED) {
        return stateMachine_->AbilityStateTransitionLocked(abilityContext, SystemAbilityState::LOADING);
    }
    return result;
}

int32_t SystemAbilityStateScheduler::TryUnloadAllSystemAbility(
    const std::shared_ptr<SystemProcessContext>& processContext)
{
    if (processContext == nullptr) {
        HILOGE("Scheduler:proc context is null");
        return ERR_INVALID_VALUE;
    }
    std::lock_guard<std::mutex> autoLock(processContext->processLock);
    if (CanUnloadAllSystemAbilityLocked(processContext, true)) {
        return UnloadAllSystemAbilityLocked(processContext);
    }
    return ERR_OK;
}

bool SystemAbilityStateScheduler::CanUnloadAllSystemAbility(
    const std::shared_ptr<SystemProcessContext>& processContext)
{
    std::lock_guard<std::mutex> autoLock(processContext->stateCountLock);
    return CanUnloadAllSystemAbilityLocked(processContext, true);
}

bool SystemAbilityStateScheduler::CanUnloadAllSystemAbilityLocked(
    const std::shared_ptr<SystemProcessContext>& processContext, bool isNeedCheckRecycleStrategy)
{
    uint32_t notLoadAbilityCount = processContext->abilityStateCountMap[SystemAbilityState::NOT_LOADED];
    uint32_t unloadableAbilityCount = processContext->abilityStateCountMap[SystemAbilityState::UNLOADABLE];
    if (unloadableAbilityCount == 0) {
        return false;
    }
    if (notLoadAbilityCount + unloadableAbilityCount == processContext->saList.size()) {
        if (isNeedCheckRecycleStrategy) {
            return CheckSaIsImmediatelyRecycle(processContext);
        } else {
            return true;
        }
    }
    HILOGI("Scheduler proc:%{public}s SA num:%{public}zu,notloaded:%{public}d,unloadable:%{public}d",
        Str16ToStr8(processContext->processName).c_str(), processContext->saList.size(), notLoadAbilityCount,
        unloadableAbilityCount);
    return false;
}

bool SystemAbilityStateScheduler::CheckSaIsImmediatelyRecycle(
    const std::shared_ptr<SystemProcessContext>& processContext)
{
    for (auto& saId: processContext->saList) {
        if (SystemAbilityManager::GetInstance()->CheckSaIsImmediatelyRecycle(saId)) {
            return true;
        }
    }
    HILOGI("CheckSaIsImmediatelyRecycle is false");
    return false;
}

int32_t SystemAbilityStateScheduler::PostTryUnloadAllAbilityTask(
    const std::shared_ptr<SystemProcessContext>& processContext)
{
    bool result = processHandler_->PostTask([this, processContext] () {
        int32_t ret = TryUnloadAllSystemAbility(processContext);
        if (ret != ERR_OK) {
            HILOGE("Scheduler proc:%{public}s unload all SA fail",
                Str16ToStr8(processContext->processName).c_str());
        }
    });
    if (!result) {
        HILOGW("Scheduler proc:%{public}s post task fail",
            Str16ToStr8(processContext->processName).c_str());
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

int32_t SystemAbilityStateScheduler::PostUnloadTimeoutTask(const std::shared_ptr<SystemProcessContext>& processContext)
{
    auto timeoutTask = [this, processContext] () {
        std::string name = KEY_UNLOAD_TIMEOUT + Str16ToStr8(processContext->processName);
        if (processHandler_ != nullptr) {
            HILOGD("TimeoutTask deltask proc:%{public}s", name.c_str());
            processHandler_->DelTask(name);
        } else {
            HILOGE("TimeoutTask processHandler_ is null");
        }
        std::lock_guard<std::mutex> autoLock(processContext->processLock);
        if (processContext->state == SystemProcessState::STOPPING) {
            HILOGW("Scheduler proc:%{public}s unload SA timeout",
                Str16ToStr8(processContext->processName).c_str());
            int32_t result = KillSystemProcessLocked(processContext);
            HILOGI("Scheduler proc:%{public}s kill proc timeout ret:%{public}d",
                Str16ToStr8(processContext->processName).c_str(), result);
        }
    };
    bool ret = processHandler_->PostTask(timeoutTask, KEY_UNLOAD_TIMEOUT + Str16ToStr8(processContext->processName),
        UNLOAD_TIMEOUT_TIME);
    if (!ret) {
        HILOGW("Scheduler proc:%{public}s post timeout task fail",
            Str16ToStr8(processContext->processName).c_str());
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

void SystemAbilityStateScheduler::RemoveUnloadTimeoutTask(const std::shared_ptr<SystemProcessContext>& processContext)
{
    processHandler_->RemoveTask(KEY_UNLOAD_TIMEOUT + Str16ToStr8(processContext->processName));
}

int32_t SystemAbilityStateScheduler::PostTryKillProcessTask(
    const std::shared_ptr<SystemProcessContext>& processContext)
{
    bool result = processHandler_->PostTask([this, processContext] () {
        int32_t ret = TryKillSystemProcess(processContext);
        if (ret != ERR_OK) {
            HILOGE("Scheduler proc:%{public}s kill proc fail",
                Str16ToStr8(processContext->processName).c_str());
        }
    });
    if (!result) {
        HILOGW("Scheduler proc:%{public}s post task fail",
            Str16ToStr8(processContext->processName).c_str());
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

int32_t SystemAbilityStateScheduler::UnloadAllSystemAbilityLocked(
    const std::shared_ptr<SystemProcessContext>& processContext)
{
    HILOGI("Scheduler proc:%{public}s unload all SA", Str16ToStr8(processContext->processName).c_str());
    for (auto& saId : processContext->saList) {
        std::shared_ptr<SystemAbilityContext> abilityContext;
        if (!GetSystemAbilityContext(saId, abilityContext)) {
            continue;
        }
        int32_t result = ERR_OK;
        if (abilityContext->state == SystemAbilityState::UNLOADABLE) {
            result = DoUnloadSystemAbilityLocked(abilityContext);
        }
        if (result != ERR_OK) {
            HILOGE("Scheduler SA:%{public}d unload fail", saId);
        }
    }
    PostUnloadTimeoutTask(processContext);
    return stateMachine_->ProcessStateTransitionLocked(processContext, SystemProcessState::STOPPING);
}

int32_t SystemAbilityStateScheduler::DoUnloadSystemAbilityLocked(
    const std::shared_ptr<SystemAbilityContext>& abilityContext)
{
    if (abilityContext->unloadRequest == nullptr) {
        HILOGE("Scheduler SA:%{public}d DoUnloadSaLocked unloadRequest is null",
            abilityContext->systemAbilityId);
        return ERR_INVALID_VALUE;
    }
    int32_t result = ERR_OK;
    HILOGI("Scheduler SA:%{public}d unload start", abilityContext->systemAbilityId);
    result = SystemAbilityManager::GetInstance()->DoUnloadSystemAbility(abilityContext->systemAbilityId,
        abilityContext->ownProcessContext->processName, abilityContext->unloadRequest->unloadEvent);
    if (result == ERR_OK) {
        return stateMachine_->AbilityStateTransitionLocked(abilityContext, SystemAbilityState::UNLOADING);
    }
    return result;
}

int32_t SystemAbilityStateScheduler::UnloadAllIdleSystemAbility()
{
    HILOGI("Scheduler:UnloadAllIdleSa");
    int32_t result = ERR_OK;
    std::shared_lock<std::shared_mutex> readLock(processMapLock_);
    for (auto it : processContextMap_) {
        auto& processContext = it.second;
        if (processContext == nullptr) {
            continue;
        }

        int32_t ret = ERR_OK;
        std::lock_guard<std::mutex> autoLock(processContext->processLock);
        if (CanUnloadAllSystemAbilityLocked(processContext)) {
            ret = UnloadAllSystemAbilityLocked(processContext);
        }
        if (ret != ERR_OK) {
            result = ret;
            HILOGI("Scheduler proc:%{public}s unload all SA fail",
                Str16ToStr8(processContext->processName).c_str());
        }
    }
    return result;
}

int32_t SystemAbilityStateScheduler::TryKillSystemProcess(
    const std::shared_ptr<SystemProcessContext>& processContext)
{
    if (processContext == nullptr) {
        HILOGE("Scheduler:proc context is null");
        return ERR_INVALID_VALUE;
    }
    std::lock_guard<std::mutex> autoLock(processContext->processLock);
    if (CanKillSystemProcessLocked(processContext)) {
        return KillSystemProcessLocked(processContext);
    }
    return ERR_OK;
}

bool SystemAbilityStateScheduler::CanKillSystemProcess(
    const std::shared_ptr<SystemProcessContext>& processContext)
{
    std::lock_guard<std::mutex> autoLock(processContext->stateCountLock);
    return CanKillSystemProcessLocked(processContext);
}

bool SystemAbilityStateScheduler::CanKillSystemProcessLocked(
    const std::shared_ptr<SystemProcessContext>& processContext)
{
    uint32_t notLoadAbilityCount = processContext->abilityStateCountMap[SystemAbilityState::NOT_LOADED];
    HILOGI("Scheduler proc:%{public}s,SA num:%{public}zu,notloaded num:%{public}d",
        Str16ToStr8(processContext->processName).c_str(), processContext->saList.size(), notLoadAbilityCount);
    if (notLoadAbilityCount == processContext->saList.size()) {
        return true;
    }
    return false;
}

int32_t SystemAbilityStateScheduler::KillSystemProcessLocked(
    const std::shared_ptr<SystemProcessContext>& processContext)
{
    int64_t begin = GetTickCount();
    int32_t result = ERR_OK;
    {
        SamgrXCollie samgrXCollie("samgr--killProccess_" + Str16ToStr8(processContext->processName));
        result = ServiceControlWithExtra(Str16ToStr8(processContext->processName).c_str(),
            ServiceAction::STOP, nullptr, 0);
    }

    int64_t duration = GetTickCount() - begin;
    if (result != 0) {
        ReportProcessStopFail(Str16ToStr8(processContext->processName), processContext->pid, processContext->uid,
            "err:" + ToString(result));
    }
    ReportProcessStopDuration(Str16ToStr8(processContext->processName), processContext->pid,
        processContext->uid, duration);
    KHILOGI("Scheduler proc:%{public}s kill proc,pid:%{public}d,uid:%{public}d,ret:%{public}d,"
        "spend %{public}" PRId64 "ms", Str16ToStr8(processContext->processName).c_str(), processContext->pid,
        processContext->uid, result, duration);
    return result;
}

bool SystemAbilityStateScheduler::CanRestartProcessLocked(const std::shared_ptr<SystemProcessContext>& processContext)
{
    if (!processContext->enableRestart) {
        return false;
    }
    int64_t curtime = GetTickCount();
    if (processContext->restartCountsCtrl.size() < RESTART_TIMES_LIMIT) {
        processContext->restartCountsCtrl.push_back(curtime);
        return true;
    } else if (processContext->restartCountsCtrl.size() == RESTART_TIMES_LIMIT) {
        if (curtime - processContext->restartCountsCtrl.front() < RESTART_TIME_INTERVAL_LIMIT) {
            processContext->enableRestart = false;
            return false;
        }
        processContext->restartCountsCtrl.push_back(curtime);
        processContext->restartCountsCtrl.pop_front();
        return true;
    } else {
        HILOGE("Scheduler proc:%{public}s unkown err",
            Str16ToStr8(processContext->processName).c_str());
    }
    return false;
}

int32_t SystemAbilityStateScheduler::GetAbnormallyDiedAbilityLocked(
    std::shared_ptr<SystemProcessContext>& processContext,
    std::list<std::shared_ptr<SystemAbilityContext>>& abnormallyDiedAbilityList)
{
    for (auto& saId : processContext->saList) {
        std::shared_ptr<SystemAbilityContext> abilityContext;
        if (!GetSystemAbilityContext(saId, abilityContext)) {
            continue;
        }
        if (abilityContext->state == SystemAbilityState::LOADED
            || abilityContext->state == SystemAbilityState::LOADING) {
            SamgrUtil::SendUpdateSaState(abilityContext->systemAbilityId, "crash");
            HILOGI("Scheduler SA:%{public}d abnormally died", abilityContext->systemAbilityId);
            if (abilityContext->systemAbilityId == SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN) {
                SystemAbilityManager::GetInstance()->RemoveWhiteCommonEvent();
            }
            if (!abilityContext->isAutoRestart) {
                continue;
            }
            if (system::GetBoolParameter("resourceschedule.memmgr.min.memmory.watermark", false)) {
                HILOGW("restart fail,watermark=true");
                continue;
            }
            HILOGI("Scheduler SA:%{public}d is auto restart", abilityContext->systemAbilityId);
            abnormallyDiedAbilityList.emplace_back(abilityContext);
        }
    }
    return ERR_OK;
}


int32_t SystemAbilityStateScheduler::HandleAbnormallyDiedAbilityLocked(
    std::shared_ptr<SystemProcessContext>& processContext,
    std::list<std::shared_ptr<SystemAbilityContext>>& abnormallyDiedAbilityList)
{
    if (abnormallyDiedAbilityList.empty()) {
        return ERR_OK;
    }
    if (!CanRestartProcessLocked(processContext)) {
        HILOGW("Scheduler proc:%{public}s can't restart:More than 4 restarts in 20s",
            Str16ToStr8(processContext->processName).c_str());
        return ERR_OK;
    }
    OnDemandEvent onDemandEvent = {INTERFACE_CALL, "restart"};
    sptr<ISystemAbilityLoadCallback> callback(new SystemAbilityLoadCallbackStub());
    for (auto& abilityContext : abnormallyDiedAbilityList) {
        // Actively remove SA to prevent restart failure if the death recipient of SA is not processed in time.
        SystemAbilityManager::GetInstance()->RemoveDiedSystemAbility(abilityContext->systemAbilityId);
        LoadRequestInfo loadRequestInfo = {LOCAL_DEVICE, callback,
            abilityContext->systemAbilityId, -1, onDemandEvent};
        HandleLoadAbilityEventLocked(abilityContext, loadRequestInfo);
    }
    return ERR_OK;
}

void SystemAbilityStateScheduler::NotifyProcessStarted(const std::shared_ptr<SystemProcessContext>& processContext)
{
    std::shared_lock<std::shared_mutex> readLock(listenerSetLock_);
    for (auto& listener : processListeners_) {
        if (listener->AsObject() != nullptr) {
            SystemProcessInfo systemProcessInfo = {Str16ToStr8(processContext->processName), processContext->pid,
                processContext->uid};
            listener->OnSystemProcessStarted(systemProcessInfo);
        }
    }
}

void SystemAbilityStateScheduler::NotifyProcessStopped(const std::shared_ptr<SystemProcessContext>& processContext)
{
    std::shared_lock<std::shared_mutex> readLock(listenerSetLock_);
    for (auto& listener : processListeners_) {
        if (listener->AsObject() != nullptr) {
            SystemProcessInfo systemProcessInfo = {Str16ToStr8(processContext->processName), processContext->pid,
                processContext->uid};
            listener->OnSystemProcessStopped(systemProcessInfo);
        }
    }
}

void SystemAbilityStateScheduler::OnProcessStartedLocked(const std::u16string& processName)
{
    HILOGI("Scheduler proc:%{public}s started", Str16ToStr8(processName).c_str());
    std::shared_ptr<SystemProcessContext> processContext;
    if (!GetSystemProcessContext(processName, processContext)) {
        return;
    }
    NotifyProcessStarted(processContext);
}

void SystemAbilityStateScheduler::OnProcessNotStartedLocked(const std::u16string& processName)
{
    HILOGI("Scheduler proc:%{public}s stopped", Str16ToStr8(processName).c_str());
    std::shared_ptr<SystemProcessContext> processContext;
    if (!GetSystemProcessContext(processName, processContext)) {
        return;
    }
    NotifyProcessStopped(processContext);
    RemoveUnloadTimeoutTask(processContext);
    
    std::list<std::shared_ptr<SystemAbilityContext>> abnormallyDiedAbilityList;
    GetAbnormallyDiedAbilityLocked(processContext, abnormallyDiedAbilityList);
    for (auto& saId : processContext->saList) {
        std::shared_ptr<SystemAbilityContext> abilityContext;
        if (!GetSystemAbilityContext(saId, abilityContext)) {
            continue;
        }
        int32_t result = stateMachine_->AbilityStateTransitionLocked(abilityContext, SystemAbilityState::NOT_LOADED);
        if (result != ERR_OK) {
            continue;
        }
        HandlePendingLoadEventLocked(abilityContext);
    }
    HandleAbnormallyDiedAbilityLocked(processContext, abnormallyDiedAbilityList);
}

int32_t SystemAbilityStateScheduler::HandleAbilityDiedEvent(int32_t systemAbilityId)
{
    HILOGD("Scheduler SA:%{public}d handle ability died event", systemAbilityId);
    return ERR_OK;
}

void SystemAbilityStateScheduler::OnAbilityNotLoadedLocked(int32_t systemAbilityId)
{
    HILOGI("Scheduler SA:%{public}d not loaded", systemAbilityId);
    std::shared_ptr<SystemAbilityContext> abilityContext;
    if (!GetSystemAbilityContext(systemAbilityId, abilityContext)) {
        return;
    }
    RemoveDelayUnloadEventLocked(abilityContext->systemAbilityId);
    RemovePendingUnloadEventLocked(abilityContext);
    if (abilityContext->ownProcessContext->state == SystemProcessState::STOPPING) {
        PostTryKillProcessTask(abilityContext->ownProcessContext);
    } else if (abilityContext->ownProcessContext->state == SystemProcessState::STARTED) {
        PostTryUnloadAllAbilityTask(abilityContext->ownProcessContext);
    }
}

void SystemAbilityStateScheduler::OnAbilityLoadedLocked(int32_t systemAbilityId)
{
    HILOGI("Scheduler SA:%{public}d loaded", systemAbilityId);
    std::shared_ptr<SystemAbilityContext> abilityContext;
    if (!GetSystemAbilityContext(systemAbilityId, abilityContext)) {
        return;
    }
    HandlePendingUnloadEventLocked(abilityContext);
}

void SystemAbilityStateScheduler::OnAbilityUnloadableLocked(int32_t systemAbilityId)
{
    HILOGI("Scheduler SA:%{public}d unloadable", systemAbilityId);
    std::shared_ptr<SystemAbilityContext> abilityContext;
    if (!GetSystemAbilityContext(systemAbilityId, abilityContext)) {
        return;
    }
    PostTryUnloadAllAbilityTask(abilityContext->ownProcessContext);
}

int32_t SystemAbilityStateScheduler::GetSystemProcessInfo(int32_t systemAbilityId,
    SystemProcessInfo& systemProcessInfo)
{
    HILOGI("Scheduler:get proc info by [SA:%{public}d]", systemAbilityId);
    std::shared_ptr<SystemAbilityContext> abilityContext;
    if (!GetSystemAbilityContext(systemAbilityId, abilityContext)) {
        HILOGI("Scheduler:get SA context by said fail");
        return ERR_INVALID_VALUE;
    }
    std::shared_ptr<SystemProcessContext> processContext = abilityContext->ownProcessContext;
    std::lock_guard<std::mutex> autoLock(processContext->processLock);
    systemProcessInfo = {Str16ToStr8(processContext->processName), processContext->pid,
                processContext->uid};
    return ERR_OK;
}

int32_t SystemAbilityStateScheduler::GetRunningSystemProcess(std::list<SystemProcessInfo>& systemProcessInfos)
{
    HILOGI("Scheduler:get running process");
    std::shared_lock<std::shared_mutex> readLock(processMapLock_);
    for (auto it : processContextMap_) {
        auto& processContext = it.second;
        if (processContext == nullptr) {
            continue;
        }
        std::lock_guard<std::mutex> autoLock(processContext->processLock);
        if (processContext->state == SystemProcessState::STARTED) {
            SystemProcessInfo systemProcessInfo = {Str16ToStr8(processContext->processName), processContext->pid,
                processContext->uid};
            systemProcessInfos.emplace_back(systemProcessInfo);
        }
    }
    return ERR_OK;
}

int32_t SystemAbilityStateScheduler::GetProcessNameByProcessId(int32_t pid, std::u16string& processName)
{
    HILOGD("[SA Scheduler] get processName by processId");
    std::shared_lock<std::shared_mutex> readLock(processMapLock_);
    for (auto it : processContextMap_) {
        auto& processContext = it.second;
        if (processContext == nullptr) {
            continue;
        }
        std::lock_guard<std::mutex> autoLock(processContext->processLock);
        if (processContext->pid == pid) {
            processName = processContext->processName;
            return ERR_OK;
        }
    }
    return ERR_INVALID_VALUE;
}

void SystemAbilityStateScheduler::GetAllSystemAbilityInfo(std::string& result)
{
    std::shared_lock<std::shared_mutex> readLock(abiltyMapLock_);
    for (auto it : abilityContextMap_) {
        if (it.second == nullptr) {
            continue;
        }
        result += "said:                           ";
        result += std::to_string(it.second->systemAbilityId);
        result += "\n";
        result += "sa_state:                       ";
        result += SA_STATE_ENUM_STR[static_cast<int32_t>(it.second->state)];
        result += "\n";
        result += "sa_pending_event:               ";
        result += PENDINGEVENT_ENUM_STR[static_cast<int32_t>(it.second->pendingEvent)];
        if (it.second->ownProcessContext != nullptr) {
            std::lock_guard<std::mutex> autoLock(it.second->ownProcessContext->stateCountLock);
            result += '\n';
            result += "process_name:                   ";
            result += Str16ToStr8(it.second->ownProcessContext->processName);
            result += '\n';
            result += "pid:                            ";
            result += std::to_string(it.second->ownProcessContext->pid);
            result += '\n';
            result += "uid:                            ";
            result += std::to_string(it.second->ownProcessContext->uid);
        }
        result += "\n---------------------------------------------------\n";
    }
}

void SystemAbilityStateScheduler::GetSystemAbilityInfo(int32_t said, std::string& result)
{
    std::shared_ptr<SystemAbilityContext> abilityContext;
    if (!GetSystemAbilityContext(said, abilityContext)) {
        result.append("said is not exist");
        return;
    }
    std::lock_guard<std::mutex> autoLock(abilityContext->ownProcessContext->processLock);
    result += "said:                           ";
    result += std::to_string(said);
    result += "\n";
    result += "sa_state:                       ";
    result += SA_STATE_ENUM_STR[static_cast<int32_t>(abilityContext->state)];
    result += "\n";
    result += "sa_pending_event:               ";
    result += PENDINGEVENT_ENUM_STR[static_cast<int32_t>(abilityContext->pendingEvent)];
    result += "\n";
    result += "process_name:                   ";
    result += Str16ToStr8(abilityContext->ownProcessContext->processName);
    result += "\n";
    result += "process_state:                  ";
    result += PROCESS_STATE_ENUM_STR[static_cast<int32_t>(abilityContext->ownProcessContext->state)];
    result += "\n";
    result += "pid:                            ";
    result += std::to_string(abilityContext->ownProcessContext->pid);
    result += "\n";
}

void SystemAbilityStateScheduler::GetProcessInfo(const std::string& processName, std::string& result)
{
    std::shared_ptr<SystemProcessContext> processContext;
    if (!GetSystemProcessContext(Str8ToStr16(processName), processContext)) {
        result.append("process is not exist");
        return;
    }
    std::lock_guard<std::mutex> autoLock(processContext->processLock);
    result += "process_name:                   ";
    result += Str16ToStr8(processContext->processName);
    result += "\n";
    result += "process_state:                  ";
    result += PROCESS_STATE_ENUM_STR[static_cast<int32_t>(processContext->state)];
    result += "\n";
    result += "pid:                            ";
    result += std::to_string(processContext->pid);
    result += "\n---------------------------------------------------\n";
    for (auto it : processContext->saList) {
        std::shared_ptr<SystemAbilityContext> abilityContext;
        if (!GetSystemAbilityContext(it, abilityContext)) {
            result.append("process said is not exist");
            return;
        }
        result += "said:                           ";
        result += std::to_string(abilityContext->systemAbilityId);
        result += '\n';
        result += "sa_state:                       ";
        result += SA_STATE_ENUM_STR[static_cast<int32_t>(abilityContext->state)];
        result += '\n';
        result += "sa_pending_event:               ";
        result += PENDINGEVENT_ENUM_STR[static_cast<int32_t>(abilityContext->pendingEvent)];
        result += "\n---------------------------------------------------\n";
    }
}

void SystemAbilityStateScheduler::GetAllSystemAbilityInfoByState(const std::string& state, std::string& result)
{
    std::shared_lock<std::shared_mutex> readLock(abiltyMapLock_);
    for (auto it : abilityContextMap_) {
        if (it.second == nullptr || SA_STATE_ENUM_STR[static_cast<int32_t>(it.second->state)] != state) {
            continue;
        }
        result += "said:                           ";
        result += std::to_string(it.second->systemAbilityId);
        result += '\n';
        result += "sa_state:                       ";
        result += SA_STATE_ENUM_STR[static_cast<int32_t>(it.second->state)];
        result += '\n';
        result += "sa_pending_event:               ";
        result += PENDINGEVENT_ENUM_STR[static_cast<int32_t>(it.second->pendingEvent)];
        if (it.second->ownProcessContext != nullptr) {
            std::lock_guard<std::mutex> autoLock(it.second->ownProcessContext->stateCountLock);
            result += '\n';
            result += "process_name:                   ";
            result += Str16ToStr8(it.second->ownProcessContext->processName);
            result += '\n';
            result += "pid:                            ";
            result += std::to_string(it.second->ownProcessContext->pid);
            result += '\n';
            result += "uid:                            ";
            result += std::to_string(it.second->ownProcessContext->uid);
        }
        result += "\n---------------------------------------------------\n";
    }
}

int32_t SystemAbilityStateScheduler::SubscribeSystemProcess(const sptr<ISystemProcessStatusChange>& listener)
{
    std::unique_lock<std::shared_mutex> writeLock(listenerSetLock_);
    auto iter = std::find_if(processListeners_.begin(), processListeners_.end(),
        [listener](sptr<ISystemProcessStatusChange>& item) {
        return item->AsObject() == listener->AsObject();
    });
    if (iter == processListeners_.end()) {
        if (processListenerDeath_ != nullptr) {
            bool ret = listener->AsObject()->AddDeathRecipient(processListenerDeath_);
            HILOGI("SubscribeSystemProcess AddDeathRecipient %{public}s", ret ? "succeed" : "failed");
        }
        processListeners_.emplace_back(listener);
    } else {
        HILOGI("SubscribeSystemProcess listener already exists");
    }
    return ERR_OK;
}

int32_t SystemAbilityStateScheduler::UnSubscribeSystemProcess(const sptr<ISystemProcessStatusChange>& listener)
{
    std::unique_lock<std::shared_mutex> writeLock(listenerSetLock_);
    auto iter = std::find_if(processListeners_.begin(), processListeners_.end(),
        [listener](sptr<ISystemProcessStatusChange>& item) {
        return item->AsObject() == listener->AsObject();
    });
    if (iter != processListeners_.end()) {
        if (processListenerDeath_ != nullptr) {
            listener->AsObject()->RemoveDeathRecipient(processListenerDeath_);
        }
        processListeners_.erase(iter);
        HILOGI("UnSubscribeSystemProcess listener remove success");
    } else {
        HILOGI("UnSubscribeSystemProcess listener not exists");
    }
    return ERR_OK;
}

bool SystemAbilityStateScheduler::IsSystemProcessNeverStartedLocked(const std::u16string& processName)
{
    std::shared_ptr<SystemProcessContext> processContext;
    if (!GetSystemProcessContext(processName, processContext)) {
        return true;
    }
    return processContext->pid < 0;
}

int32_t SystemAbilityStateScheduler::ProcessDelayUnloadEvent(int32_t systemAbilityId)
{
    std::shared_ptr<SystemAbilityContext> abilityContext;
    if (!GetSystemAbilityContext(systemAbilityId, abilityContext)) {
        return GET_SA_CONTEXT_FAIL;
    }
    std::lock_guard<std::mutex> autoLock(abilityContext->ownProcessContext->processLock);
    return ProcessDelayUnloadEventLocked(systemAbilityId);
}

int32_t SystemAbilityStateScheduler::ProcessDelayUnloadEventLocked(int32_t systemAbilityId)
{
    std::shared_ptr<SystemAbilityContext> abilityContext;
    if (!GetSystemAbilityContext(systemAbilityId, abilityContext)) {
        return GET_SA_CONTEXT_FAIL;
    }
    if (abilityContext->unloadRequest == nullptr) {
        HILOGE("Scheduler SA:%{public}d unloadRequest is null", abilityContext->systemAbilityId);
        return UNLOAD_REQUEST_NULL;
    }
    if (abilityContext->state != SystemAbilityState::LOADED) {
        HILOGW("Scheduler SA:%{public}d can't proc delay unload event", systemAbilityId);
        return ERR_OK;
    }
    HILOGI("Scheduler SA:%{public}d proc delay unload event", systemAbilityId);
    int32_t delayTime = 0;
    nlohmann::json idleReason;
    idleReason[KEY_EVENT_ID] = abilityContext->unloadRequest->unloadEvent.eventId;
    idleReason[KEY_NAME] = abilityContext->unloadRequest->unloadEvent.name;
    idleReason[KEY_VALUE] = abilityContext->unloadRequest->unloadEvent.value;
    idleReason[KEY_EXTRA_DATA_ID] = abilityContext->unloadRequest->unloadEvent.extraDataId;
    bool result = SystemAbilityManager::GetInstance()->IdleSystemAbility(abilityContext->systemAbilityId,
        abilityContext->ownProcessContext->processName, idleReason, delayTime);
    if (!result) {
        HILOGE("Scheduler SA:%{public}d idle fail", systemAbilityId);
        return IDLE_SA_FAIL;
    }
    if (delayTime < 0) {
        HILOGI("Scheduler SA:%{public}d reject unload", systemAbilityId);
        return ERR_OK;
    } else if (delayTime == 0) {
        HILOGI("Scheduler SA:%{public}d agree unload", systemAbilityId);
        return stateMachine_->AbilityStateTransitionLocked(abilityContext, SystemAbilityState::UNLOADABLE);
    } else {
        HILOGI("Scheduler SA:%{public}d choose delay unload", systemAbilityId);
        return SendDelayUnloadEventLocked(abilityContext->systemAbilityId, fmin(delayTime, MAX_DELAY_TIME));
    }
}

void SystemAbilityStateScheduler::CheckEnableOnce(const OnDemandEvent& event,
    const std::list<SaControlInfo>& saControlList)
{
    sptr<ISystemAbilityLoadCallback> callback(new SystemAbilityLoadCallbackStub());
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
            HILOGE("process ondemand event failed, ondemandId:%{public}d, SA:%{public}d",
                saControl.ondemandId, saControl.saId);
        }
    }
}

int32_t SystemAbilityStateScheduler::CheckStartEnableOnce(const OnDemandEvent& event,
    const SaControlInfo& saControl, sptr<ISystemAbilityLoadCallback> callback)
{
    int32_t result = ERR_INVALID_VALUE;
    if (saControl.enableOnce) {
        lock_guard<mutex> autoLock(startEnableOnceLock_);
        auto iter = startEnableOnceMap_.find(saControl.saId);
        if (iter != startEnableOnceMap_.end() && SamgrUtil::IsSameEvent(event, startEnableOnceMap_[saControl.saId])) {
            HILOGI("ondemand canceled for enable-once, ondemandId:%{public}d, SA:%{public}d",
                saControl.ondemandId, saControl.saId);
            return result;
        }
        startEnableOnceMap_[saControl.saId].emplace_back(event);
        HILOGI("startEnableOnceMap_ add SA:%{public}d, eventId:%{public}d",
            saControl.saId, event.eventId);
    }
    auto callingPid = IPCSkeleton::GetCallingPid();
    LoadRequestInfo loadRequestInfo = {LOCAL_DEVICE, callback, saControl.saId, callingPid, event};
    result = HandleLoadAbilityEvent(loadRequestInfo);
    if (saControl.enableOnce && result != ERR_OK) {
        lock_guard<mutex> autoLock(startEnableOnceLock_);
        auto& events = startEnableOnceMap_[saControl.saId];
        events.remove(event);
        if (events.empty()) {
            startEnableOnceMap_.erase(saControl.saId);
        }
        HILOGI("startEnableOnceMap_remove SA:%{public}d, eventId:%{public}d",
            saControl.saId, event.eventId);
    }
    if (result != ERR_OK) {
        ReportSamgrSaLoadFail(saControl.saId, IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid(),
            "ondemand load err:" + ToString(result));
    }
    return result;
}

int32_t SystemAbilityStateScheduler::CheckStopEnableOnce(const OnDemandEvent& event,
    const SaControlInfo& saControl)
{
    int32_t result = ERR_INVALID_VALUE;
    if (saControl.enableOnce) {
        lock_guard<mutex> autoLock(stopEnableOnceLock_);
        auto iter = stopEnableOnceMap_.find(saControl.saId);
        if (iter != stopEnableOnceMap_.end() && SamgrUtil::IsSameEvent(event, stopEnableOnceMap_[saControl.saId])) {
            HILOGI("ondemand canceled for enable-once, ondemandId:%{public}d, SA:%{public}d",
                saControl.ondemandId, saControl.saId);
            return result;
        }
        stopEnableOnceMap_[saControl.saId].emplace_back(event);
        HILOGI("stopEnableOnceMap_ add SA:%{public}d, eventId:%{public}d",
            saControl.saId, event.eventId);
    }
    auto callingPid = IPCSkeleton::GetCallingPid();
    std::shared_ptr<UnloadRequestInfo> unloadRequestInfo =
        std::make_shared<UnloadRequestInfo>(event, saControl.saId, callingPid);
    result = HandleUnloadAbilityEvent(unloadRequestInfo);
    if (saControl.enableOnce && result != ERR_OK) {
        lock_guard<mutex> autoLock(stopEnableOnceLock_);
        auto& events = stopEnableOnceMap_[saControl.saId];
        events.remove(event);
        if (events.empty()) {
            stopEnableOnceMap_.erase(saControl.saId);
        }
        HILOGI("stopEnableOnceMap_ remove SA:%{public}d, eventId:%{public}d",
            saControl.saId, event.eventId);
    }
    if (result != ERR_OK) {
        ReportSaUnLoadFail(saControl.saId, IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid(),
            "Ondemand unload err:" + ToString(result));
    }
    return result;
}

void SystemAbilityStateScheduler::UnloadEventHandler::ProcessEvent(uint32_t eventId)
{
    int32_t systemAbilityId = static_cast<int32_t>(eventId);
    if (handler_ != nullptr) {
        HILOGD("ProcessEvent deltask SA:%{public}d", systemAbilityId);
        handler_->DelTask(std::to_string(eventId));
    } else {
        HILOGE("ProcessEvent handler_ is null");
    }
    auto stateScheduler = stateScheduler_.lock();
    int32_t result = ERR_OK;
    if (stateScheduler != nullptr) {
        result = stateScheduler->ProcessDelayUnloadEvent(systemAbilityId);
    }
    if (result != ERR_OK) {
        HILOGE("Scheduler SA:%{public}d proc delay unload event fail", systemAbilityId);
    }
}

bool SystemAbilityStateScheduler::UnloadEventHandler::SendEvent(uint32_t eventId,
    int64_t extraDataId, uint64_t delayTime)
{
    if (handler_ == nullptr) {
        HILOGE("SystemAbilityStateScheduler SendEvent handler is null!");
        return false;
    }
    auto task = [this, eventId] {this->ProcessEvent(eventId);};
    return handler_->PostTask(task, std::to_string(eventId), delayTime);
}

void SystemAbilityStateScheduler::UnloadEventHandler::RemoveEvent(uint32_t eventId)
{
    if (handler_ == nullptr) {
        HILOGE("SystemAbilityStateScheduler SendEvent handler is null!");
        return;
    }
    handler_->RemoveTask(std::to_string(eventId));
}

bool SystemAbilityStateScheduler::UnloadEventHandler::HasInnerEvent(uint32_t eventId)
{
    if (handler_ == nullptr) {
        HILOGE("SystemAbilityStateScheduler SendEvent handler is null!");
        return false;
    }
    return handler_->HasInnerEvent(std::to_string(eventId));
}

void SystemAbilityStateScheduler::UnloadEventHandler::CleanFfrt()
{
    if (handler_ != nullptr) {
        handler_->CleanFfrt();
    }
}

void SystemAbilityStateScheduler::UnloadEventHandler::SetFfrt()
{
    if (handler_ != nullptr) {
        handler_->SetFfrt("UnloadEventHandler");
    }
}

}  // namespace OHOS