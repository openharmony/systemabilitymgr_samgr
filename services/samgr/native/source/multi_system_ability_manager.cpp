/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "multi_system_ability_manager.h"
#include "sam_log.h"
#include "samgr_xcollie.h"
#include "service_control.h"
#include "string_ex.h"
#include "system_ability_manager_util.h"

namespace OHOS {

MultiSystemAbilityManager::MultiSystemAbilityManager(int32_t userId)
    : userId_(userId)
{
    logPrefix_ = "[U" + std::to_string(userId_) + "] ";
}

MultiSystemAbilityManager::~MultiSystemAbilityManager() {}

int32_t MultiSystemAbilityManager::Init(const std::list<SaProfile>& saProfiles)
{
    HILOGI("MultiSAManager Init for userId:%{public}d", userId_);

    BaseSystemAbilityManager::Init();

    std::set<int32_t> multiInstanceSaIds = GetMultiInstanceSaIds();

    std::list<SaProfile> filteredProfiles;
    for (const auto& saProfile : saProfiles) {
        if (multiInstanceSaIds.count(saProfile.saId) > 0) {
            filteredProfiles.push_back(saProfile);
        }
    }

    {
        lock_guard<samgr::mutex> autoLock(saProfileMapLock_);
        saProfileMap_.clear();
        onDemandSaIdsSet_.clear();
        for (const auto& saProfile : filteredProfiles) {
            SamgrUtil::FilterCommonSaProfile(saProfile, saProfileMap_[saProfile.saId]);
            if (!saProfile.runOnCreate) {
                onDemandSaIdsSet_.insert(saProfile.saId);
            }
        }
    }

    abilityStateScheduler_ = std::make_shared<SystemAbilityStateScheduler>();
    if (abilityStateScheduler_ != nullptr) {
        abilityStateScheduler_->Init(this, filteredProfiles);
    }

    collectManager_ = sptr<DeviceStatusCollectManager>(new DeviceStatusCollectManager());
    if (collectManager_ != nullptr) {
        collectManager_->Init(this, filteredProfiles);
    }

    HILOGI("MultiSAManager Init done, userId:%{public}d, saCount:%{public}zu",
        userId_, filteredProfiles.size());
    return ERR_OK;
}

int32_t MultiSystemAbilityManager::Destroy()
{
    HILOGI("MultiSAManager Destroy for userId:%{public}d", userId_);
    BaseSystemAbilityManager::Destroy();
    HILOGI("MultiSAManager Destroy done for userId:%{public}d", userId_);
    return ERR_OK;
}

int32_t MultiSystemAbilityManager::StartDynamicSystemProcess(const std::u16string& name,
    int32_t systemAbilityId, const OnDemandEvent& event)
{
    std::string eventStr = std::to_string(systemAbilityId) + "#" + std::to_string(event.eventId) + "#"
        + event.name + "#" + event.value + "#" + std::to_string(event.extraDataId) + "#"
        + std::to_string(userId_) + "#";
    auto extraArgv = eventStr.c_str();
    if (abilityStateScheduler_ && !abilityStateScheduler_->IsSystemProcessNeverStartedLocked(name)) {
        int ret = ServiceWaitForStatus(Str16ToStr8(name).c_str(), ServiceStatus::SERVICE_STOPPED, 1);
        if (ret != 0) {
            HILOGE("ServiceWaitForStatus proc:%{public}s,SA:%{public}d timeout",
                Str16ToStr8(name).c_str(), systemAbilityId);
        }
    }
    int64_t begin = GetTickCount();
    int result = ERR_INVALID_VALUE;
    if (!IsInitBootFinished()) {
        result = ServiceControlWithExtra(Str16ToStr8(name).c_str(),
            ServiceAction::START, &extraArgv, 1);
    } else {
        SamgrXCollie samgrXCollie("samgr--startProc_" + ToString(systemAbilityId));
        result = ServiceControlWithExtra(Str16ToStr8(name).c_str(),
            ServiceAction::START, &extraArgv, 1);
    }
    int64_t duration = GetTickCount() - begin;
    auto callingPid = IPCSkeleton::GetCallingPid();
    auto callingUid = IPCSkeleton::GetCallingUid();
    if (result != 0) {
        ReportProcessStartFail(Str16ToStr8(name), callingPid, callingUid, "err:" + ToString(result));
    }
    HILOGI("StartUserProc:%{public}s,SA:%{public}d,ret:%{public}d,%{public}" PRId64 "ms,uid:%{public}d",
        Str16ToStr8(name).c_str(), systemAbilityId, result, duration, userId_);
    return result;
}

} // namespace OHOS
