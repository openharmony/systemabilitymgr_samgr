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
#ifndef SYSTEM_ABILITY_MANAGER_COMMON_EVENT_COLLECT_H
#define SYSTEM_ABILITY_MANAGER_COMMON_EVENT_COLLECT_H

#include <memory>
#include <mutex>

#include "common_event_subscriber.h"
#include "event_handler.h"
#include "icollect_plugin.h"
#include "iremote_object.h"
#include "system_ability_status_change_stub.h"

namespace OHOS {
class CommonEventCollect : public ICollectPlugin {
public:
    explicit CommonEventCollect(const sptr<IReport>& report);
    ~CommonEventCollect() = default;

    int32_t OnStart() override;
    int32_t OnStop() override;
    void SaveAction(const std::string& action);
    bool CheckCondition(const OnDemandCondition& condition) override;
    int32_t AddCollectEvent(const OnDemandEvent& event) override;
    int32_t RemoveUnusedEvent(const OnDemandEvent& event) override;
    void Init(const std::list<SaProfile>& saProfiles) override;
    int64_t SaveOnDemandReasonExtraData(const EventFwk::CommonEventData& data);
    void RemoveOnDemandReasonExtraData(int64_t extraDataId);
    bool GetOnDemandReasonExtraData(int64_t extraDataId, OnDemandReasonExtraData& extraData) override;
    bool CreateCommonEventSubscriber();
    bool SendEvent(uint32_t eventId);
    std::string GetParamFromWant(const std::string& key, const AAFwk::Want& data);
    void InitCommonEventState(const OnDemandEvent& evnet);
    bool CheckExtraMessage(int64_t extraDataId, const OnDemandEvent& profileEvent) override;
    void SaveOnDemandConditionExtraData(const EventFwk::CommonEventData& data);
private:
    int64_t GenerateExtraDataIdLocked();
    bool AddCommonEventName(const std::string& eventName);
    void AddSkillsEvent(EventFwk::MatchingSkills& skill);
    void CleanFailedEventLocked(const std::string& eventName);
    std::mutex commomEventLock_;
    std::recursive_mutex commonEventSubscriberLock_;
    sptr<IRemoteObject::DeathRecipient> commonEventDeath_;
    //common event collection that require plugins to subscribe to
    std::set<std::string> commonEventNames_;
    std::shared_ptr<AppExecFwk::EventHandler> workHandler_;
    std::shared_ptr<EventFwk::CommonEventSubscriber> commonEventSubscriber_ = nullptr;
    std::mutex commonEventStateLock_;
    //Save device screen and charging status
    std::set<std::string> commonEventWhitelist;
    //Save the extra data carried by the common event as ondemand condition
    std::map<std::string, std::map<std::string, std::string>> commonEventConditionExtraData_;
    //Save the value carried by the common event as ondemand condition
    std::map<std::string, std::string> commonEventConditionValue_;
    std::mutex extraDataLock_;
    int64_t extraDataId_ = 0;
    //Save the extra data carried by the common event as ondemand event
    std::map<int64_t, OnDemandReasonExtraData> extraDatas_;
    //Save the key of extra data that SA care about in the common event as ondemand event
    std::map<std::string, std::set<std::string>> extraDataKey_;
};

class CommonEventListener : public SystemAbilityStatusChangeStub {
public:
    CommonEventListener(const sptr<CommonEventCollect>& commonEventCollect);
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    void OnRemoveSystemAbility(int32_t systemAblityId, const std::string& deviceId) override;
private:
    sptr<CommonEventCollect> commonEventCollect_;
};
class CommonHandler : public AppExecFwk::EventHandler {
    public:
        CommonHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner,
            const sptr<CommonEventCollect>& collect)
            :AppExecFwk::EventHandler(runner), commonCollect_(collect) {}
        ~CommonHandler() = default;
        void ProcessEvent(const OHOS::AppExecFwk::InnerEvent::Pointer& event) override;

    private:
        wptr<CommonEventCollect> commonCollect_;
};

class CommonEventSubscriber : public EventFwk::CommonEventSubscriber {
public:
    CommonEventSubscriber(const EventFwk::CommonEventSubscribeInfo& subscribeInfo,
        const sptr<CommonEventCollect>& collect);
    ~CommonEventSubscriber() override = default;
    void OnReceiveEvent(const EventFwk::CommonEventData& data) override;
private:
    wptr<CommonEventCollect> collect_;
};
} // namespace OHOS
#endif // SYSTEM_ABILITY_MANAGER_COMMON_EVENT_COLLECT_H