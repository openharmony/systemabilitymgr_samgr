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

#include "system_ability_manager_proxy.h"

#include <condition_variable>
#include <unistd.h>
#include <vector>
#include <dlfcn.h>

#include "errors.h"
#include "ipc_types.h"
#include "iremote_object.h"
#include "isystem_ability_load_callback.h"
#include "isystem_ability_status_change.h"
#include "message_option.h"
#include "message_parcel.h"
#include "refbase.h"
#include "sam_log.h"
#include "string_ex.h"

#include "local_abilitys.h"
#include "system_ability_load_callback_stub.h"

using namespace std;
namespace OHOS {
namespace {
const int32_t MAX_TIMEOUT = 4;
const int32_t MIN_TIMEOUT = 0;
const int32_t RETRY_TIME_OUT_NUMBER = 6;
const int32_t SLEEP_INTERVAL_TIME = 200;
const int32_t GET_SYSTEM_ABILITY_CODE = 1;
const int32_t CHECK_SYSTEM_ABILITY_CODE = 2;
const int32_t SLEEP_ONE_MILLI_SECOND_TIME = 1000;
constexpr const char* PARAM_KEY = "persist.samgr.cache.sa";
}
class SystemAbilityProxyCallback : public SystemAbilityLoadCallbackStub {
public:
    void OnLoadSystemAbilitySuccess(int32_t systemAbilityId,
        const sptr<IRemoteObject> &remoteObject) override;
    void OnLoadSystemAbilityFail(int32_t systemAbilityId) override;
    std::mutex callbackLock_;
    std::condition_variable cv_;
    sptr<IRemoteObject> loadproxy_;
};

static void* g_selfSoHandle = nullptr;

extern "C" __attribute__((constructor)) void InitSamgrProxy()
{
    if (g_selfSoHandle != nullptr) {
        return;
    }
    Dl_info info;
    int ret = dladdr(reinterpret_cast<void *>(InitSamgrProxy), &info);
    if (ret == 0) {
        HILOGE("InitSamgrProxy dladdr fail");
        return;
    }

    char path[PATH_MAX] = {'\0'};
    if (realpath(info.dli_fname, path) == nullptr) {
        HILOGE("InitSamgrProxy realpath fail");
        return;
    }
    std::vector<std::string> strVector;
    SplitStr(path, "/", strVector);
    auto vectorSize = strVector.size();
    if (vectorSize == 0) {
        HILOGE("InitSamgrProxy SplitStr fail");
        return;
    }
    auto& fileName = strVector[vectorSize - 1];
    g_selfSoHandle = dlopen(fileName.c_str(), RTLD_LAZY);
    if (g_selfSoHandle == nullptr) {
        HILOGE("InitSamgrProxy dlopen fail");
        return;
    }
    HILOGD("InitSamgrProxy::done");
}

void SystemAbilityProxyCallback::OnLoadSystemAbilitySuccess(
    int32_t systemAbilityId, const sptr<IRemoteObject> &remoteObject)
{
    std::lock_guard<std::mutex> lock(callbackLock_);
    loadproxy_ = remoteObject;
    cv_.notify_one();
    HILOGI("LoadSystemAbility on load SA:%{public}d success!", systemAbilityId);
}

void SystemAbilityProxyCallback::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    std::lock_guard<std::mutex> lock(callbackLock_);
    loadproxy_ = nullptr;
    cv_.notify_one();
    HILOGI("LoadSystemAbility on load SA:%{public}d failed!", systemAbilityId);
}

sptr<IRemoteObject> SystemAbilityManagerProxy::GetSystemAbility(int32_t systemAbilityId)
{
    if (IsOnDemandSystemAbility(systemAbilityId)) {
        return GetSystemAbilityWrapper(systemAbilityId);
    }

    bool ret = SetKey(PARAM_KEY);
    if (!ret) {
        return GetSystemAbilityWrapper(systemAbilityId);
    }
    return QueryResult(systemAbilityId, GET_SYSTEM_ABILITY_CODE);
}

bool SystemAbilityManagerProxy::IsOnDemandSystemAbility(int32_t systemAbilityId)
{
    {
        std::lock_guard<std::mutex> autoLock(onDemandSaLock_);
        if (!onDemandSystemAbilityIdsSet_.empty()) {
            auto pos = onDemandSystemAbilityIdsSet_.find(systemAbilityId);
            if (pos != onDemandSystemAbilityIdsSet_.end()) {
                return true;
            }
            return false;
        }
    }
    std::vector<int32_t> onDemandSystemAbilityIds;
    GetOnDemandSystemAbilityIds(onDemandSystemAbilityIds);
    {
        std::lock_guard<std::mutex> autoLock(onDemandSaLock_);
        for (auto onDemandSystemAbilityId : onDemandSystemAbilityIds) {
            onDemandSystemAbilityIdsSet_.insert(onDemandSystemAbilityId);
        }

        auto pos = onDemandSystemAbilityIdsSet_.find(systemAbilityId);
        if (pos != onDemandSystemAbilityIdsSet_.end()) {
            return true;
        }
        return false;
    }
}

sptr<IRemoteObject> SystemAbilityManagerProxy::Recompute(int32_t systemAbilityId, int32_t code)
{
    ClearCache();
    if (code == GET_SYSTEM_ABILITY_CODE) {
        return GetSystemAbilityWrapper(systemAbilityId);
    }
    return CheckSystemAbilityTransaction(systemAbilityId);
}

sptr<IRemoteObject> SystemAbilityManagerProxy::GetSystemAbility(int32_t systemAbilityId,
    const std::string& deviceId)
{
    return GetSystemAbilityWrapper(systemAbilityId, deviceId);
}

sptr<IRemoteObject> SystemAbilityManagerProxy::GetSystemAbilityWrapper(int32_t systemAbilityId, const string& deviceId)
{
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("GetSaWrap SA invalid:%{public}d!", systemAbilityId);
        return nullptr;
    }

    bool isExist = false;
    int32_t timeout = RETRY_TIME_OUT_NUMBER;
    HILOGD("GetSaWrap:Waiting for SA:%{public}d, ", systemAbilityId);
    do {
        sptr<IRemoteObject> svc;
        int32_t errCode = ERR_NONE;
        if (deviceId.empty()) {
            svc = CheckSystemAbility(systemAbilityId, isExist, errCode);
            if (errCode == ERR_PERMISSION_DENIED) {
                HILOGE("GetSaWrap SA:%{public}d selinux denied", systemAbilityId);
                return nullptr;
            }
            if (!isExist) {
                HILOGD("%{public}s:SA:%{public}d is not exist", __func__, systemAbilityId);
            }
        } else {
            svc = CheckSystemAbility(systemAbilityId, deviceId, errCode);
            if (errCode == ERR_PERMISSION_DENIED) {
                HILOGE("GetSaWrap SA:%{public}d deviceId selinux denied", systemAbilityId);
                return nullptr;
            }
        }

        if (svc != nullptr) {
            return svc;
        }
        if (timeout > 0) {
            usleep(SLEEP_ONE_MILLI_SECOND_TIME * SLEEP_INTERVAL_TIME);
        }
    } while (timeout--);
    HILOGE("GetSaWrap SA:%{public}d not start", systemAbilityId);
    return nullptr;
}

sptr<IRemoteObject> SystemAbilityManagerProxy::CheckSystemAbilityWrapper(int32_t code, MessageParcel& data)
{
    int32_t errCode = ERR_NONE;
    return CheckSystemAbilityWrapper(code, data, errCode);
}

sptr<IRemoteObject> SystemAbilityManagerProxy::CheckSystemAbilityWrapper(int32_t code, MessageParcel& data,
    int32_t& errCode)
{
    auto remote = Remote();
    if (remote == nullptr) {
        HILOGI("CheckSaWrap remote is nullptr !");
        return nullptr;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(code, data, reply, option);
    if (err != ERR_NONE) {
        errCode = err;
        return nullptr;
    }
    return reply.ReadRemoteObject();
}

sptr<IRemoteObject> SystemAbilityManagerProxy::CheckSystemAbility(int32_t systemAbilityId)
{
    HILOGD("%{public}s called", __func__);
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("SA:%{public}d invalid!", systemAbilityId);
        return nullptr;
    }

    if (IsOnDemandSystemAbility(systemAbilityId)) {
        return CheckSystemAbilityTransaction(systemAbilityId);
    }

    bool ret = SetKey(PARAM_KEY);
    if (!ret) {
        return CheckSystemAbilityTransaction(systemAbilityId);
    }
    return QueryResult(systemAbilityId, CHECK_SYSTEM_ABILITY_CODE);
}

sptr<IRemoteObject> SystemAbilityManagerProxy::CheckSystemAbilityTransaction(int32_t systemAbilityId)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        return nullptr;
    }
    bool ret = data.WriteInt32(systemAbilityId);
    if (!ret) {
        HILOGW("CheckSystemAbility Write SAId failed!");
        return nullptr;
    }
    return CheckSystemAbilityWrapper(
        static_cast<uint32_t>(SamgrInterfaceCode::CHECK_SYSTEM_ABILITY_TRANSACTION), data);
}

sptr<IRemoteObject> SystemAbilityManagerProxy::CheckSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    int32_t errCode = ERR_NONE;
    return CheckSystemAbility(systemAbilityId, deviceId, errCode);
}

sptr<IRemoteObject> SystemAbilityManagerProxy::CheckSystemAbility(int32_t systemAbilityId, const std::string& deviceId,
    int32_t& errCode)
{
    if (!CheckInputSysAbilityId(systemAbilityId) || deviceId.empty()) {
        HILOGW("CheckSystemAbility:SA:%{public}d or deviceId is nullptr.", systemAbilityId);
        return nullptr;
    }

    HILOGD("CheckSystemAbility: SA:%{public}d.", systemAbilityId);

    auto remote = Remote();
    if (remote == nullptr) {
        HILOGE("CheckSystemAbility remote is nullptr !");
        return nullptr;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        return nullptr;
    }
    bool ret = data.WriteInt32(systemAbilityId);
    if (!ret) {
        HILOGE("CheckSystemAbility parcel write name failed");
        return nullptr;
    }
    ret = data.WriteString(deviceId);
    if (!ret) {
        HILOGE("CheckSystemAbility parcel write deviceId failed");
        return nullptr;
    }

    return CheckSystemAbilityWrapper(
        static_cast<uint32_t>(SamgrInterfaceCode::CHECK_REMOTE_SYSTEM_ABILITY_TRANSACTION), data, errCode);
}

sptr<IRemoteObject> SystemAbilityManagerProxy::CheckSystemAbility(int32_t systemAbilityId, bool& isExist)
{
    int32_t errCode = ERR_NONE;
    return CheckSystemAbility(systemAbilityId, isExist, errCode);
}
sptr<IRemoteObject> SystemAbilityManagerProxy::CheckSystemAbility(int32_t systemAbilityId, bool& isExist,
    int32_t& errCode)
{
    HILOGD("%{public}s called, SA:%{public}d, isExist is %{public}d", __func__, systemAbilityId, isExist);
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("CheckSystemAbility:SA:%{public}d invalid!", systemAbilityId);
        return nullptr;
    }

    auto proxy = LocalAbilitys::GetInstance().GetAbility(systemAbilityId);
    if (proxy != nullptr) {
        isExist = true;
        return proxy;
    }

    auto remote = Remote();
    if (remote == nullptr) {
        HILOGE("CheckSystemAbility remote is nullptr !");
        return nullptr;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        return nullptr;
    }
    bool ret = data.WriteInt32(systemAbilityId);
    if (!ret) {
        HILOGW("CheckSystemAbility Write SAId failed!");
        return nullptr;
    }

    ret = data.WriteBool(isExist);
    if (!ret) {
        HILOGW("CheckSystemAbility Write isExist failed!");
        return nullptr;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(
        static_cast<uint32_t>(SamgrInterfaceCode::CHECK_SYSTEM_ABILITY_IMMEDIATELY_TRANSACTION), data, reply, option);
    if (err != ERR_NONE) {
        errCode = err;
        return nullptr;
    }

    sptr<IRemoteObject> irsp = reply.ReadRemoteObject();
    if (irsp == nullptr) {
        HILOGW("CheckSystemAbility read remote object failed");
        return nullptr;
    }

    ret = reply.ReadBool(isExist);
    if (!ret) {
        HILOGW("CheckSystemAbility Read isExist failed!");
        return nullptr;
    }

    return irsp;
}

int32_t SystemAbilityManagerProxy::AddOnDemandSystemAbilityInfo(int32_t systemAbilityId,
    const std::u16string& localAbilityManagerName)
{
    HILOGD("%{public}s called, SA:%{public}d ", __func__, systemAbilityId);
    if (!CheckInputSysAbilityId(systemAbilityId) || localAbilityManagerName.empty()) {
        HILOGI("AddOnDemandSystemAbilityInfo invalid params!");
        return ERR_INVALID_VALUE;
    }

    auto remote = Remote();
    if (remote == nullptr) {
        HILOGE("AddOnDemandSystemAbilityInfo remote is nullptr !");
        return ERR_INVALID_OPERATION;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    bool ret = data.WriteInt32(systemAbilityId);
    if (!ret) {
        HILOGW("AddOnDemandSystemAbilityInfo Write SAId failed!");
        return ERR_FLATTEN_OBJECT;
    }

    ret = data.WriteString16(localAbilityManagerName);
    if (!ret) {
        HILOGW("AddOnDemandSystemAbilityInfo Write localAbilityManagerName failed!");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(
        static_cast<uint32_t>(SamgrInterfaceCode::ADD_ONDEMAND_SYSTEM_ABILITY_TRANSACTION), data, reply, option);

    HILOGI("AddOnDemandSaInfo SA:%{public}d %{public}s,rtn:%{public}d", systemAbilityId, err ? "fail" : "suc", err);
    if (err != ERR_NONE) {
        return err;
    }

    int32_t result = 0;
    ret = reply.ReadInt32(result);
    if (!ret) {
        HILOGW("AddOnDemandSystemAbilityInfo Read result failed!");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

int32_t SystemAbilityManagerProxy::RemoveSystemAbilityWrapper(int32_t code, MessageParcel& data)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGI("remote is nullptr !");
        return ERR_INVALID_OPERATION;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(code, data, reply, option);
    if (err != ERR_NONE) {
        HILOGE("RemoveSystemAbility SendRequest error:%{public}d!", err);
        return err;
    }

    int32_t result = 0;
    bool ret = reply.ReadInt32(result);
    if (!ret) {
        HILOGW("RemoveSystemAbility Read result failed!");
        return ERR_FLATTEN_OBJECT;
    }

    return result;
}

int32_t SystemAbilityManagerProxy::RemoveSystemAbility(int32_t systemAbilityId)
{
    HILOGD("%{public}s called, SA:%{public}d", __func__, systemAbilityId);
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("SA:%{public}d is invalid!", systemAbilityId);
        return ERR_INVALID_VALUE;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    bool ret = data.WriteInt32(systemAbilityId);
    if (!ret) {
        HILOGW("RemoveSystemAbility Write SAId failed!");
        return ERR_FLATTEN_OBJECT;
    }

    int32_t result = RemoveSystemAbilityWrapper(
        static_cast<uint32_t>(SamgrInterfaceCode::REMOVE_SYSTEM_ABILITY_TRANSACTION), data);
    if (result == ERR_OK) {
        LocalAbilitys::GetInstance().RemoveAbility(systemAbilityId);
    }
    return result;
}

std::vector<u16string> SystemAbilityManagerProxy::ListSystemAbilities(unsigned int dumpFlags)
{
    HILOGD("%{public}s called", __func__);
    std::vector<u16string> saNames;

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGI("remote is nullptr !");
        return saNames;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        HILOGW("ListSystemAbilities write token failed!");
        return saNames;
    }
    bool ret = data.WriteInt32(dumpFlags);
    if (!ret) {
        HILOGW("ListSystemAbilities write dumpFlags failed!");
        return saNames;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(
        static_cast<uint32_t>(SamgrInterfaceCode::LIST_SYSTEM_ABILITY_TRANSACTION), data, reply, option);
    if (err != ERR_NONE) {
        HILOGW("ListSystemAbilities transact failed!");
        return saNames;
    }
    if (reply.ReadInt32() != ERR_NONE) {
        HILOGW("ListSystemAbilities remote failed!");
        return saNames;
    }
    if (!reply.ReadString16Vector(&saNames)) {
        HILOGW("ListSystemAbilities read reply failed");
        saNames.clear();
    }
    return saNames;
}

int32_t SystemAbilityManagerProxy::SubscribeSystemAbility(int32_t systemAbilityId,
    const sptr<ISystemAbilityStatusChange>& listener)
{
    HILOGD("%{public}s called, SA:%{public}d", __func__, systemAbilityId);
    if (!CheckInputSysAbilityId(systemAbilityId) || listener == nullptr) {
        HILOGE("SubscribeSystemAbility SA:%{public}d or listener invalid!", systemAbilityId);
        return ERR_INVALID_VALUE;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGI("remote is nullptr !");
        return ERR_INVALID_OPERATION;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    bool ret = data.WriteInt32(systemAbilityId);
    if (!ret) {
        HILOGW("SubscribeSystemAbility Write saId failed!");
        return ERR_FLATTEN_OBJECT;
    }

    ret = data.WriteRemoteObject(listener->AsObject());
    if (!ret) {
        HILOGW("SubscribeSystemAbility Write listenerName failed!");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(
        static_cast<uint32_t>(SamgrInterfaceCode::SUBSCRIBE_SYSTEM_ABILITY_TRANSACTION), data, reply, option);
    if (err != ERR_NONE) {
        HILOGE("SubscribeSystemAbility SendRequest error:%{public}d!", err);
        return err;
    }
    HILOGD("SubscribeSystemAbility SendRequest succeed!");
    int32_t result = 0;
    ret = reply.ReadInt32(result);
    if (!ret) {
        HILOGW("SubscribeSystemAbility Read result failed!");
        return ERR_FLATTEN_OBJECT;
    }

    return result;
}

int32_t SystemAbilityManagerProxy::UnSubscribeSystemAbility(int32_t systemAbilityId,
    const sptr<ISystemAbilityStatusChange>& listener)
{
    HILOGD("%{public}s called, SA:%{public}d", __func__, systemAbilityId);
    if (!CheckInputSysAbilityId(systemAbilityId) || listener == nullptr) {
        HILOGE("UnSubscribeSystemAbility SA:%{public}d or listener invalid!", systemAbilityId);
        return ERR_INVALID_VALUE;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGI("remote is nullptr !");
        return ERR_INVALID_OPERATION;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    bool ret = data.WriteInt32(systemAbilityId);
    if (!ret) {
        HILOGW("UnSubscribeSystemAbility Write SAId failed!");
        return ERR_FLATTEN_OBJECT;
    }

    ret = data.WriteRemoteObject(listener->AsObject());
    if (!ret) {
        HILOGW("UnSubscribeSystemAbility Write listenerSaId failed!");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(
        static_cast<uint32_t>(SamgrInterfaceCode::UNSUBSCRIBE_SYSTEM_ABILITY_TRANSACTION), data, reply, option);
    if (err != ERR_NONE) {
        HILOGE("UnSubscribeSystemAbility SendRequest error:%{public}d!", err);
        return err;
    }
    HILOGD("UnSubscribeSystemAbility SendRequest succeed!");
    int32_t result = 0;
    ret = reply.ReadInt32(result);
    if (!ret) {
        HILOGW("UnSubscribeSystemAbility Read result failed!");
        return ERR_FLATTEN_OBJECT;
    }

    return result;
}

sptr<IRemoteObject> SystemAbilityManagerProxy::LoadSystemAbility(int32_t systemAbilityId, int32_t timeout)
{
    if (timeout < MIN_TIMEOUT) {
        timeout = MIN_TIMEOUT;
    } else if (timeout > MAX_TIMEOUT) {
        timeout = MAX_TIMEOUT;
    }
    sptr<SystemAbilityProxyCallback> callback = new SystemAbilityProxyCallback();
    std::unique_lock<std::mutex> lock(callback->callbackLock_);
    int32_t ret = LoadSystemAbility(systemAbilityId, callback);
    if (ret != ERR_OK) {
        HILOGE("LoadSystemAbility failed!");
        return nullptr;
    }
    auto waitStatus = callback->cv_.wait_for(lock, std::chrono::seconds(timeout),
        [&callback]() { return callback->loadproxy_ != nullptr; });
    if (!waitStatus) {
        HILOGE("LoadSystemAbility SA:%{public}d timeout", systemAbilityId);
        return nullptr;
    }
    return callback->loadproxy_;
}

int32_t SystemAbilityManagerProxy::LoadSystemAbility(int32_t systemAbilityId,
    const sptr<ISystemAbilityLoadCallback>& callback)
    __attribute__((no_sanitize("cfi")))
{
    if (!CheckInputSysAbilityId(systemAbilityId) || callback == nullptr) {
        HILOGE("LoadSystemAbility SA:%{public}d or callback invalid!", systemAbilityId);
        return ERR_INVALID_VALUE;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("LoadSystemAbility remote is null!");
        return ERR_INVALID_OPERATION;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        HILOGW("LoadSystemAbility Write interface token failed!");
        return ERR_FLATTEN_OBJECT;
    }
    bool ret = data.WriteInt32(systemAbilityId);
    if (!ret) {
        HILOGW("LoadSystemAbility Write SAId failed!");
        return ERR_FLATTEN_OBJECT;
    }
    ret = data.WriteRemoteObject(callback->AsObject());
    if (!ret) {
        HILOGW("LoadSystemAbility Write callback failed!");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(
        static_cast<uint32_t>(SamgrInterfaceCode::LOAD_SYSTEM_ABILITY_TRANSACTION), data, reply, option);
    if (err != ERR_NONE) {
        HILOGE("LoadSystemAbility SA:%{public}d invalid error:%{public}d!", systemAbilityId, err);
        return err;
    }
    HILOGD("LoadSystemAbility SA:%{public}d, SendRequest succeed!", systemAbilityId);
    int32_t result = 0;
    ret = reply.ReadInt32(result);
    if (!ret) {
        HILOGW("LoadSystemAbility Read reply failed!");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

int32_t SystemAbilityManagerProxy::LoadSystemAbility(int32_t systemAbilityId, const std::string& deviceId,
    const sptr<ISystemAbilityLoadCallback>& callback)
{
    if (!CheckInputSysAbilityId(systemAbilityId) || deviceId.empty() || callback == nullptr) {
        HILOGE("LoadSystemAbility SA:%{public}d ,deviceId or callback invalid!", systemAbilityId);
        return ERR_INVALID_VALUE;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("LoadSystemAbility remote is null!");
        return ERR_INVALID_OPERATION;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        HILOGW("LoadSystemAbility write interface token failed!");
        return ERR_FLATTEN_OBJECT;
    }
    bool ret = data.WriteInt32(systemAbilityId);
    if (!ret) {
        HILOGW("LoadSystemAbility write SAId failed!");
        return ERR_FLATTEN_OBJECT;
    }
    ret = data.WriteString(deviceId);
    if (!ret) {
        HILOGW("LoadSystemAbility write deviceId failed!");
        return ERR_FLATTEN_OBJECT;
    }
    ret = data.WriteRemoteObject(callback->AsObject());
    if (!ret) {
        HILOGW("LoadSystemAbility Write callback failed!");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(
        static_cast<uint32_t>(SamgrInterfaceCode::LOAD_REMOTE_SYSTEM_ABILITY_TRANSACTION), data, reply, option);
    if (err != ERR_NONE) {
        HILOGE("LoadSystemAbility SA:%{public}d invalid error:%{public}d!", systemAbilityId, err);
        return err;
    }
    HILOGD("LoadSystemAbility SA:%{public}d for remote, SendRequest succeed!", systemAbilityId);
    int32_t result = 0;
    ret = reply.ReadInt32(result);
    if (!ret) {
        HILOGW("LoadSystemAbility read reply failed for remote!");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

int32_t SystemAbilityManagerProxy::UnloadSystemAbility(int32_t systemAbilityId)
{
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGE("UnloadSystemAbility SA:%{public}d invalid!", systemAbilityId);
        return ERR_INVALID_VALUE;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("UnloadSystemAbility remote is null!");
        return ERR_INVALID_OPERATION;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        HILOGW("UnloadSystemAbility Write interface token failed!");
        return ERR_FLATTEN_OBJECT;
    }
    bool ret = data.WriteInt32(systemAbilityId);
    if (!ret) {
        HILOGW("UnloadSystemAbility Write systemAbilityId failed!");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(
        static_cast<uint32_t>(SamgrInterfaceCode::UNLOAD_SYSTEM_ABILITY_TRANSACTION), data, reply, option);
    if (err != ERR_NONE) {
        HILOGE("UnloadSystemAbility SA:%{public}d invalid error:%{public}d!", systemAbilityId, err);
        return err;
    }
    HILOGD("UnloadSystemAbility SA:%{public}d, SendRequest succeed!", systemAbilityId);
    int32_t result = 0;
    ret = reply.ReadInt32(result);
    if (!ret) {
        HILOGW("UnloadSystemAbility Read reply failed!");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

int32_t SystemAbilityManagerProxy::CancelUnloadSystemAbility(int32_t systemAbilityId)
{
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGE("CancelUnloadSystemAbility SA:%{public}d invalid!", systemAbilityId);
        return ERR_INVALID_VALUE;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("CancelUnloadSystemAbility remote is null!");
        return ERR_INVALID_OPERATION;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        HILOGW("CancelUnloadSystemAbility Write interface token failed!");
        return ERR_FLATTEN_OBJECT;
    }
    bool ret = data.WriteInt32(systemAbilityId);
    if (!ret) {
        HILOGW("CancelUnloadSystemAbility Write SAId failed!");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(
        static_cast<uint32_t>(SamgrInterfaceCode::CANCEL_UNLOAD_SYSTEM_ABILITY_TRANSACTION), data, reply, option);
    if (err != ERR_NONE) {
        HILOGE("CancelUnloadSystemAbility SA:%{public}d SendRequest failed, error:%{public}d!",
            systemAbilityId, err);
        return err;
    }
    HILOGD("CancelUnloadSystemAbility SA:%{public}d, SendRequest succeed!", systemAbilityId);
    int32_t result = 0;
    ret = reply.ReadInt32(result);
    if (!ret) {
        HILOGW("CancelUnloadSystemAbility Read reply failed!");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

int32_t SystemAbilityManagerProxy::UnloadAllIdleSystemAbility()
{
    HILOGI("UnloadAllIdleSystemAbility called");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("UnloadAllIdleSystemAbility remote is nullptr");
        return ERR_INVALID_OPERATION;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        HILOGE("UnloadAllIdleSystemAbility write interface token failed");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int32_t err = remote->SendRequest(
        static_cast<uint32_t>(SamgrInterfaceCode::UNLOAD_ALL_IDLE_SYSTEM_ABILITY_TRANSACTION), data, reply, option);
    if (err != ERR_NONE) {
        HILOGE("UnloadAllIdleSystemAbility SendRequest error:%{public}d", err);
        return err;
    }
    HILOGD("UnloadAllIdleSystemAbility SendRequest succeed");
    return ERR_OK;
}

int32_t SystemAbilityManagerProxy::MarshalSAExtraProp(const SAExtraProp& extraProp, MessageParcel& data) const
{
    if (!data.WriteBool(extraProp.isDistributed)) {
        HILOGW("MarshalSAExtraProp Write isDistributed failed!");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(extraProp.dumpFlags)) {
        HILOGW("MarshalSAExtraProp Write dumpFlags failed!");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteString16(extraProp.capability)) {
        HILOGW("MarshalSAExtraProp Write capability failed!");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteString16(extraProp.permission)) {
        HILOGW("MarshalSAExtraProp Write defPermission failed!");
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_OK;
}

int32_t SystemAbilityManagerProxy::AddSystemAbility(int32_t systemAbilityId, const sptr<IRemoteObject>& ability,
    const SAExtraProp& extraProp)
{
    HILOGD("%{public}s called, SA:%{public}d", __func__, systemAbilityId);
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("SA:%{public}d invalid.", systemAbilityId);
        return ERR_INVALID_VALUE;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(systemAbilityId)) {
        HILOGW("AddSystemAbility Write saId failed!");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteRemoteObject(ability)) {
        HILOGW("AddSystemAbility Write ability failed!");
        return ERR_FLATTEN_OBJECT;
    }

    int32_t ret = MarshalSAExtraProp(extraProp, data);
    if (ret != ERR_OK) {
        HILOGW("AddSystemAbility MarshalSAExtraProp failed!");
        return ret;
    }

    int32_t result = AddSystemAbilityWrapper(
        static_cast<uint32_t>(SamgrInterfaceCode::ADD_SYSTEM_ABILITY_TRANSACTION), data);
    if (result == ERR_OK) {
        LocalAbilitys::GetInstance().AddAbility(systemAbilityId, ability);
    }
    return result;
}

int32_t SystemAbilityManagerProxy::AddSystemAbilityWrapper(int32_t code, MessageParcel& data)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGI("remote is nullptr !");
        return ERR_INVALID_OPERATION;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(code, data, reply, option);
    if (err != ERR_NONE) {
        HILOGE("AddSystemAbility SA invalid error:%{public}d!", err);
        return err;
    }
    int32_t result = 0;
    bool ret = reply.ReadInt32(result);
    if (!ret) {
        HILOGE("AddSystemAbility read result error!");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

int32_t SystemAbilityManagerProxy::AddSystemProcess(const u16string& procName, const sptr<IRemoteObject>& procObject)
{
    HILOGD("%{public}s called, process name is %{public}s", __func__, Str16ToStr8(procName).c_str());
    if (procName.empty()) {
        HILOGI("process name is invalid!");
        return ERR_INVALID_VALUE;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteString16(procName)) {
        HILOGW("AddSystemProcess Write name failed!");
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteRemoteObject(procObject)) {
        HILOGW("AddSystemProcess Write ability failed!");
        return ERR_FLATTEN_OBJECT;
    }
    return AddSystemAbilityWrapper(
        static_cast<uint32_t>(SamgrInterfaceCode::ADD_SYSTEM_PROCESS_TRANSACTION), data);
}

int32_t SystemAbilityManagerProxy::GetSystemProcessInfo(int32_t systemAbilityId, SystemProcessInfo& systemProcessInfo)
{
    HILOGD("GetSystemProcessInfo called");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGI("GetSystemProcessInfo remote is nullptr");
        return ERR_INVALID_OPERATION;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(systemAbilityId)) {
        HILOGW("GetSystemProcessInfo Write saId failed!");
        return ERR_FLATTEN_OBJECT;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(
        static_cast<uint32_t>(SamgrInterfaceCode::GET_SYSTEM_PROCESS_INFO_TRANSACTION), data, reply, option);
    if (err != ERR_NONE) {
        HILOGE("GetSystemProcessInfo SendRequest error: %{public}d!", err);
        return err;
    }
    HILOGD("GetSystemProcessInfo SendRequest succeed!");
    int32_t result = 0;
    bool ret = reply.ReadInt32(result);
    if (!ret) {
        HILOGW("GetSystemProcessInfo Read result failed!");
        return ERR_FLATTEN_OBJECT;
    }
    if (result != ERR_OK) {
        HILOGE("GetSystemProcessInfo failed: %{public}d!", result);
        return result;
    }
    return ReadProcessInfoFromParcel(reply, systemProcessInfo);
}

int32_t SystemAbilityManagerProxy::ReadProcessInfoFromParcel(MessageParcel& reply,
    SystemProcessInfo& systemProcessInfo)
{
    bool ret = reply.ReadString(systemProcessInfo.processName);
    if (!ret) {
        HILOGW("GetSystemProcessInfo Read processName failed!");
        return ERR_FLATTEN_OBJECT;
    }
    ret = reply.ReadInt32(systemProcessInfo.pid);
    if (!ret) {
        HILOGW("GetSystemProcessInfo Read pid failed!");
        return ERR_FLATTEN_OBJECT;
    }
    ret = reply.ReadInt32(systemProcessInfo.uid);
    if (!ret) {
        HILOGW("GetSystemProcessInfo Read uid failed!");
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_OK;
}

int32_t SystemAbilityManagerProxy::GetRunningSystemProcess(std::list<SystemProcessInfo>& systemProcessInfos)
{
    HILOGD("GetRunningSystemProcess called");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGI("GetRunningSystemProcess remote is nullptr");
        return ERR_INVALID_OPERATION;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(
        static_cast<uint32_t>(SamgrInterfaceCode::GET_RUNNING_SYSTEM_PROCESS_TRANSACTION), data, reply, option);
    if (err != ERR_NONE) {
        HILOGE("GetRunningSystemProcess SendRequest error: %{public}d!", err);
        return err;
    }
    HILOGD("GetRunningSystemProcess SendRequest succeed!");
    int32_t result = 0;
    bool ret = reply.ReadInt32(result);
    if (!ret) {
        HILOGW("GetRunningSystemProcess Read result failed!");
        return ERR_FLATTEN_OBJECT;
    }
    if (result != ERR_OK) {
        HILOGE("GetRunningSystemProcess failed: %{public}d!", result);
        return result;
    }
    return ReadSystemProcessFromParcel(reply, systemProcessInfos);
}

int32_t SystemAbilityManagerProxy::ReadSystemProcessFromParcel(MessageParcel& reply,
    std::list<SystemProcessInfo>& systemProcessInfos)
{
    int32_t size = 0;
    bool ret = reply.ReadInt32(size);
    if (!ret) {
        HILOGW("GetRunningSystemProcess Read list size failed!");
        return ERR_FLATTEN_OBJECT;
    }
    systemProcessInfos.clear();
    if (size == 0) {
        return ERR_OK;
    }
    if (static_cast<size_t>(size) > reply.GetReadableBytes() || size < 0) {
        HILOGE("Failed to read proc list, size=%{public}d", size);
        return ERR_FLATTEN_OBJECT;
    }
    for (int32_t i = 0; i < size; i++) {
        SystemProcessInfo systemProcessInfo;
        ret = reply.ReadString(systemProcessInfo.processName);
        if (!ret) {
            HILOGW("GetRunningSystemProcess Read processName failed!");
            return ERR_FLATTEN_OBJECT;
        }
        ret = reply.ReadInt32(systemProcessInfo.pid);
        if (!ret) {
            HILOGW("GetRunningSystemProcess Read pid failed!");
            return ERR_FLATTEN_OBJECT;
        }
        ret = reply.ReadInt32(systemProcessInfo.uid);
        if (!ret) {
            HILOGW("GetRunningSystemProcess Read uid failed!");
            return ERR_FLATTEN_OBJECT;
        }
        systemProcessInfos.emplace_back(systemProcessInfo);
    }
    return ERR_OK;
}

int32_t SystemAbilityManagerProxy::SubscribeSystemProcess(const sptr<ISystemProcessStatusChange>& listener)
{
    HILOGD("SubscribeSystemProcess called");
    if (listener == nullptr) {
        HILOGE("SubscribeSystemProcess listener is nullptr");
        return ERR_INVALID_VALUE;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGI("SubscribeSystemProcess remote is nullptr");
        return ERR_INVALID_OPERATION;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    bool ret = data.WriteRemoteObject(listener->AsObject());
    if (!ret) {
        HILOGW("SubscribeSystemProcess Write listenerName failed");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(
        static_cast<uint32_t>(SamgrInterfaceCode::SUBSCRIBE_SYSTEM_PROCESS_TRANSACTION), data, reply, option);
    if (err != ERR_NONE) {
        HILOGE("SubscribeSystemProcess SendRequest error:%{public}d!", err);
        return err;
    }
    HILOGD("SubscribeSystemProcesss SendRequest succeed!");
    int32_t result = 0;
    ret = reply.ReadInt32(result);
    if (!ret) {
        HILOGW("SubscribeSystemProcess Read result failed!");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

int32_t SystemAbilityManagerProxy::UnSubscribeSystemProcess(const sptr<ISystemProcessStatusChange>& listener)
{
    HILOGD("UnSubscribeSystemProcess called");
    if (listener == nullptr) {
        HILOGE("UnSubscribeSystemProcess listener is nullptr");
        return ERR_INVALID_VALUE;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGI("UnSubscribeSystemProcess remote is nullptr");
        return ERR_INVALID_OPERATION;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    bool ret = data.WriteRemoteObject(listener->AsObject());
    if (!ret) {
        HILOGW("UnSubscribeSystemProcess Write listenerName failed");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(
        static_cast<uint32_t>(SamgrInterfaceCode::UNSUBSCRIBE_SYSTEM_PROCESS_TRANSACTION), data, reply, option);
    if (err != ERR_NONE) {
        HILOGE("UnSubscribeSystemProcess SendRequest error:%{public}d!", err);
        return err;
    }
    HILOGD("UnSubscribeSystemProcess SendRequest succeed!");
    int32_t result = 0;
    ret = reply.ReadInt32(result);
    if (!ret) {
        HILOGW("UnSubscribeSystemProcess Read result failed!");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

int32_t SystemAbilityManagerProxy::GetOnDemandReasonExtraData(int64_t extraDataId, MessageParcel& extraDataParcel)
{
    HILOGD("GetOnDemandReasonExtraData called");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("GetOnDemandReasonExtraData remote is nullptr");
        return ERR_INVALID_OPERATION;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        HILOGE("GetOnDemandReasonExtraData write interface token failed");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteInt64(extraDataId)) {
        HILOGE("GetOnDemandReasonExtraData write extraDataId failed");
        return ERR_FLATTEN_OBJECT;
    }

    MessageOption option;
    int32_t err = remote->SendRequest(
        static_cast<uint32_t>(SamgrInterfaceCode::GET_ONDEMAND_REASON_EXTRA_DATA_TRANSACTION),
        data, extraDataParcel, option);
    if (err != ERR_NONE) {
        HILOGE("GetOnDemandReasonExtraData SendRequest error:%{public}d", err);
        return err;
    }
    HILOGD("GetOnDemandReasonExtraData SendRequest succeed");
    int32_t result = 0;
    if (!extraDataParcel.ReadInt32(result)) {
        HILOGE("GetOnDemandReasonExtraData read result failed");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

int32_t SystemAbilityManagerProxy::GetOnDemandPolicy(int32_t systemAbilityId, OnDemandPolicyType type,
    std::vector<SystemAbilityOnDemandEvent>& abilityOnDemandEvents)
{
    HILOGD("GetOnDemandPolicy called");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGI("GetOnDemandPolicy remote is nullptr");
        return ERR_INVALID_OPERATION;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        HILOGE("GetOnDemandPolicy write interface token failed!");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(systemAbilityId)) {
        HILOGE("GetOnDemandPolicy write said failed!");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(static_cast<int32_t>(type))) {
        HILOGE("GetOnDemandPolicy write type failed!");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(
        static_cast<uint32_t>(SamgrInterfaceCode::GET_ONDEAMND_POLICY_TRANSACTION), data, reply, option);
    if (err != ERR_NONE) {
        HILOGE("GetOnDemandPolicy SendRequest error: %{public}d!", err);
        return err;
    }
    HILOGD("GetOnDemandPolicy SendRequest succeed!");
    int32_t result = 0;
    if (!reply.ReadInt32(result)) {
        HILOGE("GetOnDemandPolicy Read result failed!");
        return ERR_FLATTEN_OBJECT;
    }
    if (result != ERR_OK) {
        HILOGE("GetOnDemandPolicy failed: %{public}d!", result);
        return result;
    }
    if (!OnDemandEventToParcel::ReadOnDemandEventsFromParcel(abilityOnDemandEvents, reply)) {
        HILOGE("GetOnDemandPolicy Read on demand events failed!");
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_OK;
}

int32_t SystemAbilityManagerProxy::GetOnDemandSystemAbilityIds(std::vector<int32_t>& systemAbilityIds)
{
    HILOGD("GetOnDemandSystemAbilityIds called");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGI("GetOnDemandSystemAbilityIds remote is nullptr");
        return ERR_INVALID_OPERATION;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        HILOGE("GetOnDemandPolicy write interface token failed!");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(
        static_cast<uint32_t>(SamgrInterfaceCode::GET_ONDEMAND_SYSTEM_ABILITY_IDS_TRANSACTION), data, reply, option);
    if (err != ERR_NONE) {
        HILOGE("GetOnDemandSystemAbilityIds SendRequest error: %{public}d!", err);
        return err;
    }
    HILOGD("GetOnDemandSystemAbilityIds SendRequest succeed!");
    int32_t result = 0;
    if (!reply.ReadInt32(result)) {
        HILOGE("GetOnDemandSystemAbilityIds Read result failed!");
        return ERR_FLATTEN_OBJECT;
    }
    if (result != ERR_OK) {
        HILOGE("GetOnDemandSystemAbilityIds failed: %{public}d!", result);
        return result;
    }
    if (!reply.ReadInt32Vector(&systemAbilityIds)) {
        HILOGW("GetOnDemandSystemAbilityIds SAIds read reply failed");
        systemAbilityIds.clear();
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_OK;
}

int32_t SystemAbilityManagerProxy::UpdateOnDemandPolicy(int32_t systemAbilityId, OnDemandPolicyType type,
    const std::vector<SystemAbilityOnDemandEvent>& abilityOnDemandEvents)
{
    HILOGD("UpdateOnDemandPolicy called");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGI("UpdateOnDemandPolicy remote is nullptr");
        return ERR_INVALID_OPERATION;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        HILOGE("UpdateOnDemandPolicy write interface token failed!");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(systemAbilityId)) {
        HILOGE("UpdateOnDemandPolicy write said failed!");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(static_cast<int32_t>(type))) {
        HILOGE("UpdateOnDemandPolicy write type failed!");
        return ERR_FLATTEN_OBJECT;
    }
    if (!OnDemandEventToParcel::WriteOnDemandEventsToParcel(abilityOnDemandEvents, data)) {
        HILOGW("UpdateOnDemandPolicy write on demand events failed!");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(
        static_cast<uint32_t>(SamgrInterfaceCode::UPDATE_ONDEAMND_POLICY_TRANSACTION), data, reply, option);
    if (err != ERR_NONE) {
        HILOGE("UpdateOnDemandPolicy SendRequest error: %{public}d!", err);
        return err;
    }
    HILOGD("UpdateOnDemandPolicy SendRequest succeed!");
    int32_t result = 0;
    if (!reply.ReadInt32(result)) {
        HILOGE("UpdateOnDemandPolicy Read result failed!");
        return ERR_FLATTEN_OBJECT;
    }
    if (result != ERR_OK) {
        HILOGE("UpdateOnDemandPolicy failed: %{public}d!", result);
    }
    return result;
}

int32_t SystemAbilityManagerProxy::SendStrategy(int32_t type, std::vector<int32_t>& systemAbilityIds,
    int32_t level, std::string& action)
{
    HILOGD("SendStrategy called");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGI("SendStrategy remote is nullptr");
        return ERR_INVALID_OPERATION;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        HILOGE("SendStrategy write interface token failed!");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(type)) {
        HILOGE("SendStrategy write type failed!");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32Vector(systemAbilityIds)) {
        HILOGE("SendStrategy write said failed!");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(level)) {
        HILOGE("SendStrategy write level failed!");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteString(action)) {
        HILOGW("SendStrategy write action failed!");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(
        static_cast<uint32_t>(SamgrInterfaceCode::SEND_STRATEGY_TRANASACTION), data, reply, option);
    if (err != ERR_NONE) {
        HILOGE("SendStrategy SendRequest error: %{public}d!", err);
        return err;
    }
    HILOGD("SendStrategy SendRequest succeed!");
    int32_t result = 0;
    if (!reply.ReadInt32(result)) {
        HILOGE("SendStrategy Read result failed!");
        return ERR_FLATTEN_OBJECT;
    }
    if (result != ERR_OK) {
        HILOGE("SendStrategy failed: %{public}d!", result);
    }
    return result;
}

int32_t SystemAbilityManagerProxy::ListExtensionSendReq(const std::string& extension,
    SamgrInterfaceCode cmd, MessageParcel& reply, MessageOption& option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("remote is nullptr !");
        return ERR_INVALID_OPERATION;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        HILOGW("%{public}s write token failed!", __func__);
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteString(extension)) {
        HILOGW("%{public}s write extension failed!", __func__);
        return ERR_FLATTEN_OBJECT;
    }
    int32_t err = remote->SendRequest(
        static_cast<uint32_t>(cmd), data, reply, option);
    if (err != ERR_NONE) {
        HILOGW("%{public}s transact failed!", __func__);
        return err;
    }
    int32_t result;
    if (!reply.ReadInt32(result)) {
        HILOGW("%{public}s Read result failed!", __func__);
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

int32_t SystemAbilityManagerProxy::GetExtensionSaIds(const std::string& extension, std::vector<int32_t>& saIds)
{
    HILOGD("%{public}s called", __func__);

    MessageParcel reply;
    MessageOption option;
    int32_t ret = ListExtensionSendReq(extension,
        SamgrInterfaceCode::GET_EXTENSION_SA_IDS_TRANSCATION, reply, option);
    if (ret != ERR_OK) {
        return ret;
    }
    if (!reply.ReadInt32Vector(&saIds)) {
        HILOGW("%{public}s read reply failed", __func__);
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_OK;
}

int32_t SystemAbilityManagerProxy::GetExtensionRunningSaList(const std::string& extension,
    std::vector<sptr<IRemoteObject>>& saList)
{
    HILOGD("%{public}s called", __func__);

    MessageParcel reply;
    MessageOption option;
    int32_t ret = ListExtensionSendReq(extension,
        SamgrInterfaceCode::GET_EXTERNSION_SA_LIST_TRANSCATION, reply, option);
    if (ret != ERR_OK) {
        return ret;
    }
    int32_t size;
    if (!reply.ReadInt32(size)) {
        HILOGW("%{public}s read reply failed", __func__);
        return ERR_FLATTEN_OBJECT;
    }
    for (int32_t i = 0; i < size; ++i) {
        sptr<IRemoteObject> obj = reply.ReadRemoteObject();
        if (obj == nullptr) {
            HILOGW("%{public}s read reply loop(%{public}d) size(%{public}d) failed", __func__, i, size);
            saList.clear();
            return ERR_FLATTEN_OBJECT;
        }
        saList.emplace_back(obj);
    }
    return ERR_OK;
}

int32_t SystemAbilityManagerProxy::GetRunningSaExtensionInfoList(const std::string& extension,
    std::vector<SaExtensionInfo>& infoList)
{
    HILOGD("%{public}s called", __func__);
    MessageParcel reply;
    MessageOption option;
    int32_t ret = ListExtensionSendReq(extension,
        SamgrInterfaceCode::GET_SA_EXTENSION_INFO_TRANSCATION, reply, option);
    if (ret != ERR_OK) {
        return ret;
    }
    int32_t size = 0;
    if (!reply.ReadInt32(size)) {
        HILOGW("get SaExtInfoList read reply size failed");
        return ERR_FLATTEN_OBJECT;
    }
    for (int32_t i = 0; i < size; ++i) {
        SaExtensionInfo tmp;
        if (!reply.ReadInt32(tmp.saId)) {
            HILOGW("get SaExtInfoList read reply id failed");
            infoList.clear();
            return ERR_FLATTEN_OBJECT;
        }
        tmp.processObj = reply.ReadRemoteObject();
        if (tmp.processObj == nullptr) {
            HILOGW("get SaExtInfoList read reply loop:%{public}d size:%{public}d failed", i, size);
            infoList.clear();
            return ERR_FLATTEN_OBJECT;
        }
        infoList.emplace_back(tmp);
    }
    return ERR_OK;
}

int32_t SystemAbilityManagerProxy::GetCommonEventExtraDataIdlist(int32_t saId, std::vector<int64_t>& extraDataIdList,
    const std::string& eventName)
{
    HILOGD("getExtraIdList called");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("getExtraIdList remote is nullptr");
        return ERR_INVALID_OPERATION;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        HILOGE("getExtraIdList write token failed!");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(saId)) {
        HILOGE("getExtraIdList write said failed!");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteString(eventName)) {
        HILOGW("getExtraIdList write eventname failed!");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(
        static_cast<uint32_t>(SamgrInterfaceCode::GET_COMMON_EVENT_EXTRA_ID_LIST_TRANSCATION), data, reply, option);
    if (err != ERR_NONE) {
        HILOGE("getExtraIdList SendRequest error: %{public}d!", err);
        return err;
    }
    HILOGD("getExtraIdList SendRequest succeed!");
    int32_t result = 0;
    if (!reply.ReadInt32(result)) {
        HILOGE("getExtraIdList Read result failed!");
        return ERR_FLATTEN_OBJECT;
    }
    if (result != ERR_OK) {
        HILOGE("getExtraIdList failed: %{public}d!", result);
        return result;
    }
    if (!reply.ReadInt64Vector(&extraDataIdList)) {
        HILOGW("getExtraIdList read idlist failed");
        extraDataIdList.clear();
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_OK;
}
} // namespace OHOS
