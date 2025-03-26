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

#ifndef SERVICES_SAMGR_NATIVE_INCLUDE_SYSTEM_ABILITY_MANAGER_STUB_H
#define SERVICES_SAMGR_NATIVE_INCLUDE_SYSTEM_ABILITY_MANAGER_STUB_H

#include <map>
#include "if_system_ability_manager.h"
#include "iremote_object.h"
#include "iremote_stub.h"

namespace OHOS {
class SystemAbilityManagerStub : public IRemoteStub<ISystemAbilityManager> {
public:
    SystemAbilityManagerStub();
    ~SystemAbilityManagerStub() = default;
    int32_t OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption &option) override;

protected:
    static bool CanRequest();
    static bool EnforceInterceToken(MessageParcel& data);
    static bool CheckPermission(const std::string& permission);

private:
    static int32_t LocalListSystemAbility(SystemAbilityManagerStub* stub,
        MessageParcel& data, MessageParcel& reply)
    {
        return stub->ListSystemAbilityInner(data, reply);
    }
    static int32_t LocalSubsSystemAbility(SystemAbilityManagerStub* stub,
        MessageParcel& data, MessageParcel& reply)
    {
        return stub->SubsSystemAbilityInner(data, reply);
    }
    static int32_t LocalUnSubsSystemAbility(SystemAbilityManagerStub* stub,
        MessageParcel& data, MessageParcel& reply)
    {
        return stub->UnSubsSystemAbilityInner(data, reply);
    }
    static int32_t LocalCheckRemtSystemAbility(SystemAbilityManagerStub* stub,
        MessageParcel& data, MessageParcel& reply)
    {
        return stub->CheckRemtSystemAbilityInner(data, reply);
    }
    static int32_t LocalAddOndemandSystemAbility(SystemAbilityManagerStub* stub,
        MessageParcel& data, MessageParcel& reply)
    {
        return stub->AddOndemandSystemAbilityInner(data, reply);
    }
    static int32_t LocalCheckSystemAbilityImme(SystemAbilityManagerStub* stub,
        MessageParcel& data, MessageParcel& reply)
    {
        return stub->CheckSystemAbilityImmeInner(data, reply);
    }
    static int32_t LocalAddSystemAbility(SystemAbilityManagerStub* stub,
        MessageParcel& data, MessageParcel& reply)
    {
        return stub->AddSystemAbilityInner(data, reply);
    }
    static int32_t LocalGetSystemAbility(SystemAbilityManagerStub* stub,
        MessageParcel& data, MessageParcel& reply)
    {
        return stub->GetSystemAbilityInner(data, reply);
    }
    static int32_t LocalCheckSystemAbility(SystemAbilityManagerStub* stub,
        MessageParcel& data, MessageParcel& reply)
    {
        return stub->CheckSystemAbilityInner(data, reply);
    }
    static int32_t LocalAddSystemProcess(SystemAbilityManagerStub* stub,
        MessageParcel& data, MessageParcel& reply)
    {
        return stub->AddSystemProcessInner(data, reply);
    }
    static int32_t LocalRemoveSystemAbility(SystemAbilityManagerStub* stub,
        MessageParcel& data, MessageParcel& reply)
    {
        return stub->RemoveSystemAbilityInner(data, reply);
    }
    static int32_t LocalGetSystemProcessInfo(SystemAbilityManagerStub* stub,
        MessageParcel& data, MessageParcel& reply)
    {
        return stub->GetSystemProcessInfoInner(data, reply);
    }
    static int32_t LocalGetRunningSystemProcess(SystemAbilityManagerStub* stub,
        MessageParcel& data, MessageParcel& reply)
    {
        return stub->GetRunningSystemProcessInner(data, reply);
    }
    static int32_t LocalSubscribeSystemProcess(SystemAbilityManagerStub* stub,
        MessageParcel& data, MessageParcel& reply)
    {
        return stub->SubscribeSystemProcessInner(data, reply);
    }
    static int32_t LocalUnSubscribeSystemProcess(SystemAbilityManagerStub* stub,
        MessageParcel& data, MessageParcel& reply)
    {
        return stub->UnSubscribeSystemProcessInner(data, reply);
    }
    static int32_t LocalLoadSystemAbility(SystemAbilityManagerStub* stub,
        MessageParcel& data, MessageParcel& reply)
    {
        return stub->LoadSystemAbilityInner(data, reply);
    }
    static int32_t LocalLoadRemoteSystemAbility(SystemAbilityManagerStub* stub,
        MessageParcel& data, MessageParcel& reply)
    {
        return stub->LoadRemoteSystemAbilityInner(data, reply);
    }
    static int32_t LocalUnloadSystemAbility(SystemAbilityManagerStub* stub,
        MessageParcel& data, MessageParcel& reply)
    {
        return stub->UnloadSystemAbilityInner(data, reply);
    }
    static int32_t LocalCancelUnloadSystemAbility(SystemAbilityManagerStub* stub,
        MessageParcel& data, MessageParcel& reply)
    {
        return stub->CancelUnloadSystemAbilityInner(data, reply);
    }
    static int32_t LocalUnloadAllIdleSystemAbility(SystemAbilityManagerStub* stub,
        MessageParcel& data, MessageParcel& reply)
    {
        return stub->UnloadAllIdleSystemAbilityInner(data, reply);
    }
    static int32_t LocalGetOnDemandReasonExtraData(SystemAbilityManagerStub* stub,
        MessageParcel& data, MessageParcel& reply)
    {
        return stub->GetOnDemandReasonExtraDataInner(data, reply);
    }
    static int32_t LocalGetOnDemandPolicy(SystemAbilityManagerStub* stub,
        MessageParcel& data, MessageParcel& reply)
    {
        return stub->GetOnDemandPolicyInner(data, reply);
    }
    static int32_t LocalUpdateOnDemandPolicy(SystemAbilityManagerStub* stub,
        MessageParcel& data, MessageParcel& reply)
    {
        return stub->UpdateOnDemandPolicyInner(data, reply);
    }
    static int32_t LocalGetOnDemandSystemAbilityIds(SystemAbilityManagerStub* stub,
        MessageParcel& data, MessageParcel& reply)
    {
        return stub->GetOnDemandSystemAbilityIdsInner(data, reply);
    }
    static int32_t LocalSendStrategy(SystemAbilityManagerStub* stub,
        MessageParcel& data, MessageParcel& reply)
    {
        return stub->SendStrategyInner(data, reply);
    }
    static int32_t LocalGetExtensionSaIds(SystemAbilityManagerStub* stub,
        MessageParcel& data, MessageParcel& reply)
    {
        return stub->GetExtensionSaIdsInner(data, reply);
    }
    static int32_t LocalGetExtensionRunningSaList(SystemAbilityManagerStub* stub,
        MessageParcel& data, MessageParcel& reply)
    {
        return stub->GetExtensionRunningSaListInner(data, reply);
    }
    static int32_t LocalGetRunningSaExtensionInfoList(SystemAbilityManagerStub* stub,
        MessageParcel& data, MessageParcel& reply)
    {
        return stub->GetRunningSaExtensionInfoListInner(data, reply);
    }
    static int32_t LocalGetCommonEventExtraDataIdlist(SystemAbilityManagerStub* stub,
        MessageParcel& data, MessageParcel& reply)
    {
        return stub->GetCommonEventExtraDataIdlistInner(data, reply);
    }
    int32_t ListSystemAbilityInner(MessageParcel& data, MessageParcel& reply);
    int32_t SubsSystemAbilityInner(MessageParcel& data, MessageParcel& reply);
    int32_t UnSubsSystemAbilityInner(MessageParcel& data, MessageParcel& reply);
    int32_t CheckRemtSystemAbilityInner(MessageParcel& data, MessageParcel& reply);
    int32_t AddOndemandSystemAbilityInner(MessageParcel& data, MessageParcel& reply);
    int32_t CheckSystemAbilityImmeInner(MessageParcel& data, MessageParcel& reply);
    int32_t AddSystemAbilityInner(MessageParcel& data, MessageParcel& reply);
    int32_t GetSystemAbilityInner(MessageParcel& data, MessageParcel& reply);
    int32_t CheckSystemAbilityInner(MessageParcel& data, MessageParcel& reply);
    int32_t AddSystemProcessInner(MessageParcel& data, MessageParcel& reply);
    int32_t RemoveSystemAbilityInner(MessageParcel& data, MessageParcel& reply);
    int32_t GetSystemProcessInfoInner(MessageParcel& data, MessageParcel& reply);
    int32_t GetRunningSystemProcessInner(MessageParcel& data, MessageParcel& reply);
    int32_t SubscribeSystemProcessInner(MessageParcel& data, MessageParcel& reply);
    int32_t UnSubscribeSystemProcessInner(MessageParcel& data, MessageParcel& reply);
    int32_t LoadSystemAbilityInner(MessageParcel& data, MessageParcel& reply);
    int32_t LoadRemoteSystemAbilityInner(MessageParcel& data, MessageParcel& reply);
    int32_t UnloadSystemAbilityInner(MessageParcel& data, MessageParcel& reply);
    int32_t CancelUnloadSystemAbilityInner(MessageParcel& data, MessageParcel& reply);
    int32_t UnloadAllIdleSystemAbilityInner(MessageParcel& data, MessageParcel& reply);
    int32_t GetOnDemandReasonExtraDataInner(MessageParcel& data, MessageParcel& reply);
    int32_t GetOnDemandPolicyInner(MessageParcel& data, MessageParcel& reply);
    int32_t UpdateOnDemandPolicyInner(MessageParcel& data, MessageParcel& reply);
    int32_t UnmarshalingSaExtraProp(MessageParcel& data, SAExtraProp& extraProp);
    int32_t GetOnDemandSystemAbilityIdsInner(MessageParcel& data, MessageParcel& reply);
    int32_t SendStrategyInner(MessageParcel& data, MessageParcel& reply);
    int32_t GetExtensionSaIdsInner(MessageParcel& data, MessageParcel& reply);
    int32_t GetExtensionRunningSaListInner(MessageParcel& data, MessageParcel& reply);
    int32_t GetRunningSaExtensionInfoListInner(MessageParcel& data, MessageParcel& reply);
    int32_t GetCommonEventExtraDataIdlistInner(MessageParcel& data, MessageParcel& reply);
    static int32_t GetHapIdMultiuser(int32_t uid);
    void SetAbilityFuncMap();
    void SetProcessFuncMap();

    using SystemAbilityManagerStubFunc =
        int32_t (*)(SystemAbilityManagerStub* stub, MessageParcel& data, MessageParcel& reply);
    std::map<uint32_t, SystemAbilityManagerStubFunc> memberFuncMap_;
};
} // namespace OHOS

#endif // !defined(SERVICES_SAMGR_NATIVE_INCLUDE_SYSTEM_ABILITY_MANAGER_STUB_H)
