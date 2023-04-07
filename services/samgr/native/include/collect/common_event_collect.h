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

namespace OHOS {
class CommonEventCollect : public ICollectPlugin {
public:
    explicit CommonEventCollect(const sptr<IReport>& report);
    ~CommonEventCollect() = default;

    int32_t OnStart() override;
    int32_t OnStop() override;
    void SaveAction(const std::string& action);
    bool CheckCondition(const OnDemandCondition& condition) override;
    void Init(const std::list<SaProfile>& saProfiles);
    bool AddCommonListener();
    int64_t SaveOnDemandReasonExtraData(const EventFwk::CommonEventData& data);
    void RemoveOnDemandReasonExtraData(int64_t extraDataId);
    bool GetOnDemandReasonExtraData(int64_t extraDataId, OnDemandReasonExtraData& extraData) override;
private:
    bool IsCesReady();
    void CreateCommonEventSubscriber();
    int64_t GenerateExtraDataIdLocked();
    std::mutex commomEventLock_;
    sptr<IRemoteObject::DeathRecipient> commonEventDeath_;
    std::vector<std::string> commonEventNames_;
    std::shared_ptr<AppExecFwk::EventHandler> workHandler_;
    std::shared_ptr<EventFwk::CommonEventSubscriber> commonEventSubscriber_ = nullptr;
    std::mutex commonEventStateLock_;
    std::set<std::string> commonEventState_;
    std::mutex extraDataLock_;
    int64_t extraDataId_ = 0;
    std::map<int64_t, OnDemandReasonExtraData> extraDatas_;
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

class CommonEventDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    void OnRemoteDied(const wptr<IRemoteObject>& remote) override;
    explicit CommonEventDeathRecipient(const std::shared_ptr<AppExecFwk::EventHandler>& handler) : handler_(handler) {}
    ~CommonEventDeathRecipient() override = default;
private:
    std::shared_ptr<AppExecFwk::EventHandler> handler_;
};
} // namespace OHOS
#endif // SYSTEM_ABILITY_MANAGER_COMMON_EVENT_COLLECT_H