// Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "system_ability_manager_wrapper.h"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "cxx.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "isystem_ability_status_change.h"
#include "isystem_process_status_change.h"
#include "refbase.h"
#include "status_change_wrapper.h"
#include "string_ex.h"
#include "wrapper.rs.h"

namespace OHOS {
namespace SamgrRust {

static constexpr size_t MAX_RUST_STR_LEN = 1024;

rust::Vec<rust::String> ListSystemAbilities()
{
    auto sysm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto res = rust::Vec<rust::String>();

    if (sysm == nullptr) {
        return res;
    }

    auto list = sysm->ListSystemAbilities();
    for (auto s : list) {
        res.push_back(s.data());
    }
    return res;
}

rust::Vec<rust::String> ListSystemAbilitiesWithDumpFlag(unsigned int dumpFlags)
{
    auto sysm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto res = rust::Vec<rust::String>();

    if (sysm == nullptr) {
        return res;
    }

    auto list = sysm->ListSystemAbilities(dumpFlags);
    for (auto s : list) {
        char16_t *c = s.data();
        res.push_back(c);
    }
    return res;
}

std::unique_ptr<SptrIRemoteObject> LoadSystemAbility(int32_t systemAbilityId, int32_t timeout)
{
    auto sysm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sysm == nullptr) {
        return nullptr;
    }

    auto ability = sysm->LoadSystemAbility(systemAbilityId, timeout);
    if (ability == nullptr) {
        return nullptr;
    }
    return std::make_unique<SptrIRemoteObject>(std::move(ability));
}

int32_t LoadSystemAbilityWithCallback(int32_t systemAbilityId, rust::Fn<void()> on_success, rust::Fn<void()> on_fail)
{
    auto sysm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sysm == nullptr) {
        return -1;
    }
    auto callback = sptr<LoadCallbackWrapper>::MakeSptr(on_success, on_fail);
    return sysm->LoadSystemAbility(systemAbilityId, callback);
}

std::unique_ptr<SptrIRemoteObject> GetContextManager()
{
    sptr<IRemoteObject> saMgr = IPCSkeleton::GetContextObject();
    if (saMgr == nullptr) {
        return nullptr;
    }
    return std::make_unique<SptrIRemoteObject>(std::move(saMgr));
}

std::unique_ptr<SptrIRemoteObject> GetSystemAbility(int32_t systemAbilityId)
{
    auto sysm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sysm == nullptr) {
        return nullptr;
    }

    auto ability = sysm->GetSystemAbility(systemAbilityId);
    if (ability == nullptr) {
        return nullptr;
    }
    return std::make_unique<SptrIRemoteObject>(std::move(ability));
}

std::unique_ptr<SptrIRemoteObject> CheckSystemAbility(int32_t systemAbilityId)
{
    auto sysm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sysm == nullptr) {
        return nullptr;
    }

    auto ability = sysm->CheckSystemAbility(systemAbilityId);
    if (ability == nullptr) {
        return nullptr;
    }
    return std::make_unique<SptrIRemoteObject>(std::move(ability));
}

int32_t RemoveSystemAbility(int32_t systemAbilityId)
{
    auto sysm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sysm == nullptr) {
        return -1;
    }
    return sysm->RemoveSystemAbility(systemAbilityId);
}

std::unique_ptr<SptrIRemoteObject> GetSystemAbilityWithDeviceId(int32_t systemAbilityId, const std::string &deviceId)
{
    auto sysm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sysm == nullptr) {
        return nullptr;
    }
    auto ability = sysm->GetSystemAbility(systemAbilityId, deviceId);
    if (ability == nullptr) {
        return nullptr;
    }
    return std::make_unique<SptrIRemoteObject>(std::move(ability));
}

std::unique_ptr<SptrIRemoteObject> CheckSystemAbilityWithDeviceId(int32_t systemAbilityId, const std::string &deviceId)
{
    auto sysm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sysm == nullptr) {
        return nullptr;
    }
    auto ability = sysm->CheckSystemAbility(systemAbilityId, deviceId);
    if (ability == nullptr) {
        return nullptr;
    }
    return std::make_unique<SptrIRemoteObject>(std::move(ability));
}

std::unique_ptr<UnSubscribeSystemAbilityHandler> SubscribeSystemAbility(int32_t systemAbilityId,
    rust::Fn<void(int32_t systemAbilityId, const rust::str deviceId)> onAdd,
    rust::Fn<void(int32_t systemAbilityId, const rust::str deviceId)> onRemove)
{
    auto sysm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sysm == nullptr) {
        return nullptr;
    }

    sptr<ISystemAbilityStatusChange> listener = new SystemAbilityStatusChangeWrapper(onAdd, onRemove);
    sysm->SubscribeSystemAbility(systemAbilityId, listener);
    return std::make_unique<UnSubscribeSystemAbilityHandler>(systemAbilityId, listener);
}

int32_t AddOnDemandSystemAbilityInfo(int32_t systemAbilityId, const rust::str localAbilityManagerName)
{
    if (localAbilityManagerName.length() > MAX_RUST_STR_LEN) {
        return -1;
    }

    auto sysm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sysm == nullptr) {
        return -1;
    }
    std::u16string s = Str8ToStr16(std::string(localAbilityManagerName));
    return sysm->AddOnDemandSystemAbilityInfo(systemAbilityId, s);
}

int32_t UnloadSystemAbility(int32_t systemAbilityId)
{
    auto sysm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sysm == nullptr) {
        return -1;
    }
    return sysm->UnloadSystemAbility(systemAbilityId);
}

int32_t CancelUnloadSystemAbility(int32_t systemAbilityId)
{
    auto sysm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sysm == nullptr) {
        return -1;
    }
    return sysm->CancelUnloadSystemAbility(systemAbilityId);
}

int32_t UnloadAllIdleSystemAbility()
{
    auto sysm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sysm == nullptr) {
        return -1;
    }
    return sysm->UnloadAllIdleSystemAbility();
}

int32_t AddSystemAbility(int32_t systemAbilityId, rust::Box<AbilityStub> ability, AddSystemAbilityConfig config)
{
    auto sysm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sysm == nullptr) {
        return -1;
    }
    auto capability_u16 = Str8ToStr16(std::string(config.capability));
    auto permission_u16 = Str8ToStr16(std::string(config.permission));

    ISystemAbilityManager::SAExtraProp extra(config.is_distributed, config.dump_flags, capability_u16, permission_u16);
    auto stub = sptr<RemoteServiceStub>::MakeSptr(ability.into_raw());

    return sysm->AddSystemAbility(systemAbilityId, stub);
}

SystemProcessInfo GetSystemProcessInfo(int32_t systemAbilityId)
{
    auto sysm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    OHOS::SystemProcessInfo info;
    if (sysm == nullptr) {
        return SystemProcessInfo{
            .processName = info.processName.data(),
            .pid = info.pid,
            .uid = info.uid,
        };
    }
    sysm->GetSystemProcessInfo(systemAbilityId, info);
    return SystemProcessInfo{
        .processName = info.processName.data(),
        .pid = info.pid,
        .uid = info.uid,
    };
}

rust::Vec<SystemProcessInfo> GetRunningSystemProcess()
{
    auto sysm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();

    auto res = rust::Vec<SystemProcessInfo>();
    if (sysm == nullptr) {
        return res;
    }

    auto infos = std::list<OHOS::SystemProcessInfo>();
    sysm->GetRunningSystemProcess(infos);
    for (auto info : infos) {
        res.push_back(SystemProcessInfo{
            .processName = info.processName,
            .pid = info.pid,
            .uid = info.uid,
        });
    }
    return res;
}

int32_t GetCommonEventExtraDataIdlist(int32_t saId, rust::Vec<int64_t> &extraDataIdList, const std::string &eventName)
{
    auto sysm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sysm == nullptr) {
        return -1;
    }

    std::vector<int64_t> idList;
    auto ret = sysm->GetCommonEventExtraDataIdlist(saId, idList, eventName);
    if (ret != ERR_OK) {
        return ret;
    }
    for (auto id : idList) {
        extraDataIdList.push_back(id);
    }
    return ret;
}

std::unique_ptr<UnSubscribeSystemProcessHandler> SubscribeSystemProcess(
    rust::Fn<void(const OHOS::SamgrRust::SystemProcessInfo &systemProcessInfo)> onStart_,
    rust::Fn<void(const OHOS::SamgrRust::SystemProcessInfo &systemProcessInfo)> onStop_)
{
    auto sysm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sysm == nullptr) {
        return nullptr;
    }
    sptr<ISystemProcessStatusChange> listener = new SystemProcessStatusChangeWrapper(nullptr, onStart_, onStop_);
    sysm->SubscribeSystemProcess(listener);
    return std::make_unique<UnSubscribeSystemProcessHandler>(listener);
}

int32_t GetOnDemandReasonExtraData(int64_t extraDataId, MessageParcel &parcel)
{
    auto sysm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sysm == nullptr) {
        return -1;
    }
    return sysm->GetOnDemandReasonExtraData(extraDataId, parcel);
}

int32_t SendStrategy(int32_t type, rust::Vec<int32_t> systemAbilityIds, int32_t level, std::string &action)
{
    auto sysm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sysm == nullptr) {
        return -1;
    }
    auto v = std::vector<int32_t>();
    for (auto id : systemAbilityIds) {
        v.push_back(id);
    }
    return sysm->SendStrategy(type, v, level, action);
}

RemoteServiceStub::RemoteServiceStub(AbilityStub *ability)
{
    this->inner_ = ability;
}

RemoteServiceStub::~RemoteServiceStub()
{
    auto ability = rust::Box<AbilityStub>::from_raw(this->inner_);
}

int RemoteServiceStub ::OnRemoteRequest(
    uint32_t code, OHOS::MessageParcel &data, OHOS::MessageParcel &reply, OHOS::MessageOption &option)
{
    return inner_->on_remote_request(code, data, reply);
}

int RemoteServiceStub ::OnRemoteDump(
    uint32_t code, OHOS::MessageParcel &data, OHOS::MessageParcel &reply, OHOS::MessageOption &option)
{
    return 0;
}

LoadCallbackWrapper::LoadCallbackWrapper(rust::Fn<void()> on_success, rust::Fn<void()> on_fail)
    : on_success_(on_success), on_fail_(on_fail)
{
}

void LoadCallbackWrapper::OnLoadSystemAbilitySuccess(int32_t systemAbilityId, const sptr<IRemoteObject> &remoteObject)
{
    on_success_();
}

void LoadCallbackWrapper::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    on_fail_();
}

} // namespace SamgrRust
} // namespace OHOS