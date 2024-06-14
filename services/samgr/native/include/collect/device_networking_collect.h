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
#include "ffrt_handler.h"

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
       std::mutex deviceOnlineLock_;
};

class WorkHandler;
class DeviceInitCallBack : public DistributedHardware::DmInitCallback {
    public:
        explicit DeviceInitCallBack(const std::shared_ptr<WorkHandler>& handler) : handler_(handler) {}
        ~DeviceInitCallBack() = default;
        void OnRemoteDied() override;
    private:
        std::shared_ptr<WorkHandler> handler_;
};

class DeviceNetworkingCollect : public ICollectPlugin {
public:
    explicit DeviceNetworkingCollect(const sptr<IReport>& report);
    ~DeviceNetworkingCollect() = default;

    void CleanFfrt() override;
    void SetFfrt() override;
    int32_t OnStart() override;
    int32_t OnStop() override;
    bool CheckCondition(const OnDemandCondition& condition) override;
    bool AddDeviceChangeListener();
private:
    std::shared_ptr<DeviceStateCallback> stateCallback_;
    std::shared_ptr<DistributedHardware::DmInitCallback> initCallback_;
    std::shared_ptr<WorkHandler> workHandler_;

    bool IsDmReady();
    void ClearDeviceOnlineSet();
    bool IsOnline();
    void UpdateDeviceOnlineSet(const std::string& deviceId);
    bool ReportMissedEvents();
};

class WorkHandler {
    public:
        WorkHandler(const sptr<DeviceNetworkingCollect>& collect) : collect_(collect)
        {
            handler_ = std::make_shared<FFRTHandler>("WorkHandler");
        }
        ~WorkHandler() = default;
        void ProcessEvent(uint32_t eventId);
        bool SendEvent(uint32_t eventId);
        bool SendEvent(uint32_t eventId, uint64_t delayTime);
        void CleanFfrt();
        void SetFfrt();
        
    private:
        sptr<DeviceNetworkingCollect> collect_;
        std::shared_ptr<FFRTHandler> handler_;
};
} // namespace OHOS
#endif // OHOS_SYSTEM_ABILITY_MANAGER_DEVICE_NETWORKING_COLLECT_H