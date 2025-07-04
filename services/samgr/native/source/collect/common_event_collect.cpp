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

#include <cinttypes>
#include <fstream>
#include <sys/sysinfo.h>
#include "securec.h"

#include "common_event_collect.h"

#include "datetime_ex.h"
#include "ability_death_recipient.h"
#include "system_ability_manager_util.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "ipc_skeleton.h"
#include "matching_skills.h"
#include "parse_util.h"
#include "want.h"
#include "sam_log.h"
#include "sa_profiles.h"
#include "system_ability_manager.h"
#include "samgr_xcollie.h"

using namespace OHOS::AppExecFwk;
namespace OHOS {
namespace {
constexpr uint32_t INIT_EVENT = 10;
constexpr uint32_t SUB_COMMON_EVENT = 11;
constexpr uint32_t REMOVE_EXTRA_DATA_EVENT = 12;
constexpr uint32_t REMOVE_EXTRA_DATA_DELAY_TIME = 300000;
constexpr uint32_t UNSUB_DELAY_TIME = 10 * 1000;
constexpr int64_t MAX_EXTRA_DATA_ID = 1000000000;
constexpr int32_t COMMON_EVENT_SERVICE_ID = 3299;
constexpr int32_t TRIGGER_THREAD_RECLAIM_DELAY_TIME = 130;
constexpr int32_t TRIGGER_THREAD_RECLAIM_DURATION_TIME = 2;
constexpr int32_t CPU_STAT_MIN_FIELDS = 7;
constexpr int32_t CPU_STAT_CHECK_INTERVAL = 5;
constexpr int32_t CPU_LOAD_SHIFT = 16;
constexpr int32_t CPU_LOAD_CHECK_INTERVAL = 300;
constexpr float CPU_LOAD_INVALID = 0.0f;
constexpr float CPU_LOAD_IDLE_THRESHOLD = 10.0f;
constexpr float CPU_LOAD_PERCENT = 100.0f;
constexpr const char* CPU_STAT_INFO = "/proc/stat";
constexpr const char* UID = "uid";
constexpr const char* NET_TYPE = "NetType";
constexpr const char* BUNDLE_NAME = "bundleName";
constexpr const char* COMMON_EVENT_ACTION_NAME = "common_event_action_name";
constexpr const char* COMMON_RECENT_EVENT = "RECENT_EVENT";
constexpr const char* COMMON_RECENT_CLEAR_ALL = "RECENT_CLEAR_ALL";
}

CommonEventCollect::CommonEventCollect(const sptr<IReport>& report)
    : ICollectPlugin(report)
{
}

void CommonEventCollect::RemoveWhiteCommonEvent()
{
    std::lock_guard<std::mutex> autoLock(commonEventStateLock_);
    commonEventWhitelist.erase(EventFwk::CommonEventSupport::COMMON_EVENT_USER_UNLOCKED);
    HILOGI("rm USER_UNLOCKED,size=%{public}zu", commonEventWhitelist.size());
}

void CommonEventCollect::CleanFfrt()
{
    if (workHandler_ != nullptr) {
        workHandler_->CleanFfrt();
    }
    if (unsubHandler_ != nullptr) {
        unsubHandler_->CleanFfrt();
    }
}

void CommonEventCollect::SetFfrt()
{
    if (workHandler_ != nullptr) {
        workHandler_->SetFfrt();
    }
    if (unsubHandler_ != nullptr) {
        unsubHandler_->SetFfrt();
    }
}

int32_t CommonEventCollect::OnStart()
{
    HILOGI("CommonEventCollect OnStart called");
    if (commonEventNames_.empty()) {
        HILOGW("CommonEventCollect commonEventNames_ is empty");
        return ERR_OK;
    }

    workHandler_ = std::make_shared<CommonHandler>(this);
    unsubHandler_ = std::make_shared<CommonHandler>(this);
    workHandler_->SendEvent(INIT_EVENT);
    StartMonitorThread();
    return ERR_OK;
}

int32_t CommonEventCollect::OnStop()
{
    if (workHandler_ != nullptr) {
        workHandler_ = nullptr;
    }
    if (unsubHandler_ != nullptr) {
        unsubHandler_ = nullptr;
    }
    StopMonitorThread();
    return ERR_OK;
}

void CommonEventCollect::InitCommonEventState(const OnDemandEvent& event)
{
    if (event.eventId == COMMON_EVENT) {
        std::lock_guard<std::mutex> autoLock(commomEventLock_);
        commonEventNames_.insert(event.name);
    }
    for (auto& condition : event.conditions) {
        if (condition.eventId != COMMON_EVENT) {
            continue;
        }
        {
            std::lock_guard<std::mutex> autoLock(commomEventLock_);
            commonEventNames_.insert(condition.name);
        }
        if (condition.extraMessages.size() > 0) {
            std::lock_guard<std::mutex> autoLock(commonEventStateLock_);
            for (auto [key, value] : condition.extraMessages) {
                commonEventConditionExtraData_[condition.name][key] = "";
            }
        }
    }
}

void CommonEventCollect::Init(const std::list<SaProfile>& onDemandSaProfiles)
{
    {
        std::lock_guard<std::mutex> autoLock(commonEventStateLock_);
        commonEventWhitelist.insert(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON);
        commonEventWhitelist.insert(EventFwk::CommonEventSupport::COMMON_EVENT_DISCHARGING);
        commonEventWhitelist.insert(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED);
    }
    {
        std::lock_guard<std::mutex> autoLock(commomEventLock_);
        commonEventNames_.insert(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON);
        commonEventNames_.insert(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
        commonEventNames_.insert(EventFwk::CommonEventSupport::COMMON_EVENT_CHARGING);
        commonEventNames_.insert(EventFwk::CommonEventSupport::COMMON_EVENT_DISCHARGING);
        commonEventNames_.insert(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED);
        commonEventNames_.insert(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED);
        commonEventNames_.insert(EventFwk::CommonEventSupport::COMMON_EVENT_USER_UNLOCKED);
        commonEventNames_.insert(COMMON_RECENT_EVENT);
    }

    for (auto& profile : onDemandSaProfiles) {
        for (auto iterStart = profile.startOnDemand.onDemandEvents.begin();
            iterStart != profile.startOnDemand.onDemandEvents.end(); iterStart++) {
            InitCommonEventState(*iterStart);
        }
        for (auto iterStop = profile.stopOnDemand.onDemandEvents.begin();
            iterStop != profile.stopOnDemand.onDemandEvents.end(); iterStop++) {
            InitCommonEventState(*iterStop);
        }
    }
}

void CommonEventCollect::AddSkillsEvent(EventFwk::MatchingSkills& skill)
{
    std::lock_guard<std::mutex> autoLock(commomEventLock_);
    for (auto& commonEventName : commonEventNames_) {
        HILOGD("CommonEventCollect add event: %{public}s", commonEventName.c_str());
        skill.AddEvent(commonEventName);
    }
}

void CommonEventCollect::CleanFailedEventLocked(const std::vector<std::string>& eventNames)
{
    if (commonEventSubscriber_ == nullptr) {
        HILOGE("commonEventSubscriber_ is nullptr!");
        return;
    }
    EventFwk::MatchingSkills skill = commonEventSubscriber_->GetSubscribeInfo().GetMatchingSkills();
    for (auto& eventName : eventNames) {
        skill.RemoveEvent(eventName);
        std::lock_guard<std::mutex> autoLock(commomEventLock_);
        commonEventNames_.erase(eventName);
    }
}

bool CommonEventCollect::CreateCommonEventSubscriber()
{
    std::lock_guard<std::mutex> autoLock(commonEventSubscriberLock_);
    return CreateCommonEventSubscriberLocked();
}

bool CommonEventCollect::CreateCommonEventSubscriberLocked()
{
    int64_t begin = GetTickCount();
    EventFwk::MatchingSkills skill = EventFwk::MatchingSkills();
    AddSkillsEvent(skill);
    EventFwk::CommonEventSubscribeInfo info(skill);
    std::shared_ptr<EventFwk::CommonEventSubscriber> comEvtScrb = commonEventSubscriber_;
    commonEventSubscriber_ = std::make_shared<CommonEventSubscriber>(info, this);
    bool ret = EventFwk::CommonEventManager::SubscribeCommonEvent(commonEventSubscriber_);
    HILOGI("SubsComEvt %{public}" PRId64 "ms %{public}s", (GetTickCount() - begin), ret ? "suc" : "fail");
    if (comEvtScrb != nullptr) {
        auto unsubTask = [comEvtScrb]() {
            HILOGI("UnSubsComEvt start");
            {
                SamgrXCollie samgrXCollie("samgr--UnSubscribeCommonEvent");
                bool isUnsubscribe = EventFwk::CommonEventManager::UnSubscribeCommonEvent(comEvtScrb);
                if (!isUnsubscribe) {
                    HILOGE("CreateCommonEventSubscriberLocked isUnsubscribe failed!");
                }
            }
        };
        if (unsubHandler_ != nullptr) {
            unsubHandler_->PostTask(unsubTask, UNSUB_DELAY_TIME);
        } else {
            HILOGE("CreateCommonEventSubscriberLocked unsubHandler is null!");
        }
    }
    return ret;
}

bool CommonEventCollect::SendEvent(uint32_t eventId)
{
    if (workHandler_ == nullptr) {
        HILOGI("CommonEventCollect workHandler is nullptr");
        return false;
    }
    return workHandler_->SendEvent(eventId);
}

CommonEventListener::CommonEventListener(const sptr<CommonEventCollect>& commonEventCollect)
    : commonEventCollect_(commonEventCollect) {}

void CommonEventListener::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    if (commonEventCollect_ == nullptr) {
        HILOGE("commonEventCollect_ is nullptr!");
        return;
    }
    if (systemAbilityId == COMMON_EVENT_SERVICE_ID) {
        HILOGI("CommonEventCollect ces is ready");
        commonEventCollect_->SendEvent(SUB_COMMON_EVENT);
    }
}

void CommonEventListener::OnRemoveSystemAbility(int32_t systemAblityId, const std::string& deviceId)
{
    HILOGI("CommonEventListener OnRemoveSystemAblity systemAblityId:%{public}d", systemAblityId);
}

void CommonEventCollect::SaveAction(const std::string& action)
{
    std::lock_guard<std::mutex> autoLock(commonEventStateLock_);
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON) {
        commonEventWhitelist.insert(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON);
        commonEventWhitelist.erase(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF) {
        commonEventWhitelist.insert(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
        commonEventWhitelist.erase(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON);
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_CHARGING) {
        commonEventWhitelist.insert(EventFwk::CommonEventSupport::COMMON_EVENT_CHARGING);
        commonEventWhitelist.erase(EventFwk::CommonEventSupport::COMMON_EVENT_DISCHARGING);
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_DISCHARGING) {
        commonEventWhitelist.insert(EventFwk::CommonEventSupport::COMMON_EVENT_DISCHARGING);
        commonEventWhitelist.erase(EventFwk::CommonEventSupport::COMMON_EVENT_CHARGING);
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED) {
        commonEventWhitelist.insert(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED);
        commonEventWhitelist.erase(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED);
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED) {
        commonEventWhitelist.insert(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED);
        commonEventWhitelist.erase(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED);
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USER_UNLOCKED) {
        commonEventWhitelist.insert(EventFwk::CommonEventSupport::COMMON_EVENT_USER_UNLOCKED);
    }
}

bool CommonEventCollect::CheckCondition(const OnDemandCondition& condition)
{
    std::lock_guard<std::mutex> autoLock(commonEventStateLock_);
    std::map<std::string, std::string> stateMap = commonEventConditionExtraData_[condition.name];
    for (auto [key, profileValue] : condition.extraMessages) {
        if (!ParseUtil::CheckLogicRelationship(stateMap[key], profileValue)) {
            return false;
        }
    }
    if (commonEventConditionValue_[condition.name] != condition.value && condition.value != "") {
        return false;
    }
    if (condition.name == EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON ||
        condition.name == EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF ||
        condition.name == EventFwk::CommonEventSupport::COMMON_EVENT_CHARGING ||
        condition.name == EventFwk::CommonEventSupport::COMMON_EVENT_DISCHARGING ||
        condition.name == EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED ||
        condition.name == EventFwk::CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED ||
        condition.name == EventFwk::CommonEventSupport::COMMON_EVENT_USER_UNLOCKED) {
        return commonEventWhitelist.count(condition.name) > 0;
    }
    return true;
}

bool CommonEventCollect::CheckExtraMessage(int64_t extraDataId, const OnDemandEvent& profileEvent)
{
    OnDemandReasonExtraData extraData;
    if (!GetOnDemandReasonExtraData(extraDataId, extraData)) {
        return false;
    }
    std::map<std::string, std::string> eventExtraMessages = extraData.GetWant();
    for (auto [key, profileValue] : profileEvent.extraMessages) {
        if (!ParseUtil::CheckLogicRelationship(eventExtraMessages[key], profileValue)) {
            return false;
        }
    }
    return true;
}

int64_t CommonEventCollect::GenerateExtraDataIdLocked()
{
    extraDataId_++;
    if (extraDataId_ > MAX_EXTRA_DATA_ID) {
        extraDataId_ = 1;
    }
    return extraDataId_;
}

std::string CommonEventCollect::GetParamFromWant(const std::string& key, const AAFwk::Want& want)
{
    std::string valueString;
    int32_t valueInt = want.GetIntParam(key, -1);
    if (valueInt == -1) {
        valueString = want.GetStringParam(key);
    } else {
        valueString = std::to_string(valueInt);
    }
    if (want.GetBoolParam(key, false)) {
        valueString = "true";
    } else if (!want.GetBoolParam(key, true)) {
        valueString = "false";
    }
    HILOGD("key:%{public}s || value:%{public}s", key.c_str(), valueString.c_str());
    return valueString;
}

int64_t CommonEventCollect::SaveOnDemandReasonExtraData(const EventFwk::CommonEventData& data)
{
    HILOGD("CommonEventCollect extraData code: %{public}d, data: %{public}s", data.GetCode(),
        data.GetData().c_str());
    AAFwk::Want want = data.GetWant();
    auto keySet = want.GetParams().KeySet();
    std::map<std::string, std::string> wantMap;
    for (const auto& key : keySet) {
        wantMap[key] = GetParamFromWant(key, want);
        HILOGD("CommonEventCollect want key:%{public}s, val:%{public}s", key.c_str(), wantMap[key].c_str());
    }
    int32_t uid = want.GetIntParam(UID, -1);
    int32_t netType = want.GetIntParam(NET_TYPE, -1);
    wantMap[UID] = std::to_string(uid);
    wantMap[NET_TYPE] = std::to_string(netType);
    wantMap[BUNDLE_NAME] = want.GetBundle();
    int64_t extraDataId = 0;
    {
        std::lock_guard<std::mutex> autoLock(extraDataLock_);
        wantMap[COMMON_EVENT_ACTION_NAME] = want.GetAction();
        OnDemandReasonExtraData extraData(data.GetCode(), data.GetData(), wantMap);

        extraDataId = GenerateExtraDataIdLocked();
        extraDatas_[extraDataId] = extraData;
        HILOGD("CommonEventCollect save extraData %{public}d,n:%{public}zu",
            static_cast<int32_t>(extraDataId), extraDatas_.size());
    }
    if (workHandler_ == nullptr) {
        HILOGI("CommonEventCollect workHandler is nullptr");
        return -1;
    }
    workHandler_->SendEvent(REMOVE_EXTRA_DATA_EVENT, extraDataId, REMOVE_EXTRA_DATA_DELAY_TIME);
    return extraDataId;
}

void CommonEventCollect::SaveOnDemandConditionExtraData(const EventFwk::CommonEventData& data)
{
    std::lock_guard<std::mutex> autoLock(commonEventStateLock_);
    AAFwk::Want want = data.GetWant();
    commonEventConditionValue_[want.GetAction()] = std::to_string(data.GetCode());
    for (auto& [key, value] : commonEventConditionExtraData_[want.GetAction()]) {
        value = GetParamFromWant(key, want);
    }
}

void CommonEventCollect::RemoveOnDemandReasonExtraData(int64_t extraDataId)
{
    {
        std::lock_guard<std::mutex> autoLock(extraDataLock_);
        extraDatas_.erase(extraDataId);
    }
    HILOGD("CommonEventCollect remove extraData %{public}d", static_cast<int32_t>(extraDataId));
    RemoveSaExtraDataId(extraDataId);
}

bool CommonEventCollect::GetOnDemandReasonExtraData(int64_t extraDataId, OnDemandReasonExtraData& extraData)
{
    std::lock_guard<std::mutex> autoLock(extraDataLock_);
    HILOGD("CommonEventCollect get extraData %{public}d", static_cast<int32_t>(extraDataId));
    if (extraDatas_.count(extraDataId) == 0) {
        return false;
    }
    extraData = extraDatas_[extraDataId];
    return true;
}

void CommonEventCollect::SaveCacheCommonEventSaExtraId(const OnDemandEvent& event,
    const std::list<SaControlInfo>& saControlList)
{
    std::list<int32_t> saList = SamgrUtil::GetCacheCommonEventSa(event, saControlList);
    if (saList.empty()) {
        return;
    }
    for (auto& item : saList) {
        SaveSaExtraDataId(item, event.extraDataId);
    }
}

void CommonEventCollect::SaveSaExtraDataId(int32_t saId, int64_t extraDataId)
{
    std::lock_guard<std::mutex> autoLock(saExtraDataIdLock_);
    auto& extraIdList = saExtraDataIdMap_[saId];
    extraIdList.emplace_back(extraDataId);
    HILOGI("save SA:%{public}d,exId:%{public}d,n:%{public}zu", saId, static_cast<int32_t>(extraDataId),
        extraIdList.size());
}

void CommonEventCollect::RemoveSaExtraDataId(int64_t extraDataId)
{
    std::lock_guard<std::mutex> autoLock(saExtraDataIdLock_);
    HILOGD("rm saExtraId:%{public}d", static_cast<int32_t>(extraDataId));
    auto iter = saExtraDataIdMap_.begin();
    while (iter != saExtraDataIdMap_.end()) {
        auto& tmpList = iter->second;
        auto listIter = std::find(tmpList.begin(), tmpList.end(), extraDataId);
        if (listIter != tmpList.end()) {
            HILOGI("rm SA:%{public}d,exId:%{public}d,n:%{public}zu", iter->first,
                static_cast<int32_t>(extraDataId), tmpList.size());
            tmpList.erase(listIter);
        }
        if (tmpList.size() == 0) {
            HILOGI("rm exId SA:%{public}d", iter->first);
            iter = saExtraDataIdMap_.erase(iter);
        } else {
            ++iter;
        }
    }
}

void CommonEventCollect::ClearSaExtraDataId(int32_t saId)
{
    std::lock_guard<std::mutex> autoLock(saExtraDataIdLock_);
    if (saExtraDataIdMap_.count(saId) == 0) {
        return;
    }
    HILOGI("clear SA:%{public}d,map n:%{public}zu", saId, saExtraDataIdMap_.size());
    saExtraDataIdMap_[saId].clear();
    saExtraDataIdMap_.erase(saId);
}

int32_t CommonEventCollect::GetSaExtraDataIdList(int32_t saId, std::vector<int64_t>& extraDataidList,
    const std::string& eventName)
{
    std::list<int64_t> temp;
    {
        std::lock_guard<std::mutex> autoLock(saExtraDataIdLock_);
        if (saExtraDataIdMap_.count(saId) == 0) {
            HILOGD("NF exId SA:%{public}d", saId);
            return ERR_OK;
        }
        HILOGD("get exId SA:%{public}d event:%{public}s", saId, eventName.c_str());
        temp = saExtraDataIdMap_[saId];
    }
    if (eventName == "") {
        std::copy(temp.begin(), temp.end(), std::back_inserter(extraDataidList));
        return ERR_OK;
    }
    for (auto& item : temp) {
        OnDemandReasonExtraData extraData;
        if (!GetOnDemandReasonExtraData(item, extraData)) {
            HILOGD("NF exId:%{public}d", static_cast<int32_t>(item));
            continue;
        }
        std::map<std::string, std::string> want = extraData.GetWant();
        std::string extraEventName = want[COMMON_EVENT_ACTION_NAME];
        if (extraEventName != eventName) {
            HILOGD("exId:%{public}d event:%{public}s not match extra:%{public}s", static_cast<int32_t>(item),
                eventName.c_str(), extraEventName.c_str());
            continue;
        }
        HILOGD("get exId:%{public}d", static_cast<int32_t>(item));
        extraDataidList.push_back(item);
    }
    return ERR_OK;
}
std::vector<std::string> CommonEventCollect::AddCommonEventName(const std::vector<OnDemandEvent>& events)
{
    std::lock_guard<std::mutex> autoLock(commomEventLock_);
    std::vector<std::string> insertNames;
    for (auto& event : events) {
        auto iter = commonEventNames_.find(event.name);
        if (iter != commonEventNames_.end()) {
            continue;
        }
        HILOGI("CommonEventCollect add collect events: %{public}s", event.name.c_str());
        commonEventNames_.insert(event.name);
        insertNames.emplace_back(event.name);
    }
    return insertNames;
}

int32_t CommonEventCollect::AddCollectEvent(const std::vector<OnDemandEvent>& events)
{
    std::lock_guard<std::mutex> autoLock(commonEventSubscriberLock_);
    auto insertNames = AddCommonEventName(events);
    if (insertNames.empty()) {
        return ERR_OK;
    }
    if (!CreateCommonEventSubscriberLocked()) {
        HILOGE("AddCollectEvent CreateCommonEventSubscriber failed");
        CleanFailedEventLocked(insertNames);
        CreateCommonEventSubscriberLocked();
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

int32_t CommonEventCollect::RemoveUnusedEvent(const OnDemandEvent& event)
{
    std::lock_guard<std::mutex> autoLock(commomEventLock_);
    auto iter = commonEventNames_.find(event.name);
    if (iter != commonEventNames_.end()) {
        HILOGI("CommonEventCollect remove event name: %{public}s", event.name.c_str());
        commonEventNames_.erase(iter);
    }
    return ERR_OK;
}

void CommonEventCollect::StartReclaimIpcThreadWork(const EventFwk::CommonEventData& data)
{
    bool isTrigger = false;
    std::string eventName = data.GetWant().GetAction();
    std::string eventType = data.GetData();

    if (eventName == EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF) {
        isTrigger = true;
        isCancel_ = false;
    } else if (eventName == EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON) {
        isCancel_ = true;
    } else if (eventName == COMMON_RECENT_EVENT && eventType == COMMON_RECENT_CLEAR_ALL) {
        isTrigger = true;
        isCancel_ = false;
        HILOGI("TriggerSystemIPCThreadReclaim");
        IPCSkeleton::TriggerSystemIPCThreadReclaim();
    }

    if (isTrigger && !isTriggerTaskStart_.exchange(true)) {
        SendKernalReclaimIpcThread();
    }
}

void CommonEventCollect::SendKernalReclaimIpcThread()
{
    auto task = [this]() {
        for (int i = 0; i < TRIGGER_THREAD_RECLAIM_DELAY_TIME; i+= TRIGGER_THREAD_RECLAIM_DURATION_TIME) {
            if (isCancel_) {
                isTriggerTaskStart_ = false;
                return;
            }
            std::this_thread::sleep_for(std::chrono::seconds(TRIGGER_THREAD_RECLAIM_DURATION_TIME));
        }
        HILOGI("TriggerSystemIPCThreadReclaim");
        IPCSkeleton::TriggerSystemIPCThreadReclaim();
        isTriggerTaskStart_ = false;
    };
    std::thread reclaimThread(task);
    reclaimThread.detach();
}

bool CommonHandler::PostTask(std::function<void()> func, uint64_t delayTime)
{
    if (handler_ == nullptr) {
        HILOGE("CommonEventCollect PostTask handler is null!");
        return false;
    }
    return handler_->PostTask(func, delayTime);
}

bool CommonHandler::PostTask(std::function<void()> func, const std::string& name, uint64_t delayTime)
{
    if (handler_ == nullptr) {
        HILOGE("CommonEventCollect PostTask handler is null!");
        return false;
    }
    return handler_->PostTask(func, name, delayTime);
}

void CommonHandler::RemoveTask(const std::string& name)
{
    if (handler_ == nullptr) {
        HILOGE("CommonEventCollect RemoveTask handler is null!");
        return;
    }
    handler_->RemoveTask(name);
}

void CommonHandler::DelTask(const std::string& name)
{
    if (handler_ == nullptr) {
        HILOGE("CommonEventCollect DelTask handler is null!");
        return;
    }
    handler_->DelTask(name);
}

void CommonHandler::CleanFfrt()
{
    if (handler_ != nullptr) {
        handler_->CleanFfrt();
    }
}

void CommonHandler::SetFfrt()
{
    if (handler_ != nullptr) {
        handler_->SetFfrt("CommonHandler");
    }
}

void CommonHandler::ProcessEvent(uint32_t eventId, int64_t extraDataId)
{
    if (commonCollect_ == nullptr) {
        HILOGE("CommonEventCollect ProcessEvent collect or event is null!");
        return;
    }
    if (eventId != INIT_EVENT && eventId != REMOVE_EXTRA_DATA_EVENT && eventId != SUB_COMMON_EVENT) {
        HILOGE("CommonEventCollect ProcessEvent error event code!");
        return;
    }
    auto commonCollect = commonCollect_.promote();
    if (commonCollect == nullptr) {
        HILOGE("CommonEventCollect collect is nullptr");
        return;
    }
    if (eventId == REMOVE_EXTRA_DATA_EVENT) {
        commonCollect->RemoveOnDemandReasonExtraData(extraDataId);
        return;
    }
    if (eventId == SUB_COMMON_EVENT) {
        if (!commonCollect->CreateCommonEventSubscriber()) {
            HILOGE("OnAddSystemAbility CreateCommonEventSubscriber failed!");
        }
        return;
    }
    sptr<CommonEventListener> listener = new CommonEventListener(commonCollect);
    SystemAbilityManager::GetInstance()->SubscribeSystemAbility(COMMON_EVENT_SERVICE_ID, listener);
}

bool CommonHandler::SendEvent(uint32_t eventId)
{
    if (handler_ == nullptr) {
        HILOGE("CommonEventCollect SendEvent handler is null!");
        return false;
    }
    auto task = [this, eventId] {this->ProcessEvent(eventId, 0);};
    return handler_->PostTask(task);
}

bool CommonHandler::SendEvent(uint32_t eventId, int64_t extraDataId, uint64_t delayTime)
{
    if (handler_ == nullptr) {
        HILOGE("CommonEventCollect SendEvent handler is null!");
        return false;
    }
    auto task = [this, eventId, extraDataId] {this->ProcessEvent(eventId, extraDataId);};
    return handler_->PostTask(task, delayTime);
}

CommonEventSubscriber::CommonEventSubscriber(const EventFwk::CommonEventSubscribeInfo& subscribeInfo,
    const sptr<CommonEventCollect>& collect)
    :EventFwk::CommonEventSubscriber(subscribeInfo), collect_(collect) {}

void CommonEventSubscriber::OnReceiveEvent(const EventFwk::CommonEventData& data)
{
    SamgrXCollie samgrXCollie("samgr--OnReceiveEvent");
    std::string action = data.GetWant().GetAction();
    int32_t code = data.GetCode();
    auto collect = collect_.promote();
    if (collect == nullptr) {
        HILOGE("CommonEventCollect collect is nullptr");
        return;
    }
    collect->SaveAction(action);
    int64_t extraDataId = collect->SaveOnDemandReasonExtraData(data);
    HILOGI("RecvEvent:%{public}s,%{public}d_%{public}d", action.c_str(), code, static_cast<int32_t>(extraDataId));
    collect->SaveOnDemandConditionExtraData(data);
    OnDemandEvent event = {COMMON_EVENT, action, std::to_string(code), extraDataId};
    collect->ReportEvent(event);
    collect->StartReclaimIpcThreadWork(data);
}

bool CommonEventCollect::GetCpuTimes(const char* file, uint64_t& total, uint64_t& idle)
{
    if (file == nullptr) {
        HILOGE("Invalid file name");
        return false;
    }

    std::ifstream cpuStatFile(file);
    if (!cpuStatFile.is_open()) {
        HILOGE("Failed to open %{public}s", file);
        return false;
    }

    std::string line;
    if (!std::getline(cpuStatFile, line)) {
        HILOGE("Failed to read %{public}s", file);
        return false;
    }

    uint64_t user;
    uint64_t nice;
    uint64_t system;
    uint64_t iowait;
    uint64_t irq;
    uint64_t softirq;
    uint64_t steal;
    uint64_t guest = 0;
    uint64_t guestNice = 0;

    int num = sscanf_s(line.c_str(), "cpu %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64
        " %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64,
        &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &guestNice);

    if (num < CPU_STAT_MIN_FIELDS) {
        HILOGE("Failed to parse %{public}s (got %{public}d fields)", file, num);
        return false;
    }

    total = user + nice + system + idle + iowait + irq + softirq + steal;
    if (num > CPU_STAT_MIN_FIELDS) {
        total += guest + guestNice;
    }

    return true;
}

float CommonEventCollect::GetCpuUsage(const char* file, uint32_t interval)
{
    if (file == nullptr) {
        HILOGE("Invalid file name");
        return CPU_LOAD_INVALID;
    }

    if (interval <= 0) {
        HILOGE("Invalid interval");
        return CPU_LOAD_INVALID;
    }

    uint64_t totalPre = 0;
    uint64_t idlePre = 0;
    uint64_t total = 0;
    uint64_t idle = 0;

    if (!GetCpuTimes(file, totalPre, idlePre)) {
        HILOGE("Failed to get pre CPU times");
        return CPU_LOAD_INVALID;
    }

    std::this_thread::sleep_for(std::chrono::seconds(interval));
    if (!GetCpuTimes(file, total, idle)) {
        HILOGE("Failed to get CPU times");
        return CPU_LOAD_INVALID;
    }

    constexpr uint64_t MAX = std::numeric_limits<uint64_t>::max();
    uint64_t totalDelta = (total >= totalPre) ? (total - totalPre) : (MAX - totalPre + total);
    uint64_t idleDelta = (idle >= idlePre) ? (idle - idlePre) : (MAX - idlePre + idle);
    if (totalDelta == 0 || totalDelta < idleDelta) {
        return CPU_LOAD_INVALID;
    }

    float usage = static_cast<float>(totalDelta - idleDelta) / totalDelta;
    return usage * CPU_LOAD_PERCENT;
}

void CommonEventCollect::MonitorCpuUsageThread()
{
    struct sysinfo info;
    uint64_t coreNum = static_cast<uint64_t>(sysconf(_SC_NPROCESSORS_ONLN));
    uint64_t baseLoad = coreNum << CPU_LOAD_SHIFT;
    pthread_setname_np(pthread_self(), "OS_CPU_MONITOR");

    while (keepRunning_) {
        std::this_thread::sleep_for(std::chrono::seconds(CPU_LOAD_CHECK_INTERVAL - CPU_STAT_CHECK_INTERVAL));
        float usage = GetCpuUsage(CPU_STAT_INFO, CPU_STAT_CHECK_INTERVAL);
        if (sysinfo(&info) == 0) {
            HILOGI("cpu usage: %{public}f 1min %{public}lu 5min %{public}lu", usage, info.loads[0], info.loads[1]);
            if (info.loads[0] - baseLoad < (1 << CPU_LOAD_SHIFT) && // 1min avg load <= (logic core num + 1)
                info.loads[0] < info.loads[1] && // 1min avg load less than 5min avg load
                usage > CPU_LOAD_INVALID && // cpu usage > 0% and <= 10%
                usage <= CPU_LOAD_IDLE_THRESHOLD) {
                HILOGI("cpu idle TriggerSystemIPCThreadReclaim");
                IPCSkeleton::TriggerSystemIPCThreadReclaim();
            }
        }
    }
}

void CommonEventCollect::StartMonitorThread()
{
    keepRunning_ = true;
    monitorThread_ = std::thread(&CommonEventCollect::MonitorCpuUsageThread, this);
    monitorThread_.detach();
}

void CommonEventCollect::StopMonitorThread()
{
    if (keepRunning_) {
        keepRunning_ = false;
        if (monitorThread_.joinable()) {
            monitorThread_.join();
        }
    }
}
} // namespace OHOS