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

#ifndef OHOS_SYSTEM_ABILITY_MANAGER_SYSTEM_ABILITY_STATE_CONTEXT_H
#define OHOS_SYSTEM_ABILITY_MANAGER_SYSTEM_ABILITY_STATE_CONTEXT_H

#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <shared_mutex>

#include "refbase.h"
#include "sa_profiles.h"
#include "isystem_ability_load_callback.h"
#include "sa_profiles.h"

namespace OHOS {
enum class SystemAbilityState {
    NOT_LOADED = 0,
    LOADING,
    LOADED,
    UNLOADABLE,
    UNLOADING,
};

enum class SystemProcessState {
    NOT_STARTED = 0,
    STARTED,
    STOPPING,
};

enum class PendingEvent {
    NO_EVENT = 0,
    LOAD_ABILITY_EVENT,
    UNLOAD_ABILITY_EVENT,
};

struct LoadRequestInfo {
    std::string deviceId;
    sptr<ISystemAbilityLoadCallback> callback;
    int32_t systemAbilityId = -1;
    int32_t callingPid = -1;
    OnDemandEvent loadEvent;
};

struct UnloadRequestInfo {
    OnDemandEvent unloadEvent;
    int32_t systemAbilityId;
    int32_t callingPid;
    UnloadRequestInfo(OnDemandEvent ue, int32_t said = -1, int32_t cpid = -1)
        :unloadEvent(ue), systemAbilityId(said), callingPid(cpid) {}
};

struct SystemProcessContext {
    std::mutex stateCountLock;
    std::mutex processLock;
    std::u16string processName;
    std::list<int32_t> saList;
    std::map<SystemAbilityState, uint32_t> abilityStateCountMap;
    std::list<int64_t> restartCountsCtrl;
    int32_t pid = -1;
    int32_t uid = -1;
    SystemProcessState state = SystemProcessState::NOT_STARTED;
    bool enableRestart = true;
};

struct SystemAbilityContext {
    std::map<int32_t, int32_t> pendingLoadEventCountMap;
    std::list<LoadRequestInfo> pendingLoadEventList;
    std::shared_ptr<SystemProcessContext> ownProcessContext;
    std::shared_ptr<UnloadRequestInfo> pendingUnloadEvent;
    std::shared_ptr<UnloadRequestInfo> unloadRequest;
    int32_t systemAbilityId = -1;
    int32_t delayUnloadTime = 0;
    int64_t lastStartTime = 0;
    SystemAbilityState state = SystemAbilityState::NOT_LOADED;
    PendingEvent pendingEvent = PendingEvent::NO_EVENT;
    bool isAutoRestart = false;
};
} // namespace OHOS

#endif // !defined(OHOS_SYSTEM_ABILITY_MANAGER_SYSTEM_ABILITY_STATE_CONTEXT_H)