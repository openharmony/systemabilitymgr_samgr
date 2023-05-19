/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *e
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "device_switch_collect.h"

#include "common_event_manager.h"
#include "common_event_support.h"
#include "matching_skills.h"
#include "sa_profiles.h"
#include "sam_log.h"
#include "system_ability_manager.h"

using namespace std;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
const std::string BLUETOOTH_NAME = "bluetooth_status";
const std::string WIFI_NAME = "wifi_status";
constexpr int32_t WIFI_ON = 3;
constexpr int32_t WIFI_OFF = 1;
constexpr int32_t BLUETOOTH_STATE_TURN_ON = 1;
constexpr int32_t BLUETOOTH_STATE_TURN_OFF = 3;
constexpr int32_t COMMON_EVENT_SERVICE_ID = 3299;
}

DeviceSwitchCollect::DeviceSwitchCollect(const sptr<IReport>& report)
    : ICollectPlugin(report)
{
}

void DeviceSwitchCollect::Init(const std::list<SaProfile>& saProfiles)
{
    HILOGI("DeviceSwitchCollect Init begin");
    for (auto saProfile : saProfiles) {
        for (auto onDemandEvent : saProfile.startOnDemand.onDemandEvents) {
            SetSwitchEvent(onDemandEvent);
        }
        for (auto onDemandEvent : saProfile.stopOnDemand.onDemandEvents) {
            SetSwitchEvent(onDemandEvent);
        }
    }
}

void DeviceSwitchCollect::SetSwitchEvent(const OnDemandEvent& onDemandEvent)
{
    if (onDemandEvent.eventId != SETTING_SWITCH) {
        return;
    }
    if (onDemandEvent.name == WIFI_NAME || onDemandEvent.name == BLUETOOTH_NAME) {
        std::lock_guard<std::mutex> autoLock(switchEventLock_);
        switches_.insert(onDemandEvent.name);
    }
}

int32_t DeviceSwitchCollect::OnStart()
{
    HILOGI("DeviceSwitchCollect OnStart called");
    sptr<SwitchStateListener> listener = new SwitchStateListener(this);
    std::lock_guard<std::mutex> autoLock(switchEventLock_);
    for (auto switchItem : switches_) {
        if (switchItem == WIFI_NAME) {
            SystemAbilityManager::GetInstance()->SubscribeSystemAbility(COMMON_EVENT_SERVICE_ID, listener);
        } else if (switchItem == BLUETOOTH_NAME) {
            SystemAbilityManager::GetInstance()->SubscribeSystemAbility(COMMON_EVENT_SERVICE_ID, listener);
        } else {
            HILOGI("invalid item!");
        }
    }
    return ERR_OK;
}

int32_t DeviceSwitchCollect::OnStop()
{
    HILOGI("DeviceSwitchCollect OnStop called");
    return ERR_OK;
}

int32_t DeviceSwitchCollect::AddCollectEvent(const OnDemandEvent& event)
{
    std::lock_guard<std::mutex> autoLock(switchEventLock_);
    auto iter = switches_.find(event.name);
    if (iter != switches_.end()) {
        return ERR_OK;
    }
    HILOGI("DeviceSwitchCollect add collect events: %{public}s", event.name.c_str());
    int32_t result = ERR_OK;
    sptr<SwitchStateListener> listener = new SwitchStateListener(this);
    if (event.name == WIFI_NAME) {
        result = SystemAbilityManager::GetInstance()->SubscribeSystemAbility(COMMON_EVENT_SERVICE_ID, listener);
    } else if (event.name == BLUETOOTH_NAME) {
        result = SystemAbilityManager::GetInstance()->SubscribeSystemAbility(COMMON_EVENT_SERVICE_ID, listener);
    } else {
        HILOGE("invalid event name %{public}s!", event.name.c_str());
        result = ERR_INVALID_VALUE;
    }
    if (result != ERR_OK) {
        return result;
    }
    switches_.insert(event.name);
    return ERR_OK;
}

bool DeviceSwitchCollect::isContainSwitch(const std::string& switches)
{
    std::lock_guard<std::mutex> autoLock(switchEventLock_);
    return switches_.find(switches) != switches_.end();
}

SwitchStateListener::SwitchStateListener(const sptr<DeviceSwitchCollect>& deviceSwitchCollect)
    : deviceSwitchCollect_(deviceSwitchCollect) {}

void SwitchStateListener::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    HILOGI("DeviceSwitchCollect OnAddSystemAbility systemAbilityId:%{public}d", systemAbilityId);
    if (systemAbilityId == COMMON_EVENT_SERVICE_ID) {
        HILOGI("OnAddSystemAbility COMMON_EVENT_SERVICE_ID!");
        if (deviceSwitchCollect_->isContainSwitch(WIFI_NAME)) {
            std::shared_ptr<WifiSwitchCollect> wifiSwitchCollect = std::make_shared<WifiSwitchCollect>();
            wifiSwitchCollect->WatchState(deviceSwitchCollect_);
        }
        if (deviceSwitchCollect_->isContainSwitch(BLUETOOTH_NAME)) {
            std::shared_ptr<BlueToothSwitchCollect> btSwitchCollect = std::make_shared<BlueToothSwitchCollect>();
            btSwitchCollect->WatchState(deviceSwitchCollect_);
        }
    } else {
        HILOGI("DeviceSwitchCollect OnAddSystemAbility unhandled sysabilityId:%{public}d", systemAbilityId);
    }
}

void SwitchStateListener::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    HILOGI("DeviceSwitchCollect OnRemoveSystemAbility systemAbilityId:%{public}d", systemAbilityId);
}

void BlueToothSwitchCollect::WatchState(const sptr<DeviceSwitchCollect>& deviceSwitchCollect)
{
    HILOGI("DeviceSwitchCollect Watch bluetooth state");
    EventFwk::MatchingSkills skill = EventFwk::MatchingSkills();
    skill.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE);
    EventFwk::CommonEventSubscribeInfo info(skill);
    std::shared_ptr<EventFwk::CommonEventSubscriber> bluetoothEventSubscriber
        = std::make_shared<BluetoothEventSubscriber>(info, deviceSwitchCollect);
    EventFwk::CommonEventManager::SubscribeCommonEvent(bluetoothEventSubscriber);
}

void BluetoothEventSubscriber::OnReceiveEvent(const EventFwk::CommonEventData& data)
{
    HILOGI("DeviceSwitchCollect Bluetooth state changed");
    std::string action = data.GetWant().GetAction();
    if (action != EventFwk::CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE) {
        HILOGE("invalid action: %{public}s", action.c_str());
        return;
    }
    std::string eventValue;
    int32_t code = data.GetCode();
    if (code ==  BLUETOOTH_STATE_TURN_ON) {
        HILOGD("Bluetooth turn on");
        eventValue = "on";
    } else if (code ==  BLUETOOTH_STATE_TURN_OFF) {
        HILOGD("Bluetooth turn off");
        eventValue = "off";
    } else {
        HILOGE("value error!");
        return;
    }
    OnDemandEvent event = {SETTING_SWITCH, BLUETOOTH_NAME, eventValue};
    if (deviceSwitchCollect_ == nullptr) {
        HILOGE("deviceSwitchCollect is nullptr");
        return;
    }
    deviceSwitchCollect_->ReportEvent(event);
}

void WifiSwitchCollect::WatchState(const sptr<DeviceSwitchCollect>& deviceSwitchCollect)
{
    HILOGI("DeviceSwitchCollect Watch wifi state");
    EventFwk::MatchingSkills skill = EventFwk::MatchingSkills();
    skill.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_WIFI_POWER_STATE);
    EventFwk::CommonEventSubscribeInfo info(skill);
    std::shared_ptr<EventFwk::CommonEventSubscriber> wifiEventSubscriber
        = std::make_shared<WifiEventSubscriber>(info, deviceSwitchCollect);
    EventFwk::CommonEventManager::SubscribeCommonEvent(wifiEventSubscriber);
}

void WifiEventSubscriber::OnReceiveEvent(const EventFwk::CommonEventData& data)
{
    HILOGI("DeviceSwitchCollect Wifi state changed");
    std::string action = data.GetWant().GetAction();
    if (action != EventFwk::CommonEventSupport::COMMON_EVENT_WIFI_POWER_STATE) {
        HILOGE("invalid action: %{public}s", action.c_str());
        return;
    }
    std::string eventValue;
    int32_t code = data.GetCode();
    if (code == WIFI_ON) {
        HILOGD("Wifi turn on");
        eventValue = "on";
    } else if (code == WIFI_OFF) {
        HILOGD("Wifi turn off");
        eventValue = "off";
    } else {
        HILOGE("value error!");
        return;
    }
    OnDemandEvent event = {SETTING_SWITCH, WIFI_NAME, eventValue};
    if (deviceSwitchCollect_ == nullptr) {
        HILOGE("deviceSwitchCollect is nullptr");
        return;
    }
    deviceSwitchCollect_->ReportEvent(event);
}
}