/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_INNERKITS_SAMGR_INCLUDE_STATUS_CHANGE_WRAPPER_H
#define INTERFACES_INNERKITS_SAMGR_INCLUDE_STATUS_CHANGE_WRAPPER_H

#include <memory>

#include "cxx.h"
#include "if_system_ability_manager.h"
#include "ipc_object_stub.h"
#include "system_ability_status_change_stub.h"

namespace OHOS {
namespace SamgrRust {
struct SystemProcessInfo;
class SystemAbilityStatusChangeWrapper : public SystemAbilityStatusChangeStub {
public:
    SystemAbilityStatusChangeWrapper(const rust::Fn<void(int32_t systemAbilityId, const rust::str deviceId)> onAdd,
        const rust::Fn<void(int32_t systemAbilityId, const rust::str deviceId)> onRemove);
    ~SystemAbilityStatusChangeWrapper() = default;
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
    
private:
    rust::Fn<void(int32_t systemAbilityId, const rust::str deviceId)> onAdd_;
    rust::Fn<void(int32_t systemAbilityId, const rust::str deviceId)> onRemove_;
};

class SystemProcessStatusChangeWrapper : public IRemoteProxy<ISystemProcessStatusChange> {
public:
    SystemProcessStatusChangeWrapper(const sptr<IRemoteObject> &impl,
        const rust::Fn<void(const OHOS::SamgrRust::SystemProcessInfo &systemProcessInfo)> onStart,
        const rust::Fn<void(const OHOS::SamgrRust::SystemProcessInfo &systemProcessInfo)> onStop);
    ~SystemProcessStatusChangeWrapper() = default;

    void OnSystemProcessStarted(OHOS::SystemProcessInfo &systemProcessInfo) override;
    void OnSystemProcessStopped(OHOS::SystemProcessInfo &systemProcessInfo) override;

private:
    rust::Fn<void(const OHOS::SamgrRust::SystemProcessInfo &systemProcessInfo)> onStart_;
    rust::Fn<void(const OHOS::SamgrRust::SystemProcessInfo &systemProcessInfo)> onStop_;
};

class UnSubscribeSystemAbilityHandler {
public:
    UnSubscribeSystemAbilityHandler(int32_t systemAbilityId, sptr<ISystemAbilityStatusChange> listener);
    void UnSubscribe();

private:
    int32_t said_;
    sptr<ISystemAbilityStatusChange> listener_;
};

class UnSubscribeSystemProcessHandler {
public:
    UnSubscribeSystemProcessHandler(sptr<ISystemProcessStatusChange> listener);
    void UnSubscribe();

private:
    sptr<ISystemProcessStatusChange> listener_;
};

} // namespace SamgrRust
} // namespace OHOS

#endif