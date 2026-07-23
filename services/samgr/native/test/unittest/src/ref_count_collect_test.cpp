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

#define private public
#include "ref_count_collect_test.h"

#include "test_log.h"
using namespace testing::ext;

namespace OHOS {
using namespace OHOS::Utils;
constexpr int32_t REF_ONDEMAND_TIMER_INTERVAL = 1000 * 60 * 1;
void RefCountCollectTest::SetUpTestCase()
{
    DTEST_LOG << "SetUpTestCase" << std::endl;
}

void RefCountCollectTest::TearDownTestCase()
{
    DTEST_LOG << "TearDownTestCase" << std::endl;
}

void RefCountCollectTest::SetUp()
{
    DTEST_LOG << "SetUp" << std::endl;
}

void RefCountCollectTest::TearDown()
{
    DTEST_LOG << "TearDown" << std::endl;
}

/**
 * @tc.name: Init001
 * @tc.desc: test Init001, ondemand sa config unrefUnload init
 * @tc.type: FUNC
 */
HWTEST_F(RefCountCollectTest, Init001, TestSize.Level1)
{
    sptr<DeviceStatusCollectManager> manager = new DeviceStatusCollectManager();
    sptr<RefCountCollect> collect = new RefCountCollect(manager);
    std::list<SaProfile> saProfiles;
    {
        SaProfile saProfile;
        saProfile.saId = 1;
        saProfile.stopOnDemand.unrefUnload = true;
        saProfiles.push_back(saProfile);
    }
    {
        SaProfile saProfile;
        saProfile.saId = 2;
        saProfile.stopOnDemand.unrefUnload = false;
        saProfiles.push_back(saProfile);
    }
    collect->Init(saProfiles);
    EXPECT_EQ(collect->unrefUnloadSaList_.size(), 1);
    EXPECT_EQ(collect->unrefUnloadSaList_.front(), 1);
}

/**
 * @tc.name: Init003
 * @tc.desc: test Init003, ondemand sa not config unrefUnload init
 * @tc.type: FUNC
 */
HWTEST_F(RefCountCollectTest, Init003, TestSize.Level1)
{
    sptr<DeviceStatusCollectManager> manager = new DeviceStatusCollectManager();
    sptr<RefCountCollect> collect = new RefCountCollect(manager);
    std::list<SaProfile> saProfiles;
    SaProfile saProfile;
    saProfile.saId = 1;
    saProfile.runOnCreate = false;
    saProfile.stopOnDemand.unrefUnload = false;
    saProfiles.push_back(saProfile);

    collect->Init(saProfiles);
    EXPECT_EQ(collect->unrefUnloadSaList_.size(), 0);
}

/**
 * @tc.name: OnStart001
 * @tc.desc: test OnStart001, start ondemand sa config unrefUnload check timer
 * @tc.type: FUNC
 */
HWTEST_F(RefCountCollectTest, OnStart001, TestSize.Level1)
{
    sptr<DeviceStatusCollectManager> manager = new DeviceStatusCollectManager();
    sptr<RefCountCollect> collect = new RefCountCollect(manager);
    collect->unrefUnloadSaList_.push_back(1);
    auto ret = collect->OnStart();
    EXPECT_EQ(ret, ERR_OK);
    ret = collect->OnStop();
    usleep(500 * 1000);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: OnStart003
 * @tc.desc: test OnStart003, both list are empty onstart test
 * @tc.type: FUNC
 */
HWTEST_F(RefCountCollectTest, OnStart003, TestSize.Level1)
{
    sptr<DeviceStatusCollectManager> manager = new DeviceStatusCollectManager();
    sptr<RefCountCollect> collect = new RefCountCollect(manager);
    collect->unrefUnloadSaList_.clear();
    auto ret = collect->OnStart();
    EXPECT_EQ(ret, ERR_OK);
    ret = collect->OnStop();
    usleep(500 * 1000);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: OnStop001
 * @tc.desc: test OnStop001, timer is empty stop test
 * @tc.type: FUNC
 */
HWTEST_F(RefCountCollectTest, OnStop001, TestSize.Level1)
{
    sptr<DeviceStatusCollectManager> manager = new DeviceStatusCollectManager();
    sptr<RefCountCollect> collect = new RefCountCollect(manager);
    collect->timer_ = nullptr;
    auto ret = collect->OnStop();
    EXPECT_EQ(ret, ERR_OK);
}

HWTEST_F(RefCountCollectTest, IdentifyUnrefOndemand001, TestSize.Level1)
{
    DTEST_LOG<<"IdentifyUnrefOndemand001 BEGIN"<<std::endl;
    sptr<DeviceStatusCollectManager> manager = new DeviceStatusCollectManager();
    sptr<RefCountCollect> statuCollect = new RefCountCollect(manager);
    uint32_t timerId = 0;
    statuCollect->timer_ = std::make_unique<Utils::Timer>("refCountCollectTimer");
    statuCollect->timer_->Setup();
    statuCollect->unrefUnloadSaList_.push_back(1);
    timerId = statuCollect->timer_->Register(std::bind(&RefCountCollect::IdentifyUnrefOndemand, statuCollect),
       REF_ONDEMAND_TIMER_INTERVAL);

    EXPECT_NE(nullptr, statuCollect->timer_);
    int32_t ret = statuCollect->OnStop();
    EXPECT_EQ(ret, ERR_OK);
    DTEST_LOG<<"IdentifyUnrefOndemand001 END"<<std::endl;
}

} // namespace OHOS