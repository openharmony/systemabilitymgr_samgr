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

#include "loadremotesystemability_fuzzer.h"

#include "fuzztest_utils.h"
#include "samgr_ipc_interface_code.h"
namespace OHOS {
namespace Samgr {
namespace {
constexpr size_t THRESHOLD = 4;
const std::u16string SAMGR_INTERFACE_TOKEN = u"ohos.samgr.accessToken";
}

void FuzzLoadSystemAbility(const uint8_t *data, size_t size)
{
    sptr<MockSystemAbilityLoadCallback> saLoadCb = new(std::nothrow) MockSystemAbilityLoadCallback();
    if (saLoadCb == nullptr) {
        return;
    }
    MessageParcel parcelData;
    parcelData.WriteInterfaceToken(SAMGR_INTERFACE_TOKEN);
    int32_t systemAbilityId = FuzzTestUtils::BuildInt32FromData(data, size);
    parcelData.WriteInt32(systemAbilityId);
    parcelData.WriteRemoteObject(saLoadCb);
    FuzzTestUtils::FuzzTestRemoteRequest(parcelData,
        static_cast<uint32_t>(SamgrInterfaceCode::LOAD_SYSTEM_ABILITY_TRANSACTION));
}

void FuzzLoadRemoteSystemAbility(const uint8_t *data, size_t size)
{
    FuzzTestUtils::FuzzTestRemoteRequest(data, size,
        static_cast<uint32_t>(SamgrInterfaceCode::LOAD_REMOTE_SYSTEM_ABILITY_TRANSACTION));
}
}
}
/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::Samgr::THRESHOLD) {
        return 0;
    }
    OHOS::Samgr::FuzzLoadSystemAbility(data, size);
    OHOS::Samgr::FuzzLoadRemoteSystemAbility(data, size);
    return 0;
}