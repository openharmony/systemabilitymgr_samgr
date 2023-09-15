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

#include "system_ability_manager_dumper_test.h"

#include "test_log.h"
#define private public
#include "system_ability_manager_dumper.h"
using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS;

namespace OHOS {
void SystemAbilityManagerDumperTest::SetUpTestCase()
{
    DTEST_LOG << "SetUpTestCase" << std::endl;
}

void SystemAbilityManagerDumperTest::TearDownTestCase()
{
    DTEST_LOG << "TearDownTestCase" << std::endl;
}

void SystemAbilityManagerDumperTest::SetUp()
{
    DTEST_LOG << "SetUp" << std::endl;
}

void SystemAbilityManagerDumperTest::TearDown()
{
    DTEST_LOG << "TearDown" << std::endl;
}

/**
 * @tc.name: CanDump001
 * @tc.desc: call CanDump, nativeTokenInfo.processName is not HIDUMPER_PROCESS_NAME
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityManagerDumperTest, CanDump001, TestSize.Level3)
{
    bool result = SystemAbilityManagerDumper::CanDump();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: ShowAllSystemAbilityInfo001
 * @tc.desc: ShowAllSystemAbilityInfo
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityManagerDumperTest, ShowAllSystemAbilityInfo001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    string result;
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    systemAbilityStateScheduler->abilityContextMap_.clear();
    systemAbilityStateScheduler->abilityContextMap_[401] = systemAbilityContext;
    SystemAbilityManagerDumper::ShowAllSystemAbilityInfo(systemAbilityStateScheduler, result);
    EXPECT_NE(result.size(), 0);
}


/**
 * @tc.name: ShowAllSystemAbilityInfo002
 * @tc.desc: ShowAllSystemAbilityInfo systemAbilityStateScheduler is nullptr
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityManagerDumperTest, ShowAllSystemAbilityInfo002, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler = nullptr;
    string result;
    SystemAbilityManagerDumper::ShowAllSystemAbilityInfo(systemAbilityStateScheduler, result);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: ShowSystemAbilityInfo001
 * @tc.desc: call ShowSystemAbilityInfo
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityManagerDumperTest, ShowSystemAbilityInfo001, TestSize.Level3)
{
    string result;
    int32_t said = 401;
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    systemAbilityStateScheduler->abilityContextMap_.clear();
    systemAbilityStateScheduler->abilityContextMap_[said] = systemAbilityContext;
    SystemAbilityManagerDumper::ShowSystemAbilityInfo(said, systemAbilityStateScheduler, result);
    EXPECT_NE(result.size(), 0);
}

/**
 * @tc.name: ShowSystemAbilityInfo002
 * @tc.desc: call ShowSystemAbilityInfo systemAbilityStateScheduler is nullptr
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityManagerDumperTest, ShowSystemAbilityInfo002, TestSize.Level3)
{
    string result;
    int32_t said = 401;
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler = nullptr;
    SystemAbilityManagerDumper::ShowSystemAbilityInfo(said, systemAbilityStateScheduler, result);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: ShowProcessInfo001
 * @tc.desc: call ShowProcessInfo, return true;
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityManagerDumperTest, ShowProcessInfo001, TestSize.Level3)
{
    string result;
    string processName = "deviceprofile";
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemAbilityStateScheduler->processContextMap_.clear();
    systemAbilityStateScheduler->processContextMap_[Str8ToStr16(processName)] = systemProcessContext;
    SystemAbilityManagerDumper::ShowProcessInfo(processName, systemAbilityStateScheduler, result);
    EXPECT_NE(result.size(), 0);
}

/**
 * @tc.name: ShowProcessInfo002
 * @tc.desc: call ShowProcessInfo, systemAbilityStateScheduler is nullptr;
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityManagerDumperTest, ShowProcessInfo002, TestSize.Level3)
{
    string result;
    string processName = "deviceprofile";
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler = nullptr;
    SystemAbilityManagerDumper::ShowProcessInfo(processName, systemAbilityStateScheduler, result);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: ShowAllSystemAbilityInfoInState001
 * @tc.desc: call ShowAllSystemAbilityInfoInState, return true;
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityManagerDumperTest, ShowAllSystemAbilityInfoInState001, TestSize.Level3)
{
    string result;
    string state = "LOADED";
    int32_t said = 401;
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    systemAbilityStateScheduler->Init(saProfiles);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    systemAbilityContext->state = SystemAbilityState::LOADED;
    systemAbilityStateScheduler->abilityContextMap_[said] = systemAbilityContext;
    SystemAbilityManagerDumper::ShowAllSystemAbilityInfoInState(state, systemAbilityStateScheduler, result);
    EXPECT_NE(result.size(), 0);
}

/**
 * @tc.name: ShowAllSystemAbilityInfoInState002
 * @tc.desc: call ShowAllSystemAbilityInfoInState, systemAbilityStateScheduler is nullptr;
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityManagerDumperTest, ShowAllSystemAbilityInfoInState002, TestSize.Level3)
{
    string result;
    string state = "LOADED";
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler = nullptr;
    SystemAbilityManagerDumper::ShowAllSystemAbilityInfoInState(state, systemAbilityStateScheduler, result);
    EXPECT_TRUE(result.empty());
}
}