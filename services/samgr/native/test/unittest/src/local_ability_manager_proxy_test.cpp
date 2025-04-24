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
const std::string TEST_STRING = "test";
const std::string EVENT_NAME = "name";
const std::string EVENT_ID = "eventId";
const std::string EVENT_STR = "name:usual.event.SCREEN_ON,said:1499,type:4,value:";
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
    bool res = localAbility->StartAbility(TEST_SAID_INVAILD, EVENT_STR);
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
    bool res = localAbility->StartAbility(TEST_SAID_VAILD, EVENT_STR);
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
    bool res = localAbility->StartAbility(TEST_SAID_VAILD, EVENT_STR);
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
    bool res = localAbility->StartAbility(TEST_SAID_VAILD, EVENT_STR);
    EXPECT_EQ(res, false);
}

/**
 * @tc.name: StartAbility001
 * @tc.desc: test StartAbility with eventStr is empty
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(LocalAbilityManagerProxyTest, StartAbility001, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<LocalAbilityManagerProxy> localAbility(new LocalAbilityManagerProxy(testAbility));
    string eventStr = "";
    bool ret = localAbility->StartAbility(TEST_SAID_VAILD, eventStr);
    EXPECT_FALSE(ret);
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
    bool res = localAbility->StopAbility(TEST_SAID_INVAILD, EVENT_STR);
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
    bool res = localAbility->StopAbility(TEST_SAID_VAILD, EVENT_STR);
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
    bool res = localAbility->StopAbility(TEST_SAID_VAILD, EVENT_STR);
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
    nlohmann::json activeReason;
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
    nlohmann::json activeReason;
    bool res = localAbility->ActiveAbility(TEST_SAID_VAILD, activeReason);
    EXPECT_EQ(res, false);
}

/**
 * @tc.name: ActiveAbility003
 * @tc.desc: test ActiveAbility with activeReason is not empty
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(LocalAbilityManagerProxyTest, ActiveAbility003, TestSize.Level3)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<LocalAbilityManagerProxy> localAbility(new LocalAbilityManagerProxy(testAbility));
    nlohmann::json activeReason;
    activeReason[EVENT_ID] = TEST_STRING;
    activeReason[EVENT_NAME] = TEST_STRING;
    bool ret = localAbility->ActiveAbility(TEST_SAID_VAILD, activeReason);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IdleAbility001
 * @tc.desc: test IdleAbility with SaID is invalid
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(LocalAbilityManagerProxyTest, IdleAbility001, TestSize.Level3)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<LocalAbilityManagerProxy> localAbility(new LocalAbilityManagerProxy(testAbility));
    nlohmann::json idleReason;
    int32_t delayTime = 0;
    bool ret = localAbility->IdleAbility(TEST_SAID_INVAILD, idleReason, delayTime);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IdleAbility002
 * @tc.desc: test IdleAbility001 with idleReason is not empty
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(LocalAbilityManagerProxyTest, IdleAbility002, TestSize.Level3)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<LocalAbilityManagerProxy> localAbility(new LocalAbilityManagerProxy(testAbility));
    nlohmann::json idleReason;
    idleReason[EVENT_ID] = TEST_STRING;
    idleReason[EVENT_NAME] = TEST_STRING;
    int32_t delayTime = 0;
    bool ret = localAbility->IdleAbility(TEST_SAID_VAILD, idleReason, delayTime);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: SendStrategyToSA001
 * @tc.desc: test SendStrategyToSA001 with idleReason is not empty
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(LocalAbilityManagerProxyTest, SendStrategyToSA001, TestSize.Level3)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<LocalAbilityManagerProxy> localAbility(new LocalAbilityManagerProxy(testAbility));
    int32_t type = 0;
    int32_t level = 0;
    std::string action;
    bool ret = localAbility->SendStrategyToSA(type, TEST_SAID_INVAILD, level, action);
    EXPECT_FALSE(ret);
    ret = localAbility->SendStrategyToSA(type, 1, level, action);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: IpcStatCmdProc001
 * @tc.desc: test IpcStatCmdProc001 with fd valid and cmd valid
 * @tc.type: FUNC
 * @tc.require: I9DR69
 */
HWTEST_F(LocalAbilityManagerProxyTest, IpcStatCmdProc001, TestSize.Level3)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<LocalAbilityManagerProxy> localAbility(new LocalAbilityManagerProxy(testAbility));
    int32_t fd = 1;
    int32_t cmd = 0;
    std::string action;
    bool ret = localAbility->IpcStatCmdProc(fd, cmd);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IpcStatCmdProc002
 * @tc.desc: test IpcStatCmdProc002 with fd invalid and cmd valid
 * @tc.type: FUNC
 * @tc.require: I9DR69
 */
HWTEST_F(LocalAbilityManagerProxyTest, IpcStatCmdProc002, TestSize.Level3)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<LocalAbilityManagerProxy> localAbility(new LocalAbilityManagerProxy(testAbility));
    int32_t fd = -1;
    int32_t cmd = 0;
    std::string action;
    bool ret = localAbility->IpcStatCmdProc(fd, cmd);
    EXPECT_FALSE(ret);
}


/**
 * @tc.name: IpcStatCmdProc003
 * @tc.desc: test IpcStatCmdProc003 with fd invalid and cmd valid
 * @tc.type: FUNC
 * @tc.require: I9DR69
 */
HWTEST_F(LocalAbilityManagerProxyTest, IpcStatCmdProc003, TestSize.Level3)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<LocalAbilityManagerProxy> localAbility(new LocalAbilityManagerProxy(testAbility));
    int32_t fd = -1;
    int32_t cmd = -1;
    std::string action;
    bool ret = localAbility->IpcStatCmdProc(fd, cmd);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: FfrtDumperProc001
 * @tc.desc: test FfrtDumperProc
 * @tc.type: FUNC
 * @tc.require: I9DR69
 */
HWTEST_F(LocalAbilityManagerProxyTest, FfrtDumperProc001, TestSize.Level3)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<LocalAbilityManagerProxy> localAbility(new LocalAbilityManagerProxy(testAbility));
    std::string ffrtDumperInfo;
    bool ret = localAbility->FfrtDumperProc(ffrtDumperInfo);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: SystemAbilityExtProc001
 * @tc.desc: test SystemAbilityExtProc001
 * @tc.type: FUNC
 * @tc.require: I9IE98
 */
HWTEST_F(LocalAbilityManagerProxyTest, SystemAbilityExtProc001, TestSize.Level3)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<LocalAbilityManagerProxy> localAbility(new LocalAbilityManagerProxy(testAbility));
    SystemAbilityExtensionPara callback;

    int32_t result = localAbility->SystemAbilityExtProc(std::string(), TEST_SAID_VAILD, &callback);
    EXPECT_EQ(result, INVALID_DATA);
}

/**
 * @tc.name: SystemAbilityExtProc002
 * @tc.desc: test SystemAbilityExtProc002
 * @tc.type: FUNC
 * @tc.require: I9IE98
 */
HWTEST_F(LocalAbilityManagerProxyTest, SystemAbilityExtProc002, TestSize.Level3)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<LocalAbilityManagerProxy> localAbility(new LocalAbilityManagerProxy(testAbility));
    SystemAbilityExtensionPara callback;

    int32_t result = localAbility->SystemAbilityExtProc("OnBackup", TEST_SAID_VAILD, &callback);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: SystemAbilityExtProc003
 * @tc.desc: test SystemAbilityExtProc003
 * @tc.type: FUNC
 * @tc.require: I9IE98
 */
HWTEST_F(LocalAbilityManagerProxyTest, SystemAbilityExtProc003, TestSize.Level3)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<LocalAbilityManagerProxy> localAbility(new LocalAbilityManagerProxy(testAbility));
    SystemAbilityExtensionPara callback;

    int32_t result = localAbility->SystemAbilityExtProc("OnBackup", TEST_SAID_VAILD, &callback, true);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: SystemAbilityExtProc004
 * @tc.desc: test SystemAbilityExtProc004
 * @tc.type: FUNC
 * @tc.require: I9IE98
 */
HWTEST_F(LocalAbilityManagerProxyTest, SystemAbilityExtProc004, TestSize.Level3)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<LocalAbilityManagerProxy> localAbility(new LocalAbilityManagerProxy(testAbility));
    SystemAbilityExtensionPara callback;

    int32_t result = localAbility->SystemAbilityExtProc("OnBackup", TEST_SAID_INVAILD, &callback, true);
    EXPECT_EQ(result, INVALID_DATA);
}
}