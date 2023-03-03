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

#include "icollect_plugin.h"

namespace OHOS {
ICollectPlugin::ICollectPlugin(const sptr<IReport>& report) : report_(report)
{
}

void ICollectPlugin::ReportEvent(const OnDemandEvent& event)
{
    if (report_ != nullptr) {
        report_->ReportEvent(event);
    }
}

void ICollectPlugin::PostDelayTask(std::function<void()> callback, int32_t delayTime)
{
    if (report_ != nullptr) {
        report_->PostDelayTask(callback, delayTime);
    }
}
}  // namespace OHOS
