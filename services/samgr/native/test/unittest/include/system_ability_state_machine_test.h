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

#ifndef SYSTEM_ABILITY_STATE_MACHINE_TEST_H
#define SYSTEM_ABILITY_STATE_MACHINE_TEST_H

#include "gtest/gtest.h"
#include "schedule/system_ability_state_listener.h"

namespace OHOS {
class SystemAbilityStateMachineTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};
class StateMachineListener : public SystemAbilityStateListener {
public:
    StateMachineListener() = default;
    ~StateMachineListener() = default;
    void OnAbilityNotLoadedLocked(int32_t systemAbilityId) override;
    void OnAbilityLoadingLocked(int32_t systemAbilityId) override;
    void OnAbilityLoadedLocked(int32_t systemAbilityId) override;
    void OnAbilityUnloadableLocked(int32_t systemAbilityId) override;
    void OnAbilityUnloadingLocked(int32_t systemAbilityId) override;
    void OnProcessNotStartedLocked(const std::u16string& processName) override;
    void OnProcessStartedLocked(const std::u16string& processName) override;
    void OnProcessStoppingLocked(const std::u16string& processName) override;
};
} // OHOS
#endif /* SYSTEM_ABILITY_STATE_MACHINE_TEST_H */