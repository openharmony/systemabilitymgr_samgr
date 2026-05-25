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

#include <memory>
#include "iremote_object.h"

namespace OHOS {

class BaseSystemAbilityManager;

class AbilityDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    explicit AbilityDeathRecipient(const std::weak_ptr<BaseSystemAbilityManager>& manager);
    ~AbilityDeathRecipient() override = default;
    void OnRemoteDied(const wptr<IRemoteObject>& remote) override;
private:
    std::weak_ptr<BaseSystemAbilityManager> manager_;
};

class SystemProcessDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    explicit SystemProcessDeathRecipient(const std::weak_ptr<BaseSystemAbilityManager>& manager);
    ~SystemProcessDeathRecipient() override = default;
    void OnRemoteDied(const wptr<IRemoteObject>& remote) override;
private:
    std::weak_ptr<BaseSystemAbilityManager> manager_;
};

class AbilityStatusDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    explicit AbilityStatusDeathRecipient(const std::weak_ptr<BaseSystemAbilityManager>& manager);
    ~AbilityStatusDeathRecipient() override = default;
    void OnRemoteDied(const wptr<IRemoteObject>& remote) override;
private:
    std::weak_ptr<BaseSystemAbilityManager> manager_;
};

class AbilityCallbackDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    explicit AbilityCallbackDeathRecipient(const std::weak_ptr<BaseSystemAbilityManager>& manager);
    ~AbilityCallbackDeathRecipient() override = default;
    void OnRemoteDied(const wptr<IRemoteObject>& remote) override;
private:
    std::weak_ptr<BaseSystemAbilityManager> manager_;
};

class RemoteCallbackDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    explicit RemoteCallbackDeathRecipient(const std::weak_ptr<BaseSystemAbilityManager>& manager);
    ~RemoteCallbackDeathRecipient() override = default;
    void OnRemoteDied(const wptr<IRemoteObject>& remote) override;
private:
    std::weak_ptr<BaseSystemAbilityManager> manager_;
};

class SystemProcessListenerDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    explicit SystemProcessListenerDeathRecipient(const std::weak_ptr<BaseSystemAbilityManager>& manager);
    ~SystemProcessListenerDeathRecipient() override = default;
    void OnRemoteDied(const wptr<IRemoteObject>& remote) override;
private:
    std::weak_ptr<BaseSystemAbilityManager> manager_;
};
} // namespace OHOS

#endif // !defined(SERVICES_SAMGR_NATIVE_INCLUDE_ABILITY_DEATH_RECIPIENT_H)
