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

#include "gtest/gtest.h"
#include "parameter.h"
#include "test_log.h"

namespace OHOS {
namespace SAMGR {
class SystemAbilityManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void SystemAbilityManagerTest::SetUpTestCase()
{
    DTEST_LOG << "SystemAbilityManagerTest SetUpTestCase" << std::endl;
}

void SystemAbilityManagerTest::TearDownTestCase()
{
    DTEST_LOG << "SystemAbilityManagerTest TearDownTestCase" << std::endl;
}

void SystemAbilityManagerTest::SetUp()
{
    DTEST_LOG << "SystemAbilityManagerTest SetUp" << std::endl;
}

void SystemAbilityManagerTest::TearDown()
{
    DTEST_LOG << "SystemAbilityManagerTest TearDown" << std::endl;
}

/**
 * @tc.name: param check samgr ready event
 * @tc.desc: param check samgr ready event
 * @tc.type: FUNC
 * @tc.require: AR000H0IFC
 */
HWTEST_F(SystemAbilityManagerTest, SamgrReady001, TestSize.Level1)
{
    DTEST_LOG << " SamgrReady001 start " << std::endl;
    /**
     * @tc.steps: step1. param check samgr ready event
     * @tc.expected: step1. param check samgr ready event
     */
    auto ret = WaitParameter("bootevent.samgr.ready", "true", 1);
    ASSERT_EQ(ret, 0);
}
}
}