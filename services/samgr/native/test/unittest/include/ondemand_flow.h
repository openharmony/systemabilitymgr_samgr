/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef SAMGR_TEST_UNITTEST_INCLUDE_ONDEMAND_FLOW_H
#define SAMGR_TEST_UNITTEST_INCLUDE_ONDEMAND_FLOW_H

#include <string>

#ifdef SUPPORT_MULTI_INSTANCE
#include "ondemand_helper.h"

namespace OHOS {

struct SubscriptionFlowOptions {
    OnDemandTarget target;
    bool timeoutLoad = false;
    bool verifyUnsubscribe = false;
    bool subscribeProcess = false;
    bool expectRejected = false;
    std::string processName;
};

struct GlobalSubscriptionOptions {
    int32_t saId = 0;
    int32_t foregroundUserId = ONDEMAND_BASE_USER_ID;
    int32_t otherUserId = ONDEMAND_BASE_USER_ID;
    bool switchForeground = false;
    bool activateNewUser = false;
    std::string processName;
};

struct RemoveFlowOptions {
    OnDemandTarget target;
    int32_t isolationUserId = ONDEMAND_BASE_USER_ID;
    std::string processName;
};

struct ApiFlowOptions {
    OnDemandTarget target;
    bool expectRejected = false;
    std::string processName;
};

struct ConcurrentLoadOptions {
    int32_t saId = 0;
    int32_t firstUserId = ONDEMAND_BASE_USER_ID;
    int32_t secondUserId = ONDEMAND_BASE_USER_ID;
};

int32_t RunSubscriptionFlow(OnDemandHelper& helper, const SubscriptionFlowOptions& options);
int32_t RunGlobalSubscriptionFlow(OnDemandHelper& helper, const GlobalSubscriptionOptions& options);
int32_t RunRemoveFlow(OnDemandHelper& helper, const RemoveFlowOptions& options);
int32_t RunApiFlow(OnDemandHelper& helper, const ApiFlowOptions& options);
int32_t RunConcurrentLoadFlow(OnDemandHelper& helper, const ConcurrentLoadOptions& options);

} // namespace OHOS
#endif

#endif // SAMGR_TEST_UNITTEST_INCLUDE_ONDEMAND_FLOW_H
