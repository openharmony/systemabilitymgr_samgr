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

#include "device_networking_collect_test.h"

#include "device_status_collect_manager.h"
#include "sa_profiles.h"
#include "string_ex.h"
#include "test_log.h"

#define private public
#include "device_networking_collect.h"
#include "event_handler.h"

using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS;

namespace OHOS {
namespace {
constexpr uint32_t DM_DIED_EVENT = 11;
constexpr int32_t MAX_WAIT_TIME = 1000;
}

bool DeviceNetworkingCollectTest::isCaseDone_ = false;
std::mutex DeviceNetworkingCollectTest::caseDoneLock_;
std::condition_variable DeviceNetworkingCollectTest::caseDoneCondition_;
std::shared_ptr<AppExecFwk::EventHandler> collectHandler_ = nullptr;

void DeviceNetworkingCollectTest::SetUpTestCase()
{
    auto runner = AppExecFwk::EventRunner::Create("collect_test");
    collectHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    DTEST_LOG << "SetUpTestCase" << std::endl;
}

void DeviceNetworkingCollectTest::TearDownTestCase()
{
    DTEST_LOG << "TearDownTestCase" << std::endl;
}

void DeviceNetworkingCollectTest::SetUp()
{
    DTEST_LOG << "SetUp" << std::endl;
}

void DeviceNetworkingCollectTest::TearDown()
{
    DTEST_LOG << "TearDown" << std::endl;
}

/**
 * @tc.name: OnStart001
 * @tc.desc: test onStart
 * @tc.type: FUNC
 */
HWTEST_F(DeviceNetworkingCollectTest, OnStart001, TestSize.Level3)
{
    DTEST_LOG << " OnStart001 BEGIN" << std::endl;
    sptr<DeviceNetworkingCollect> networkingCollect = new DeviceNetworkingCollect(nullptr);
    networkingCollect->OnStart();
    auto callback = [networkingCollect] () {
        networkingCollect->OnStart();
    };
    collectHandler_->PostTask(callback);
    auto initDoneTask = []() {
        std::lock_guard<std::mutex> autoLock(caseDoneLock_);
        isCaseDone_ = true;
        caseDoneCondition_.notify_all();
    };
    collectHandler_->PostTask(initDoneTask);
    std::unique_lock<std::mutex> lock(caseDoneLock_);
    caseDoneCondition_.wait_for(lock, std::chrono::milliseconds(MAX_WAIT_TIME),
        [&] () { return isCaseDone_; });
    isCaseDone_ = false;
    OnDemandEvent event;
    networkingCollect->ReportEvent(event);
    networkingCollect->UpdateDeviceOnlineSet("deviceId");
    networkingCollect->IsOnline();
    networkingCollect->ClearDeviceOnlineSet();
    networkingCollect->stateCallback_ = nullptr;
    networkingCollect->ReportEvent(event);
    networkingCollect->UpdateDeviceOnlineSet("deviceId");
    networkingCollect->IsOnline();
    networkingCollect->ClearDeviceOnlineSet();
    networkingCollect->AddDeviceChangeListener();
    EXPECT_EQ(true, networkingCollect->initCallback_ != nullptr);
    networkingCollect->initCallback_ = nullptr;
    int32_t ret = networkingCollect->AddDeviceChangeListener();
    EXPECT_EQ(true, networkingCollect->initCallback_ == nullptr);
    EXPECT_EQ(false, ret);
    DTEST_LOG << " OnStart001 END" << std::endl;
}

/**
 * @tc.name: OnStart002
 * @tc.desc: test onStart
 * @tc.type: FUNC
 */
HWTEST_F(DeviceNetworkingCollectTest, OnStart002, TestSize.Level3)
{
    DTEST_LOG << " OnStart002 BEGIN" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceNetworkingCollect> networkingCollect = new DeviceNetworkingCollect(collect);
    auto callback = [networkingCollect] () {
        networkingCollect->OnStart();
    };
    collectHandler_->PostTask(callback);
    auto initDoneTask = []() {
        std::lock_guard<std::mutex> autoLock(caseDoneLock_);
        isCaseDone_ = true;
        caseDoneCondition_.notify_all();
    };
    collectHandler_->PostTask(initDoneTask);
    std::unique_lock<std::mutex> lock(caseDoneLock_);
    caseDoneCondition_.wait_for(lock, std::chrono::milliseconds(MAX_WAIT_TIME),
        [&] () { return isCaseDone_; });
    isCaseDone_ = false;
    OnDemandEvent event;
    networkingCollect->ReportEvent(event);
    EXPECT_EQ(true, networkingCollect->initCallback_ != nullptr);
    networkingCollect->workHandler_ = nullptr;
    networkingCollect->OnStop();
    DTEST_LOG << " OnStart002 END" << std::endl;
}

/**
 * @tc.name: OnRemoteDied001
 * @tc.desc: test when dm is died
 * @tc.type: FUNC
 */
HWTEST_F(DeviceNetworkingCollectTest, OnRemoteDied001, TestSize.Level3)
{
    DTEST_LOG << " OnRemoteDied001 BEGIN" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceNetworkingCollect> networkingCollect = new DeviceNetworkingCollect(collect);
    auto callback = [networkingCollect] () {
        networkingCollect->OnStart();
    };
    collectHandler_->PostTask(callback);
    auto initDoneTask = []() {
        std::lock_guard<std::mutex> autoLock(caseDoneLock_);
        isCaseDone_ = true;
        caseDoneCondition_.notify_all();
    };
    collectHandler_->PostTask(initDoneTask);
    std::unique_lock<std::mutex> lock(caseDoneLock_);
    caseDoneCondition_.wait_for(lock, std::chrono::milliseconds(MAX_WAIT_TIME),
        [&] () { return isCaseDone_; });
    isCaseDone_ = false;
    auto initCallback = std::static_pointer_cast<DeviceInitCallBack>(networkingCollect->initCallback_);
    initCallback->OnRemoteDied();
    initCallback->handler_ = nullptr;
    initCallback->OnRemoteDied();
    EXPECT_EQ(true, initCallback != nullptr);
    EXPECT_EQ(true, initCallback->handler_ == nullptr);
    networkingCollect->OnStop();
    DTEST_LOG << " OnRemoteDied001 END" << std::endl;
}

/**
 * @tc.name: OnDeviceOnline001
 * @tc.desc: test OnDeviceOnline
 * @tc.type: FUNC
 */
HWTEST_F(DeviceNetworkingCollectTest, OnDeviceOnline001, TestSize.Level3)
{
    DTEST_LOG << " OnDeviceOnline001 BEGIN" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceNetworkingCollect> networkingCollect = new DeviceNetworkingCollect(collect);
    auto callback = [networkingCollect] () {
        networkingCollect->OnStart();
    };
    collectHandler_->PostTask(callback);
    auto initDoneTask = []() {
        std::lock_guard<std::mutex> autoLock(caseDoneLock_);
        isCaseDone_ = true;
        caseDoneCondition_.notify_all();
    };
    collectHandler_->PostTask(initDoneTask);
    std::unique_lock<std::mutex> lock(caseDoneLock_);
    caseDoneCondition_.wait_for(lock, std::chrono::milliseconds(MAX_WAIT_TIME),
        [&] () { return isCaseDone_; });
    isCaseDone_ = false;
    DistributedHardware::DmDeviceInfo dmDeviceInfo = {
        .deviceId = "asdad",
        .deviceName = "asda",
        .deviceTypeId = 1,
    };
    networkingCollect->stateCallback_->OnDeviceOnline(dmDeviceInfo);
    EXPECT_EQ(true, networkingCollect->IsOnline());
    networkingCollect->stateCallback_->OnDeviceOnline(dmDeviceInfo);
    networkingCollect->stateCallback_->collect_ = nullptr;
    networkingCollect->ClearDeviceOnlineSet();
    networkingCollect->stateCallback_->OnDeviceOnline(dmDeviceInfo);
    EXPECT_EQ(true, networkingCollect->IsOnline());
    DTEST_LOG << " OnDeviceOnline001 END" << std::endl;
}

/**
 * @tc.name: OnDeviceOffline001
 * @tc.desc: test OnDeviceOffline
 * @tc.type: FUNC
 */
HWTEST_F(DeviceNetworkingCollectTest, OnDeviceOffline001, TestSize.Level3)
{
    DTEST_LOG << " OnDeviceOffline001 BEGIN" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceNetworkingCollect> networkingCollect = new DeviceNetworkingCollect(collect);
    auto callback = [networkingCollect] () {
        networkingCollect->OnStart();
    };
    collectHandler_->PostTask(callback);
    auto initDoneTask = []() {
        std::lock_guard<std::mutex> autoLock(caseDoneLock_);
        isCaseDone_ = true;
        caseDoneCondition_.notify_all();
    };
    collectHandler_->PostTask(initDoneTask);
    std::unique_lock<std::mutex> lock(caseDoneLock_);
    caseDoneCondition_.wait_for(lock, std::chrono::milliseconds(MAX_WAIT_TIME),
        [&] () { return isCaseDone_; });
    isCaseDone_ = false;
    DistributedHardware::DmDeviceInfo dmDeviceInfo = {
        .deviceId = "asdad",
        .deviceName = "asda",
        .deviceTypeId = 1,
    };
    networkingCollect->stateCallback_->OnDeviceOffline(dmDeviceInfo);
    networkingCollect->stateCallback_->collect_ = nullptr;
    networkingCollect->stateCallback_->OnDeviceOffline(dmDeviceInfo);
    EXPECT_EQ(true, !networkingCollect->IsOnline());
    DTEST_LOG << " OnDeviceOffline001 END" << std::endl;
}

/**
 * @tc.name: ProcessEvent001
 * @tc.desc: test ProcessEvent, with error param.
 * @tc.type: FUNC
 */
HWTEST_F(DeviceNetworkingCollectTest, ProcessEvent001, TestSize.Level3)
{
    DTEST_LOG << " ProcessEvent001 BEGIN" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    sptr<DeviceNetworkingCollect> networkingCollect = new DeviceNetworkingCollect(collect);
    auto callback = [networkingCollect] () {
        networkingCollect->OnStart();
    };
    collectHandler_->PostTask(callback);
    auto initDoneTask = []() {
        std::lock_guard<std::mutex> autoLock(caseDoneLock_);
        isCaseDone_ = true;
        caseDoneCondition_.notify_all();
    };
    collectHandler_->PostTask(initDoneTask);
    std::unique_lock<std::mutex> lock(caseDoneLock_);
    caseDoneCondition_.wait_for(lock, std::chrono::milliseconds(MAX_WAIT_TIME),
        [&] () { return isCaseDone_; });
    isCaseDone_ = false;
    int32_t ret = networkingCollect->workHandler_->SendEvent(DM_DIED_EVENT + 1);
    EXPECT_EQ(true, ret);
    auto workHandler = std::static_pointer_cast<WorkHandler>(networkingCollect->workHandler_);
    workHandler->collect_ = nullptr;
    ret = networkingCollect->workHandler_->SendEvent(DM_DIED_EVENT + 1);
    EXPECT_EQ(true, ret);
    ret = networkingCollect->workHandler_->SendEvent(DM_DIED_EVENT);
    EXPECT_EQ(true, ret);
    DTEST_LOG << " ProcessEvent001 END" << std::endl;
}
} // namespace OHOS
