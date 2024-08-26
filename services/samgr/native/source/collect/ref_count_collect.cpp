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
constexpr int32_t REF_RESIDENT_TIMER_INTERVAL = 1000 * 60 * 10;
constexpr int32_t REF_ONDEMAND_TIMER_INTERVAL = 1000 * 60 * 1;
}

RefCountCollect::RefCountCollect(const sptr<IReport>& report) : ICollectPlugin(report), timer_(nullptr) {}

void RefCountCollect::Init(const std::list<SaProfile>& saProfiles)
{
    for (const auto& saProfile : saProfiles) {
        if (saProfile.runOnCreate) {
            residentSaList_.push_back(saProfile.saId);
        } else if (saProfile.stopOnDemand.unrefUnload) {
            unrefUnloadSaList_.push_back(saProfile.saId);
        }
    }
    HILOGI("RefCountCollect Init residentSaList size:%{public}zu, unrefUnloadSaList size:%{public}zu",
        residentSaList_.size(), unrefUnloadSaList_.size());
}

int32_t RefCountCollect::OnStart()
{
    uint32_t timerId = 0;
    timer_ = std::make_unique<Utils::Timer>("RefCountCollectTimer");
    timer_->Setup();

    if (!residentSaList_.empty()) {
        timerId = timer_->Register(std::bind(&RefCountCollect::IdentifyUnrefResident, this),
            REF_RESIDENT_TIMER_INTERVAL);
        HILOGI("RefCountCollect register resident timerId:%{public}u, interval:%{public}d",
            timerId, REF_RESIDENT_TIMER_INTERVAL);
    }

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
        timer_->Shutdown();
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
        HILOGD("ondemand SA:%{public}d, ref count:%{public}d", saId, refCount);
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

void RefCountCollect::IdentifyUnrefResident()
{
    sptr<SystemAbilityManager> samgr = SystemAbilityManager::GetInstance();
    for (const auto& saId : residentSaList_) {
        sptr<IRemoteObject> object = samgr->CheckSystemAbility(saId);
        if (object == nullptr) {
            continue;
        }
        sptr<IPCObjectProxy> saProxy = reinterpret_cast<IPCObjectProxy*>(object.GetRefPtr());
        if (saProxy == nullptr) {
            continue;
        }
        uint32_t refCount = saProxy->GetStrongRefCountForStub();
        HILOGD("resident SA:%{public}d, ref count:%{public}d", saId, refCount);
        if (refCount == 1) {
            ReportSAIdle(saId, "ref count 1");
            HILOGI("resident SA:%{public}d, ref count:1", saId);
        }
    }
}
} // namespace OHOS