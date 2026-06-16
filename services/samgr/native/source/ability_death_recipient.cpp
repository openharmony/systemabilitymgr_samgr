/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "ability_death_recipient.h"

#include "base_system_ability_manager.h"
#include "iremote_proxy.h"
#include "sam_log.h"
#include "hitrace_meter.h"

namespace OHOS {

AbilityDeathRecipient::AbilityDeathRecipient(const std::weak_ptr<BaseSystemAbilityManager>& manager)
    : manager_(manager) {}

void AbilityDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    HILOGD("AbilityDeathRecipient OnRemoteDied called");
    std::string OnRemoteDiedTag = "AbilityDeath";
    HitraceScopedEx samgrHitrace(HITRACE_LEVEL_INFO, HITRACE_TAG_SAMGR, OnRemoteDiedTag.c_str());
    auto manager = manager_.lock();
    if (manager != nullptr) {
        manager->RemoveSystemAbility(remote.promote());
    }
    HILOGD("AbilityDeathRecipients death notice success");
}

SystemProcessDeathRecipient::SystemProcessDeathRecipient(const std::weak_ptr<BaseSystemAbilityManager>& manager)
    : manager_(manager) {}

void SystemProcessDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    HILOGD("SystemProcessDeathRecipient called!");
    std::string OnRemoteDiedTag = "SystemProcessDeath";
    HitraceScopedEx samgrHitrace(HITRACE_LEVEL_INFO, HITRACE_TAG_SAMGR, OnRemoteDiedTag.c_str());
    auto manager = manager_.lock();
    if (manager != nullptr) {
        manager->RemoveSystemProcess(remote.promote());
    }
    HILOGD("SystemProcessDeathRecipient death notice success");
}

AbilityStatusDeathRecipient::AbilityStatusDeathRecipient(const std::weak_ptr<BaseSystemAbilityManager>& manager)
    : manager_(manager) {}

void AbilityStatusDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    HILOGD("AbilityStatusDeathRecipient called!");
    std::string OnRemoteDiedTag = "AbilityStatusDeath";
    HitraceScopedEx samgrHitrace(HITRACE_LEVEL_INFO, HITRACE_TAG_SAMGR, OnRemoteDiedTag.c_str());
    auto manager = manager_.lock();
    if (manager != nullptr) {
        manager->UnSubscribeSystemAbility(remote.promote());
    }
    HILOGD("AbilityStatusDeathRecipient death notice success");
}

AbilityCallbackDeathRecipient::AbilityCallbackDeathRecipient(const std::weak_ptr<BaseSystemAbilityManager>& manager)
    : manager_(manager) {}

void AbilityCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    HILOGD("AbilityCallbackDeathRecipient called!");
    std::string OnRemoteDiedTag = "AbilityCallbackDeath";
    HitraceScopedEx samgrHitrace(HITRACE_LEVEL_INFO, HITRACE_TAG_SAMGR, OnRemoteDiedTag.c_str());
    auto manager = manager_.lock();
    if (manager != nullptr) {
        manager->OnAbilityCallbackDied(remote.promote());
    }
    HILOGD("AbilityCallbackDeathRecipient death notice success");
}

RemoteCallbackDeathRecipient::RemoteCallbackDeathRecipient(const std::weak_ptr<BaseSystemAbilityManager>& manager)
    : manager_(manager) {}

void RemoteCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    HILOGD("RemoteCallbackDeathRecipient called!");
    std::string OnRemoteDiedTag = "RemoteCallbackDeath";
    HitraceScopedEx samgrHitrace(HITRACE_LEVEL_INFO, HITRACE_TAG_SAMGR, OnRemoteDiedTag.c_str());
    auto manager = manager_.lock();
    if (manager != nullptr) {
        manager->OnRemoteCallbackDied(remote.promote());
    }
    HILOGD("RemoteCallbackDeathRecipient death notice success");
}

SystemProcessListenerDeathRecipient::SystemProcessListenerDeathRecipient(
    const std::weak_ptr<BaseSystemAbilityManager>& manager)
    : manager_(manager) {}

void SystemProcessListenerDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    HILOGD("SystemProcessListenerDeathRecipient called!");
    std::string OnRemoteDiedTag = "SystemProcessListenerDeath";
    HitraceScopedEx samgrHitrace(HITRACE_LEVEL_INFO, HITRACE_TAG_SAMGR, OnRemoteDiedTag.c_str());
    sptr<ISystemProcessStatusChange> listener = iface_cast<ISystemProcessStatusChange>(remote.promote());
    auto manager = manager_.lock();
    if (manager != nullptr) {
        manager->UnSubscribeSystemProcess(listener);
    }
    HILOGD("SystemProcessListenerDeathRecipient death notice success");
}
} // namespace OHOS
