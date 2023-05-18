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
#include <mutex>
#include <set>

#include "common_event_subscriber.h"
#include "icollect_plugin.h"
#include "system_ability_status_change_stub.h"

namespace OHOS {
class DeviceSwitchCollect : public ICollectPlugin {
public:
    explicit DeviceSwitchCollect(const sptr<IReport>& report);
    ~DeviceSwitchCollect() = default;
    int32_t OnStart() override;
    int32_t OnStop() override;
    void Init(const std::list<SaProfile>& saProfiles) override;
    int32_t AddCollectEvent(const OnDemandEvent& event) override;
    void SetSwitchEvent(const OnDemandEvent& onDemandEvent);
    bool isContainSwitch(const std::string& swithes);
private:
    std::mutex switchEventLock_;
    std::set<std::string> switches_;
};

class SwitchStateListener : public SystemAbilityStatusChangeStub {
public:
    SwitchStateListener(const sptr<DeviceSwitchCollect>& deviceSwitchCollect);
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
private:
    sptr<DeviceSwitchCollect> deviceSwitchCollect_;
};

class ISwitchCollect {
public:
    virtual void WatchState(const sptr<DeviceSwitchCollect>& deviceSwitchCollect) = 0;
};

class BlueToothSwitchCollect : public ISwitchCollect {
public:
    BlueToothSwitchCollect() = default;
    virtual ~BlueToothSwitchCollect() = default;
    void WatchState(const sptr<DeviceSwitchCollect>& deviceSwitchCollect) override;
};

class WifiSwitchCollect : public ISwitchCollect {
public:
    WifiSwitchCollect() = default;
    virtual ~WifiSwitchCollect() = default;
    void WatchState(const sptr<DeviceSwitchCollect>& deviceSwitchCollect) override;
};

class BluetoothEventSubscriber : public EventFwk::CommonEventSubscriber {
public:
    BluetoothEventSubscriber(const EventFwk::CommonEventSubscribeInfo& subscribeInfo,
    const sptr<DeviceSwitchCollect>& deviceSwitchCollect):EventFwk::CommonEventSubscriber(subscribeInfo),
        deviceSwitchCollect_(deviceSwitchCollect) {}
    ~BluetoothEventSubscriber() override = default;
    void OnReceiveEvent(const EventFwk::CommonEventData& data) override;
private:
    sptr<DeviceSwitchCollect> deviceSwitchCollect_;
};

class WifiEventSubscriber : public EventFwk::CommonEventSubscriber {
public:
    WifiEventSubscriber(const EventFwk::CommonEventSubscribeInfo& subscribeInfo,
    const sptr<DeviceSwitchCollect>& deviceSwitchCollect):EventFwk::CommonEventSubscriber(subscribeInfo),
        deviceSwitchCollect_(deviceSwitchCollect) {}
    ~WifiEventSubscriber() override = default;
    void OnReceiveEvent(const EventFwk::CommonEventData& data) override;
private:
    sptr<DeviceSwitchCollect> deviceSwitchCollect_;
};
} // namespace OHOS
#endif // OHOS_SYSTEM_ABILITY_MANAGER_DEVICE_PARAM_COLLECT_H