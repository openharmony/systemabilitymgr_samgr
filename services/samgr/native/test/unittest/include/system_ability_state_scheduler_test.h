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

#ifndef SYSTEM_ABILITY_STATE_SCHEDULER_TEST_H
#define SYSTEM_ABILITY_STATE_SCHEDULER_TEST_H

#include "gtest/gtest.h"
#include "system_process_status_change_stub.h"

namespace OHOS {
class SystemAbilityStateSchedulerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};
class SystemProcessStatusChange : public SystemProcessStatusChangeStub {
public:
    void OnSystemProcessStarted(SystemProcessInfo& systemProcessInfo) override;
    void OnSystemProcessStopped(SystemProcessInfo& systemProcessInfo) override;
};
void SystemProcessStatusChange::OnSystemProcessStarted(SystemProcessInfo& systemProcessInfo)
{
    std::cout << "OnSystemProcessStarted, processName: " << systemProcessInfo.processName << " pid:"
        << systemProcessInfo.pid << " uid:" << systemProcessInfo.uid << std::endl;
}

void SystemProcessStatusChange::OnSystemProcessStopped(SystemProcessInfo& systemProcessInfo)
{
    std::cout << "OnSystemProcessStopped, processName: " << systemProcessInfo.processName << " pid:"
        << systemProcessInfo.pid << " uid:" << systemProcessInfo.uid << std::endl;
}
} // OHOS
#endif /* SYSTEM_ABILITY_STATE_SCHEDULER_TEST_H */