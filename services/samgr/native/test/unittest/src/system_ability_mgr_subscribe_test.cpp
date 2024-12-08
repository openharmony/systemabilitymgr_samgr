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

#include "system_ability_mgr_test.h"
#include "hisysevent_adapter.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "itest_transaction_service.h"
#include "sam_mock_permission.h"
#include "parameter.h"
#include "parameters.h"
#include "sa_profiles.h"
#include "sa_status_change_mock.h"
#include "string_ex.h"
#include "system_ability_definition.h"
#include "samgr_err_code.h"
#include "system_process_status_change_proxy.h"
#include "system_ability_manager_util.h"
#include "ability_death_recipient.h"
#include "test_log.h"
#define private public
#include "ipc_skeleton.h"
#include "accesstoken_kit.h"
#include "system_ability_manager.h"
#ifdef SUPPORT_COMMON_EVENT
#include "common_event_collect.h"
#endif

using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS;

namespace OHOS {
namespace {
constexpr int32_t SAID = 1234;
constexpr int32_t OVERFLOW_TIME = 257;
constexpr int32_t TEST_OVERFLOW_SAID = 99999;

void InitSaMgr(sptr<SystemAbilityManager>& saMgr)
{
    saMgr->abilityDeath_ = sptr<IRemoteObject::DeathRecipient>(new AbilityDeathRecipient());
    saMgr->systemProcessDeath_ = sptr<IRemoteObject::DeathRecipient>(new SystemProcessDeathRecipient());
    saMgr->abilityStatusDeath_ = sptr<IRemoteObject::DeathRecipient>(new AbilityStatusDeathRecipient());
    saMgr->abilityCallbackDeath_ = sptr<IRemoteObject::DeathRecipient>(new AbilityCallbackDeathRecipient());
    saMgr->remoteCallbackDeath_ = sptr<IRemoteObject::DeathRecipient>(new RemoteCallbackDeathRecipient());
    saMgr->workHandler_ = make_shared<FFRTHandler>("workHandler");
    saMgr->collectManager_ = sptr<DeviceStatusCollectManager>(new DeviceStatusCollectManager());
    saMgr->abilityStateScheduler_ = std::make_shared<SystemAbilityStateScheduler>();
}
}

/**
 * @tc.name: SubscribeSystemAbility001
 * @tc.desc: test SubscribeSystemAbility, ERR_INVALID_VALUE.
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrTest, SubscribeSystemAbility001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
    int32_t systemAbilityId = -1;
    const sptr<ISystemAbilityStatusChange> listener;
    int32_t ret = saMgr->SubscribeSystemAbility(systemAbilityId, listener);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: UnSubscribeSystemAbility001
 * @tc.desc: test UnSubscribeSystemAbility, ERR_INVALID_VALUE.
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrTest, UnSubscribeSystemAbility001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
    int32_t systemAbilityId = -1;
    const sptr<ISystemAbilityStatusChange> listener;
    int32_t ret = saMgr->UnSubscribeSystemAbility(systemAbilityId, listener);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: ReportSubscribeOverflow001
 * @tc.desc: ReportSubscribeOverflow001
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, ReportSubscribeOverflow001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
    saMgr->workHandler_ = make_shared<FFRTHandler>("workHandler");
    ASSERT_TRUE(saMgr != nullptr);
    std::vector<sptr<SaStatusChangeMock>> tmpCallbak;
    sptr<SaStatusChangeMock> callback = nullptr;
    for (int i = 0; i < OVERFLOW_TIME; ++i) {
        callback = new SaStatusChangeMock();
        tmpCallbak.emplace_back(callback);
        saMgr->SubscribeSystemAbility(TEST_OVERFLOW_SAID, callback);
    }
    for (const auto& callback : tmpCallbak) {
        saMgr->UnSubscribeSystemAbility(TEST_OVERFLOW_SAID, callback);
    }
}

/**
 * @tc.name: UnSubscribeSystemAbilityDied001
 * @tc.desc: test UnSubscribeSystemAbility with OnRemoteDied
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrTest, UnSubscribeSystemAbilityDied001, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
    ASSERT_TRUE(saMgr != nullptr);
    sptr<SaStatusChangeMock> callback(new SaStatusChangeMock());
    saMgr->listenerMap_[SAID].push_back({callback, SAID});
    auto& count = saMgr->subscribeCountMap_[SAID];
    ++count;
    saMgr->UnSubscribeSystemAbility(callback->AsObject());
}

/**
 * @tc.name: Test SubscribeSystemProcess001
 * @tc.desc: SubscribeSystemProcess001
 * @tc.type: FUNC
 * @tc.require: I6H10P
 */
HWTEST_F(SystemAbilityMgrTest, SubscribeSystemProcess001, TestSize.Level3)
{
    DTEST_LOG << " SubscribeSystemProcess001 " << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
    sptr<ISystemProcessStatusChange> systemProcessStatusChange = new SystemProcessStatusChange();
    saMgr->abilityStateScheduler_ = std::make_shared<SystemAbilityStateScheduler>();
    int32_t ret = saMgr->SubscribeSystemProcess(systemProcessStatusChange);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: Test SubscribeSystemProcess002
 * @tc.desc: SubscribeSystemProcess002
 * @tc.type: FUNC
 * @tc.require: I6H10P
 */
HWTEST_F(SystemAbilityMgrTest, SubscribeSystemProcess002, TestSize.Level3)
{
    DTEST_LOG << " SubscribeSystemProcess002 " << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
    saMgr->abilityStateScheduler_ = nullptr;
    sptr<ISystemProcessStatusChange> systemProcessStatusChange = new SystemProcessStatusChange();
    int32_t ret = saMgr->SubscribeSystemProcess(systemProcessStatusChange);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: SubscribeSystemProcess003
 * @tc.desc: test SubscribeSystemProcess with abilityStateScheduler_ is nullptr
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrTest, SubscribeSystemProcess003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
    sptr<SystemProcessStatusChange> listener = new SystemProcessStatusChange();
    saMgr->abilityStateScheduler_ = nullptr;
    std::list<SystemProcessInfo> systemProcessInfos;
    int32_t ret = saMgr->SubscribeSystemProcess(listener);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: Test UnSubscribeSystemProcess001
 * @tc.desc: UnSubscribeSystemProcess001
 * @tc.type: FUNC
 * @tc.require: I6H10P
 */
HWTEST_F(SystemAbilityMgrTest, UnSubscribeSystemProcess001, TestSize.Level3)
{
    DTEST_LOG << " UnSubscribeSystemProcess001" << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    sptr<ISystemProcessStatusChange> systemProcessStatusChange = new SystemProcessStatusChange();
    int32_t ret = saMgr->UnSubscribeSystemProcess(systemProcessStatusChange);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: Test UnSubscribeSystemProcess002
 * @tc.desc: UnSubscribeSystemProcess002
 * @tc.type: FUNC
 * @tc.require: I6H10P
 */
HWTEST_F(SystemAbilityMgrTest, UnSubscribeSystemProcess002, TestSize.Level3)
{
    DTEST_LOG << " UnSubscribeSystemProcess002" << std::endl;
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
    sptr<ISystemProcessStatusChange> systemProcessStatusChange = new SystemProcessStatusChange();
    saMgr->abilityStateScheduler_ = std::make_shared<SystemAbilityStateScheduler>();
    int32_t ret = saMgr->UnSubscribeSystemProcess(systemProcessStatusChange);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: UnSubscribeSystemProcess003
 * @tc.desc: test UnSubscribeSystemProcess with abilityStateScheduler_ is nullptr
 * @tc.type: FUNC
 * @tc.require: I6NKWX
 */
HWTEST_F(SystemAbilityMgrTest, UnSubscribeSystemProcess003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = new SystemAbilityManager;
    EXPECT_NE(saMgr, nullptr);
    InitSaMgr(saMgr);
    sptr<SystemProcessStatusChange> listener = new SystemProcessStatusChange();
    saMgr->abilityStateScheduler_ = nullptr;
    std::list<SystemProcessInfo> systemProcessInfos;
    int32_t ret = saMgr->UnSubscribeSystemProcess(listener);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: Test OnSystemProcessStarted001
 * @tc.desc: OnSystemProcessStarted
 * @tc.type: FUNC
 * @tc.require: I6H10P
 */
HWTEST_F(SystemAbilityMgrTest, OnSystemProcessStarted001, TestSize.Level3)
{
    DTEST_LOG << " OnSystemProcessStarted001" << std::endl;
    sptr<ISystemProcessStatusChange> systemProcessStatusChange = new SystemProcessStatusChange();
    SystemProcessInfo systemProcessInfos;
    systemProcessStatusChange->OnSystemProcessStarted(systemProcessInfos);
    EXPECT_NE(systemProcessStatusChange, nullptr);
}

/**
 * @tc.name: Test OnSystemProcessStopped001
 * @tc.desc: OnSystemProcessStopped001
 * @tc.type: FUNC
 * @tc.require: I6H10P
 */
HWTEST_F(SystemAbilityMgrTest, OnSystemProcessStopped001, TestSize.Level3)
{
    DTEST_LOG << " OnSystemProcessStopped001" << std::endl;
    sptr<SystemProcessStatusChangeStub> systemProcessStatusChange = new SystemProcessStatusChange();
    SystemProcessInfo systemProcessInfos;
    systemProcessStatusChange->OnSystemProcessStopped(systemProcessInfos);
    EXPECT_NE(systemProcessStatusChange, nullptr);
}

/**
 * @tc.name: Test SendRequestInner001
 * @tc.desc: SendRequestInner001
 * @tc.type: FUNC
 * @tc.require: I6H10P
 */
HWTEST_F(SystemAbilityMgrTest, SendRequestInner001, TestSize.Level3)
{
    DTEST_LOG << " SendRequestInner001" << std::endl;
    sptr<SystemProcessStatusChangeStub> stub = new SystemProcessStatusChange();
    sptr<SystemProcessStatusChangeProxy> systemProcessStatusChange = new SystemProcessStatusChangeProxy(stub);
    SystemProcessInfo systemProcessInfos;
    uint32_t code = 1;
    bool ret = systemProcessStatusChange->SendRequestInner(code, systemProcessInfos);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: Test SendRequestInner002
 * @tc.desc: SendRequestInner002
 * @tc.type: FUNC
 * @tc.require: I6H10P
 */
HWTEST_F(SystemAbilityMgrTest, SendRequestInner002, TestSize.Level3)
{
    DTEST_LOG << " SendRequestInner002" << std::endl;
    sptr<SystemProcessStatusChangeStub> stub = new SystemProcessStatusChange();
    sptr<SystemProcessStatusChangeProxy> systemProcessStatusChange = new SystemProcessStatusChangeProxy(stub);
    SystemProcessInfo systemProcessInfos;
    systemProcessInfos.processName = "test";
    uint32_t code = 1;
    bool ret = systemProcessStatusChange->SendRequestInner(code, systemProcessInfos);
    EXPECT_EQ(ret, true);
}

} // namespace OHOS