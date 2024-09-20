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

#include "device_networking_collect.h"

#include "sam_log.h"
#include "sa_profiles.h"
#include "system_ability_manager.h"

using namespace std;

using namespace OHOS::DistributedHardware;

namespace OHOS {
namespace {
const std::string PKG_NAME = "Samgr_Networking";
const std::string SA_TAG_DEVICE_ON_LINE = "deviceonline";
constexpr uint32_t INIT_EVENT = 10;
constexpr uint32_t DM_DIED_EVENT = 11;
constexpr uint64_t DELAY_TIME = 1000;
constexpr int32_t DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID = 4802;
}
DeviceNetworkingCollect::DeviceNetworkingCollect(const sptr<IReport>& report)
    : ICollectPlugin(report)
{
}

void DeviceNetworkingCollect::CleanFfrt()
{
    if (workHandler_ != nullptr) {
        workHandler_->CleanFfrt();
    }
}

void DeviceNetworkingCollect::SetFfrt()
{
    if (workHandler_ != nullptr) {
        workHandler_->SetFfrt();
    }
}

int32_t DeviceNetworkingCollect::OnStart()
{
    HILOGI("DeviceNetworkingCollect OnStart called");
    workHandler_ = std::make_shared<WorkHandler>(this);
    initCallback_ = std::make_shared<DeviceInitCallBack>(workHandler_);
    stateCallback_ = std::make_shared<DeviceStateCallback>(this);
    workHandler_->SendEvent(INIT_EVENT);
    return ERR_OK;
}

int32_t DeviceNetworkingCollect::OnStop()
{
    DeviceManager::GetInstance().UnRegisterDevStateCallback(PKG_NAME);
    if (workHandler_ != nullptr) {
        workHandler_ = nullptr;
    }
    initCallback_ = nullptr;
    ClearDeviceOnlineSet();
    stateCallback_ = nullptr;
    return ERR_OK;
}

bool DeviceNetworkingCollect::IsDmReady()
{
    auto dmProxy = SystemAbilityManager::GetInstance()->CheckSystemAbility(
        DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID);
    if (dmProxy != nullptr) {
        IPCObjectProxy* proxy = reinterpret_cast<IPCObjectProxy*>(dmProxy.GetRefPtr());
        // make sure the proxy is not dead
        if (proxy != nullptr && !proxy->IsObjectDead()) {
            return true;
        }
    }
    return false;
}

bool DeviceNetworkingCollect::ReportMissedEvents()
{
    std::vector<DmDeviceInfo> devList;
    int32_t ret = DeviceManager::GetInstance().GetTrustedDeviceList(PKG_NAME, "", devList);
    if (ret != ERR_OK) {
        HILOGE("DeviceNetworkingCollect GetTrustedDeviceList error");
        return false;
    }
    bool isPreviousOnline = IsOnline();
    if (isPreviousOnline) {
        ClearDeviceOnlineSet();
        if (devList.empty()) {
            // send offline msg
            OnDemandEvent event = { DEVICE_ONLINE, SA_TAG_DEVICE_ON_LINE, "off" };
            ReportEvent(event);
        } else {
            // update the online set;
            for (DmDeviceInfo& devInfo : devList) {
                UpdateDeviceOnlineSet(devInfo.networkId);
            }
        }
    } else {
        // offline --> online
        if (!devList.empty()) {
            // update the online set;
            for (DmDeviceInfo& devInfo : devList) {
                UpdateDeviceOnlineSet(devInfo.networkId);
            }
            // send online msg
            OnDemandEvent event = { DEVICE_ONLINE, SA_TAG_DEVICE_ON_LINE, "on" };
            ReportEvent(event);
        }
    }
    return true;
}

bool DeviceNetworkingCollect::AddDeviceChangeListener()
{
    HILOGI("AddDeviceChangeListener called");
    if (IsDmReady()) {
        int32_t ret = DeviceManager::GetInstance().InitDeviceManager(PKG_NAME, initCallback_);
        if (ret != ERR_OK) {
            HILOGE("InitDeviceManager error");
            return false;
        }
        if (!ReportMissedEvents()) {
            HILOGE("ReportMissedEvents error");
            return false;
        }
        ret = DeviceManager::GetInstance().RegisterDevStateCallback(PKG_NAME, "", stateCallback_);
        if (ret != ERR_OK) {
            DeviceManager::GetInstance().UnRegisterDevStateCallback(PKG_NAME);
            HILOGE("RegisterDevStateCallback error");
            return false;
        }
        HILOGI("AddDeviceChangeListener success");
        return true;
    }
    return false;
}

void DeviceNetworkingCollect::UpdateDeviceOnlineSet(const std::string& deviceId)
{
    if (stateCallback_ != nullptr) {
        stateCallback_->UpdateDeviceOnlineSet(deviceId);
    }
}

void DeviceNetworkingCollect::ClearDeviceOnlineSet()
{
    if (stateCallback_ != nullptr) {
        stateCallback_->ClearDeviceOnlineSet();
    }
}

bool DeviceNetworkingCollect::CheckCondition(const OnDemandCondition& condition)
{
    bool isOnline = IsOnline();
    if (condition.value == "on" && isOnline) {
        return true;
    }
    if (condition.value == "off" && !isOnline) {
        return true;
    }
    return false;
}

bool DeviceNetworkingCollect::IsOnline()
{
    if (stateCallback_ != nullptr) {
        return stateCallback_->IsOnline();
    }
    return false;
}

void DeviceInitCallBack::OnRemoteDied()
{
    HILOGI("DeviceNetworkingCollect DeviceInitCallBack OnRemoteDied");
    if (handler_ != nullptr) {
        handler_->SendEvent(DM_DIED_EVENT, DELAY_TIME);
    }
}

void DeviceStateCallback::OnDeviceOnline(const DmDeviceInfo& deviceInfo)
{
    HILOGI("DeviceNetworkingCollect OnDeviceOnline size %{public}zu", deviceOnlineSet_.size());
    {
        lock_guard<mutex> autoLock(deviceOnlineLock_);
        deviceOnlineSet_.emplace(deviceInfo.networkId);
    }

    if (collect_ != nullptr) {
        OnDemandEvent event = { DEVICE_ONLINE, SA_TAG_DEVICE_ON_LINE, "on" };
        collect_->ReportEvent(event);
    } else {
        HILOGE("OnDeviceOnline collect_ isnull");
    }
}

void DeviceStateCallback::OnDeviceOffline(const DmDeviceInfo& deviceInfo)
{
    HILOGI("DeviceNetworkingCollect OnDeviceOffline size %{public}zu", deviceOnlineSet_.size());
    bool isOffline = false;
    {
        lock_guard<mutex> autoLock(deviceOnlineLock_);
        deviceOnlineSet_.erase(deviceInfo.networkId);
        isOffline = deviceOnlineSet_.empty();
    }
    if (isOffline) {
        OnDemandEvent event = { DEVICE_ONLINE, SA_TAG_DEVICE_ON_LINE, "off" };
        if (collect_ != nullptr) {
            collect_->ReportEvent(event);
        } else {
            HILOGE("OnDeviceOffline collect_ isnull");
        }
    } else {
        HILOGI("OnDeviceOffline not report size %{public}zu", deviceOnlineSet_.size());
    }
}

void DeviceStateCallback::ClearDeviceOnlineSet()
{
    lock_guard<mutex> autoLock(deviceOnlineLock_);
    deviceOnlineSet_.clear();
}

bool DeviceStateCallback::IsOnline()
{
    lock_guard<mutex> autoLock(deviceOnlineLock_);
    return !deviceOnlineSet_.empty();
}

void DeviceStateCallback::UpdateDeviceOnlineSet(const std::string& deviceId)
{
    lock_guard<mutex> autoLock(deviceOnlineLock_);
    deviceOnlineSet_.emplace(deviceId);
}

void DeviceStateCallback::OnDeviceChanged(const DmDeviceInfo& deviceInfo)
{
    HILOGD("DeviceNetworkingCollect OnDeviceChanged called");
}

void DeviceStateCallback::OnDeviceReady(const DmDeviceInfo& deviceInfo)
{
    HILOGI("DeviceNetworkingCollect DeviceStateCallback OnDeviceReady");
    OnDemandEvent event = { DEVICE_ONLINE, SA_TAG_DEVICE_ON_LINE, "ready" };

    if (collect_ != nullptr) {
        collect_->ReportEvent(event);
    } else {
        HILOGE("OnDeviceOnline collect_ isnull");
    }
}

void WorkHandler::CleanFfrt()
{
    if (handler_ != nullptr) {
        handler_->CleanFfrt();
    }
}

void WorkHandler::SetFfrt()
{
    if (handler_ != nullptr) {
        handler_->SetFfrt("WorkHandler");
    }
}

void WorkHandler::ProcessEvent(uint32_t eventId)
{
    if (collect_ == nullptr) {
        HILOGE("NetworkingCollect ProcessEvent collect or event is null!");
        return;
    }
    if (eventId != INIT_EVENT && eventId != DM_DIED_EVENT) {
        HILOGE("NetworkingCollect ProcessEvent error event code!");
        return;
    }
    if (handler_ == nullptr) {
        HILOGE("NetworkingCollect SendEvent handler is null!");
        return;
    }
    if (!collect_->AddDeviceChangeListener()) {
        HILOGW("AddDeviceChangeListener retry");
        auto task = [this] {this->ProcessEvent(INIT_EVENT);};
        if (handler_ == nullptr) {
            HILOGE("NetworkingCollect ProcessEvent handler is null!");
            return;
        }
        handler_->PostTask(task, DELAY_TIME);
    }
}

bool WorkHandler::SendEvent(uint32_t eventId)
{
    if (handler_ == nullptr) {
        HILOGE("NetworkingCollect SendEvent handler is null!");
        return false;
    }
    auto task = [this, eventId] {this->ProcessEvent(eventId);};
    return handler_->PostTask(task);
}

bool WorkHandler::SendEvent(uint32_t eventId, uint64_t delayTime)
{
    if (handler_ == nullptr) {
        HILOGE("NetworkingCollect SendEvent handler is null!");
        return false;
    }
    auto task = [this, eventId] {this->ProcessEvent(eventId);};
    return handler_->PostTask(task, delayTime);
}
}  // namespace OHOS
