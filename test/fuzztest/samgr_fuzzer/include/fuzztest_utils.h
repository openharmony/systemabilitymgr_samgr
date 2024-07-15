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

#ifndef SAMGR_TEST_FUZZTEST_SYSTEM_ABILITY_MANAGER_FUZZER_UTILS_H
#define SAMGR_TEST_FUZZTEST_SYSTEM_ABILITY_MANAGER_FUZZER_UTILS_H
#include <cstddef>
#include <cstdint>
#include "system_ability_status_change_stub.h"
#include "system_ability_load_callback_stub.h"
#include "system_process_status_change_stub.h"
namespace OHOS {
namespace Samgr {
class FuzzTestUtils {
public:
    static void FuzzTestRemoteRequest(const uint8_t *rawData, size_t size, uint32_t code);
    static void FuzzTestRemoteRequest(MessageParcel& data, uint32_t code);
    static bool BuildBoolFromData(const uint8_t* data, size_t size)
    {
        if ((data == nullptr) || (size < sizeof(bool))) {
            return false;
        }
        bool boolVal = static_cast<bool>(*data);
        return boolVal;
    }
    static int32_t BuildInt32FromData(const uint8_t* data, size_t size)
    {
        if ((data == nullptr) || (size < sizeof(int32_t))) {
            return 0;
        }
        int32_t int32Val = *reinterpret_cast<const int32_t *>(data);
        return int32Val;
    }
    static std::string BuildStringFromData(const uint8_t* data, size_t size)
    {
        if ((data == nullptr) || (size == 0)) {
            return "";
        }
        std::string strVal(reinterpret_cast<const char *>(data), size);
        return strVal;
    }
private:
    static bool IsDmReady();
    static void AddDeviceManager();
};

class MockSystemProcessStatusChange : public SystemProcessStatusChangeStub {
public:
    void OnSystemProcessStarted(SystemProcessInfo& systemProcessInfo) override {}
    void OnSystemProcessStopped(SystemProcessInfo& systemProcessInfo) override {}
};

class MockSystemAbilityStatusChange : public SystemAbilityStatusChangeStub {
public:
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override {}
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override {}
};

class MockSystemAbilityLoadCallback : public SystemAbilityLoadCallbackStub {
public:
    void OnLoadSystemAbilitySuccess(int32_t systemAbilityId, const sptr<IRemoteObject>& remoteObject) override {}

    void OnLoadSystemAbilityFail(int32_t systemAbilityId) override {}
};
}
} // namespace OHOS

#endif
