/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "samgrparallelize_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>
#include "sam_mock_permission.h"
#include "system_ability_manager.h"
#include "sa_status_change_mock.h"
#include "system_process_status_change_stub.h"

#include <cinttypes>
#include <cstddef>
#include <cstdint>
#include <unistd.h>
#include <cstdlib>
#include <fcntl.h>

using namespace OHOS;
namespace OHOS {
constexpr int32_t MAX_STRING_LENGTH = 64;
constexpr int32_t MAX_SYS_ABILITY_ID = 0x00ffffff;
sptr<SystemAbilityManager> g_saMgrService = nullptr;

class SystemProcessStatusChange : public SystemProcessStatusChangeStub {
public:
    void OnSystemProcessStarted(SystemProcessInfo& systemProcessInfo) override;
    void OnSystemProcessStopped(SystemProcessInfo& systemProcessInfo) override;
};
void SystemProcessStatusChange::OnSystemProcessStarted(SystemProcessInfo& systemProcessInfo)
{
}

void SystemProcessStatusChange::OnSystemProcessStopped(SystemProcessInfo& systemProcessInfo)
{
}

enum IpcCode : std::int16_t {
    GET_SYSTEM_ABILITY = 1,
    CHECK_SYSTEM_ABILITY = 2,
    ADD_SYSTEM_ABILITY = 3,
    REMOVE_SYSTEM_ABILITY = 4,
    LIST_SYSTEM_ABILITY = 5,
    SUBSCRIBE_SYSTEM_ABILITY = 6,
    LOAD_SYSTEM_ABILITY = 7,
    LOAD_REMOTE_SYSTEM_ABILITY = 8,
    CHECK_REMOTE_SYSTEM_ABILITY = 9,
    ADD_ONDEMAND_SYSTEM_ABILITY = 10,
    CHECK_SYSTEM_ABILITY_IMMEDIATELY = 12,
    UNSUBSCRIBE_SYSTEM_ABILITY = 18,
    ADD_SYSTEM_PROCESS = 20,
    UNLOAD_SYSTEM_ABILITY = 21,
    CANCEL_UNLOAD_SYSTEM_ABILITY = 22,
    GET_RUNNING_SYSTEM_PROCESS = 23,
    SUBSCRIBE_SYSTEM_PROCESS = 24,
    UNSUBSCRIBE_SYSTEM_PROCESS = 25,
    GET_ONDEMAND_REASON_EXTRA_DATA = 26,
    GET_ONDEAMND_POLICY = 27,
    UPDATE_ONDEAMND_POLICY = 28,
    GET_SYSTEM_PROCESS_INFO = 29,
    GET_ONDEMAND_SYSTEM_ABILITY_IDS = 30,
    SEND_STRATEGY_TRANASACTION = 31,
    UNLOAD_ALL_IDLE_SYSTEM_ABILITY = 32,
    GET_EXTENSION_SA_IDS = 33,
    GET_EXTERNSION_SA_LIST = 34,
    GET_SA_EXTENSION_INFO = 35,
    GET_COMMON_EVENT_EXTRA_ID_LIST = 36,
    GET_LOCAL_ABILITY_MANAGER_PROXY = 37,
    UNLOAD_IDLE_PROCESS_BYLIST = 38,
    GET_LRU_IDLE_SYSTEM_ABILITY_PROCESS = 39,
};
}

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
    SamMockPermission::MockPermission();
    g_saMgrService = SystemAbilityManager::GetInstance();
    g_saMgrService->Init();
    return 0;
}

extern "C" int FuzzSaMgrService(FuzzedDataProvider &provider)
{
    if (g_saMgrService == nullptr) {
        return 0;
    }

    static const int fuzzCodes[] = {
        IpcCode::GET_SYSTEM_ABILITY, IpcCode::CHECK_SYSTEM_ABILITY, IpcCode::ADD_SYSTEM_ABILITY,
        IpcCode::REMOVE_SYSTEM_ABILITY, IpcCode::LIST_SYSTEM_ABILITY, IpcCode::SUBSCRIBE_SYSTEM_ABILITY,
        IpcCode::LOAD_SYSTEM_ABILITY, IpcCode::LOAD_REMOTE_SYSTEM_ABILITY, IpcCode::CHECK_REMOTE_SYSTEM_ABILITY,
        IpcCode::ADD_ONDEMAND_SYSTEM_ABILITY, IpcCode::CHECK_SYSTEM_ABILITY_IMMEDIATELY,
        IpcCode::UNSUBSCRIBE_SYSTEM_ABILITY, IpcCode::ADD_SYSTEM_PROCESS, IpcCode::UNLOAD_SYSTEM_ABILITY,
        IpcCode::CANCEL_UNLOAD_SYSTEM_ABILITY, IpcCode::GET_RUNNING_SYSTEM_PROCESS,
        IpcCode::SUBSCRIBE_SYSTEM_PROCESS, IpcCode::UNSUBSCRIBE_SYSTEM_PROCESS,
        IpcCode::GET_ONDEMAND_REASON_EXTRA_DATA, IpcCode::GET_ONDEAMND_POLICY,
        IpcCode::UPDATE_ONDEAMND_POLICY, IpcCode::GET_SYSTEM_PROCESS_INFO,
        IpcCode::GET_ONDEMAND_SYSTEM_ABILITY_IDS, IpcCode::SEND_STRATEGY_TRANASACTION,
        IpcCode::UNLOAD_ALL_IDLE_SYSTEM_ABILITY, IpcCode::GET_EXTENSION_SA_IDS,
        IpcCode::GET_EXTERNSION_SA_LIST, IpcCode::GET_SA_EXTENSION_INFO,
        IpcCode::GET_COMMON_EVENT_EXTRA_ID_LIST, IpcCode::GET_LOCAL_ABILITY_MANAGER_PROXY,
        IpcCode::UNLOAD_IDLE_PROCESS_BYLIST, IpcCode::GET_LRU_IDLE_SYSTEM_ABILITY_PROCESS,
    };
    int code = provider.PickValueInArray(fuzzCodes);
    switch (code) {
        case IpcCode::GET_SYSTEM_ABILITY: {
            int32_t said = provider.ConsumeIntegralInRange<int32_t>(0, MAX_SYS_ABILITY_ID);
            g_saMgrService->GetSystemAbility(said);
            break;
        }
        case IpcCode::CHECK_SYSTEM_ABILITY: {
            int32_t said = provider.ConsumeIntegralInRange<int32_t>(0, MAX_SYS_ABILITY_ID);
            g_saMgrService->CheckSystemAbility(said);
            break;
        }
        case IpcCode::ADD_SYSTEM_ABILITY: {
            int32_t said = provider.ConsumeIntegralInRange<int32_t>(0, MAX_SYS_ABILITY_ID);
            sptr<IRemoteObject> testAbility(new SaStatusChangeMock());
            ISystemAbilityManager::SAExtraProp extraProp;
            extraProp.isDistributed = provider.ConsumeBool();
            extraProp.dumpFlags = provider.ConsumeIntegral<uint32_t>();
            extraProp.capability = Str8ToStr16(provider.ConsumeRandomLengthString(MAX_STRING_LENGTH));
            extraProp.permission = Str8ToStr16(provider.ConsumeRandomLengthString(MAX_STRING_LENGTH));
            g_saMgrService->AddSystemAbility(said, testAbility, extraProp);
            break;
        }
        case IpcCode::REMOVE_SYSTEM_ABILITY: {
            int32_t said = provider.ConsumeIntegralInRange<int32_t>(0, MAX_SYS_ABILITY_ID);
            g_saMgrService->RemoveSystemAbility(said);
            break;
        }
        case IpcCode::LIST_SYSTEM_ABILITY: {
            int32_t dumpFlag = provider.ConsumeIntegral<int32_t>();
            g_saMgrService->ListSystemAbilities(dumpFlag);
            break;
        }
        case IpcCode::SUBSCRIBE_SYSTEM_ABILITY: {
            int32_t said = provider.ConsumeIntegralInRange<int32_t>(0, MAX_SYS_ABILITY_ID);
            sptr<SaStatusChangeMock> listener(new SaStatusChangeMock());
            g_saMgrService->SubscribeSystemAbility(said, listener);
            break;
        }
        case IpcCode::LOAD_SYSTEM_ABILITY: {
            int32_t said = provider.ConsumeIntegralInRange<int32_t>(0, MAX_SYS_ABILITY_ID);
            sptr<ISystemAbilityLoadCallback> callback = new SystemAbilityLoadCallbackMock();
            g_saMgrService->LoadSystemAbility(said, callback);
            break;
        }
        case IpcCode::LOAD_REMOTE_SYSTEM_ABILITY: {
            int32_t said = provider.ConsumeIntegralInRange<int32_t>(0, MAX_SYS_ABILITY_ID);
            std::string deviceId = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
            sptr<ISystemAbilityLoadCallback> callback = new SystemAbilityLoadCallbackMock();
            g_saMgrService->LoadSystemAbility(said, deviceId, callback);
            break;
        }
        case IpcCode::CHECK_REMOTE_SYSTEM_ABILITY: {
            int32_t said = provider.ConsumeIntegralInRange<int32_t>(0, MAX_SYS_ABILITY_ID);
            std::string uuid = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
            g_saMgrService->GetSystemAbility(said, uuid);
            break;
        }
        case IpcCode::ADD_ONDEMAND_SYSTEM_ABILITY: {
            int32_t said = provider.ConsumeIntegralInRange<int32_t>(0, MAX_SYS_ABILITY_ID);
            std::u16string localManagerName = Str8ToStr16(provider.ConsumeRandomLengthString(MAX_STRING_LENGTH));
            g_saMgrService->AddOnDemandSystemAbilityInfo(said, localManagerName);
            break;
        }
        case IpcCode::CHECK_SYSTEM_ABILITY_IMMEDIATELY: {
            int32_t said = provider.ConsumeIntegralInRange<int32_t>(0, MAX_SYS_ABILITY_ID);
            bool isExist = provider.ConsumeBool();
            g_saMgrService->CheckSystemAbility(said, isExist);
            break;
        }
        case IpcCode::UNSUBSCRIBE_SYSTEM_ABILITY: {
            int32_t said = provider.ConsumeIntegralInRange<int32_t>(0, MAX_SYS_ABILITY_ID);
            sptr<SaStatusChangeMock> listener(new SaStatusChangeMock());
            g_saMgrService->UnSubscribeSystemAbility(said, listener);
            break;
        }
        case IpcCode::ADD_SYSTEM_PROCESS: {
            std::u16string procName = Str8ToStr16(provider.ConsumeRandomLengthString(MAX_STRING_LENGTH));
            sptr<IRemoteObject> procObject(new SaStatusChangeMock());
            g_saMgrService->AddSystemProcess(procName, procObject);
            break;
        }
        case IpcCode::UNLOAD_SYSTEM_ABILITY: {
            int32_t said = provider.ConsumeIntegralInRange<int32_t>(0, MAX_SYS_ABILITY_ID);
            g_saMgrService->UnloadSystemAbility(said);
            break;
        }
        case IpcCode::CANCEL_UNLOAD_SYSTEM_ABILITY: {
            int32_t said = provider.ConsumeIntegralInRange<int32_t>(0, MAX_SYS_ABILITY_ID);
            g_saMgrService->CancelUnloadSystemAbility(said);
            break;
        }
        case IpcCode::GET_RUNNING_SYSTEM_PROCESS: {
            std::list<SystemProcessInfo> systemProcessInfos;
            g_saMgrService->GetRunningSystemProcess(systemProcessInfos);
            break;
        }
        case IpcCode::SUBSCRIBE_SYSTEM_PROCESS: {
            sptr<ISystemProcessStatusChange> listener = new SystemProcessStatusChange();
            g_saMgrService->SubscribeSystemProcess(listener);
            break;
        }
        case IpcCode::UNSUBSCRIBE_SYSTEM_PROCESS: {
            sptr<ISystemProcessStatusChange> listener = new SystemProcessStatusChange();
            g_saMgrService->UnSubscribeSystemProcess(listener);
            break;
        }
        case IpcCode::GET_ONDEMAND_REASON_EXTRA_DATA: {
            int64_t extraDataId = provider.ConsumeIntegral<int64_t>();
            MessageParcel extraDataParcel;
            g_saMgrService->GetOnDemandReasonExtraData(extraDataId, extraDataParcel);
            break;
        }
        case IpcCode::GET_ONDEAMND_POLICY: {
            int32_t said = provider.ConsumeIntegralInRange<int32_t>(0, MAX_SYS_ABILITY_ID);
            bool type = provider.ConsumeBool();
            OnDemandPolicyType typeEnum = type ? OnDemandPolicyType::START_POLICY : OnDemandPolicyType::STOP_POLICY;
            std::vector<SystemAbilityOnDemandEvent> abilityOnDemandEvents;
            g_saMgrService->GetOnDemandPolicy(said, typeEnum, abilityOnDemandEvents);
            break;
        }
        case IpcCode::UPDATE_ONDEAMND_POLICY: {
            int32_t said = provider.ConsumeIntegralInRange<int32_t>(0, MAX_SYS_ABILITY_ID);
            bool type = provider.ConsumeBool();
            OnDemandPolicyType typeEnum = type ? OnDemandPolicyType::START_POLICY : OnDemandPolicyType::STOP_POLICY;
            std::vector<SystemAbilityOnDemandEvent> abilityOnDemandEvents;
            g_saMgrService->UpdateOnDemandPolicy(said, typeEnum, abilityOnDemandEvents);
            break;
        }
        case IpcCode::GET_SYSTEM_PROCESS_INFO: {
            int32_t said = provider.ConsumeIntegralInRange<int32_t>(0, MAX_SYS_ABILITY_ID);
            SystemProcessInfo processInfo;
            g_saMgrService->GetSystemProcessInfo(said, processInfo);
            break;
        }
        case IpcCode::GET_ONDEMAND_SYSTEM_ABILITY_IDS: {
            std::vector<int32_t> systemAbilityIds;
            g_saMgrService->GetOnDemandSystemAbilityIds(systemAbilityIds);
            break;
        }
        case IpcCode::SEND_STRATEGY_TRANASACTION: {
            int32_t type = provider.ConsumeIntegral<int32_t>();
            int32_t said1 = provider.ConsumeIntegralInRange<int32_t>(0, MAX_SYS_ABILITY_ID);
            int32_t said2 = provider.ConsumeIntegralInRange<int32_t>(0, MAX_SYS_ABILITY_ID);
            std::vector<int32_t> systemAbilityIds;
            systemAbilityIds.emplace_back(said1);
            systemAbilityIds.emplace_back(said2);
            int32_t level = provider.ConsumeIntegral<int32_t>();
            std::string action = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
            g_saMgrService->SendStrategy(type, systemAbilityIds, level, action);
            break;
        }
        case IpcCode::UNLOAD_ALL_IDLE_SYSTEM_ABILITY: {
            g_saMgrService->UnloadAllIdleSystemAbility();
            break;
        }
        case IpcCode::GET_EXTENSION_SA_IDS: {
            std::string extension = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
            std::vector<int32_t> saIds;
            g_saMgrService->GetExtensionSaIds(extension, saIds);
            break;
        }
        case IpcCode::GET_EXTERNSION_SA_LIST: {
            std::string extension = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
            std::vector<sptr<IRemoteObject>> saList;
            g_saMgrService->GetExtensionRunningSaList(extension, saList);
            break;
        }
        case IpcCode::GET_SA_EXTENSION_INFO: {
            std::string extension = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
            std::vector<ISystemAbilityManager::SaExtensionInfo> infoList;
            g_saMgrService->GetRunningSaExtensionInfoList(extension, infoList);
            break;
        }
        case IpcCode::GET_COMMON_EVENT_EXTRA_ID_LIST: {
            int32_t said = provider.ConsumeIntegralInRange<int32_t>(0, MAX_SYS_ABILITY_ID);
            std::string eventName = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
            std::vector<int64_t> extraDataIdList;
            g_saMgrService->GetCommonEventExtraDataIdlist(said, extraDataIdList, eventName);
            break;
        }
        case IpcCode::GET_LOCAL_ABILITY_MANAGER_PROXY: {
            int32_t said = provider.ConsumeIntegralInRange<int32_t>(0, MAX_SYS_ABILITY_ID);
            g_saMgrService->GetLocalAbilityManagerProxy(said);
            break;
        }
        case IpcCode::UNLOAD_IDLE_PROCESS_BYLIST: {
            std::vector<std::u16string> processList;
            std::u16string processName1 = Str8ToStr16(provider.ConsumeRandomLengthString(MAX_STRING_LENGTH));
            std::u16string processName2 = Str8ToStr16(provider.ConsumeRandomLengthString(MAX_STRING_LENGTH));
            processList.emplace_back(processName1);
            processList.emplace_back(processName2);
            g_saMgrService->UnloadProcess(processList);
            break;
        }
        case IpcCode::GET_LRU_IDLE_SYSTEM_ABILITY_PROCESS: {
            std::vector<IdleProcessInfo> infos;
            g_saMgrService->GetLruIdleSystemAbilityProc(infos);
            break;
        }
        default:
            break;
    }
    return 0;
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    g_saMgrService->SetFfrt();
    FuzzSaMgrService(fdp);
    g_saMgrService->CleanFfrt();
    return 0;
}
