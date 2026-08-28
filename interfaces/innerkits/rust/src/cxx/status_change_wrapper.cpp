// Copyright (C) 2024-2026 Huawei Device Co., Ltd.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "status_change_wrapper.h"

#include <algorithm>
#include <memory>
#include <vector>

#include "cxx.h"
#include "iservice_registry.h"
#include "wrapper.rs.h"
namespace OHOS {
namespace SamgrRust {
SystemAbilityStatusChangeWrapper::SystemAbilityStatusChangeWrapper(
    const rust::Fn<void(int32_t systemAbilityId, const rust::str deviceId)> onAdd,
    const rust::Fn<void(int32_t systemAbilityId, const rust::str deviceId)> onRemove)
{
    this->onAdd_ = onAdd;
    this->onRemove_ = onRemove;
}

void SystemAbilityStatusChangeWrapper::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    onAdd_(systemAbilityId, deviceId);
}

void SystemAbilityStatusChangeWrapper::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    onRemove_(systemAbilityId, deviceId);
}

SystemProcessStatusChangeWrapper::SystemProcessStatusChangeWrapper(
    const rust::Fn<void(const OHOS::SamgrRust::SystemProcessInfo &systemProcessInfo)> onStart,
    const rust::Fn<void(const OHOS::SamgrRust::SystemProcessInfo &systemProcessInfo)> onStop)
{
    this->onStart_ = onStart;
    this->onStop_ = onStop;
}

void SystemProcessStatusChangeWrapper::OnSystemProcessStarted(OHOS::SystemProcessInfo &systemProcessInfo)
{
    auto info = OHOS::SamgrRust::SystemProcessInfo{
        .processName = systemProcessInfo.processName,
        .pid = systemProcessInfo.pid,
        .uid = systemProcessInfo.uid,
    };
    onStart_(info);
}
void SystemProcessStatusChangeWrapper::OnSystemProcessStopped(OHOS::SystemProcessInfo &systemProcessInfo)
{
    auto info = OHOS::SamgrRust::SystemProcessInfo{
        .processName = systemProcessInfo.processName,
        .pid = systemProcessInfo.pid,
        .uid = systemProcessInfo.uid,
    };
    onStop_(info);
}

UnSubscribeSystemAbilityHandler::UnSubscribeSystemAbilityHandler(
    int32_t systemAbilityId, sptr<ISystemAbilityStatusChange> listener)
{
    this->said_ = systemAbilityId;
    this->listener_ = listener;
}

void UnSubscribeSystemAbilityHandler::UnSubscribe()
{
    SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager()->UnSubscribeSystemAbility(said_, listener_);
}

#ifdef SUPPORT_MULTI_INSTANCE
int32_t UnSubscribeSystemAbilityHandler::UnSubscribeSystemAbilityByUserId(int32_t userId)
{
    auto sysm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sysm == nullptr) {
        return -1;
    }
    return sysm->UnSubscribeSystemAbility(said_, listener_, userId);
}
#endif

UnSubscribeSystemProcessHandler::UnSubscribeSystemProcessHandler(sptr<ISystemProcessStatusChange> listener)
{
    this->listener_ = listener;
}

void UnSubscribeSystemProcessHandler::UnSubscribe()
{
    SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager()->UnSubscribeSystemProcess(listener_);
}

#ifdef SUPPORT_MULTI_INSTANCE
int32_t UnSubscribeSystemProcessHandler::UnSubscribeSystemProcessByUserId(int32_t userId)
{
    auto sysm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sysm == nullptr) {
        return -1;
    }
    return sysm->UnSubscribeSystemProcess(listener_, userId);
}
#endif

} // namespace SamgrRust
} // namespace OHOS