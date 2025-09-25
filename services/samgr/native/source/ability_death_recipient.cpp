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

#include "iremote_proxy.h"
#include "sam_log.h"
#include "system_ability_manager.h"
#include "hitrace_meter.h"

namespace OHOS {
void AbilityDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    HILOGD("AbilityDeathRecipient OnRemoteDied called");
    string OnRemoteDiedTag = "_AbilityDeath";
    HitraceScopedEx samgrHitrace(HITRACE_LEVEL_INFO, HITRACE_TAG_SAMGR, OnRemoteDiedTag.c_str());
    SystemAbilityManager::GetInstance()->RemoveSystemAbility(remote.promote());
    HILOGD("AbilityDeathRecipients death notice success");
}

void SystemProcessDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    HILOGD("SystemProcessDeathRecipient called!");
    string OnRemoteDiedTag = "_SystemProcessDeath";
    HitraceScopedEx samgrHitrace(HITRACE_LEVEL_INFO, HITRACE_TAG_SAMGR, OnRemoteDiedTag.c_str());
    SystemAbilityManager::GetInstance()->RemoveSystemProcess(remote.promote());
    HILOGD("SystemProcessDeathRecipient death notice success");
}

void AbilityStatusDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    HILOGD("AbilityStatusDeathRecipient called!");
    string OnRemoteDiedTag = "_AbilityStatusDeath";
    HitraceScopedEx samgrHitrace(HITRACE_LEVEL_INFO, HITRACE_TAG_SAMGR, OnRemoteDiedTag.c_str());
    SystemAbilityManager::GetInstance()->UnSubscribeSystemAbility(remote.promote());
    HILOGD("AbilityStatusDeathRecipient death notice success");
}

void AbilityCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    auto callPid = IPCSkeleton::GetCallingPid();
    HILOGD("AbilityCallbackDeathRecipient called!");
    string OnRemoteDiedTag = "_AbilityCallbackDeath";
    HitraceScopedEx samgrHitrace(HITRACE_LEVEL_INFO, HITRACE_TAG_SAMGR, OnRemoteDiedTag.c_str());
    SystemAbilityManager::GetInstance()->OnAbilityCallbackDied(remote.promote());
    HILOGD("AbilityCallbackDeathRecipient death notice success");
}

void RemoteCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    auto callPid = IPCSkeleton::GetCallingPid();
    HILOGD("RemoteCallbackDeathRecipient called!");
    string OnRemoteDiedTag = "_RemoteCallbackDeath";
    HitraceScopedEx samgrHitrace(HITRACE_LEVEL_INFO, HITRACE_TAG_SAMGR, OnRemoteDiedTag.c_str());
    SystemAbilityManager::GetInstance()->OnRemoteCallbackDied(remote.promote());
    HILOGD("RemoteCallbackDeathRecipient death notice success");
}

void SystemProcessListenerDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    auto callPid = IPCSkeleton::GetCallingPid();
    HILOGD("SystemProcessListenerDeathRecipient called!");
    string OnRemoteDiedTag = "_SystemProcessListenerDeath";
    HitraceScopedEx samgrHitrace(HITRACE_LEVEL_INFO, HITRACE_TAG_SAMGR, OnRemoteDiedTag.c_str());
    sptr<ISystemProcessStatusChange> listener = iface_cast<ISystemProcessStatusChange>(remote.promote());
    SystemAbilityManager::GetInstance()->UnSubscribeSystemProcess(listener);
    HILOGD("SystemProcessListenerDeathRecipient death notice success");
}
} // namespace OHOS
