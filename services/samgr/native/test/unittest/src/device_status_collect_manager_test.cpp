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

#include "device_status_collect_manager_test.h"

#include "sa_profiles.h"
#include "string_ex.h"
#include "test_log.h"

#define private public
#include "device_status_collect_manager.h"

using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS;

namespace OHOS {
namespace {
const std::string SA_TAG_DEVICE_ON_LINE = "deviceonline";
}

void DeviceStatusCollectManagerTest::SetUpTestCase()
{
    DTEST_LOG << "SetUpTestCase" << std::endl;
}

void DeviceStatusCollectManagerTest::TearDownTestCase()
{
    DTEST_LOG << "TearDownTestCase" << std::endl;
}

void DeviceStatusCollectManagerTest::SetUp()
{
    DTEST_LOG << "SetUp" << std::endl;
}

void DeviceStatusCollectManagerTest::TearDown()
{
    DTEST_LOG << "TearDown" << std::endl;
}

/**
 * @tc.name: FilterOnDemandSaProfiles001
 * @tc.desc: test FilterOnDemandSaProfiles with different parameters
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusCollectManagerTest, FilterOnDemandSaProfiles001, TestSize.Level3)
{
    DTEST_LOG << " FilterOnDemandSaProfiles001 BEGIN" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    std::list<SaProfile> saProfiles;
    collect->FilterOnDemandSaProfiles(saProfiles);
    EXPECT_EQ(true, collect->onDemandSaProfiles_.empty());
    SaProfile saProfile;
    OnDemandEvent event = { DEVICE_ONLINE, SA_TAG_DEVICE_ON_LINE, "on" };
    saProfile.startOnDemand.emplace_back(event);
    saProfiles.emplace_back(saProfile);
    collect->FilterOnDemandSaProfiles(saProfiles);
    EXPECT_EQ(false, collect->onDemandSaProfiles_.empty());
    OnDemandEvent event1 = { DEVICE_ONLINE, SA_TAG_DEVICE_ON_LINE, "off" };
    saProfile.stopOnDemand.emplace_back(event1);
    saProfiles.emplace_back(saProfile);
    collect->FilterOnDemandSaProfiles(saProfiles);
    EXPECT_EQ(false, collect->onDemandSaProfiles_.empty());
    DTEST_LOG << " FilterOnDemandSaProfiles001 END" << std::endl;
}

/**
 * @tc.name: GetSaControlListByEvent001
 * @tc.desc: test GetSaControlListByEvent with different parameters
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusCollectManagerTest, GetSaControlListByEvent001, TestSize.Level3)
{
    DTEST_LOG << " GetSaControlListByEvent001 BEGIN" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    OnDemandEvent event = { DEVICE_ONLINE, SA_TAG_DEVICE_ON_LINE, "on" };
    std::list<SaControlInfo> saControlList;
    collect->GetSaControlListByEvent(event, saControlList);
    EXPECT_EQ(true, saControlList.empty());
    SaProfile saProfile;
    OnDemandEvent event1 = { DEVICE_ONLINE, SA_TAG_DEVICE_ON_LINE, "on" };
    OnDemandEvent event2 = { DEVICE_ONLINE, SA_TAG_DEVICE_ON_LINE, "off" };
    saProfile.startOnDemand.emplace_back(event1);
    saProfile.stopOnDemand.emplace_back(event2);
    collect->onDemandSaProfiles_.emplace_back(saProfile);
    collect->GetSaControlListByEvent(event, saControlList);
    EXPECT_EQ(false, saControlList.empty());
    saControlList.clear();
    event.value = "off";
    collect->GetSaControlListByEvent(event, saControlList);
    EXPECT_EQ(false, saControlList.empty());
    saControlList.clear();
    event.value = "";
    collect->GetSaControlListByEvent(event, saControlList);
    EXPECT_EQ(true, saControlList.empty());
    event.name = "settingswitch";
    collect->GetSaControlListByEvent(event, saControlList);
    EXPECT_EQ(true, saControlList.empty());
    event.eventId = SETTING_SWITCH;
    collect->GetSaControlListByEvent(event, saControlList);
    EXPECT_EQ(true, saControlList.empty());
    DTEST_LOG << " GetSaControlListByEvent001 END" << std::endl;
}

/**
 * @tc.name: UnInit001
 * @tc.desc: test UnInit
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusCollectManagerTest, UnInit001, TestSize.Level3)
{
    DTEST_LOG << " UnInit001 BEGIN" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    collect->UnInit();
    std::list<SaProfile> saProfiles;
    collect->Init(saProfiles);
    collect->UnInit();
    EXPECT_EQ(true, collect->collectPluginMap_.empty());
    DTEST_LOG << " UnInit001 END" << std::endl;
}

/**
 * @tc.name: StartCollect001
 * @tc.desc: test StartCollect with empty collectHandler.
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusCollectManagerTest, StartCollect001, TestSize.Level3)
{
    DTEST_LOG << " StartCollect001 BEGIN" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    collect->StartCollect();
    EXPECT_EQ(nullptr, collect->collectHandler_);
    DTEST_LOG << " StartCollect001 END" << std::endl;
}

/**
 * @tc.name: ReportEvent001
 * @tc.desc: test ReportEvent, with empty collectHandler.
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusCollectManagerTest, ReportEvent001, TestSize.Level3)
{
    DTEST_LOG << " ReportEvent001 BEGIN" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    OnDemandEvent event;
    collect->ReportEvent(event);
    EXPECT_EQ(nullptr, collect->collectHandler_);
    DTEST_LOG << " ReportEvent001 END" << std::endl;
}

/**
 * @tc.name: ReportEvent002
 * @tc.desc: test ReportEvent, with empty saControlList.
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusCollectManagerTest, ReportEvent002, TestSize.Level3)
{
    DTEST_LOG << " ReportEvent002 BEGIN" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    std::list<SaProfile> saProfiles;
    collect->Init(saProfiles);
    OnDemandEvent event;
    collect->ReportEvent(event);
    EXPECT_EQ(true, collect->collectHandler_ != nullptr);
    DTEST_LOG << " ReportEvent002 END" << std::endl;
}

/**
 * @tc.name: ReportEvent003
 * @tc.desc: test ReportEvent, report success.
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusCollectManagerTest, ReportEvent003, TestSize.Level3)
{
    DTEST_LOG << " ReportEvent003 BEGIN" << std::endl;
    sptr<DeviceStatusCollectManager> collect = new DeviceStatusCollectManager();
    std::list<SaProfile> saProfiles;
    collect->Init(saProfiles);
    OnDemandEvent event = { DEVICE_ONLINE, SA_TAG_DEVICE_ON_LINE, "on" };
    std::list<SaControlInfo> saControlList;
    SaProfile saProfile;
    OnDemandEvent event1 = { DEVICE_ONLINE, SA_TAG_DEVICE_ON_LINE, "on" };
    OnDemandEvent event2 = { DEVICE_ONLINE, SA_TAG_DEVICE_ON_LINE, "off" };
    saProfile.startOnDemand.emplace_back(event1);
    saProfile.stopOnDemand.emplace_back(event2);
    collect->onDemandSaProfiles_.emplace_back(saProfile);
    collect->ReportEvent(event);
    EXPECT_EQ(true, collect->collectHandler_ != nullptr);
    DTEST_LOG << " ReportEvent003 END" << std::endl;
}
} // namespace OHOS
