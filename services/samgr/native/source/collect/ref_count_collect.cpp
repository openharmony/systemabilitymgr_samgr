/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "ref_count_collect.h"

namespace OHOS {
namespace {
constexpr int32_t REF_ONDEMAND_TIMER_INTERVAL = 1000 * 60 * 1;
}

RefCountCollect::RefCountCollect(const sptr<IReport>& report) : ICollectPlugin(report), timer_(nullptr) {}

void RefCountCollect::Init(const std::list<SaProfile>& saProfiles)
{
    for (const auto& saProfile : saProfiles) {
        if (saProfile.stopOnDemand.unrefUnload) {
            unrefUnloadSaList_.push_back(saProfile.saId);
        }
    }
    HILOGI("RefCountCollect Init unrefUnloadSaList size:%{public}zu", unrefUnloadSaList_.size());
}

int32_t RefCountCollect::OnStart()
{
    uint32_t timerId = 0;
    timer_ = std::make_unique<Utils::Timer>("RefCountCollectTimer", -1);
    timer_->Setup();

    if (!unrefUnloadSaList_.empty()) {
        timerId = timer_->Register(std::bind(&RefCountCollect::IdentifyUnrefOndemand, this),
            REF_ONDEMAND_TIMER_INTERVAL);
        HILOGI("RefCountCollect register ondemand timerId:%{public}u, interval:%{public}d",
            timerId, REF_ONDEMAND_TIMER_INTERVAL);
    }

    return ERR_OK;
}

int32_t RefCountCollect::OnStop()
{
    if (timer_ != nullptr) {
        HILOGI("RefCountCollect stop timer");
        timer_->Shutdown(false);
    }
    return ERR_OK;
}

void RefCountCollect::IdentifyUnrefOndemand()
{
    sptr<SystemAbilityManager> samgr = SystemAbilityManager::GetInstance();
    std::list<SaControlInfo> saControlList;

    for (const auto& saId : unrefUnloadSaList_) {
        sptr<IRemoteObject> object = samgr->CheckSystemAbility(saId);
        if (object == nullptr) {
            continue;
        }
        sptr<IPCObjectProxy> saProxy = reinterpret_cast<IPCObjectProxy*>(object.GetRefPtr());
        if (saProxy == nullptr) {
            continue;
        }
        uint32_t refCount = saProxy->GetStrongRefCountForStub();
        HILOGD("ondemand SA:%{public}d, ref count:%{public}u", saId, refCount);
        if (refCount == 1) {
            HILOGI("ondemand SA:%{public}d, ref count:1", saId);
            SaControlInfo control = { STOP_ON_DEMAND, saId };
            saControlList.push_back(control);
        }
    }
    if (saControlList.empty()) {
        return;
    }

    OnDemandEvent event = { UNREF_EVENT };
    auto callback = [event, saIdleList = std::move(saControlList)] () {
        SystemAbilityManager::GetInstance()->ProcessOnDemandEvent(event, saIdleList);
    };
    PostTask(callback);
}
} // namespace OHOS