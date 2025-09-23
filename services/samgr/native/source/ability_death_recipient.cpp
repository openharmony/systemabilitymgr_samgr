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
#include "ipc_skeleton.h"
#include "hitrace_meter.h"

namespace OHOS {
void AbilityDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    auto callPid = IPCSkeleton::GetCallingPid();
    HILOGD("AbilityDeathRecipient OnRemoteDied called, pid:%{public}d", callingPid);
    string OnRemoteDiedTag = ToString(callingPid) + "_AbilityDeathRecipient";
    HitraceScopedEx samgrHitrace(HITRACE_LEVEL_INFO, HITRACE_TAG_SAMGR, OnRemoteDiedTag.c_str());
    SystemAbilityManager::GetInstance()->RemoveSystemAbility(remote.promote());
    HILOGD("AbilityDeathRecipients death notice success");
}

void SystemProcessDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    auto callPid = IPCSkeleton::GetCallingPid();
    HILOGD("SystemProcessDeathRecipient called!, pid:%{public}d", callingPid);
    string OnRemoteDiedTag = ToString(callingPid) + "_SystemProcessDeathRecipient";
    HitraceScopedEx samgrHitrace(HITRACE_LEVEL_INFO, HITRACE_TAG_SAMGR, OnRemoteDiedTag.c_str());
    SystemAbilityManager::GetInstance()->RemoveSystemProcess(remote.promote());
    HILOGD("SystemProcessDeathRecipient death notice success");
}

void AbilityStatusDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    auto callPid = IPCSkeleton::GetCallingPid();
    HILOGD("AbilityStatusDeathRecipient called!, pid:%{public}d", callingPid);
    string OnRemoteDiedTag = ToString(callingPid) + "_AbilityStatusDeathRecipient";
    HitraceScopedEx samgrHitrace(HITRACE_LEVEL_INFO, HITRACE_TAG_SAMGR, OnRemoteDiedTag.c_str());
    SystemAbilityManager::GetInstance()->UnSubscribeSystemAbility(remote.promote());
    HILOGD("AbilityStatusDeathRecipient death notice success");
}

void AbilityCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    auto callPid = IPCSkeleton::GetCallingPid();
    HILOGD("AbilityCallbackDeathRecipient called!, pid:%{public}d", callingPid);
    string OnRemoteDiedTag = ToString(callingPid) + "_AbilityCallbackDeathRecipient";
    HitraceScopedEx samgrHitrace(HITRACE_LEVEL_INFO, HITRACE_TAG_SAMGR, OnRemoteDiedTag.c_str());
    SystemAbilityManager::GetInstance()->OnAbilityCallbackDied(remote.promote());
    HILOGD("AbilityCallbackDeathRecipient death notice success");
}

void RemoteCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    auto callPid = IPCSkeleton::GetCallingPid();
    HILOGD("RemoteCallbackDeathRecipient called!, pid:%{public}d", callingPid);
    string OnRemoteDiedTag = ToString(callingPid) + "_RemoteCallbackDeathRecipient";
    HitraceScopedEx samgrHitrace(HITRACE_LEVEL_INFO, HITRACE_TAG_SAMGR, OnRemoteDiedTag.c_str());
    SystemAbilityManager::GetInstance()->OnRemoteCallbackDied(remote.promote());
    HILOGD("RemoteCallbackDeathRecipient death notice success");
}

void SystemProcessListenerDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    auto callPid = IPCSkeleton::GetCallingPid();
    HILOGD("SystemProcessListenerDeathRecipient called!, pid:%{public}d", callingPid);
    string OnRemoteDiedTag = ToString(callingPid) + "_SystemProcessListenerDeathRecipient";
    HitraceScopedEx samgrHitrace(HITRACE_LEVEL_INFO, HITRACE_TAG_SAMGR, OnRemoteDiedTag.c_str());
    sptr<ISystemProcessStatusChange> listener = iface_cast<ISystemProcessStatusChange>(remote.promote());
    SystemAbilityManager::GetInstance()->UnSubscribeSystemProcess(listener);
    HILOGD("SystemProcessListenerDeathRecipient death notice success");
}
} // namespace OHOS
