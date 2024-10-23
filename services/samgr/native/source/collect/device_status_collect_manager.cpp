/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "device_status_collect_manager.h"

#include "datetime_ex.h"
#include "device_timed_collect.h"
#ifdef SUPPORT_DEVICE_MANAGER
#include "device_networking_collect.h"
#endif
#ifdef SUPPORT_COMMON_EVENT
#include "common_event_collect.h"
#endif
#ifdef SUPPORT_SWITCH_COLLECT
#include "device_switch_collect.h"
#endif
#include "device_param_collect.h"
#include "memory_guard.h"
#include "sam_log.h"
#include "system_ability_manager.h"

namespace OHOS {
namespace {
constexpr int32_t TO_MILLISECOND = 1000;
}
void DeviceStatusCollectManager::Init(const std::list<SaProfile>& saProfiles)
{
    HILOGI("DeviceStaMgr Init begin");
    FilterOnDemandSaProfiles(saProfiles);
    collectHandler_ = std::make_shared<FFRTHandler>("collect");
    sptr<ICollectPlugin> deviceParamCollect = new DeviceParamCollect(this);
    deviceParamCollect->Init(saProfiles);
    collectPluginMap_[PARAM] = deviceParamCollect;
#ifdef SUPPORT_DEVICE_MANAGER
    sptr<ICollectPlugin> networkingCollect = new DeviceNetworkingCollect(this);
    collectPluginMap_[DEVICE_ONLINE] = networkingCollect;
#endif
#ifdef SUPPORT_COMMON_EVENT
    sptr<ICollectPlugin> eventStatuscollect = new CommonEventCollect(this);
    eventStatuscollect->Init(onDemandSaProfiles_);
    collectPluginMap_[COMMON_EVENT] = eventStatuscollect;
#endif
#ifdef SUPPORT_SWITCH_COLLECT
    sptr<ICollectPlugin> deviceSwitchCollect = new DeviceSwitchCollect(this);
    deviceSwitchCollect->Init(saProfiles);
    collectPluginMap_[SETTING_SWITCH] = deviceSwitchCollect;
#endif
    sptr<ICollectPlugin> timedCollect = new DeviceTimedCollect(this);
    timedCollect->Init(onDemandSaProfiles_);
    collectPluginMap_[TIMED_EVENT] = timedCollect;
    StartCollect();
    HILOGI("DeviceStaMgr Init end");
}

void DeviceStatusCollectManager::RemoveWhiteCommonEvent()
{
    if (IsExistInPluginMap(COMMON_EVENT) == ERR_OK) {
        collectPluginMap_[COMMON_EVENT]->RemoveWhiteCommonEvent();
    }
}

void DeviceStatusCollectManager::FilterOnDemandSaProfiles(const std::list<SaProfile>& saProfiles)
{
    std::unique_lock<std::shared_mutex> writeLock(saProfilesLock_);
    for (auto& saProfile : saProfiles) {
        if (saProfile.startOnDemand.onDemandEvents.empty() && saProfile.stopOnDemand.onDemandEvents.empty()) {
            continue;
        }
        onDemandSaProfiles_.emplace_back(saProfile);
    }
}

void DeviceStatusCollectManager::GetSaControlListByPersistEvent(const OnDemandEvent& event,
    std::list<SaControlInfo>& saControlList)
{
#ifdef PREFERENCES_ENABLE
    std::shared_ptr<PreferencesUtil> preferencesUtil = PreferencesUtil::GetInstance();
    if (preferencesUtil == nullptr) {
        HILOGW("GetSaControlListByPersistEvent preferencesUtil is nullptr");
        return;
    }
    std::string value = preferencesUtil->ObtainString(event.ToString(), std::string());
    if (value == std::string()) {
        return;
    }
    std::vector<std::string> strVector;
    SplitStr(value, "+", strVector);
    size_t vectorSize = strVector.size();
    for (size_t i = 0; i < vectorSize; i++) {
        OnDemandPolicyType type = OnDemandPolicyType::START_POLICY;
        int32_t systemAbilityId = -1;
        HILOGD("vector is : %{public}s", strVector[i].c_str());
        StringToTypeAndSaid(strVector[i], type, systemAbilityId);
        SaControlInfo control;
        if (type == OnDemandPolicyType::START_POLICY) {
            control = {START_ON_DEMAND, systemAbilityId };
        } else {
            control = {STOP_ON_DEMAND, systemAbilityId };
        }
        saControlList.emplace_back(control);
    }
#endif
}

void DeviceStatusCollectManager::GetSaControlListByEvent(const OnDemandEvent& event,
    std::list<SaControlInfo>& saControlList)
{
    std::shared_lock<std::shared_mutex> readLock(saProfilesLock_);
    for (auto& profile : onDemandSaProfiles_) {
        // start on demand
        for (auto iterStart = profile.startOnDemand.onDemandEvents.begin();
            iterStart != profile.startOnDemand.onDemandEvents.end(); iterStart++) {
            if (IsSameEvent(event, *iterStart) && CheckConditions(*iterStart) &&
                CheckExtraMessages(event, *iterStart)) {
                // maybe the process is being killed, let samgr make decisions.
                SaControlInfo control = { START_ON_DEMAND, profile.saId, iterStart->enableOnce,
                    iterStart->loadPriority, profile.cacheCommonEvent };
                saControlList.emplace_back(control);
                break;
            }
        }
        // stop on demand
        for (auto iterStop = profile.stopOnDemand.onDemandEvents.begin();
            iterStop != profile.stopOnDemand.onDemandEvents.end(); iterStop++) {
            if (IsSameEvent(event, *iterStop) && CheckConditions(*iterStop) &&
                CheckExtraMessages(event, *iterStop)) {
                // maybe the process is starting, let samgr make decisions.
                SaControlInfo control = { STOP_ON_DEMAND, profile.saId, iterStop->enableOnce,
                    iterStop->loadPriority, profile.cacheCommonEvent };
                saControlList.emplace_back(control);
                break;
            }
        }
    }
    HILOGD("DeviceStaMgr saControlList size %{public}zu", saControlList.size());
}

void DeviceStatusCollectManager::SortSaControlListByLoadPriority(std::list<SaControlInfo>& saControlList)
{
    saControlList.sort([](const SaControlInfo& control1, const SaControlInfo& control2) {
        return control1.loadPriority < control2.loadPriority;
    });
}

bool DeviceStatusCollectManager::IsSameEvent(const OnDemandEvent& ev1, const OnDemandEvent& ev2)
{
    return (ev1.eventId == ev2.eventId && ev1.name == ev2.name &&
        ev1.persistence == ev2.persistence && (ev1.value == ev2.value || "" == ev2.value));
}

bool DeviceStatusCollectManager::IsSameEventName(const OnDemandEvent& ev1, const OnDemandEvent& ev2)
{
    if (ev1.eventId != TIMED_EVENT) {
        if (ev1.eventId == ev2.eventId && ev1.name == ev2.name) {
            return true;
        }
    } else {
        if (ev1.eventId == ev2.eventId && ev1.name == ev2.name && ev1.value == ev2.value &&
            ev1.persistence == ev2.persistence) {
            return true;
        }
    }
    return false;
}

bool DeviceStatusCollectManager::CheckConditions(const OnDemandEvent& onDemandEvent)
{
    if (onDemandEvent.conditions.empty()) {
        return true;
    }
    for (auto& condition : onDemandEvent.conditions) {
        if (collectPluginMap_.count(condition.eventId) == 0) {
            HILOGE("not support condition: %{public}d", condition.eventId);
            return false;
        }
        if (collectPluginMap_[condition.eventId] == nullptr) {
            HILOGE("not support condition: %{public}d", condition.eventId);
            return false;
        }
        bool ret = collectPluginMap_[condition.eventId]->CheckCondition(condition);
        if (!ret) {
            HILOGW("CheckCondition:%{public}s, value:%{public}s not pass",
                condition.name.c_str(), condition.value.c_str());
            return false;
        }
    }
    return true;
}

bool DeviceStatusCollectManager::CheckExtraMessages(const OnDemandEvent& ev1, const OnDemandEvent& ev2)
{
    HILOGD("CheckExtraMessages begin evt1:%{public}d, evt2:%{public}d", ev1.eventId, ev2.eventId);
    if (collectPluginMap_.count(ev1.eventId) == 0) {
        HILOGE("not support CheckExtraMessages");
        return false;
    }
    if (collectPluginMap_[ev1.eventId] == nullptr) {
        HILOGE("CommonEventCollect is nullptr");
        return false;
    }
    if (collectPluginMap_[ev1.eventId]->CheckExtraMessage(ev1.extraDataId, ev2)) {
        return true;
    }
    return false;
}

void DeviceStatusCollectManager::UnInit()
{
    for (auto& iter : collectPluginMap_) {
        if (iter.second != nullptr) {
            iter.second->OnStop();
        }
    }
    collectPluginMap_.clear();

    if (collectHandler_ != nullptr) {
        collectHandler_ = nullptr;
    }
}

void DeviceStatusCollectManager::CleanFfrt()
{
    for (auto& iter : collectPluginMap_) {
        if ((iter.first == DEVICE_ONLINE || iter.first == COMMON_EVENT) && (iter.second != nullptr)) {
            iter.second->CleanFfrt();
        }
    }
    if (collectHandler_ != nullptr) {
        collectHandler_->CleanFfrt();
    }
}

void DeviceStatusCollectManager::SetFfrt()
{
    for (auto& iter : collectPluginMap_) {
        if ((iter.first == DEVICE_ONLINE || iter.first == COMMON_EVENT) && (iter.second != nullptr)) {
            iter.second->SetFfrt();
        }
    }
    if (collectHandler_ != nullptr) {
        collectHandler_->SetFfrt("collect");
    }
}

void DeviceStatusCollectManager::StartCollect()
{
    HILOGI("DeviceStaMgr OnStart begin");
    if (collectHandler_ == nullptr) {
        return;
    }
    auto callback = [this] () {
        for (auto& iter : collectPluginMap_) {
            iter.second->OnStart();
        }
    };
    collectHandler_->PostTask(callback);
}

void DeviceStatusCollectManager::ReportEvent(const OnDemandEvent& event)
{
    if (collectHandler_ == nullptr) {
        HILOGW("DeviceStaMgr collectHandler_ is nullptr");
        return;
    }
    auto callback = [event, this] () {
        std::list<SaControlInfo> saControlList;
        GetSaControlListByEvent(event, saControlList);
        GetSaControlListByPersistEvent(event, saControlList);
        SortSaControlListByLoadPriority(saControlList);
        if (saControlList.empty()) {
            HILOGD("DeviceStaMgr no matched event");
            if (event.eventId == DEVICE_ONLINE) {
                HILOGI("deviceOnline is empty");
            }
            return;
        }
        SystemAbilityManager::GetInstance()->ProcessOnDemandEvent(event, saControlList);
    };
    collectHandler_->PostTask(callback);
}

void DeviceStatusCollectManager::PostDelayTask(std::function<void()> callback, int32_t delayTime)
{
    HILOGI("DeviceStaMgr PostDelayTask begin");
    if (delayTime < 0 || delayTime > std::numeric_limits<int32_t>::max() / TO_MILLISECOND) {
        HILOGE("DeviceStaMgr PostDelayTask Failed : delayTime out of range %{public}d", delayTime);
        return;
    }
    collectHandler_->PostTask(callback, delayTime * TO_MILLISECOND);
}

int32_t DeviceStatusCollectManager::IsExistInPluginMap(int32_t eventId)
{
    if (collectPluginMap_.count(eventId) == 0) {
        HILOGE("eventid:%{public}d collect not exist", eventId);
        return ERR_INVALID_VALUE;
    }
    if (collectPluginMap_[eventId] == nullptr) {
        HILOGE("eventid:%{public}d collect is null", eventId);
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

int32_t DeviceStatusCollectManager::GetOnDemandReasonExtraData(int64_t extraDataId, OnDemandReasonExtraData& extraData)
{
    HILOGD("DeviceStaMgr GetOnDemandReasonExtraData begin, extraDataId:%{public}d",
        static_cast<int32_t>(extraDataId));
    if (IsExistInPluginMap(COMMON_EVENT) != ERR_OK) {
        HILOGE("not support get extra data");
        return ERR_INVALID_VALUE;
    }
    if (!collectPluginMap_[COMMON_EVENT]->GetOnDemandReasonExtraData(extraDataId, extraData)) {
        HILOGE("get extra data failed");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

void DeviceStatusCollectManager::SaveSaExtraDataId(int32_t saId, int64_t extraDataId)
{
    HILOGD("DeviceStaMgr SaveSaExtraDataId begin, SA:%{public}d, extraDataId:%{public}d",
        saId, static_cast<int32_t>(extraDataId));
    if (IsExistInPluginMap(COMMON_EVENT) != ERR_OK) {
        HILOGE("CommonEventCollect is nullptr");
        return;
    }
    collectPluginMap_[COMMON_EVENT]->SaveSaExtraDataId(saId, extraDataId);
    return;
}

void DeviceStatusCollectManager::ClearSaExtraDataId(int32_t saId)
{
    HILOGD("DeviceStaMgr ClearSaExtraDataId begin, SA:%{public}d", saId);
    if (IsExistInPluginMap(COMMON_EVENT) != ERR_OK) {
        HILOGE("CommonEventCollect is nullptr");
        return;
    }
    collectPluginMap_[COMMON_EVENT]->ClearSaExtraDataId(saId);
    return;
}

void DeviceStatusCollectManager::SaveCacheCommonEventSaExtraId(const OnDemandEvent& event,
    const std::list<SaControlInfo>& saControlList)
{
    HILOGD("DeviceStaMgr SaveCacheCommonEventSaExtraId begin");
    if (IsExistInPluginMap(COMMON_EVENT) != ERR_OK) {
        HILOGE("CommonEventCollect is nullptr");
        return;
    }
    collectPluginMap_[COMMON_EVENT]->SaveCacheCommonEventSaExtraId(event, saControlList);
    return;
}

int32_t DeviceStatusCollectManager::GetSaExtraDataIdList(int32_t saId, std::vector<int64_t>& extraDataIdList,
    const std::string& eventName)
{
    HILOGD("DeviceStaMgr GetSaExtraDataIdList begin, SA:%{public}d, event:%{public}s",
        saId, eventName.c_str());
    if (IsExistInPluginMap(COMMON_EVENT) != ERR_OK) {
        HILOGE("CommonEventCollect is nullptr");
        return ERR_INVALID_VALUE;
    }
    return collectPluginMap_[COMMON_EVENT]->GetSaExtraDataIdList(saId, extraDataIdList, eventName);
}

int32_t DeviceStatusCollectManager::AddCollectEvents(const std::vector<OnDemandEvent>& events)
{
    if (events.size() == 0) {
        return ERR_OK;
    }
    for (auto& event : events) {
        if (collectPluginMap_.count(event.eventId) == 0) {
            HILOGE("not support eventId: %{public}d", event.eventId);
            return ERR_INVALID_VALUE;
        }
        if (collectPluginMap_[event.eventId] == nullptr) {
            HILOGE("not support eventId: %{public}d", event.eventId);
            return ERR_INVALID_VALUE;
        }
        int32_t ret = collectPluginMap_[event.eventId]->AddCollectEvent(event);
        if (ret != ERR_OK) {
            HILOGE("add collect event failed, eventId: %{public}d", event.eventId);
            return ret;
        }
    }
    return ERR_OK;
}

int32_t DeviceStatusCollectManager::GetOnDemandEvents(int32_t systemAbilityId, OnDemandPolicyType type,
    std::vector<OnDemandEvent>& events)
{
    HILOGI("GetOnDemandEvents begin");
    std::shared_lock<std::shared_mutex> readLock(saProfilesLock_);
    auto iter = std::find_if(onDemandSaProfiles_.begin(), onDemandSaProfiles_.end(), [systemAbilityId](auto saProfile) {
        return saProfile.saId == systemAbilityId;
    });
    if (iter == onDemandSaProfiles_.end()) {
        HILOGI("GetOnDemandEvents invalid saId:%{public}d", systemAbilityId);
        return ERR_INVALID_VALUE;
    }
    if (type == OnDemandPolicyType::START_POLICY) {
        events = (*iter).startOnDemand.onDemandEvents;
    } else if (type == OnDemandPolicyType::STOP_POLICY) {
        events = (*iter).stopOnDemand.onDemandEvents;
    } else {
        HILOGE("GetOnDemandEvents invalid policy types");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

int32_t DeviceStatusCollectManager::RemoveUnusedEventsLocked(const std::vector<OnDemandEvent>& events)
{
    HILOGD("RemoveUnusedEventsLocked start");
    if (events.size() == 0) {
        return ERR_OK;
    }
    for (auto& event : events) {
        if (collectPluginMap_.count(event.eventId) == 0) {
            HILOGE("not support eventId: %{public}d", event.eventId);
            continue;
        }
        if (collectPluginMap_[event.eventId] == nullptr) {
            HILOGE("not support eventId: %{public}d", event.eventId);
            continue;
        }
        bool eventUsed = CheckEventUsedLocked(event);
        if (!eventUsed) {
            HILOGI("CheckEventUsedLocked name:%{public}s,value:%{public}s", event.name.c_str(), event.value.c_str());
            int32_t ret = collectPluginMap_[event.eventId]->RemoveUnusedEvent(event);
            if (ret != ERR_OK) {
                HILOGE("Remove event failed, eventId: %{public}d", event.eventId);
            }
        }
    }
    return ERR_OK;
}

bool DeviceStatusCollectManager::CheckEventUsedLocked(const OnDemandEvent& event)
{
    for (auto& profile : onDemandSaProfiles_) {
        // start on demand
        for (auto iterStart = profile.startOnDemand.onDemandEvents.begin();
            iterStart != profile.startOnDemand.onDemandEvents.end(); iterStart++) {
            if (IsSameEventName(event, *iterStart)) {
                return true;
            }
        }
        // stop on demand
        for (auto iterStop = profile.stopOnDemand.onDemandEvents.begin();
            iterStop != profile.stopOnDemand.onDemandEvents.end(); iterStop++) {
            if (IsSameEventName(event, *iterStop)) {
                return true;
            }
        }
    }
    return false;
}

bool DeviceStatusCollectManager::NeedPersistOnDemandEvent(const OnDemandEvent& event)
{
    if (event.eventId == TIMED_EVENT && event.name == "timedevent" && event.persistence) {
        return true;
    }
    return false;
}

void DeviceStatusCollectManager::PersistOnDemandEvent(int32_t systemAbilityId, OnDemandPolicyType type,
    const std::vector<OnDemandEvent>& events)
{
#ifdef PREFERENCES_ENABLE
    std::shared_ptr<PreferencesUtil> preferencesUtil = PreferencesUtil::GetInstance();
    if (preferencesUtil == nullptr) {
        return;
    }
    for (OnDemandEvent event : events) {
        if (!NeedPersistOnDemandEvent(event)) {
            continue;
        }
        std::string strEvent = event.ToString();
        std::string strTypeAndSaid = TypeAndSaidToString(type, systemAbilityId);
        if (preferencesUtil->IsExist(strEvent)) {
            std::string orgStrTypeAndSaid = preferencesUtil->ObtainString(strEvent, "");
            orgStrTypeAndSaid += "+";
            orgStrTypeAndSaid += strTypeAndSaid;
            HILOGI("PersistOnDemandEvent Save orgStrTypeAndSaid is : %{public}s", orgStrTypeAndSaid.c_str());
            preferencesUtil->SaveString(strEvent, orgStrTypeAndSaid);
        } else {
            preferencesUtil->SaveString(strEvent, strTypeAndSaid);
            HILOGI("PersistOnDemandEvent Save strTypeAndSaid is : %{public}s", strTypeAndSaid.c_str());
        }
    }
#endif
}

std::string DeviceStatusCollectManager::TypeAndSaidToString(OnDemandPolicyType type, int32_t systemAbilityId)
{
    std::string strSaid = std::to_string(systemAbilityId);
    if (type == OnDemandPolicyType::START_POLICY) {
        return "start#" + strSaid + "#";
    } else if (type == OnDemandPolicyType::STOP_POLICY) {
        return "stop#" + strSaid + "#";
    }
    return "";
}

void DeviceStatusCollectManager::StringToTypeAndSaid(const std::string& eventStr, OnDemandPolicyType& type,
    int32_t& systemAbilityId)
{
    std::size_t pos = eventStr.find("#");
    std::string strType = eventStr.substr(0, pos);
    if (strType == "start") {
        type = OnDemandPolicyType::START_POLICY;
    } else if (strType == "stop") {
        type = OnDemandPolicyType::STOP_POLICY;
    } else {
        HILOGW("StringToTypeAndSaid failed");
        return;
    }
    if (pos == string::npos) {
        HILOGW("StringToSaid failed");
        return;
    }
    systemAbilityId = atoi((eventStr.substr(pos + 1, eventStr.size() - pos - 1)).c_str());
    HILOGD("systemAbilityId is : %{public}d", systemAbilityId);
}

int32_t DeviceStatusCollectManager::UpdateOnDemandEvents(int32_t systemAbilityId, OnDemandPolicyType type,
    const std::vector<OnDemandEvent>& events)
{
    HILOGI("UpdateOnDemandEvents begin saId:%{public}d, type:%{public}d", systemAbilityId, type);
    std::vector<OnDemandEvent> oldEvents;
    std::unique_lock<std::shared_mutex> writeLock(saProfilesLock_);
    auto iter = std::find_if(onDemandSaProfiles_.begin(), onDemandSaProfiles_.end(),
    [systemAbilityId](auto saProfile) {
        return saProfile.saId == systemAbilityId;
    });
    if (iter == onDemandSaProfiles_.end()) {
        HILOGI("UpdateOnDemandEvents invalid saId:%{public}d", systemAbilityId);
        return ERR_INVALID_VALUE;
    }
    if (AddCollectEvents(events) != ERR_OK) {
        HILOGI("AddCollectEvents failed saId:%{public}d", systemAbilityId);
        return ERR_INVALID_VALUE;
    }
    if (type == OnDemandPolicyType::START_POLICY) {
        oldEvents = (*iter).startOnDemand.onDemandEvents;
        (*iter).startOnDemand.onDemandEvents = events;
    } else if (type == OnDemandPolicyType::STOP_POLICY) {
        oldEvents = (*iter).stopOnDemand.onDemandEvents;
        (*iter).stopOnDemand.onDemandEvents = events;
    } else {
        HILOGE("UpdateOnDemandEvents policy types");
        return ERR_INVALID_VALUE;
    }
    PersistOnDemandEvent(systemAbilityId, type, events);
    if (RemoveUnusedEventsLocked(oldEvents) != ERR_OK) {
        HILOGE("RemoveUnusedEventsLocked failed saId:%{public}d", systemAbilityId);
    }
    return ERR_OK;
}
}  // namespace OHOS
