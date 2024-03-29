/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "systemabilitymanager_fuzzer.h"

#include "if_system_ability_manager.h"
#include "sam_mock_permission.h"
#include "system_ability_manager.h"
#include "iservice_registry.h"

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
    constexpr uint8_t MAX_CALL_TRANSACTION = 64;
    constexpr int32_t OFFSET = 4;
    constexpr int32_t INIT_TIME = 1;
    constexpr int32_t RETRY_TIME_OUT_NUMBER = 10;
    constexpr int32_t SLEEP_INTERVAL_TIME = 200000;
    constexpr int32_t DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID = 4802;
    unsigned int g_dumpLevel = 0;
    const std::u16string SAMGR_INTERFACE_TOKEN = u"ohos.samgr.accessToken";
    bool g_flag = false;
}

uint32_t Convert2Uint32(const uint8_t* ptr)
{
    if (ptr == nullptr) {
        return 0;
    }
    return (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | (ptr[3]); // this is a general method of converting in fuzz
}

bool IsDmReady()
{
    auto dmProxy = SystemAbilityManager::GetInstance()->CheckSystemAbility(
        DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID);
    if (dmProxy != nullptr) {
        IPCObjectProxy* proxy = reinterpret_cast<IPCObjectProxy*>(dmProxy.GetRefPtr());
        if (proxy != nullptr && !proxy->IsObjectDead()) {
            return true;
        }
    }
    HILOGE("samgrFuzz:DM isn't ready");
    return false;
}

void AddDeviceManager()
{
    if (IsDmReady()) {
        return;
    }
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sm == nullptr) {
        HILOGE("samgrFuzz:GetSystemAbilityManager fail");
        return;
    }
    int32_t timeout = RETRY_TIME_OUT_NUMBER;
    int64_t begin = OHOS::GetTickCount();
    sptr<IRemoteObject> dmAbility = nullptr;
    do {
        dmAbility = sm->CheckSystemAbility(DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID);
        if (dmAbility != nullptr) {
            break;
        }
        usleep(SLEEP_INTERVAL_TIME);
    } while (timeout--);
    HILOGI("samgrFuzz:Add DM spend %{public}" PRId64 " ms", OHOS::GetTickCount() - begin);
    if (dmAbility == nullptr) {
        HILOGE("samgrFuzz:dmAbility is null");
        return;
    }
    sptr<SystemAbilityManager> fuzzSAManager = SystemAbilityManager::GetInstance();
    ISystemAbilityManager::SAExtraProp saExtra(false, g_dumpLevel, u"", u"");
    int32_t ret = fuzzSAManager->AddSystemAbility(DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID, dmAbility, saExtra);
    if (ret == ERR_OK) {
        HILOGI("samgrFuzz:Add DM sucess");
        return;
    }
    HILOGE("samgrFuzz:Add DM fail");
}

void FuzzSystemAbilityManager(const uint8_t* rawData, size_t size)
{
    SamMockPermission::MockPermission();
    uint32_t code = Convert2Uint32(rawData);
    rawData = rawData + OFFSET;
    size = size - OFFSET;
    MessageParcel data;
    data.WriteInterfaceToken(SAMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;
    sptr<SystemAbilityManager> manager = SystemAbilityManager::GetInstance();
    if (!g_flag) {
        HILOGI("samgrFuzz:Init");
        manager->Init();
        g_flag = true;
        AddDeviceManager();
        if (!IsDmReady()) {
            manager->CleanFfrt();
            return;
        }
        sleep(INIT_TIME);
    } else {
        AddDeviceManager();
        if (!IsDmReady()) {
            return;
        }
        manager->SetFfrt();
    }
    manager->OnRemoteRequest(code % MAX_CALL_TRANSACTION, data, reply, option);
    manager->CleanFfrt();
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::Samgr::THRESHOLD) {
        return 0;
    }

    OHOS::Samgr::FuzzSystemAbilityManager(data, size);
    return 0;
}

