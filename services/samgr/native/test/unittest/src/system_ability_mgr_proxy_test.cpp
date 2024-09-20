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
#include "samgr_err_code.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "itest_transaction_service.h"
#include "mock_iro_sendrequest.h"
#include "sam_mock_permission.h"
#include "sa_status_change_mock.h"
#include "system_ability_load_callback_proxy.h"
#include "system_ability_status_change_proxy.h"
#include "string_ex.h"
#include "test_log.h"

#define private public
#include "system_ability_manager_proxy.h"

using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS;
namespace OHOS {
namespace {
std::u16string SAMANAGER_INTERFACE_TOKEN = u"OHOS.ISystemProcessStatusChange";
constexpr int RESULT = 0;
constexpr int32_t TEST_ID_NORANGE_SAID = -1;
constexpr int32_t TEST_ID_VAILD = 9999;
constexpr int32_t TEST_ID_INVAILD = 9990;
constexpr int32_t TEST_SAID_INVALID = 54321;
constexpr int64_t EXTRA_DATA_ID = 1;
}
void SystemProcessStatusChange::OnSystemProcessStarted(SystemProcessInfo& systemProcessInfo)
{
    cout << "OnSystemProcessStarted, processName: ";
}

void SystemProcessStatusChange::OnSystemProcessStopped(SystemProcessInfo& systemProcessInfo)
{
    cout << "OnSystemProcessStopped, processName: ";
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
    SamMockPermission::MockPermission();
    DTEST_LOG << "SetUp" << std::endl;
}

void SystemAbilityMgrProxyTest::TearDown()
{
    DTEST_LOG << "TearDown" << std::endl;
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
 * @tc.name: CheckSystemAbility001
 * @tc.desc: CheckSystemAbility, systemAbilityId: invalid!
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, CheckSystemAbility001, TestSize.Level1)
{
    DTEST_LOG << " CheckSystemAbility001 start " << std::endl;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    sptr<IRemoteObject> ret = sm->CheckSystemAbility(TEST_ID_NORANGE_SAID);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: CheckSystemAbility002
 * @tc.desc: CheckSystemAbility, CheckSystemAbility:systemAbilityId or deviceId is nullptr
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, CheckSystemAbility002, TestSize.Level1)
{
    DTEST_LOG << " CheckSystemAbility002 start " << std::endl;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    std::string deviceId = "";
    sptr<IRemoteObject> ret = sm->CheckSystemAbility(TEST_ID_NORANGE_SAID, deviceId);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: CheckSystemAbility003
 * @tc.desc: CheckSystemAbility, CheckSystemAbility:systemAbilityId or deviceId is nullptr
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, CheckSystemAbility003, TestSize.Level1)
{
    DTEST_LOG << " CheckSystemAbility003 start " << std::endl;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    std::string deviceId = "";
    sptr<IRemoteObject> ret = sm->CheckSystemAbility(TEST_ID_VAILD, deviceId);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: CheckSystemAbility004
 * @tc.desc: CheckSystemAbility, CheckSystemAbility:systemAbilityId  invalid!
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, CheckSystemAbility004, TestSize.Level1)
{
    DTEST_LOG << " CheckSystemAbility004 start " << std::endl;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    bool isExist = true;
    sptr<IRemoteObject> ret = sm->CheckSystemAbility(TEST_SAID_INVALID, isExist);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: CheckSystemAbility005
 * @tc.desc: CheckSystemAbility, CheckSystemAbility:systemAbilityId  invalid!
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, CheckSystemAbility005, TestSize.Level1)
{
    DTEST_LOG << " CheckSystemAbility005 start " << std::endl;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    bool isExist = true;
    sptr<IRemoteObject> ret = sm->CheckSystemAbility(TEST_ID_NORANGE_SAID, isExist);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: AddOnDemandSystemAbilityInfo001
 * @tc.desc: AddOnDemandSystemAbilityInfo, AddOnDemandSystemAbilityInfo invalid params
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, AddOnDemandSystemAbilityInfo001, TestSize.Level1)
{
    DTEST_LOG << " AddOnDemandSystemAbilityInfo001 start " << std::endl;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    std::u16string localAbilityManagerName = u"";
    int32_t ret = sm->AddOnDemandSystemAbilityInfo(TEST_ID_NORANGE_SAID, localAbilityManagerName);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AddOnDemandSystemAbilityInfo002
 * @tc.desc: AddOnDemandSystemAbilityInfo, AddOnDemandSystemAbilityInfo invalid params
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, AddOnDemandSystemAbilityInfo002, TestSize.Level1)
{
    DTEST_LOG << " AddOnDemandSystemAbilityInfo002 start " << std::endl;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    std::u16string localAbilityManagerName = u"";
    int32_t ret = sm->AddOnDemandSystemAbilityInfo(TEST_ID_VAILD, localAbilityManagerName);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: SubscribeSystemAbility001
 * @tc.desc: SubscribeSystemAbility, SubscribeSystemAbility systemAbilityId: or listener invalid!
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, SubscribeSystemAbility001, TestSize.Level1)
{
    DTEST_LOG << " SubscribeSystemAbility001 start " << std::endl;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    int32_t ret = sm->SubscribeSystemAbility(TEST_ID_NORANGE_SAID, nullptr);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: SubscribeSystemAbility002
 * @tc.desc: SubscribeSystemAbility, SubscribeSystemAbility systemAbilityId: or listener invalid!
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, SubscribeSystemAbility002, TestSize.Level1)
{
    DTEST_LOG << " SubscribeSystemAbility002 start " << std::endl;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    int32_t ret = sm->SubscribeSystemAbility(TEST_ID_VAILD, nullptr);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
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

/**
 * @tc.name: UnSubscribeSystemAbility003
 * @tc.desc: check UnSubscribeSystemAbility
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, UnSubscribeSystemAbility003, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    sptr<SaStatusChangeMock> callback(new SaStatusChangeMock());
    int32_t res = sm->UnSubscribeSystemAbility(TEST_ID_NORANGE_SAID, callback);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: UnSubscribeSystemAbility004
 * @tc.desc: check UnSubscribeSystemAbility
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, UnSubscribeSystemAbility004, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    int32_t res = sm->UnSubscribeSystemAbility(TEST_ID_VAILD, nullptr);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: LoadSystemAbility001
 * @tc.desc: check LoadSystemAbility
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, LoadSystemAbility001, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    std::string deviceId = "deviceId";
    sptr<SystemAbilityLoadCallbackMock> callback(new SystemAbilityLoadCallbackMock());
    int32_t res = sm->LoadSystemAbility(TEST_ID_NORANGE_SAID, deviceId, callback);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: LoadSystemAbility002
 * @tc.desc: check LoadSystemAbility
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, LoadSystemAbility002, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    std::string deviceId = "";
    sptr<SystemAbilityLoadCallbackMock> callback(new SystemAbilityLoadCallbackMock());
    int32_t res = sm->LoadSystemAbility(TEST_ID_VAILD, deviceId, callback);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: LoadSystemAbility003
 * @tc.desc: check LoadSystemAbility
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, LoadSystemAbility003, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    std::string deviceId = "deviceId";
    int32_t res = sm->LoadSystemAbility(TEST_ID_VAILD, deviceId, nullptr);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: LoadSystemAbility004
 * @tc.desc: check LoadSystemAbility
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, LoadSystemAbility004, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    std::string deviceId = "deviceId";
    sptr<SystemAbilityLoadCallbackMock> callback(new SystemAbilityLoadCallbackMock());
    int32_t res = sm->LoadSystemAbility(TEST_ID_VAILD, deviceId, callback);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: LoadSystemAbility005
 * @tc.desc: cover LoadSystemAbility
 * @tc.type: FUNC
 * @tc.require: I7XT8Z
 */
HWTEST_F(SystemAbilityMgrProxyTest, LoadSystemAbility005, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    int32_t timeout = 0;
    sptr<IRemoteObject> res = sm->LoadSystemAbility(TEST_SAID_INVALID, timeout);
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: UnloadSystemAbility001
 * @tc.desc: call UnloadSystemAbility with invalid said
 * @tc.type: FUNC
 * @tc.require: I6AJ3S
 */
HWTEST_F(SystemAbilityMgrProxyTest, UnloadSystemAbility001, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    int32_t res = sm->UnloadSystemAbility(TEST_ID_NORANGE_SAID);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: UnloadSystemAbility002
 * @tc.desc: call UnloadSystemAbility
 * @tc.type: FUNC
 * @tc.require: I6AJ3S
 */
HWTEST_F(SystemAbilityMgrProxyTest, UnloadSystemAbility002, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    int32_t res = sm->UnloadSystemAbility(TEST_ID_VAILD);
    EXPECT_EQ(res, PROFILE_NOT_EXIST);
}

/**
 * @tc.name: ServiceRegistry001
 * @tc.desc: check ServiceRegistry
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, ServiceRegistry001, TestSize.Level1)
{
    sptr<IServiceRegistry> sr = ServiceRegistry::GetInstance();
    EXPECT_EQ(sr, nullptr);
}

/**
 * @tc.name: OnLoadSystemAbilitySuccess001
 * @tc.desc: check OnLoadSystemAbilitySuccess001
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnLoadSystemAbilitySuccess001, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<SystemAbilityLoadCallbackProxy> systemAbility(new SystemAbilityLoadCallbackProxy(testAbility));
    systemAbility->OnLoadSystemAbilitySuccess(TEST_ID_NORANGE_SAID, testAbility);
    EXPECT_EQ(testAbility->flag_, false);
}

/**
 * @tc.name: OnLoadSystemAbilitySuccess002
 * @tc.desc: check OnLoadSystemAbilitySuccess002
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnLoadSystemAbilitySuccess002, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<SystemAbilityLoadCallbackProxy> systemAbility(new SystemAbilityLoadCallbackProxy(nullptr));
    systemAbility->OnLoadSystemAbilitySuccess(TEST_ID_VAILD, testAbility);
    EXPECT_EQ(testAbility->flag_, false);
}

/**
 * @tc.name: OnLoadSystemAbilitySuccess003
 * @tc.desc: check OnLoadSystemAbilitySuccess003
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnLoadSystemAbilitySuccess003, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<SystemAbilityLoadCallbackProxy> systemAbility(new SystemAbilityLoadCallbackProxy(testAbility));
    systemAbility->OnLoadSystemAbilitySuccess(TEST_ID_VAILD, nullptr);
    EXPECT_EQ(testAbility->flag_, false);
}

/**
 * @tc.name: OnLoadSystemAbilitySuccess004
 * @tc.desc: check OnLoadSystemAbilitySuccess004
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnLoadSystemAbilitySuccess004, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<SystemAbilityLoadCallbackProxy> systemAbility(new SystemAbilityLoadCallbackProxy(testAbility));
    systemAbility->OnLoadSystemAbilitySuccess(TEST_ID_VAILD, testAbility);
    EXPECT_EQ(testAbility->flag_, true);
}

/**
 * @tc.name: OnLoadSystemAbilitySuccess005
 * @tc.desc: check OnLoadSystemAbilitySuccess005
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnLoadSystemAbilitySuccess005, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    testAbility->result_ = 1;
    sptr<SystemAbilityLoadCallbackProxy> systemAbility(new SystemAbilityLoadCallbackProxy(testAbility));
    systemAbility->OnLoadSystemAbilitySuccess(TEST_ID_VAILD, testAbility);
    EXPECT_EQ(testAbility->flag_, true);
}

/**
 * @tc.name: OnLoadSystemAbilityFail001
 * @tc.desc: check OnLoadSystemAbilityFail001
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnLoadSystemAbilityFail001, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<SystemAbilityLoadCallbackProxy> systemAbility(new SystemAbilityLoadCallbackProxy(testAbility));
    systemAbility->OnLoadSystemAbilityFail(TEST_ID_NORANGE_SAID);
    EXPECT_EQ(testAbility->flag_, false);
}

/**
 * @tc.name: OnLoadSystemAbilityFail002
 * @tc.desc: check OnLoadSystemAbilityFail002
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnLoadSystemAbilityFail002, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<SystemAbilityLoadCallbackProxy> systemAbility(new SystemAbilityLoadCallbackProxy(nullptr));
    systemAbility->OnLoadSystemAbilityFail(TEST_ID_VAILD);
    EXPECT_EQ(testAbility->flag_, false);
}

/**
 * @tc.name: OnLoadSystemAbilityFail003
 * @tc.desc: check OnLoadSystemAbilityFail003
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnLoadSystemAbilityFail003, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<SystemAbilityLoadCallbackProxy> systemAbility(new SystemAbilityLoadCallbackProxy(testAbility));
    systemAbility->OnLoadSystemAbilityFail(TEST_ID_VAILD);
    EXPECT_EQ(testAbility->flag_, true);
}

/**
 * @tc.name: OnLoadSystemAbilityFail004
 * @tc.desc: check OnLoadSystemAbilityFail004
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnLoadSystemAbilityFail004, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    testAbility->result_ = 1;
    sptr<SystemAbilityLoadCallbackProxy> systemAbility(new SystemAbilityLoadCallbackProxy(testAbility));
    systemAbility->OnLoadSystemAbilityFail(TEST_ID_VAILD);
    EXPECT_EQ(testAbility->flag_, true);
}

/**
 * @tc.name: OnLoadSACompleteForRemote001
 * @tc.desc: check OnLoadSACompleteForRemote001
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnLoadSACompleteForRemote001, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<SystemAbilityLoadCallbackProxy> systemAbility(new SystemAbilityLoadCallbackProxy(testAbility));
    string deviceId = "test";
    systemAbility->OnLoadSACompleteForRemote(deviceId, TEST_ID_NORANGE_SAID, testAbility);
    EXPECT_EQ(testAbility->flag_, false);
}

/**
 * @tc.name: OnLoadSACompleteForRemote002
 * @tc.desc: check OnLoadSACompleteForRemote002
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnLoadSACompleteForRemote002, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<SystemAbilityLoadCallbackProxy> systemAbility(new SystemAbilityLoadCallbackProxy(nullptr));
    string deviceId = "test";
    systemAbility->OnLoadSACompleteForRemote(deviceId, TEST_ID_VAILD, testAbility);
    EXPECT_EQ(testAbility->flag_, false);
}

/**
 * @tc.name: OnLoadSACompleteForRemote003
 * @tc.desc: check OnLoadSACompleteForRemote003
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnLoadSACompleteForRemote003, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<SystemAbilityLoadCallbackProxy> systemAbility(new SystemAbilityLoadCallbackProxy(testAbility));
    string deviceId = "test";
    systemAbility->OnLoadSACompleteForRemote(deviceId, TEST_ID_VAILD, nullptr);
    EXPECT_EQ(testAbility->flag_, true);
}

/**
 * @tc.name: OnLoadSACompleteForRemote004
 * @tc.desc: check OnLoadSACompleteForRemote004
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnLoadSACompleteForRemote004, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<SystemAbilityLoadCallbackProxy> systemAbility(new SystemAbilityLoadCallbackProxy(testAbility));
    string deviceId = "test";
    systemAbility->OnLoadSACompleteForRemote(deviceId, TEST_ID_VAILD, testAbility);
    EXPECT_EQ(testAbility->flag_, true);
}

/**
 * @tc.name: OnLoadSACompleteForRemote005
 * @tc.desc: check OnLoadSACompleteForRemote005
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnLoadSACompleteForRemote005, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    testAbility->result_ = 1;
    sptr<SystemAbilityLoadCallbackProxy> systemAbility(new SystemAbilityLoadCallbackProxy(testAbility));
    string deviceId = "test";
    systemAbility->OnLoadSACompleteForRemote(deviceId, TEST_ID_VAILD, testAbility);
    EXPECT_EQ(testAbility->flag_, true);
}

/**
 * @tc.name: OnAddSystemAbility001
 * @tc.desc: check SystemAbilityStatusChangeProxy OnAddSystemAbility001
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnAddSystemAbility001, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<SystemAbilityStatusChangeProxy> systemAbility(new SystemAbilityStatusChangeProxy(testAbility));
    string deviceId = "test";
    systemAbility->OnAddSystemAbility(TEST_ID_NORANGE_SAID, deviceId);
    EXPECT_EQ(testAbility->flag_, false);
}

/**
 * @tc.name: OnAddSystemAbility002
 * @tc.desc: check SystemAbilityStatusChangeProxy OnAddSystemAbility002
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnAddSystemAbility002, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<SystemAbilityStatusChangeProxy> systemAbility(new SystemAbilityStatusChangeProxy(nullptr));
    string deviceId = "test";
    systemAbility->OnAddSystemAbility(TEST_ID_VAILD, deviceId);
    EXPECT_EQ(testAbility->flag_, false);
}

/**
 * @tc.name: OnAddSystemAbility003
 * @tc.desc: check SystemAbilityStatusChangeProxy OnAddSystemAbility003
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnAddSystemAbility003, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<SystemAbilityStatusChangeProxy> systemAbility(new SystemAbilityStatusChangeProxy(testAbility));
    string deviceId = "test";
    systemAbility->OnAddSystemAbility(TEST_ID_VAILD, deviceId);
    EXPECT_EQ(testAbility->flag_, true);
}

/**
 * @tc.name: OnAddSystemAbility004
 * @tc.desc: check SystemAbilityStatusChangeProxy OnAddSystemAbility004
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnAddSystemAbility004, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    testAbility->result_ = 1;
    sptr<SystemAbilityStatusChangeProxy> systemAbility(new SystemAbilityStatusChangeProxy(testAbility));
    string deviceId = "test";
    systemAbility->OnAddSystemAbility(TEST_ID_VAILD, deviceId);
    EXPECT_EQ(testAbility->flag_, true);
}

/**
 * @tc.name: OnRemoveSystemAbility001
 * @tc.desc: check SystemAbilityStatusChangeProxy OnRemoveSystemAbility001
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnRemoveSystemAbility001, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    testAbility->result_ = 1;
    sptr<SystemAbilityStatusChangeProxy> systemAbility(new SystemAbilityStatusChangeProxy(testAbility));
    string deviceId = "test";
    systemAbility->OnRemoveSystemAbility(TEST_ID_VAILD, deviceId);
    EXPECT_EQ(testAbility->flag_, true);
}

/**
 * @tc.name: OnRemoveSystemAbility002
 * @tc.desc: check SystemAbilityStatusChangeProxy OnRemoveSystemAbility002
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnRemoveSystemAbility002, TestSize.Level1)
{
    sptr<MockIroSendrequesteStub> testAbility(new MockIroSendrequesteStub());
    sptr<SystemAbilityStatusChangeProxy> systemAbility(new SystemAbilityStatusChangeProxy(testAbility));
    string deviceId = "test";
    systemAbility->OnRemoveSystemAbility(TEST_ID_VAILD, deviceId);
    EXPECT_EQ(testAbility->flag_, true);
}

/**
 * @tc.name: DestroySystemAbilityManagerObject001
 * @tc.desc: check  DestroySystemAbilityManagerObject
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, DestroySystemAbilityManagerObject001, TestSize.Level4)
{
    SystemAbilityManagerClient::GetInstance().DestroySystemAbilityManagerObject();
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
}

/**
 * @tc.name: GetSystemProcessInfo001
 * @tc.desc: GetSystemProcessInfo
 * @tc.type: FUNC
 * @tc.require: I7VQQG
 */
HWTEST_F(SystemAbilityMgrProxyTest, GetSystemProcessInfo001, TestSize.Level3)
{
    DTEST_LOG << " GetSystemProcessInfo001 start " << std::endl;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    SystemProcessInfo processInfo;
    int32_t systemAbilityId = 401;
    int32_t ret = sm->GetSystemProcessInfo(systemAbilityId, processInfo);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: GetRunningSystemProcess001
 * @tc.desc: GetRunningSystemProcess
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, GetRunningSystemProcess001, TestSize.Level3)
{
    DTEST_LOG << " GetRunningSystemProcess001 start " << std::endl;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    SystemProcessInfo processInfo = {"test"};
    std::list<SystemProcessInfo> systemProcessInfos;
    systemProcessInfos.push_back(processInfo);
    int32_t ret = sm->GetRunningSystemProcess(systemProcessInfos);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: ReadSystemProcessFromParcel001
 * @tc.desc: ReadSystemProcessFromParcel
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, ReadSystemProcessFromParcel001, TestSize.Level3)
{
    DTEST_LOG << " ReadSystemProcessFromParcel001 start " << std::endl;
    sptr<SystemAbilityManagerProxy> sm = new SystemAbilityManagerProxy(nullptr);
    std::list<SystemProcessInfo> systemProcessInfos;
    MessageParcel reply;
    int32_t ret = sm->ReadSystemProcessFromParcel(reply, systemProcessInfos);
    EXPECT_EQ(ret, ERR_FLATTEN_OBJECT);
}

/**
 * @tc.name: ReadSystemProcessFromParcel002
 * @tc.desc: ReadSystemProcessFromParce2
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, ReadSystemProcessFromParcel002, TestSize.Level3)
{
    DTEST_LOG << " ReadSystemProcessFromParcel002 start " << std::endl;
    sptr<SystemAbilityManagerProxy> sm = new SystemAbilityManagerProxy(nullptr);
    std::list<SystemProcessInfo> systemProcessInfos;
    MessageParcel reply;
    int32_t size = 0;
    reply.WriteInt32(size);
    int32_t ret = sm->ReadSystemProcessFromParcel(reply, systemProcessInfos);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: ReadSystemProcessFromParcel003
 * @tc.desc: ReadSystemProcessFromParcel
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, ReadSystemProcessFromParcel003, TestSize.Level3)
{
    DTEST_LOG << " ReadSystemProcessFromParcel003 start " << std::endl;
    sptr<SystemAbilityManagerProxy> sm = new SystemAbilityManagerProxy(nullptr);
    std::list<SystemProcessInfo> systemProcessInfos;
    MessageParcel reply;
    int32_t size = 1;
    reply.WriteInt32(size);
    int32_t ret = sm->ReadSystemProcessFromParcel(reply, systemProcessInfos);
    EXPECT_EQ(ret, ERR_FLATTEN_OBJECT);
}

/**
 * @tc.name: ReadSystemProcessFromParcel004
 * @tc.desc: ReadSystemProcessFromParcel
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, ReadSystemProcessFromParcel004, TestSize.Level3)
{
    DTEST_LOG << " ReadSystemProcessFromParcel004 start " << std::endl;
    sptr<SystemAbilityManagerProxy> sm = new SystemAbilityManagerProxy(nullptr);
    std::list<SystemProcessInfo> systemProcessInfos;
    MessageParcel reply;
    int32_t size = 1;
    reply.WriteInt32(size);
    reply.WriteString("testProcessName");
    int32_t ret = sm->ReadSystemProcessFromParcel(reply, systemProcessInfos);
    EXPECT_EQ(ret, ERR_FLATTEN_OBJECT);
}

/**
 * @tc.name: ReadSystemProcessFromParcel005
 * @tc.desc: ReadSystemProcessFromParcel
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, ReadSystemProcessFromParcel005, TestSize.Level3)
{
    DTEST_LOG << " ReadSystemProcessFromParcel005 start " << std::endl;
    sptr<SystemAbilityManagerProxy> sm = new SystemAbilityManagerProxy(nullptr);
    std::list<SystemProcessInfo> systemProcessInfos;
    MessageParcel reply;
    int32_t size = 1;
    reply.WriteInt32(size);
    reply.WriteString("testProcessName");
    int32_t pid = 12345;
    reply.WriteInt32(pid);
    int32_t ret = sm->ReadSystemProcessFromParcel(reply, systemProcessInfos);
    EXPECT_EQ(ret, ERR_FLATTEN_OBJECT);
}

/**
 * @tc.name: ReadSystemProcessFromParcel006
 * @tc.desc: ReadSystemProcessFromParcel
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, ReadSystemProcessFromParcel006, TestSize.Level3)
{
    DTEST_LOG << " ReadSystemProcessFromParcel006 start " << std::endl;
    sptr<SystemAbilityManagerProxy> sm = new SystemAbilityManagerProxy(nullptr);
    std::list<SystemProcessInfo> systemProcessInfos;
    MessageParcel reply;
    int32_t size = 1;
    reply.WriteInt32(size);
    reply.WriteString("testProcessName");
    int32_t pid = 12345;
    reply.WriteInt32(pid);
    int32_t uid = 6789;
    reply.WriteInt32(uid);
    int32_t ret = sm->ReadSystemProcessFromParcel(reply, systemProcessInfos);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: LoadSystemAbility006
 * @tc.desc: LoadSystemAbility
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, LoadSystemAbility006, TestSize.Level1)
{
    DTEST_LOG << " LoadSystemAbility006 start " << std::endl;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    int32_t timeout = -1;
    sptr<IRemoteObject> ret = sm->LoadSystemAbility(TEST_SAID_INVALID, timeout);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: ReadOnDemandEventsFromParcel005
 * @tc.desc: ReadOnDemandEventsFromParcel
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, ReadOnDemandEventsFromParcel005, TestSize.Level3)
{
    DTEST_LOG << " ReadOnDemandEventsFromParcel005 start " << std::endl;
    std::vector<SystemAbilityOnDemandEvent> abilityOnDemandEvents;
    MessageParcel reply;
    int32_t size = 1;
    reply.WriteInt32(size);
    bool prersistence = false;
    reply.WriteBool(prersistence);
    int32_t conditionsSize = -1;
    reply.WriteInt32(conditionsSize);
    bool ret = OnDemandEventToParcel::ReadOnDemandEventsFromParcel(abilityOnDemandEvents, reply);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: OnRemoteRequest003
 * @tc.desc: OnRemoteRequest
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnRemoteRequest003, TestSize.Level3)
{
    DTEST_LOG << " OnRemoteRequest003 start " << std::endl;
    sptr<SystemProcessStatusChangeStub> procStub = new SystemProcessStatusChange();
    uint32_t code = 1000;
    MessageParcel data;
    data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN);
    MessageParcel reply;
    MessageOption option;
    int32_t ret = procStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_NE(ret, ERR_NONE);
}

/**
 * @tc.name: SubscribeSystemProcess001
 * @tc.desc: SubscribeSystemProcess
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, SubscribeSystemProcess001, TestSize.Level3)
{
    DTEST_LOG << " SubscribeSystemProcess001 start " << std::endl;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    sptr<ISystemProcessStatusChange> systemProcessStatusChange = new SystemProcessStatusChange();
    int32_t ret = sm->SubscribeSystemProcess(systemProcessStatusChange);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: UnSubscribeSystemProcess001
 * @tc.desc: UnSubscribeSystemProcess
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, UnSubscribeSystemProcess001, TestSize.Level3)
{
    DTEST_LOG << " UnSubscribeSystemProcess001 start " << std::endl;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    sptr<ISystemProcessStatusChange> systemProcessStatusChange = new SystemProcessStatusChange();
    int32_t ret = sm->UnSubscribeSystemProcess(systemProcessStatusChange);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: UnSubscribeSystemProcess002
 * @tc.desc: UnSubscribeSystemProcess
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, UnSubscribeSystemProcess002, TestSize.Level3)
{
    DTEST_LOG << " UnSubscribeSystemProcess002 start " << std::endl;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sm, nullptr);
    sptr<ISystemProcessStatusChange> systemProcessStatusChange = nullptr;
    int32_t ret = sm->UnSubscribeSystemProcess(systemProcessStatusChange);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: UnloadAllIdleSa001
 * @tc.desc: UnloadAllIdleSystemAbility
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrProxyTest, UnloadAllIdleSa001, TestSize.Level1)
{
    SamMockPermission::MockProcess("memmgrservice");
    sptr<ISystemAbilityManager> samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    int32_t res = samgrProxy->UnloadAllIdleSystemAbility();
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: OnRemoteRequest001
 * @tc.desc: OnRemoteRequest001
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnRemoteRequest001, TestSize.Level3)
{
    sptr<SystemProcessStatusChange> processStub = new SystemProcessStatusChange();
    uint32_t code = 1;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    int32_t ret = processStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: OnRemoteRequest002
 * @tc.desc: OnRemoteRequest002
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnRemoteRequest002, TestSize.Level3)
{
    sptr<SystemProcessStatusChangeStub> processStub = new SystemProcessStatusChange();
    uint32_t code = 1;
    MessageParcel data;
    data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN);
    MessageParcel reply;
    MessageOption option;
    int32_t ret = processStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, ERR_NULL_OBJECT);
}

/**
 * @tc.name: OnSystemProcessStartedInner001
 * @tc.desc: OnSystemProcessStartedInner001
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnSystemProcessStartedInner001, TestSize.Level3)
{
    sptr<SystemProcessStatusChangeStub> processStub = new SystemProcessStatusChange();
    MessageParcel data;
    data.WriteString("test");
    MessageParcel reply;
    int32_t ret = processStub->OnSystemProcessStartedInner(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

/**
 * @tc.name: OnSystemProcessStoppedInner001
 * @tc.desc: OnSystemProcessStoppedInner001
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnSystemProcessStoppedInner001, TestSize.Level3)
{
    sptr<SystemProcessStatusChangeStub> processStub = new SystemProcessStatusChange();
    MessageParcel data;
    data.WriteString("test");
    MessageParcel reply;
    int32_t ret = processStub->OnSystemProcessStoppedInner(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

/**
 * @tc.name: OnSystemProcessStoppedInner002
 * @tc.desc: OnSystemProcessStoppedInner002
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, OnSystemProcessStoppedInner002, TestSize.Level3)
{
    sptr<SystemProcessStatusChangeStub> processStub = new SystemProcessStatusChange();
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = processStub->OnSystemProcessStoppedInner(data, reply);
    EXPECT_EQ(ret, ERR_NULL_OBJECT);
}

/**
 * @tc.name: CancelUnloadSystemAbility001
 * @tc.desc: CancelUnloadSystemAbility001
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(SystemAbilityMgrProxyTest, CancelUnloadSystemAbility001, TestSize.Level3)
{
    sptr<ISystemAbilityManager> samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    int32_t said = -1;
    int32_t result = samgrProxy->CancelUnloadSystemAbility(said);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: GetOnDemandPolicy001
 * @tc.desc: GetOnDemandPolicy001
 * @tc.type: FUNC
 * @tc.require: I6T116
 */
HWTEST_F(SystemAbilityMgrProxyTest, GetOnDemandPolicy001, TestSize.Level3)
{
    sptr<ISystemAbilityManager> samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    int32_t systemAbilityId = -1;
    OnDemandPolicyType type = OnDemandPolicyType::START_POLICY;
    std::vector<SystemAbilityOnDemandEvent> abilityOnDemandEvents;
    int32_t result = samgrProxy->GetOnDemandPolicy(systemAbilityId, type, abilityOnDemandEvents);
    EXPECT_NE(result, ERR_OK);
}

/**
 * @tc.name: UpdateOnDemandPolicy001
 * @tc.desc: UpdateOnDemandPolicy001
 * @tc.type: FUNC
 * @tc.require: I6T116
 */
HWTEST_F(SystemAbilityMgrProxyTest, UpdateOnDemandPolicy001, TestSize.Level3)
{
    sptr<ISystemAbilityManager> samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    int32_t systemAbilityId = -1;
    OnDemandPolicyType type = OnDemandPolicyType::START_POLICY;
    std::vector<SystemAbilityOnDemandEvent> abilityOnDemandEvents;
    int32_t result = samgrProxy->UpdateOnDemandPolicy(systemAbilityId, type, abilityOnDemandEvents);
    EXPECT_NE(result, ERR_OK);
}

/**
 * @tc.name: GetOnDemandReasonExtraData001
 * @tc.desc: test GetOnDemandReasonExtraData
 * @tc.type: FUNC
 * @tc.require: I6XB42
 */
HWTEST_F(SystemAbilityMgrProxyTest, GetOnDemandReasonExtraData001, TestSize.Level3)
{
    sptr<ISystemAbilityManager> samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    MessageParcel extraDataParcel;
    extraDataParcel.WriteInt32(RESULT);
    int32_t ret = samgrProxy->GetOnDemandReasonExtraData(EXTRA_DATA_ID, extraDataParcel);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: SubscribeSystemProcess002
 * @tc.desc: test SubscribeSystemProcess
 * @tc.type: FUNC
 * @tc.require: I7FBV6
 */
HWTEST_F(SystemAbilityMgrProxyTest, SubscribeSystemProcess002, TestSize.Level3)
{
    sptr<ISystemAbilityManager> samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    int32_t ret = samgrProxy->SubscribeSystemProcess(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: SendStrategy001
 * @tc.desc: check SendStrategy
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrProxyTest, SendStrategy001, TestSize.Level1)
{
    sptr<ISystemAbilityManager> samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    std::vector<int32_t> saIds;
    saIds.push_back(-1);
    int32_t type = 1;
    int32_t level = 1;
    std::string action = "";
    int32_t res = samgrProxy->SendStrategy(type, saIds, level, action);
    EXPECT_EQ(res, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: GetExtensionSaIds001
 * @tc.desc: test GetExtensionSaIds, GetExtensionSaIds
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrProxyTest, GetExtensionSaIds002, TestSize.Level3)
{
    sptr<ISystemAbilityManager> samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    std::vector<int32_t> saIds;

    int32_t ret = samgrProxy->GetExtensionSaIds("backup_test", saIds);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(saIds.size(), 0);
}

/**
 * @tc.name: GetExtensionRunningSaList001
 * @tc.desc: test GetExtensionRunningSaList, GetExtensionRunningSaList
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrProxyTest, GetExtensionRunningSaList002, TestSize.Level3)
{
    sptr<ISystemAbilityManager> samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    std::vector<sptr<IRemoteObject>> saList;

    int32_t ret = samgrProxy->GetExtensionRunningSaList("restore_test", saList);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(saList.size(), 0);
}

HWTEST_F(SystemAbilityMgrProxyTest, GetRunningSaExtensionInfoList001, TestSize.Level3)
{
    sptr<ISystemAbilityManager> samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();

    std::vector<ISystemAbilityManager::SaExtensionInfo> infoList;
    int32_t ret = samgrProxy->GetRunningSaExtensionInfoList("backup_test", infoList);
    EXPECT_EQ(ret, 1);
    EXPECT_EQ(infoList.size(), 0);
}

/**
 * @tc.name: GetCommonEventExtraDataIdlist001
 * @tc.desc: test GetCommonEventExtraDataIdlist
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrProxyTest, GetCommonEventExtraDataIdlist001, TestSize.Level3)
{
    sptr<ISystemAbilityManager> samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    std::vector<int64_t> idList;
    int32_t ret = samgrProxy->GetCommonEventExtraDataIdlist(-1, idList);
    EXPECT_EQ(ret, ERR_NULL_OBJECT);
    EXPECT_EQ(idList.size(), 0);

    ret = samgrProxy->GetCommonEventExtraDataIdlist(-1, idList, "test");
    EXPECT_EQ(ret, ERR_NULL_OBJECT);
    EXPECT_EQ(idList.size(), 0);

    ret = samgrProxy->GetCommonEventExtraDataIdlist(1, idList);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(idList.size(), 0);

    ret = samgrProxy->GetCommonEventExtraDataIdlist(1, idList, "test");
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(idList.size(), 0);
}
}