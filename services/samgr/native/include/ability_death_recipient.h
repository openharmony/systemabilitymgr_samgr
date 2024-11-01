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

#ifndef SERVICES_SAMGR_NATIVE_INCLUDE_ABILITY_DEATH_RECIPIENT_H
#define SERVICES_SAMGR_NATIVE_INCLUDE_ABILITY_DEATH_RECIPIENT_H

#include "iremote_object.h"

namespace OHOS {
class AbilityDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    void OnRemoteDied(const wptr<IRemoteObject>& remote) override;
    AbilityDeathRecipient() = default;
    ~AbilityDeathRecipient() override = default;
};

class SystemProcessDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    void OnRemoteDied(const wptr<IRemoteObject>& remote) override;
    SystemProcessDeathRecipient() = default;
    ~SystemProcessDeathRecipient() override = default;
};

class AbilityStatusDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    void OnRemoteDied(const wptr<IRemoteObject>& remote) override;
    AbilityStatusDeathRecipient() = default;
    ~AbilityStatusDeathRecipient() override = default;
};

class AbilityCallbackDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    void OnRemoteDied(const wptr<IRemoteObject>& remote) override;
    AbilityCallbackDeathRecipient() = default;
    ~AbilityCallbackDeathRecipient() override = default;
};

class RemoteCallbackDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    void OnRemoteDied(const wptr<IRemoteObject>& remote) override;
    RemoteCallbackDeathRecipient() = default;
    ~RemoteCallbackDeathRecipient() override = default;
};

class SystemProcessListenerDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    void OnRemoteDied(const wptr<IRemoteObject>& remote) override;
    SystemProcessListenerDeathRecipient() = default;
    ~SystemProcessListenerDeathRecipient() override = default;
};
} // namespace OHOS

#endif // !defined(SERVICES_SAMGR_NATIVE_INCLUDE_ABILITY_DEATH_RECIPIENT_H)
