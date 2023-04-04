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

#include "common_event_collect.h"

#include "ability_death_recipient.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "matching_skills.h"
#include "want.h"
#include "sam_log.h"
#include "sa_profiles.h"
#include "system_ability_definition.h"
#include "system_ability_manager.h"

using namespace OHOS::AppExecFwk;
namespace OHOS {
namespace {
constexpr uint32_t INIT_EVENT = 10;
constexpr uint32_t COMMON_DIED_EVENT = 11;
constexpr int64_t DELAY_TIME = 1000;
}

CommonEventCollect::CommonEventCollect(const sptr<IReport>& report)
    : ICollectPlugin(report)
{
}

int32_t CommonEventCollect::OnStart()
{
    HILOGI("CommonEventCollect OnStart called");
    if (commonEventNames_.empty()) {
        HILOGW("CommonEventCollect commonEventNames_ is empty");
        return ERR_OK;
    }
    std::shared_ptr<EventHandler> handler = EventHandler::Current();
    if (handler == nullptr) {
        HILOGW("CommonEventCollect current handler is nullptr");
        return ERR_INVALID_VALUE;
    }
    workHandler_ = std::make_shared<CommonHandler>(handler->GetEventRunner(), this);
    commonEventDeath_ = sptr<IRemoteObject::DeathRecipient>(new CommonEventDeathRecipient(workHandler_));
    workHandler_->SendEvent(INIT_EVENT);
    return ERR_OK;
}

int32_t CommonEventCollect::OnStop()
{
    if (workHandler_ != nullptr) {
        workHandler_->SetEventRunner(nullptr);
        workHandler_ = nullptr;
    }
    return ERR_OK;
}

void CommonEventCollect::Init(const std::list<SaProfile>& onDemandSaProfiles)
{
    {
        std::lock_guard<std::mutex> autoLock(commonEventStateLock_);
        commonEventState_.insert(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON);
        commonEventState_.insert(EventFwk::CommonEventSupport::COMMON_EVENT_DISCHARGING);
    }
    std::lock_guard<std::mutex> autoLock(commomEventLock_);
    commonEventNames_.push_back(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON);
    commonEventNames_.push_back(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
    commonEventNames_.push_back(EventFwk::CommonEventSupport::COMMON_EVENT_CHARGING);
    commonEventNames_.push_back(EventFwk::CommonEventSupport::COMMON_EVENT_DISCHARGING);
    for (auto& profile : onDemandSaProfiles) {
        for (auto iterStart : profile.startOnDemand.onDemandEvents) {
            if (iterStart.eventId == COMMON_EVENT) {
                commonEventNames_.push_back(iterStart.name);
            }
        }
        for (auto iterStop : profile.stopOnDemand.onDemandEvents) {
            if (iterStop.eventId == COMMON_EVENT) {
                commonEventNames_.push_back(iterStop.name);
            }
        }
    }
}

bool CommonEventCollect::AddCommonListener()
{
    if (!IsCesReady()) {
        return false;
    }
    HILOGI("CommonEventCollect AddCommonListener ces is ready");
    CreateCommonEventSubscriber();
    return EventFwk::CommonEventManager::SubscribeCommonEvent(commonEventSubscriber_);
}

void CommonEventCollect::CreateCommonEventSubscriber()
{
    EventFwk::MatchingSkills skill = EventFwk::MatchingSkills();
    {
        std::lock_guard<std::mutex> autoLock(commomEventLock_);
        for (auto& commonEventName : commonEventNames_) {
            HILOGD("CommonEventCollect add event: %{puhlic}s", commonEventName.c_str());
            skill.AddEvent(commonEventName);
        }
    }
    EventFwk::CommonEventSubscribeInfo info(skill);
    commonEventSubscriber_ = std::make_shared<CommonEventSubscriber>(info, this);
}

bool CommonEventCollect::IsCesReady()
{
    auto cesProxy = SystemAbilityManager::GetInstance()->CheckSystemAbility(
        COMMON_EVENT_SERVICE_ID);
    if (cesProxy != nullptr) {
        IPCObjectProxy* proxy = reinterpret_cast<IPCObjectProxy*>(cesProxy.GetRefPtr());
        if (commonEventDeath_ != nullptr) {
            cesProxy->AddDeathRecipient(commonEventDeath_);
        }
        // make sure the proxy is not dead
        if (proxy != nullptr && !proxy->IsObjectDead()) {
            return true;
        }
    }
    return false;
}

void CommonEventCollect::SaveAction(const std::string& action)
{
    std::lock_guard<std::mutex> autoLock(commonEventStateLock_);
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON) {
        commonEventState_.insert(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON);
        commonEventState_.erase(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF) {
        commonEventState_.insert(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
        commonEventState_.erase(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON);
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_CHARGING) {
        commonEventState_.insert(EventFwk::CommonEventSupport::COMMON_EVENT_CHARGING);
        commonEventState_.erase(EventFwk::CommonEventSupport::COMMON_EVENT_DISCHARGING);
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_DISCHARGING) {
        commonEventState_.insert(EventFwk::CommonEventSupport::COMMON_EVENT_DISCHARGING);
        commonEventState_.erase(EventFwk::CommonEventSupport::COMMON_EVENT_CHARGING);
    }
}

bool CommonEventCollect::CheckCondition(const OnDemandCondition& condition)
{
    std::lock_guard<std::mutex> autoLock(commonEventStateLock_);
    return commonEventState_.count(condition.name) > 0;
}

void CommonHandler::ProcessEvent(const InnerEvent::Pointer& event)
{
    if (commonCollect_ == nullptr || event == nullptr) {
        HILOGE("CommonEventCollect ProcessEvent collect or event is null!");
        return;
    }
    auto eventId = event->GetInnerEventId();
    if (eventId != INIT_EVENT && eventId != COMMON_DIED_EVENT) {
        HILOGE("CommonEventCollect ProcessEvent error event code!");
        return;
    }
    auto commonCollect = commonCollect_.promote();
    if (commonCollect == nullptr) {
        HILOGE("CommonEventCollect collect is nullptr");
        return;
    }
    if (!commonCollect->AddCommonListener()) {
        HILOGW("CommonEventCollect AddCommonListener retry");
        SendEvent(INIT_EVENT, DELAY_TIME);
        return;
    }
    HILOGI("CommonEventCollect AddCommonListener success");
}

CommonEventSubscriber::CommonEventSubscriber(const EventFwk::CommonEventSubscribeInfo& subscribeInfo,
    const sptr<CommonEventCollect>& collect)
    :EventFwk::CommonEventSubscriber(subscribeInfo), collect_(collect) {}

void CommonEventSubscriber::OnReceiveEvent(const EventFwk::CommonEventData& data)
{
    std::string action = data.GetWant().GetAction();
    HILOGI("OnReceiveEvent get action: %{public}s", action.c_str());
    OnDemandEvent event = {COMMON_EVENT, action, ""};
    auto collect = collect_.promote();
    if (collect == nullptr) {
        HILOGE("CommonEventCollect collect is nullptr");
        return;
    }
    collect->SaveAction(action);
    collect->ReportEvent(event);
}

void CommonEventDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    HILOGI("CommonEventDeathRecipient called!");
    if (handler_ != nullptr) {
        handler_->SendEvent(COMMON_DIED_EVENT, DELAY_TIME);
    }
}
} // namespace OHOS