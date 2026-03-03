/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "system_process_status_change_stub.h"

#include "errors.h"
#include "ipc_object_stub.h"
#include "ipc_types.h"
#include "message_option.h"
#include "message_parcel.h"
#include "refbase.h"
#include "sam_log.h"

namespace OHOS {
SystemProcessStatusChangeStub::SystemProcessStatusChangeStub()
{
    memberFuncMap_[ON_SYSTEM_PROCESS_STARTED] =
        SystemProcessStatusChangeStub::LocalSystemProcessStarted;
    memberFuncMap_[ON_SYSTEM_PROCESS_STOPPED] =
        SystemProcessStatusChangeStub::LocalSystemProcessStopped;
    memberFuncMap_[ON_SYSTEM_PROCESS_ACTIVATED] =
        SystemProcessStatusChangeStub::LocalSystemProcessActivated;
    memberFuncMap_[ON_SYSTEM_PROCESS_IDLED] =
        SystemProcessStatusChangeStub::LocalSystemProcessIdled;
}

int32_t SystemProcessStatusChangeStub::OnRemoteRequest(uint32_t code,
    MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    HILOGD("ProcStaChange,code:%{public}u,flags:%{public}d", code, option.GetFlags());
    if (!EnforceInterceToken(data)) {
        HILOGW("check interface token failed!");
        return ERR_PERMISSION_DENIED;
    }
    auto iter = memberFuncMap_.find(code);
    if (iter != memberFuncMap_.end()) {
        return iter->second(this, data, reply);
    }
    HILOGW("unknown request code!");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

namespace {
// local function so that it does not break abi
bool ReadProcessInfo(MessageParcel& data, SystemProcessInfo& info)
{
    if (!data.ReadString(info.processName)) {
        HILOGW("read processName failed!");
        return false;
    }
    if (!data.ReadInt32(info.pid)) {
        HILOGW("read pid failed!");
        return false;
    }
    if (!data.ReadInt32(info.uid)) {
        HILOGW("read tid failed!");
        return false;
    }
    return true;
}
} // namespace

int32_t SystemProcessStatusChangeStub::OnSystemProcessStartedInner(MessageParcel& data, MessageParcel& reply)
{
    SystemProcessInfo systemProcessInfo;
    if (!ReadProcessInfo(data, systemProcessInfo)) {
        return ERR_NULL_OBJECT;
    }
    HILOGI("onProcStart,pid:%{public}d", systemProcessInfo.pid);
    OnSystemProcessStarted(systemProcessInfo);
    return ERR_NONE;
}

int32_t SystemProcessStatusChangeStub::OnSystemProcessStoppedInner(MessageParcel& data, MessageParcel& reply)
{
    SystemProcessInfo systemProcessInfo;
    if (!ReadProcessInfo(data, systemProcessInfo)) {
        return ERR_NULL_OBJECT;
    }
    HILOGI("onProcStop,pid:%{public}d", systemProcessInfo.pid);
    OnSystemProcessStopped(systemProcessInfo);
    return ERR_NONE;
}

int32_t SystemProcessStatusChangeStub::OnSystemProcessActivatedInner(MessageParcel& data, MessageParcel& reply)
{
    SystemProcessInfo systemProcessInfo;
    if (!ReadProcessInfo(data, systemProcessInfo)) {
        return ERR_NULL_OBJECT;
    }
    HILOGI("onProcActivate,pid:%{public}d", systemProcessInfo.pid);
    OnSystemProcessActivated(systemProcessInfo);
    return ERR_NONE;
}

int32_t SystemProcessStatusChangeStub::OnSystemProcessIdledInner(MessageParcel& data, MessageParcel& reply)
{
    SystemProcessInfo systemProcessInfo;
    if (!ReadProcessInfo(data, systemProcessInfo)) {
        return ERR_NULL_OBJECT;
    }
    HILOGI("onProcIdle,pid:%{public}d", systemProcessInfo.pid);
    OnSystemProcessIdled(systemProcessInfo);
    return ERR_NONE;
}

bool SystemProcessStatusChangeStub::EnforceInterceToken(MessageParcel& data)
{
    std::u16string interfaceToken = data.ReadInterfaceToken();
    return interfaceToken == GetDescriptor();
}
}
