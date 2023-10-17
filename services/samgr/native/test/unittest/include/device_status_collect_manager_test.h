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

#ifndef SAMGR_TEST_UNITTEST_INCLUDE_DEVICE_STATUS_COLLECT_MANAGER_TEST_H
#define SAMGR_TEST_UNITTEST_INCLUDE_DEVICE_STATUS_COLLECT_MANAGER_TEST_H

#include <condition_variable>
#include <shared_mutex>

#include "ffrt_handler.h"
#include "gtest/gtest.h"
#include "icollect_plugin.h"

namespace OHOS {
class DeviceStatusCollectManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    void PostTask(std::shared_ptr<FFRTHandler>& collectHandler);
    bool isCaseDone;
    std::mutex caseDoneLock_;
    std::condition_variable caseDoneCondition_;
};

class MockCollectPlugin : public ICollectPlugin {
public:
    explicit MockCollectPlugin(const sptr<IReport>& report) : ICollectPlugin(report) {};
    ~MockCollectPlugin() = default;
    bool CheckCondition(const OnDemandCondition& condition) override;

    int32_t OnStart() override
    {
        return 0;
    }

    int32_t OnStop() override
    {
        return 0;
    }
    
    bool mockCheckConditionResult_ = false;
};
} // OHOS
#endif /* SAMGR_TEST_UNITTEST_INCLUDE_DEVICE_STATUS_COLLECT_MANAGER_TEST_H */
