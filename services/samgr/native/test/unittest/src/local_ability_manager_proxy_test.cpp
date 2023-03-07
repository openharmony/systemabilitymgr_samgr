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
#include "local_ability_manager_proxy_test.h"

#include "itest_transaction_service.h"
#include "local_ability_manager_proxy.h"
#include "mock_iro_sendrequest.h"
#include "string_ex.h"
#include "test_log.h"

#define private public

using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS;

namespace OHOS {
namespace {
constexpr int32_t TEST_SAID_INVAILD = -1;
constexpr int32_t TEST_SAID_VAILD = 9999;
}
void LocalAbilityManagerProxyTest::SetUpTestCase()
{
    DTEST_LOG << "SetUpTestCase" << std::endl;
}

void LocalAbilityManagerProxyTest::TearDownTestCase()
{
    DTEST_LOG << "TearDownTestCase" << std::endl;
}

void LocalAbilityManagerProxyTest::SetUp()
{
    DTEST_LOG << "SetUp" << std::endl;
}

void LocalAbilityManagerProxyTest::TearDown()
{
    DTEST_LOG << "TearDown" << std::endl;
}

/**
 * @tc.name: LocalAbilityManagerProxy001
 * @tc.desc: LocalAbilityManagerProxy and check StartAbility
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(LocalAbilityManagerProxyTest, LocalAbilityManagerProxy001, TestSize.Level1)
{
    sptr<IRemoteObject> testAbility(new TestTransactionService());
    sptr<LocalAbilityManagerProxy> localAbility(new LocalAbilityManagerProxy(testAbility));
    string eventStr = "name:usual.event.SCREEN_ON,said:1499,type:4,value:";
    bool res = localAbility->StartAbility(TEST_SAID_INVAILD, eventStr);
    EXPECT_EQ(res, false);
}

/**
 * @tc.name: LocalAbilityManagerProxy002
 * @tc.desc: LocalAbilityManagerProxy and check StartAbility
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(LocalAbilityManagerProxyTest, LocalAbilityManagerProxy002, TestSize.Level1)
{
    sptr<LocalAbilityManagerProxy> localAbility(new LocalAbilityManagerProxy(nullptr));
    string eventStr = "name:usual.event.SCREEN_ON,said:1499,type:4,value:";
    bool res = localAbility->StartAbility(TEST_SAID_VAILD, eventStr);
    EXPECT_EQ(res, false);
}

/**
 * @tc.name: LocalAbilityManagerProxy003
 * @tc.desc: LocalAbilityManagerProxy and check StartAbility
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(LocalAbilityManagerProxyTest, LocalAbilityManagerProxy003, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<LocalAbilityManagerProxy> localAbility(new LocalAbilityManagerProxy(testAbility));
    string eventStr = "name:usual.event.SCREEN_ON,said:1499,type:4,value:";
    bool res = localAbility->StartAbility(TEST_SAID_VAILD, eventStr);
    EXPECT_EQ(res, true);
}

/**
 * @tc.name: LocalAbilityManagerProxy004
 * @tc.desc: LocalAbilityManagerProxy and check StartAbility
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(LocalAbilityManagerProxyTest, LocalAbilityManagerProxy004, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    testAbility->result_ = 1;
    sptr<LocalAbilityManagerProxy> localAbility(new LocalAbilityManagerProxy(testAbility));
    EXPECT_NE(localAbility, nullptr);
    string eventStr = "name:usual.event.SCREEN_ON,said:1499,type:4,value:";
    bool res = localAbility->StartAbility(TEST_SAID_VAILD, eventStr);
    EXPECT_EQ(res, false);
}

/**
 * @tc.name: StopAbility001
 * @tc.desc: test StopAbility, said is invalid
 * @tc.type: FUNC
 * @tc.require: I6J4T7
 */
HWTEST_F(LocalAbilityManagerProxyTest, StopAbility001, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    testAbility->result_ = 1;
    sptr<LocalAbilityManagerProxy> localAbility(new LocalAbilityManagerProxy(testAbility));
    EXPECT_NE(localAbility, nullptr);
    string eventStr = "name:usual.event.SCREEN_ON,said:1499,type:4,value:";
    bool res = localAbility->StopAbility(TEST_SAID_INVAILD, eventStr);
    EXPECT_EQ(res, false);
}

/**
 * @tc.name: StopAbility002
 * @tc.desc: test StopAbility, eventStr is empty
 * @tc.type: FUNC
 * @tc.require: I6J4T7
 */
HWTEST_F(LocalAbilityManagerProxyTest, StopAbility002, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    testAbility->result_ = 1;
    sptr<LocalAbilityManagerProxy> localAbility(new LocalAbilityManagerProxy(testAbility));
    EXPECT_NE(localAbility, nullptr);
    string eventStr = "";
    bool res = localAbility->StopAbility(TEST_SAID_VAILD, eventStr);
    EXPECT_EQ(res, false);
}

/**
 * @tc.name: StopAbility003
 * @tc.desc: test StopAbility,return success
 * @tc.type: FUNC
 * @tc.require: I6J4T7
 */
HWTEST_F(LocalAbilityManagerProxyTest, StopAbility003, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<LocalAbilityManagerProxy> localAbility(new LocalAbilityManagerProxy(testAbility));
    EXPECT_NE(localAbility, nullptr);
    string eventStr = "name:usual.event.SCREEN_ON,said:1499,type:4,value:";
    bool res = localAbility->StopAbility(TEST_SAID_VAILD, eventStr);
    EXPECT_EQ(res, true);
}

/**
 * @tc.name: StopAbility004
 * @tc.desc: test StopAbility, return failed
 * @tc.type: FUNC
 * @tc.require: I6J4T7
 */
HWTEST_F(LocalAbilityManagerProxyTest, StopAbility004, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    testAbility->result_ = 1;
    sptr<LocalAbilityManagerProxy> localAbility(new LocalAbilityManagerProxy(testAbility));
    EXPECT_NE(localAbility, nullptr);
    string eventStr = "name:usual.event.SCREEN_ON,said:1499,type:4,value:";
    bool res = localAbility->StopAbility(TEST_SAID_VAILD, eventStr);
    EXPECT_EQ(res, false);
}

/**
 * @tc.name: ActiveAbility001
 * @tc.desc: test ActiveAbility, said is invalid
 * @tc.type: FUNC
 * @tc.require: I6J4T7
 */
HWTEST_F(LocalAbilityManagerProxyTest, ActiveAbility001, TestSize.Level3)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<LocalAbilityManagerProxy> localAbility(new LocalAbilityManagerProxy(testAbility));
    EXPECT_NE(localAbility, nullptr);
    std::unordered_map<std::string, std::string> activeReason;
    bool res = localAbility->ActiveAbility(TEST_SAID_INVAILD, activeReason);
    EXPECT_EQ(res, false);
}

/**
 * @tc.name: ActiveAbility002
 * @tc.desc: test ActiveAbility, said is valid
 * @tc.type: FUNC
 * @tc.require: I6J4T7
 */
HWTEST_F(LocalAbilityManagerProxyTest, ActiveAbility002, TestSize.Level3)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<LocalAbilityManagerProxy> localAbility(new LocalAbilityManagerProxy(testAbility));
    EXPECT_NE(localAbility, nullptr);
    std::unordered_map<std::string, std::string> activeReason;
    bool res = localAbility->ActiveAbility(TEST_SAID_VAILD, activeReason);
    EXPECT_EQ(res, false);
}
}