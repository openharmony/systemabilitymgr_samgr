/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "distributed_sched_service.h"

#include <cinttypes>
#include <unistd.h>

#include "adapter/dnetwork_adapter.h"
#include "distributed_sched_adapter.h"
#include "dtbschedmgr_log.h"

#include "ability_manager_client.h"
#include "datetime_ex.h"
#include "element_name.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "parcel_helper.h"
#include "string_ex.h"
#include "system_ability_definition.h"

#include "distributed_sched_adapter.h"
#include "dtbschedmgr_device_info_storage.h"

namespace OHOS {
namespace DistributedSchedule {
using namespace AAFwk;
using namespace AppExecFwk;

namespace {
const std::string PARAM_CALLING_UID = "dms.extra.param.key.callingUid";
const std::string PARAM_CALLING_PID = "dms.extra.param.key.callingPid";
const std::string PARAM_CALLING_NETWORK_ID = "dms.extra.param.key.srcNetworkId";
}

IMPLEMENT_SINGLE_INSTANCE(DistributedSchedService);

const bool REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(&DistributedSchedService::GetInstance());

DistributedSchedService::DistributedSchedService() : SystemAbility(DISTRIBUTED_SCHED_SA_ID, true)
{
}

void DistributedSchedService::OnStart()
{
    if (!Init()) {
        HILOGE("failed to init DistributedSchedService");
        return;
    }
    HILOGI("DistributedSchedService::OnStart start service success.");
}

bool DistributedSchedService::Init()
{
    HILOGD("DistributedSchedService::Init ready to init.");

    bool ret = Publish(this);
    if (!ret) {
        HILOGE("DistributedSchedService::Init Publish failed!");
        return false;
    }

    if (!DtbschedmgrDeviceInfoStorage::GetInstance().Init()) {
        HILOGE("DistributedSchedService::Init DtbschedmgrDeviceInfoStorage init failed.");
    }
    HILOGD("DistributedSchedService::Init init success.");
    DistributedSchedAdapter::GetInstance().Init();
    DnetworkAdapter::GetInstance()->Init();
    return true;
}

void DistributedSchedService::OnStop()
{
    HILOGD("DistributedSchedService::OnStop ready to stop service.");
}

int32_t DistributedSchedService::StartRemoteAbility(const OHOS::AAFwk::Want& want,
    const OHOS::AppExecFwk::AbilityInfo& abilityInfo, int32_t requestCode)
{
    std::string deviceId = want.GetElement().GetDeviceID();
    if (deviceId.empty()) {
        HILOGE("StartRemoteAbility check deviceId fail");
        return INVALID_PARAMETERS_ERR;
    }
    sptr<IDistributedSched> remoteDms = GetRemoteDms(deviceId);
    if (remoteDms == nullptr) {
        HILOGE("StartRemoteAbility DMS get remoteDms failed");
        return INVALID_PARAMETERS_ERR;
    }
    CallerInfo callerInfo;
    AccountInfo accountInfo;
    HILOGI("[PerformanceTest] DistributedSchedService StartRemoteAbility transact begin");
    int32_t result = remoteDms->StartAbilityFromRemote(want, abilityInfo, requestCode, callerInfo, accountInfo);
    HILOGI("[PerformanceTest] DistributedSchedService StartRemoteAbility transact end");
    return result;
}

int32_t DistributedSchedService::StartAbilityFromRemote(const OHOS::AAFwk::Want& want,
    const OHOS::AppExecFwk::AbilityInfo& abilityInfo, int32_t requestCode,
    const CallerInfo& callerInfo, const AccountInfo& accountInfo)
{
    std::string deviceId = want.GetElement().GetDeviceID();
    if (deviceId.empty()) {
        HILOGE("StartAbilityFromRemote check deviceId fail");
        return INVALID_REMOTE_PARAMETERS_ERR;
    }
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->Connect();
    if (err != ERR_OK) {
        HILOGE("StartAbilityFromRemote connect ability server failed %{public}d", err);
        return err;
    }
    err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, requestCode);
    if (err != ERR_OK) {
        HILOGE("StartAbilityFromRemote is failed %{public}d", err);
    }
    return err;
}

int32_t DistributedSchedService::StartContinuation(const OHOS::AAFwk::Want& want,
    const OHOS::AppExecFwk::AbilityInfo& abilityInfo, const sptr<IRemoteObject>& abilityToken)
{
    return 0;
}

void DistributedSchedService::NotifyCompleteContinuation(const std::u16string& devId, int32_t sessionId, bool isSuccess)
{
}

int32_t DistributedSchedService::NotifyContinuationResultFromRemote(int32_t sessionId, bool isSuccess)
{
    return 0;
}

int32_t DistributedSchedService::RegisterAbilityToken(const sptr<IRemoteObject>& abilityToken,
    const sptr<IRemoteObject>& continuationCallback)
{
    return 0;
}

int32_t DistributedSchedService::UnregisterAbilityToken(const sptr<IRemoteObject>& abilityToken,
    const sptr<IRemoteObject>& continuationCallback)
{
    return 0;
}

sptr<IDistributedSched> DistributedSchedService::GetRemoteDms(const std::string& remoteDeviceId)
{
    if (remoteDeviceId.empty()) {
        HILOGE("GetRemoteDms remoteDeviceId is empty");
        return nullptr;
    }
    HILOGD("GetRemoteDms connect deviceid is %s", remoteDeviceId.c_str());
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        HILOGE("GetRemoteDms failed to connect to systemAbilityMgr!");
        return nullptr;
    }
    HILOGD("[PerformanceTest] DistributedSchedService GetRemoteDms begin");
    auto object = samgr->CheckSystemAbility(DISTRIBUTED_SCHED_SA_ID, remoteDeviceId);
    HILOGD("[PerformanceTest] DistributedSchedService GetRemoteDms end");
    if (object == nullptr) {
        HILOGE("GetRemoteDms failed to get remote DistributedSched %{private}s", remoteDeviceId.c_str());
        return nullptr;
    }
    return iface_cast<IDistributedSched>(object);
}
} // namespace DistributedSchedule
} // namespace OHOS