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

#include "fuzztest_utils.h"

#include "if_system_ability_manager.h"
#include "sam_mock_permission.h"
#include "system_ability_manager.h"
#include "iservice_registry.h"
#include "hisysevent_adapter.h"

#include <cinttypes>
#include <unistd.h>
#include <cstdlib>
#include <fcntl.h>

namespace OHOS {
namespace Samgr {

namespace {
    constexpr int32_t INIT_TIME = 3;
    constexpr int32_t RETRY_TIME_OUT_NUMBER = 10;
    constexpr int32_t SLEEP_INTERVAL_TIME = 200000;
    constexpr int32_t DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID = 4802;
    unsigned int g_dumpLevel = 0;
    const std::u16string SAMGR_INTERFACE_TOKEN = u"ohos.samgr.accessToken";
    bool g_flag = false;
}

bool FuzzTestUtils::IsDmReady()
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

void FuzzTestUtils::AddDeviceManager()
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

void FuzzTestUtils::FuzzTestRemoteRequest(const uint8_t *rawData, size_t size, uint32_t code)
{
    SamMockPermission::MockPermission();
    MessageParcel data;
    data.WriteInterfaceToken(SAMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;
    sptr<SystemAbilityManager> manager = SystemAbilityManager::GetInstance();
    if (!g_flag) {
        HILOGI("TestRequest=%{public}u:Init", code);
        manager->Init();
        g_flag = true;
        HILOGI("TestRequest=%{public}u:Init AddDeviceManager", code);
        AddDeviceManager();
        sleep(INIT_TIME);
        if (!IsDmReady()) {
            HILOGE("TestRequest=%{public}u:Init CleanFfrt", code);
            manager->CleanFfrt();
            return;
        }
    } else {
        HILOGI("TestRequest=%{public}u:AddDeviceManager", code);
        AddDeviceManager();
        if (!IsDmReady()) {
            HILOGE("TestRequest=%{public}u:dm no ready,return", code);
            return;
        }
        HILOGI("TestRequest=%{public}u:SetFfrt", code);
        manager->SetFfrt();
    }
    int32_t ret = manager->OnRemoteRequest(code, data, reply, option);
    HILOGI("TestRequest=%{public}u: ret=%{public}u", code, ret);
    manager->CleanFfrt();
}

void FuzzTestUtils::FuzzTestRemoteRequest(MessageParcel& data, uint32_t code)
{
    SamMockPermission::MockPermission();
    MessageParcel reply;
    MessageOption option;
    sptr<SystemAbilityManager> manager = SystemAbilityManager::GetInstance();
    if (!g_flag) {
        HILOGI("FuzzTestRequest=%{public}u:Init", code);
        manager->Init();
        g_flag = true;
        HILOGI("FuzzTestRequest=%{public}u:Init AddDeviceManager", code);
        AddDeviceManager();
        sleep(INIT_TIME);
        if (!IsDmReady()) {
            HILOGE("FuzzTestRequest=%{public}u:Init CleanFfrt", code);
            manager->CleanFfrt();
            return;
        }
    } else {
        HILOGI("FuzzTestRequest=%{public}u:AddDeviceManager", code);
        AddDeviceManager();
        if (!IsDmReady()) {
            HILOGE("FuzzTestRequest=%{public}u:dm no ready,return", code);
            return;
        }
        HILOGI("FuzzTestRequest=%{public}u:SetFfrt", code);
        manager->SetFfrt();
    }
    int32_t ret = manager->OnRemoteRequest(code, data, reply, option);
    HILOGI("FuzzTestRequest=%{public}u: ret=%{public}u", code, ret);
    manager->CleanFfrt();
}
}
} // namespace OHOS
