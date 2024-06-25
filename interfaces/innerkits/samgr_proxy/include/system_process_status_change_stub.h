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

#ifndef SAMGR_INTERFACES_INNERKITS_SAMGR_PROXY_INCLUDE_SYSTEM_PROCESS_STATUS_CHANGE_STUB_H
#define SAMGR_INTERFACES_INNERKITS_SAMGR_PROXY_INCLUDE_SYSTEM_PROCESS_STATUS_CHANGE_STUB_H

#include <map>
#include "iremote_stub.h"
#include "isystem_process_status_change.h"

namespace OHOS {
class SystemProcessStatusChangeStub : public IRemoteStub<ISystemProcessStatusChange> {
public:
    SystemProcessStatusChangeStub();
    ~SystemProcessStatusChangeStub() = default;
    int32_t OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;
private:
    static int32_t LocalSystemProcessStarted(SystemProcessStatusChangeStub *stub,
        MessageParcel& data, MessageParcel& reply)
    {
        return stub->OnSystemProcessStartedInner(data, reply);
    }
    static int32_t LocalSystemProcessStopped(SystemProcessStatusChangeStub *stub,
        MessageParcel& data, MessageParcel& reply)
    {
        return stub->OnSystemProcessStoppedInner(data, reply);
    }
    int32_t OnSystemProcessStartedInner(MessageParcel& data, MessageParcel& reply);
    int32_t OnSystemProcessStoppedInner(MessageParcel& data, MessageParcel& reply);
    static bool EnforceInterceToken(MessageParcel& data);

    using SystemProcessStatusChangeStubFunc =
        int32_t (*)(SystemProcessStatusChangeStub* stub, MessageParcel& data, MessageParcel& reply);
    std::map<uint32_t, SystemProcessStatusChangeStubFunc> memberFuncMap_;
};
}
#endif /* SAMGR_INTERFACES_INNERKITS_SAMGR_PROXY_INCLUDE_SYSTEM_PROCESS_STATUS_CHANGE_STUB_H */
