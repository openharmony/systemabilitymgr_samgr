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

#ifndef OHOS_SYSTEM_ABILITY_MANAGER_DEVICE_NETWORKING_COLLECT_H
#define OHOS_SYSTEM_ABILITY_MANAGER_DEVICE_NETWORKING_COLLECT_H
#include "icollect_plugin.h"

#include <mutex>
#include <set>

#include "device_manager.h"
#include "event_handler.h"

namespace OHOS {
class DeviceStateCallback : public DistributedHardware::DeviceStateCallback {
    public:
        explicit DeviceStateCallback(const sptr<ICollectPlugin>& collect) : collect_(collect) {}
        ~DeviceStateCallback() = default;
        void OnDeviceOnline(const DistributedHardware::DmDeviceInfo& deviceInfo) override;
        void OnDeviceOffline(const DistributedHardware::DmDeviceInfo& deviceInfo) override;
        void OnDeviceChanged(const DistributedHardware::DmDeviceInfo& deviceInfo) override;
        void OnDeviceReady(const DistributedHardware::DmDeviceInfo& deviceInfo) override;
        void ClearDeviceOnlineSet();
        bool IsOnline();
        void UpdateDeviceOnlineSet(const std::string& deviceId);
    private:
       sptr<ICollectPlugin> collect_;
       std::set<std::string> deviceOnlineSet_;
       bool isExistDeviceReady_ = false;
       std::mutex deviceOnlineLock_;
};

class DeviceInitCallBack : public DistributedHardware::DmInitCallback {
    public:
        explicit DeviceInitCallBack(const std::shared_ptr<AppExecFwk::EventHandler>& handler) : handler_(handler) {}
        ~DeviceInitCallBack() = default;
        void OnRemoteDied() override;
    private:
        std::shared_ptr<AppExecFwk::EventHandler> handler_;
};

class DeviceNetworkingCollect : public ICollectPlugin {
public:
    explicit DeviceNetworkingCollect(const sptr<IReport>& report);
    ~DeviceNetworkingCollect() = default;

    int32_t OnStart() override;
    int32_t OnStop() override;
    bool CheckCondition(const OnDemandCondition& condition) override;
    bool AddDeviceChangeListener();
private:
    std::shared_ptr<DeviceStateCallback> stateCallback_;
    std::shared_ptr<DistributedHardware::DmInitCallback> initCallback_;
    std::shared_ptr<AppExecFwk::EventHandler> workHandler_;

    bool IsDmReady();
    void ClearDeviceOnlineSet();
    bool IsOnline();
    void UpdateDeviceOnlineSet(const std::string& deviceId);
    bool ReportMissedEvents();
};

class WorkHandler : public AppExecFwk::EventHandler {
    public:
        WorkHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner,
            const sptr<DeviceNetworkingCollect>& collect) : AppExecFwk::EventHandler(runner), collect_(collect) {}
        ~WorkHandler() = default;
        void ProcessEvent(const OHOS::AppExecFwk::InnerEvent::Pointer& event) override;

    private:
        sptr<DeviceNetworkingCollect> collect_;
};
} // namespace OHOS
#endif // OHOS_SYSTEM_ABILITY_MANAGER_DEVICE_NETWORKING_COLLECT_H