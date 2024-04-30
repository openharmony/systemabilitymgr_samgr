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

#include "localsamgrproxy_fuzzer.h"

#include "local_ability_manager_proxy.h"

namespace OHOS {
namespace Samgr {
namespace {
    constexpr size_t THRESHOLD = 10;
    constexpr size_t ONDEMANDSIZE = 6;
    constexpr int32_t MAX_DELAY_TIME = 5 * 60 * 1000;
    constexpr int32_t TYPE = 0;
    constexpr int32_t LEVEL = 0;
    constexpr int32_t LAST_SYS_ABILITY_ID = 16777215;
    const std::string KEY_EVENT_ID = "eventId";
    const std::string KEY_NAME = "name";
    const std::string KEY_VALUE = "value";
    const std::string KEY_EXTRA_DATA_ID = "extraDataId";
class LocalSamgrProxyFuzzer : public IRemoteStub<ILocalAbilityManager> {
public:
    bool StartAbility(int32_t systemAbilityId, const std::string& eventStr) override
    {
        return true;
    }
    bool StopAbility(int32_t systemAbilityId, const std::string& eventStr) override
    {
        return true;
    }
    bool ActiveAbility(int32_t systemAbilityId, const nlohmann::json& activeReason) override
    {
        return true;
    }
    bool IdleAbility(int32_t systemAbilityId, const nlohmann::json& idleReason, int32_t& delayTime) override
    {
        return true;
    }
    bool SendStrategyToSA(int32_t type, int32_t systemAbilityId, int32_t level, std::string& action) override
    {
        return true;
    }
    bool IpcStatCmdProc(int32_t fd, int32_t cmd) override
    {
        return true;
    }
    bool FfrtDumperProc(std::string& ffrtDumperInfo) override
    {
        return true;
    }

    int32_t SystemAbilityExtProc(const std::string& extension, int32_t said,
        SystemAbilityExtensionPara* callback, bool isAsync = false) override
    {
        return 0;
    }

    int32_t OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override
    {
        return 0;
    }
};
sptr<LocalAbilityManagerProxy> CreateLocalAbilityManagerProxy()
{
    MessageParcel data;
    sptr<ILocalAbilityManager> testService = new (std::nothrow) LocalSamgrProxyFuzzer();
    if (testService == nullptr) {
        return nullptr;
    }
    data.WriteRemoteObject(testService->AsObject());
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    sptr<LocalAbilityManagerProxy> testLocalSamgrProxy = new (std::nothrow) LocalAbilityManagerProxy(remoteObject);
    return testLocalSamgrProxy;
}

sptr<LocalAbilityManagerProxy> g_LocalSamgrProxy = CreateLocalAbilityManagerProxy();
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

void LocalSamgrProxyFuzzTest(const uint8_t* data, size_t size)
{
    if (g_LocalSamgrProxy == nullptr) {
        return;
    }
    int32_t intValue = BuildInt32FromData(data, size);
    std::string stringValue = BuildStringFromData(data, size);

    int32_t saId = intValue;
    if (saId > LAST_SYS_ABILITY_ID) {
        return;
    }
    std::string eventStr = stringValue;
    g_LocalSamgrProxy->StartAbility(intValue, stringValue);
    g_LocalSamgrProxy->StopAbility(intValue, stringValue);

    nlohmann::json reason;
    int32_t eventId = intValue % ONDEMANDSIZE;
    reason[KEY_EVENT_ID] = eventId;
    reason[KEY_NAME] = "";
    reason[KEY_VALUE] = "";
    reason[KEY_EXTRA_DATA_ID] = -1;
    g_LocalSamgrProxy->ActiveAbility(saId, reason);
    
    int32_t delayTime = intValue;
    if (delayTime > MAX_DELAY_TIME) {
        delayTime = MAX_DELAY_TIME;
    }
    g_LocalSamgrProxy->IdleAbility(saId, reason, delayTime);

    std::string action = stringValue;
    g_LocalSamgrProxy->SendStrategyToSA(TYPE, saId, LEVEL, action);

    int32_t fd = intValue;
    int32_t cmd = intValue;
    g_LocalSamgrProxy->IpcStatCmdProc(fd, cmd);

    std::string ffrtDumpInfo = eventStr;
    g_LocalSamgrProxy->FfrtDumperProc(ffrtDumpInfo);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::Samgr::THRESHOLD) {
        return 0;
    }
    OHOS::Samgr::LocalSamgrProxyFuzzTest(data, size);
    return 0;
}

