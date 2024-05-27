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

#ifndef INTERFACES_INNERKITS_SAMGR_INCLUDE_SYSTEM_ABILITY_MANAGER_WRAPPER_H
#define INTERFACES_INNERKITS_SAMGR_INCLUDE_SYSTEM_ABILITY_MANAGER_WRAPPER_H

#include <memory>

#include "cxx.h"
#include "if_system_ability_manager.h"
#include "isystem_process_status_change.h"
#include "status_change_wrapper.h"
#include "system_ability_load_callback_stub.h"

namespace OHOS {
typedef sptr<IRemoteObject> SptrIRemoteObject;
namespace SamgrRust {
struct SAExtraPropWrapper;
struct AbilityStub;
struct SystemProcessInfo;
struct AddSystemAbilityConfig;

std::unique_ptr<SptrIRemoteObject> GetSystemAbility(int32_t systemAbilityId);
std::unique_ptr<SptrIRemoteObject> CheckSystemAbility(int32_t systemAbilityId);
std::unique_ptr<SptrIRemoteObject> LoadSystemAbility(int32_t systemAbilityId, int32_t timeout);

std::unique_ptr<SptrIRemoteObject> LoadSystemAbility(int32_t systemAbilityId, int32_t timeout);
int32_t LoadSystemAbilityWithCallback(int32_t systemAbilityId, rust::Fn<void()> on_success, rust::Fn<void()> on_fail);

std::unique_ptr<SptrIRemoteObject> GetContextManager();

int32_t RemoveSystemAbility(int32_t systemAbilityId);

std::unique_ptr<SptrIRemoteObject> GetSystemAbilityWithDeviceId(int32_t systemAbilityId, const std::string &deviceId);
std::unique_ptr<SptrIRemoteObject> CheckSystemAbilityWithDeviceId(int32_t systemAbilityId, const std::string &deviceId);
rust::Vec<rust::String> ListSystemAbilities();
rust::Vec<rust::String> ListSystemAbilitiesWithDumpFlag(unsigned int dumpFlags);
std::unique_ptr<UnSubscribeSystemAbilityHandler> SubscribeSystemAbility(int32_t systemAbilityId,
    rust::Fn<void(int32_t systemAbilityId, const rust::str deviceId)> onAdd,
    rust::Fn<void(int32_t systemAbilityId, const rust::str deviceId)> onRemove);
int32_t AddOnDemandSystemAbilityInfo(int32_t systemAbilityId, const rust::str localAbilityManagerName);
int32_t UnloadSystemAbility(int32_t systemAbilityId);
int32_t CancelUnloadSystemAbility(int32_t systemAbilityId);
int32_t UnloadAllIdleSystemAbility();
int32_t GetCommonEventExtraDataIdlist(int32_t saId, rust::Vec<int64_t>& extraDataIdList, const std::string& eventName);
SystemProcessInfo GetSystemProcessInfo(int32_t systemAbilityId);
rust::Vec<SystemProcessInfo> GetRunningSystemProcess();
int32_t SendStrategy(int32_t type, rust::Vec<int32_t> systemAbilityIds, int32_t level, std::string &action);
int32_t AddSystemAbility(int32_t systemAbilityId, rust::Box<AbilityStub> ability, AddSystemAbilityConfig config);
std::unique_ptr<UnSubscribeSystemProcessHandler> SubscribeSystemProcess(
    rust::Fn<void(const OHOS::SamgrRust::SystemProcessInfo &systemProcessInfo)> onStart_,
    rust::Fn<void(const OHOS::SamgrRust::SystemProcessInfo &systemProcessInfo)> onStop_);

int32_t GetOnDemandReasonExtraData(int64_t extraDataId, MessageParcel &parcel);

struct RemoteServiceStub : public OHOS::IPCObjectStub {
public:
    explicit RemoteServiceStub(AbilityStub *stub);
    ~RemoteServiceStub();

    int OnRemoteRequest(
        uint32_t code, OHOS::MessageParcel &data, OHOS::MessageParcel &reply, OHOS::MessageOption &option) override;

    int OnRemoteDump(
        uint32_t code, OHOS::MessageParcel &data, OHOS::MessageParcel &reply, OHOS::MessageOption &option) override;

private:
    AbilityStub *inner_;
};

class LoadCallbackWrapper : public SystemAbilityLoadCallbackStub {
public:
    LoadCallbackWrapper(rust::Fn<void()> on_success, rust::Fn<void()> on_fail);
    void OnLoadSystemAbilitySuccess(int32_t systemAbilityId, const sptr<IRemoteObject> &remoteObject) override;
    void OnLoadSystemAbilityFail(int32_t systemAbilityId) override;

private:
    rust::Fn<void()> on_success_;
    rust::Fn<void()> on_fail_;
};

} // namespace SamgrRust
} // namespace OHOS
#endif