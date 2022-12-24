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
#include "system_ability_definition.h"
#include "system_ability_manager.h"

using namespace std;

using namespace OHOS::AppExecFwk;
using namespace OHOS::DistributedHardware;

namespace OHOS {
namespace {
const std::string PKG_NAME = "Samgr_Networking";
const std::string SA_TAG_DEVICE_ON_LINE = "deviceonline";
constexpr uint32_t INIT_EVENT = 10;
constexpr uint32_t DM_DIED_EVENT = 11;
constexpr int64_t DELAY_TIME = 1000;
}
DeviceNetworkingCollect::DeviceNetworkingCollect(const sptr<IReport>& report)
    : ICollectPlugin(report)
{
}

int32_t DeviceNetworkingCollect::OnStart()
{
    HILOGI("DeviceNetworkingCollect OnStart called");
    std::shared_ptr<EventHandler> handler = EventHandler::Current();
    if (handler == nullptr) {
        return ERR_INVALID_VALUE;
    }
    workHandler_ = std::make_shared<WorkHandler>(handler->GetEventRunner(), this);
    initCallback_ = std::make_shared<DeviceInitCallBack>(workHandler_);
    stateCallback_ = std::make_shared<DeviceStateCallback>(this);
    workHandler_->SendEvent(INIT_EVENT);
    return ERR_OK;
}

int32_t DeviceNetworkingCollect::OnStop()
{
    DeviceManager::GetInstance().UnRegisterDevStateCallback(PKG_NAME);
    if (workHandler_ != nullptr) {
        workHandler_->SetEventRunner(nullptr);
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

bool DeviceNetworkingCollect::AddDeviceChangeListener()
{
    HILOGI("DeviceNetworkingCollect AddDeviceChangeListener called");
    if (IsDmReady()) {
        int32_t ret = DeviceManager::GetInstance().InitDeviceManager(PKG_NAME, initCallback_);
        if (ret != ERR_OK) {
            HILOGE("DeviceNetworkingCollect InitDeviceManager error");
            return false;
        }
        ret = DeviceManager::GetInstance().RegisterDevStateCallback(PKG_NAME, "", stateCallback_);
        if (ret != ERR_OK) {
            DeviceManager::GetInstance().UnRegisterDevStateCallback(PKG_NAME);
            HILOGE("DeviceNetworkingCollect RegisterDevStateCallback error");
            return false;
        }
        return true;
    }
    return false;
}

void DeviceNetworkingCollect::ClearDeviceOnlineSet()
{
    if (stateCallback_ != nullptr) {
        stateCallback_->ClearDeviceOnlineSet();
    }
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
    HILOGI("DeviceNetworkingCollect DeviceStateCallback OnDeviceOnline");
    bool isOnline = false;
    {
        lock_guard<mutex> autoLock(deviceOnlineLock_);
        isOnline = deviceOnlineSet_.empty();
        deviceOnlineSet_.emplace(deviceInfo.deviceId);
    }
    if (isOnline) {
        OnDemandEvent event = { DEVICE_ONLINE, SA_TAG_DEVICE_ON_LINE, "on" };
        if (collect_ != nullptr) {
            collect_->ReportEvent(event);
        }
    }
}

void DeviceStateCallback::OnDeviceOffline(const DmDeviceInfo& deviceInfo)
{
    HILOGI("DeviceNetworkingCollect DeviceStateCallback OnDeviceOffline");
    bool isOffline = false;
    {
        lock_guard<mutex> autoLock(deviceOnlineLock_);
        deviceOnlineSet_.erase(deviceInfo.deviceId);
        isOffline = deviceOnlineSet_.empty();
    }
    if (isOffline) {
        OnDemandEvent event = { DEVICE_ONLINE, SA_TAG_DEVICE_ON_LINE, "off" };
        if (collect_ != nullptr) {
            collect_->ReportEvent(event);
        }
    }
}

void DeviceStateCallback::ClearDeviceOnlineSet()
{
    lock_guard<mutex> autoLock(deviceOnlineLock_);
    deviceOnlineSet_.clear();
}

void DeviceStateCallback::OnDeviceChanged(const DmDeviceInfo& deviceInfo)
{
    HILOGD("DeviceNetworkingCollect OnDeviceChanged called");
}

void DeviceStateCallback::OnDeviceReady(const DmDeviceInfo& deviceInfo)
{
    HILOGD("DeviceNetworkingCollect OnDeviceReady called");
}

void WorkHandler::ProcessEvent(const InnerEvent::Pointer& event)
{
    if (collect_ == nullptr || event == nullptr) {
        HILOGE("DeviceNetworkingCollect ProcessEvent collect or event is null!");
        return;
    }
    auto eventId = event->GetInnerEventId();
    if (eventId != INIT_EVENT && eventId != DM_DIED_EVENT) {
        HILOGE("DeviceNetworkingCollect ProcessEvent error event code!");
        return;
    }
    if (eventId == DM_DIED_EVENT) {
        collect_->ClearDeviceOnlineSet();
    }
    if (!collect_->AddDeviceChangeListener()) {
        HILOGW("DeviceNetworkingCollect AddDeviceChangeListener retry");
        SendEvent(INIT_EVENT, DELAY_TIME);
    }
    HILOGI("DeviceNetworkingCollect AddDeviceChangeListener success");
}
}  // namespace OHOS
