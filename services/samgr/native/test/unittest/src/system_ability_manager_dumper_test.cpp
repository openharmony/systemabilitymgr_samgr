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
#include "system_ability_manager.h"
#include "samgr_err_code.h"
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
 * @tc.name: FfrtDumpProc001
 * @tc.desc: FfrtDumpProc
 * @tc.type: FUNC
 * @tc.require: I9I86P
 */

HWTEST_F(SystemAbilityManagerDumperTest, FfrtDumpProc001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler;
    int32_t fd = -1;
    const std::vector<std::string> args;
    auto ret = SystemAbilityManagerDumper::FfrtDumpProc(abilityStateScheduler, fd, args);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: GetSAMgrFfrtInfo001
 * @tc.desc: GetSAMgrFfrtInfo
 * @tc.type: FUNC
 * @tc.require: I9I86P
 */

HWTEST_F(SystemAbilityManagerDumperTest, GetSAMgrFfrtInfo001, TestSize.Level3)
{
    std::string result;
    SystemAbilityManagerDumper::GetSAMgrFfrtInfo(result);
    EXPECT_NE(result.size(), 0);
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
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    systemAbilityContext->ownProcessContext = systemProcessContext;
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
    std::shared_ptr<SystemProcessContext> systemProcessContext = std::make_shared<SystemProcessContext>();
    std::shared_ptr<SystemAbilityContext> systemAbilityContext = std::make_shared<SystemAbilityContext>();
    systemAbilityContext->ownProcessContext = systemProcessContext;
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

/**
 * @tc.name: IpcDumpCmdParser001
 * @tc.desc: ipc dump cmd parser, IpcDumpCmdParser
 * @tc.type: FUNC
 * @tc.require: I9DR69
 */

HWTEST_F(SystemAbilityManagerDumperTest, IpcDumpCmdParser001, TestSize.Level3)
{
    SamMockPermission::MockProcess("hidumper_service");
    int32_t cmd = -1;
    std::vector<std::string> args;
    args.push_back("--ipc");
    args.push_back("all");
    args.push_back("--start-stat");
    std::string result;
    bool ret = SystemAbilityManagerDumper::IpcDumpCmdParser(cmd, args);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(cmd == 0);
}

/**
 * @tc.name: IpcDumpCmdParser002
 * @tc.desc: ipc dump cmd parser, IpcDumpCmdParser
 * @tc.type: FUNC
 * @tc.require: I9DR69
 */

HWTEST_F(SystemAbilityManagerDumperTest, IpcDumpCmdParser002, TestSize.Level3)
{
    SamMockPermission::MockProcess("hidumper_service");
    int32_t cmd = 0;
    std::vector<std::string> args;
    args.push_back("--ipc");
    args.push_back("all");
    args.push_back("--stop-stat");
    std::string result;
    bool ret = SystemAbilityManagerDumper::IpcDumpCmdParser(cmd, args);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(cmd == 1);
}

/**
 * @tc.name: IpcDumpCmdParser003
 * @tc.desc: ipc dump cmd parser, IpcDumpCmdParser
 * @tc.type: FUNC
 * @tc.require: I9DR69
 */

HWTEST_F(SystemAbilityManagerDumperTest, IpcDumpCmdParser003, TestSize.Level3)
{
    SamMockPermission::MockProcess("hidumper_service");
    int32_t cmd = 0;
    std::vector<std::string> args;
    args.push_back("--ipc");
    args.push_back("all");
    args.push_back("--stat");
    std::string result;
    bool ret = SystemAbilityManagerDumper::IpcDumpCmdParser(cmd, args);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(cmd == 2);
}

/**
 * @tc.name: IpcDumpCmdParser004
 * @tc.desc: ipc dump cmd parser, IpcDumpCmdParser
 * @tc.type: FUNC
 * @tc.require: I9DR69
 */

HWTEST_F(SystemAbilityManagerDumperTest, IpcDumpCmdParser004, TestSize.Level3)
{
    SamMockPermission::MockProcess("hidumper_service");
    int32_t cmd = 0;
    std::vector<std::string> args;
    args.push_back("--ipc");
    args.push_back("wifi_manager_service");
    args.push_back("--start-stat");
    std::string result;
    bool ret = SystemAbilityManagerDumper::IpcDumpCmdParser(cmd, args);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(cmd == 0);
}

/**
 * @tc.name: IpcDumpCmdParser005
 * @tc.desc: ipc dump cmd parser, IpcDumpCmdParser
 * @tc.type: FUNC
 * @tc.require: I9DR69
 */

HWTEST_F(SystemAbilityManagerDumperTest, IpcDumpCmdParser005, TestSize.Level3)
{
    SamMockPermission::MockProcess("hidumper_service");
    int32_t cmd = 0;
    std::vector<std::string> args;
    args.push_back("--ipc");
    args.push_back("wifi_manager_service");
    args.push_back("--stop-stat");
    std::string result;
    bool ret = SystemAbilityManagerDumper::IpcDumpCmdParser(cmd, args);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(cmd == 1);
}

/**
 * @tc.name: IpcDumpCmdParser006
 * @tc.desc: ipc dump cmd parser, IpcDumpCmdParser
 * @tc.type: FUNC
 * @tc.require: I9DR69
 */

HWTEST_F(SystemAbilityManagerDumperTest, IpcDumpCmdParser006, TestSize.Level3)
{
    SamMockPermission::MockProcess("hidumper_service");
    int32_t cmd = 0;
    std::vector<std::string> args;
    args.push_back("--ipc");
    args.push_back("wifi_manager_service");
    args.push_back("--stat");
    std::string result;
    bool ret = SystemAbilityManagerDumper::IpcDumpCmdParser(cmd, args);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(cmd == 2);
}

/**
 * @tc.name: IpcDumpCmdParser007
 * @tc.desc: ipc dump cmd parser, IpcDumpCmdParser
 * @tc.type: FUNC
 * @tc.require: I9DR69
 */

HWTEST_F(SystemAbilityManagerDumperTest, IpcDumpCmdParser007, TestSize.Level3)
{
    SamMockPermission::MockProcess("demo_service");
    int32_t cmd = 0;
    std::vector<std::string> args;
    args.push_back("--ipc");
    args.push_back("wifi_manager_service");
    args.push_back("--start-stat");
    std::string result;
    bool ret = SystemAbilityManagerDumper::IpcDumpCmdParser(cmd, args);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IpcDumpCmdParser008
 * @tc.desc: ipc dump cmd parser, IpcDumpCmdParser
 * @tc.type: FUNC
 * @tc.require: I9DR69
 */

HWTEST_F(SystemAbilityManagerDumperTest, IpcDumpCmdParser008, TestSize.Level3)
{
    SamMockPermission::MockProcess("demo_service");
    int32_t cmd = 0;
    std::vector<std::string> args;
    args.push_back("--ipc");
    args.push_back("wifi_manager_service");
    args.push_back("--stop-stat");
    std::string result;
    bool ret = SystemAbilityManagerDumper::IpcDumpCmdParser(cmd, args);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IpcDumpCmdParser009
 * @tc.desc: ipc dump cmd parser, IpcDumpCmdParser
 * @tc.type: FUNC
 * @tc.require: I9DR69
 */

HWTEST_F(SystemAbilityManagerDumperTest, IpcDumpCmdParser009, TestSize.Level3)
{
    SamMockPermission::MockProcess("demo_service");
    int32_t cmd = 0;
    std::vector<std::string> args;
    args.push_back("--ipc");
    args.push_back("wifi_manager_service");
    args.push_back("--stat");
    std::string result;
    bool ret = SystemAbilityManagerDumper::IpcDumpCmdParser(cmd, args);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IpcDumpCmdParser010
 * @tc.desc: ipc dump cmd parser, IpcDumpCmdParser
 * @tc.type: FUNC
 * @tc.require: I9DR69
 */

HWTEST_F(SystemAbilityManagerDumperTest, IpcDumpCmdParser010, TestSize.Level3)
{
    SamMockPermission::MockProcess("demo_service");
    int32_t cmd = 0;
    std::vector<std::string> args;
    args.push_back("--ipc");
    std::string result;
    bool ret = SystemAbilityManagerDumper::IpcDumpCmdParser(cmd, args);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IpcDumpCmdParser011
 * @tc.desc: ipc dump cmd parser, IpcDumpCmdParser
 * @tc.type: FUNC
 * @tc.require: I9DR69
 */

HWTEST_F(SystemAbilityManagerDumperTest, IpcDumpCmdParser011, TestSize.Level3)
{
    SamMockPermission::MockProcess("demo_service");
    int32_t cmd = 0;
    std::vector<std::string> args;
    args.push_back("--ipc");
    args.push_back("wifi_manager_service");
    std::string result;
    bool ret = SystemAbilityManagerDumper::IpcDumpCmdParser(cmd, args);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IpcDumpCmdParser012
 * @tc.desc: ipc dump cmd parser, IpcDumpCmdParser
 * @tc.type: FUNC
 * @tc.require: I9DR69
 */

HWTEST_F(SystemAbilityManagerDumperTest, IpcDumpCmdParser012, TestSize.Level3)
{
    SamMockPermission::MockProcess("demo_service");
    int32_t cmd = 0;
    std::vector<std::string> args;
    args.push_back("--ipc");
    args.push_back("wifi_manager_service");
    args.push_back("test001");
    std::string result;
    bool ret = SystemAbilityManagerDumper::IpcDumpCmdParser(cmd, args);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IpcDumpCmdParser013
 * @tc.desc: ipc dump cmd parser, IpcDumpCmdParser
 * @tc.type: FUNC
 * @tc.require: I9DR69
 */

HWTEST_F(SystemAbilityManagerDumperTest, IpcDumpCmdParser013, TestSize.Level3)
{
    SamMockPermission::MockProcess("demo_service");
    int32_t cmd = 0;
    std::vector<std::string> args;
    args.push_back("--ipc");
    args.push_back("testProcess");
    args.push_back("test001");
    std::string result;
    bool ret = SystemAbilityManagerDumper::IpcDumpCmdParser(cmd, args);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IpcDumpCmdParser014
 * @tc.desc: ipc dump cmd parser, IpcDumpCmdParser
 * @tc.type: FUNC
 * @tc.require: I9DR69
 */

HWTEST_F(SystemAbilityManagerDumperTest, IpcDumpCmdParser014, TestSize.Level3)
{
    SamMockPermission::MockProcess("hidumper_service");
    int32_t cmd = 0;
    std::vector<std::string> args;
    args.push_back("--ipc");
    args.push_back("testProcess");
    args.push_back("test001");
    std::string result;
    bool ret = SystemAbilityManagerDumper::IpcDumpCmdParser(cmd, args);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IpcDumpIsAllProcess001
 * @tc.desc: is ipc dump all process cmd, IpcDumpIsAllProcess
 * @tc.type: FUNC
 * @tc.require: I9DR69
 */

HWTEST_F(SystemAbilityManagerDumperTest, IpcDumpIsAllProcess001, TestSize.Level3)
{
    std::string processName = "all";
    bool ret = SystemAbilityManagerDumper::IpcDumpIsAllProcess(processName);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: IpcDumpIsAllProcess002
 * @tc.desc: is ipc dump all process cmd, IpcDumpIsAllProcess
 * @tc.type: FUNC
 * @tc.require: I9DR69
 */

HWTEST_F(SystemAbilityManagerDumperTest, IpcDumpIsAllProcess002, TestSize.Level3)
{
    std::string processName = "test001";
    bool ret = SystemAbilityManagerDumper::IpcDumpIsAllProcess(processName);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IpcDumpIsAllProcess003
 * @tc.desc: is ipc dump all process cmd, IpcDumpIsAllProcess
 * @tc.type: FUNC
 * @tc.require: I9DR69
 */

HWTEST_F(SystemAbilityManagerDumperTest, IpcDumpIsAllProcess003, TestSize.Level3)
{
    std::string processName = "";
    bool ret = SystemAbilityManagerDumper::IpcDumpIsAllProcess(processName);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IpcDumpIsSamgr001
 * @tc.desc: is ipc dump samgr process cmd, IpcDumpIsSamgr001
 * @tc.type: FUNC
 * @tc.require: I9DR69
 */

HWTEST_F(SystemAbilityManagerDumperTest, IpcDumpIsSamgr001, TestSize.Level3)
{
    std::string processName = "samgr";
    bool ret = SystemAbilityManagerDumper::IpcDumpIsSamgr(processName);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: IpcDumpIsSamgr002
 * @tc.desc: is ipc dump samgr process cmd, IpcDumpIsSamgr002
 * @tc.type: FUNC
 * @tc.require: I9DR69
 */

HWTEST_F(SystemAbilityManagerDumperTest, IpcDumpIsSamgr002, TestSize.Level3)
{
    std::string processName = "test001";
    bool ret = SystemAbilityManagerDumper::IpcDumpIsSamgr(processName);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: StartSamgrIpcStatistics001
 * @tc.desc: test StartSamgrIpcStatistics
 * @tc.type: FUNC
 * @tc.require: I6W28
 */
HWTEST_F(SystemAbilityManagerDumperTest, StartSamgrIpcStatistics001, TestSize.Level2)
{
    std::string result;
    bool ret = SystemAbilityManagerDumper::StartSamgrIpcStatistics(result);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: StopSamgrIpcStatistics001
 * @tc.desc: test StopSamgrIpcStatistics
 * @tc.type: FUNC
 * @tc.require: I6W28
 */
HWTEST_F(SystemAbilityManagerDumperTest, StopSamgrIpcStatistics001, TestSize.Level2)
{
    std::string result;
    bool ret = SystemAbilityManagerDumper::StopSamgrIpcStatistics(result);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: GetSamgrIpcStatistics001
 * @tc.desc: test GetSamgrIpcStatistics
 * @tc.type: FUNC
 * @tc.require: I6W28
 */
HWTEST_F(SystemAbilityManagerDumperTest, GetSamgrIpcStatistics001, TestSize.Level2)
{
    std::string result;
    bool ret = SystemAbilityManagerDumper::GetSamgrIpcStatistics(result);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: FfrtDumpParser001
 * @tc.desc: test FfrtDumpParser
 * @tc.type: FUNC
 * @tc.require: I6W28
 */
HWTEST_F(SystemAbilityManagerDumperTest, FfrtDumpParser001, TestSize.Level2)
{
    SamMockPermission::MockProcess("hidumper_service");
    std::vector<int32_t> processIds;
    std::string pidStr = "123";
    bool ret = SystemAbilityManagerDumper::FfrtDumpParser(processIds, pidStr);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: FfrtDumpParser002
 * @tc.desc: test FfrtDumpParser
 * @tc.type: FUNC
 * @tc.require: I6W28
 */
HWTEST_F(SystemAbilityManagerDumperTest, FfrtDumpParser002, TestSize.Level2)
{
    SamMockPermission::MockProcess("hidumper_service");
    std::vector<int32_t> processIds;
    std::string pidStr = "123|234";
    bool ret = SystemAbilityManagerDumper::FfrtDumpParser(processIds, pidStr);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(processIds.size(), 2);
}

/**
 * @tc.name: FfrtDumpParser003
 * @tc.desc: test FfrtDumpParser
 * @tc.type: FUNC
 * @tc.require: I6W28
 */
HWTEST_F(SystemAbilityManagerDumperTest, FfrtDumpParser003, TestSize.Level2)
{
    SamMockPermission::MockProcess("hidumper_service");
    std::vector<int32_t> processIds;
    std::string pidStr = "12k|234";
    bool ret = SystemAbilityManagerDumper::FfrtDumpParser(processIds, pidStr);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(processIds.size(), 1);
}

/**
 * @tc.name: FfrtDumpParser004
 * @tc.desc: test FfrtDumpParser
 * @tc.type: FUNC
 * @tc.require: I6W28
 */
HWTEST_F(SystemAbilityManagerDumperTest, FfrtDumpParser004, TestSize.Level2)
{
    SamMockPermission::MockProcess("hidumper_service");
    std::vector<int32_t> processIds;
    std::string pidStr = "12k";
    bool ret = SystemAbilityManagerDumper::FfrtDumpParser(processIds, pidStr);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(processIds.size(), 0);
}

/**
 * @tc.name: GetFfrtDumpInfoProc001
 * @tc.desc: test GetFfrtDumpInfoProc
 * @tc.type: FUNC
 * @tc.require: I6W28
 */
HWTEST_F(SystemAbilityManagerDumperTest, GetFfrtDumpInfoProc001, TestSize.Level2)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::vector<std::string> args;
    args.emplace_back("--ffrt");
    std::string result;
    bool ret = SystemAbilityManagerDumper::GetFfrtDumpInfoProc(systemAbilityStateScheduler, args, result);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: GetFfrtDumpInfoProc002
 * @tc.desc: test GetFfrtDumpInfoProc
 * @tc.type: FUNC
 * @tc.require: I6W28
 */
HWTEST_F(SystemAbilityManagerDumperTest, GetFfrtDumpInfoProc002, TestSize.Level2)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::vector<std::string> args;
    args.emplace_back("--ffrt");
    args.emplace_back("");
    std::string result;
    bool ret = SystemAbilityManagerDumper::GetFfrtDumpInfoProc(systemAbilityStateScheduler, args, result);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: GetFfrtDumpInfoProc003
 * @tc.desc: test GetFfrtDumpInfoProc
 * @tc.type: FUNC
 * @tc.require: I6W28
 */
HWTEST_F(SystemAbilityManagerDumperTest, GetFfrtDumpInfoProc003, TestSize.Level2)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::vector<std::string> args;
    args.emplace_back("--ffrt");
    args.emplace_back("12k");
    std::string result;
    bool ret = SystemAbilityManagerDumper::GetFfrtDumpInfoProc(systemAbilityStateScheduler, args, result);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: GetFfrtDumpInfoProc004
 * @tc.desc: test GetFfrtDumpInfoProc
 * @tc.type: FUNC
 * @tc.require: I6W28
 */
HWTEST_F(SystemAbilityManagerDumperTest, GetFfrtDumpInfoProc004, TestSize.Level2)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->abilityStateScheduler_ = systemAbilityStateScheduler;
    std::vector<std::string> args;
    args.emplace_back("--ffrt");
    args.emplace_back("1234|12345");
    std::string result;
    bool ret = SystemAbilityManagerDumper::GetFfrtDumpInfoProc(systemAbilityStateScheduler, args, result);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: GetFfrtDumpInfoProc005
 * @tc.desc: test GetFfrtDumpInfoProc
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityManagerDumperTest, GetFfrtDumpInfoProc005, TestSize.Level1)
{
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    saMgr->abilityStateScheduler_ = systemAbilityStateScheduler;

    u16string procName = u"proTest";
    auto processContext = std::make_shared<SystemProcessContext>();
    processContext->pid = 123;
    processContext->processName = u"proTest";
    saMgr->abilityStateScheduler_->processContextMap_[procName] = processContext;
    std::vector<std::string> args;
    args.emplace_back("--ffrt");
    args.emplace_back("123");
    std::string result;
    bool ret = SystemAbilityManagerDumper::GetFfrtDumpInfoProc(systemAbilityStateScheduler, args, result);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: SaveDumpResultToFd001
 * @tc.desc: test SaveDumpResultToFd
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityManagerDumperTest, SaveDumpResultToFd001, TestSize.Level1)
{
    int32_t fd = -1;
    std::string result = "";
    int32_t ret = SystemAbilityManagerDumper::SaveDumpResultToFd(fd, result);
    EXPECT_EQ(ret, SAVE_FD_FAIL);
}

/**
 * @tc.name: SaveDumpResultToFd002
 * @tc.desc: test SaveDumpResultToFd
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityManagerDumperTest, SaveDumpResultToFd002, TestSize.Level1)
{
    int32_t fd = 1;
    std::string result = "";
    int32_t ret = SystemAbilityManagerDumper::SaveDumpResultToFd(fd, result);
    EXPECT_EQ(ret, ERR_OK);
}
}