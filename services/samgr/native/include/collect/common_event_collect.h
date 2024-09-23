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
#include "ffrt_handler.h"
#include "icollect_plugin.h"
#include "iremote_object.h"
#include "system_ability_status_change_stub.h"

namespace OHOS {
class CommonHandler;
class CommonEventCollect : public ICollectPlugin {
public:
    explicit CommonEventCollect(const sptr<IReport>& report);
    ~CommonEventCollect() = default;

    void CleanFfrt() override;
    void SetFfrt() override;
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
    bool CreateCommonEventSubscriberLocked();
    bool SendEvent(uint32_t eventId);
    std::string GetParamFromWant(const std::string& key, const AAFwk::Want& data);
    void InitCommonEventState(const OnDemandEvent& evnet);
    bool CheckExtraMessage(int64_t extraDataId, const OnDemandEvent& profileEvent) override;
    void SaveOnDemandConditionExtraData(const EventFwk::CommonEventData& data);
    void SaveSaExtraDataId(int32_t saId, int64_t extraDataId) override;
    void RemoveSaExtraDataId(int64_t extraDataId);
    void ClearSaExtraDataId(int32_t saId) override;
    void SaveCacheCommonEventSaExtraId(const OnDemandEvent& event,
        const std::list<SaControlInfo>& saControlList) override;
    int GetSaExtraDataIdList(int32_t saId, std::vector<int64_t>& extraDataIdList,
        const std::string& eventName = "") override;
    void RemoveWhiteCommonEvent() override;
private:
    int64_t GenerateExtraDataIdLocked();
    bool AddCommonEventName(const std::string& eventName);
    void AddSkillsEvent(EventFwk::MatchingSkills& skill);
    void CleanFailedEventLocked(const std::string& eventName);
    std::mutex commomEventLock_;
    std::mutex commonEventSubscriberLock_;
    sptr<IRemoteObject::DeathRecipient> commonEventDeath_;
    std::set<std::string> commonEventNames_;
    std::shared_ptr<CommonHandler> workHandler_;
    std::shared_ptr<EventFwk::CommonEventSubscriber> commonEventSubscriber_ = nullptr;
    std::mutex commonEventStateLock_;
    std::set<std::string> commonEventWhitelist;
    std::map<std::string, std::map<std::string, std::string>> commonEventConditionExtraData_;
    std::map<std::string, std::string> commonEventConditionValue_;
    std::mutex extraDataLock_;
    int64_t extraDataId_ = 0;
    std::map<int64_t, OnDemandReasonExtraData> extraDatas_;
    std::map<std::string, std::set<std::string>> extraDataKey_;
    std::mutex saExtraDataIdLock_;
    std::map<int32_t, std::list<int64_t>> saExtraDataIdMap_;
};

class CommonEventListener : public SystemAbilityStatusChangeStub {
public:
    CommonEventListener(const sptr<CommonEventCollect>& commonEventCollect);
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    void OnRemoveSystemAbility(int32_t systemAblityId, const std::string& deviceId) override;
private:
    sptr<CommonEventCollect> commonEventCollect_;
};
class CommonHandler {
    public:
        CommonHandler(const sptr<CommonEventCollect>& collect) :commonCollect_(collect)
        {
            handler_ = std::make_shared<FFRTHandler>("CommonHandler");
        }
        ~CommonHandler() = default;
        void ProcessEvent(uint32_t eventId, int64_t extraDataId);
        bool SendEvent(uint32_t eventId);
        bool SendEvent(uint32_t eventId, int64_t extraDataId, uint64_t delayTime);
        void CleanFfrt();
        void SetFfrt();
        
    private:
        wptr<CommonEventCollect> commonCollect_;
        std::shared_ptr<FFRTHandler> handler_;
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