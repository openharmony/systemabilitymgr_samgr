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

void DeviceSwitchCollect::InitCommonEventSubscriber()
{
    EventFwk::MatchingSkills skill = EventFwk::MatchingSkills();
    skill.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_WIFI_POWER_STATE);
    skill.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE);
    EventFwk::CommonEventSubscribeInfo info(skill);
    switchEventSubscriber_ = std::make_shared<SwitchEventSubscriber>(info, this);
    cesStateListener_ = new CesStateListener(this);
}

int32_t DeviceSwitchCollect::SubscribeSwitchEvent()
{
    if (switchEventSubscriber_ == nullptr) {
        HILOGE("DeviceSwitchCollect switchEventSubscriber is nullptr");
        return ERR_INVALID_VALUE;
    }
    return switchEventSubscriber_->SubscribeSwitchEvent();
}

int32_t DeviceSwitchCollect::CheckSwitchEvent(const OnDemandEvent& onDemandEvent)
{
    if (onDemandEvent.eventId != SETTING_SWITCH) {
        return ERR_INVALID_VALUE;
    }
    if (onDemandEvent.name != WIFI_NAME && onDemandEvent.name != BLUETOOTH_NAME) {
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

void DeviceSwitchCollect::Init(const std::list<SaProfile>& saProfiles)
{
    HILOGI("DeviceSwitchCollect Init called");
    InitCommonEventSubscriber();
    for (auto saProfile : saProfiles) {
        for (auto onDemandEvent : saProfile.startOnDemand.onDemandEvents) {
            if (CheckSwitchEvent(onDemandEvent) == ERR_OK) {
                needListenSwitchEvent_ = true;
                return;
            }
        }
        for (auto onDemandEvent : saProfile.stopOnDemand.onDemandEvents) {
            if (CheckSwitchEvent(onDemandEvent) == ERR_OK) {
                needListenSwitchEvent_ = true;
                return;
            }
        }
    }
}

int32_t DeviceSwitchCollect::OnStart()
{
    HILOGI("DeviceSwitchCollect OnStart called");
    if (!needListenSwitchEvent_) {
        return ERR_OK;
    }
    return SystemAbilityManager::GetInstance()->SubscribeSystemAbility(COMMON_EVENT_SERVICE_ID,
        cesStateListener_);
}

int32_t DeviceSwitchCollect::OnStop()
{
    HILOGI("DeviceSwitchCollect OnStop called");
    if (cesStateListener_ != nullptr) {
        SystemAbilityManager::GetInstance()->UnSubscribeSystemAbility(COMMON_EVENT_SERVICE_ID, cesStateListener_);
    }
    if (switchEventSubscriber_ != nullptr) {
        switchEventSubscriber_->UnSubscribeSwitchEvent();
    }
    return ERR_OK;
}

int32_t DeviceSwitchCollect::AddCollectEvent(const OnDemandEvent& event)
{
    if (CheckSwitchEvent(event) != ERR_OK) {
        HILOGE("DeviceSwitchCollect invalid event name %{public}s!", event.name.c_str());
        return ERR_INVALID_VALUE;
    }
    return SubscribeSwitchEvent();
}

void CesStateListener::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    HILOGI("DeviceSwitchCollect OnAddSystemAbility systemAbilityId:%{public}d", systemAbilityId);
    if (systemAbilityId != COMMON_EVENT_SERVICE_ID) {
        HILOGE("DeviceSwitchCollect OnAddSystemAbility unhandled sysabilityId:%{public}d", systemAbilityId);
        return;
    }
    auto deviceSwitchCollect = deviceSwitchCollect_.promote();
    if (deviceSwitchCollect == nullptr) {
        HILOGE("DeviceSwitchCollect switchEventSubscriber is nullptr");
        return;
    }
    auto task = [this] () {
        auto deviceSwitchCollect = deviceSwitchCollect_.promote();
        if (deviceSwitchCollect == nullptr) {
            HILOGE("DeviceSwitchCollect switchEventSubscriber is nullptr");
            return;
        }
        deviceSwitchCollect->SubscribeSwitchEvent();
    };
    deviceSwitchCollect->PostDelayTask(task, 0);
}

void CesStateListener::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    HILOGI("DeviceSwitchCollect OnRemoveSystemAbility systemAbilityId:%{public}d", systemAbilityId);
}

int32_t SwitchEventSubscriber::SubscribeSwitchEvent()
{
    HILOGI("DeviceSwitchCollect Subscribe Switch State");
    std::lock_guard<std::mutex> autoLock(isListenEventLock_);
    if (isListenSwitchEvent_) {
        return ERR_OK;
    }
    if (EventFwk::CommonEventManager::SubscribeCommonEvent(shared_from_this())) {
        isListenSwitchEvent_ = true;
        return ERR_OK;
    }
    return ERR_INVALID_VALUE;
}

int32_t SwitchEventSubscriber::UnSubscribeSwitchEvent()
{
    HILOGI("DeviceSwitchCollect UnSubscribe Switch State");
    std::lock_guard<std::mutex> autoLock(isListenEventLock_);
    if (!isListenSwitchEvent_) {
        return ERR_OK;
    }
    if (EventFwk::CommonEventManager::UnSubscribeCommonEvent(shared_from_this())) {
        isListenSwitchEvent_ = false;
        return ERR_OK;
    }
    return ERR_INVALID_VALUE;
}

void SwitchEventSubscriber::ReportEvent(const OnDemandEvent& event)
{
    auto deviceSwitchCollect = deviceSwitchCollect_.promote();
    if (deviceSwitchCollect == nullptr) {
        HILOGE("DeviceSwitchCollect is nullptr");
        return;
    }
    deviceSwitchCollect->ReportEvent(event);
}

void SwitchEventSubscriber::OnReceiveWifiEvent(const EventFwk::CommonEventData& data)
{
    std::string eventValue;
    int32_t code = data.GetCode();
    HILOGI("DeviceSwitchCollect wifi state changed, code %{public}d", code);
    if (code == WIFI_ON) {
        eventValue = "on";
    } else if (code == WIFI_OFF) {
        eventValue = "off";
    } else {
        return;
    }
    OnDemandEvent event = {SETTING_SWITCH, WIFI_NAME, eventValue};
    ReportEvent(event);
}

void SwitchEventSubscriber::OnReceiveBluetoothEvent(const EventFwk::CommonEventData& data)
{
    std::string eventValue;
    int32_t code = data.GetCode();
    HILOGI("DeviceSwitchCollect bluetooth state changed, code %{public}d", code);
    if (code == BLUETOOTH_STATE_TURN_ON) {
        eventValue = "on";
    } else if (code == BLUETOOTH_STATE_TURN_OFF) {
        eventValue = "off";
    } else {
        return;
    }
    OnDemandEvent event = {SETTING_SWITCH, BLUETOOTH_NAME, eventValue};
    ReportEvent(event);
}

void SwitchEventSubscriber::OnReceiveEvent(const EventFwk::CommonEventData& data)
{
    std::string action = data.GetWant().GetAction();
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_WIFI_POWER_STATE) {
        OnReceiveWifiEvent(data);
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE) {
        OnReceiveBluetoothEvent(data);
    } else {
        HILOGE("DeviceSwitchCollect invalid action: %{public}s", action.c_str());
    }
}
}