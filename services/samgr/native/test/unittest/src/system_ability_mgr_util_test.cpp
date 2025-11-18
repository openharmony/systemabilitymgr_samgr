/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "system_ability_mgr_util_test.h"
#include "system_ability_manager.h"
#include "system_ability_manager_util.h"
#include "sam_mock_permission.h"
#include "ability_death_recipient.h"
#include "test_log.h"
#include <fstream>

using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS;

namespace OHOS {
// Mock the system::GetParameter function
namespace system {
    std::string mockValue;
    std::string GetParameter(const std::string& key, const std::string& defaultValue)
    {
        return mockValue;
    }
}
const std::u16string PROCESS_NAME = u"test_process_name";
constexpr const char* PENG_LAI = "penglai";

struct CfgFiles {
    const char* paths[MAX_CFG_POLICY_DIRS_CNT];
};

CfgFiles* g_mockCfgFiles = nullptr;
std::vector<std::string> mockDirFiles;

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

void GetDirFiles(const char* path, std::vector<std::string>& files)
{
    files = mockDirFiles;
}

CfgFiles* GetCfgFiles(const char* path)
{
    return g_mockCfgFiles;
}

void SamgrUtilTest::SetUpTestCase()
{
    DTEST_LOG << "SetUpTestCase" << std::endl;
}

void SamgrUtilTest::TearDownTestCase()
{
    DTEST_LOG << "TearDownTestCase" << std::endl;
}

void SamgrUtilTest::SetUp()
{
    SamMockPermission::MockPermission();
    system::mockValue = "";
    mockDirFiles.clear();
    g_mockCfgFiles = nullptr;
    DTEST_LOG << "SetUp" << std::endl;
}

void SamgrUtilTest::TearDown()
{
    DTEST_LOG << "TearDown" << std::endl;
}

/**
 * @tc.name: IsNameInValid001
 * @tc.desc: test IsNameInValid, name is empty
 * @tc.type: FUNC
 * @tc.require: I6MO6A
 */
HWTEST_F(SamgrUtilTest, IsNameInValid001, TestSize.Level3)
{
    DTEST_LOG << " IsNameInValid001 " << std::endl;
    std::u16string name;
    bool ret = SamgrUtil::IsNameInValid(name);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: IsNameInValid002
 * @tc.desc: test IsNameInValid, DeleteBlank is empty
 * @tc.type: FUNC
 * @tc.require: I6MO6A
 */
HWTEST_F(SamgrUtilTest, IsNameInValid002, TestSize.Level3)
{
    DTEST_LOG << " IsNameInValid002 " << std::endl;
    std::u16string name = u"/t";
    bool ret = SamgrUtil::IsNameInValid(name);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: IsNameInValid003
 * @tc.desc: test IsNameInValid, name is not empty
 * @tc.type: FUNC
 * @tc.require: I6MO6A
 */
HWTEST_F(SamgrUtilTest, IsNameInValid003, TestSize.Level3)
{
    DTEST_LOG << " IsNameInValid003 " << std::endl;
    std::u16string name = u"test";
    bool ret = SamgrUtil::IsNameInValid(name);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: IsNameInValid004
 * @tc.desc: test IsNameInValid, DeleteBlank is not empty
 * @tc.type: FUNC
 * @tc.require: I6MO6A
 */
HWTEST_F(SamgrUtilTest, IsNameInValid004, TestSize.Level3)
{
    DTEST_LOG << " IsNameInValid004 " << std::endl;
    std::u16string name = u"name";
    bool ret = SamgrUtil::IsNameInValid(name);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: CheckDistributedPermission001
 * @tc.desc: test CheckDistributedPermission! return true!
 * @tc.type: FUNC
 */
HWTEST_F(SamgrUtilTest, CheckDistributedPermission001, TestSize.Level1)
{
    DTEST_LOG << " CheckDistributedPermission001 " << std::endl;
    bool res = SamgrUtil::CheckDistributedPermission();
    EXPECT_EQ(res, true);
}

/**
 * @tc.name: EventToStr001
 * @tc.desc: test EventToStr with event is initialized
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SamgrUtilTest, EventToStr001, TestSize.Level3)
{
    OnDemandEvent onDemandEvent;
    onDemandEvent.eventId = 1234;
    onDemandEvent.name = "name";
    onDemandEvent.value = "value";
    string ret = SamgrUtil::EventToStr(onDemandEvent);
    EXPECT_FALSE(ret.empty());
}

/**
 * @tc.name: TransformDeviceId001
 * @tc.desc: TransformDeviceId, isPrivate false
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SamgrUtilTest, TransformDeviceId001, TestSize.Level3)
{
    string result = SamgrUtil::TransformDeviceId("123", 1, false);
    EXPECT_EQ(result, "123");
}

/**
 * @tc.name: TransformDeviceId002
 * @tc.desc: TransformDeviceId, isPrivate true
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */

HWTEST_F(SamgrUtilTest, TransformDeviceId002, TestSize.Level3)
{
    string result = SamgrUtil::TransformDeviceId("123", 1, true);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name: CheckCallerProcess001
 * @tc.desc: test CheckCallerProcess with process is null
 * @tc.type: FUNC
 * @tc.require: I6V4AX
 */
#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SamgrUtilTest, CheckCallerProcess001, TestSize.Level3)
{
    CommonSaProfile saProfile;
    saProfile.process = u"";
    /**
     * @tc.steps: step1. test ConvertToOnDemandEvent
     */
    SystemAbilityOnDemandCondition condition;
    condition.eventId = OnDemandEventId::DEVICE_ONLINE;
    SystemAbilityOnDemandEvent from;
    from.conditions.push_back(condition);

    OnDemandEvent to;
    SamgrUtil::ConvertToOnDemandEvent(from, to);

    bool ret = SamgrUtil::CheckCallerProcess(saProfile);
    EXPECT_EQ(false, ret);
}
#endif

/**
 * @tc.name: CheckCallerProcess002
 * @tc.desc: test CheckCallerProcess with process is PROCESS_NAME
 * @tc.type: FUNC
 * @tc.require: I6V4AX
 */
#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SamgrUtilTest, CheckCallerProcess002, TestSize.Level3)
{
    CommonSaProfile saProfile;
    saProfile.process = PROCESS_NAME;
    /**
     * @tc.steps: step1. test ConvertToSystemAbilityOnDemandEvent
     */
    OnDemandCondition condition;
    condition.eventId = -1;
    OnDemandEvent from;
    from.conditions.push_back(condition);

    SystemAbilityOnDemandEvent to;
    SamgrUtil::ConvertToSystemAbilityOnDemandEvent(from, to);

    bool ret = SamgrUtil::CheckCallerProcess(saProfile);
    EXPECT_EQ(false, ret);
}
#endif

/**
 * @tc.name: CheckAllowUpdate001
 * @tc.desc: test CheckAllowUpdate with OnDemandPolicyType is START_POLICY, allowUpdate is true
 * @tc.type: FUNC
 * @tc.require: I6V4AX
 */
HWTEST_F(SamgrUtilTest, CheckAllowUpdate001, TestSize.Level3)
{
    OnDemandPolicyType type = OnDemandPolicyType::START_POLICY;
    CommonSaProfile saProfile;
    saProfile.startAllowUpdate = true;
    bool ret = SamgrUtil::CheckAllowUpdate(type, saProfile);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name: CheckAllowUpdate002
 * @tc.desc: test CheckAllowUpdate with OnDemandPolicyType is STOP_POLICY, allowUpdate is true
 * @tc.type: FUNC
 * @tc.require: I6V4AX
 */
HWTEST_F(SamgrUtilTest, CheckAllowUpdate002, TestSize.Level3)
{
    OnDemandPolicyType type = OnDemandPolicyType::STOP_POLICY;
    CommonSaProfile saProfile;
    saProfile.stopAllowUpdate = true;
    bool ret = SamgrUtil::CheckAllowUpdate(type, saProfile);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name: CheckAllowUpdate003
 * @tc.desc: test CheckAllowUpdate with OnDemandPolicyType is START_POLICY, allowUpdate is false
 * @tc.type: FUNC
 * @tc.require: I6V4AX
 */
HWTEST_F(SamgrUtilTest, CheckAllowUpdate003, TestSize.Level3)
{
    OnDemandPolicyType type = OnDemandPolicyType::START_POLICY;
    CommonSaProfile saProfile;
    saProfile.startAllowUpdate= false;
    bool ret = SamgrUtil::CheckAllowUpdate(type, saProfile);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: CheckAllowUpdate004
 * @tc.desc: test CheckAllowUpdate with OnDemandPolicyType is STOP_POLICY, allowUpdate is false
 * @tc.type: FUNC
 * @tc.require: I6V4AX
 */
HWTEST_F(SamgrUtilTest, CheckAllowUpdate004, TestSize.Level3)
{
    OnDemandPolicyType type = OnDemandPolicyType::STOP_POLICY;
    CommonSaProfile saProfile;
    saProfile.startAllowUpdate = false;
    bool ret = SamgrUtil::CheckAllowUpdate(type, saProfile);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: GetCacheCommonEventSa001
 * @tc.desc: call GetCacheCommonEventSa, return salist
 * @tc.type: FUNC
 * @tc.require: I7VEPG
 */
HWTEST_F(SamgrUtilTest, GetCacheCommonEventSa001, TestSize.Level3)
{
    SaControlInfo info1;
    info1.saId = 1;
    info1.cacheCommonEvent = true;
    SaControlInfo info2;
    info2.saId = 2;
    info2.cacheCommonEvent = true;
    SaControlInfo info3;
    info3.saId = 3;
    std::list<SaControlInfo> saControlList;
    saControlList.push_back(info1);
    saControlList.push_back(info2);
    saControlList.push_back(info3);

    OnDemandEvent event;
    event.eventId = 0;
    std::list<int32_t> saList = SamgrUtil::GetCacheCommonEventSa(event, saControlList);
    EXPECT_EQ(saList.size(), 0);

    event.eventId = COMMON_EVENT;
    event.extraDataId = -1;
    saList = SamgrUtil::GetCacheCommonEventSa(event, saControlList);
    EXPECT_EQ(saList.size(), 0);

    event.eventId = COMMON_EVENT;
    event.extraDataId = 1;
    saList = SamgrUtil::GetCacheCommonEventSa(event, saControlList);
    EXPECT_EQ(saList.size(), 2);
}

/**
 * @tc.name: SendUpdateSaState001
 * @tc.desc: test SendUpdateSaState
 * @tc.type: FUNC
 * @tc.require: I6V4AX
 */
HWTEST_F(SamgrUtilTest, SendUpdateSaState001, TestSize.Level3)
{
    std::string key = "";
    std::string value = "";
    SamgrUtil::SetModuleUpdateParam(key, value);
    std::string key_a = "key";
    std::string value_a = "value";
    SamgrUtil::SetModuleUpdateParam(key_a, value_a);
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
    saMgr->saProfileMap_.clear();
    CommonSaProfile saprofile;
    saprofile.moduleUpdate = true;
    saMgr->saProfileMap_[saprofile.saId] = saprofile;
    SamgrUtil::SendUpdateSaState(saprofile.saId, "test");
}

/**
 * @tc.name: CheckPengLai001
 * @tc.desc: test CheckPengLai
 * @tc.type: FUNC
 */
HWTEST_F(SamgrUtilTest, CheckPengLai001, TestSize.Level3)
{
    system::mockValue = PENG_LAI;
    EXPECT_TRUE(SamgrUtil::CheckPengLai());
}

/**
 * @tc.name: CheckPengLai002
 * @tc.desc: test CheckPengLai
 * @tc.type: FUNC
 */
HWTEST_F(SamgrUtilTest, CheckPengLai002, TestSize.Level3)
{
    system::mockValue = "not_penglai";
    EXPECT_FALSE(SamgrUtil::CheckPengLai());
}

/**
 * @tc.name: CheckPengLai003
 * @tc.desc: test CheckPengLai
 * @tc.type: FUNC
 */
HWTEST_F(SamgrUtilTest, CheckPengLai003, TestSize.Level3)
{
    system::mockValue = "";
    EXPECT_FALSE(SamgrUtil::CheckPengLai());
}

/**
 * @tc.name: TestGetFilesByPriority001
 * @tc.desc: test penglai mode
 * @tc.type: FUNC
 */
HWTEST_F(SamgrUtilTest, TestGetFilesByPriority001, TestSize.Level3)
{
    system::mockValue = PENG_LAI;
    mockDirFiles = {"/sys_prod/profile/penglai/file1", "/sys_prod/profile/penglai/file2"};
    
    std::vector<std::string> result;
    SamgrUtil::GetFilesByPriority("test_path", result);
    
    ASSERT_TRUE(result.empty());
    system::mockValue = "";
}

/**
 * @tc.name: TestGetFilesByPriority002
 * @tc.desc: test empty paths
 * @tc.type: FUNC
 */
HWTEST_F(SamgrUtilTest, TestGetFilesByPriority002, TestSize.Level3)
{
    system::mockValue = "";
    g_mockCfgFiles = new CfgFiles();
    
    std::vector<std::string> result;
    SamgrUtil::GetFilesByPriority("test_path", result);
    
    ASSERT_TRUE(result.empty());
    delete g_mockCfgFiles;
}

#ifdef SUPPORT_PENGLAI_MODE
/**
 * @tc.name: InitPenglaiFunc001
 * @tc.desc: test InitPenglaiFunc
 * @tc.type: FUNC
 */
HWTEST_F(SamgrUtilTest, InitPenglaiFunc001, TestSize.Level3)
{
    system::mockValue = PENG_LAI;
    void* func = SamgrUtil::InitPenglaiFunc();
    EXPECT_TRUE(func != nullptr);
}

/**
 * @tc.name: CheckPengLaiPermission001
 * @tc.desc: test CheckPengLaiPermission
 * @tc.type: FUNC
 */
HWTEST_F(SamgrUtilTest, CheckPengLaiPermission001, TestSize.Level3)
{
    SamgrUtil::penglaiFunc_ = nullptr;
    bool ret = SamgrUtil::CheckPengLaiPermission(1);
    EXPECT_TRUE(ret);
}
#endif

/**
 * @tc.name: GetProcessNameByPid001
 * @tc.desc: test valid GetProcessNameByPid
 * @tc.type: FUNC
 */
HWTEST_F(SamgrUtilTest, GetProcessNameByPid001, TestSize.Level3)
{
    DTEST_LOG << "GetProcessNameByPid001 start" << std::endl;
    int32_t pid = getpid(); // test value
    std::string testName = "SystemAbilityMgrCollectTest";
    std::string procName = SamgrUtil::GetProcessNameByPid(pid);
    EXPECT_EQ(testName, procName);
    DTEST_LOG << "GetProcessNameByPid001 end" << std::endl;
}

/**
 * @tc.name: GetProcessNameByPid002
 * @tc.desc: test invalid GetProcessNameByPid
 * @tc.type: FUNC
 */
HWTEST_F(SamgrUtilTest, GetProcessNameByPid002, TestSize.Level3)
{
    DTEST_LOG << "GetProcessNameByPid002 start" << std::endl;
    int32_t pid = 70000; // test value
    std::string procName = SamgrUtil::GetProcessNameByPid(pid);
    EXPECT_EQ("", procName);
    DTEST_LOG << "GetProcessNameByPid002 end" << std::endl;
}

/**
 * @tc.name: ParsePeerBinderPid001
 * @tc.desc: test valid ParsePeerBinderPid
 * @tc.type: FUNC
 */
HWTEST_F(SamgrUtilTest, ParsePeerBinderPid001, TestSize.Level3)
{
    DTEST_LOG << "ParsePeerBinderPid001 start" << std::endl;
    int32_t pid = 70000; // test value
    int32_t tid = 70000; // test value
    std::string path = "/data/test/test1.txt";
    std::ofstream ofs(path, std::ios::trunc);
    if (!ofs.is_open()) {
        DTEST_LOG << "open file failed!, path=" << path << std::endl;
        FAIL();
    }
    ofs << "aync 1:1 to 2:2 code 9 wait:4 s test" << std::endl;
    ofs << ":70000 to 70001:70001 code 9 wait:1 s test" << std::endl;
    ofs << "70000:70000 to :70001 code 9 wait:1 s test" << std::endl;
    ofs << "70000:70000 to 70001:70001 code 9 wait: s test" << std::endl;
    ofs << ":70000 to :70001 code 9 wait:1 s test" << std::endl;
    ofs << ":70000 to 70001:70001 code 9 wait: s test" << std::endl;
    ofs << "70000:70000 to :70001 code 9 wait: s test" << std::endl;
    ofs << ":70000 to :70001 code 9 wait: s test" << std::endl;
    ofs << "70000:70002 to 70001:70001 code 9 wait:1 s test" << std::endl;
    ofs << "70002:70000 to 70001:70001 code 9 wait:1 s test" << std::endl;
    ofs << "70002:70002 to 70001:70001 code 9 wait:61 s test" << std::endl;
    ofs << "70000:70000 to 70001:70001 code 9 wait:1 s test" << std::endl;
    ofs << "70000:70002 to 70001:70001 code 9 wait:61 s test" << std::endl;
    ofs << "70002:70000 to 70001:70001 code 9 wait:61 s test" << std::endl;
    ofs << "70000:70000 to 70001:70001 code 9 wait:61 s test" << std::endl;
    ofs << "72000:72000 to 70001:70001 code 9 wait:1 s test" << std::endl;
    ofs << "72abc:72000 to 70001:70001 code 9 wait:1 s test" << std::endl;
    ofs << "abc:72000 to 70001:70001 code 9 wait:1 s test" << std::endl;
    ofs << "3000000000:72000 to 70001:70001 code 9 wait:1 s test" << std::endl;
    ofs << "-3000000000:72000 to 70001:70001 code 9 wait:1 s test" << std::endl;
    ofs.close();
    std::ifstream fin(path);
    if (!fin.is_open()) {
        DTEST_LOG << "open file failed!, path=" << path << std::endl;
        FAIL();
    }
    int result = SamgrUtil::ParsePeerBinderPid(fin, pid, tid);
    fin.close();
    EXPECT_TRUE(result > 0);
    DTEST_LOG << "ParsePeerBinderPid001 end" << std::endl;
}

/**
 * @tc.name: ParsePeerBinderPid002
 * @tc.desc: test invalid ParsePeerBinderPid
 * @tc.type: FUNC
 */
HWTEST_F(SamgrUtilTest, ParsePeerBinderPid002, TestSize.Level3)
{
    DTEST_LOG << "ParsePeerBinderPid002 start" << std::endl;
    int32_t pid = 70000; // test value
    int32_t tid = 70000; // test value
    std::string path = "/data/test/test2.txt";
    std::ofstream ofs(path, std::ios::trunc);
    if (!ofs.is_open()) {
        DTEST_LOG << "open file failed!, path=" << path << std::endl;
        FAIL();
    }
    ofs << "context" << std::endl;
    ofs.close();
    std::ifstream fin(path);
    if (!fin.is_open()) {
        DTEST_LOG << "open file failed!, path=" << path << std::endl;
        FAIL();
    }
    int result = SamgrUtil::ParsePeerBinderPid(fin, pid, tid);
    fin.close();
    EXPECT_TRUE(result < 0);
    DTEST_LOG << "ParsePeerBinderPid002 end" << std::endl;
}

/**
 * @tc.name: KillProcessByPid001
 * @tc.desc: test kill process
 * @tc.type: FUNC
 */
HWTEST_F(SamgrUtilTest, KillProcessByPid001, TestSize.Level3)
{
    DTEST_LOG << "KillProcessByPid001 start" << std::endl;
    int32_t pid = 70000; // test value
    int32_t tid = 70000; // test value
    bool ret = SamgrUtil::KillProcessByPid(pid, tid);
    EXPECT_EQ(ret, false);
    DTEST_LOG << "KillProcessByPid001 end" << std::endl;
}
}