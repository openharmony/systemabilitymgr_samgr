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
#include "ability_death_recipient.h"
#include "system_ability_status_change_proxy.h"
#include "ipc_skeleton.h"

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
namespace {
const std::string strHidumperSerName = "hidumper";
const std::string strArgsQuerySA = "-sa";
const std::string strArgsQueryProcess = "-p";
const std::string strArgsHelp = "-h";
const std::string strArgsQueryAll = "-l";
const std::string strIllegal = "The arguments are illegal and you can enter '-h' for help.\n";
constexpr int LISTENER_BASE_INDEX = 1;
}
void InitSaMgr(sptr<SystemAbilityManager>& saMgr)
{
    saMgr->abilityDeath_ = sptr<IRemoteObject::DeathRecipient>(new AbilityDeathRecipient());
    saMgr->systemProcessDeath_ = sptr<IRemoteObject::DeathRecipient>(new SystemProcessDeathRecipient());
    saMgr->abilityStatusDeath_ = sptr<IRemoteObject::DeathRecipient>(new AbilityStatusDeathRecipient());
    saMgr->abilityCallbackDeath_ = sptr<IRemoteObject::DeathRecipient>(new AbilityCallbackDeathRecipient());
    saMgr->remoteCallbackDeath_ = sptr<IRemoteObject::DeathRecipient>(new RemoteCallbackDeathRecipient());
    saMgr->workHandler_ = make_shared<FFRTHandler>("workHandler");
    saMgr->collectManager_ = sptr<DeviceStatusCollectManager>(new DeviceStatusCollectManager());
    saMgr->abilityStateScheduler_ = std::make_shared<SystemAbilityStateScheduler>();
}
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

#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityManagerDumperTest, CanDump001, TestSize.Level3)
{
    bool result = SystemAbilityManagerDumper::CanDump();
    EXPECT_FALSE(result);
}
#endif

/**
 * @tc.name: FfrtDumpProc001
 * @tc.desc: FfrtDumpProc
 * @tc.type: FUNC
 * @tc.require: I9I86P
 */

#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityManagerDumperTest, FfrtDumpProc001, TestSize.Level3)
{
    DTEST_LOG << "FfrtDumpProc001 begin" << std::endl;
    std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler;
    int32_t fd = -1;
    const std::vector<std::string> args;
    auto ret = SystemAbilityManagerDumper::FfrtDumpProc(abilityStateScheduler, fd, args);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
    DTEST_LOG << "FfrtDumpProc001 end" << std::endl;
}
#endif

/**
 * @tc.name: FfrtDumpProc002
 * @tc.desc: FfrtDumpProc IllegalInput
 * @tc.type: FUNC
 * @tc.require: IBMM2R
 */
#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityManagerDumperTest, FfrtDumpProc002, TestSize.Level3)
{
    DTEST_LOG << "FfrtDumpProc002 begin" << std::endl;
    SamMockPermission::MockProcess("hidumper_service");
    std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    int32_t fd = -1;
    std::vector<std::string> args;
    auto ret = SystemAbilityManagerDumper::FfrtDumpProc(abilityStateScheduler, fd, args);
    EXPECT_EQ(ret, SAVE_FD_FAIL);
    args.emplace_back("--ffrt");
    args.emplace_back("pid1");
    args.emplace_back("pid2");
    args.emplace_back("pid3");
    ret = SystemAbilityManagerDumper::FfrtDumpProc(abilityStateScheduler, fd, args);
    EXPECT_EQ(ret, SAVE_FD_FAIL);
    DTEST_LOG << "FfrtDumpProc002 end" << std::endl;
}
#endif

/**
 * @tc.name: FfrtDumpProc003
 * @tc.desc: FfrtDumpProc GetFfrtDumpInfoProc
 * @tc.type: FUNC
 * @tc.require: IBMM2R
 */
#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityManagerDumperTest, FfrtDumpProc003, TestSize.Level3)
{
    DTEST_LOG << "FfrtDumpProc003 begin" << std::endl;
    SamMockPermission::MockProcess("hidumper_service");
    std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    int32_t fd = -1;
    std::vector<std::string> args;
    args.emplace_back("--ffrt");
    args.emplace_back("9999");
    auto ret = SystemAbilityManagerDumper::FfrtDumpProc(abilityStateScheduler, fd, args);
    EXPECT_EQ(ret, SAVE_FD_FAIL);
    DTEST_LOG << "FfrtDumpProc003 end" << std::endl;
}
#endif

/**
 * @tc.name: FfrtDumpProc004
 * @tc.desc: FfrtDumpProc GetFfrtLoadMetrics
 * @tc.type: FUNC
 * @tc.require: IBMM2R
 */
#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityManagerDumperTest, FfrtDumpProc004, TestSize.Level3)
{
    DTEST_LOG << "FfrtDumpProc004 begin" << std::endl;
    SamMockPermission::MockProcess("hidumper_service");
    std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    int32_t fd = -1;
    std::vector<std::string> args;
    args.emplace_back("--ffrt");
    args.emplace_back("9999");
    args.emplace_back("--stat");
    auto ret = SystemAbilityManagerDumper::FfrtDumpProc(abilityStateScheduler, fd, args);
    EXPECT_EQ(ret, SAVE_FD_FAIL);
    DTEST_LOG << "FfrtDumpProc004 end" << std::endl;
}
#endif

/**
 * @tc.name: GetFfrtLoadMetrics001
 * @tc.desc: GetFfrtLoadMetrics processIds empty
 * @tc.type: FUNC
 * @tc.require: IBMM2R
 */
HWTEST_F(SystemAbilityManagerDumperTest, GetFfrtLoadMetrics001, TestSize.Level3)
{
    DTEST_LOG << "GetFfrtLoadMetrics001 begin" << std::endl;
    std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    int32_t fd = -1;
    std::vector<std::string> args;
    args.emplace_back("--ffrt");
    args.emplace_back("test");
    std::string result;
    SystemAbilityManagerDumper::GetFfrtLoadMetrics(abilityStateScheduler, fd, args, result);
    EXPECT_NE(result.size(), 0);
    DTEST_LOG << "GetFfrtLoadMetrics001 end" << std::endl;
}

/**
 * @tc.name: GetFfrtLoadMetrics002
 * @tc.desc: GetFfrtLoadMetrics cmd invalid
 * @tc.type: FUNC
 * @tc.require: IBMM2R
 */
HWTEST_F(SystemAbilityManagerDumperTest, GetFfrtLoadMetrics002, TestSize.Level3)
{
    DTEST_LOG << "GetFfrtLoadMetrics002 begin" << std::endl;
    std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    int32_t fd = -1;
    std::vector<std::string> args;
    args.emplace_back("--ffrt");
    args.emplace_back("9999");
    args.emplace_back("xxx");
    std::string result;
    SystemAbilityManagerDumper::GetFfrtLoadMetrics(abilityStateScheduler, fd, args, result);
    EXPECT_NE(result.size(), 0);
    DTEST_LOG << "GetFfrtLoadMetrics002 end" << std::endl;
}

/**
 * @tc.name: GetFfrtLoadMetrics003
 * @tc.desc: GetFfrtLoadMetrics cmd invalid
 * @tc.type: FUNC
 * @tc.require: IBMM2R
 */
HWTEST_F(SystemAbilityManagerDumperTest, GetFfrtLoadMetrics003, TestSize.Level3)
{
    DTEST_LOG << "GetFfrtLoadMetrics003 begin" << std::endl;
    std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    int32_t fd = -1;
    std::vector<std::string> args;
    args.emplace_back("--ffrt");
    args.emplace_back("9999");
    args.emplace_back("--stat");
    std::string result;
    SystemAbilityManagerDumper::GetFfrtLoadMetrics(abilityStateScheduler, fd, args, result);
    EXPECT_NE(result.size(), 0);
    DTEST_LOG << "GetFfrtLoadMetrics003 end" << std::endl;
}

/**
 * @tc.name: FfrtStatCmdParser001
 * @tc.desc: FfrtStatCmdParser
 * @tc.type: FUNC
 * @tc.require: IBMM2R
 */
HWTEST_F(SystemAbilityManagerDumperTest, FfrtStatCmdParser001, TestSize.Level3)
{
    DTEST_LOG << "FfrtStatCmdParser001 begin" << std::endl;
    int32_t cmd = -1;
    std::vector<std::string> args;
    args.emplace_back("--ffrt");
    args.emplace_back("9999");
    args.emplace_back("--start-stat");
    SystemAbilityManagerDumper::FfrtStatCmdParser(cmd, args);
    EXPECT_NE(cmd, -1);
    DTEST_LOG << "FfrtStatCmdParser001 end" << std::endl;
}

/**
 * @tc.name: FfrtStatCmdParser002
 * @tc.desc: FfrtStatCmdParser
 * @tc.type: FUNC
 * @tc.require: IBMM2R
 */
HWTEST_F(SystemAbilityManagerDumperTest, FfrtStatCmdParser002, TestSize.Level3)
{
    DTEST_LOG << "FfrtStatCmdParser002 begin" << std::endl;
    int32_t cmd = -1;
    std::vector<std::string> args;
    args.emplace_back("--ffrt");
    args.emplace_back("9999");
    args.emplace_back("--stop-stat");
    SystemAbilityManagerDumper::FfrtStatCmdParser(cmd, args);
    EXPECT_NE(cmd, -1);
    DTEST_LOG << "FfrtStatCmdParser002 end" << std::endl;
}

/**
 * @tc.name: FfrtStatCmdParser003
 * @tc.desc: FfrtStatCmdParser
 * @tc.type: FUNC
 * @tc.require: IBMM2R
 */
HWTEST_F(SystemAbilityManagerDumperTest, FfrtStatCmdParser003, TestSize.Level3)
{
    DTEST_LOG << "FfrtStatCmdParser003 begin" << std::endl;
    int32_t cmd = -1;
    std::vector<std::string> args;
    args.emplace_back("--ffrt");
    args.emplace_back("9999");
    args.emplace_back("--stat");
    SystemAbilityManagerDumper::FfrtStatCmdParser(cmd, args);
    EXPECT_NE(cmd, -1);
    DTEST_LOG << "FfrtStatCmdParser003 end" << std::endl;
}

/**
 * @tc.name: FfrtStatCmdParser004
 * @tc.desc: FfrtStatCmdParser cmd invalid
 * @tc.type: FUNC
 * @tc.require: IBMM2R
 */
HWTEST_F(SystemAbilityManagerDumperTest, FfrtStatCmdParser004, TestSize.Level3)
{
    DTEST_LOG << "FfrtStatCmdParser004 begin" << std::endl;
    int32_t cmd = -1;
    std::vector<std::string> args;
    args.emplace_back("--ffrt");
    args.emplace_back("9999");
    args.emplace_back("xxxx");
    SystemAbilityManagerDumper::FfrtStatCmdParser(cmd, args);
    EXPECT_EQ(cmd, -1);
    DTEST_LOG << "FfrtStatCmdParser004 end" << std::endl;
}

/**
 * @tc.name: CollectFfrtMetricInfoInProcs001
 * @tc.desc: CollectFfrtMetricInfoInProcs
 * @tc.type: FUNC
 * @tc.require: IBMM2R
 */
HWTEST_F(SystemAbilityManagerDumperTest, CollectFfrtMetricInfoInProcs001, TestSize.Level3)
{
    DTEST_LOG << "CollectFfrtMetricInfoInProcs001 begin" << std::endl;
    std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    int32_t fd = -1;
    std::vector<int32_t> processIds;
    processIds.emplace_back(getpid());
    int32_t cmd = -1;
    std::string result;
    SystemAbilityManagerDumper::CollectFfrtMetricInfoInProcs(fd, processIds, abilityStateScheduler, cmd, result);
    EXPECT_NE(result.size(), 0);
    DTEST_LOG << "CollectFfrtMetricInfoInProcs001 end" << std::endl;
}

/**
 * @tc.name: CollectFfrtStatistics001
 * @tc.desc: CollectFfrtStatistics
 * @tc.type: FUNC
 * @tc.require: IBMM2R
 */
HWTEST_F(SystemAbilityManagerDumperTest, CollectFfrtStatistics001, TestSize.Level3)
{
    DTEST_LOG << "CollectFfrtStatistics001 begin" << std::endl;
    std::string result;
    auto ret = SystemAbilityManagerDumper::CollectFfrtStatistics(FFRT_STAT_CMD_START, result);
    SystemAbilityManagerDumper::ClearFfrtStatistics();
    EXPECT_TRUE(ret);
    ret = SystemAbilityManagerDumper::CollectFfrtStatistics(FFRT_STAT_CMD_GET, result);
    EXPECT_FALSE(ret);
    ret = SystemAbilityManagerDumper::CollectFfrtStatistics(FFRT_STAT_CMD_STOP, result);
    EXPECT_FALSE(ret);
    ret = SystemAbilityManagerDumper::CollectFfrtStatistics(FFRT_STAT_CMD_START, result);
    EXPECT_TRUE(ret);
    ret = SystemAbilityManagerDumper::CollectFfrtStatistics(FFRT_STAT_CMD_START, result);
    EXPECT_FALSE(ret);
    ret = SystemAbilityManagerDumper::CollectFfrtStatistics(FFRT_STAT_CMD_GET, result);
    EXPECT_FALSE(ret);
    ret = SystemAbilityManagerDumper::CollectFfrtStatistics(FFRT_STAT_CMD_STOP, result);
    EXPECT_TRUE(ret);
    ret = SystemAbilityManagerDumper::CollectFfrtStatistics(FFRT_STAT_CMD_GET, result);
    EXPECT_TRUE(ret);
    DTEST_LOG << "CollectFfrtStatistics001 end" << std::endl;
}

/**
 * @tc.name: GetSAMgrFfrtInfo001
 * @tc.desc: GetSAMgrFfrtInfo
 * @tc.type: FUNC
 * @tc.require: I9I86P
 */

HWTEST_F(SystemAbilityManagerDumperTest, GetSAMgrFfrtInfo001, TestSize.Level3)
{
    DTEST_LOG << "GetSAMgrFfrtInfo001 begin" << std::endl;
    std::string result;
    SystemAbilityManagerDumper::GetSAMgrFfrtInfo(result);
    EXPECT_NE(result.size(), 0);
    DTEST_LOG << "GetSAMgrFfrtInfo001 end" << std::endl;
}

/**
 * @tc.name: ShowAllSystemAbilityInfo001
 * @tc.desc: ShowAllSystemAbilityInfo
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityManagerDumperTest, ShowAllSystemAbilityInfo001, TestSize.Level3)
{
    DTEST_LOG << "ShowAllSystemAbilityInfo001 begin" << std::endl;
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
    DTEST_LOG << "ShowAllSystemAbilityInfo001 end" << std::endl;
}


/**
 * @tc.name: ShowAllSystemAbilityInfo002
 * @tc.desc: ShowAllSystemAbilityInfo systemAbilityStateScheduler is nullptr
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityManagerDumperTest, ShowAllSystemAbilityInfo002, TestSize.Level3)
{
    DTEST_LOG << "ShowAllSystemAbilityInfo002 begin" << std::endl;
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler = nullptr;
    string result;
    SystemAbilityManagerDumper::ShowAllSystemAbilityInfo(systemAbilityStateScheduler, result);
    EXPECT_TRUE(result.empty());
    DTEST_LOG << "ShowAllSystemAbilityInfo002 end" << std::endl;
}

/**
 * @tc.name: ShowSystemAbilityInfo001
 * @tc.desc: call ShowSystemAbilityInfo
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityManagerDumperTest, ShowSystemAbilityInfo001, TestSize.Level3)
{
    DTEST_LOG << "ShowSystemAbilityInfo001 begin" << std::endl;
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
    DTEST_LOG << "ShowSystemAbilityInfo001 end" << std::endl;
}

/**
 * @tc.name: ShowSystemAbilityInfo002
 * @tc.desc: call ShowSystemAbilityInfo systemAbilityStateScheduler is nullptr
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityManagerDumperTest, ShowSystemAbilityInfo002, TestSize.Level3)
{
    DTEST_LOG << "ShowSystemAbilityInfo002 begin" << std::endl;
    string result;
    int32_t said = 401;
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler = nullptr;
    SystemAbilityManagerDumper::ShowSystemAbilityInfo(said, systemAbilityStateScheduler, result);
    EXPECT_TRUE(result.empty());
    DTEST_LOG << "ShowSystemAbilityInfo002 end" << std::endl;
}

/**
 * @tc.name: ShowProcessInfo001
 * @tc.desc: call ShowProcessInfo, return true;
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityManagerDumperTest, ShowProcessInfo001, TestSize.Level3)
{
    DTEST_LOG << "ShowProcessInfo001 begin" << std::endl;
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
    DTEST_LOG << "ShowProcessInfo001 end" << std::endl;
}

/**
 * @tc.name: ShowProcessInfo002
 * @tc.desc: call ShowProcessInfo, systemAbilityStateScheduler is nullptr;
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityManagerDumperTest, ShowProcessInfo002, TestSize.Level3)
{
    DTEST_LOG << "ShowProcessInfo002 begin" << std::endl;
    string result;
    string processName = "deviceprofile";
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler = nullptr;
    SystemAbilityManagerDumper::ShowProcessInfo(processName, systemAbilityStateScheduler, result);
    EXPECT_TRUE(result.empty());
    DTEST_LOG << "ShowProcessInfo002 end" << std::endl;
}

/**
 * @tc.name: ShowAllSystemAbilityInfoInState001
 * @tc.desc: call ShowAllSystemAbilityInfoInState, return true;
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityManagerDumperTest, ShowAllSystemAbilityInfoInState001, TestSize.Level3)
{
    DTEST_LOG << "ShowAllSystemAbilityInfoInState001 begin" << std::endl;
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
    DTEST_LOG << "ShowAllSystemAbilityInfoInState001 end" << std::endl;
}

/**
 * @tc.name: ShowAllSystemAbilityInfoInState002
 * @tc.desc: call ShowAllSystemAbilityInfoInState, systemAbilityStateScheduler is nullptr;
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityManagerDumperTest, ShowAllSystemAbilityInfoInState002, TestSize.Level3)
{
    DTEST_LOG << "ShowAllSystemAbilityInfoInState002 begin" << std::endl;
    string result;
    string state = "LOADED";
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler = nullptr;
    SystemAbilityManagerDumper::ShowAllSystemAbilityInfoInState(state, systemAbilityStateScheduler, result);
    EXPECT_TRUE(result.empty());
    DTEST_LOG << "ShowAllSystemAbilityInfoInState002 end" << std::endl;
}

/**
 * @tc.name: IllegalInput001
 * @tc.desc: IllegalInput
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityManagerDumperTest, IllegalInput001, TestSize.Level3)
{
    DTEST_LOG << "IllegalInput001 begin" << std::endl;
    string result;
    SystemAbilityManagerDumper::IllegalInput(result);
    EXPECT_NE(result.size(), 0);
    DTEST_LOG << "IllegalInput001 end" << std::endl;
}

/**
 * @tc.name: ShowHelp001
 * @tc.desc: ShowHelp
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityManagerDumperTest, ShowHelp001, TestSize.Level3)
{
    DTEST_LOG << "ShowHelp001 begin" << std::endl;
    string result;
    SystemAbilityManagerDumper::ShowHelp(result);
    EXPECT_NE(result.size(), 0);
    DTEST_LOG << "ShowHelp001 end" << std::endl;
}

/**
 * @tc.name: CanDump002
 * @tc.desc: call CanDump, nativeTokenInfo.processName is HIDUMPER_PROCESS_NAME
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityManagerDumperTest, CanDump002, TestSize.Level3)
{
    DTEST_LOG << "CanDump002 begin" << std::endl;
    SamMockPermission::MockProcess("hidumper_service");
    bool result = SystemAbilityManagerDumper::CanDump();
    EXPECT_TRUE(result);
    DTEST_LOG << "CanDump002 end" << std::endl;
}

/**
 * @tc.name: Dump001
 * @tc.desc: call Dump, ShowAllSystemAbilityInfo
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityManagerDumperTest, Dump001, TestSize.Level3)
{
    DTEST_LOG << "Dump001 begin" << std::endl;
    SamMockPermission::MockProcess("hidumper_service");
    std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler = nullptr;
    std::vector<std::string> args;
    args.push_back("-l");
    std::string result;
    bool ret = SystemAbilityManagerDumper::Dump(abilityStateScheduler, args, result);
    EXPECT_TRUE(ret);
    DTEST_LOG << "Dump001 end" << std::endl;
}

/**
 * @tc.name: Dump002
 * @tc.desc: call Dump, ShowHelp
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityManagerDumperTest, Dump002, TestSize.Level3)
{
    DTEST_LOG << "Dump002 begin" << std::endl;
    SamMockPermission::MockProcess("hidumper_service");
    std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler = nullptr;
    std::vector<std::string> args;
    args.push_back("-h");
    std::string result;
    bool ret = SystemAbilityManagerDumper::Dump(abilityStateScheduler, args, result);
    EXPECT_TRUE(ret);
    DTEST_LOG << "Dump002 end" << std::endl;
}

/**
 * @tc.name: Dump003
 * @tc.desc: call Dump, ShowSystemAbilityInfo
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityManagerDumperTest, Dump003, TestSize.Level3)
{
    DTEST_LOG << "Dump003 begin" << std::endl;
    SamMockPermission::MockProcess("hidumper_service");
    std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler = nullptr;
    std::vector<std::string> args;
    string said;
    args.push_back("-sa");
    args.push_back(said);
    std::string result;
    bool ret = SystemAbilityManagerDumper::Dump(abilityStateScheduler, args, result);
    EXPECT_TRUE(ret);
    DTEST_LOG << "Dump003 end" << std::endl;
}

/**
 * @tc.name: Dump004
 * @tc.desc: call Dump, ShowProcessInfo
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityManagerDumperTest, Dump004, TestSize.Level3)
{
    DTEST_LOG << "Dump004 begin" << std::endl;
    SamMockPermission::MockProcess("hidumper_service");
    std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler = nullptr;
    std::vector<std::string> args;
    std::string processName;
    args.push_back("-p");
    args.push_back(processName);
    std::string result;
    bool ret = SystemAbilityManagerDumper::Dump(abilityStateScheduler, args, result);
    EXPECT_TRUE(ret);
    DTEST_LOG << "Dump004 end" << std::endl;
}

/**
 * @tc.name: Dump005
 * @tc.desc: call Dump, ShowAllSystemAbilityInfoInState
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityManagerDumperTest, Dump005, TestSize.Level3)
{
    DTEST_LOG << "Dump005 begin" << std::endl;
    SamMockPermission::MockProcess("hidumper_service");
    std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler = nullptr;
    std::vector<std::string> args;
    std::string state;
    args.push_back("-sm");
    args.push_back(state);
    std::string result;
    bool ret = SystemAbilityManagerDumper::Dump(abilityStateScheduler, args, result);
    EXPECT_TRUE(ret);
    DTEST_LOG << "Dump005 end" << std::endl;
}

/**
 * @tc.name: Dump006
 * @tc.desc: call Dump, false
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityManagerDumperTest, Dump006, TestSize.Level3)
{
    DTEST_LOG << "Dump006 begin" << std::endl;
    SamMockPermission::MockProcess("hidumper_service");
    std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler = nullptr;
    std::vector<std::string> args;
    std::string result;
    bool ret = SystemAbilityManagerDumper::Dump(abilityStateScheduler, args, result);
    EXPECT_FALSE(ret);
    DTEST_LOG << "Dump006 end" << std::endl;
}

/**
 * @tc.name: Dump007
 * @tc.desc: call Dump, ShowAllSystemAbilityInfo
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityManagerDumperTest, Dump007, TestSize.Level3)
{
    DTEST_LOG << "Dump007 begin" << std::endl;
    SamMockPermission::MockProcess("hidumper_service");
    std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler = nullptr;
    std::vector<std::string> args;
    args.push_back("-sm");
    std::string result;
    bool ret = SystemAbilityManagerDumper::Dump(abilityStateScheduler, args, result);
    EXPECT_FALSE(ret);
    DTEST_LOG << "Dump007 end" << std::endl;
}

/**
 * @tc.name: Dump008
 * @tc.desc: call Dump, ShowAllSystemAbilityInfoInState
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SystemAbilityManagerDumperTest, Dump008, TestSize.Level3)
{
    DTEST_LOG << "Dump008 begin" << std::endl;
    SamMockPermission::MockProcess("hidumper_service");
    std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler = nullptr;
    std::vector<std::string> args;
    std::string state;
    args.push_back("-h");
    args.push_back(state);
    std::string result;
    bool ret = SystemAbilityManagerDumper::Dump(abilityStateScheduler, args, result);
    EXPECT_FALSE(ret);
    DTEST_LOG << "Dump008 end" << std::endl;
}

/**
 * @tc.name: IpcDumpCmdParser001
 * @tc.desc: ipc dump cmd parser, IpcDumpCmdParser
 * @tc.type: FUNC
 * @tc.require: I9DR69
 */

HWTEST_F(SystemAbilityManagerDumperTest, IpcDumpCmdParser001, TestSize.Level3)
{
    DTEST_LOG << "IpcDumpCmdParser001 begin" << std::endl;
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
    DTEST_LOG << "IpcDumpCmdParser001 end" << std::endl;
}

/**
 * @tc.name: IpcDumpCmdParser002
 * @tc.desc: ipc dump cmd parser, IpcDumpCmdParser
 * @tc.type: FUNC
 * @tc.require: I9DR69
 */

HWTEST_F(SystemAbilityManagerDumperTest, IpcDumpCmdParser002, TestSize.Level3)
{
    DTEST_LOG << "IpcDumpCmdParser002 begin" << std::endl;
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
    DTEST_LOG << "IpcDumpCmdParser002 end" << std::endl;
}

/**
 * @tc.name: IpcDumpCmdParser003
 * @tc.desc: ipc dump cmd parser, IpcDumpCmdParser
 * @tc.type: FUNC
 * @tc.require: I9DR69
 */

HWTEST_F(SystemAbilityManagerDumperTest, IpcDumpCmdParser003, TestSize.Level3)
{
    DTEST_LOG << "IpcDumpCmdParser003 begin" << std::endl;
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
    DTEST_LOG << "IpcDumpCmdParser003 end" << std::endl;
}

/**
 * @tc.name: IpcDumpCmdParser004
 * @tc.desc: ipc dump cmd parser, IpcDumpCmdParser
 * @tc.type: FUNC
 * @tc.require: I9DR69
 */

HWTEST_F(SystemAbilityManagerDumperTest, IpcDumpCmdParser004, TestSize.Level3)
{
    DTEST_LOG << "IpcDumpCmdParser004 begin" << std::endl;
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
    DTEST_LOG << "IpcDumpCmdParser004 end" << std::endl;
}

/**
 * @tc.name: IpcDumpCmdParser005
 * @tc.desc: ipc dump cmd parser, IpcDumpCmdParser
 * @tc.type: FUNC
 * @tc.require: I9DR69
 */

HWTEST_F(SystemAbilityManagerDumperTest, IpcDumpCmdParser005, TestSize.Level3)
{
    DTEST_LOG << "IpcDumpCmdParser005 begin" << std::endl;
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
    DTEST_LOG << "IpcDumpCmdParser005 end" << std::endl;
}

/**
 * @tc.name: IpcDumpCmdParser006
 * @tc.desc: ipc dump cmd parser, IpcDumpCmdParser
 * @tc.type: FUNC
 * @tc.require: I9DR69
 */

HWTEST_F(SystemAbilityManagerDumperTest, IpcDumpCmdParser006, TestSize.Level3)
{
    DTEST_LOG << "IpcDumpCmdParser006 begin" << std::endl;
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
    DTEST_LOG << "IpcDumpCmdParser006 end" << std::endl;
}

/**
 * @tc.name: IpcDumpCmdParser007
 * @tc.desc: ipc dump cmd parser, IpcDumpCmdParser
 * @tc.type: FUNC
 * @tc.require: I9DR69
 */

#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityManagerDumperTest, IpcDumpCmdParser007, TestSize.Level3)
{
    DTEST_LOG << "IpcDumpCmdParser007 begin" << std::endl;
    SamMockPermission::MockProcess("demo_service");
    int32_t cmd = 0;
    std::vector<std::string> args;
    args.push_back("--ipc");
    args.push_back("wifi_manager_service");
    args.push_back("--start-stat");
    std::string result;
    bool ret = SystemAbilityManagerDumper::IpcDumpCmdParser(cmd, args);
    EXPECT_FALSE(ret);
    DTEST_LOG << "IpcDumpCmdParser007 end" << std::endl;
}
#endif

/**
 * @tc.name: IpcDumpCmdParser008
 * @tc.desc: ipc dump cmd parser, IpcDumpCmdParser
 * @tc.type: FUNC
 * @tc.require: I9DR69
 */

#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityManagerDumperTest, IpcDumpCmdParser008, TestSize.Level3)
{
    DTEST_LOG << "IpcDumpCmdParser008 begin" << std::endl;
    SamMockPermission::MockProcess("demo_service");
    int32_t cmd = 0;
    std::vector<std::string> args;
    args.push_back("--ipc");
    args.push_back("wifi_manager_service");
    args.push_back("--stop-stat");
    std::string result;
    bool ret = SystemAbilityManagerDumper::IpcDumpCmdParser(cmd, args);
    EXPECT_FALSE(ret);
    DTEST_LOG << "IpcDumpCmdParser008 end" << std::endl;
}
#endif

/**
 * @tc.name: IpcDumpCmdParser009
 * @tc.desc: ipc dump cmd parser, IpcDumpCmdParser
 * @tc.type: FUNC
 * @tc.require: I9DR69
 */

#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityManagerDumperTest, IpcDumpCmdParser009, TestSize.Level3)
{
    DTEST_LOG << "IpcDumpCmdParser009 begin" << std::endl;
    SamMockPermission::MockProcess("demo_service");
    int32_t cmd = 0;
    std::vector<std::string> args;
    args.push_back("--ipc");
    args.push_back("wifi_manager_service");
    args.push_back("--stat");
    std::string result;
    bool ret = SystemAbilityManagerDumper::IpcDumpCmdParser(cmd, args);
    EXPECT_FALSE(ret);
    DTEST_LOG << "IpcDumpCmdParser009 end" << std::endl;
}
#endif

/**
 * @tc.name: IpcDumpCmdParser010
 * @tc.desc: ipc dump cmd parser, IpcDumpCmdParser
 * @tc.type: FUNC
 * @tc.require: I9DR69
 */

HWTEST_F(SystemAbilityManagerDumperTest, IpcDumpCmdParser010, TestSize.Level3)
{
    DTEST_LOG << "IpcDumpCmdParser010 begin" << std::endl;
    SamMockPermission::MockProcess("demo_service");
    int32_t cmd = 0;
    std::vector<std::string> args;
    args.push_back("--ipc");
    std::string result;
    bool ret = SystemAbilityManagerDumper::IpcDumpCmdParser(cmd, args);
    EXPECT_FALSE(ret);
    DTEST_LOG << "IpcDumpCmdParser010 end" << std::endl;
}

/**
 * @tc.name: IpcDumpCmdParser011
 * @tc.desc: ipc dump cmd parser, IpcDumpCmdParser
 * @tc.type: FUNC
 * @tc.require: I9DR69
 */

HWTEST_F(SystemAbilityManagerDumperTest, IpcDumpCmdParser011, TestSize.Level3)
{
    DTEST_LOG << "IpcDumpCmdParser011 begin" << std::endl;
    SamMockPermission::MockProcess("demo_service");
    int32_t cmd = 0;
    std::vector<std::string> args;
    args.push_back("--ipc");
    args.push_back("wifi_manager_service");
    std::string result;
    bool ret = SystemAbilityManagerDumper::IpcDumpCmdParser(cmd, args);
    EXPECT_FALSE(ret);
    DTEST_LOG << "IpcDumpCmdParser011 end" << std::endl;
}

/**
 * @tc.name: IpcDumpCmdParser012
 * @tc.desc: ipc dump cmd parser, IpcDumpCmdParser
 * @tc.type: FUNC
 * @tc.require: I9DR69
 */

HWTEST_F(SystemAbilityManagerDumperTest, IpcDumpCmdParser012, TestSize.Level3)
{
    DTEST_LOG << "IpcDumpCmdParser012 begin" << std::endl;
    SamMockPermission::MockProcess("demo_service");
    int32_t cmd = 0;
    std::vector<std::string> args;
    args.push_back("--ipc");
    args.push_back("wifi_manager_service");
    args.push_back("test001");
    std::string result;
    bool ret = SystemAbilityManagerDumper::IpcDumpCmdParser(cmd, args);
    EXPECT_FALSE(ret);
    DTEST_LOG << "IpcDumpCmdParser012 end" << std::endl;
}

/**
 * @tc.name: IpcDumpCmdParser013
 * @tc.desc: ipc dump cmd parser, IpcDumpCmdParser
 * @tc.type: FUNC
 * @tc.require: I9DR69
 */

HWTEST_F(SystemAbilityManagerDumperTest, IpcDumpCmdParser013, TestSize.Level3)
{
    DTEST_LOG << "IpcDumpCmdParser013 begin" << std::endl;
    SamMockPermission::MockProcess("demo_service");
    int32_t cmd = 0;
    std::vector<std::string> args;
    args.push_back("--ipc");
    args.push_back("testProcess");
    args.push_back("test001");
    std::string result;
    bool ret = SystemAbilityManagerDumper::IpcDumpCmdParser(cmd, args);
    EXPECT_FALSE(ret);
    DTEST_LOG << "IpcDumpCmdParser013 end" << std::endl;
}

/**
 * @tc.name: IpcDumpCmdParser014
 * @tc.desc: ipc dump cmd parser, IpcDumpCmdParser
 * @tc.type: FUNC
 * @tc.require: I9DR69
 */

HWTEST_F(SystemAbilityManagerDumperTest, IpcDumpCmdParser014, TestSize.Level3)
{
    DTEST_LOG << "IpcDumpCmdParser014 begin" << std::endl;
    SamMockPermission::MockProcess("hidumper_service");
    int32_t cmd = 0;
    std::vector<std::string> args;
    args.push_back("--ipc");
    args.push_back("testProcess");
    args.push_back("test001");
    std::string result;
    bool ret = SystemAbilityManagerDumper::IpcDumpCmdParser(cmd, args);
    EXPECT_FALSE(ret);
    DTEST_LOG << "IpcDumpCmdParser014 end" << std::endl;
}

/**
 * @tc.name: IpcDumpIsAllProcess001
 * @tc.desc: is ipc dump all process cmd, IpcDumpIsAllProcess
 * @tc.type: FUNC
 * @tc.require: I9DR69
 */

HWTEST_F(SystemAbilityManagerDumperTest, IpcDumpIsAllProcess001, TestSize.Level3)
{
    DTEST_LOG << "IpcDumpIsAllProcess001 begin" << std::endl;
    std::string processName = "all";
    bool ret = SystemAbilityManagerDumper::IpcDumpIsAllProcess(processName);
    EXPECT_TRUE(ret);
    DTEST_LOG << "IpcDumpIsAllProcess001 end" << std::endl;
}

/**
 * @tc.name: IpcDumpIsAllProcess002
 * @tc.desc: is ipc dump all process cmd, IpcDumpIsAllProcess
 * @tc.type: FUNC
 * @tc.require: I9DR69
 */

HWTEST_F(SystemAbilityManagerDumperTest, IpcDumpIsAllProcess002, TestSize.Level3)
{
    DTEST_LOG << "IpcDumpIsAllProcess002 begin" << std::endl;
    std::string processName = "test001";
    bool ret = SystemAbilityManagerDumper::IpcDumpIsAllProcess(processName);
    EXPECT_FALSE(ret);
    DTEST_LOG << "IpcDumpIsAllProcess002 end" << std::endl;
}

/**
 * @tc.name: IpcDumpIsAllProcess003
 * @tc.desc: is ipc dump all process cmd, IpcDumpIsAllProcess
 * @tc.type: FUNC
 * @tc.require: I9DR69
 */

HWTEST_F(SystemAbilityManagerDumperTest, IpcDumpIsAllProcess003, TestSize.Level3)
{
    DTEST_LOG << "IpcDumpIsAllProcess003 begin" << std::endl;
    std::string processName = "";
    bool ret = SystemAbilityManagerDumper::IpcDumpIsAllProcess(processName);
    EXPECT_FALSE(ret);
    DTEST_LOG << "IpcDumpIsAllProcess003 end" << std::endl;
}

/**
 * @tc.name: IpcDumpIsSamgr001
 * @tc.desc: is ipc dump samgr process cmd, IpcDumpIsSamgr001
 * @tc.type: FUNC
 * @tc.require: I9DR69
 */

HWTEST_F(SystemAbilityManagerDumperTest, IpcDumpIsSamgr001, TestSize.Level3)
{
    DTEST_LOG << "IpcDumpIsSamgr001 begin" << std::endl;
    std::string processName = "samgr";
    bool ret = SystemAbilityManagerDumper::IpcDumpIsSamgr(processName);
    EXPECT_TRUE(ret);
    DTEST_LOG << "IpcDumpIsSamgr001 end" << std::endl;
}

/**
 * @tc.name: IpcDumpIsSamgr002
 * @tc.desc: is ipc dump samgr process cmd, IpcDumpIsSamgr002
 * @tc.type: FUNC
 * @tc.require: I9DR69
 */

HWTEST_F(SystemAbilityManagerDumperTest, IpcDumpIsSamgr002, TestSize.Level3)
{
    DTEST_LOG << "IpcDumpIsSamgr002 begin" << std::endl;
    std::string processName = "test001";
    bool ret = SystemAbilityManagerDumper::IpcDumpIsSamgr(processName);
    EXPECT_FALSE(ret);
    DTEST_LOG << "IpcDumpIsSamgr002 end" << std::endl;
}

/**
 * @tc.name: StartSamgrIpcStatistics001
 * @tc.desc: test StartSamgrIpcStatistics
 * @tc.type: FUNC
 * @tc.require: I6W28
 */
HWTEST_F(SystemAbilityManagerDumperTest, StartSamgrIpcStatistics001, TestSize.Level2)
{
    DTEST_LOG << "StartSamgrIpcStatistics001 begin" << std::endl;
    std::string result;
    bool ret = SystemAbilityManagerDumper::StartSamgrIpcStatistics(result);
    EXPECT_EQ(ret, true);
    DTEST_LOG << "StartSamgrIpcStatistics001 end" << std::endl;
}

/**
 * @tc.name: StopSamgrIpcStatistics001
 * @tc.desc: test StopSamgrIpcStatistics
 * @tc.type: FUNC
 * @tc.require: I6W28
 */
HWTEST_F(SystemAbilityManagerDumperTest, StopSamgrIpcStatistics001, TestSize.Level2)
{
    DTEST_LOG << "StopSamgrIpcStatistics001 begin" << std::endl;
    std::string result;
    bool ret = SystemAbilityManagerDumper::StopSamgrIpcStatistics(result);
    EXPECT_EQ(ret, true);
    DTEST_LOG << "StopSamgrIpcStatistics001 end" << std::endl;
}

/**
 * @tc.name: GetSamgrIpcStatistics001
 * @tc.desc: test GetSamgrIpcStatistics
 * @tc.type: FUNC
 * @tc.require: I6W28
 */
HWTEST_F(SystemAbilityManagerDumperTest, GetSamgrIpcStatistics001, TestSize.Level2)
{
    DTEST_LOG << "GetSamgrIpcStatistics001 begin" << std::endl;
    std::string result;
    bool ret = SystemAbilityManagerDumper::GetSamgrIpcStatistics(result);
    EXPECT_EQ(ret, true);
    DTEST_LOG << "GetSamgrIpcStatistics001 end" << std::endl;
}

/**
 * @tc.name: FfrtDumpParser001
 * @tc.desc: test FfrtDumpParser
 * @tc.type: FUNC
 * @tc.require: I6W28
 */
HWTEST_F(SystemAbilityManagerDumperTest, FfrtDumpParser001, TestSize.Level2)
{
    DTEST_LOG << "FfrtDumpParser001 begin" << std::endl;
    SamMockPermission::MockProcess("hidumper_service");
    std::vector<int32_t> processIds;
    std::string pidStr = "123";
    bool ret = SystemAbilityManagerDumper::FfrtDumpParser(processIds, pidStr);
    EXPECT_EQ(ret, true);
    DTEST_LOG << "FfrtDumpParser001 end" << std::endl;
}

/**
 * @tc.name: FfrtDumpParser002
 * @tc.desc: test FfrtDumpParser
 * @tc.type: FUNC
 * @tc.require: I6W28
 */
HWTEST_F(SystemAbilityManagerDumperTest, FfrtDumpParser002, TestSize.Level2)
{
    DTEST_LOG << "FfrtDumpParser002 begin" << std::endl;
    SamMockPermission::MockProcess("hidumper_service");
    std::vector<int32_t> processIds;
    std::string pidStr = "123|234";
    bool ret = SystemAbilityManagerDumper::FfrtDumpParser(processIds, pidStr);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(processIds.size(), 2);
    DTEST_LOG << "FfrtDumpParser002 end" << std::endl;
}

/**
 * @tc.name: FfrtDumpParser003
 * @tc.desc: test FfrtDumpParser
 * @tc.type: FUNC
 * @tc.require: I6W28
 */
HWTEST_F(SystemAbilityManagerDumperTest, FfrtDumpParser003, TestSize.Level2)
{
    DTEST_LOG << "FfrtDumpParser003 begin" << std::endl;
    SamMockPermission::MockProcess("hidumper_service");
    std::vector<int32_t> processIds;
    std::string pidStr = "12k|234";
    bool ret = SystemAbilityManagerDumper::FfrtDumpParser(processIds, pidStr);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(processIds.size(), 1);
    DTEST_LOG << "FfrtDumpParser003 end" << std::endl;
}

/**
 * @tc.name: FfrtDumpParser004
 * @tc.desc: test FfrtDumpParser
 * @tc.type: FUNC
 * @tc.require: I6W28
 */
HWTEST_F(SystemAbilityManagerDumperTest, FfrtDumpParser004, TestSize.Level2)
{
    DTEST_LOG << "FfrtDumpParser004 begin" << std::endl;
    SamMockPermission::MockProcess("hidumper_service");
    std::vector<int32_t> processIds;
    std::string pidStr = "12k";
    bool ret = SystemAbilityManagerDumper::FfrtDumpParser(processIds, pidStr);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(processIds.size(), 0);
    DTEST_LOG << "FfrtDumpParser004 end" << std::endl;
}

/**
 * @tc.name: GetFfrtDumpInfoProc001
 * @tc.desc: test GetFfrtDumpInfoProc
 * @tc.type: FUNC
 * @tc.require: I6W28
 */
HWTEST_F(SystemAbilityManagerDumperTest, GetFfrtDumpInfoProc001, TestSize.Level2)
{
    DTEST_LOG << "GetFfrtDumpInfoProc001 begin" << std::endl;
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::vector<std::string> args;
    args.emplace_back("--ffrt");
    std::string result;
    bool ret = SystemAbilityManagerDumper::GetFfrtDumpInfoProc(systemAbilityStateScheduler, args, result);
    EXPECT_EQ(ret, false);
    DTEST_LOG << "GetFfrtDumpInfoProc001 end" << std::endl;
}

/**
 * @tc.name: GetFfrtDumpInfoProc002
 * @tc.desc: test GetFfrtDumpInfoProc
 * @tc.type: FUNC
 * @tc.require: I6W28
 */
HWTEST_F(SystemAbilityManagerDumperTest, GetFfrtDumpInfoProc002, TestSize.Level2)
{
    DTEST_LOG << "GetFfrtDumpInfoProc002 begin" << std::endl;
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::vector<std::string> args;
    args.emplace_back("--ffrt");
    args.emplace_back("");
    std::string result;
    bool ret = SystemAbilityManagerDumper::GetFfrtDumpInfoProc(systemAbilityStateScheduler, args, result);
    EXPECT_EQ(ret, false);
    DTEST_LOG << "GetFfrtDumpInfoProc002 end" << std::endl;
}

/**
 * @tc.name: GetFfrtDumpInfoProc003
 * @tc.desc: test GetFfrtDumpInfoProc
 * @tc.type: FUNC
 * @tc.require: I6W28
 */
HWTEST_F(SystemAbilityManagerDumperTest, GetFfrtDumpInfoProc003, TestSize.Level2)
{
    DTEST_LOG << "GetFfrtDumpInfoProc003 begin" << std::endl;
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    std::vector<std::string> args;
    args.emplace_back("--ffrt");
    args.emplace_back("12k");
    std::string result;
    bool ret = SystemAbilityManagerDumper::GetFfrtDumpInfoProc(systemAbilityStateScheduler, args, result);
    EXPECT_EQ(ret, false);
    DTEST_LOG << "GetFfrtDumpInfoProc003 end" << std::endl;
}

/**
 * @tc.name: GetFfrtDumpInfoProc004
 * @tc.desc: test GetFfrtDumpInfoProc
 * @tc.type: FUNC
 * @tc.require: I6W28
 */
HWTEST_F(SystemAbilityManagerDumperTest, GetFfrtDumpInfoProc004, TestSize.Level2)
{
    DTEST_LOG << "GetFfrtDumpInfoProc004 begin" << std::endl;
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
    saMgr->abilityStateScheduler_ = systemAbilityStateScheduler;
    std::vector<std::string> args;
    args.emplace_back("--ffrt");
    args.emplace_back("1234|12345");
    std::string result;
    bool ret = SystemAbilityManagerDumper::GetFfrtDumpInfoProc(systemAbilityStateScheduler, args, result);
    EXPECT_EQ(ret, true);
    DTEST_LOG << "GetFfrtDumpInfoProc004 end" << std::endl;
}

/**
 * @tc.name: GetFfrtDumpInfoProc005
 * @tc.desc: test GetFfrtDumpInfoProc
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityManagerDumperTest, GetFfrtDumpInfoProc005, TestSize.Level1)
{
    DTEST_LOG << "GetFfrtDumpInfoProc005 begin" << std::endl;
    std::shared_ptr<SystemAbilityStateScheduler> systemAbilityStateScheduler =
        std::make_shared<SystemAbilityStateScheduler>();
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
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
    DTEST_LOG << "GetFfrtDumpInfoProc005 end" << std::endl;
}

/**
 * @tc.name: SaveDumpResultToFd001
 * @tc.desc: test SaveDumpResultToFd
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityManagerDumperTest, SaveDumpResultToFd001, TestSize.Level1)
{
    DTEST_LOG << "SaveDumpResultToFd001 begin" << std::endl;
    int32_t fd = -1;
    std::string result = "";
    int32_t ret = SystemAbilityManagerDumper::SaveDumpResultToFd(fd, result);
    EXPECT_EQ(ret, SAVE_FD_FAIL);
    DTEST_LOG << "SaveDumpResultToFd001 end" << std::endl;
}

/**
 * @tc.name: SaveDumpResultToFd002
 * @tc.desc: test SaveDumpResultToFd
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityManagerDumperTest, SaveDumpResultToFd002, TestSize.Level1)
{
    DTEST_LOG << "SaveDumpResultToFd002 begin" << std::endl;
    int32_t fd = 1;
    std::string result = "";
    int32_t ret = SystemAbilityManagerDumper::SaveDumpResultToFd(fd, result);
    EXPECT_EQ(ret, ERR_OK);
    DTEST_LOG << "SaveDumpResultToFd002 end" << std::endl;
}

HWTEST_F(SystemAbilityManagerDumperTest, ShowListenerHelp001, TestSize.Level1)
{
    DTEST_LOG<<"ShowListenerHelp001 BEGIN"<<std::endl;
    std::string result;
    SystemAbilityManagerDumper::ShowListenerHelp(result);
    EXPECT_FALSE(result.empty());
    
    int32_t fd = 1;
    std::vector<std::string> args;
    std::map<int32_t, std::list<SAListener>> listeners;
    int32_t ret = SystemAbilityManagerDumper::ListenerDumpProc(listeners, fd, args);
    EXPECT_EQ(ERR_OK, ret);

    SamMockPermission::MockProcess("hidumper_service");
    ret = SystemAbilityManagerDumper::ListenerDumpProc(listeners, fd, args);
    EXPECT_EQ(ERR_OK, ret);
    DTEST_LOG<<"ShowListenerHelp001 END"<<std::endl;
}

HWTEST_F(SystemAbilityManagerDumperTest, GetListenerDumpProc001, TestSize.Level1)
{
    DTEST_LOG<<"GetListenerDumpProc001 BEGIN"<<std::endl;
    std::string result;
    std::vector<std::string> args;
    args.push_back(strHidumperSerName);
    std::map<int32_t, std::list<SAListener>> listeners;
    sptr<ISystemAbilityStatusChange> lster = new SystemAbilityStatusChangeProxy(nullptr);
    auto cPid = IPCSkeleton::GetCallingPid();
    SAListener saLst(lster, cPid, ListenerState::INIT);
    listeners[1].push_back(saLst);
    SystemAbilityManagerDumper::GetListenerDumpProc(listeners, args, result);
    EXPECT_EQ(strIllegal, result);
    result.clear();
    args.push_back(strArgsHelp);
    SystemAbilityManagerDumper::GetListenerDumpProc(listeners, args, result);
    EXPECT_FALSE(result.empty());
    result.clear();
    DTEST_LOG<<"GetListenerDumpProc001 END"<<std::endl;
}

HWTEST_F(SystemAbilityManagerDumperTest, GetListenerDumpProc002, TestSize.Level1)
{
    DTEST_LOG<<"GetListenerDumpProc002 BEGIN"<<std::endl;
    std::string result;
    std::vector<std::string> args;
    args.push_back(strHidumperSerName);
    std::map<int32_t, std::list<SAListener>> listeners;
    sptr<ISystemAbilityStatusChange> lster = new SystemAbilityStatusChangeProxy(nullptr);
    pid_t cPid = IPCSkeleton::GetCallingPid();
    SAListener saLst(lster, cPid, ListenerState::INIT);
    listeners[100].push_back(saLst);
    args.push_back(strArgsQueryAll);
    args.push_back(strArgsQuerySA);
    SystemAbilityManagerDumper::GetListenerDumpProc(listeners, args, result);
    EXPECT_FALSE(result.empty());
    result.clear();
    args[LISTENER_BASE_INDEX + 1] = strArgsQueryProcess;
    SystemAbilityManagerDumper::GetListenerDumpProc(listeners, args, result);
    EXPECT_FALSE(result.empty());
    result.clear();
    args[LISTENER_BASE_INDEX] = strArgsQuerySA;
    args[LISTENER_BASE_INDEX + 1] = "100";
    SystemAbilityManagerDumper::GetListenerDumpProc(listeners, args, result);
    EXPECT_FALSE(result.empty());
    result.clear();
    args[LISTENER_BASE_INDEX] = strArgsQueryProcess;
    args[LISTENER_BASE_INDEX + 1] = std::to_string(static_cast<int>(cPid));
    SystemAbilityManagerDumper::GetListenerDumpProc(listeners, args, result);
    EXPECT_FALSE(result.empty());
    result.clear();
    DTEST_LOG<<"GetListenerDumpProc002 END"<<std::endl;
}
}