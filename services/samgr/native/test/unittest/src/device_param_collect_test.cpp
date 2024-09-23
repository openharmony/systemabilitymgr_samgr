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
#include "system_ability_definition.h"
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
    saProfile.startOnDemand.onDemandEvents.push_back(onDemandEvent);
    SaProfiles.push_back(saProfile);
    deviceParamCollect->pendingParams_.clear();
    deviceParamCollect->Init(SaProfiles);
    EXPECT_FALSE(deviceParamCollect->pendingParams_.empty());
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
    saProfile.stopOnDemand.onDemandEvents.push_back(onDemandEvent);
    SaProfiles.push_back(saProfile);
    deviceParamCollect->pendingParams_.clear();
    deviceParamCollect->Init(SaProfiles);
    EXPECT_FALSE(deviceParamCollect->pendingParams_.empty());
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

/**
 * @tc.name: OnAddSystemAbility001
 * @tc.desc: test OnAddSystemAbility, with invalid sa id
 * @tc.type: FUNC
 * @tc.require: I6OU0A
 */

HWTEST_F(DeviceParamCollectTest, OnAddSystemAbility001, TestSize.Level3)
{
    sptr<IReport> report;
    sptr<DeviceParamCollect> deviceParamCollect = new DeviceParamCollect(report);
    sptr<SystemAbilityStatusChange> statusChangeListener = new SystemAbilityStatusChange();
    statusChangeListener->Init(deviceParamCollect);
    statusChangeListener->OnAddSystemAbility(-1, "");
    EXPECT_NE(-1, PARAM_WATCHER_DISTRIBUTED_SERVICE_ID);
}

/**
 * @tc.name: OnAddSystemAbility002
 * @tc.desc: test OnAddSystemAbility, with valid sa id
 * @tc.type: FUNC
 * @tc.require: I6OU0A
 */

HWTEST_F(DeviceParamCollectTest, OnAddSystemAbility002, TestSize.Level3)
{
    sptr<IReport> report;
    sptr<DeviceParamCollect> deviceParamCollect = new DeviceParamCollect(report);
    sptr<SystemAbilityStatusChange> statusChangeListener = new SystemAbilityStatusChange();
    statusChangeListener->Init(deviceParamCollect);
    statusChangeListener->OnAddSystemAbility(PARAM_WATCHER_DISTRIBUTED_SERVICE_ID, "");
    EXPECT_NE(-1, PARAM_WATCHER_DISTRIBUTED_SERVICE_ID);
}

/**
 * @tc.name: OnAddSystemAbility003
 * @tc.desc: test OnAddSystemAbility, with deviceParamCollect_ is nullptr
 * @tc.type: FUNC
 * @tc.require: I7G775
 */

HWTEST_F(DeviceParamCollectTest, OnAddSystemAbility003, TestSize.Level3)
{
    sptr<IReport> report;
    sptr<DeviceParamCollect> deviceParamCollect = new DeviceParamCollect(report);
    sptr<SystemAbilityStatusChange> statusChangeListener = new SystemAbilityStatusChange();
    statusChangeListener->Init(deviceParamCollect);
    statusChangeListener->deviceParamCollect_ = nullptr;
    statusChangeListener->OnAddSystemAbility(PARAM_WATCHER_DISTRIBUTED_SERVICE_ID, "");
    EXPECT_EQ(statusChangeListener->deviceParamCollect_, nullptr);
}

/**
 * @tc.name: AddCollectEvent001
 * @tc.desc: test AddCollectEvent, with event
 * @tc.type: FUNC
 * @tc.require: I6UUNW
 */

HWTEST_F(DeviceParamCollectTest, AddCollectEvent001, TestSize.Level3)
{
    DTEST_LOG << "AddCollectEvent001 begin" << std::endl;
    sptr<IReport> report;
    std::shared_ptr<DeviceParamCollect> deviceParamCollect =
        std::make_shared<DeviceParamCollect>(report);
    OnDemandEvent event;
    int32_t ret = deviceParamCollect->AddCollectEvent(event);
    EXPECT_NE(ret, ERR_OK);
    DTEST_LOG << "AddCollectEvent001 end" << std::endl;
}

/**
 * @tc.name: RemoveUnusedEvent001
 * @tc.desc: test RemoveUnusedEvent, with event.name is not in params_
 * @tc.type: FUNC
 * @tc.require: I7VZ98
 */
HWTEST_F(DeviceParamCollectTest, RemoveUnusedEvent001, TestSize.Level3)
{
    sptr<IReport> report;
    std::shared_ptr<DeviceParamCollect> deviceParamCollect =
        std::make_shared<DeviceParamCollect>(report);
    OnDemandEvent event = {3, TEST_NAME, "true"};
    int32_t ret = deviceParamCollect->RemoveUnusedEvent(event);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: RemoveUnusedEvent002
 * @tc.desc: test RemoveUnusedEvent, with event.name in params_
 * @tc.type: FUNC
 * @tc.require: I7VZ98
 */
HWTEST_F(DeviceParamCollectTest, RemoveUnusedEvent002, TestSize.Level3)
{
    sptr<IReport> report;
    std::shared_ptr<DeviceParamCollect> deviceParamCollect =
        std::make_shared<DeviceParamCollect>(report);
    OnDemandEvent event = {3, TEST_NAME, "true"};
    deviceParamCollect->params_.insert(TEST_NAME);
    deviceParamCollect->RemoveUnusedEvent(event);
    EXPECT_NE(deviceParamCollect->params_.size(), 0);
}

/**
 * @tc.name: RemoveUnusedEvent003
 * @tc.desc: test RemoveUnusedEvent, with event.name in params_
 * @tc.type: FUNC
 * @tc.require: I7VZ98
 */
HWTEST_F(DeviceParamCollectTest, RemoveUnusedEvent003, TestSize.Level3)
{
    sptr<IReport> report;
    std::shared_ptr<DeviceParamCollect> deviceParamCollect =
        std::make_shared<DeviceParamCollect>(report);
    OnDemandEvent event = {3, TEST_NAME, "true"};
    deviceParamCollect->pendingParams_.insert(TEST_NAME);
    deviceParamCollect->WatchParameters();
    EXPECT_EQ(deviceParamCollect->params_.size(), 1);
    deviceParamCollect->RemoveUnusedEvent(event);
    EXPECT_EQ(deviceParamCollect->params_.size(), 0);
}

/**
 * @tc.name: CheckCondition001
 * @tc.desc: test CheckCondition, with condition
 * @tc.type: FUNC
 * @tc.require: I76X9Q
 */

HWTEST_F(DeviceParamCollectTest, CheckCondition001, TestSize.Level3)
{
    DTEST_LOG << "CheckCondition001 begin" << std::endl;
    sptr<IReport> report;
    std::shared_ptr<DeviceParamCollect> deviceParamCollect =
        std::make_shared<DeviceParamCollect>(report);
    sptr<DeviceParamCollect> collect = new DeviceParamCollect(report);
    sptr<SystemAbilityStatusChange> statusChangeListener = new SystemAbilityStatusChange();
    statusChangeListener->Init(collect);
    OnDemandCondition condition;
    condition.value = "on";
    condition.name = "test";
    int32_t saId = 123;
    std::string deviceId = "test_deviceId";
    bool ret = deviceParamCollect->CheckCondition(condition);
    statusChangeListener->OnRemoveSystemAbility(saId, deviceId);
    EXPECT_FALSE(ret);
    DTEST_LOG << "CheckCondition001 end" << std::endl;
}

/**
 * @tc.name: AddCollectEvent002
 * @tc.desc: test AddCollectEvent, with param is already existed
 * @tc.type: FUNC
 * @tc.require: I7FBV6
 */

HWTEST_F(DeviceParamCollectTest, AddCollectEvent002, TestSize.Level3)
{
    sptr<IReport> report;
    std::shared_ptr<DeviceParamCollect> deviceParamCollect =
        std::make_shared<DeviceParamCollect>(report);
    deviceParamCollect->params_.insert("test");
    OnDemandEvent event;
    event.eventId = PARAM;
    event.name = "test";
    int32_t ret = deviceParamCollect->AddCollectEvent(event);
    EXPECT_EQ(ret, ERR_OK);
}
}  // namespace OHOS