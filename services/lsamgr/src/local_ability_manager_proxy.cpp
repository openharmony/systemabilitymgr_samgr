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

#include "local_ability_manager_proxy.h"

#include "ipc_types.h"
#include "iremote_object.h"
#include "message_option.h"
#include "message_parcel.h"
#include "nlohmann/json.hpp"
#include "refbase.h"

using namespace std;
using namespace OHOS::HiviewDFX;

namespace OHOS {
#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD001800
#undef LOG_TAG
#define LOG_TAG "SA"
bool LocalAbilityManagerProxy::StartAbility(int32_t systemAbilityId, const std::string& eventStr)
{
    if (systemAbilityId <= 0) {
        HILOG_WARN(LOG_CORE, "StartAbility systemAbilityId invalid.");
        return false;
    }

    if (eventStr.empty()) {
        HILOG_WARN(LOG_CORE, "StartAbility eventStr invalid.");
        return false;
    }

    sptr<IRemoteObject> iro = Remote();
    if (iro == nullptr) {
        HILOG_ERROR(LOG_CORE, "StartAbility Remote return null");
        return false;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(LOCAL_ABILITY_MANAGER_INTERFACE_TOKEN)) {
        HILOG_WARN(LOG_CORE, "StartAbility interface token check failed");
        return false;
    }
    bool ret = data.WriteInt32(systemAbilityId);
    if (!ret) {
        HILOG_WARN(LOG_CORE, "StartAbility write systemAbilityId failed!");
        return false;
    }
    ret = data.WriteString(eventStr);
    if (!ret) {
        HILOG_WARN(LOG_CORE, "StartAbility write eventStr failed!");
        return false;
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int32_t status = iro->SendRequest(
        static_cast<uint32_t>(SafwkInterfaceCode::START_ABILITY_TRANSACTION), data, reply, option);
    if (status != NO_ERROR) {
        HILOG_ERROR(LOG_CORE, "StartAbility SendRequest failed, return value : %{public}d", status);
        return false;
    }
    return true;
}

bool LocalAbilityManagerProxy::StopAbility(int32_t systemAbilityId, const std::string& eventStr)
{
    if (systemAbilityId <= 0) {
        HILOG_WARN(LOG_CORE, "StopAbility systemAbilityId invalid.");
        return false;
    }

    if (eventStr.empty()) {
        HILOG_WARN(LOG_CORE, "StartAbility eventStr invalid.");
        return false;
    }
    
    sptr<IRemoteObject> iro = Remote();
    if (iro == nullptr) {
        HILOG_ERROR(LOG_CORE, "StopAbility Remote return null");
        return false;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(LOCAL_ABILITY_MANAGER_INTERFACE_TOKEN)) {
        HILOG_WARN(LOG_CORE, "StopAbility interface token check failed");
        return false;
    }
    bool ret = data.WriteInt32(systemAbilityId);
    if (!ret) {
        HILOG_WARN(LOG_CORE, "StopAbility write systemAbilityId failed!");
        return false;
    }
    ret = data.WriteString(eventStr);
    if (!ret) {
        HILOG_WARN(LOG_CORE, "StopAbility write eventStr failed!");
        return false;
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int32_t status = iro->SendRequest(
        static_cast<uint32_t>(SafwkInterfaceCode::STOP_ABILITY_TRANSACTION), data, reply, option);
    if (status != NO_ERROR) {
        HILOG_ERROR(LOG_CORE, "StopAbility SendRequest failed, return value : %{public}d", status);
        return false;
    }
    return true;
}

bool LocalAbilityManagerProxy::ActiveAbility(int32_t systemAbilityId,
    const nlohmann::json& activeReason)
{
    if (systemAbilityId <= 0) {
        HILOG_WARN(LOG_CORE, "ActiveAbility systemAbilityId invalid.");
        return false;
    }

    sptr<IRemoteObject> iro = Remote();
    if (iro == nullptr) {
        HILOG_ERROR(LOG_CORE, "ActiveAbility Remote return null");
        return false;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(LOCAL_ABILITY_MANAGER_INTERFACE_TOKEN)) {
        HILOG_WARN(LOG_CORE, "ActiveAbility interface token check failed");
        return false;
    }
    if (!data.WriteInt32(systemAbilityId)) {
        HILOG_WARN(LOG_CORE, "ActiveAbility write systemAbilityId failed!");
        return false;
    }
    if (!data.WriteString(activeReason.dump())) {
        HILOG_WARN(LOG_CORE, "ActiveAbility write activeReason failed!");
        return false;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t status = iro->SendRequest(
        static_cast<uint32_t>(SafwkInterfaceCode::ACTIVE_ABILITY_TRANSACTION), data, reply, option);
    if (status != NO_ERROR) {
        HILOG_ERROR(LOG_CORE, "ActiveAbility SendRequest failed, return value : %{public}d", status);
        return false;
    }
    bool result = false;
    if (!reply.ReadBool(result)) {
        HILOG_WARN(LOG_CORE, "ActiveAbility read result failed!");
        return false;
    }
    return result;
}

bool LocalAbilityManagerProxy::IdleAbility(int32_t systemAbilityId,
    const nlohmann::json& idleReason, int32_t& delayTime)
{
    if (systemAbilityId <= 0) {
        HILOG_WARN(LOG_CORE, "IdleAbility systemAbilityId invalid.");
        return false;
    }

    sptr<IRemoteObject> iro = Remote();
    if (iro == nullptr) {
        HILOG_ERROR(LOG_CORE, "IdleAbility Remote return null");
        return false;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(LOCAL_ABILITY_MANAGER_INTERFACE_TOKEN)) {
        HILOG_WARN(LOG_CORE, "IdleAbility interface token check failed");
        return false;
    }
    if (!data.WriteInt32(systemAbilityId)) {
        HILOG_WARN(LOG_CORE, "IdleAbility write systemAbilityId failed!");
        return false;
    }
    if (!data.WriteString(idleReason.dump())) {
        HILOG_WARN(LOG_CORE, "IdleAbility write ildeReason failed!");
        return false;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t status = iro->SendRequest(
        static_cast<uint32_t>(SafwkInterfaceCode::IDLE_ABILITY_TRANSACTION), data, reply, option);
    if (status != NO_ERROR) {
        HILOG_ERROR(LOG_CORE, "IdleAbility SendRequest failed, return value : %{public}d", status);
        return false;
    }
    bool result = false;
    if (!reply.ReadBool(result)) {
        HILOG_WARN(LOG_CORE, "IdleAbility read result failed!");
        return false;
    }
    if (!reply.ReadInt32(delayTime)) {
        HILOG_WARN(LOG_CORE, "IdleAbility read delayTime failed!");
        return false;
    }
    return result;
}

bool LocalAbilityManagerProxy::SendStrategyToSA(int32_t type, int32_t systemAbilityId,
    int32_t level, std::string& action)
{
    if (systemAbilityId <= 0) {
        HILOG_WARN(LOG_CORE, "SendStrategy systemAbilityId invalid.");
        return false;
    }

    sptr<IRemoteObject> iro = Remote();
    if (iro == nullptr) {
        HILOG_ERROR(LOG_CORE, "SendStrategy Remote return null");
        return false;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(LOCAL_ABILITY_MANAGER_INTERFACE_TOKEN)) {
        HILOG_WARN(LOG_CORE, "SendStrategy interface token check failed");
        return false;
    }
    bool ret = data.WriteInt32(type);
    if (!ret) {
        HILOG_WARN(LOG_CORE, "SendStrategy write type failed!");
        return false;
    }
    ret = data.WriteInt32(systemAbilityId);
    if (!ret) {
        HILOG_WARN(LOG_CORE, "SendStrategy write systemAbilityId failed!");
        return false;
    }
    ret = data.WriteInt32(level);
    if (!ret) {
        HILOG_WARN(LOG_CORE, "SendStrategy write level failed!");
        return false;
    }
    ret = data.WriteString(action);
    if (!ret) {
        HILOG_WARN(LOG_CORE, "SendStrategy write action failed!");
        return false;
    }
    
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int32_t status = iro->SendRequest(
        static_cast<uint32_t>(SafwkInterfaceCode::SEND_STRATEGY_TO_SA_TRANSACTION), data, reply, option);
    if (status != NO_ERROR) {
        HILOG_ERROR(LOG_CORE, "SendStrategy SendRequest failed, return value : %{public}d", status);
        return false;
    }
    bool result = false;
    if (!reply.ReadBool(result)) {
        HILOG_WARN(LOG_CORE, "SendStrategy read result failed!");
        return false;
    }
    return result;
}

bool LocalAbilityManagerProxy::IpcStatCmdProc(int32_t fd, int32_t cmd)
{
    sptr<IRemoteObject> iro = Remote();
    if (iro == nullptr) {
        HILOG_ERROR(LOG_CORE, "IpcStatCmdProc Remote null");
        return false;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(LOCAL_ABILITY_MANAGER_INTERFACE_TOKEN)) {
        HILOG_WARN(LOG_CORE, "IpcStatCmdProc interface token check failed");
        return false;
    }

    if (!data.WriteFileDescriptor(fd)) {
        HILOG_WARN(LOG_CORE, "IpcStatCmdProc write fd failed");
        return false;
    }

    if (!data.WriteInt32(cmd)) {
        HILOG_WARN(LOG_CORE, "IpcStatCmdProc write cmd faild");
        return false;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t status = iro->SendRequest(
        static_cast<uint32_t>(SafwkInterfaceCode::IPC_STAT_CMD_TRANSACTION), data, reply, option);
    if (status != NO_ERROR) {
        HILOG_ERROR(LOG_CORE, "IpcStatCmdProc SendRequest failed, return value : %{public}d", status);
        return false;
    }
    return true;
}
}
