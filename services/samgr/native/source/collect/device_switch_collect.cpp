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
#include "system_ability_definition.h"
#include "system_ability_manager.h"

using namespace std;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr int32_t WIFI_ON = 3;
constexpr int32_t WIFI_OFF = 1;
static const std::string BLUETOOTH_NAME = "bluetooth_status";
static const std::string WIFI_NAME = "wifi_status";
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
            SystemAbilityManager::GetInstance()->SubscribeSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID, listener);
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

SwitchStateListener::SwitchStateListener(const sptr<DeviceSwitchCollect>& deviceSwitchCollect)
    : deviceSwitchCollect_(deviceSwitchCollect) {}

void SwitchStateListener::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    HILOGI("DeviceSwitchCollect OnAddSystemAbility systemAbilityId:%{public}d", systemAbilityId);
    if (systemAbilityId == COMMON_EVENT_SERVICE_ID) {
        std::shared_ptr<WifiSwitchCollect> wifiSwitchCollect = std::make_shared<WifiSwitchCollect>();
        wifiSwitchCollect->WatchState(deviceSwitchCollect_);
    } else if (systemAbilityId == BLUETOOTH_HOST_SYS_ABILITY_ID) {
        std::shared_ptr<BlueToothSwitchCollect> btSwitchCollect = std::make_shared<BlueToothSwitchCollect>();
        btSwitchCollect->WatchState(deviceSwitchCollect_);
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
    BluetoothEventSubscriber* bluetoothEventSubscriber = new BluetoothEventSubscriber(deviceSwitchCollect);
    OHOS::Bluetooth::BluetoothHost::GetDefaultHost().RegisterObserver(*bluetoothEventSubscriber);
}

BluetoothEventSubscriber::BluetoothEventSubscriber(const sptr<DeviceSwitchCollect>& deviceSwitchCollect)
    : deviceSwitchCollect_(deviceSwitchCollect) {}

void BluetoothEventSubscriber::OnStateChanged(const int transport, const int status)
{
    HILOGI("DeviceSwitchCollect OnStateChanged, %{public}d", status);
    if (transport == OHOS::Bluetooth::BTTransport::ADAPTER_BREDR) {
        return;
    }
    std::string eventValue;
    switch (status) {
        case OHOS::Bluetooth::BTStateID::STATE_TURN_ON:
            HILOGD("Bluetooth turn on");
            eventValue = "on";
            break;
        case OHOS::Bluetooth::BTStateID::STATE_TURN_OFF:
            HILOGD("Bluetooth turn off");
            eventValue = "off";
            break;
        default:
            HILOGD("invalid status");
            return;
    }
    OnDemandEvent event = {SETTING_SWITCH, BLUETOOTH_NAME, eventValue};
    if (deviceSwitchCollect_ == nullptr) {
        HILOGE("collect is nullptr");
        return;
    }
    deviceSwitchCollect_->ReportEvent(event);
}

void WifiSwitchCollect::WatchState(const sptr<DeviceSwitchCollect>& deviceSwitchCollect)
{
    EventFwk::MatchingSkills skill = EventFwk::MatchingSkills();
    skill.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_WIFI_POWER_STATE);
    EventFwk::CommonEventSubscribeInfo info(skill);
    std::shared_ptr<EventFwk::CommonEventSubscriber> wifiEventSubscriber
        = std::make_shared<WifiEventSubscriber>(info, deviceSwitchCollect);
    EventFwk::CommonEventManager::SubscribeCommonEvent(wifiEventSubscriber);
}

WifiEventSubscriber::WifiEventSubscriber(const EventFwk::CommonEventSubscribeInfo& subscribeInfo,
    const sptr<DeviceSwitchCollect>& deviceSwitchCollect)
    :EventFwk::CommonEventSubscriber(subscribeInfo), deviceSwitchCollect_(deviceSwitchCollect) {}

void WifiEventSubscriber::OnReceiveEvent(const EventFwk::CommonEventData& data)
{
    std::string action = data.GetWant().GetAction();
    if (action != EventFwk::CommonEventSupport::COMMON_EVENT_WIFI_POWER_STATE) {
        HILOGI("invalid action: %{public}s", action.c_str());
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