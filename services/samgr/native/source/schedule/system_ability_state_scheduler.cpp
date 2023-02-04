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

#include "ipc_skeleton.h"
#ifdef RESSCHED_ENABLE
#include "res_sched_client.h"
#endif
#include "sam_log.h"
#include "string_ex.h"
#include "system_ability_manager.h"
#include "schedule/system_ability_state_scheduler.h"

namespace OHOS {
namespace {
constexpr int32_t MAX_SUBSCRIBE_COUNT = 256;
constexpr int32_t UNLOAD_DELAY_TIME = 20 * 1000;
constexpr int32_t UNLOAD_TIMEOUT_TIME = 5 * 1000;
const std::string LOCAL_DEVICE = "local";
}
void SystemAbilityStateScheduler::Init(const std::list<SaProfile>& saProfiles)
{
    InitStateContext(saProfiles);
    auto unloadRunner = AppExecFwk::EventRunner::Create("UnloadHandler");
    unloadEventHandler_ = std::make_shared<UnloadEventHandler>(unloadRunner, weak_from_this());

    auto listener =  std::dynamic_pointer_cast<SystemAbilityStateListener>(shared_from_this());
    stateMachine_ = std::make_shared<SystemAbilityStateMachine>(listener);
    stateEventHandler_ = std::make_shared<SystemAbilityEventHandler>(stateMachine_);

    auto processRunner = AppExecFwk::EventRunner::Create("ProcssHandler");
    processHandler_ = std::make_shared<AppExecFwk::EventHandler>(processRunner);
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
        abilityContext->ownProcessContext = processContextMap_[saProfile.process];
        std::unique_lock<std::shared_mutex> abiltyWriteLock(abiltyMapLock_);
        abiltyContextMap_[saProfile.saId] = abilityContext;
    }
}

bool SystemAbilityStateScheduler::GetSystemAbilityContext(int32_t systemAbilityId,
    std::shared_ptr<SystemAbilityContext>& abilityContext)
{
    std::shared_lock<std::shared_mutex> readLock(abiltyMapLock_);
    if (abiltyContextMap_.count(systemAbilityId) == 0) {
        HILOGE("[SA Scheduler][SA: %{public}d] not in SA profiles", systemAbilityId);
        return false;
    }
    abilityContext = abiltyContextMap_[systemAbilityId];
    if (abilityContext == nullptr) {
        HILOGE("[SA Scheduler][SA: %{public}d] context is nullptr", systemAbilityId);
        return false;
    }
    if (abilityContext->ownProcessContext == nullptr) {
        HILOGE("[SA Scheduler][SA: %{public}d] not in any process", systemAbilityId);
        return false;
    }
    return true;
}

bool SystemAbilityStateScheduler::GetSystemProcessContext(const std::u16string& processName,
    std::shared_ptr<SystemProcessContext>& processContext)
{
    std::shared_lock<std::shared_mutex> readLock(processMapLock_);
    if (processContextMap_.count(processName) == 0) {
        HILOGE("[SA Scheduler][processName: %{public}s] invalid", Str16ToStr8(processName).c_str());
        return false;
    }
    processContext = processContextMap_[processName];
    if (processContext == nullptr) {
        HILOGE("[SA Scheduler][processName: %{public}s] context is nullptr", Str16ToStr8(processName).c_str());
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
    HILOGI("[SA Scheduler][SA: %{public}d] handle load event from CheckSystemAbility",
        systemAbilityId);
    std::shared_ptr<SystemAbilityContext> abilityContext;
    if (!GetSystemAbilityContext(systemAbilityId, abilityContext)) {
        return ERR_INVALID_VALUE;
    }
    std::lock_guard<std::mutex> autoLock(abilityContext->ownProcessContext->processLock);
    if (abilityContext->state ==SystemAbilityState::UNLOADING
        || abilityContext->ownProcessContext->state == SystemProcessState::STOPPING) {
        isExist = true;
        return ERR_OK;
    }
    bool result = SystemAbilityManager::GetInstance()->DoLoadOnDemandAbility(systemAbilityId, isExist);
    if (result && abilityContext->state == SystemAbilityState::NOT_LOADED) {
        return stateMachine_->AbilityStateTransitionLocked(abilityContext, SystemAbilityState::LOADING);
    }
    return result;
}

int32_t SystemAbilityStateScheduler::HandleLoadAbilityEvent(const LoadRequestInfo& loadRequestInfo)
{
    HILOGI("[SA Scheduler][SA: %{public}d] handle load event start, deviceId: %{public}s, callingpid: %{public}d",
        loadRequestInfo.systemAbilityId, loadRequestInfo.deviceId.c_str(), loadRequestInfo.callingPid);
    std::shared_ptr<SystemAbilityContext> abilityContext;
    if (!GetSystemAbilityContext(loadRequestInfo.systemAbilityId, abilityContext)) {
        return ERR_INVALID_VALUE;
    }
    std::lock_guard<std::mutex> autoLock(abilityContext->ownProcessContext->processLock);
    if (abilityContext->state ==SystemAbilityState::UNLOADING
        || abilityContext->ownProcessContext->state == SystemProcessState::STOPPING) {
        return PendLoadEventLocked(abilityContext, loadRequestInfo);
    }
    int32_t result = ERR_INVALID_VALUE;
    switch (abilityContext->state) {
        case SystemAbilityState::LOADING:
            result = RemovePendingUnloadEventLocked(abilityContext);
            break;
        case SystemAbilityState::LOADED:
            result = RemoveDelayUnloadEvent(abilityContext->systemAbilityId);
            break;
        case SystemAbilityState::UNLOADABLE:
            result = stateMachine_->AbilityStateTransitionLocked(abilityContext, SystemAbilityState::LOADED);
            break;
        case SystemAbilityState::NOT_LOADED:
            result = ERR_OK;
            break;
        default:
            result = ERR_OK;
            HILOGI("[SA Scheduler][SA: %{public}d] in state %{public}d, not need handle load event",
                loadRequestInfo.systemAbilityId, abilityContext->state);
            break;
    }
    if (result == ERR_OK) {
        return DoLoadSystemAbilityLocked(abilityContext, loadRequestInfo);
    }
    return result;
}

int32_t SystemAbilityStateScheduler::HandleUnloadAbilityEvent(int32_t systemAbilityId)
{
    HILOGI("[SA Scheduler][SA: %{public}d] handle unload event start", systemAbilityId);
    std::shared_ptr<SystemAbilityContext> abilityContext;
    if (!GetSystemAbilityContext(systemAbilityId, abilityContext)) {
        return ERR_INVALID_VALUE;
    }
    std::lock_guard<std::mutex> autoLock(abilityContext->ownProcessContext->processLock);
    int32_t result = ERR_INVALID_VALUE;
    switch (abilityContext->state) {
        case SystemAbilityState::LOADING:
            result = PendUnloadEventLocked(abilityContext);
            break;
        case SystemAbilityState::LOADED:
            result = SendDelayUnloadEvent(systemAbilityId);
            break;
        default:
            result = ERR_OK;
            HILOGI("[SA Scheduler][SA: %{public}d] in state %{public}d, not need handle unload event",
                systemAbilityId, abilityContext->state);
            break;
    }
    return result;
}

int32_t SystemAbilityStateScheduler::SendAbilityStateEvent(int32_t systemAbilityId, AbilityStateEvent event)
{
    HILOGI("[SA Scheduler][SA: %{public}d] receive state event", systemAbilityId);
    std::shared_ptr<SystemAbilityContext> abilityContext;
    if (!GetSystemAbilityContext(systemAbilityId, abilityContext)) {
        return ERR_INVALID_VALUE;
    }
    std::lock_guard<std::mutex> autoLock(abilityContext->ownProcessContext->processLock);
    return stateEventHandler_->HandleAbilityEventLocked(abilityContext, event);
}

int32_t SystemAbilityStateScheduler::SendProcessStateEvent(const ProcessInfo& processInfo, ProcessStateEvent event)
{
    HILOGI("[SA Scheduler][process: %{public}s] receive state event",
        Str16ToStr8(processInfo.processName).c_str());
    std::shared_ptr<SystemProcessContext> processContext;
    if (!GetSystemProcessContext(processInfo.processName, processContext)) {
        return ERR_INVALID_VALUE;
    }
    std::lock_guard<std::mutex> autoLock(processContext->processLock);
    processContext->pid = processInfo.pid;
    processContext->uid = processInfo.uid;
    return stateEventHandler_->HandleProcessEventLocked(processContext, event);
}

int32_t SystemAbilityStateScheduler::SendDelayUnloadEvent(int32_t systemAbilityId)
{
    HILOGI("[SA Scheduler][SA: %{public}d] send delay unload event", systemAbilityId);
    if (unloadEventHandler_ == nullptr) {
        HILOGE("[SA Scheduler] unload handler not initialized!");
        return ERR_INVALID_VALUE;
    }
    bool ret = unloadEventHandler_->SendEvent(systemAbilityId, 0, UNLOAD_DELAY_TIME);
    if (!ret) {
        HILOGE("[SA Scheduler] send event failed!");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

int32_t SystemAbilityStateScheduler::RemoveDelayUnloadEvent(int32_t systemAbilityId)
{
    HILOGI("[SA Scheduler][SA: %{public}d] remove delay unload event", systemAbilityId);
    if (unloadEventHandler_ == nullptr) {
        HILOGE("[SA Scheduler] unload handler not initialized!");
        return ERR_INVALID_VALUE;
    }
    unloadEventHandler_->RemoveEvent(systemAbilityId);
    return ERR_OK;
}

int32_t SystemAbilityStateScheduler::PendLoadEventLocked(const std::shared_ptr<SystemAbilityContext>& abilityContext,
    const LoadRequestInfo& loadRequestInfo)
{
    HILOGI("[SA Scheduler][SA: %{public}d] save load event", abilityContext->systemAbilityId);
    if (loadRequestInfo.callback == nullptr) {
        HILOGW("[SA Scheduler] callback invalid!");
        return ERR_INVALID_VALUE;
    }
    for (const auto& loadEventItem : abilityContext->pendingLoadEventList) {
        if (loadRequestInfo.callback->AsObject() == loadEventItem.callback->AsObject()) {
            HILOGI("[SA Scheduler][SA: %{public}d] already existed callback object",
                abilityContext->systemAbilityId);
            return ERR_OK;
        }
    }
    auto& count = abilityContext->pendingLoadEventCountMap[loadRequestInfo.callingPid];
    if (count >= MAX_SUBSCRIBE_COUNT) {
        HILOGE("[SA Scheduler][SA: %{public}d] pid:%{public}d overflow max callback count!",
            abilityContext->systemAbilityId, loadRequestInfo.callingPid);
        return ERR_PERMISSION_DENIED;
    }
    ++count;
    abilityContext->pendingLoadEventList.emplace_back(loadRequestInfo);
    abilityContext->pendingEvent = PendingEvent::LOAD_ABILITY_EVENT;
    return ERR_OK;
}

int32_t SystemAbilityStateScheduler::PendUnloadEventLocked(
    const std::shared_ptr<SystemAbilityContext>& abilityContext)
{
    HILOGI("[SA Scheduler][SA: %{public}d] save unload event", abilityContext->systemAbilityId);
    abilityContext->pendingEvent = PendingEvent::UNLOAD_ABILITY_EVENT;
    return ERR_OK;
}

int32_t SystemAbilityStateScheduler::RemovePendingUnloadEventLocked(
    const std::shared_ptr<SystemAbilityContext>& abilityContext)
{
    if (abilityContext->pendingEvent == PendingEvent::UNLOAD_ABILITY_EVENT) {
        HILOGI("[SA Scheduler][SA: %{public}d] remove pending unload event", abilityContext->systemAbilityId);
        abilityContext->pendingEvent = PendingEvent::NO_EVENT;
    }
    return ERR_OK;
}

int32_t SystemAbilityStateScheduler::HandlePendingLoadEventLocked(
    const std::shared_ptr<SystemAbilityContext>& abilityContext)
{
    HILOGI("[SA Scheduler][SA: %{public}d] handle pending load event start", abilityContext->systemAbilityId);
    if (abilityContext->pendingEvent != PendingEvent::LOAD_ABILITY_EVENT
        || abilityContext->state != SystemAbilityState::NOT_LOADED) {
        HILOGI("[SA Scheduler][SA: %{public}d] no pending load event", abilityContext->systemAbilityId);
        return ERR_OK;
    }
    for (auto& loadRequestInfo : abilityContext->pendingLoadEventList) {
        int32_t result = DoLoadSystemAbilityLocked(abilityContext, loadRequestInfo);
        if (result != ERR_OK) {
            HILOGE("[SA Scheduler][SA: %{public}d] handle pending load event failed, callingPid: %{public}d",
                abilityContext->systemAbilityId, loadRequestInfo.callingPid);
        }
    }
    abilityContext->pendingLoadEventList.clear();
    abilityContext->pendingLoadEventCountMap.clear();
    abilityContext->pendingEvent = PendingEvent::NO_EVENT;
    return ERR_OK;
}

int32_t SystemAbilityStateScheduler::HandlePendingUnloadEventLocked(
    const std::shared_ptr<SystemAbilityContext>& abilityContext)
{
    HILOGI("[SA Scheduler][SA: %{public}d] handle pending unload event start", abilityContext->systemAbilityId);
    if (abilityContext->pendingEvent != PendingEvent::UNLOAD_ABILITY_EVENT
        || abilityContext->state != SystemAbilityState::LOADED) {
        HILOGI("[SA Scheduler][SA: %{public}d] no pending unload event", abilityContext->systemAbilityId);
        return ERR_OK;
    }
    int32_t result = ERR_OK;
    result = SendDelayUnloadEvent(abilityContext->systemAbilityId);
    abilityContext->pendingEvent = PendingEvent::NO_EVENT;
    return result;
}

int32_t SystemAbilityStateScheduler::DoLoadSystemAbilityLocked(
    const std::shared_ptr<SystemAbilityContext>& abilityContext, const LoadRequestInfo& loadRequestInfo)
{
    int32_t result = ERR_OK;
    if (loadRequestInfo.deviceId == LOCAL_DEVICE) {
        HILOGI("[SA Scheduler][SA: %{public}d] load ability from local start", abilityContext->systemAbilityId);
        result = SystemAbilityManager::GetInstance()->DoLoadSystemAbility(abilityContext->systemAbilityId,
            abilityContext->ownProcessContext->processName, loadRequestInfo.callback, loadRequestInfo.callingPid);
    } else {
        HILOGI("[SA Scheduler][SA: %{public}d] load ability from remote start", abilityContext->systemAbilityId);
        result = SystemAbilityManager::GetInstance()->DoLoadSystemAbilityFromRpc(loadRequestInfo.deviceId,
            abilityContext->systemAbilityId, abilityContext->ownProcessContext->processName, loadRequestInfo.callback);
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
        HILOGE("[SA Scheduler] process context is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::lock_guard<std::mutex> autoLock(processContext->processLock);
    if (CanUnloadAllSystemAbility(processContext)) {
        return UnloadAllSystemAbilityLocked(processContext);
    }
    return ERR_OK;
}

bool SystemAbilityStateScheduler::CanUnloadAllSystemAbility(
    const std::shared_ptr<SystemProcessContext>& processContext)
{
    std::shared_lock<std::shared_mutex> sharedLock(processContext->stateCountLock);
    uint32_t notLoadAbilityCount = processContext->abilityStateCountMap[SystemAbilityState::NOT_LOADED];
    uint32_t unloadableAbilityCount = processContext->abilityStateCountMap[SystemAbilityState::UNLOADABLE];
    HILOGI("[SA Scheduler][process: %{public}s] SA num: %{public}zu, notloaded: %{public}d, unloadable: %{public}d",
        Str16ToStr8(processContext->processName).c_str(), processContext->saList.size(), notLoadAbilityCount,
        unloadableAbilityCount);
    if (unloadableAbilityCount <= 0) {
        return false;
    }
    if (notLoadAbilityCount + unloadableAbilityCount == processContext->saList.size()) {
        return true;
    }
    return false;
}

int32_t SystemAbilityStateScheduler::UnloadAllSystemAbilityLocked(
    const std::shared_ptr<SystemProcessContext>& processContext)
{
    HILOGI("[SA Scheduler][process: %{public}s] unload all SA", Str16ToStr8(processContext->processName).c_str());
    int32_t result = ERR_OK;
    for (auto& saId : processContext->saList) {
        std::shared_ptr<SystemAbilityContext> abilityContext;
        if (!GetSystemAbilityContext(saId, abilityContext)) {
            continue;
        }
        if (abilityContext->state == SystemAbilityState::UNLOADABLE) {
            result = DoUnloadSystemAbilityLocked(abilityContext);
        }
        if (result != ERR_OK) {
            HILOGE("[SA Scheduler][SA: %{public}d] unload failed", saId);
        }
    }
    auto timeoutTask = [this, processContext] () {
        std::lock_guard<std::mutex> autoLock(processContext->processLock);
        if (processContext->state == SystemProcessState::STOPPING) {
            HILOGW("[SA Scheduler][process: %{public}s] unload sa timeout",
                Str16ToStr8(processContext->processName).c_str());
            KillSystemProcessLocked(processContext);
        }
    };
    processHandler_->PostTask(timeoutTask, UNLOAD_TIMEOUT_TIME);
    return stateMachine_->ProcessStateTransitionLocked(processContext, SystemProcessState::STOPPING);
}

int32_t SystemAbilityStateScheduler::DoUnloadSystemAbilityLocked(
    const std::shared_ptr<SystemAbilityContext>& abilityContext)
{
    int32_t result = ERR_OK;
    HILOGI("[SA Scheduler][SA: %{public}d] unload start", abilityContext->systemAbilityId);
    result = SystemAbilityManager::GetInstance()->DoUnloadSystemAbility(abilityContext->systemAbilityId,
        abilityContext->ownProcessContext->processName);
    if (result == ERR_OK) {
        return stateMachine_->AbilityStateTransitionLocked(abilityContext, SystemAbilityState::UNLOADING);
    }
    return result;
}

int32_t SystemAbilityStateScheduler::TryKillSystemProcess(
    const std::shared_ptr<SystemProcessContext>& processContext)
{
    if (processContext == nullptr) {
        HILOGE("[SA Scheduler] process context is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::lock_guard<std::mutex> autoLock(processContext->processLock);
    if (CanKillSystemProcess(processContext)) {
        return KillSystemProcessLocked(processContext);
    }
    return ERR_OK;
}

bool SystemAbilityStateScheduler::CanKillSystemProcess(
    const std::shared_ptr<SystemProcessContext>& processContext)
{
    std::shared_lock<std::shared_mutex> sharedLock(processContext->stateCountLock);
    uint32_t notLoadAbilityCount = processContext->abilityStateCountMap[SystemAbilityState::NOT_LOADED];
    HILOGI("[SA Scheduler][process: %{public}s] SA num: %{public}zu, not loaded num: %{public}d",
        Str16ToStr8(processContext->processName).c_str(), processContext->saList.size(), notLoadAbilityCount);
    if (notLoadAbilityCount == processContext->saList.size()) {
        return true;
    }
    return false;
}

int32_t SystemAbilityStateScheduler::KillSystemProcessLocked(
    const std::shared_ptr<SystemProcessContext>& processContext)
{
    #ifdef RESSCHED_ENABLE
    HILOGI("[SA Scheduler][process: %{public}s] kill system process start, pid: %{public}d, uid: %{public}d",
        Str16ToStr8(processContext->processName).c_str(), processContext->pid, processContext->uid);
    std::unordered_map<std::string, std::string> payload;
    payload["pid"] = std::to_string(processContext->pid);
    payload["uid"] = std::to_string(processContext->uid);
    payload["processName"] = Str16ToStr8(processContext->processName);
    ResourceSchedule::ResSchedClient::GetInstance().KillProcess(payload);
    #endif
    return ERR_OK;
}

void SystemAbilityStateScheduler::OnProcessNotStartedLocked(const std::u16string& processName)
{
    HILOGI("[SA Scheduler][process: %{public}s] stopped", Str16ToStr8(processName).c_str());
    std::shared_ptr<SystemProcessContext> processContext;
    if (!GetSystemProcessContext(processName, processContext)) {
        return;
    }
    for (auto& saId : processContext->saList) {
        std::shared_ptr<SystemAbilityContext> abilityContext;
        if (!GetSystemAbilityContext(saId, abilityContext)) {
            return;
        }
        int32_t result = ERR_OK;
        result = stateMachine_->AbilityStateTransitionLocked(abilityContext, SystemAbilityState::NOT_LOADED);
        if (result == ERR_OK) {
            HandlePendingLoadEventLocked(abilityContext);
        }
    }
}

void SystemAbilityStateScheduler::OnAbilityNotLoadedLocked(int32_t systemAbilityId)
{
    HILOGI("[SA Scheduler][SA: %{public}d] not loaded", systemAbilityId);
    std::shared_ptr<SystemAbilityContext> abilityContext;
    if (!GetSystemAbilityContext(systemAbilityId, abilityContext)) {
        return;
    }
    bool result = true;
    if (abilityContext->ownProcessContext->state == SystemProcessState::STOPPING) {
        result = processHandler_->PostTask([this, abilityContext] () {
            int32_t ret = TryKillSystemProcess(abilityContext->ownProcessContext);
            if (ret != ERR_OK) {
                HILOGE("[SA Scheduler][process: %{public}s] kill process failed",
                    Str16ToStr8(abilityContext->ownProcessContext->processName).c_str());
            }
        });
    } else if (abilityContext->ownProcessContext->state == SystemProcessState::STARTED) {
        result = processHandler_->PostTask([this, abilityContext] () {
            int32_t ret = TryUnloadAllSystemAbility(abilityContext->ownProcessContext);
            if (ret != ERR_OK) {
                HILOGE("[SA Scheduler][process: %{public}s] unload all SA failed",
                    Str16ToStr8(abilityContext->ownProcessContext->processName).c_str());
            }
        });
    }
    if (!result) {
        HILOGE("[SA Scheduler] post task failed");
    }
}

void SystemAbilityStateScheduler::OnAbilityLoadedLocked(int32_t systemAbilityId)
{
    HILOGI("[SA Scheduler][SA: %{public}d] loaded", systemAbilityId);
    std::shared_ptr<SystemAbilityContext> abilityContext;
    if (!GetSystemAbilityContext(systemAbilityId, abilityContext)) {
        return;
    }
    HandlePendingUnloadEventLocked(abilityContext);
}

void SystemAbilityStateScheduler::OnAbilityUnloadableLocked(int32_t systemAbilityId)
{
    HILOGI("[SA Scheduler][SA: %{public}d] unloadable", systemAbilityId);
    std::shared_ptr<SystemAbilityContext> abilityContext;
    if (!GetSystemAbilityContext(systemAbilityId, abilityContext)) {
        return;
    }
    bool result = processHandler_->PostTask([this, abilityContext] () {
        int32_t ret = TryUnloadAllSystemAbility(abilityContext->ownProcessContext);
        if (ret != ERR_OK) {
            HILOGE("[SA Scheduler][process: %{public}s] unload all SA failed",
                Str16ToStr8(abilityContext->ownProcessContext->processName).c_str());
        }
    });
    if (!result) {
        HILOGE("[SA Scheduler] post unload all SA task failed");
    }
}

int32_t SystemAbilityStateScheduler::ProcessDelayUnloadEvent(int32_t systemAbilityId)
{
    HILOGI("[SA Scheduler][SA: %{public}d] process delay unload event", systemAbilityId);
    std::shared_ptr<SystemAbilityContext> abilityContext;
    if (!GetSystemAbilityContext(systemAbilityId, abilityContext)) {
        return ERR_INVALID_VALUE;
    }
    std::lock_guard<std::mutex> autoLock(abilityContext->ownProcessContext->processLock);
    return stateMachine_->AbilityStateTransitionLocked(abilityContext, SystemAbilityState::UNLOADABLE);
}

void SystemAbilityStateScheduler::UnloadEventHandler::ProcessEvent(const OHOS::AppExecFwk::InnerEvent::Pointer& event)
{
    if (event == nullptr) {
        HILOGE("[SA Scheduler] ProcessEvent event is nullptr!");
        return;
    }
    auto eventId = event->GetInnerEventId();
    int32_t systemAbilityId = static_cast<int32_t>(eventId);
    auto stateScheduler = stateScheduler_.lock();
    int32_t result = ERR_OK;
    if (stateScheduler != nullptr) {
        result = stateScheduler->ProcessDelayUnloadEvent(systemAbilityId);
    }
    if (result != ERR_OK) {
        HILOGE("[SA Scheduler][SA: %{public}d] process delay unload event failed", systemAbilityId);
    }
}
}  // namespace OHOS