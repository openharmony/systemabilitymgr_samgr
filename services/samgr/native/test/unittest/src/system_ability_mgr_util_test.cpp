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
#include "system_ability_manager_util.h"
#include "sam_mock_permission.h"
#include "test_log.h"

using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS;

namespace OHOS {

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
}