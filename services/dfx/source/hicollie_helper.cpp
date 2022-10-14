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
#include "hicollie_helper.h"
#ifdef HICOLLIE_ENABLE
#include "xcollie/watchdog.h"
#endif

namespace OHOS {
int HicollieHelper::AddThread(const std::string& name, std::shared_ptr<AppExecFwk::EventHandler> handler,
    std::function<void(const std::string& name, int waitState)> timeOutCallback, uint64_t interval)
{
#ifdef HICOLLIE_ENABLE
    return HiviewDFX::Watchdog::GetInstance().AddThread(name, handler, timeOutCallback, interval);
#else
    // HICOLLIE_ENABLE is false, do nothing and return 1.
    return 1;
#endif
}
}  // namespace OHOS