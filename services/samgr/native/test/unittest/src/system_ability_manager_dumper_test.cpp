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
#include <sam_mock_permission.h>
#include <vector>
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

/**
 * @tc.name: IllegalInput001
 * @tc.desc: IllegalInput
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityManagerDumperTest, IllegalInput001, TestSize.Level3)
{
    string result;
    SystemAbilityManagerDumper::IllegalInput(result);
    EXPECT_NE(result.size(), 0);
}

/**
 * @tc.name: ShowHelp001
 * @tc.desc: ShowHelp
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityManagerDumperTest, ShowHelp001, TestSize.Level3)
{
    string result;
    SystemAbilityManagerDumper::ShowHelp(result);
    EXPECT_NE(result.size(), 0);
}

/**
 * @tc.name: CanDump002
 * @tc.desc: call CanDump, nativeTokenInfo.processName is HIDUMPER_PROCESS_NAME
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityManagerDumperTest, CanDump002, TestSize.Level3)
{
    SamMockPermission::MockProcess("hidumper_service");
    bool result = SystemAbilityManagerDumper::CanDump();
    EXPECT_TRUE(result);
}

/**
 * @tc.name: Dump001
 * @tc.desc: call Dump, ShowAllSystemAbilityInfo
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityManagerDumperTest, Dump001, TestSize.Level3)
{
    SamMockPermission::MockProcess("hidumper_service");
    std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler = nullptr;
    std::vector<std::string> args;
    args.push_back("-l");
    std::string result;
    bool ret = SystemAbilityManagerDumper::Dump(abilityStateScheduler, args, result);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: Dump002
 * @tc.desc: call Dump, ShowHelp
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityManagerDumperTest, Dump002, TestSize.Level3)
{
    SamMockPermission::MockProcess("hidumper_service");
    std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler = nullptr;
    std::vector<std::string> args;
    args.push_back("-h");
    std::string result;
    bool ret = SystemAbilityManagerDumper::Dump(abilityStateScheduler, args, result);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: Dump003
 * @tc.desc: call Dump, ShowSystemAbilityInfo
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityManagerDumperTest, Dump003, TestSize.Level3)
{
    SamMockPermission::MockProcess("hidumper_service");
    std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler = nullptr;
    std::vector<std::string> args;
    string said;
    args.push_back("-sa");
    args.push_back(said);
    std::string result;
    bool ret = SystemAbilityManagerDumper::Dump(abilityStateScheduler, args, result);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: Dump004
 * @tc.desc: call Dump, ShowProcessInfo
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityManagerDumperTest, Dump004, TestSize.Level3)
{
    SamMockPermission::MockProcess("hidumper_service");
    std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler = nullptr;
    std::vector<std::string> args;
    std::string processName;
    args.push_back("-p");
    args.push_back(processName);
    std::string result;
    bool ret = SystemAbilityManagerDumper::Dump(abilityStateScheduler, args, result);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: Dump005
 * @tc.desc: call Dump, ShowAllSystemAbilityInfoInState
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityManagerDumperTest, Dump005, TestSize.Level3)
{
    SamMockPermission::MockProcess("hidumper_service");
    std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler = nullptr;
    std::vector<std::string> args;
    std::string state;
    args.push_back("-sm");
    args.push_back(state);
    std::string result;
    bool ret = SystemAbilityManagerDumper::Dump(abilityStateScheduler, args, result);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: Dump006
 * @tc.desc: call Dump, false
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityManagerDumperTest, Dump006, TestSize.Level3)
{
    SamMockPermission::MockProcess("hidumper_service");
    std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler = nullptr;
    std::vector<std::string> args;
    std::string result;
    bool ret = SystemAbilityManagerDumper::Dump(abilityStateScheduler, args, result);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: Dump007
 * @tc.desc: call Dump, ShowAllSystemAbilityInfo
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityManagerDumperTest, Dump007, TestSize.Level3)
{
    SamMockPermission::MockProcess("hidumper_service");
    std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler = nullptr;
    std::vector<std::string> args;
    args.push_back("-sm");
    std::string result;
    bool ret = SystemAbilityManagerDumper::Dump(abilityStateScheduler, args, result);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: Dump008
 * @tc.desc: call Dump, ShowAllSystemAbilityInfoInState
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityManagerDumperTest, Dump008, TestSize.Level3)
{
    SamMockPermission::MockProcess("hidumper_service");
    std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler = nullptr;
    std::vector<std::string> args;
    std::string state;
    args.push_back("-h");
    args.push_back(state);
    std::string result;
    bool ret = SystemAbilityManagerDumper::Dump(abilityStateScheduler, args, result);
    EXPECT_FALSE(ret);
}
}