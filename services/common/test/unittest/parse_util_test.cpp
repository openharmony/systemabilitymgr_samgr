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
#include "string_ex.h"
#include "system_ability_definition.h"
#include "test_log.h"
#include "tools.h"

#define private public
#include "parse_util.h"
using namespace std;
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace SAMGR {
namespace {
    const std::string TEST_RESOURCE_PATH = "/data/test/resource/samgr/profile/";
    const std::u16string TEST_PROCESS_NAME = u"sa_test";
    const std::string EVENT_STRING;
    const std::string EVENT_TYPE ;
    const std::string EVENT_NAME ;
    const std::string EVENT_VALUE ;
    const int32_t TEST_NUM = 123;
    const int32_t MOCK_SAID = 1492;
    const int32_t TEST_PROFILE_SAID = 9999;
    const int32_t TEST_PROFILE_SAID_INVAILD = 9990;
    constexpr const char* SA_TAG_DEVICE_ON_LINE = "deviceonline";
    constexpr const char* SA_TAG_SETTING_SWITCH = "settingswitch";
    constexpr const char* SA_TAG_COMMON_EVENT = "commonevent";
    constexpr const char* SA_TAG_PARAM = "param";
    constexpr const char* SA_TAG_TIEMD_EVENT = "timedevent";
}

class ParseUtilTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
protected:
    std::shared_ptr<ParseUtil> parser_;
};

void ParseUtilTest::SetUpTestCase()
{
    DTEST_LOG << "SetUpTestCase" << std::endl;
}

void ParseUtilTest::TearDownTestCase()
{
    DTEST_LOG << "TearDownTestCase" << std::endl;
}

void ParseUtilTest::SetUp()
{
    DTEST_LOG << "SetUp" << std::endl;
    if (parser_ == nullptr) {
        parser_ = std::make_shared<ParseUtil>();
    }
}

void ParseUtilTest::TearDown()
{
    DTEST_LOG << "TearDown" << std::endl;
    if (parser_ != nullptr) {
        parser_->ClearResource();
    }
}

/**
 * @tc.name: ParseSaProfile001
 * @tc.desc: Verify if can load not exist file
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, ParseSaProfile001, TestSize.Level2)
{
    DTEST_LOG << " ParseSaProfile001 start " << std::endl;
    /**
     * @tc.steps: step1. parse not exsit config file
     * @tc.expected: step1. return false when load not exist file
     */
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "notExist");
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: ParseSaProfile002
 * @tc.desc: Verify if can load invalid root file
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, ParseSaProfile002, TestSize.Level2)
{
    DTEST_LOG << " ParseSaProfile002 start " << std::endl;
    /**
     * @tc.steps: step1. load invalid root config file
     * @tc.expected: step1. return false when load invalid format config file
     */
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "invalid_root.xml");
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: ParseSaProfile003
 * @tc.desc: Verify if can load normal profile
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, ParseSaProfile003, TestSize.Level2)
{
    DTEST_LOG << " ParseSaProfile003 start " << std::endl;
    /**
     * @tc.steps: step1. load correct system ability profile
     * @tc.expected: step1. return true when load invalid format config file
     */
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "profile.xml");
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: ParseSaProfile004
 * @tc.desc: Verify if can load normal profile
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, ParseSaProfile004, TestSize.Level2)
{
    DTEST_LOG << " ParseSaProfile004 start " << std::endl;
    /**
     * @tc.steps: step1. load correct system ability profile
     * @tc.expected: step1. return false when load noxmlfile
     */
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "no_xml_file");
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: ParseSaProfile005
 * @tc.desc: Verify if can load normal profile
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, ParseSaProfile005, TestSize.Level2)
{
    DTEST_LOG << " ParseSaProfile005 start " << std::endl;
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "empty_process.xml");
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: ParseSaProfile006
 * @tc.desc: Verify if can load normal profile
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, ParseSaProfile006, TestSize.Level2)
{
    DTEST_LOG << " ParseSaProfile006 start " << std::endl;
    /**
     * @tc.steps: step1. load correct system ability profile
     * @tc.expected: step1. return false when load invalid format config file
     */
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "empty_systemability.xml");
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: GetSaProfiles001
 * @tc.desc:  Verify if not load sa file return empty list
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, GetSaProfiles001, TestSize.Level3)
{
    DTEST_LOG << " GetSaProfiles001 start " << std::endl;
    /**
     * @tc.steps: step1. Get empty config when not parse sa file.
     * @tc.expected: step1. return empty list when not load sa file
     */
    list<SaProfile> profiles = parser_->GetAllSaProfiles();
    EXPECT_TRUE(profiles.empty());
}

/**
 * @tc.name: GetSaProfiles002
 * @tc.desc:  Verify if can load normal sa profile
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, GetSaProfiles002, TestSize.Level3)
{
    DTEST_LOG << " GetSaProfiles002 start " << std::endl;
    /**
     * @tc.steps: step1. Get correct profile when parse sa file.
     * @tc.expected: step1. return correct profile object when load correct config file
     */
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "profile.xml");
    ASSERT_TRUE(ret);
    SaProfile saRunOnCreateTrue;
    saRunOnCreateTrue.runOnCreate = true;
    SaProfile saRunOnCreateFalse;
    parser_->saProfiles_.emplace_back(saRunOnCreateTrue);
    parser_->saProfiles_.emplace_back(saRunOnCreateFalse);
    list<SaProfile> profiles = parser_->GetAllSaProfiles();
    if (!profiles.empty()) {
        SaProfile& profile = *(profiles.begin());
        EXPECT_EQ(profile.process, TEST_PROCESS_NAME);
        EXPECT_EQ(profile.saId, TEST_PROFILE_SAID);
    }
}

/**
 * @tc.name: ParseTrustConfig001
 * @tc.desc:  Verify if can load file with one sa
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, ParseTrustConfig001, TestSize.Level1)
{
    DTEST_LOG << " ParseTrustConfig001 start " << std::endl;
    /**
     * @tc.steps: step1. Get correct map when parse config file.
     * @tc.expected: step1. return correct profile object when load correct config file
     */
    std::map<std::u16string, std::set<int32_t>> values;
    bool ret = parser_->ParseTrustConfig(TEST_RESOURCE_PATH + "test_trust_one_sa.json", values);
    ASSERT_TRUE(ret);
    /**
     * @tc.steps: step2. Check map values
     * @tc.expected: step2. return expect values
     */
    for (const auto& [process, saIds] : values) {
        EXPECT_EQ(Str16ToStr8(process), "test");
        EXPECT_EQ(saIds.size(), 1);
        EXPECT_EQ(saIds.count(1401), 1);
    }
}

/**
 * @tc.name: ParseTrustConfig002
 * @tc.desc:  Verify if can load file with muti sa
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, ParseTrustConfig002, TestSize.Level1)
{
    DTEST_LOG << " ParseTrustConfig002 start " << std::endl;
    /**
     * @tc.steps: step1. Get correct map when parse config file.
     * @tc.expected: step1. return correct profile object when load correct config file
     */
    std::map<std::u16string, std::set<int32_t>> values;
    bool ret = parser_->ParseTrustConfig(TEST_RESOURCE_PATH + "test_trust_muti_sa.json", values);
    ASSERT_TRUE(ret);
    /**
     * @tc.steps: step2. Check map values
     * @tc.expected: step2. return expect values
     */
    for (const auto& [process, saIds] : values) {
        EXPECT_EQ(Str16ToStr8(process), "test");
        EXPECT_EQ(saIds.size(), 5);
        EXPECT_EQ(saIds.count(1401), 1);
    }
}

/**
 * @tc.name: ParseTrustConfig003
 * @tc.desc:  Verify if can load not invalid root file
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, ParseTrustConfig003, TestSize.Level1)
{
    DTEST_LOG << " ParseTrustConfig003 start " << std::endl;
    /**
     * @tc.steps: step1. Get correct map when parse config file.
     * @tc.expected: step1. return false when load invalid root file
     */
    std::map<std::u16string, std::set<int32_t>> values;
    bool ret = parser_->ParseTrustConfig(TEST_RESOURCE_PATH + "invalid_root_trust.json", values);
    ASSERT_FALSE(ret);
}

/**
 * @tc.name: ParseTrustConfig004
 * @tc.desc:  Verify if can load not exist file
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, ParseTrustConfig004, TestSize.Level1)
{
    DTEST_LOG << " ParseTrustConfig004 start " << std::endl;
    /**
     * @tc.steps: step1. Get correct profile when parse sa file.
     * @tc.expected: step1. return false when not exist file
     */
    std::map<std::u16string, std::set<int32_t>> values;
    bool ret = parser_->ParseTrustConfig(TEST_RESOURCE_PATH + "notExist", values);
    ASSERT_FALSE(ret);
}

/**
 * @tc.name: ParseTrustConfig005
 * @tc.desc:  Verify if can load invalid element config
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, ParseTrustConfig005, TestSize.Level1)
{
    DTEST_LOG << " ParseTrustConfig005 start " << std::endl;
    /**
     * @tc.steps: step1. Get correct profile when parse invalid element config.
     * @tc.expected: step1. return correct profile object when load correct config file
     */
    std::map<std::u16string, std::set<int32_t>> values;
    bool ret = parser_->ParseTrustConfig(TEST_RESOURCE_PATH + "invalid_element_trust.json", values);
    ASSERT_TRUE(ret);
    for (const auto& [process, saIds] : values) {
        EXPECT_EQ(Str16ToStr8(process), "test");
        EXPECT_EQ(saIds.size(), 3);
        EXPECT_EQ(saIds.count(1401), 1);
    }
}

/**
 * @tc.name: ParseTrustConfig006
 * @tc.desc:  Verify if can load invalid muti root file
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, ParseTrustConfig006, TestSize.Level1)
{
    DTEST_LOG << " ParseTrustConfig006 start " << std::endl;
    /**
     * @tc.steps: step1. Get correct profile when parse sa file.
     * @tc.expected: step1. return correct profile object when load correct config file
     */
    std::map<std::u16string, std::set<int32_t>> values;
    bool ret = parser_->ParseTrustConfig(TEST_RESOURCE_PATH + "invalid_muti_root_trust.json", values);
    ASSERT_FALSE(ret);
}

/**
 * @tc.name: RemoveSaProfile001
 * @tc.desc:  Verify if can remove not-existed id
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, RemoveSaProfile001, TestSize.Level3)
{
    DTEST_LOG << " RemoveSaProfile001 start " << std::endl;
    /**
     * @tc.steps: step1. parse not exsit config file
     * @tc.expected: step1. return false when load not exist file
     */
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "notExist");
    EXPECT_FALSE(ret);
    /**
     * @tc.steps: step2. remove not-existed id
     * @tc.expected: step2. not crash
     */
    parser_->RemoveSaProfile(111);
    auto profiles = parser_->GetAllSaProfiles();
    EXPECT_EQ(profiles.size(), 0);
}

/**
 * @tc.name: RemoveSaProfile002
 * @tc.desc:  Verify if can can remove not-existed id
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, RemoveSaProfile002, TestSize.Level3)
{
    DTEST_LOG << " RemoveSaProfile002 start " << std::endl;
    /**
     * @tc.steps: step1. parse multi-sa profile
     * @tc.expected: step1. return true when load multi-sa profile
     */
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "multi_sa_profile.xml");
    EXPECT_TRUE(ret);
    auto profiles = parser_->GetAllSaProfiles();
    EXPECT_EQ(profiles.size(), 4);
    /**
     * @tc.steps: step2. remove not-existed id
     * @tc.expected: step2. not crash
     */
    parser_->RemoveSaProfile(111);
    profiles = parser_->GetAllSaProfiles();
    EXPECT_EQ(profiles.size(), 4);
}

/**
 * @tc.name: RemoveSaProfile003
 * @tc.desc:  Verify if can remove one existed id
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, RemoveSaProfile003, TestSize.Level3)
{
    DTEST_LOG << " RemoveSaProfile003 start " << std::endl;
    /**
     * @tc.steps: step1. parse multi-sa profile
     * @tc.expected: step1. return true when load multi-sa profile
     */
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "multi_sa_profile.xml");
    EXPECT_TRUE(ret);
    auto profiles = parser_->GetAllSaProfiles();
    EXPECT_EQ(profiles.size(), 4);
    /**
     * @tc.steps: step2. remove one existed id
     * @tc.expected: step2. remove successfully
     */
    parser_->RemoveSaProfile(9999);
    profiles = parser_->GetAllSaProfiles();
    EXPECT_EQ(profiles.size(), 3);
}

/**
 * @tc.name: RemoveSaProfile004
 * @tc.desc:  Verify if can remove one existed id
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, RemoveSaProfile004, TestSize.Level3)
{
    DTEST_LOG << " RemoveSaProfile004 start " << std::endl;
    /**
     * @tc.steps: step1. parse multi-sa profile
     * @tc.expected: step1. return true when load multi-sa profile
     */
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "multi_sa_profile.xml");
    EXPECT_TRUE(ret);
    auto profiles = parser_->GetAllSaProfiles();
    EXPECT_EQ(profiles.size(), 4);
    /**
     * @tc.steps: step2. remove one existed id
     * @tc.expected: step2. remove successfully
     */
    parser_->RemoveSaProfile(9997);
    profiles = parser_->GetAllSaProfiles();
    EXPECT_EQ(profiles.size(), 2);
}

/**
 * @tc.name: RemoveSaProfile005
 * @tc.desc:  Verify if can remove more existed id
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, RemoveSaProfile005, TestSize.Level3)
{
    DTEST_LOG << " RemoveSaProfile004 start " << std::endl;
    /**
     * @tc.steps: step1. parse multi-sa profile
     * @tc.expected: step1. return true when load multi-sa profile
     */
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "multi_sa_profile.xml");
    EXPECT_TRUE(ret);
    auto profiles = parser_->GetAllSaProfiles();
    EXPECT_EQ(profiles.size(), 4);
    /**
     * @tc.steps: step2. remove more existed id
     * @tc.expected: step2. remove successfully
     */
    parser_->RemoveSaProfile(9997);
    parser_->RemoveSaProfile(9998);
    parser_->RemoveSaProfile(9998);
    profiles = parser_->GetAllSaProfiles();
    EXPECT_EQ(profiles.size(), 1);
}

/**
 * @tc.name: CheckPathExist001
 * @tc.desc:  Verify if can check not exist file
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, CheckPathExist001, TestSize.Level2)
{
    DTEST_LOG << " CheckPathExist001 start " << std::endl;
    /**
     * @tc.steps: step1. check not exsit config file
     * @tc.expected: step1. return false when check not exist file
     */
    bool ret = parser_->CheckPathExist(TEST_RESOURCE_PATH + "not_exist.xml");
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: CheckPathExist002
 * @tc.desc:  Verify if can check exist file
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, CheckPathExist002, TestSize.Level2)
{
    DTEST_LOG << " CheckPathExist002 start " << std::endl;
    /**
     * @tc.steps: step1. check exsit config file
     * @tc.expected: step1. return true when load not exist file
     */
    bool ret = parser_->CheckPathExist(TEST_RESOURCE_PATH + "multi_sa_profile.xml");
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: GetProfile001
 * @tc.desc: Verify if can get not-exist profile
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(ParseUtilTest, GetProfile001, TestSize.Level2)
{
    DTEST_LOG << " GetProfile001 start " << std::endl;
    /**
     * @tc.steps: step1. check exsit config file
     * @tc.expected: step1. return true when load not exist file
     */
    SaProfile saProfile;
    bool ret = parser_->GetProfile(TEST_PROFILE_SAID, saProfile);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: GetProfile002
 * @tc.desc: Verify if can get exist profile
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(ParseUtilTest, GetProfile002, TestSize.Level2)
{
    DTEST_LOG << " GetProfile002 start " << std::endl;
    /**
     * @tc.steps: step1. check exsit config file
     * @tc.expected: step1. return true when load not exist file
     */
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "multi_sa_profile.xml");
    EXPECT_EQ(ret, true);
    SaProfile saProfile;
    ret = parser_->GetProfile(TEST_PROFILE_SAID, saProfile);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(saProfile.saId, TEST_PROFILE_SAID);
    EXPECT_EQ(saProfile.runOnCreate, true);
}

/**
 * @tc.name: LoadSaLib001
 * @tc.desc: Verify if can load salib
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(ParseUtilTest, LoadSaLib001, TestSize.Level2)
{
    DTEST_LOG << " LoadSaLib001 start " << std::endl;
    /**
     * @tc.steps: step1. check exsit salib
     * @tc.expected: step1. return true when load exist salib
     */
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "multi_sa_profile.xml");
    EXPECT_EQ(ret, true);
    ret = parser_->LoadSaLib(TEST_PROFILE_SAID);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: LoadSaLib002
 * @tc.desc: Verify if can load salib
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(ParseUtilTest, LoadSaLib002, TestSize.Level2)
{
    DTEST_LOG << " LoadSaLib002 start " << std::endl;
    /**
     * @tc.steps: step1. check exsit salib
     * @tc.expected: step1. return false when load not exist salib
     */
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "multi_sa_profile.xml");
    EXPECT_EQ(ret, true);
    ret = parser_->LoadSaLib(TEST_PROFILE_SAID_INVAILD);
    parser_->CloseSo(MOCK_SAID);
    EXPECT_NE(ret, true);
}

/**
 * @tc.name: LoadSaLib003
 * @tc.desc: Verify if can load salib
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(ParseUtilTest, LoadSaLib003, TestSize.Level2)
{
    DTEST_LOG << " LoadSaLib003 start " << std::endl;
    /**
     * @tc.steps: step1. check exsit salib
     * @tc.expected: step1. return false when load not exist salib
     */
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "mock.xml");
    EXPECT_EQ(ret, true);
    ret = parser_->LoadSaLib(MOCK_SAID);
    parser_->CloseSo(MOCK_SAID);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: LoadSaLib004
 * @tc.desc: Verify if can load salib
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(ParseUtilTest, LoadSaLib004, TestSize.Level2)
{
    DTEST_LOG << " LoadSaLib004 start " << std::endl;
    /**
     * @tc.steps: step1. check exsit salib
     * @tc.expected: step1. return false when load not exist salib
     */
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "mock.xml");
    EXPECT_EQ(ret, true);
    ret = parser_->LoadSaLib(MOCK_SAID);
    EXPECT_EQ(ret, true);
    parser_->LoadSaLib(MOCK_SAID);
}

/**
 * @tc.name: LoadSaLib005
 * @tc.desc: Verify if can load salib
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(ParseUtilTest, LoadSaLib005, TestSize.Level2)
{
    DTEST_LOG << " LoadSaLib005 start " << std::endl;
    /**
     * @tc.steps: step1. check exsit salib
     * @tc.expected: step1. return false when load not exist salib
     */
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "empty_libpath.xml");
    EXPECT_EQ(ret, true);
    ret = parser_->LoadSaLib(MOCK_SAID);
    EXPECT_EQ(ret, true);
}
/**
 * @tc.name: GetProcessName001
 * @tc.desc: Verify if can get procesname
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(ParseUtilTest, GetProcessName001, TestSize.Level3)
{
    DTEST_LOG << " GetProcessName001 " << std::endl;
    /**
     * @tc.steps: step1. get SaProfiles
     * @tc.expected: step1. return true when SaProfiles
     */
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "multi_sa_profile.xml");
    EXPECT_EQ(ret, true);
    std::u16string Name = parser_->GetProcessName();
    EXPECT_EQ(Str16ToStr8(Name), "test");
}

/**
 * @tc.name: GetProcessName002
 * @tc.desc: Verify if can get procesname
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(ParseUtilTest, GetProcessName002, TestSize.Level3)
{
    DTEST_LOG << " GetProcessName002 " << std::endl;
    /**
    * @tc.steps: step1. get SaProfiles
    * @tc.expected: step1. return true when SaProfiles
    */
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "multi_sa_profile.xml");
    EXPECT_EQ(ret, true);
    std::u16string Name = parser_->GetProcessName();
    EXPECT_NE(Str16ToStr8(Name), "test_1");
}

/**
 * @tc.name: ParseSAProp001
 * @tc.desc: Verify if can ParseSAProp
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(ParseUtilTest, ParseSAProp001, TestSize.Level3)
{
    DTEST_LOG << " ParseSAProp001 " << std::endl;
    string nodeName = "depend";
    string nodeContent = "TEST";
    SaProfile saProfile;
    parser_->ParseSAProp(nodeName, nodeContent, saProfile);
    EXPECT_EQ(saProfile.dependSa.size(), 1);
}

/**
 * @tc.name: ParseSAProp002
 * @tc.desc: Verify if can ParseSAProp
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(ParseUtilTest, ParseSAProp002, TestSize.Level3)
{
    DTEST_LOG << " ParseSAProp002 " << std::endl;
    string nodeName = "depend-time-out";
    string nodeContent = "123";
    SaProfile saProfile;
    parser_->ParseSAProp(nodeName, nodeContent, saProfile);
    EXPECT_EQ(saProfile.dependTimeout, TEST_NUM);
}

/**
 * @tc.name: ParseSAProp003
 * @tc.desc: Verify if can ParseSAProp
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(ParseUtilTest, ParseSAProp003, TestSize.Level3)
{
    DTEST_LOG << " ParseSAProp003 " << std::endl;
    string nodeName = "capability";
    string nodeContent = "TEST";
    SaProfile saProfile;
    parser_->ParseSAProp(nodeName, nodeContent, saProfile);
    EXPECT_EQ(nodeContent, Str16ToStr8(saProfile.capability));
}

/**
 * @tc.name: ParseSAProp004
 * @tc.desc: Verify if can ParseSAProp
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(ParseUtilTest, ParseSAProp004, TestSize.Level3)
{
    DTEST_LOG << " ParseSAProp004 " << std::endl;
    string nodeName = "permission";
    string nodeContent = "TEST";
    SaProfile saProfile;
    parser_->ParseSAProp(nodeName, nodeContent, saProfile);
    EXPECT_EQ(nodeContent, Str16ToStr8(saProfile.permission));
}

/**
 * @tc.name: ParseSAProp005
 * @tc.desc: Verify if can ParseSAProp
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(ParseUtilTest, ParseSAProp005, TestSize.Level3)
{
    DTEST_LOG << " ParseSAProp005 " << std::endl;
    string nodeName = "bootphase";
    string nodeContent = "TEST";
    SaProfile saProfile;
    parser_->ParseSAProp(nodeName, nodeContent, saProfile);
    EXPECT_EQ(3, saProfile.bootPhase);
}

/**
 * @tc.name: ParseSystemAbility001
 * @tc.desc: Verify if can ParseSystemAbility
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(ParseUtilTest, ParseSystemAbility001, TestSize.Level3)
{
    DTEST_LOG << " ParseSystemAbility001 " << std::endl;
    xmlNode rootNode;
    rootNode.xmlChildrenNode = nullptr;
    u16string process = u"tempprocess";
    bool res = parser_->ParseSystemAbility(rootNode, process);
    EXPECT_EQ(res, false);
}

/**
 * @tc.name: ParseSystemAbility002
 * @tc.desc: Verify if can ParseSystemAbility
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(ParseUtilTest, ParseSystemAbility002, TestSize.Level3)
{
    DTEST_LOG << " ParseSystemAbility002 " << std::endl;
    xmlNode rootNode;
    xmlNode tempNode;
    xmlNode* childrenNode = &tempNode;
    const char* nodeName = "test";
    childrenNode->name = reinterpret_cast<const xmlChar*>(nodeName);
    childrenNode->type = XML_PI_NODE;
    childrenNode->content = nullptr;
    childrenNode->next = nullptr;
    rootNode.xmlChildrenNode = childrenNode;
    u16string process = u"tempprocess";
    bool res = parser_->ParseSystemAbility(rootNode, process);
    EXPECT_EQ(res, true);
}

/**
 * @tc.name: ParseProcess001
 * @tc.desc: Verify if can ParseProcess
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(ParseUtilTest, ParseProcess001, TestSize.Level3)
{
    DTEST_LOG << " ParseProcess001 " << std::endl;
    xmlNode tempNode;
    xmlNode* rootNode = &tempNode;
    u16string process = u"tempprocess";
    bool res = parser_->ParseProcess(rootNode, process);
    EXPECT_EQ(res, false);
}

/**
 * @tc.name: ParseProcess002
 * @tc.desc: Verify if can ParseProcess
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(ParseUtilTest, ParseProcess002, TestSize.Level3)
{
    DTEST_LOG << " ParseProcess002 " << std::endl;
    xmlNode tempNode;
    xmlNode* rootNode = &tempNode;
    const char* nodeName = "test";
    rootNode->name = reinterpret_cast<const xmlChar*>(nodeName);
    rootNode->type = XML_PI_NODE;
    rootNode->content = nullptr;
    u16string process = u"tempprocess";
    bool res = parser_->ParseProcess(rootNode, process);
    EXPECT_EQ(res, false);
}

/**
 * @tc.name: DeleteAllMark001
 * @tc.desc: Verify if can DeleteAllMark
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(ParseUtilTest, DeleteAllMark001, TestSize.Level3)
{
    DTEST_LOG << " DeleteAllMark001 " << std::endl;
    u16string temp = u"stests";
    u16string mask = u"s";
    u16string res = DeleteAllMark(temp, mask);
    EXPECT_EQ(res, u"tet");
}

/**
 * @tc.name: GetOnDemandConditionsFromJson001
 * @tc.desc: parse OnDemandConditions, conditions is empty.
 * @tc.type: FUNC
 * @tc.require: I6JE38
 */
HWTEST_F(ParseUtilTest, GetOnDemandConditionsFromJson001, TestSize.Level3)
{
    DTEST_LOG << " GetOnDemandConditionsFromJson001 BEGIN" << std::endl;
    nlohmann::json obj;
    std::string key;
    std::vector<OnDemandCondition> out;
    SaProfile saProfile;
    parser_->GetOnDemandConditionsFromJson(obj, key, out);
    EXPECT_TRUE(out.empty());
    DTEST_LOG << " GetOnDemandConditionsFromJson001 END" << std::endl;
}

/**
 * @tc.name: GetOnDemandConditionsFromJson002
 * @tc.desc: parse OnDemandConditions, one condition.
 * @tc.type: FUNC
 * @tc.require: I6JE38
 */
HWTEST_F(ParseUtilTest, GetOnDemandConditionsFromJson002, TestSize.Level3)
{
    DTEST_LOG << " GetOnDemandConditionsFromJson002 BEGIN" << std::endl;
    nlohmann::json obj;
    nlohmann::json conditions;
    nlohmann::json condition;
    condition["eventId"] = SA_TAG_DEVICE_ON_LINE;
    condition["name"] = "mockName";
    condition["value"] = "mockValue";
    conditions[0] = condition;
    obj["conditions"] = conditions;

    std::string key = "conditions";
    std::vector<OnDemandCondition> out;
    SaProfile saProfile;
    parser_->GetOnDemandConditionsFromJson(obj, key, out);
    EXPECT_EQ(out.size(), 1);
    DTEST_LOG << " GetOnDemandConditionsFromJson002 END" << std::endl;
}

/**
 * @tc.name: GetOnDemandConditionsFromJson003
 * @tc.desc: parse OnDemandConditions, five condition.
 * @tc.type: FUNC
 * @tc.require: I6JE38
 */
HWTEST_F(ParseUtilTest, GetOnDemandConditionsFromJson003, TestSize.Level3)
{
    DTEST_LOG << " GetOnDemandConditionsFromJson003 BEGIN" << std::endl;
    nlohmann::json obj;
    nlohmann::json conditions;
    nlohmann::json condition;
    condition["eventId"] = SA_TAG_DEVICE_ON_LINE;
    condition["name"] = "mockName";
    condition["value"] = "mockValue";
    nlohmann::json condition2;
    condition2["eventId"] = SA_TAG_SETTING_SWITCH;
    condition2["name"] = "mockName";
    condition2["value"] = "mockValue";
    nlohmann::json condition3;
    condition3["eventId"] = SA_TAG_COMMON_EVENT;
    condition3["name"] = "mockName";
    condition3["value"] = "mockValue";
    nlohmann::json condition4;
    condition4["eventId"] = SA_TAG_PARAM;
    condition4["name"] = "mockName";
    condition4["value"] = "mockValue";
    nlohmann::json condition5;
    condition5["eventId"] = SA_TAG_TIEMD_EVENT;
    condition5["name"] = "mockName";
    condition5["value"] = "mockValue";
    conditions[0] = condition;
    conditions[1] = condition2;
    conditions[2] = condition3;
    conditions[3] = condition4;
    conditions[4] = condition5;
    obj["conditions"] = conditions;

    std::string key = "conditions";
    std::vector<OnDemandCondition> out;
    SaProfile saProfile;
    parser_->GetOnDemandConditionsFromJson(obj, key, out);
    EXPECT_EQ(out.size(), 5);
    DTEST_LOG << " GetOnDemandConditionsFromJson003 END" << std::endl;
}

/**
 * @tc.name: GetOnDemandConditionsFromJson004
 * @tc.desc: parse OnDemandConditions, invalid condition.
 * @tc.type: FUNC
 * @tc.require: I6JE38
 */
HWTEST_F(ParseUtilTest, GetOnDemandConditionsFromJson004, TestSize.Level3)
{
    DTEST_LOG << " GetOnDemandConditionsFromJson004 BEGIN" << std::endl;
    nlohmann::json obj;
    nlohmann::json conditions;
    nlohmann::json condition;
    condition["eventId"] = "mockeventId";
    condition["name"] = "mockName";
    condition["value"] = "mockValue";
    conditions[0] = condition;
    obj["conditions"] = conditions;

    std::string key = "conditions";
    std::vector<OnDemandCondition> out;
    SaProfile saProfile;
    parser_->GetOnDemandConditionsFromJson(obj, key, out);
    EXPECT_EQ(out.size(), 0);
    DTEST_LOG << " GetOnDemandConditionsFromJson004 END" << std::endl;
}

/**
 * @tc.name: ParseJsonFile001
 * @tc.desc: parse json file using big json file
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, ParseJsonFile001, TestSize.Level3)
{
    DTEST_LOG << " ParseJsonFile001 BEGIN" << std::endl;
    parser_->saProfiles_.clear();
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "sa_profile_large.json");
    EXPECT_EQ(ret, false);
    DTEST_LOG << " ParseJsonFile001 END" << std::endl;
}

/**
 * @tc.name: ParseJsonFile002
 * @tc.desc: parse json file using too long proces.
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, ParseJsonFile002, TestSize.Level3)
{
    DTEST_LOG << " ParseJsonFile002 BEGIN" << std::endl;
    parser_->saProfiles_.clear();
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "sa_profile_long_process.json");
    EXPECT_EQ(ret, false);
    DTEST_LOG << " ParseJsonFile002 END" << std::endl;
}

/**
 * @tc.name: ParseJsonFile003
 * @tc.desc: parse json file using error said.
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, ParseJsonFile003, TestSize.Level3)
{
    DTEST_LOG << " ParseJsonFile003 BEGIN" << std::endl;
    parser_->saProfiles_.clear();
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "sa_profile_error_said.json");
    EXPECT_EQ(ret, false);
    DTEST_LOG << " ParseJsonFile003 END" << std::endl;
}

/**
 * @tc.name: ParseJsonFile004
 * @tc.desc: parse json file using too long libpath.
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, ParseJsonFile004, TestSize.Level3)
{
    DTEST_LOG << " ParseJsonFile004 BEGIN" << std::endl;
    parser_->saProfiles_.clear();
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "sa_profile_long_libpath.json");
    EXPECT_EQ(ret, false);
    DTEST_LOG << " ParseJsonFile004 END" << std::endl;
}

/**
 * @tc.name: ParseJsonFile005
 * @tc.desc: parse json file using error bootPhase
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, ParseJsonFile005, TestSize.Level3)
{
    DTEST_LOG << " ParseJsonFile005 BEGIN" << std::endl;
    parser_->saProfiles_.clear();
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "sa_profile_error_bootphase.json");
    EXPECT_EQ(ret, true);
    DTEST_LOG << " ParseJsonFile005 END" << std::endl;
}

/**
 * @tc.name: ParseJsonFile006
 * @tc.desc: parse json file using error ondemand tag
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, ParseJsonFile006, TestSize.Level3)
{
    DTEST_LOG << " ParseJsonFile006 BEGIN" << std::endl;
    parser_->saProfiles_.clear();
    // name or vale is more than 128 bytes
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "sa_profile_error_ondemanad_tag.json");
    EXPECT_EQ(ret, true);
    SaProfile saProfile;
    parser_->GetProfile(1401, saProfile);
    EXPECT_EQ(true, saProfile.startOnDemand.onDemandEvents.empty());
    EXPECT_EQ(true, saProfile.stopOnDemand.onDemandEvents.empty());
    DTEST_LOG << " ParseJsonFile006 END" << std::endl;
}

/**
 * @tc.name: ParseJsonFile007
 * @tc.desc: parse json file using correct profile
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, ParseJsonFile007, TestSize.Level3)
{
    DTEST_LOG << " ParseJsonFile007 BEGIN" << std::endl;
    parser_->saProfiles_.clear();
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "sa_profile.json");
    EXPECT_EQ(ret, true);
    EXPECT_EQ(parser_->saProfiles_.empty(), false);
    SaProfile saProfile1;
    parser_->GetProfile(1401, saProfile1);
    EXPECT_EQ(1401, saProfile1.saId);
    EXPECT_EQ(true, !saProfile1.startOnDemand.onDemandEvents.empty());
    EXPECT_EQ(1, saProfile1.startOnDemand.onDemandEvents[0].eventId);
    EXPECT_EQ("deviceonline", saProfile1.startOnDemand.onDemandEvents[0].name);
    EXPECT_EQ("on", saProfile1.startOnDemand.onDemandEvents[0].value);
    EXPECT_EQ(true, !saProfile1.stopOnDemand.onDemandEvents.empty());
    EXPECT_EQ(1, saProfile1.stopOnDemand.onDemandEvents[0].eventId);
    EXPECT_EQ("deviceonline", saProfile1.stopOnDemand.onDemandEvents[0].name);
    EXPECT_EQ("off", saProfile1.stopOnDemand.onDemandEvents[0].value);
    SaProfile saProfile2;
    parser_->GetProfile(6001, saProfile2);
    EXPECT_EQ(6001, saProfile2.saId);
    DTEST_LOG << " ParseJsonFile007 END" << std::endl;
}

/**
 * @tc.name: ParseSystemAbility003
 * @tc.desc: parse sytemability tag with error param.
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, ParseSystemAbility003, TestSize.Level3)
{
    DTEST_LOG << " ParseSystemAbility003 BEGIN" << std::endl;
    nlohmann::json systemAbilityJson;
    SaProfile saProfile;
    bool ret = parser_->ParseSystemAbility(saProfile, systemAbilityJson);
    EXPECT_EQ(ret, false);
    systemAbilityJson["name"] = -1; // invalid said
    ret = parser_->ParseSystemAbility(saProfile, systemAbilityJson);
    EXPECT_EQ(ret, false);
    systemAbilityJson["name"] = DISTRIBUTED_DEVICE_PROFILE_SA_ID;
    ret = parser_->ParseSystemAbility(saProfile, systemAbilityJson);
    EXPECT_EQ(ret, false);
    systemAbilityJson["libpath"] = "/system/lib/test.so";
    ret = parser_->ParseSystemAbility(saProfile, systemAbilityJson);
    EXPECT_EQ(ret, true);
    systemAbilityJson["bootphase"] = "aaa";
    ret = parser_->ParseSystemAbility(saProfile, systemAbilityJson);
    EXPECT_EQ(ret, true);
    DTEST_LOG << " ParseSystemAbility003 END" << std::endl;
}

/**
 * @tc.name: JsonObjToMap
 * @tc.desc: eventJson.contains is nullptr
 * @tc.type: FUNC
 * @tc.require: I6MNUA
 */
HWTEST_F(ParseUtilTest, JsonObjToMap001, TestSize.Level3)
{
    DTEST_LOG << " JsonObjToMap001 BEGIN" << std::endl;
    nlohmann::json systemAbilityJson;
    std::unordered_map<std::string, std::string> res = parser_->JsonObjToMap(systemAbilityJson);
    EXPECT_EQ(res.size(), 3);
    DTEST_LOG << " JsonObjToMap001 END" << std::endl;
}
/**
 * @tc.name: JsonObjToMap
 * @tc.desc: eventJson.contains is event_type,evnt_name and event_value
 * @tc.type: FUNC
 * @tc.require: I6MNUA
 */
HWTEST_F(ParseUtilTest, JsonObjToMap002, TestSize.Level3)
{
    DTEST_LOG << " JsonObjToMap002 BEGIN" << std::endl;
    std::unordered_map<std::string, std::string> strMap;
    strMap[EVENT_TYPE] = "test";
    strMap[EVENT_NAME] = "test";
    strMap[EVENT_VALUE] = "test";
    nlohmann::json systemAbilityJson;
    for (auto it = strMap.begin(); it != strMap.end(); it++) {
        systemAbilityJson[it->first] = it->second;
    }
    std::unordered_map<std::string, std::string> res = parser_->JsonObjToMap(systemAbilityJson);
    EXPECT_EQ(res.size(), 3);
    DTEST_LOG << " JsonObjToMap002 END" << std::endl;
}
/**
 * @tc.name: JsonObjToMap
 * @tc.desc: eventJson.contains is evnt_name and event_value
 * @tc.type: FUNC
 * @tc.require: I6MNUA
 */
HWTEST_F(ParseUtilTest, JsonObjToMap003, TestSize.Level3)
{
    DTEST_LOG << " JsonObjToMap003 BEGIN" << std::endl;
    std::unordered_map<std::string, std::string> strMap;
    strMap[EVENT_NAME] = "test";
    strMap[EVENT_VALUE] = "test";
    nlohmann::json systemAbilityJson;
    for (auto it = strMap.begin(); it != strMap.end(); it++) {
        systemAbilityJson[it->first] = it->second;
    }
    std::unordered_map<std::string, std::string> res = parser_->JsonObjToMap(systemAbilityJson);
    EXPECT_EQ(res.size(), 3);
    DTEST_LOG << " JsonObjToMap003 END" << std::endl;
}
/**
 * @tc.name: JsonObjToMap
 * @tc.desc: eventJson.contains is event_type and event_value
 * @tc.type: FUNC
 * @tc.require: I6MNUA
 */
HWTEST_F(ParseUtilTest, JsonObjToMap004, TestSize.Level3)
{
    DTEST_LOG << " JsonObjToMap004 BEGIN" << std::endl;
    std::unordered_map<std::string, std::string> strMap;
    strMap[EVENT_TYPE] = "test";
    strMap[EVENT_VALUE] = "test";
    nlohmann::json systemAbilityJson;
    for (auto it = strMap.begin(); it != strMap.end(); it++) {
        systemAbilityJson[it->first] = it->second;
    }
    std::unordered_map<std::string, std::string> res = parser_->JsonObjToMap(systemAbilityJson);
    EXPECT_EQ(res.size(), 3);
    DTEST_LOG << " JsonObjToMap004 END" << std::endl;
}
/**
 * @tc.name: JsonObjToMap
 * @tc.desc: eventJson.contains is event_type and evnt_name
 * @tc.type: FUNC
 * @tc.require: I6MNUA
 */
HWTEST_F(ParseUtilTest, JsonObjToMap005, TestSize.Level3)
{
    DTEST_LOG << " JsonObjToMap005 BEGIN" << std::endl;
    std::unordered_map<std::string, std::string> strMap;
    strMap[EVENT_TYPE] = "test";
    strMap[EVENT_NAME] = "test";
    nlohmann::json systemAbilityJson;
    for (auto it = strMap.begin(); it != strMap.end(); it++) {
        systemAbilityJson[it->first] = it->second;
    }
    std::unordered_map<std::string, std::string> res = parser_->JsonObjToMap(systemAbilityJson);
    EXPECT_EQ(res.size(), 3);
    DTEST_LOG << " JsonObjToMap005 END" << std::endl;
}
/**
 * @tc.name: JsonObjToMap
 * @tc.desc: eventJson.contains is event_type
 * @tc.type: FUNC
 * @tc.require: I6MNUA
 */
HWTEST_F(ParseUtilTest, JsonObjToMap006, TestSize.Level3)
{
    DTEST_LOG << " JsonObjToMap006 BEGIN" << std::endl;
    std::unordered_map<std::string, std::string> strMap;
    strMap[EVENT_TYPE] = "test";
    nlohmann::json systemAbilityJson;
    for (auto it = strMap.begin(); it != strMap.end(); it++) {
        systemAbilityJson[it->first] = it->second;
    }
    std::unordered_map<std::string, std::string> res = parser_->JsonObjToMap(systemAbilityJson);
    EXPECT_EQ(res.size(), 3);
    DTEST_LOG << " JsonObjToMap006 END" << std::endl;
}
/**
 * @tc.name: JsonObjToMap
 * @tc.desc: eventJson.contains is event_name
 * @tc.type: FUNC
 * @tc.require: I6MNUA
 */
HWTEST_F(ParseUtilTest, JsonObjToMap007, TestSize.Level3)
{
    DTEST_LOG << " JsonObjToMap007 BEGIN" << std::endl;
    std::unordered_map<std::string, std::string> strMap;
    strMap[EVENT_NAME] = "test";
    nlohmann::json systemAbilityJson;
    for (auto it = strMap.begin(); it != strMap.end(); it++) {
        systemAbilityJson[it->first] = it->second;
    }
    std::unordered_map<std::string, std::string> res = parser_->JsonObjToMap(systemAbilityJson);
    EXPECT_EQ(res.size(), 3);
    DTEST_LOG << " JsonObjToMap007 END" << std::endl;
}
/**
 * @tc.name: JsonObjToMap
 * @tc.desc: eventJson.contains is event_value
 * @tc.type: FUNC
 * @tc.require: I6MNUA
 */
HWTEST_F(ParseUtilTest, JsonObjToMap008, TestSize.Level3)
{
    DTEST_LOG << " JsonObjToMap008 BEGIN" << std::endl;
    std::unordered_map<std::string, std::string> strMap;
    strMap[EVENT_VALUE] = "test";
    nlohmann::json systemAbilityJson;
    for (auto it = strMap.begin(); it != strMap.end(); it++) {
        systemAbilityJson[it->first] = it->second;
    }
    std::unordered_map<std::string, std::string> res = parser_->JsonObjToMap(systemAbilityJson);
    EXPECT_EQ(res.size(), 3);
    DTEST_LOG << " JsonObjToMap008 END" << std::endl;
}
/**
 * @tc.name: JsonObjToMap
 * @tc.desc: eventJson.contains is event_type,evnt_name and event_value;eventJson.is_string is false
 * @tc.type: FUNC
 * @tc.require: I6MNUA
 */
HWTEST_F(ParseUtilTest, JsonObjToMap009, TestSize.Level3)
{
    DTEST_LOG << " JsonObjToMap009 BEGIN" << std::endl;
    std::unordered_map<std::string, int> strMap;
    strMap[EVENT_TYPE] = TEST_NUM;
    strMap[EVENT_NAME] = TEST_NUM;
    strMap[EVENT_VALUE] = TEST_NUM;
    nlohmann::json systemAbilityJson;
    for (auto it = strMap.begin(); it != strMap.end(); it++) {
        systemAbilityJson[it->first] = it->second;
    }
    std::unordered_map<std::string, std::string> res = parser_->JsonObjToMap(systemAbilityJson);
    EXPECT_EQ(res.size(), 3);
    DTEST_LOG << " JsonObjToMap009 END" << std::endl;
}
/**
 * @tc.name: JsonObjToMap
 * @tc.desc: eventJson.contains is evnt_name and event_value;eventJson.is_string is false
 * @tc.type: FUNC
 * @tc.require: I6MNUA
 */
HWTEST_F(ParseUtilTest, JsonObjToMap010, TestSize.Level3)
{
    DTEST_LOG << " JsonObjToMap010 BEGIN" << std::endl;
    std::unordered_map<std::string, int> strMap;
    strMap[EVENT_NAME] = TEST_NUM;
    strMap[EVENT_VALUE] = TEST_NUM;
    nlohmann::json systemAbilityJson;
    for (auto it = strMap.begin(); it != strMap.end(); it++) {
        systemAbilityJson[it->first] = it->second;
    }
    std::unordered_map<std::string, std::string> res = parser_->JsonObjToMap(systemAbilityJson);
    EXPECT_EQ(res.size(), 3);
    DTEST_LOG << " JsonObjToMap010 END" << std::endl;
}
/**
 * @tc.name: JsonObjToMap
 * @tc.desc: eventJson.contains is event_value;eventJson.is_string is false
 * @tc.type: FUNC
 * @tc.require: I6MNUA
 */
HWTEST_F(ParseUtilTest, JsonObjToMap011, TestSize.Level3)
{
    DTEST_LOG << " JsonObjToMap011 BEGIN" << std::endl;
    std::unordered_map<std::string, int> strMap;
    strMap[EVENT_VALUE] = TEST_NUM;
    nlohmann::json systemAbilityJson;
    for (auto it = strMap.begin(); it != strMap.end(); it++) {
        systemAbilityJson[it->first] = it->second;
    }
    std::unordered_map<std::string, std::string> res = parser_->JsonObjToMap(systemAbilityJson);
    EXPECT_EQ(res.size(), 3);
    DTEST_LOG << " JsonObjToMap011 END" << std::endl;
}

/**
 * @tc.name: StringToMap001
 * @tc.desc: test StringToMap with empty string
 * @tc.type: FUNC
 * @tc.require: I6YJH6
 */
HWTEST_F(ParseUtilTest, StringtoMap001, TestSize.Level3)
{
    unordered_map<std::string, std::string> ret = parser_->StringToMap(EVENT_STRING);
    EXPECT_FALSE(ret.empty());
}
} // namespace SAMGR
} // namespace OHOS
