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

#include "device_param_collect_test.h"

#include "device_status_collect_manager.h"
#include "sa_profiles.h"
#include "string_ex.h"
#include "test_log.h"
#include "icollect_plugin.h"

#define private public
#include "collect/device_param_collect.h"

using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS;

namespace OHOS {
namespace {
const std::string TEST_NAME = "param_test";
}
void DeviceParamCollectTest::SetUpTestCase()
{
    DTEST_LOG << "SetUpTestCase" << std::endl;
}

void DeviceParamCollectTest::TearDownTestCase()
{
    DTEST_LOG << "TearDownTestCase" << std::endl;
}

void DeviceParamCollectTest::SetUp()
{
    DTEST_LOG << "SetUp" << std::endl;
}

void DeviceParamCollectTest::TearDown()
{
    DTEST_LOG << "TearDown" << std::endl;
}

/**
 * @tc.name: DeviceParamInit001
 * @tc.desc: test DeviceParamInit
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(DeviceParamCollectTest, DeviceParamInit001, TestSize.Level3)
{
    sptr<IReport> report;
    std::shared_ptr<DeviceParamCollect> deviceParamCollect =
        std::make_shared<DeviceParamCollect>(report);
    std::list<SaProfile> SaProfiles;
    SaProfile saProfile;
    OnDemandEvent onDemandEvent = {3, TEST_NAME, "true"};
    saProfile.startOnDemand.push_back(onDemandEvent);
    SaProfiles.push_back(saProfile);
    deviceParamCollect->params_.clear();
    deviceParamCollect->Init(SaProfiles);
    EXPECT_FALSE(deviceParamCollect->params_.empty());
}

/**
 * @tc.name: DeviceParamInit002
 * @tc.desc: test DeviceParamInit
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(DeviceParamCollectTest, DeviceParamInit002, TestSize.Level3)
{
    sptr<IReport> report;
    std::shared_ptr<DeviceParamCollect> deviceParamCollect =
        std::make_shared<DeviceParamCollect>(report);
    std::list<SaProfile> SaProfiles;
    SaProfile saProfile;
    OnDemandEvent onDemandEvent = {3, TEST_NAME, "false"};
    saProfile.stopOnDemand.push_back(onDemandEvent);
    SaProfiles.push_back(saProfile);
    deviceParamCollect->params_.clear();
    deviceParamCollect->Init(SaProfiles);
    EXPECT_FALSE(deviceParamCollect->params_.empty());
}

/**
 * @tc.name: OnStop001
 * @tc.desc: test OnStop
 * @tc.type: FUNC
 * @tc.require: I6FDNZ
 */

HWTEST_F(DeviceParamCollectTest, OnStop001, TestSize.Level3)
{
    sptr<IReport> report;
    std::shared_ptr<DeviceParamCollect> deviceParamCollect =
        std::make_shared<DeviceParamCollect>(report);
    int32_t ret = deviceParamCollect->OnStop();
    EXPECT_EQ(ret, ERR_OK);
}
}  // namespace OHOS