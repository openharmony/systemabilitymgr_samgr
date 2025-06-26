/*
 * Copyright (C) 2021-2025 Huawei Device Co., Ltd.
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

#include "dbinder_service.h"
#include "sam_log.h"

namespace OHOS {
DBinderService::DBinderService()
{
    HILOGI("Mock_DBinderService::DBinderService");
}

DBinderService::~DBinderService()
{
    HILOGI("Mock_DBinderService::~DBinderService");
}

sptr<DBinderServiceStub> DBinderService::MakeRemoteBinder(const std::u16string &serviceName,
    const std::string &deviceID, int32_t binderObject, uint32_t pid, uint32_t uid)
{
    HILOGI("Mock_DBinderService::MakeRemoteBinder");
    return nullptr;
}

void DBinderService::LoadSystemAbilityComplete(const std::string& srcNetworkId, int32_t systemAbilityId,
    const sptr<IRemoteObject>& remoteObject)
{
    HILOGI("Mock_DBinderService::LoadSystemAbilityComplete");
}

bool DBinderService::RegisterRemoteProxy(std::u16string serviceName, int32_t systemAbilityId)
{
    HILOGI("Mock_DBinderService::RegisterRemoteProxy");
    return true;
}

bool DBinderService::StartDBinderService(std::shared_ptr<RpcSystemAbilityCallback> &callbackImpl)
{
    HILOGI("Mock_DBinderService::StartDBinderService");
    return true;
}

int32_t DBinderService::NoticeServiceDie(const std::u16string &serviceName, const std::string &deviceID)
{
    HILOGI("Mock_DBinderService::NoticeServiceDie");
    return ERR_NONE;
}

int32_t DBinderService::NoticeDeviceDie(const std::string &deviceID)
{
    HILOGI("Mock_DBinderService::NoticeDeviceDie");
    return ERR_NONE;
}
} // namespace OHOS
