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

#ifndef OHOS_SYSTEM_ABILITY_MANAGER_COLLECT_PLUGIN_INTERFACE_H
#define OHOS_SYSTEM_ABILITY_MANAGER_COLLECT_PLUGIN_INTERFACE_H

#include <list>

#include "errors.h"
#include "ireport.h"
#include "refbase.h"
#include "sa_profiles.h"
#include "system_ability_ondemand_reason.h"

namespace OHOS {
class ICollectPlugin : public virtual RefBase {
public:
    explicit ICollectPlugin(const sptr<IReport>& report);
    virtual ~ICollectPlugin() = default;

    virtual void CleanFfrt()
    {
    }
    virtual void SetFfrt()
    {
    }

    virtual void RemoveWhiteCommonEvent() {}
    virtual int32_t OnStart() = 0;
    virtual int32_t OnStop() = 0;
    virtual void Init(const std::list<SaProfile>& saProfiles) {};

    virtual bool CheckCondition(const OnDemandCondition& condition)
    {
        return false;
    }
    
    virtual bool CheckExtraMessage(int64_t extraDataId, const OnDemandEvent& profileEvent)
    {
        return true;
    }

    virtual bool GetOnDemandReasonExtraData(int64_t extraDataId, OnDemandReasonExtraData& extraData)
    {
        return false;
    }

    virtual int32_t AddCollectEvent(const OnDemandEvent& event)
    {
        return ERR_OK;
    }

    virtual int32_t RemoveUnusedEvent(const OnDemandEvent& event)
    {
        return ERR_OK;
    }
    virtual void SaveSaExtraDataId(int32_t saId, int64_t extraDataId) {};
    virtual void ClearSaExtraDataId(int32_t saId) {};
    virtual void SaveCacheCommonEventSaExtraId(const OnDemandEvent& event,
        const std::list<SaControlInfo>& saControlList) {};
    virtual int32_t GetSaExtraDataIdList(int32_t saId, std::vector<int64_t>& extraDataIdList,
        const std::string& eventName = "")
    {
        return ERR_OK;
    }
    void ReportEvent(const OnDemandEvent& event);
    void PostDelayTask(std::function<void()> callback, int32_t delayTime);
private:
    sptr<IReport> report_;
};
} // namespace OHOS
#endif // OHOS_SYSTEM_ABILITY_MANAGER_COLLECT_PLUGIN_INTERFACE_H