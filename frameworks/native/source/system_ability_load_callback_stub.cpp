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

#include "system_ability_load_callback_stub.h"

#include <cinttypes>
#include "errors.h"
#include "ipc_object_stub.h"
#include "ipc_types.h"
#include "message_parcel.h"
#include "refbase.h"
#include "datetime_ex.h"
#include "sam_log.h"

namespace OHOS {
namespace {
constexpr int32_t FIRST_SYS_ABILITY_ID = 0x00000000;
constexpr int32_t LAST_SYS_ABILITY_ID = 0x00ffffff;
}
int32_t SystemAbilityLoadCallbackStub::OnRemoteRequest(uint32_t code,
    MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    HILOGD("SystemAbilityLoadCallbackStub::OnRemoteRequest, code = %{public}u", code);
    if (!EnforceInterceToken(data)) {
        HILOGW("SystemAbilityLoadCallbackStub::OnRemoteRequest check interface token failed!");
        return ERR_PERMISSION_DENIED;
    }
    switch (code) {
        case ON_LOAD_SYSTEM_ABILITY_SUCCESS:
            return OnLoadSystemAbilitySuccessInner(data, reply);
        case ON_LOAD_SYSTEM_ABILITY_FAIL:
            return OnLoadSystemAbilityFailInner(data, reply);
        case ON_LOAD_SYSTEM_ABILITY_COMPLETE_FOR_REMOTE:
            return OnLoadSACompleteForRemoteInner(data, reply);
        default:
            HILOGW("SystemAbilityLoadCallbackStub::OnRemoteRequest unknown request code!");
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}

int32_t SystemAbilityLoadCallbackStub::OnLoadSystemAbilitySuccessInner(MessageParcel& data, MessageParcel& reply)
    __attribute__((no_sanitize("cfi")))
{
    int32_t systemAbilityId = -1;
    bool ret = data.ReadInt32(systemAbilityId);
    if (!ret) {
        HILOGW("OnLoadSystemAbilitySuccessInner read SA:%{public}d fail!", systemAbilityId);
        return ERR_INVALID_VALUE;
    }
    HILOGD("OnLoadSystemAbilitySuccessInner, SA:%{public}d", systemAbilityId);
    if (!CheckInputSystemAbilityId(systemAbilityId)) {
        HILOGW("OnLoadSystemAbilitySuccessInner invalid SA:%{public}d !", systemAbilityId);
        return ERR_INVALID_VALUE;
    }
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    if (remoteObject == nullptr) {
        HILOGW("OnLoadSystemAbilitySuccessInner read remoteObject fail, SA:%{public}d!", systemAbilityId);
    }
    int64_t begin = OHOS::GetTickCount();
    OnLoadSystemAbilitySuccess(systemAbilityId, remoteObject);
    HILOGW("OnLoadSaSucInner SA:%{public}d spend %{public}" PRId64 "ms", systemAbilityId, GetTickCount() - begin);
    return ERR_NONE;
}

int32_t SystemAbilityLoadCallbackStub::OnLoadSystemAbilityFailInner(MessageParcel& data, MessageParcel& reply)
{
    int32_t systemAbilityId = -1;
    bool ret = data.ReadInt32(systemAbilityId);
    if (!ret) {
        HILOGW("OnLoadSystemAbilityFailInner read SA:%{public}d fail!", systemAbilityId);
        return ERR_INVALID_VALUE;
    }
    HILOGI("OnLoadSystemAbilityFailInner, SA:%{public}d", systemAbilityId);
    if (!CheckInputSystemAbilityId(systemAbilityId)) {
        HILOGW("OnLoadSystemAbilityFailInner invalid SA:%{public}d !", systemAbilityId);
        return ERR_INVALID_VALUE;
    }
    int64_t begin = OHOS::GetTickCount();
    OnLoadSystemAbilityFail(systemAbilityId);
    HILOGW("OnLoadSaFailInner SA:%{public}d spend %{public}" PRId64 "ms", systemAbilityId, GetTickCount() - begin);
    return ERR_NONE;
}

int32_t SystemAbilityLoadCallbackStub::OnLoadSACompleteForRemoteInner(MessageParcel& data, MessageParcel& reply)
{
    std::string deviceId = data.ReadString();
    int32_t systemAbilityId = -1;
    bool ret = data.ReadInt32(systemAbilityId);
    if (!ret) {
        HILOGW("OnLoadSACompleteForRemoteInner read SA:%{public}d fail!", systemAbilityId);
        return ERR_INVALID_VALUE;
    }
    if (!CheckInputSystemAbilityId(systemAbilityId)) {
        HILOGW("OnLoadSACompleteForRemoteInner invalid SA:%{public}d !", systemAbilityId);
        return ERR_INVALID_VALUE;
    }
    ret = data.ReadBool();
    HILOGI("OnLoadSACompleteForRemoteInner load SA:%{public}d %{public}s",
        systemAbilityId, ret ? "succeed" : "failed");
    sptr<IRemoteObject> remoteObject = ret ? data.ReadRemoteObject() : nullptr;
    int64_t begin = OHOS::GetTickCount();
    OnLoadSACompleteForRemote(deviceId, systemAbilityId, remoteObject);
    HILOGW("OnLoadRemoteSaInner SA:%{public}d spend %{public}" PRId64 "ms", systemAbilityId, GetTickCount() - begin);
    return ERR_NONE;
}

bool SystemAbilityLoadCallbackStub::CheckInputSystemAbilityId(int32_t systemAbilityId)
{
    return (systemAbilityId >= FIRST_SYS_ABILITY_ID) && (systemAbilityId <= LAST_SYS_ABILITY_ID);
}

bool SystemAbilityLoadCallbackStub::EnforceInterceToken(MessageParcel& data)
{
    std::u16string interfaceToken = data.ReadInterfaceToken();
    return interfaceToken == GetDescriptor();
}
}
