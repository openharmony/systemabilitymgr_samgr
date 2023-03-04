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

#include "system_process_status_change_proxy.h"

#include "ipc_types.h"
#include "iremote_object.h"
#include "message_option.h"
#include "message_parcel.h"
#include "refbase.h"
#include "sam_log.h"
#include "string_ex.h"

namespace OHOS {
void SystemProcessStatusChangeProxy::OnSystemProcessStarted(SystemProcessInfo& systemProcessInfo)
{
    bool ret = SendRequestInner(ON_SYSTEM_PROCESS_STARTED, systemProcessInfo);
    if (!ret) {
        HILOGE("OnSystemProcessStarted SendRequest failed!");
    }
}

void SystemProcessStatusChangeProxy::OnSystemProcessStopped(SystemProcessInfo& systemProcessInfo)
{
    bool ret = SendRequestInner(ON_SYSTEM_PROCESS_STOPPED, systemProcessInfo);
    if (!ret) {
        HILOGE("ON_SYSTEM_PROCESS_STOPPED SendRequest failed!");
    }
}

bool SystemProcessStatusChangeProxy::SendRequestInner(uint32_t code, SystemProcessInfo systemProcessInfo)
{
    HILOGD("%{public}s called, process name is %{public}s", __func__,
        systemProcessInfo.processName.c_str());
    if (systemProcessInfo.processName.empty()) {
        HILOGI("process name is invalid!");
        return false;
    }

    sptr<IRemoteObject> iro = Remote();
    if (iro == nullptr) {
        HILOGE("SendRequestInner Remote return null");
        return false;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return false;
    }
    if (!data.WriteString(systemProcessInfo.processName)) {
        HILOGW("SendRequestInner write processName failed!");
        return false;
    }
    if (!data.WriteInt32(systemProcessInfo.pid)) {
        HILOGW("SendRequestInner write pid failed!");
        return false;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int32_t status = iro->SendRequest(code, data, reply, option);
    if (status != NO_ERROR) {
        HILOGE("SendRequestInner SendRequest failed, return value : %{public}d", status);
        return false;
    }
    return true;
}
}
