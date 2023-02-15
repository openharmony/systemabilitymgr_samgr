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

#ifndef SERVICES_DFX_INCLUDE_HICOLLIE_HELLER_H
#define SERVICES_DFX_INCLUDE_HICOLLIE_HELLER_H
#include "event_handler.h"
#include <string>
namespace OHOS {
class HicollieHelper {
public:
    static int AddThread(const std::string& name, std::shared_ptr<AppExecFwk::EventHandler> handler,
        std::function<void(const std::string& name, int waitState)> timeOutCallback, uint64_t interval);
};
}  // namespace OHOS
#endif  // SERVICES_DFX_INCLUDE_HICOLLIE_HELLER_H
