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
#include "system_ability_mgr_proxy_test.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "itest_transaction_service.h"
#include "sa_status_change_mock.h"
#include "string_ex.h"
#include "system_ability_manager_proxy.h"
#include "test_log.h"

#define private public

using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS;

namespace OHOS {
namespace {
constexpr int32_t TEST_ID_VAILD = 9999;
constexpr int32_t TEST_ID_INVAILD = 9990;
}
void SystemAbilityMgrProxyTest::SetUpTestCase()
{
    DTEST_LOG << "SetUpTestCase" << std::endl;
}

void SystemAbilityMgrProxyTest::TearDownTestCase()
{
    DTEST_LOG << "TearDownTestCase" << std::endl;
}

void SystemAbilityMgrProxyTest::SetUp()
{
    DTEST_LOG << "SetUp" << std::endl;
}

void SystemAbilityMgrProxyTest::TearDown()
{
    DTEST_LOG << "TearDown" << std::endl;
}

/**
 * @tc.name: GetRegistryRemoteObject001 test
 * @tc.desc: test for get RegistryRemoteObject
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, GetRegistryRemoteObject001, TestSize.Level1)
{
    sptr<IRemoteObject> remote = SystemAbilityManagerClient::GetInstance().GetRegistryRemoteObject();
    EXPECT_EQ(remote, nullptr);
}

/**
 * @tc.name: AddSystemProcess001
 * @tc.desc: check add process remoteobject
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, AddSystemProcess001, TestSize.Level1)
{
    DTEST_LOG << " AddSystemProcess001 start " << std::endl;
    /**
     * @tc.steps: step1. get samgr instance
     * @tc.expected: step1. samgr instance not nullptr
     */
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    sptr<IRemoteObject> remote(new TestTransactionService());
    EXPECT_NE(remote, nullptr);
    u16string procName(u"");
    int32_t ret = sm->AddSystemProcess(procName, remote);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AddSystemProcess002
 * @tc.desc: check add process remoteobject
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, AddSystemProcess002, TestSize.Level1)
{
    DTEST_LOG << " AddSystemProcess002 start " << std::endl;
    /**
     * @tc.steps: step1. get samgr instance
     * @tc.expected: step1. samgr instance not nullptr
     */
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    sptr<IRemoteObject> remote(new TestTransactionService());
    EXPECT_NE(remote, nullptr);
    u16string procName(u"test_process");
    int32_t ret = sm->AddSystemProcess(procName, remote);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: UnSubscribeSystemAbility001
 * @tc.desc: check UnSubscribeSystemAbility
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, UnSubscribeSystemAbility001, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    sptr<SaStatusChangeMock> callback(new SaStatusChangeMock());
    sm->SubscribeSystemAbility(TEST_ID_VAILD, callback);
    int32_t res = sm->UnSubscribeSystemAbility(TEST_ID_VAILD, callback);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: UnSubscribeSystemAbility002
 * @tc.desc: check UnSubscribeSystemAbility
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, UnSubscribeSystemAbility002, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    sptr<SaStatusChangeMock> callback(new SaStatusChangeMock());
    sm->SubscribeSystemAbility(TEST_ID_VAILD, callback);
    int32_t res = sm->UnSubscribeSystemAbility(TEST_ID_INVAILD, callback);
    EXPECT_EQ(res, ERR_OK);
}
}
