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

#include "system_ability_mgr_stub_test.h"
#include "samgr_err_code.h"
#include "ability_death_recipient.h"
#include "itest_transaction_service.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "sa_profiles.h"
#include "sam_mock_permission.h"
#include "string_ex.h"
#include "system_ability_definition.h"
#include "test_log.h"

#define private public
#include "sa_status_change_mock.h"
#include "system_ability_manager.h"

using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS;

namespace OHOS {
namespace {

const std::u16string SAMANAGER_INTERFACE_TOKEN = u"ohos.samgr.accessToken";
const string DEFAULT_LOAD_NAME = "loadevent";
constexpr uint32_t SAID = 1499;
constexpr int32_t INVALID_SAID = -1;
constexpr uint32_t INVALID_CODE = 50;
}
#ifdef SUPPORT_PENGLAI_MODE
namespace Penglai {

extern bool permissionRet;
}
#endif

void SystemProcessStatusChange::OnSystemProcessStarted(SystemProcessInfo& systemProcessInfo)
{
    cout << "OnSystemProcessStarted, processName: ";
}

void SystemProcessStatusChange::OnSystemProcessStopped(SystemProcessInfo& systemProcessInfo)
{
    cout << "OnSystemProcessStopped, processName: ";
}

void SystemAbilityMgrStubTest::SetUpTestCase()
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    saMgr->abilityStateScheduler_ = std::make_shared<SystemAbilityStateScheduler>();
    std::list<SaProfile> saProfiles;
    saMgr->abilityStateScheduler_->Init(saProfiles);
    DTEST_LOG << "SetUpTestCase" << std::endl;
}

void SystemAbilityMgrStubTest::TearDownTestCase()
{
    DTEST_LOG << "TearDownTestCase" << std::endl;
}

void SystemAbilityMgrStubTest::SetUp()
{
    DTEST_LOG << "SetUp" << std::endl;
}

void SystemAbilityMgrStubTest::TearDown()
{
    DTEST_LOG << "TearDown" << std::endl;
}

void SystemAbilityMgrStubTest::AddSystemAbilityContext(int32_t systemAbilityId, const std::u16string& processName)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    EXPECT_TRUE(saMgr->abilityStateScheduler_ != nullptr);
    std::unique_lock<std::shared_mutex> processWriteLock(saMgr->abilityStateScheduler_->processMapLock_);
    if (saMgr->abilityStateScheduler_->processContextMap_.count(processName) == 0) {
        auto processContext = std::make_shared<SystemProcessContext>();
        processContext->processName = processName;
        processContext->abilityStateCountMap[SystemAbilityState::NOT_LOADED] = 0;
        processContext->abilityStateCountMap[SystemAbilityState::LOADING] = 0;
        processContext->abilityStateCountMap[SystemAbilityState::LOADED] = 0;
        processContext->abilityStateCountMap[SystemAbilityState::UNLOADABLE] = 0;
        processContext->abilityStateCountMap[SystemAbilityState::UNLOADING] = 0;
        saMgr->abilityStateScheduler_->processContextMap_[processName] = processContext;
    }
    saMgr->abilityStateScheduler_->processContextMap_[processName]->saList.push_back(systemAbilityId);
    saMgr->abilityStateScheduler_->processContextMap_[processName]
        ->abilityStateCountMap[SystemAbilityState::NOT_LOADED]++;

    auto abilityContext = std::make_shared<SystemAbilityContext>();
    abilityContext->systemAbilityId = systemAbilityId;
    abilityContext->ownProcessContext = saMgr->abilityStateScheduler_->processContextMap_[processName];
    std::unique_lock<std::shared_mutex> abiltyWriteLock(saMgr->abilityStateScheduler_->abiltyMapLock_);
    saMgr->abilityStateScheduler_->abilityContextMap_[systemAbilityId] = abilityContext;
}

HWTEST_F(SystemAbilityMgrStubTest, OnRemoteRequest001, TestSize.Level4)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN);
    MessageParcel reply;
    MessageOption option;
    int32_t result = saMgr->OnRemoteRequest(INVALID_CODE, data, reply, option);
    EXPECT_EQ(result, IPC_STUB_UNKNOW_TRANS_ERR);
}

#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityMgrStubTest, ListSystemAbilityInner001, TestSize.Level4)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t result = saMgr->ListSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
}
#endif

HWTEST_F(SystemAbilityMgrStubTest, SubsSystemAbilityInner001, TestSize.Level4)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t result = saMgr->SubsSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

HWTEST_F(SystemAbilityMgrStubTest, UnSubsSystemAbilityInner001, TestSize.Level4)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t result = saMgr->UnSubsSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityMgrStubTest, CheckRemtSystemAbilityInner001, TestSize.Level4)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t result = saMgr->CheckRemtSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
}
#endif

#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityMgrStubTest, AddOndemandSystemAbilityInner001, TestSize.Level4)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t result = saMgr->AddOndemandSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
}
#endif

#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityMgrStubTest, RemoveSystemAbilityInner001, TestSize.Level4)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t result = saMgr->RemoveSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
}
#endif

#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityMgrStubTest, GetSystemProcessInfoInner001, TestSize.Level3)
{
    DTEST_LOG << "GetSystemProcessInfoInner001" << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = saMgr->GetSystemProcessInfoInner(data, reply);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}
#endif

#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityMgrStubTest, GetRunningSystemProcessInner001, TestSize.Level3)
{
    DTEST_LOG << "GetRunningSystemProcessInner001" << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = saMgr->GetRunningSystemProcessInner(data, reply);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}
#endif

#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityMgrStubTest, UnSubscribeSystemProcessInner001, TestSize.Level3)
{
    DTEST_LOG << "UnSubscribeSystemProcessInner001" << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = saMgr->UnSubscribeSystemProcessInner(data, reply);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}
#endif

#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityMgrStubTest, GetOnDemandPolicyInner001, TestSize.Level3)
{
    DTEST_LOG << "GetOnDemandPolicyInner001" << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = saMgr->GetOnDemandPolicyInner(data, reply);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}
#endif

/**
 * @tc.name: Test UpdateOnDemandPolicyInner001
 * @tc.desc: UpdateOnDemandPolicyInner001
 * @tc.type: FUNC
 * @tc.require: I6T116
 */
#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityMgrStubTest, UpdateOnDemandPolicyInner001, TestSize.Level3)
{
    DTEST_LOG << "UpdateOnDemandPolicyInner001" << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = saMgr->UpdateOnDemandPolicyInner(data, reply);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}
#endif

/**
 * @tc.name: GetOnDemandReasonExtraDataInner001
 * @tc.desc: test GetOnDemandReasonExtraDataInner with permission is denied
 * @tc.type: FUNC
 * @tc.require: I6XB42
 */
#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityMgrStubTest, GetOnDemandReasonExtraDataInner001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = saMgr->GetOnDemandReasonExtraDataInner(data, reply);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}
#endif

/**
 * @tc.name: GetExtensionSaIdsInner001
 * @tc.desc: test GetExtensionSaIdsInner with permission is denied
 * @tc.type: FUNC
 * @tc.require: I6XB42
 */
#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityMgrStubTest, GetExtensionSaIdsInner001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = saMgr->GetExtensionSaIdsInner(data, reply);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}
#endif

/**
 * @tc.name: GetExtensionRunningSaListInner001
 * @tc.desc: test GetExtensionRunningSaListInner with permission is denied
 * @tc.type: FUNC
 * @tc.require: I6XB42
 */
#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityMgrStubTest, GetExtensionRunningSaListInner001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = saMgr->GetExtensionRunningSaListInner(data, reply);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}
#endif

/**
 * @tc.name: GetCommonEventExtraDataIdlistInner001
 * @tc.desc: test GetCommonEventExtraDataIdlistInner with permission is denied
 * @tc.type: FUNC
 * @tc.require: I6XB42
 */
#ifdef SUPPORT_ACCESS_TOKEN
HWTEST_F(SystemAbilityMgrStubTest, GetCommonEventExtraDataIdlistInner001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = saMgr->GetCommonEventExtraDataIdlistInner(data, reply);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}
#endif

/**
 * @tc.name: ListSystemAbilityInner002
 * @tc.desc: test ListSystemAbilityInner, read dumpflag failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, ListSystemAbilityInner002, TestSize.Level3)
{
    SamMockPermission::MockPermission();
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t result = saMgr->ListSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_FLATTEN_OBJECT);
}

/**
 * @tc.name: ListSystemAbilityInner003
 * @tc.desc: test ListSystemAbilityInner, list empty!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, ListSystemAbilityInner003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t invalidDump = 1;
    data.WriteInt32(invalidDump);
    int32_t result = saMgr->ListSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: ListSystemAbilityInner004
 * @tc.desc: test ListSystemAbilityInner, list no empty!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, ListSystemAbilityInner004, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    SAInfo saInfo;
    saMgr->abilityMap_[SAID] = saInfo;
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t invalidDump = 1;
    data.WriteInt32(invalidDump);
    int32_t result = saMgr->ListSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: CheckRemtSystemAbilityInner002
 * @tc.desc: test CheckRemtSystemAbilityInner, read systemAbilityId failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, CheckRemtSystemAbilityInner002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(INVALID_SAID);
    int32_t result = saMgr->CheckRemtSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: CheckRemtSystemAbilityInner003
 * @tc.desc: test CheckRemtSystemAbilityInner, read deviceId failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, CheckRemtSystemAbilityInner003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    int32_t result = saMgr->CheckRemtSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_FLATTEN_OBJECT);
}

/**
 * @tc.name: CheckRemtSystemAbilityInner004
 * @tc.desc: test CheckRemtSystemAbilityInner!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, CheckRemtSystemAbilityInner004, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    std::string deviceId = "test";
    data.WriteString(deviceId);
    int32_t result = saMgr->CheckRemtSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: AddOndemandSystemAbilityInner002
 * @tc.desc: test AddOndemandSystemAbilityInner, read systemAbilityId failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddOndemandSystemAbilityInner002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(INVALID_SAID);
    int32_t result = saMgr->AddOndemandSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: AddOndemandSystemAbilityInner003
 * @tc.desc: test AddOndemandSystemAbilityInner, read localName failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddOndemandSystemAbilityInner003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    int32_t result = saMgr->AddOndemandSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: AddOndemandSystemAbilityInner004
 * @tc.desc: test AddOndemandSystemAbilityInner!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddOndemandSystemAbilityInner004, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    u16string localManagerName = u"test";
    data.WriteString16(localManagerName);
    int32_t result = saMgr->AddOndemandSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: UnmarshalingSaExtraProp001
 * @tc.desc: test UnmarshalingSaExtraProp, read isDistributed failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, UnmarshalingSaExtraProp001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    SystemAbilityManager::SAExtraProp extraProp;
    int32_t result = saMgr->UnmarshalingSaExtraProp(data, extraProp);
    EXPECT_EQ(result, ERR_FLATTEN_OBJECT);
}

/**
 * @tc.name: UnmarshalingSaExtraProp002
 * @tc.desc: test UnmarshalingSaExtraProp,  UnmarshalingSaExtraProp dumpFlags failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, UnmarshalingSaExtraProp002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    SystemAbilityManager::SAExtraProp extraProp;
    data.WriteBool(false);
    int32_t result = saMgr->UnmarshalingSaExtraProp(data, extraProp);
    EXPECT_EQ(result, ERR_FLATTEN_OBJECT);
}

/**
 * @tc.name: CheckSystemAbilityImmeInner001
 * @tc.desc: test CheckSystemAbilityImmeInner, read systemAbilityId failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, CheckSystemAbilityImmeInner001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(INVALID_SAID);
    int32_t result = saMgr->CheckSystemAbilityImmeInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: CheckSystemAbilityImmeInner002
 * @tc.desc: test CheckSystemAbilityImmeInner,  read isExist failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, CheckSystemAbilityImmeInner002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    int32_t result = saMgr->CheckSystemAbilityImmeInner(data, reply);
    EXPECT_EQ(result, ERR_FLATTEN_OBJECT);
}

/**
 * @tc.name: CheckSystemAbilityImmeInner003
 * @tc.desc: test CheckSystemAbilityImmeInner, WriteRemoteObject failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, CheckSystemAbilityImmeInner003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    data.WriteBool(false);
    int32_t result = saMgr->CheckSystemAbilityImmeInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: CheckSystemAbilityImmeInner004
 * @tc.desc: test CheckSystemAbilityImmeInner, penglai mode permission check failed!
 * @tc.type: FUNC
 */
#ifdef SUPPORT_PENGLAI_MODE
HWTEST_F(SystemAbilityMgrStubTest, CheckSystemAbilityImmeInner004, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    saMgr->SetPengLai(true);
    // set permission denied
    Penglai::permissionRet = false;
    int32_t result = saMgr->CheckSystemAbilityImmeInner(data, reply);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
    saMgr->SetPengLai(false);
}
#endif

/**
 * @tc.name: CheckSystemAbilityImmeInner005
 * @tc.desc: test CheckSystemAbilityImmeInner, penglai mode permission check success!
 * @tc.type: FUNC
 */
#ifdef SUPPORT_PENGLAI_MODE
HWTEST_F(SystemAbilityMgrStubTest, CheckSystemAbilityImmeInner005, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    saMgr->SetPengLai(true);
    // set permission true
    Penglai::permissionRet = true;
    int32_t result = saMgr->CheckSystemAbilityImmeInner(data, reply);
    EXPECT_NE(result, ERR_PERMISSION_DENIED);
    saMgr->SetPengLai(false);
}
#endif

/**
 * @tc.name: GetSystemAbilityInner001
 * @tc.desc: test GetSystemAbilityInner, read systemAbilityId failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, GetSystemAbilityInner001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t result = saMgr->GetSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: GetSystemAbilityInner002
 * @tc.desc: test GetSystemAbilityInner, penglai mode permission check failed!
 * @tc.type: FUNC
 */
#ifdef SUPPORT_PENGLAI_MODE
HWTEST_F(SystemAbilityMgrStubTest, GetSystemAbilityInner002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    saMgr->SetPengLai(true);
    // set permission denied
    Penglai::permissionRet = false;
    int32_t result = saMgr->GetSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
    saMgr->SetPengLai(false);
}
#endif

/**
 * @tc.name: GetSystemAbilityInner003
 * @tc.desc: test GetSystemAbilityInner, penglai mode permission check success!
 * @tc.type: FUNC
 */
#ifdef SUPPORT_PENGLAI_MODE
HWTEST_F(SystemAbilityMgrStubTest, GetSystemAbilityInner003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    saMgr->SetPengLai(true);
    // set permission true
    Penglai::permissionRet = true;
    int32_t result = saMgr->GetSystemAbilityInner(data, reply);
    EXPECT_NE(result, ERR_PERMISSION_DENIED);
    saMgr->SetPengLai(false);
}
#endif

/**
 * @tc.name: CheckSystemAbilityInner001
 * @tc.desc: test CheckSystemAbilityInner, read systemAbilityId failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, CheckSystemAbilityInner001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t result = saMgr->CheckSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: CheckSystemAbilityInner002
 * @tc.desc: test CheckSystemAbilityInner, penglai mode permission check failed!
 * @tc.type: FUNC
 */
#ifdef SUPPORT_PENGLAI_MODE
HWTEST_F(SystemAbilityMgrStubTest, CheckSystemAbilityInner002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    saMgr->SetPengLai(true);
    // set permission denied
    Penglai::permissionRet = false;
    int32_t result = saMgr->CheckSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
    saMgr->SetPengLai(false);
}
#endif

/**
 * @tc.name: CheckSystemAbilityInner003
 * @tc.desc: test CheckSystemAbilityInner, penglai mode permission check succes!
 * @tc.type: FUNC
 */
#ifdef SUPPORT_PENGLAI_MODE
HWTEST_F(SystemAbilityMgrStubTest, CheckSystemAbilityInner003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    saMgr->SetPengLai(true);
    // set permission true
    Penglai::permissionRet = true;
    int32_t result = saMgr->CheckSystemAbilityInner(data, reply);
    EXPECT_NE(result, ERR_PERMISSION_DENIED);
    saMgr->SetPengLai(false);
}
#endif

/**
 * @tc.name: AddSystemAbilityInner002
 * @tc.desc: test AddSystemAbilityInner, read systemAbilityId failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddSystemAbilityInner002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(INVALID_SAID);
    int32_t result = saMgr->AddSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: AddSystemAbilityInner003
 * @tc.desc: test AddSystemAbilityInner, readParcelable failed!!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddSystemAbilityInner003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    int32_t result = saMgr->AddSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: AddSystemAbilityInner004
 * @tc.desc: test AddSystemAbilityInner, UnmarshalingSaExtraProp failed!!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddSystemAbilityInner004, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<IRemoteObject> testAbility(new SaStatusChangeMock());
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    data.WriteRemoteObject(testAbility);
    int32_t result = saMgr->AddSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_FLATTEN_OBJECT);
}

/**
 * @tc.name: AddSystemAbilityInner005
 * @tc.desc: test AddSystemAbilityInner!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddSystemAbilityInner005, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    sptr<IRemoteObject> testAbility(new SaStatusChangeMock());
    EXPECT_TRUE(saMgr != nullptr);
    SystemAbilityManager::SAExtraProp extraProp;
    bool isExist = false;
    int32_t dumpFlags = 0;
    std::u16string capability = u"capability";
    std::u16string permission = u"permission";
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(SAID);
    data.WriteRemoteObject(testAbility);
    data.WriteBool(isExist);
    data.WriteInt32(dumpFlags);
    data.WriteString16(capability);
    data.WriteString16(permission);
    int32_t result = saMgr->AddSystemAbilityInner(data, reply);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: Test GetSystemProcessInfoInner002
 * @tc.desc: GetSystemProcessInfoInner002, permission denied!
 * @tc.type: FUNC
 * @tc.require: I7VQQG
 */
HWTEST_F(SystemAbilityMgrStubTest, GetSystemProcessInfoInner002, TestSize.Level3)
{
    DTEST_LOG << "GetSystemProcessInfoInner002" << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(INVALID_SAID);
    int32_t ret = saMgr->GetSystemProcessInfoInner(data, reply);
    EXPECT_EQ(ret, ERR_NULL_OBJECT);
}

/**
 * @tc.name: Test GetSystemProcessInfoInner003
 * @tc.desc: GetSystemProcessInfoInner003, permission denied!
 * @tc.type: FUNC
 * @tc.require: I7VQQG
 */
HWTEST_F(SystemAbilityMgrStubTest, GetSystemProcessInfoInner003, TestSize.Level3)
{
    DTEST_LOG << "GetSystemProcessInfoInner003" << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = saMgr->GetSystemProcessInfoInner(data, reply);
    EXPECT_EQ(ret, ERR_NULL_OBJECT);
}

/**
 * @tc.name: AddSystemProcessInner002
 * @tc.desc: test AddSystemProcessInner, read process name failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddSystemProcessInner002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t result = saMgr->AddSystemProcessInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: AddSystemProcessInner003
 * @tc.desc: test AddSystemProcessInner readParcelable failed!!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddSystemProcessInner003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    std::u16string procName = u"test";
    data.WriteString16(procName);
    int32_t result = saMgr->AddSystemProcessInner(data, reply);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: AddSystemProcessInner004
 * @tc.desc: test AddSystemProcessInner!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, AddSystemProcessInner004, TestSize.Level1)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    sptr<IRemoteObject> testAbility(new SaStatusChangeMock());
    MessageParcel data;
    MessageParcel reply;
    std::u16string procName = u"test";
    data.WriteString16(procName);
    data.WriteRemoteObject(testAbility);
    int32_t result = saMgr->AddSystemProcessInner(data, reply);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: Test GetRunningSystemProcessInner002
 * @tc.desc: GetRunningSystemProcessInner002
 * @tc.type: FUNC
 * @tc.require: I6H10P
 */
HWTEST_F(SystemAbilityMgrStubTest, GetRunningSystemProcessInner002, TestSize.Level3)
{
    DTEST_LOG << "GetRunningSystemProcessInner002" << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = saMgr->GetRunningSystemProcessInner(data, reply);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: Test SubscribeSystemProcessInner002
 * @tc.desc: SubscribeSystemProcessInner002
 * @tc.type: FUNC
 * @tc.require: I6H10P
 */
HWTEST_F(SystemAbilityMgrStubTest, SubscribeSystemProcessInner002, TestSize.Level3)
{
    DTEST_LOG << "SubscribeSystemProcessInner002" << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    data.WriteRemoteObject(nullptr);
    MessageParcel reply;
    int32_t ret = saMgr->SubscribeSystemProcessInner(data, reply);
    EXPECT_EQ(ret, ERR_NULL_OBJECT);
}

/**
 * @tc.name: Test SubscribeSystemProcessInner003
 * @tc.desc: SubscribeSystemProcessInner003
 * @tc.type: FUNC
 * @tc.require: I6H10P
 */
HWTEST_F(SystemAbilityMgrStubTest, SubscribeSystemProcessInner003, TestSize.Level3)
{
    DTEST_LOG << "SubscribeSystemProcessInner003" << std::endl;
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    sptr<IRemoteObject> ptr = new SystemProcessStatusChange();
    data.WriteRemoteObject(ptr);
    MessageParcel reply;
    int32_t ret = saMgr->SubscribeSystemProcessInner(data, reply);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: SendStrategyInner001
 * @tc.desc: test SendStrategyInner, read systemAbilityId failed!
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilityMgrStubTest, SendStrategyInner001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    EXPECT_TRUE(saMgr != nullptr);
    MessageParcel data;
    MessageParcel reply;
    int32_t result = saMgr->SendStrategyInner(data, reply);
    EXPECT_EQ(result, ERR_FLATTEN_OBJECT);
}

/**
 * @tc.name: GetLocalAbilityManagerProxyInner001
 * @tc.desc: test GetLocalAbilityManagerProxyInner
 * @tc.type: FUNC
 * @tc.require: I6XB42
 */
HWTEST_F(SystemAbilityMgrStubTest, GetLocalAbilityManagerProxyInner001, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = saMgr->GetLocalAbilityManagerProxyInner(data, reply);
    EXPECT_EQ(ret, ERR_NULL_OBJECT);
}

/**
 * @tc.name: GetLocalAbilityManagerProxyInner002
 * @tc.desc: test GetLocalAbilityManagerProxyInner
 * @tc.type: FUNC
 * @tc.require: I6XB42
 */
HWTEST_F(SystemAbilityMgrStubTest, GetLocalAbilityManagerProxyInner002, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    EXPECT_TRUE(data.WriteInt32(INVALID_SAID));
    int32_t ret = saMgr->GetLocalAbilityManagerProxyInner(data, reply);
    EXPECT_EQ(ret, ERR_NULL_OBJECT);
}

/**
 * @tc.name: GetLocalAbilityManagerProxyInner003
 * @tc.desc: test GetLocalAbilityManagerProxyInner
 * @tc.type: FUNC
 * @tc.require: I6XB42
 */
HWTEST_F(SystemAbilityMgrStubTest, GetLocalAbilityManagerProxyInner003, TestSize.Level3)
{
    sptr<SystemAbilityManager> saMgr = SystemAbilityManager::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    EXPECT_TRUE(data.WriteInt32(SAID));
    int32_t ret = saMgr->GetLocalAbilityManagerProxyInner(data, reply);
    EXPECT_EQ(ret, ERR_NULL_OBJECT);
}
}