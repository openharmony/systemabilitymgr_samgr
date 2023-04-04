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
#include "local_abilitys_test.h"

#include "local_abilitys.h"
#include "mock_iro_sendrequest.h"
#include "test_log.h"

using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS;

namespace OHOS {
namespace {
constexpr uint32_t SAID = 1499;
}
void LocalAbilitysTest::SetUpTestCase()
{
    DTEST_LOG << "SetUpTestCase" << std::endl;
}

void LocalAbilitysTest::TearDownTestCase()
{
    DTEST_LOG << "TearDownTestCase" << std::endl;
}

void LocalAbilitysTest::SetUp()
{
    DTEST_LOG << "SetUp" << std::endl;
}

void LocalAbilitysTest::TearDown()
{
    DTEST_LOG << "TearDown" << std::endl;
}

/**
 * @tc.name: AddAbility001
 * @tc.desc: AddAbility and check ability
 * @tc.type: FUNC
 * @tc.require: I6EOBI
 */
HWTEST_F(LocalAbilitysTest, AddAbility001, TestSize.Level1)
{
    sptr<IRemoteObject> testAbility(new MockIroSendrequesteStub());
    LocalAbilitys::GetInstance().AddAbility(SAID, testAbility);
    auto proxy = LocalAbilitys::GetInstance().GetAbility(SAID);
    EXPECT_TRUE(proxy != nullptr);
}

/**
 * @tc.name: RemoveAbility001
 * @tc.desc: RemoveAbility and check ability
 * @tc.type: FUNC
 * @tc.require: I6EOBI
 */
HWTEST_F(LocalAbilitysTest, RemoveAbility001, TestSize.Level1)
{
    sptr<IRemoteObject> testAbility(new MockIroSendrequesteStub());
    LocalAbilitys::GetInstance().AddAbility(SAID, testAbility);
    LocalAbilitys::GetInstance().RemoveAbility(SAID);
    auto proxy = LocalAbilitys::GetInstance().GetAbility(SAID);
    EXPECT_TRUE(proxy == nullptr);
}
}