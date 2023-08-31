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

#ifndef OHOS_SYSTEM_ABILITY_MANAGER_DEVICE_SWITCH_COLLECT_H
#define OHOS_SYSTEM_ABILITY_MANAGER_DEVICE_SWITCH_COLLECT_H

#include <memory>

#include "common_event_subscriber.h"
#include "icollect_plugin.h"
#include "system_ability_status_change_stub.h"

namespace OHOS {
class DeviceSwitchCollect;
class SwitchEventSubscriber : public EventFwk::CommonEventSubscriber,
    public std::enable_shared_from_this<SwitchEventSubscriber> {
public:
    SwitchEventSubscriber(const EventFwk::CommonEventSubscribeInfo& subscribeInfo,
        const sptr<DeviceSwitchCollect>& deviceSwitchCollect)
        : EventFwk::CommonEventSubscriber(subscribeInfo),
        deviceSwitchCollect_(deviceSwitchCollect) {};
    virtual ~SwitchEventSubscriber() = default;
    int32_t SubscribeSwitchEvent();
    int32_t UnSubscribeSwitchEvent();
    void OnReceiveEvent(const EventFwk::CommonEventData& data) override;
private:
    void OnReceiveWifiEvent(const EventFwk::CommonEventData& data);
    void OnReceiveBluetoothEvent(const EventFwk::CommonEventData& data);
    void ReportEvent(const OnDemandEvent& event);
    std::mutex isListenEventLock_;
    bool isListenSwitchEvent_ = false;
    wptr<DeviceSwitchCollect> deviceSwitchCollect_;
};

class CesStateListener : public SystemAbilityStatusChangeStub {
public:
    CesStateListener(const sptr<DeviceSwitchCollect>& deviceSwitchCollect)
        : deviceSwitchCollect_(deviceSwitchCollect) {};
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
private:
    wptr<DeviceSwitchCollect> deviceSwitchCollect_;
};

class DeviceSwitchCollect : public ICollectPlugin {
public:
    explicit DeviceSwitchCollect(const sptr<IReport>& report);
    ~DeviceSwitchCollect() = default;
    void Init(const std::list<SaProfile>& saProfiles) override;
    int32_t OnStart() override;
    int32_t OnStop() override;
    int32_t AddCollectEvent(const OnDemandEvent& event) override;
    int32_t SubscribeSwitchEvent();
private:
    void InitCommonEventSubscriber();
    int32_t CheckSwitchEvent(const OnDemandEvent& onDemandEvent);
    std::shared_ptr<SwitchEventSubscriber> switchEventSubscriber_;
    sptr<CesStateListener> cesStateListener_;
    std::atomic<bool> needListenSwitchEvent_ {false};
};
} // namespace OHOS
#endif // OHOS_SYSTEM_ABILITY_MANAGER_DEVICE_PARAM_COLLECT_H