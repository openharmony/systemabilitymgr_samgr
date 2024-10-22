/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "system_ability_manager_stub.h"

#include <unistd.h>
#include <cinttypes>

#include "accesstoken_kit.h"
#include "datetime_ex.h"
#include "errors.h"
#include "hitrace_meter.h"
#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "memory_guard.h"
#include "sam_log.h"
#include "string_ex.h"
#include "hisysevent_adapter.h"
#include "system_ability_manager.h"
#include "system_ability_manager_util.h"
#include "system_ability_on_demand_event.h"
#include "tools.h"
#include "samgr_xcollie.h"

#ifdef WITH_SELINUX
#include "service_checker.h"
#define HILOG_SE_DEBUG(type, fmt, ...) HILOG_IMPL((type), LOG_DEBUG, LOG_DOMAIN, "SA_SELINUX", fmt, __VA_ARGS__)
#endif

namespace {
#ifdef WITH_SELINUX
    std::unique_ptr<ServiceChecker> selinuxChecker_ = std::make_unique<ServiceChecker>(false);
#endif

    bool CheckGetSAPermission(const int32_t said)
    {
#ifdef WITH_SELINUX
        int64_t begin = OHOS::GetTickCount();
        auto callingSid = OHOS::IPCSkeleton::GetCallingSid();
        auto ret = selinuxChecker_->GetServiceCheck(callingSid, std::to_string(said)) == 0;
        HILOG_SE_DEBUG(LOG_CORE, "GetServiceCheck callingSid:%{public}s,SA:%{public}d,ret:%{public}s,spend:%{public}"
            PRId64 "ms", callingSid.c_str(), said, ret == true ? "suc" : "fail", OHOS::GetTickCount() - begin);
        return  ret;
#else
        return true; // if not support selinux, not check selinux permission
#endif
    }

    bool CheckAddOrRemovePermission(const int32_t said)
    {
#ifdef WITH_SELINUX
        int64_t begin = OHOS::GetTickCount();
        auto callingSid = OHOS::IPCSkeleton::GetCallingSid();
        auto ret = selinuxChecker_->AddServiceCheck(callingSid, std::to_string(said)) == 0;
        HILOG_SE_DEBUG(LOG_CORE, "AddServiceCheck callingSid:%{public}s,SA:%{public}d,ret:%{public}s,spend:%{public}"
            PRId64 "ms", callingSid.c_str(), said, ret == true ? "suc" : "fail", OHOS::GetTickCount() - begin);
        return ret;
#else
        return true; // if not support selinux, not check selinux permission
#endif
    }

    bool CheckGetRemoteSAPermission(const int32_t said)
    {
#ifdef WITH_SELINUX
        int64_t begin = OHOS::GetTickCount();
        auto callingSid = OHOS::IPCSkeleton::GetCallingSid();
        auto ret = selinuxChecker_->GetRemoteServiceCheck(callingSid, std::to_string(said)) == 0;
        HILOG_SE_DEBUG(LOG_CORE, "GetRemoteServiceCheck callingSid:%{public}s,SA:%{public}d,"
            "ret:%{public}s,spend:%{public}" PRId64 "ms", callingSid.c_str(), said,
            ret == true ? "suc" : "fail", OHOS::GetTickCount() - begin);
        return ret;
#else
        return true; // if not support selinux, not check selinux permission
#endif
    }

    bool CheckListSAPermission()
    {
#ifdef WITH_SELINUX
        int64_t begin = OHOS::GetTickCount();
        auto callingSid = OHOS::IPCSkeleton::GetCallingSid();
        auto ret = selinuxChecker_->ListServiceCheck(callingSid) == 0;
        HILOG_SE_DEBUG(LOG_CORE, "ListServiceCheck callingSid:%{public}s,ret:%{public}s,spend:%{public}"
            PRId64 "ms", callingSid.c_str(), ret == true ? "suc" : "fail", OHOS::GetTickCount() - begin);
        return ret;
#else
        return true; // if not support selinux, not check selinux permission
#endif
    }
}

using namespace OHOS::Security;
namespace OHOS {
namespace {
constexpr const char *EXT_TRANSACTION_PERMISSION = "ohos.permission.ACCESS_EXT_SYSTEM_ABILITY";
}

void SystemAbilityManagerStub::SetAbilityFuncMap()
{
    memberFuncMap_[static_cast<uint32_t>(SamgrInterfaceCode::GET_SYSTEM_ABILITY_TRANSACTION)] =
        SystemAbilityManagerStub::LocalGetSystemAbility;
    memberFuncMap_[static_cast<uint32_t>(SamgrInterfaceCode::CHECK_SYSTEM_ABILITY_TRANSACTION)] =
        SystemAbilityManagerStub::LocalCheckSystemAbility;
    memberFuncMap_[static_cast<uint32_t>(SamgrInterfaceCode::ADD_SYSTEM_ABILITY_TRANSACTION)] =
        SystemAbilityManagerStub::LocalAddSystemAbility;
    memberFuncMap_[static_cast<uint32_t>(SamgrInterfaceCode::REMOVE_SYSTEM_ABILITY_TRANSACTION)] =
        SystemAbilityManagerStub::LocalRemoveSystemAbility;
    memberFuncMap_[static_cast<uint32_t>(SamgrInterfaceCode::LIST_SYSTEM_ABILITY_TRANSACTION)] =
        SystemAbilityManagerStub::LocalListSystemAbility;
    memberFuncMap_[static_cast<uint32_t>(SamgrInterfaceCode::SUBSCRIBE_SYSTEM_ABILITY_TRANSACTION)] =
        SystemAbilityManagerStub::LocalSubsSystemAbility;
    memberFuncMap_[static_cast<uint32_t>(SamgrInterfaceCode::CHECK_REMOTE_SYSTEM_ABILITY_TRANSACTION)] =
        SystemAbilityManagerStub::LocalCheckRemtSystemAbility;
    memberFuncMap_[static_cast<uint32_t>(SamgrInterfaceCode::ADD_ONDEMAND_SYSTEM_ABILITY_TRANSACTION)] =
        SystemAbilityManagerStub::LocalAddOndemandSystemAbility;
    memberFuncMap_[static_cast<uint32_t>(SamgrInterfaceCode::CHECK_SYSTEM_ABILITY_IMMEDIATELY_TRANSACTION)] =
        SystemAbilityManagerStub::LocalCheckSystemAbilityImme;
    memberFuncMap_[static_cast<uint32_t>(SamgrInterfaceCode::UNSUBSCRIBE_SYSTEM_ABILITY_TRANSACTION)] =
        SystemAbilityManagerStub::LocalUnSubsSystemAbility;
    memberFuncMap_[static_cast<uint32_t>(SamgrInterfaceCode::LOAD_SYSTEM_ABILITY_TRANSACTION)] =
        SystemAbilityManagerStub::LocalLoadSystemAbility;
    memberFuncMap_[static_cast<uint32_t>(SamgrInterfaceCode::LOAD_REMOTE_SYSTEM_ABILITY_TRANSACTION)] =
        SystemAbilityManagerStub::LocalLoadRemoteSystemAbility;
    memberFuncMap_[static_cast<uint32_t>(SamgrInterfaceCode::UNLOAD_SYSTEM_ABILITY_TRANSACTION)] =
        SystemAbilityManagerStub::LocalUnloadSystemAbility;
    memberFuncMap_[static_cast<uint32_t>(SamgrInterfaceCode::CANCEL_UNLOAD_SYSTEM_ABILITY_TRANSACTION)] =
        SystemAbilityManagerStub::LocalCancelUnloadSystemAbility;
}

void SystemAbilityManagerStub::SetProcessFuncMap()
{
    memberFuncMap_[static_cast<uint32_t>(SamgrInterfaceCode::ADD_SYSTEM_PROCESS_TRANSACTION)] =
        SystemAbilityManagerStub::LocalAddSystemProcess;
    memberFuncMap_[static_cast<uint32_t>(SamgrInterfaceCode::GET_SYSTEM_PROCESS_INFO_TRANSACTION)] =
        SystemAbilityManagerStub::LocalGetSystemProcessInfo;
    memberFuncMap_[static_cast<uint32_t>(SamgrInterfaceCode::GET_RUNNING_SYSTEM_PROCESS_TRANSACTION)] =
        SystemAbilityManagerStub::LocalGetRunningSystemProcess;
    memberFuncMap_[static_cast<uint32_t>(SamgrInterfaceCode::SUBSCRIBE_SYSTEM_PROCESS_TRANSACTION)] =
        SystemAbilityManagerStub::LocalSubscribeSystemProcess;
    memberFuncMap_[static_cast<uint32_t>(SamgrInterfaceCode::UNSUBSCRIBE_SYSTEM_PROCESS_TRANSACTION)] =
        SystemAbilityManagerStub::LocalUnSubscribeSystemProcess;
}

SystemAbilityManagerStub::SystemAbilityManagerStub()
{
    SetAbilityFuncMap();
    SetProcessFuncMap();
    memberFuncMap_[static_cast<uint32_t>(SamgrInterfaceCode::GET_ONDEMAND_REASON_EXTRA_DATA_TRANSACTION)] =
        SystemAbilityManagerStub::LocalGetOnDemandReasonExtraData;
    memberFuncMap_[static_cast<uint32_t>(SamgrInterfaceCode::GET_ONDEAMND_POLICY_TRANSACTION)] =
        SystemAbilityManagerStub::LocalGetOnDemandPolicy;
    memberFuncMap_[static_cast<uint32_t>(SamgrInterfaceCode::UPDATE_ONDEAMND_POLICY_TRANSACTION)] =
        SystemAbilityManagerStub::LocalUpdateOnDemandPolicy;
    memberFuncMap_[static_cast<uint32_t>(SamgrInterfaceCode::GET_ONDEMAND_SYSTEM_ABILITY_IDS_TRANSACTION)] =
        SystemAbilityManagerStub::LocalGetOnDemandSystemAbilityIds;
    memberFuncMap_[static_cast<uint32_t>(SamgrInterfaceCode::SEND_STRATEGY_TRANASACTION)] =
        SystemAbilityManagerStub::LocalSendStrategy;
    memberFuncMap_[static_cast<uint32_t>(SamgrInterfaceCode::UNLOAD_ALL_IDLE_SYSTEM_ABILITY_TRANSACTION)] =
        SystemAbilityManagerStub::LocalUnloadAllIdleSystemAbility;
    memberFuncMap_[static_cast<uint32_t>(SamgrInterfaceCode::GET_EXTENSION_SA_IDS_TRANSCATION)] =
        SystemAbilityManagerStub::LocalGetExtensionSaIds;
    memberFuncMap_[static_cast<uint32_t>(SamgrInterfaceCode::GET_EXTERNSION_SA_LIST_TRANSCATION)] =
        SystemAbilityManagerStub::LocalGetExtensionRunningSaList;
    memberFuncMap_[static_cast<uint32_t>(SamgrInterfaceCode::GET_SA_EXTENSION_INFO_TRANSCATION)] =
        SystemAbilityManagerStub::LocalGetRunningSaExtensionInfoList;
    memberFuncMap_[static_cast<uint32_t>(SamgrInterfaceCode::GET_COMMON_EVENT_EXTRA_ID_LIST_TRANSCATION)] =
        SystemAbilityManagerStub::LocalGetCommonEventExtraDataIdlist;
}

int32_t SystemAbilityManagerStub::OnRemoteRequest(uint32_t code,
    MessageParcel& data, MessageParcel& reply, MessageOption &option)
{
    HILOGD("SAMStub::OnReceived, code = %{public}u, callerPid = %{public}d",
        code, IPCSkeleton::GetCallingPid());
    Samgr::MemoryGuard cacheGuard;
    if (!EnforceInterceToken(data)) {
        HILOGE("SAMStub::OnReceived, code = %{public}u, check interfaceToken failed", code);
        return ERR_PERMISSION_DENIED;
    }
    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        return itFunc->second(this, data, reply);
    }
    HILOGW("SAMStub: default case, need check.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

bool SystemAbilityManagerStub::EnforceInterceToken(MessageParcel& data)
{
    std::u16string interfaceToken = data.ReadInterfaceToken();
    return interfaceToken == SAMANAGER_INTERFACE_TOKEN;
}

int32_t SystemAbilityManagerStub::ListSystemAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CanRequest()) {
        HILOGE("ListSystemAbilityInner PERMISSION DENIED!");
        return ERR_PERMISSION_DENIED;
    }

    if (!CheckListSAPermission()) {
        HILOGE("ListSystemAbilityInner selinux permission denied! callSid:%{public}s",
            OHOS::IPCSkeleton::GetCallingSid().c_str());
        return ERR_PERMISSION_DENIED;
    }

    int32_t dumpFlag = 0;
    bool ret = data.ReadInt32(dumpFlag);
    if (!ret) {
        HILOGW("ListSystemAbilityInner read dumpflag failed!");
        return ERR_FLATTEN_OBJECT;
    }

    std::vector<std::u16string> saNameVector = ListSystemAbilities(dumpFlag);
    if (saNameVector.empty()) {
        HILOGI("List System Abilities list errors");
        ret = reply.WriteInt32(ERR_INVALID_VALUE);
    } else {
        HILOGI("ListSystemAbilityInner list success");
        ret = reply.WriteInt32(ERR_NONE);
        if (ret) {
            ret = reply.WriteString16Vector(saNameVector);
        }
    }

    if (!ret) {
        HILOGW("ListSystemAbilityInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t SystemAbilityManagerStub::SubsSystemAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    int32_t systemAbilityId = -1;
    bool ret = data.ReadInt32(systemAbilityId);
    if (!ret) {
        return ERR_NULL_OBJECT;
    }
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("SubsSystemAbilityInner read SAId failed!");
        return ERR_NULL_OBJECT;
    }
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    if (remoteObject == nullptr) {
        HILOGW("SubsSystemAbilityInner read listener failed!");
        return ERR_NULL_OBJECT;
    }
    sptr<ISystemAbilityStatusChange> listener = iface_cast<ISystemAbilityStatusChange>(remoteObject);
    if (listener == nullptr) {
        HILOGW("SubsSystemAbilityInner iface_cast failed!");
        return ERR_NULL_OBJECT;
    }
    SamgrXCollie samgrXCollie("samgr--SubsSA_" + ToString(systemAbilityId));
    int32_t result = SubscribeSystemAbility(systemAbilityId, listener);
    HILOGD("SubsSystemAbilityInner result is %{public}d", result);
    ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGW("SubsSystemAbilityInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }

    return result;
}

int32_t SystemAbilityManagerStub::UnSubsSystemAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    int32_t systemAbilityId = -1;
    bool ret = data.ReadInt32(systemAbilityId);
    if (!ret) {
        return ERR_NULL_OBJECT;
    }
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("UnSubsSystemAbilityInner read SAId failed!");
        return ERR_NULL_OBJECT;
    }
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    if (remoteObject == nullptr) {
        HILOGW("UnSubscribeSystemAbility read listener failed!");
        return ERR_NULL_OBJECT;
    }
    sptr<ISystemAbilityStatusChange> listener = iface_cast<ISystemAbilityStatusChange>(remoteObject);
    if (listener == nullptr) {
        HILOGW("UnSubscribeSystemAbility iface_cast failed!");
        return ERR_NULL_OBJECT;
    }
    int32_t result = UnSubscribeSystemAbility(systemAbilityId, listener);
    HILOGD("UnSubscribeSystemAbility result is %{public}d", result);
    ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGW("UnSubscribeSystemAbility write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }

    return result;
}

int32_t SystemAbilityManagerStub::CheckRemtSystemAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CanRequest()) {
        HILOGE("CheckRemoteSystemAbilityInner PERMISSION DENIED!");
        return ERR_PERMISSION_DENIED;
    }
    int32_t systemAbilityId = -1;
    bool ret = data.ReadInt32(systemAbilityId);
    if (!ret) {
        return ERR_NULL_OBJECT;
    }
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("CheckRemtSystemAbilityInner read SAId failed!");
        return ERR_NULL_OBJECT;
    }

    if (!CheckGetRemoteSAPermission(systemAbilityId)) {
        HILOGE("CheckRemtSystemAbilityInner selinux permission denied! SA:%{public}d,callSid:%{public}s",
            systemAbilityId, OHOS::IPCSkeleton::GetCallingSid().c_str());
        return ERR_PERMISSION_DENIED;
    }

    std::string deviceId;
    ret = data.ReadString(deviceId);
    if (!ret) {
        HILOGW("CheckRemtSystemAbilityInner read deviceId failed!");
        return ERR_FLATTEN_OBJECT;
    }
    std::string uuid = SamgrUtil::TransformDeviceId(deviceId, UUID, false);
    sptr<IRemoteObject> remoteObject = GetSystemAbility(systemAbilityId, uuid);
    if (remoteObject == nullptr) {
        HILOGD("CheckRemtSystemAbilityInner SA:%{public}d GetSystemAbility failed.", systemAbilityId);
        return ERR_NULL_OBJECT;
    }
    ret = reply.WriteRemoteObject(remoteObject);
    if (!ret) {
        HILOGE("CheckRemtSystemAbilityInner SA:%{public}d write reply failed.", systemAbilityId);
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t SystemAbilityManagerStub::AddOndemandSystemAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CanRequest()) {
        HILOGE("AddOndemandSystemAbilityInner PERMISSION DENIED!");
        return ERR_PERMISSION_DENIED;
    }
    int32_t systemAbilityId = -1;
    bool ret = data.ReadInt32(systemAbilityId);
    if (!ret) {
        return ERR_NULL_OBJECT;
    }
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("AddOndemandSystemAbilityInner read SAId failed!");
        return ERR_NULL_OBJECT;
    }

    if (!CheckAddOrRemovePermission(systemAbilityId)) {
        HILOGE("AddOndemandSystemAbilityInner selinux permission denied! SA:%{public}d,callSid:%{public}s",
            systemAbilityId, OHOS::IPCSkeleton::GetCallingSid().c_str());
        return ERR_PERMISSION_DENIED;
    }

    std::u16string localManagerName = data.ReadString16();
    if (localManagerName.empty()) {
        HILOGW("AddOndemandSystemAbilityInner read localName failed!");
        return ERR_NULL_OBJECT;
    }

    int32_t result = AddOnDemandSystemAbilityInfo(systemAbilityId, localManagerName);
    HILOGD("AddOndemandSystemAbilityInner result is %{public}d", result);
    ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGW("AddOndemandSystemAbilityInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }

    return result;
}

int32_t SystemAbilityManagerStub::CheckSystemAbilityImmeInner(MessageParcel& data, MessageParcel& reply)
{
    int64_t begin = OHOS::GetTickCount();
    int32_t systemAbilityId = -1;
    bool ret = data.ReadInt32(systemAbilityId);
    if (!ret) {
        return ERR_NULL_OBJECT;
    }
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("CheckSystemAbilityImmeInner read SAId failed!");
        return ERR_NULL_OBJECT;
    }

    if (!CheckGetSAPermission(systemAbilityId)) {
        HILOGD("CheckSystemAbilityImmeInner selinux permission denied! SA:%{public}d,callSid:%{public}s",
            systemAbilityId, OHOS::IPCSkeleton::GetCallingSid().c_str());
        return ERR_PERMISSION_DENIED;
    }

    bool isExist = false;
    ret = data.ReadBool(isExist);
    if (!ret) {
        HILOGW("CheckSystemAbilityImmeInner read isExist failed!");
        return ERR_FLATTEN_OBJECT;
    }
    SamgrXCollie samgrXCollie("samgr--CheckSAImme_" + ToString(systemAbilityId));
    sptr<IRemoteObject> remoteObject = CheckSystemAbility(systemAbilityId, isExist);
    if (remoteObject == nullptr) {
        HILOGD("CheckSystemAbilityImmeInner SA:%{public}d CheckSystemAbility failed.", systemAbilityId);
        return ERR_NULL_OBJECT;
    }
    ret = reply.WriteRemoteObject(remoteObject);
    if (!ret) {
        HILOGE("CheckSystemAbilityImmeInner SA:%{public}d, callpid:%{public}d, write obj fail, spend %{public}"
            PRId64 " ms", systemAbilityId, OHOS::IPCSkeleton::GetCallingPid(), OHOS::GetTickCount() - begin);
        return ERR_FLATTEN_OBJECT;
    }

    ret = reply.WriteBool(isExist);
    if (!ret) {
        HILOGW("CheckSystemAbilityImmeInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t SystemAbilityManagerStub::UnmarshalingSaExtraProp(MessageParcel& data, SAExtraProp& extraProp)
{
    bool isDistributed = false;
    bool ret = data.ReadBool(isDistributed);
    if (!ret) {
        HILOGW("UnmarshalingSaExtraProp read isDistributed failed!");
        return ERR_FLATTEN_OBJECT;
    }

    int32_t dumpFlags = 0;
    ret = data.ReadInt32(dumpFlags);
    if (!ret || dumpFlags < 0) {
        HILOGW("UnmarshalingSaExtraProp dumpFlags failed!");
        return ERR_FLATTEN_OBJECT;
    }
    std::u16string capability = data.ReadString16();
    std::u16string permission = data.ReadString16();
    extraProp.isDistributed = isDistributed;
    extraProp.dumpFlags = static_cast<uint32_t>(dumpFlags);
    extraProp.capability = capability;
    extraProp.permission = permission;
    return ERR_OK;
}

int32_t SystemAbilityManagerStub::AddSystemAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CanRequest()) {
        KHILOGE("AddSystemAbilityInner PERMISSION DENIED!");
        return ERR_PERMISSION_DENIED;
    }
    int32_t systemAbilityId = -1;
    bool ret = data.ReadInt32(systemAbilityId);
    if (!ret) {
        return ERR_NULL_OBJECT;
    }
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        KHILOGW("AddSystemAbilityExtraInner read SAId failed!");
        return ERR_NULL_OBJECT;
    }

    if (!CheckAddOrRemovePermission(systemAbilityId)) {
        KHILOGE("AddSystemAbilityInner selinux permission denied! SA:%{public}d,callSid:%{public}s",
            systemAbilityId, OHOS::IPCSkeleton::GetCallingSid().c_str());
        return ERR_PERMISSION_DENIED;
    }

    auto object = data.ReadRemoteObject();
    if (object == nullptr) {
        KHILOGW("AddSystemAbilityExtraInner readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    SAExtraProp extraProp;
    int32_t result = UnmarshalingSaExtraProp(data, extraProp);
    if (result != ERR_OK) {
        KHILOGW("AddSystemAbilityExtraInner UnmarshalingSaExtraProp failed!");
        return result;
    }
    result = AddSystemAbility(systemAbilityId, object, extraProp);
    ret = reply.WriteInt32(result);
    if (!ret) {
        KHILOGW("AddSystemAbilityExtraInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

int32_t SystemAbilityManagerStub::GetSystemAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    int32_t systemAbilityId = -1;
    bool ret = data.ReadInt32(systemAbilityId);
    if (!ret) {
        return ERR_NULL_OBJECT;
    }
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("GetSystemAbilityInner read SAId failed!");
        return ERR_NULL_OBJECT;
    }

    if (!CheckGetSAPermission(systemAbilityId)) {
        HILOGE("GetSystemAbilityInner selinux permission denied! SA:%{public}d,callSid:%{public}s",
            systemAbilityId, OHOS::IPCSkeleton::GetCallingSid().c_str());
        return ERR_PERMISSION_DENIED;
    }
    sptr<IRemoteObject> remoteObject = GetSystemAbility(systemAbilityId);
    if (remoteObject == nullptr) {
        HILOGD("GetSystemAbilityInner SA:%{public}d GetSystemAbility failed.", systemAbilityId);
        return ERR_NULL_OBJECT;
    }
    ret = reply.WriteRemoteObject(remoteObject);
    if (!ret) {
        HILOGE("GetSystemAbilityInner SA:%{public}d write reply failed.", systemAbilityId);
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_NONE;
}

int32_t SystemAbilityManagerStub::CheckSystemAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    int32_t systemAbilityId = -1;
    bool ret = data.ReadInt32(systemAbilityId);
    if (!ret) {
        return ERR_NULL_OBJECT;
    }
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("CheckSystemAbilityInner read SAId failed!");
        return ERR_NULL_OBJECT;
    }

    if (!CheckGetSAPermission(systemAbilityId)) {
        HILOGD("CheckSystemAbilityInner selinux permission denied! SA:%{public}d,callSid:%{public}s",
            systemAbilityId, OHOS::IPCSkeleton::GetCallingSid().c_str());
        return ERR_PERMISSION_DENIED;
    }
    sptr<IRemoteObject> remoteObject = CheckSystemAbility(systemAbilityId);
    if (remoteObject == nullptr) {
        HILOGD("CheckSystemAbilityInner SA:%{public}d CheckSystemAbility failed.", systemAbilityId);
        return ERR_NULL_OBJECT;
    }
    ret = reply.WriteRemoteObject(remoteObject);
    if (!ret) {
        HILOGE("CheckSystemAbilityInner SA:%{public}d write reply failed.", systemAbilityId);
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_NONE;
}

int32_t SystemAbilityManagerStub::RemoveSystemAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CanRequest()) {
        HILOGE("RemoveSystemAbilityInner PERMISSION DENIED!");
        return ERR_PERMISSION_DENIED;
    }
    int32_t systemAbilityId = -1;
    bool ret = data.ReadInt32(systemAbilityId);
    if (!ret) {
        return ERR_NULL_OBJECT;
    }
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("RemoveSystemAbilityInner read SAId failed!");
        return ERR_NULL_OBJECT;
    }

    if (!CheckAddOrRemovePermission(systemAbilityId)) {
        HILOGE("RemoveSystemAbilityInner selinux permission denied! SA:%{public}d,callSid:%{public}s",
            systemAbilityId, OHOS::IPCSkeleton::GetCallingSid().c_str());
        return ERR_PERMISSION_DENIED;
    }

    int32_t result = RemoveSystemAbility(systemAbilityId);
    ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGW("RemoveSystemAbilityInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}


int32_t SystemAbilityManagerStub::AddSystemProcessInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CanRequest()) {
        HILOGE("AddSystemProcessInner PERMISSION DENIED!");
        return ERR_PERMISSION_DENIED;
    }
    std::u16string procName = data.ReadString16();
    if (procName.empty()) {
        HILOGW("AddSystemProcessInner read procName failed!");
        return ERR_NULL_OBJECT;
    }

    sptr<IRemoteObject> procObject = data.ReadRemoteObject();
    if (procObject == nullptr) {
        HILOGW("AddSystemProcessInner readParcelable failed!");
        return ERR_NULL_OBJECT;
    }

    int32_t result = AddSystemProcess(procName, procObject);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGW("AddSystemProcessInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

int32_t SystemAbilityManagerStub::LoadSystemAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    int32_t systemAbilityId = -1;
    bool ret = false;
    sptr<ISystemAbilityLoadCallback> callback = nullptr;
    {
        SamgrXCollie samgrXCollie("samgrStub--loadSa_readData");
        ret = data.ReadInt32(systemAbilityId);
        if (!ret) {
            HILOGW("LoadSystemAbilityInner read SAId failed!");
            return ERR_INVALID_VALUE;
        }
        std::string loadSystemAbilityTag = ToString(systemAbilityId) + "_LoadSystemAbility";
        HITRACE_METER_NAME(HITRACE_TAG_SAMGR, loadSystemAbilityTag);
        if (!CheckInputSysAbilityId(systemAbilityId)) {
            HILOGW("LoadSystemAbilityInner check SAId failed!");
            return ERR_INVALID_VALUE;
        }

        if (!CheckGetSAPermission(systemAbilityId)) {
            HILOGE("LoadSystemAbilityInner selinux permission denied!SA:%{public}d,callSid:%{public}s",
                systemAbilityId, OHOS::IPCSkeleton::GetCallingSid().c_str());
            return ERR_PERMISSION_DENIED;
        }

        sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
        if (remoteObject == nullptr) {
            HILOGW("LoadSystemAbilityInner read callback failed!");
            return ERR_INVALID_VALUE;
        }
        callback = iface_cast<ISystemAbilityLoadCallback>(remoteObject);
        if (callback == nullptr) {
            HILOGW("LoadSystemAbilityInner iface_cast failed!");
            return ERR_INVALID_VALUE;
        }
    }

    int32_t result = LoadSystemAbility(systemAbilityId, callback);
    if (result != ERR_OK) {
        ReportSamgrSaLoadFail(systemAbilityId, IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid(),
            "interface load err:" + ToString(result));
        HILOGE("loadSaInner fail ret:%{public}d", result);
    }
    HILOGD("LoadSystemAbilityInner result is %{public}d", result);
    {
        SamgrXCollie samgrXCollie("samgrStub--loadSa_writeResult_" + ToString(systemAbilityId));
        ret = reply.WriteInt32(result);
    }
    if (!ret) {
        HILOGW("LoadSystemAbilityInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

int32_t SystemAbilityManagerStub::LoadRemoteSystemAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    int32_t systemAbilityId = -1;
    bool ret = false;
    std::string deviceId = "";
    sptr<IRemoteObject> remoteObject = nullptr;
    sptr<ISystemAbilityLoadCallback> callback = nullptr;
    {
        SamgrXCollie samgrXCollie("samgrStub--loadRmtSa_readData");
        ret = data.ReadInt32(systemAbilityId);
        if (!ret) {
            HILOGW("LoadRemoteSystemAbilityInner read SAId invalid");
            return ERR_INVALID_VALUE;
        }
        if (!CheckInputSysAbilityId(systemAbilityId)) {
            HILOGW("LoadRemoteSystemAbilityInner check SAId invalid");
            return ERR_INVALID_VALUE;
        }

        if (!CheckGetRemoteSAPermission(systemAbilityId)) {
            HILOGE("LoadRemoteSystemAbilityInner selinux permission denied! SA:%{public}d,callSid:%{public}s",
                systemAbilityId, OHOS::IPCSkeleton::GetCallingSid().c_str());
            return ERR_PERMISSION_DENIED;
        }

        deviceId = data.ReadString();
        if (deviceId.empty()) {
            HILOGW("LoadRemoteSystemAbilityInner read deviceId failed");
            return ERR_INVALID_VALUE;
        }
        remoteObject = data.ReadRemoteObject();
        if (remoteObject == nullptr) {
            HILOGW("LoadRemoteSystemAbilityInner read callback failed!");
            return ERR_INVALID_VALUE;
        }
        callback = iface_cast<ISystemAbilityLoadCallback>(remoteObject);
        if (callback == nullptr) {
            HILOGW("LoadRemoteSystemAbilityInner iface_cast failed!");
            return ERR_INVALID_VALUE;
        }
    }

    int32_t result = LoadSystemAbility(systemAbilityId, deviceId, callback);
    HILOGD("LoadRemoteSystemAbilityInner result is %{public}d", result);
    {
        SamgrXCollie samgrXCollie("samgrStub--loadRmtSa_writeResult_" + ToString(systemAbilityId));
        ret = reply.WriteInt32(result);
    }
    if (!ret) {
        HILOGW("LoadRemoteSystemAbilityInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

int32_t SystemAbilityManagerStub::UnloadSystemAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    int32_t systemAbilityId = -1;
    bool ret = false;
    {
        SamgrXCollie samgrXCollie("samgrStub--unloadSa_readData");
        ret = data.ReadInt32(systemAbilityId);
        if (!ret) {
            HILOGW("UnloadSystemAbilityInner read SAId invalid");
            return ERR_INVALID_VALUE;
        }
        if (!CheckInputSysAbilityId(systemAbilityId)) {
            HILOGW("UnloadSystemAbilityInner check SAId invalid");
            return ERR_INVALID_VALUE;
        }
    }

    int32_t result = UnloadSystemAbility(systemAbilityId);
    if (result != ERR_OK) {
        ReportSaUnLoadFail(systemAbilityId, IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid(),
            "interface unload err:" + ToString(result));
        HILOGE("unloadSa fail ret:%{public}d", result);
    }
    HILOGD("UnloadSystemAbilityInner result is %{public}d", result);
    {
        SamgrXCollie samgrXCollie("samgrStub--unloadSa_writeResult_" + ToString(systemAbilityId));
        ret = reply.WriteInt32(result);
    }
    if (!ret) {
        HILOGW("UnloadSystemAbilityInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

int32_t SystemAbilityManagerStub::UnloadAllIdleSystemAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    int32_t result = UnloadAllIdleSystemAbility();
    HILOGI("UnloadAllIdleSystemAbilityInner result is %{public}d", result);
    return result;
}

int32_t SystemAbilityManagerStub::GetSystemProcessInfoInner(MessageParcel& data, MessageParcel& reply)
{
    HILOGI("GetSystemProcessInfoInner called");
    if (!CanRequest()) {
        HILOGE("GetSystemProcessInfoInner PERMISSION DENIED!");
        return ERR_PERMISSION_DENIED;
    }
    int32_t systemAbilityId = -1;
    bool ret = data.ReadInt32(systemAbilityId);
    if (!ret) {
        return ERR_NULL_OBJECT;
    }
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("GetSystemProcessInfoInner read SAId failed!");
        return ERR_NULL_OBJECT;
    }
    SystemProcessInfo processInfo;
    int32_t result = GetSystemProcessInfo(systemAbilityId, processInfo);
    ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGW("GetSystemProcessInfoInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }
    if (result != ERR_OK) {
        return ERR_OK;
    }

    ret = reply.WriteString(processInfo.processName);
    if (!ret) {
        HILOGW("GetSystemProcessInfoInner write procName failed.");
        return ERR_FLATTEN_OBJECT;
    }
    ret = reply.WriteInt32(processInfo.pid);
    if (!ret) {
        HILOGW("GetSystemProcessInfoInner write pid failed.");
        return ERR_FLATTEN_OBJECT;
    }
    ret = reply.WriteInt32(processInfo.uid);
    if (!ret) {
        HILOGW("GetSystemProcessInfoInner write uid failed.");
            return ERR_FLATTEN_OBJECT;
    }
    return ERR_OK;
}

int32_t SystemAbilityManagerStub::GetRunningSystemProcessInner(MessageParcel& data, MessageParcel& reply)
{
    HILOGI("GetRunningSystemProcessInner called");
    if (!CanRequest()) {
        HILOGE("GetRunningSystemProcessInner PERMISSION DENIED!");
        return ERR_PERMISSION_DENIED;
    }
    std::list<SystemProcessInfo> systemProcessInfos;
    int32_t result = GetRunningSystemProcess(systemProcessInfos);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGW("GetRunningSystemProcessInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }
    if (result != ERR_OK) {
        return ERR_OK;
    }

    size_t size = systemProcessInfos.size();
    ret = reply.WriteInt32(size);
    if (!ret) {
        HILOGW("GetRunningSystemProcessInner write ProcInfos size failed.");
        return ERR_FLATTEN_OBJECT;
    }
    for (auto& systemProcessInfo : systemProcessInfos) {
        ret = reply.WriteString(systemProcessInfo.processName);
        if (!ret) {
            HILOGW("GetRunningSystemProcessInner write procName failed.");
            return ERR_FLATTEN_OBJECT;
        }
        ret = reply.WriteInt32(systemProcessInfo.pid);
        if (!ret) {
            HILOGW("GetRunningSystemProcessInner write pid failed.");
            return ERR_FLATTEN_OBJECT;
        }
        ret = reply.WriteInt32(systemProcessInfo.uid);
        if (!ret) {
            HILOGW("GetRunningSystemProcessInner write uid failed.");
            return ERR_FLATTEN_OBJECT;
        }
    }
    return ERR_OK;
}

int32_t SystemAbilityManagerStub::SubscribeSystemProcessInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CanRequest()) {
        HILOGE("SubscribeSystemProcessInner PERMISSION DENIED!");
        return ERR_PERMISSION_DENIED;
    }
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    if (remoteObject == nullptr) {
        HILOGW("SubscribeSystemProcessInner read listener failed!");
        return ERR_NULL_OBJECT;
    }
    sptr<ISystemProcessStatusChange> listener = iface_cast<ISystemProcessStatusChange>(remoteObject);
    if (listener == nullptr) {
        HILOGW("SubscribeSystemProcessInner iface_cast failed!");
        return ERR_NULL_OBJECT;
    }
    int32_t result = SubscribeSystemProcess(listener);
    HILOGD("SubscribeSystemProcess result is %{public}d", result);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGW("SubscribeSystemProcessInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

int32_t SystemAbilityManagerStub::UnSubscribeSystemProcessInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CanRequest()) {
        HILOGE("UnSubscribeSystemProcessInner PERMISSION DENIED!");
        return ERR_PERMISSION_DENIED;
    }
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    if (remoteObject == nullptr) {
        HILOGW("UnSubscribeSystemProcessInner read listener failed!");
        return ERR_NULL_OBJECT;
    }
    sptr<ISystemProcessStatusChange> listener = iface_cast<ISystemProcessStatusChange>(remoteObject);
    if (listener == nullptr) {
        HILOGW("UnSubscribeSystemProcessInner iface_cast failed!");
        return ERR_NULL_OBJECT;
    }
    int32_t result = UnSubscribeSystemProcess(listener);
    HILOGD("UnSubscribeSystemProcessInner result is %{public}d", result);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGW("UnSubscribeSystemProcessInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

int32_t SystemAbilityManagerStub::CancelUnloadSystemAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    int32_t systemAbilityId = -1;
    bool ret = data.ReadInt32(systemAbilityId);
    if (!ret) {
        return ERR_INVALID_VALUE;
    }
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("CancelUnloadSystemAbilityInner SAId invalid");
        return ERR_INVALID_VALUE;
    }
    int32_t result = CancelUnloadSystemAbility(systemAbilityId);
    HILOGD("CancelUnloadSystemAbilityInner result is %{public}d", result);
    ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGW("CancelUnloadSystemAbilityInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

int32_t SystemAbilityManagerStub::GetOnDemandReasonExtraDataInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CanRequest()) {
        HILOGE("GetOnDemandReasonExtraData PERMISSION DENIED!");
        return ERR_PERMISSION_DENIED;
    }
    int64_t extraDataId = -1;
    if (!data.ReadInt64(extraDataId)) {
        HILOGW("GetOnDemandReasonExtraData read extraDataId failed.");
        return ERR_FLATTEN_OBJECT;
    }
    MessageParcel extraDataParcel;
    int32_t result = GetOnDemandReasonExtraData(extraDataId, extraDataParcel);
    HILOGD("GetOnDemandReasonExtraData result is %{public}d", result);
    if (!reply.WriteInt32(result)) {
        HILOGW("GetOnDemandReasonExtraData write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }
    sptr<OnDemandReasonExtraData> extraData;
    extraData = extraDataParcel.ReadParcelable<OnDemandReasonExtraData>();
    if (extraData == nullptr) {
        HILOGW("GetOnDemandReasonExtraData read extraData failed.");
        return ERR_FLATTEN_OBJECT;
    }
    if (!reply.WriteParcelable(extraData)) {
        HILOGW("GetOnDemandReasonExtraData write extraData failed.");
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_OK;
}

int32_t SystemAbilityManagerStub::GetOnDemandPolicyInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CanRequest()) {
        HILOGE("GetOnDemandPolicyInner PERMISSION DENIED!");
        return ERR_PERMISSION_DENIED;
    }
    int32_t systemAbilityId = -1;
    if (!data.ReadInt32(systemAbilityId)) {
        HILOGW("GetOnDemandPolicyInner read saId failed.");
        return ERR_FLATTEN_OBJECT;
    }
    int32_t type = 0;
    if (!data.ReadInt32(type)) {
        HILOGW("GetOnDemandPolicyInner read type failed.");
        return ERR_FLATTEN_OBJECT;
    }
    OnDemandPolicyType typeEnum = static_cast<OnDemandPolicyType>(type);
    std::vector<SystemAbilityOnDemandEvent> abilityOnDemandEvents;
    int32_t result = GetOnDemandPolicy(systemAbilityId, typeEnum, abilityOnDemandEvents);
    if (!reply.WriteInt32(result)) {
        HILOGW("GetOnDemandPolicyInner write result failed.");
        return ERR_FLATTEN_OBJECT;
    }
    if (!OnDemandEventToParcel::WriteOnDemandEventsToParcel(abilityOnDemandEvents, reply)) {
        HILOGW("GetOnDemandPolicyInner write on demand event failed.");
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_OK;
}

int32_t SystemAbilityManagerStub::UpdateOnDemandPolicyInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CanRequest()) {
        HILOGE("UpdateOnDemandPolicyInner PERMISSION DENIED!");
        return ERR_PERMISSION_DENIED;
    }
    int32_t systemAbilityId = -1;
    if (!data.ReadInt32(systemAbilityId)) {
        HILOGW("UpdateOnDemandPolicyInner read saId failed.");
        return ERR_FLATTEN_OBJECT;
    }
    int32_t type = 0;
    if (!data.ReadInt32(type)) {
        HILOGW("GetOnDemandPolicyInner read type failed.");
        return ERR_FLATTEN_OBJECT;
    }
    OnDemandPolicyType typeEnum = static_cast<OnDemandPolicyType>(type);
    std::vector<SystemAbilityOnDemandEvent> abilityOnDemandEvents;
    if (!OnDemandEventToParcel::ReadOnDemandEventsFromParcel(abilityOnDemandEvents, data)) {
        HILOGW("UpdateOnDemandPolicyInner read on demand event failed.");
        return ERR_FLATTEN_OBJECT;
    }
    int32_t result = UpdateOnDemandPolicy(systemAbilityId, typeEnum, abilityOnDemandEvents);
    if (!reply.WriteInt32(result)) {
        HILOGW("UpdateOnDemandPolicyInner write result failed.");
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_OK;
}

int32_t SystemAbilityManagerStub::SendStrategyInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CanRequest()) {
        HILOGE("SendStrategy PERMISSION DENIED!");
        return ERR_PERMISSION_DENIED;
    }
    int32_t type = 0;
    if (!data.ReadInt32(type)) {
        HILOGW("SendStrategy read type failed.");
        return ERR_FLATTEN_OBJECT;
    }
    std::vector<int32_t> systemAbilityIds;
    if (!data.ReadInt32Vector(&systemAbilityIds)) {
        HILOGW("SendStrategy read saId failed.");
        return ERR_FLATTEN_OBJECT;
    }
    int32_t level = -1;
    if (!data.ReadInt32(level)) {
        HILOGW("SendStrategy read level failed.");
        return ERR_FLATTEN_OBJECT;
    }
    std::string action;
    if (!data.ReadString(action)) {
        HILOGW("SendStrategy read action failed!");
        return ERR_FLATTEN_OBJECT;
    }
    int32_t result = SendStrategy(type, systemAbilityIds, level, action);
    if (!reply.WriteInt32(result)) {
        HILOGW("SendStrategy write result failed.");
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_OK;
}

int32_t SystemAbilityManagerStub::GetOnDemandSystemAbilityIdsInner(MessageParcel& data, MessageParcel& reply)
{
    std::vector<int32_t> systemAbilityIds;
    int32_t result = GetOnDemandSystemAbilityIds(systemAbilityIds);
    if (result != ERR_OK) {
        HILOGW("GetOnDemandSystemAbilityIds failed, ret:%{public}d", result);
        return result;
    }
    if (!reply.WriteInt32(result)) {
        HILOGE("GetOnDemandSystemAbilityIdsInner write result failed.");
        return ERR_FLATTEN_OBJECT;
    }
    if (!reply.WriteInt32Vector(systemAbilityIds)) {
        HILOGE("GetOnDemandSystemAbilityIdsInner write result failed.");
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_OK;
}

bool SystemAbilityManagerStub::CanRequest()
{
    auto tid = IPCSkeleton::GetCallingTokenID();
    AccessToken::ATokenTypeEnum tokenType = AccessToken::AccessTokenKit::GetTokenTypeFlag(tid);
    HILOGD("CanRequest tid:%{private}u, tokenType:%{public}d",
        tid, tokenType);
    return (tokenType == AccessToken::ATokenTypeEnum::TOKEN_NATIVE);
}

bool SystemAbilityManagerStub::CheckPermission(const std::string& permission)
{
    uint32_t accessToken = IPCSkeleton::GetCallingTokenID();
    int32_t ret = Security::AccessToken::AccessTokenKit::VerifyAccessToken(accessToken, permission);
    return (ret == Security::AccessToken::PermissionState::PERMISSION_GRANTED);
}

int32_t SystemAbilityManagerStub::GetExtensionSaIdsInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CanRequest()) {
        HILOGE("%{public}s  PERMISSION DENIED!", __func__);
        return ERR_PERMISSION_DENIED;
    }
    std::string extension;
    if (!data.ReadString(extension)) {
        HILOGW("%{public}s read extension failed!", __func__);
        return ERR_FLATTEN_OBJECT;
    }

    std::vector<int32_t> saIds;
    int32_t result = GetExtensionSaIds(extension, saIds);
    if (result != ERR_OK) {
        HILOGW("GetExtensionSaIds failed, ret:%{public}d", result);
        return result;
    }
    if (!reply.WriteInt32(result)) {
        HILOGW("%{public}s write reply failed.", __func__);
        return ERR_FLATTEN_OBJECT;
    }
    if (!reply.WriteInt32Vector(saIds)) {
        HILOGW("%{public}s write saids reply failed.", __func__);
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t SystemAbilityManagerStub::GetExtensionRunningSaListInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CanRequest()) {
        HILOGE("%{public}s PERMISSION DENIED!", __func__);
        return ERR_PERMISSION_DENIED;
    }
    std::string extension;
    if (!data.ReadString(extension)) {
        HILOGW("%{public}s read extension failed!", __func__);
        return ERR_FLATTEN_OBJECT;
    }

    std::vector<sptr<IRemoteObject>> saList;
    int32_t result = GetExtensionRunningSaList(extension, saList);
    if (result != ERR_OK) {
        HILOGW("GetExtensionRunningSaList failed, ret:%{public}d", result);
        return result;
    }
    if (!reply.WriteInt32(result)) {
        HILOGW("%{public}s write reply failed.", __func__);
        return ERR_FLATTEN_OBJECT;
    }
    if (!reply.WriteInt32(saList.size())) {
        HILOGW("%{public}s write saHandle size failed.", __func__);
        return ERR_FLATTEN_OBJECT;
    }
    for (auto& remoteObject : saList) {
        if (!reply.WriteRemoteObject(remoteObject)) {
            HILOGW("%{public}s write remote obj failed.", __func__);
            return ERR_FLATTEN_OBJECT;
        }
    }

    return ERR_NONE;
}

int32_t SystemAbilityManagerStub::GetRunningSaExtensionInfoListInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CheckPermission(EXT_TRANSACTION_PERMISSION)) {
        HILOGE("get SaExtInfoList CheckPermission fail!");
        return ERR_PERMISSION_DENIED;
    }
    std::string extension;
    if (!data.ReadString(extension)) {
        HILOGE("get SaExtInfoList read extension failed!");
        return ERR_FLATTEN_OBJECT;
    }

    std::vector<SaExtensionInfo> infoList;
    int32_t result = GetRunningSaExtensionInfoList(extension, infoList);
    if (!reply.WriteInt32(result)) {
        HILOGE("get SaExtInfoList write ret failed.");
        return ERR_FLATTEN_OBJECT;
    }
    if (result != ERR_OK) {
        HILOGE("get SaExtInfoList failed,ret:%{public}d", result);
        return result;
    }
    if (!reply.WriteInt32(infoList.size())) {
        HILOGE("get SaExtInfoList write size failed.");
        return ERR_FLATTEN_OBJECT;
    }
    for (auto& tmp : infoList) {
        if (!reply.WriteInt32(tmp.saId)) {
            HILOGE("get SaExtInfoList write said failed.");
            return ERR_FLATTEN_OBJECT;
        }
        if (!reply.WriteRemoteObject(tmp.processObj)) {
            HILOGE("get SaExtInfoList write obj failed.");
            return ERR_FLATTEN_OBJECT;
        }
    }
    return ERR_NONE;
}

int32_t SystemAbilityManagerStub::GetCommonEventExtraDataIdlistInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CanRequest()) {
        HILOGE("getExtraIdList PERMISSION DENIED!");
        return ERR_PERMISSION_DENIED;
    }
    int32_t saId = -1;
    if (!data.ReadInt32(saId)) {
        HILOGE("getExtraIdList read said fail!");
        return ERR_NULL_OBJECT;
    }
    if (!CheckInputSysAbilityId(saId)) {
        HILOGW("getExtraIdList check SAId failed!");
        return ERR_NULL_OBJECT;
    }
    std::string eventName;
    if (!data.ReadString(eventName)) {
        HILOGW("getExtraIdList read eventName failed!");
        return ERR_FLATTEN_OBJECT;
    }

    std::vector<int64_t> extraDataIdList;
    int32_t result = GetCommonEventExtraDataIdlist(saId, extraDataIdList, eventName);
    if (!reply.WriteInt32(result)) {
        HILOGW("getExtraIdList write result failed.");
        return ERR_FLATTEN_OBJECT;
    }
    if (result != ERR_OK) {
        HILOGE("getExtraIdList failed,ret:%{public}d", result);
        return result;
    }
    if (!reply.WriteInt64Vector(extraDataIdList)) {
        HILOGW("getExtraIdList write idlist failed.");
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_NONE;
}
} // namespace OHOS
