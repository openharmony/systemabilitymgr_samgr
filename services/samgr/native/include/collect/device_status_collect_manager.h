/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_SYSTEM_ABILITY_MANAGER_DEVICE_STATUS_COLLECT_MANAGER_H
#define OHOS_SYSTEM_ABILITY_MANAGER_DEVICE_STATUS_COLLECT_MANAGER_H

#include <list>
#include <memory>
#include <shared_mutex>

#include "ffrt_handler.h"
#include "icollect_plugin.h"
#include "system_ability_ondemand_reason.h"
#include "system_ability_on_demand_event.h"

namespace OHOS {
class DeviceStatusCollectManager : public IReport {
public:
    DeviceStatusCollectManager() = default;
    ~DeviceStatusCollectManager() = default;
    void Init(const std::list<SaProfile>& saProfiles);
    void UnInit();
    void CleanFfrt();
    void SetFfrt();
    void ReportEvent(const OnDemandEvent& event) override;
    void StartCollect();
    void PostDelayTask(std::function<void()> callback, int32_t delayTime) override;
    int32_t GetOnDemandReasonExtraData(int64_t extraDataId, OnDemandReasonExtraData& extraData);
    int32_t GetOnDemandEvents(int32_t systemAbilityId, OnDemandPolicyType type,
        std::vector<OnDemandEvent>& events);
    int32_t UpdateOnDemandEvents(int32_t systemAbilityId, OnDemandPolicyType type,
        const std::vector<OnDemandEvent>& events);
    void SaveSaExtraDataId(int32_t saId, int64_t extraDataId);
    void RemoveSaExtraDataId(int64_t extraDataId);
    void ClearSaExtraDataId(int32_t saId);
    void SaveCacheCommonEventSaExtraId(const OnDemandEvent& event,
        const std::list<SaControlInfo>& saControlList);
    int32_t GetSaExtraDataIdList(int32_t saId, std::vector<int64_t>& extraDataIdList,
        const std::string& eventName = "");
    int32_t IsExistInPluginMap(int32_t eventId);
    void RemoveWhiteCommonEvent();
private:
    bool NeedPersistOnDemandEvent(const OnDemandEvent& event);
    void PersistOnDemandEvent(int32_t systemAbilityId, OnDemandPolicyType type,
        const std::vector<OnDemandEvent>& events);
    void StringToTypeAndSaid(const std::string& str, OnDemandPolicyType& type, int32_t& systemAbilityId);
    std::string TypeAndSaidToString(OnDemandPolicyType type, int32_t systemAbilityId);
    void FilterOnDemandSaProfiles(const std::list<SaProfile>& saProfiles);
    void GetSaControlListByPersistEvent(const OnDemandEvent& event,
        std::list<SaControlInfo>& saControlList);
    void GetSaControlListByEvent(const OnDemandEvent& event, std::list<SaControlInfo>& saControlList);
    void SortSaControlListByLoadPriority(std::list<SaControlInfo>& saControlList);
    bool CheckEventUsedLocked(const OnDemandEvent& events);
    static bool IsSameEvent(const OnDemandEvent& ev1, const OnDemandEvent& ev2);
    bool IsSameEventName(const OnDemandEvent& ev1, const OnDemandEvent& ev2);
    bool CheckConditions(const OnDemandEvent& onDemandEvent);
    bool CheckExtraMessages(const OnDemandEvent& ev1, const OnDemandEvent& ev2);
    int32_t AddCollectEvents(const std::vector<OnDemandEvent>& events);
    int32_t RemoveUnusedEventsLocked(const std::vector<OnDemandEvent>& events);
    std::map<int32_t, sptr<ICollectPlugin>> collectPluginMap_;
    std::shared_ptr<FFRTHandler> collectHandler_;
    std::shared_mutex saProfilesLock_;
    std::list<SaProfile> onDemandSaProfiles_;
};
} // namespace OHOS
#endif // OHOS_SYSTEM_ABILITY_MANAGER_DEVICE_STATUS_COLLECT_MANAGER_H