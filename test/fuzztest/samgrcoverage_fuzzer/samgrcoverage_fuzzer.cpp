/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "samgrcoverage_fuzzer.h"

#define private public
#include "if_system_ability_manager.h"
#include "itest_transaction_service.h"
#include "sa_status_change_mock.h"
#include "sam_mock_permission.h"
#include "ability_death_recipient.h"
#include "system_ability_manager.h"
#include "iservice_registry.h"
#include "hisysevent_adapter.h"
#include "system_ability_manager_util.h"

#include <cinttypes>
#include <cstddef>
#include <cstdint>
#include <unistd.h>
#include <cstdlib>
#include <fcntl.h>

namespace OHOS {
namespace Samgr {
namespace {
    constexpr size_t THRESHOLD = 10;
    constexpr int32_t SAID = 1493;
    constexpr int32_t ADD_SA_TRANSACTION = 3;
    constexpr int32_t REMOVE_SA_TRANSACTION = 4;
    
}

int32_t BuildInt32FromData(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return 0;
    }
    int32_t int32Val = *reinterpret_cast<const int32_t *>(data);
    return int32Val;
}

std::string BuildStringFromData(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return "";
    }
    std::string strVal(reinterpret_cast<const char *>(data), size);
    return strVal;
}

void FuzzOndemandLoad(const uint8_t* data, size_t size)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    if (saMgr == nullptr) {
        return;
    }
    std::string procName = BuildStringFromData(data, size);
    std::u16string procNameU16 = Str8ToStr16(procName);
    int32_t saId = BuildInt32FromData(data, size);
    SystemAbilityManager::AbilityItem abilityItem;
    ISystemAbilityManager::SAExtraProp saExtraProp;
    bool isExist = false;
    saMgr->AddSamgrToAbilityMap();
    saMgr->OndemandLoad();
    saMgr->DoLoadForPerf();
    saMgr->RemoveWhiteCommonEvent();
    OnDemandEvent onDemandEvent;
    SaControlInfo saControlInfo;
    std::list<SaControlInfo> saControlList;
    saControlList.emplace_back(saControlInfo);
    saMgr->ProcessOnDemandEvent(onDemandEvent, saControlList);
    saMgr->GetSystemAbilityFromRemote(saId);
    saMgr->startingAbilityMap_[saId] = abilityItem;
    saMgr->StartOnDemandAbility(procNameU16, saId);
    saMgr->startingAbilityMap_.clear();
    saMgr->StopOnDemandAbility(procNameU16, saId, onDemandEvent);
    saMgr->AddOnDemandSystemAbilityInfo(saId, procNameU16);
    saMgr->StartOnDemandAbility(saId, isExist);

    sptr<IRemoteObject> testAbility = new TestTransactionService();
    saMgr->AddSystemAbility(SAID, testAbility, saExtraProp);
    nlohmann::json reason;
    int32_t delayTime = 0;
    saMgr->IdleSystemAbility(SAID, procNameU16, reason, delayTime);
    saMgr->ActiveSystemAbility(SAID, procNameU16, reason);
    saMgr->RemoveSystemAbility(SAID);
    saMgr->AddSystemAbility(SAID, testAbility, saExtraProp);
    saMgr->RemoveDiedSystemAbility(SAID);
    saMgr->AddSystemAbility(SAID, testAbility, saExtraProp);
    saMgr->RemoveSystemAbility(testAbility);
}

void FuzzRemoveSystemProcess(const uint8_t* data, size_t size)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    if (saMgr == nullptr) {
        return;
    }
    std::string procName = BuildStringFromData(data, size);
    std::u16string procNameU16 = Str8ToStr16(procName);
    sptr<IRemoteObject> testAbility(new SaStatusChangeMock());
    saMgr->AddSystemProcess(procNameU16, testAbility);
    saMgr->RemoveSystemProcess(testAbility);

    sptr<SaStatusChangeMock> callback(new SaStatusChangeMock());
    saMgr->listenerMap_[SAID].push_back({callback, SAID});
    auto& count = saMgr->subscribeCountMap_[SAID];
    ++count;
    saMgr->UnSubscribeSystemAbility(callback->AsObject());

    u16string name = u"test";
    string srcDeviceId = "srcDeviceId";
    saMgr->startingProcessMap_.clear();
    sptr<SystemAbilityLoadCallbackMock> callbackOne = new SystemAbilityLoadCallbackMock();
    SystemAbilityManager::AbilityItem abilityItem;
    abilityItem.callbackMap[srcDeviceId].push_back(make_pair(callbackOne, SAID));
    saMgr->startingAbilityMap_[SAID] = abilityItem;
    saMgr->CleanCallbackForLoadFailed(SAID, name, srcDeviceId, callbackOne);
}

void FuzzNotifySystemAbilityLoaded(const uint8_t* data, size_t size)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    if (saMgr == nullptr) {
        return;
    }
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    sptr<IRemoteObject> remoteObject = new TestTransactionService();
    saMgr->NotifySystemAbilityLoaded(SAID, remoteObject, callback);
    string srcDeviceId = "srcDeviceId";
    int32_t systemAbilityId = BuildInt32FromData(data, size);
    saMgr->LoadSystemAbilityFromRpc(srcDeviceId, systemAbilityId, callback);
    saMgr->CheckSaIsImmediatelyRecycle(systemAbilityId);

    saMgr->startingAbilityMap_.clear();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback1 = new SystemAbilityLoadCallbackMock();
    std::map<std::string, SystemAbilityManager::CallbackList> mockCallbackMap1 = {
        {"111111", {{mockLoadCallback1, 0}}}
    };
    SystemAbilityManager::AbilityItem mockAbilityItem1 = {
        SystemAbilityManager::AbilityState::INIT, mockCallbackMap1
    };
    saMgr->startingAbilityMap_[SAID] = mockAbilityItem1;
    saMgr->OnAbilityCallbackDied(mockLoadCallback1->AsObject());

    saMgr->remoteCallbacks_.clear();
    sptr<ISystemAbilityLoadCallback> mockLoadCallback2 = new SystemAbilityLoadCallbackMock();
    saMgr->remoteCallbacks_ = {
        {"11111", {mockLoadCallback2}}
    };
    saMgr->OnRemoteCallbackDied(mockLoadCallback2->AsObject());

    sptr<SystemAbilityLoadCallbackMock> callback2 = new SystemAbilityLoadCallbackMock();
    list<sptr<ISystemAbilityLoadCallback>> callbacks;
    callbacks.push_back(callback2);
    saMgr->remoteCallbackDeath_ = sptr<IRemoteObject::DeathRecipient>(new RemoteCallbackDeathRecipient());
    saMgr->RemoveRemoteCallbackLocked(callbacks, callback2);
}

void FuzzGetAllOndemandSa(const uint8_t* data, size_t size)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    if (saMgr == nullptr) {
        return;
    }
    int32_t saId = BuildInt32FromData(data, size);
    CommonSaProfile saProfile;
    saMgr->saProfileMap_[saId] = saProfile;
    saMgr->GetAllOndemandSa();
    saMgr->saProfileMap_.erase(saId);
}

void FuzzReportGetSAPeriodically(const uint8_t* data, size_t size)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    if (saMgr == nullptr) {
        return;
    }
    int32_t uid = BuildInt32FromData(data, size);
    int32_t saId = BuildInt32FromData(data, size);
    uint64_t key = SamgrUtil::GenerateFreKey(uid, saId);
    saMgr->saFrequencyMap_[key] = BuildInt32FromData(data, size);
    saMgr->ReportGetSAPeriodically();
}

void FuzzNotifySystemAbilityChanged(const uint8_t* data, size_t size)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    if (saMgr == nullptr) {
        return;
    }
    sptr<SaStatusChangeMock> testAbility(new SaStatusChangeMock());
    int32_t saId = BuildInt32FromData(data, size);
    saMgr->NotifySystemAbilityChanged(saId, "deviceId", 1, nullptr);
    saMgr->NotifySystemAbilityChanged(saId, "deviceId", ADD_SA_TRANSACTION, testAbility);
    saMgr->NotifySystemAbilityChanged(saId, "deviceId", REMOVE_SA_TRANSACTION, testAbility);
}

void FuzzRemoveOnDemandSaInDiedProc(const uint8_t* data, size_t size)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    if (saMgr == nullptr) {
        return;
    }
    int32_t saId = BuildInt32FromData(data, size);
    auto processContext = std::make_shared<SystemProcessContext>();
    processContext->saList.push_back(saId);
    saMgr->RemoveOnDemandSaInDiedProc(processContext);
}

void FuzzDoLoadOnDemandAbility(const uint8_t* data, size_t size)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    if (saMgr == nullptr) {
        return;
    }
    int32_t saId = BuildInt32FromData(data, size);
    bool isExist = false;
    saMgr->DoLoadOnDemandAbility(saId, isExist);
}

void FuzzNotifySystemAbilityAddedBySync(const uint8_t* data, size_t size)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    if (saMgr == nullptr) {
        return;
    }
    int32_t saId = BuildInt32FromData(data, size);
    sptr<ISystemAbilityStatusChange> listener;
    saMgr->NotifySystemAbilityAddedBySync(saId, listener);
}

void FuzzSendLoadedSystemAbilityMsg(const uint8_t* data, size_t size)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    if (saMgr == nullptr) {
        return;
    }
    int32_t saId = BuildInt32FromData(data, size);
    sptr<IRemoteObject> testAbility(nullptr);
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    saMgr->SendLoadedSystemAbilityMsg(saId, testAbility, callback);
}

void FuzzNotifySystemAbilityLoadFail(const uint8_t* data, size_t size)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    if (saMgr == nullptr) {
        return;
    }
    int32_t saId = BuildInt32FromData(data, size);
    sptr<SystemAbilityLoadCallbackMock> callback = new SystemAbilityLoadCallbackMock();
    saMgr->NotifySystemAbilityLoadFail(saId, callback);
}

void FuzzStartingSystemProcessLocked(const uint8_t* data, size_t size)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    if (saMgr == nullptr) {
        return;
    }
    int32_t saId = BuildInt32FromData(data, size);
    OnDemandEvent event;
    saMgr->StartingSystemProcessLocked(u"procName", saId, event);
}

void FuzzDoLoadSystemAbilityFromRpc(const uint8_t* data, size_t size)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    if (saMgr == nullptr) {
        return;
    }
    int32_t saId = BuildInt32FromData(data, size);
    sptr<ISystemAbilityLoadCallback> callback = new SystemAbilityLoadCallbackMock();
    OnDemandEvent event;
    saMgr->DoLoadSystemAbilityFromRpc("srcDeviceId", saId, u"procName", callback, event);
}

void FuzzDoUnloadSystemAbility(const uint8_t* data, size_t size)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    if (saMgr == nullptr) {
        return;
    }
    int32_t saId = BuildInt32FromData(data, size);
    OnDemandEvent event;
    saMgr->DoUnloadSystemAbility(saId, u"procName", event);
}

void FuzzNotifyRpcLoadCompleted(const uint8_t* data, size_t size)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    if (saMgr == nullptr) {
        return;
    }
    int32_t saId = BuildInt32FromData(data, size);
    sptr<IRemoteObject> testAbility(nullptr);
    saMgr->NotifyRpcLoadCompleted("srcDeviceId", saId, testAbility);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::Samgr::THRESHOLD) {
        return 0;
    }
    OHOS::Samgr::FuzzOndemandLoad(data, size);
    OHOS::Samgr::FuzzRemoveSystemProcess(data, size);
    OHOS::Samgr::FuzzNotifySystemAbilityLoaded(data, size);
    OHOS::Samgr::FuzzGetAllOndemandSa(data, size);
    OHOS::Samgr::FuzzReportGetSAPeriodically(data, size);
    OHOS::Samgr::FuzzNotifySystemAbilityChanged(data, size);
    OHOS::Samgr::FuzzRemoveOnDemandSaInDiedProc(data, size);
    OHOS::Samgr::FuzzDoLoadOnDemandAbility(data, size);
    OHOS::Samgr::FuzzNotifySystemAbilityAddedBySync(data, size);
    OHOS::Samgr::FuzzSendLoadedSystemAbilityMsg(data, size);
    OHOS::Samgr::FuzzNotifySystemAbilityLoadFail(data, size);
    OHOS::Samgr::FuzzStartingSystemProcessLocked(data, size);
    OHOS::Samgr::FuzzDoLoadSystemAbilityFromRpc(data, size);
    OHOS::Samgr::FuzzDoUnloadSystemAbility(data, size);
    OHOS::Samgr::FuzzNotifyRpcLoadCompleted(data, size);
    return 0;
}

