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

#include "manual_ondemand_helper.h"

#include <iostream>
#include <memory>
#include <thread>
#include <unistd.h>

#include "datetime_ex.h"
#include "errors.h"
#include "if_system_ability_manager.h"
#include "ipc_types.h"
#include "iremote_object.h"
#include "iservice_registry.h"
#include "isystem_ability_load_callback.h"
#include "nativetoken_kit.h"
#include "sam_mock_permission.h"
#include "softbus_bus_center.h"
#include "system_ability_ondemand_reason.h"
#include "system_ability_definition.h"
#include "token_setproc.h"
#include "parameter.h"
#include "parameters.h"

using namespace OHOS;
using namespace std;

namespace OHOS {
namespace {
constexpr int32_t LOOP_TIME = 1000;
constexpr int32_t MOCK_ONDEMAND_ABILITY_A = 1494;
constexpr int32_t MOCK_ONDEMAND_ABILITY_B = 1497;
constexpr int32_t MOCK_ONDEMAND_ABILITY_C = 1499;
constexpr int32_t SLEEP_1_SECONDS = 1 * 1000 * 1000;
constexpr int32_t SLEEP_3_SECONDS = 3 * 1000 * 1000;
constexpr int32_t SLEEP_6_SECONDS = 6 * 1000 * 1000;
std::string g_inputTimeStr = "2023-10-9-10:00:00"; // time format
}

OnDemandHelper::OnDemandHelper()
{
    loadCallback_ = new OnDemandLoadCallback();
    loadCallback2_ = new OnDemandLoadCallback();
    loadCallback3_ = new OnDemandLoadCallback();
    loadCallback4_ = new OnDemandLoadCallback();
}

OnDemandHelper& OnDemandHelper::GetInstance()
{
    static OnDemandHelper instance;
    return instance;
}

void OnDemandHelper::GetSystemProcess()
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sm == nullptr) {
        cout << "GetSystemAbilityManager samgr object null!" << endl;
        return;
    }
    std::list<SystemProcessInfo> systemProcessInfos;
    int32_t ret = sm->GetRunningSystemProcess(systemProcessInfos);
    if (ret != ERR_OK) {
        cout << "GetRunningSystemProcess failed" << endl;
        return;
    }
    cout << "GetRunningSystemProcess size: "<< systemProcessInfos.size() << endl;
    for (const auto& systemProcessInfo : systemProcessInfos) {
        cout << "processName: " << systemProcessInfo.processName << " pid:" << systemProcessInfo.pid << endl;
    }
}

void OnDemandHelper::InitSystemProcessStatusChange()
{
    systemProcessStatusChange_ = new SystemProcessStatusChange();
}

void OnDemandHelper::SubscribeSystemProcess()
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sm == nullptr) {
        cout << "GetSystemAbilityManager samgr object null!" << endl;
        return;
    }
    int32_t ret = sm->SubscribeSystemProcess(systemProcessStatusChange_);
    if (ret != ERR_OK) {
        cout << "SubscribeSystemProcess failed" << endl;
        return;
    }
    cout << "SubscribeSystemProcess success" << endl;
}

void OnDemandHelper::UnSubscribeSystemProcess()
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sm == nullptr) {
        cout << "GetSystemAbilityManager samgr object null!" << endl;
        return;
    }
    int32_t ret = sm->UnSubscribeSystemProcess(systemProcessStatusChange_);
    if (ret != ERR_OK) {
        cout << "UnSubscribeSystemProcess failed" << endl;
        return;
    }
    cout << "UnSubscribeSystemProcess success" << endl;
}

void OnDemandHelper::SystemProcessStatusChange::OnSystemProcessStarted(SystemProcessInfo& systemProcessInfo)
{
    cout << "OnSystemProcessStarted, processName: " << systemProcessInfo.processName << " pid:"
        << systemProcessInfo.pid << " uid:" << systemProcessInfo.uid << endl;
}

void OnDemandHelper::SystemProcessStatusChange::OnSystemProcessStopped(SystemProcessInfo& systemProcessInfo)
{
    cout << "OnSystemProcessStopped, processName: " << systemProcessInfo.processName << " pid:"
        << systemProcessInfo.pid << " uid:" << systemProcessInfo.uid << endl;
}

int32_t OnDemandHelper::LoadSystemAbility(int32_t systemAbilityId, const sptr<ISystemAbilityLoadCallback>& callback)
{
    int64_t begin = GetTickCount();
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sm == nullptr) {
        cout << "GetSystemAbilityManager samgr object null!" << endl;
        return ERR_NULL_OBJECT;
    }
    int32_t result = sm->LoadSystemAbility(systemAbilityId, callback);
    if (result != ERR_OK) {
        cout << "systemAbilityId:" << systemAbilityId << " unload failed, result code:" << result << endl;
        return result;
    }
    cout << "LoadSystemAbility result:" << result << " spend:" << (GetTickCount() - begin) << " ms"
            << " systemAbilityId:" << systemAbilityId << endl;
    return ERR_OK;
}

int32_t OnDemandHelper::UnloadSystemAbility(int32_t systemAbilityId)
{
    SamMockPermission::MockProcess("listen_test");
    int64_t begin = GetTickCount();
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sm == nullptr) {
        cout << "GetSystemAbilityManager samgr object null!" << endl;
        return ERR_NULL_OBJECT;
    }
    int32_t result = sm->UnloadSystemAbility(systemAbilityId);
    if (result != ERR_OK) {
        cout << "systemAbilityId:" << systemAbilityId << " unload failed, result code:" << result << endl;
        return result;
    }
    cout << "UnloadSystemAbility result:" << result << " spend:" << (GetTickCount() - begin) << " ms"
            << " systemAbilityId:" << systemAbilityId << endl;
    return ERR_OK;
}

int32_t OnDemandHelper::OnDemandAbility(int32_t systemAbilityId)
{
    int64_t begin = GetTickCount();
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sm == nullptr) {
        cout << "GetSystemAbilityManager samgr object null!" << endl;
        return ERR_NULL_OBJECT;
    }
    int32_t result = sm->LoadSystemAbility(systemAbilityId, loadCallback_);
    if (result != ERR_OK) {
        cout << "systemAbilityId:" << systemAbilityId << " load failed, result code:" << result << endl;
        return result;
    }
    cout << "LoadSystemAbility result:" << result << " spend:" << (GetTickCount() - begin) << " ms"
            << " systemAbilityId:" << systemAbilityId << endl;
    return ERR_OK;
}

void OnDemandHelper::GetSystemProcessInfo(int32_t systemAbilityId)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sm == nullptr) {
        cout << "GetSystemAbilityManager samgr object null!" << endl;
        return;
    }
    SystemProcessInfo processInfo;
    int32_t ret = sm->GetSystemProcessInfo(systemAbilityId, processInfo);
    if (ret != ERR_OK) {
        cout << "GetSystemProcessInfo failed" << endl;
        return;
    }
    cout << "processName: " << processInfo.processName << "pid:" << processInfo.pid << endl;
}

sptr<IRemoteObject> OnDemandHelper::SyncOnDemandAbility(int32_t systemAbilityId)
{
    int64_t begin = GetTickCount();
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sm == nullptr) {
        cout << "GetSystemAbilityManager samgr object null!" << endl;
        return nullptr;
    }
    int32_t timeout = 4;
    sptr<IRemoteObject> result = sm->LoadSystemAbility(systemAbilityId, timeout);
    if (result == nullptr) {
        cout << "systemAbilityId:" << systemAbilityId << "syncload failed,result code" << result << endl;
        return nullptr;
    }
    cout << "SyncLoadSystemAbility result" << result << "spend:" << (GetTickCount() - begin) << "ms"
        << "systemAbilityId:" << systemAbilityId << endl;
    return result;
}

int32_t OnDemandHelper::TestSyncOnDemandAbility(int32_t systemAbilityId)
{
    sptr<IRemoteObject> ptr = SyncOnDemandAbility(systemAbilityId);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    int32_t getdp = 2;
    int32_t errCode = ptr->SendRequest(getdp, data, reply, option);
    if (errCode != ERR_NONE) {
        cout << "transact failed,errCode = " << errCode;
        return errCode;
    }
    int32_t ret = reply.ReadInt32();
    cout << "ret = " << ret;
    return ret;
}


int32_t OnDemandHelper::LoadOndemandAbilityCase1()
{
    cout << "LoadOndemandAbilityCase1 start" << endl;
    int64_t begin = GetTickCount();
    sptr<OnDemandLoadCallback> callback1 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_A, callback1);
    usleep(SLEEP_3_SECONDS);
    cout << "LoadOndemandAbilityCase1 done, spend:" << (GetTickCount() - begin) << " ms" << endl;
    return ERR_OK;
}

int32_t OnDemandHelper::LoadOndemandAbilityCase2()
{
    cout << "LoadOndemandAbilityCase2 start" << endl;
    int64_t begin = GetTickCount();
    sptr<OnDemandLoadCallback> callback1 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_A, callback1);
    sptr<OnDemandLoadCallback> callback2 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_A, callback2);
    usleep(SLEEP_3_SECONDS);
    cout << "LoadOndemandAbilityCase2 done, spend:" << (GetTickCount() - begin) << " ms" << endl;
    return ERR_OK;
}

int32_t OnDemandHelper::LoadOndemandAbilityCase3()
{
    cout << "LoadOndemandAbilityCase3 start" << endl;
    int64_t begin = GetTickCount();
    sptr<OnDemandLoadCallback> callback1 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_A, callback1);
    UnloadSystemAbility(MOCK_ONDEMAND_ABILITY_A);
    sptr<OnDemandLoadCallback> callback2 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_A, callback2);
    usleep(SLEEP_3_SECONDS);
    cout << "LoadOndemandAbilityCase3 done, spend:" << (GetTickCount() - begin) << " ms" << endl;
    return ERR_OK;
}

int32_t OnDemandHelper::LoadOndemandAbilityCase4()
{
    cout << "LoadOndemandAbilityCase4 start" << endl;
    int64_t begin = GetTickCount();
    sptr<OnDemandLoadCallback> callback1 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_A, callback1);
    usleep(SLEEP_3_SECONDS);
    sptr<OnDemandLoadCallback> callback2 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_A, callback2);
    usleep(SLEEP_3_SECONDS);
    cout << "LoadOndemandAbilityCase4 done, spend:" << (GetTickCount() - begin) << " ms" << endl;
    return ERR_OK;
}

int32_t OnDemandHelper::LoadOndemandAbilityCase5()
{
    cout << "LoadOndemandAbilityCase5 start" << endl;
    int64_t begin = GetTickCount();
    sptr<OnDemandLoadCallback> callback1 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_A, callback1);
    usleep(SLEEP_3_SECONDS);
    UnloadSystemAbility(MOCK_ONDEMAND_ABILITY_A);
    sptr<OnDemandLoadCallback> callback2 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_A, callback2);
    usleep(SLEEP_3_SECONDS);
    cout << "LoadOndemandAbilityCase5 done, spend:" << (GetTickCount() - begin) << " ms" << endl;
    return ERR_OK;
}

int32_t OnDemandHelper::LoadOndemandAbilityCase6()
{
    cout << "LoadOndemandAbilityCase6 start" << endl;
    int64_t begin = GetTickCount();
    sptr<OnDemandLoadCallback> callback1 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_A, callback1);
    usleep(SLEEP_3_SECONDS);
    sptr<OnDemandLoadCallback> callback2 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_B, callback2);
    usleep(SLEEP_3_SECONDS);
    UnloadSystemAbility(MOCK_ONDEMAND_ABILITY_A);
    sptr<OnDemandLoadCallback> callback3 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_A, callback3);
    usleep(SLEEP_3_SECONDS);
    cout << "LoadOndemandAbilityCase6 done, spend:" << (GetTickCount() - begin) << " ms" << endl;
    return ERR_OK;
}

int32_t OnDemandHelper::LoadOndemandAbilityCase7()
{
    cout << "LoadOndemandAbilityCase7 start" << endl;
    int64_t begin = GetTickCount();
    sptr<OnDemandLoadCallback> callback1 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_A, callback1);
    usleep(SLEEP_3_SECONDS);
    UnloadSystemAbility(MOCK_ONDEMAND_ABILITY_A);
    usleep(SLEEP_1_SECONDS);
    sptr<OnDemandLoadCallback> callback2 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_A, callback2);
    usleep(SLEEP_6_SECONDS);
    cout << "LoadOndemandAbilityCase7 done, spend:" << (GetTickCount() - begin) << " ms" << endl;
    return ERR_OK;
}

int32_t OnDemandHelper::LoadOndemandAbilityCase8()
{
    cout << "LoadOndemandAbilityCase8 start" << endl;
    int64_t begin = GetTickCount();
    sptr<OnDemandLoadCallback> callback1 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_A, callback1);
    usleep(SLEEP_3_SECONDS);
    sptr<OnDemandLoadCallback> callback2 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_B, callback2);
    usleep(SLEEP_3_SECONDS);
    UnloadSystemAbility(MOCK_ONDEMAND_ABILITY_A);
    UnloadSystemAbility(MOCK_ONDEMAND_ABILITY_B);
    usleep(SLEEP_1_SECONDS);
    sptr<OnDemandLoadCallback> callback3 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_B, callback3);
    usleep(SLEEP_6_SECONDS);
    cout << "LoadOndemandAbilityCase8 done, spend:" << (GetTickCount() - begin) << " ms" << endl;
    return ERR_OK;
}

int32_t OnDemandHelper::LoadOndemandAbilityCase9()
{
    cout << "LoadOndemandAbilityCase9 start" << endl;
    int64_t begin = GetTickCount();
    sptr<OnDemandLoadCallback> callback1 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_C, callback1);
    usleep(SLEEP_6_SECONDS);
    cout << "LoadOndemandAbilityCase9 done, spend:" << (GetTickCount() - begin) << " ms" << endl;
    return ERR_OK;
}

int32_t OnDemandHelper::LoadOndemandAbilityCase10()
{
    cout << "LoadOndemandAbilityCase10 start" << endl;
    int64_t begin = GetTickCount();
    sptr<OnDemandLoadCallback> callback1 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_A, callback1);
    usleep(SLEEP_3_SECONDS);
    UnloadSystemAbility(MOCK_ONDEMAND_ABILITY_A);
    usleep(SLEEP_1_SECONDS);
    sptr<OnDemandLoadCallback> callback2 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_A, callback2);
    sptr<OnDemandLoadCallback> callback3 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_A, callback3);
    sptr<OnDemandLoadCallback> callback4 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_A, callback4);
    sptr<OnDemandLoadCallback> callback5 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_A, callback5);
    sptr<OnDemandLoadCallback> callback6 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_A, callback6);
    usleep(SLEEP_6_SECONDS);
    usleep(SLEEP_3_SECONDS);
    cout << "LoadOndemandAbilityCase10 done, spend:" << (GetTickCount() - begin) << " ms" << endl;
    return ERR_OK;
}

int32_t OnDemandHelper::LoadOndemandAbilityCase11()
{
    cout << "LoadOndemandAbilityCase10 start" << endl;
    int64_t begin = GetTickCount();
    sptr<OnDemandLoadCallback> callback1 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_A, callback1);
    usleep(SLEEP_3_SECONDS);
    sptr<OnDemandLoadCallback> callback2 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_A, callback2);
    sptr<OnDemandLoadCallback> callback3 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_A, callback3);
    sptr<OnDemandLoadCallback> callback4 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_A, callback4);
    sptr<OnDemandLoadCallback> callback5 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_A, callback5);
    sptr<OnDemandLoadCallback> callback6 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_A, callback6);
    usleep(SLEEP_6_SECONDS);
    usleep(SLEEP_3_SECONDS);
    cout << "LoadOndemandAbilityCase10 done, spend:" << (GetTickCount() - begin) << " ms" << endl;
    return ERR_OK;
}

int32_t OnDemandHelper::UnloadOndemandAbilityCase1()
{
    cout << "UnloadOndemandAbilityCase1 start" << endl;
    int64_t begin = GetTickCount();
    UnloadSystemAbility(MOCK_ONDEMAND_ABILITY_A);
    cout << "UnloadOndemandAbilityCase1 done, spend:" << (GetTickCount() - begin) << " ms" << endl;
    return ERR_OK;
}

int32_t OnDemandHelper::UnloadOndemandAbilityCase2()
{
    cout << "UnloadOndemandAbilityCase2 start" << endl;
    int64_t begin = GetTickCount();
    sptr<OnDemandLoadCallback> callback1 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_A, callback1);
    UnloadSystemAbility(MOCK_ONDEMAND_ABILITY_A);
    usleep(SLEEP_6_SECONDS);
    cout << "UnloadOndemandAbilityCase2 done, spend:" << (GetTickCount() - begin) << " ms" << endl;
    return ERR_OK;
}

int32_t OnDemandHelper::UnloadOndemandAbilityCase3()
{
    cout << "UnloadOndemandAbilityCase3 start" << endl;
    int64_t begin = GetTickCount();
    sptr<OnDemandLoadCallback> callback1 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_A, callback1);
    usleep(SLEEP_3_SECONDS);
    UnloadSystemAbility(MOCK_ONDEMAND_ABILITY_A);
    usleep(SLEEP_3_SECONDS);
    cout << "UnloadOndemandAbilityCase3 done, spend:" << (GetTickCount() - begin) << " ms" << endl;
    return ERR_OK;
}

int32_t OnDemandHelper::UnloadOndemandAbilityCase4()
{
    cout << "UnloadOndemandAbilityCase4 start" << endl;
    int64_t begin = GetTickCount();
    sptr<OnDemandLoadCallback> callback1 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_A, callback1);
    usleep(SLEEP_3_SECONDS);
    sptr<OnDemandLoadCallback> callback2 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_B, callback2);
    usleep(SLEEP_3_SECONDS);
    UnloadSystemAbility(MOCK_ONDEMAND_ABILITY_A);
    usleep(SLEEP_3_SECONDS);
    cout << "UnloadOndemandAbilityCase4 done, spend:" << (GetTickCount() - begin) << " ms" << endl;
    return ERR_OK;
}

int32_t OnDemandHelper::UnloadOndemandAbilityCase5()
{
    cout << "UnloadOndemandAbilityCase5 start" << endl;
    int64_t begin = GetTickCount();
    sptr<OnDemandLoadCallback> callback1 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_A, callback1);
    usleep(SLEEP_3_SECONDS);
    sptr<OnDemandLoadCallback> callback2 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_B, callback2);
    usleep(SLEEP_3_SECONDS);
    UnloadSystemAbility(MOCK_ONDEMAND_ABILITY_A);
    usleep(SLEEP_1_SECONDS);
    UnloadSystemAbility(MOCK_ONDEMAND_ABILITY_A);
    usleep(SLEEP_3_SECONDS);
    cout << "UnloadOndemandAbilityCase5 done, spend:" << (GetTickCount() - begin) << " ms" << endl;
    return ERR_OK;
}

int32_t OnDemandHelper::UnloadOndemandAbilityCase6()
{
    cout << "UnloadOndemandAbilityCase6 start" << endl;
    int64_t begin = GetTickCount();
    sptr<OnDemandLoadCallback> callback1 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_A, callback1);
    usleep(SLEEP_3_SECONDS);
    UnloadSystemAbility(MOCK_ONDEMAND_ABILITY_A);
    usleep(SLEEP_1_SECONDS);
    UnloadSystemAbility(MOCK_ONDEMAND_ABILITY_A);
    usleep(SLEEP_3_SECONDS);
    cout << "UnloadOndemandAbilityCase6 done, spend:" << (GetTickCount() - begin) << " ms" << endl;
    return ERR_OK;
}

int32_t OnDemandHelper::UnloadOndemandAbilityCase7()
{
    cout << "UnloadOndemandAbilityCase7 start" << endl;
    int64_t begin = GetTickCount();
    sptr<OnDemandLoadCallback> callback1 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_C, callback1);
    usleep(SLEEP_6_SECONDS);
    UnloadSystemAbility(MOCK_ONDEMAND_ABILITY_C);
    usleep(SLEEP_6_SECONDS);
    cout << "UnloadOndemandAbilityCase7 done, spend:" << (GetTickCount() - begin) << " ms" << endl;
    return ERR_OK;
}

int32_t OnDemandHelper::UnloadOndemandAbilityCase8()
{
    cout << "UnloadOndemandAbilityCase6 start" << endl;
    int64_t begin = GetTickCount();
    sptr<OnDemandLoadCallback> callback1 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_A, callback1);
    usleep(SLEEP_3_SECONDS);
    UnloadSystemAbility(MOCK_ONDEMAND_ABILITY_A);
    usleep(SLEEP_1_SECONDS);
    UnloadSystemAbility(MOCK_ONDEMAND_ABILITY_A);
    UnloadSystemAbility(MOCK_ONDEMAND_ABILITY_A);
    UnloadSystemAbility(MOCK_ONDEMAND_ABILITY_A);
    UnloadSystemAbility(MOCK_ONDEMAND_ABILITY_A);
    UnloadSystemAbility(MOCK_ONDEMAND_ABILITY_A);
    usleep(SLEEP_3_SECONDS);
    cout << "UnloadOndemandAbilityCase6 done, spend:" << (GetTickCount() - begin) << " ms" << endl;
    return ERR_OK;
}

int32_t OnDemandHelper::UnloadOndemandAbilityCase9()
{
    cout << "UnloadOndemandAbilityCase6 start" << endl;
    int64_t begin = GetTickCount();
    sptr<OnDemandLoadCallback> callback1 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_A, callback1);
    usleep(SLEEP_3_SECONDS);
    UnloadSystemAbility(MOCK_ONDEMAND_ABILITY_A);
    usleep(SLEEP_1_SECONDS);
    UnloadSystemAbility(MOCK_ONDEMAND_ABILITY_A);
    UnloadSystemAbility(MOCK_ONDEMAND_ABILITY_A);
    UnloadSystemAbility(MOCK_ONDEMAND_ABILITY_A);
    UnloadSystemAbility(MOCK_ONDEMAND_ABILITY_A);
    usleep(SLEEP_3_SECONDS);
    cout << "UnloadOndemandAbilityCase6 done, spend:" << (GetTickCount() - begin) << " ms" << endl;
    return ERR_OK;
}

int32_t OnDemandHelper::GetOndemandAbilityCase1()
{
    cout << "GetOndemandAbilityCase1 start" << endl;
    int64_t begin = GetTickCount();
    GetSystemAbility(MOCK_ONDEMAND_ABILITY_A);
    usleep(SLEEP_3_SECONDS);
    cout << "GetOndemandAbilityCase1 done, spend:" << (GetTickCount() - begin) << " ms" << endl;
    return ERR_OK;
}

int32_t OnDemandHelper::GetOndemandAbilityCase2()
{
    cout << "GetOndemandAbilityCase2 start" << endl;
    int64_t begin = GetTickCount();
    sptr<OnDemandLoadCallback> callback1 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_B, callback1);
    usleep(SLEEP_3_SECONDS);
    GetSystemAbility(MOCK_ONDEMAND_ABILITY_A);
    usleep(SLEEP_3_SECONDS);
    cout << "GetOndemandAbilityCase2 done, spend:" << (GetTickCount() - begin) << " ms" << endl;
    return ERR_OK;
}

int32_t OnDemandHelper::GetOndemandAbilityCase3()
{
    cout << "GetOndemandAbilityCase3 start" << endl;
    int64_t begin = GetTickCount();
    sptr<OnDemandLoadCallback> callback1 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_A, callback1);
    GetSystemAbility(MOCK_ONDEMAND_ABILITY_A);
    usleep(SLEEP_3_SECONDS);
    cout << "GetOndemandAbilityCase3 done, spend:" << (GetTickCount() - begin) << " ms" << endl;
    return ERR_OK;
}

int32_t OnDemandHelper::GetOndemandAbilityCase4()
{
    cout << "GetOndemandAbilityCase4 start" << endl;
    int64_t begin = GetTickCount();
    sptr<OnDemandLoadCallback> callback1 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_A, callback1);
    usleep(SLEEP_3_SECONDS);
    GetSystemAbility(MOCK_ONDEMAND_ABILITY_A);
    usleep(SLEEP_3_SECONDS);
    cout << "GetOndemandAbilityCase4 done, spend:" << (GetTickCount() - begin) << " ms" << endl;
    return ERR_OK;
}

int32_t OnDemandHelper::GetOndemandAbilityCase5()
{
    cout << "GetOndemandAbilityCase5 start" << endl;
    int64_t begin = GetTickCount();
    sptr<OnDemandLoadCallback> callback1 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_A, callback1);
    usleep(SLEEP_3_SECONDS);
    sptr<OnDemandLoadCallback> callback2 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_B, callback2);
    usleep(SLEEP_3_SECONDS);
    UnloadSystemAbility(MOCK_ONDEMAND_ABILITY_A);
    usleep(SLEEP_1_SECONDS);
    GetSystemAbility(MOCK_ONDEMAND_ABILITY_A);
    cout << "GetOndemandAbilityCase5 done, spend:" << (GetTickCount() - begin) << " ms" << endl;
    return ERR_OK;
}

int32_t OnDemandHelper::GetOndemandAbilityCase6()
{
    cout << "GetOndemandAbilityCase6 start" << endl;
    int64_t begin = GetTickCount();
    sptr<OnDemandLoadCallback> callback1 = new OnDemandLoadCallback();
    LoadSystemAbility(MOCK_ONDEMAND_ABILITY_A, callback1);
    usleep(SLEEP_3_SECONDS);
    UnloadSystemAbility(MOCK_ONDEMAND_ABILITY_A);
    usleep(SLEEP_1_SECONDS);
    GetSystemAbility(MOCK_ONDEMAND_ABILITY_A);
    cout << "GetOndemandAbilityCase6 done, spend:" << (GetTickCount() - begin) << " ms" << endl;
    return ERR_OK;
}

void TestLoad1(OHOS::OnDemandHelper& ondemandHelper)
{
    std::string pause;
    cout << "input any word to start load test case 1" << endl;
    cin >> pause;
    ondemandHelper.LoadOndemandAbilityCase1();
    cout << "input any word to start load test case 2" << endl;
    cin >> pause;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.LoadOndemandAbilityCase2();
    cout << "input any word to start load test case 3" << endl;
    cin >> pause;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.LoadOndemandAbilityCase3();
    cout << "input any word to start load test case 4" << endl;
    cin >> pause;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.LoadOndemandAbilityCase4();
}

void TestLoad2(OHOS::OnDemandHelper& ondemandHelper)
{
    std::string pause;
    cout << "input any word to start load test case 6" << endl;
    cin >> pause;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.LoadOndemandAbilityCase6();
    cout << "input any word to start load test case 7" << endl;
    cin >> pause;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.LoadOndemandAbilityCase7();
    cout << "input any word to start load test case 8" << endl;
    cin >> pause;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.LoadOndemandAbilityCase8();
    cout << "input any word to start load test case 9" << endl;
    cin >> pause;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.LoadOndemandAbilityCase9();
    cout << "input any word to start load test case 10" << endl;
    cin >> pause;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.LoadOndemandAbilityCase10();
    cout << "input any word to start load test case 11" << endl;
    cin >> pause;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.LoadOndemandAbilityCase11();
}

void TestUnload(OHOS::OnDemandHelper& ondemandHelper)
{
    std::string pause;
    cout << "input any word to start unload test case 1" << endl;
    cin >> pause;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.UnloadOndemandAbilityCase1();
    cout << "input any word to start unload test case 2" << endl;
    cin >> pause;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.UnloadOndemandAbilityCase2();
    cout << "input any word to start unload test case 3" << endl;
    cin >> pause;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.UnloadOndemandAbilityCase3();
    cout << "input any word to start unload test case 4" << endl;
    cin >> pause;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.UnloadOndemandAbilityCase4();
    cout << "input any word to start unload test case 5" << endl;
    cin >> pause;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.UnloadOndemandAbilityCase5();
    cout << "input any word to start unload test case 6" << endl;
    cin >> pause;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.UnloadOndemandAbilityCase6();
    cout << "input any word to start unload test case 7" << endl;
    cin >> pause;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.UnloadOndemandAbilityCase7();
    cout << "input any word to start unload test case 8" << endl;
    cin >> pause;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.UnloadOndemandAbilityCase8();
    cout << "input any word to start unload test case 9" << endl;
    cin >> pause;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.UnloadOndemandAbilityCase9();
}

void TestGet(OHOS::OnDemandHelper& ondemandHelper)
{
    std::string pause;
    cout << "input any word to start get test case 1" << endl;
    cin >> pause;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.GetOndemandAbilityCase1();
    cout << "input any word to start get test case 2" << endl;
    cin >> pause;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.GetOndemandAbilityCase2();
    cout << "input any word to start get test case 3" << endl;
    cin >> pause;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.GetOndemandAbilityCase3();
    cout << "input any word to start get test case 4" << endl;
    cin >> pause;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.GetOndemandAbilityCase4();
    cout << "input any word to start get test case 5" << endl;
    cin >> pause;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.GetOndemandAbilityCase5();
    cout << "input any word to start get test case 6" << endl;
    cin >> pause;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.GetOndemandAbilityCase6();
}

void TestScheduler(OHOS::OnDemandHelper& ondemandHelper)
{
    SamMockPermission::MockProcess("listen_test");
    
    TestLoad1(ondemandHelper);
    TestLoad2(ondemandHelper);
    TestUnload(ondemandHelper);
    TestGet(ondemandHelper);

    cout << "all test case done" << endl;
    ::system("kill -9 `pidof listen_test`");
}

int32_t OnDemandHelper::UnloadAllIdleSystemAbility()
{
    SamMockPermission::MockProcess("memmgrservice");
    int64_t begin = GetTickCount();
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sm == nullptr) {
        cout << "UnloadAllIdleSystemAbility samgr object null!" << endl;
        return ERR_NULL_OBJECT;
    }
    int32_t result = sm->UnloadAllIdleSystemAbility();
    if (result != ERR_OK) {
        cout << "UnloadAllIdleSystemAbility failed, result code:" << result << endl;
        return result;
    }
    cout << "UnloadAllIdleSystemAbility result:" << result << GetTickCount() - begin << endl;
    return ERR_OK;
}

void OnDemandHelper::GetDeviceList()
{
    NodeBasicInfo *info = NULL;
    int32_t infoNum = 0;
    int32_t ret = GetAllNodeDeviceInfo("ondemand", &info, &infoNum);
    if (ret != 0) {
        cout << "get remote deviceid GetAllNodeDeviceInfo failed" << endl;
        return;
    }
    for (int32_t i = 0; i < infoNum; i++) {
        cout << "networkid : " << std::string(info->networkId) << " deviceName : "
            << std::string(info->deviceName) << endl;
        info++;
    }
}

std::string OnDemandHelper::GetFirstDevice()
{
    NodeBasicInfo *info = NULL;
    int32_t infoNum = 0;
    int32_t ret = GetAllNodeDeviceInfo("ondemand", &info, &infoNum);
    if (ret != 0) {
        cout << "get remote deviceid GetAllNodeDeviceInfo failed" << endl;
        return "";
    }
    if (infoNum <= 0) {
        cout << "get remote deviceid failed, no remote device" << endl;
        return "";
    }
    return std::string(info->networkId);
}

int32_t OnDemandHelper::LoadRemoteAbility(int32_t systemAbilityId, const std::string& deviceId,
    const sptr<ISystemAbilityLoadCallback>& callback)
{
    cout << "LoadRemoteAbility start"<< endl;
    int64_t begin = GetTickCount();
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sm == nullptr) {
        cout << "GetSystemAbilityManager samgr object null!" << endl;
        return ERR_NULL_OBJECT;
    }
    int32_t result = -1;
    if (callback == nullptr) {
        result = sm->LoadSystemAbility(systemAbilityId, deviceId, loadCallback_);
    } else {
        result = sm->LoadSystemAbility(systemAbilityId, deviceId, callback);
    }

    if (result != ERR_OK) {
        cout << "systemAbilityId:" << systemAbilityId << " load failed, result code:" << result << endl;
    }
    cout << "LoadRemoteAbility result:" << result << " spend:" << (GetTickCount() - begin) << " ms"
        << " systemAbilityId:" << systemAbilityId << endl;
    return result;
}

void OnDemandHelper::LoadRemoteAbilityMuti(int32_t systemAbilityId, const std::string& deviceId)
{
    std::thread thread1([systemAbilityId, deviceId, this]() {
        cout << "LoadRemoteAbilityMuti thread1" << endl;
        LoadRemoteAbility(systemAbilityId, deviceId, loadCallback_);
    });
    std::thread thread2([systemAbilityId, deviceId, this]() {
        cout << "LoadRemoteAbilityMuti thread2" << endl;
        LoadRemoteAbility(systemAbilityId, deviceId, loadCallback_);
    });
    std::thread thread3([systemAbilityId, deviceId, this]() {
        cout << "LoadRemoteAbilityMuti thread3" << endl;
        LoadRemoteAbility(systemAbilityId, deviceId, loadCallback_);
    });
    thread1.detach();
    thread2.detach();
    thread3.detach();
}

void OnDemandHelper::LoadRemoteAbilityMutiCb(int32_t systemAbilityId, const std::string& deviceId)
{
    std::thread thread1([systemAbilityId, deviceId, this]() {
        cout << "LoadRemoteAbilityMuti thread1" << endl;
        LoadRemoteAbility(systemAbilityId, deviceId, loadCallback_);
    });
    std::thread thread2([systemAbilityId, deviceId, this]() {
        cout << "LoadRemoteAbilityMuti thread2" << endl;
        LoadRemoteAbility(systemAbilityId, deviceId, loadCallback2_);
    });
    std::thread thread3([systemAbilityId, deviceId, this]() {
        cout << "LoadRemoteAbilityMuti thread3" << endl;
        LoadRemoteAbility(systemAbilityId, deviceId, loadCallback3_);
    });
    thread1.detach();
    thread2.detach();
    thread3.detach();
}

void OnDemandHelper::LoadRemoteAbilityMutiSA(int32_t systemAbilityId, const std::string& deviceId)
{
    std::thread thread1([systemAbilityId, deviceId, this]() {
        cout << "LoadRemoteAbilityMuti thread1" << endl;
        LoadRemoteAbility(systemAbilityId, deviceId, loadCallback_);
    });
    std::thread thread2([deviceId, this]() {
        cout << "LoadRemoteAbilityMuti thread2" << endl;
        LoadRemoteAbility(DISTRIBUTED_SCHED_TEST_LISTEN_ID, deviceId, loadCallback_);
    });
    std::thread thread3([deviceId, this]() {
        cout << "LoadRemoteAbilityMuti thread3" << endl;
        LoadRemoteAbility(DISTRIBUTED_SCHED_TEST_MEDIA_ID, deviceId, loadCallback_);
    });
    thread1.detach();
    thread2.detach();
    thread3.detach();
}

void OnDemandHelper::LoadRemoteAbilityMutiSACb(int32_t systemAbilityId, const std::string& deviceId)
{
    std::thread thread1([systemAbilityId, deviceId, this]() {
        cout << "LoadRemoteAbilityMuti thread1" << endl;
        LoadRemoteAbility(systemAbilityId, deviceId, loadCallback_);
    });
    std::thread thread2([deviceId, this]() {
        cout << "LoadRemoteAbilityMuti thread2" << endl;
        LoadRemoteAbility(DISTRIBUTED_SCHED_SA_ID, deviceId, loadCallback2_);
    });
    std::thread thread3([deviceId, this]() {
        cout << "LoadRemoteAbilityMuti thread3" << endl;
        LoadRemoteAbility(DISTRIBUTED_SCHED_TEST_MEDIA_ID, deviceId, loadCallback3_);
    });
    thread1.detach();
    thread2.detach();
    thread3.detach();
}

void OnDemandHelper::LoadRemoteAbilityPressure(int32_t systemAbilityId, const std::string& deviceId)
{
    for (int i = 0 ; i < LOOP_TIME; ++i) {
        LoadRemoteAbility(systemAbilityId, deviceId, nullptr);
    }
}

sptr<IRemoteObject> OnDemandHelper::GetSystemAbility(int32_t systemAbilityId)
{
    int64_t begin = GetTickCount();
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sm == nullptr) {
        cout << "GetSystemAbilityManager samgr object null!" << endl;
        return nullptr;
    }
    sptr<IRemoteObject> remoteObject = sm->GetSystemAbility(systemAbilityId);
    if (remoteObject == nullptr) {
        cout << "GetSystemAbility systemAbilityId:" << systemAbilityId << " failed !" << endl;
        return nullptr;
    }
    cout << "GetSystemAbility result: success "<< " spend:"
        << (GetTickCount() - begin) << " ms" << " systemAbilityId:" << systemAbilityId << endl;
    return remoteObject;
}

sptr<IRemoteObject> OnDemandHelper::CheckSystemAbility(int32_t systemAbilityId)
{
    int64_t begin = GetTickCount();
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sm == nullptr) {
        cout << "GetSystemAbilityManager samgr object null!" << endl;
        return nullptr;
    }
    sptr<IRemoteObject> remoteObject = sm->CheckSystemAbility(systemAbilityId);
    if (remoteObject == nullptr) {
        cout << "CheckSystemAbility systemAbilityId:" << systemAbilityId << " failed !" << endl;
        return nullptr;
    }
    cout << "CheckSystemAbility result: success "<< " spend:"
        << (GetTickCount() - begin) << " ms" << " systemAbilityId:" << systemAbilityId << endl;
    return remoteObject;
}

sptr<IRemoteObject> OnDemandHelper::CheckSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    int64_t begin = GetTickCount();
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sm == nullptr) {
        cout << "GetSystemAbilityManager samgr object null!" << endl;
        return nullptr;
    }
    sptr<IRemoteObject> remoteObject = sm->CheckSystemAbility(systemAbilityId, deviceId);
    if (remoteObject == nullptr) {
        cout << "CheckSystemAbilityRmt systemAbilityId:" << systemAbilityId << " failed !" << endl;
        return nullptr;
    }
    cout << "CheckSystemAbilityRmt result: success "<< " spend:"
        << (GetTickCount() - begin) << " ms" << " systemAbilityId:" << systemAbilityId << endl;
    return remoteObject;
}

void OnDemandHelper::GetOnDemandPolicy(int32_t systemAbilityId, OnDemandPolicyType type)
{
    SamMockPermission::MockProcess("listen_test");
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sm == nullptr) {
        cout << "GetSystemAbilityManager samgr object null!" << endl;
        return;
    }
    std::vector<SystemAbilityOnDemandEvent> abilityOnDemandEvents;
    int32_t ret = sm->GetOnDemandPolicy(systemAbilityId, type, abilityOnDemandEvents);
    if (ret != ERR_OK) {
        cout << "GetOnDemandPolicy failed" << endl;
        return;
    }
    cout << "GetOnDemandPolicy success" << endl;
    for (auto& event : abilityOnDemandEvents) {
        cout << "eventId: " << static_cast<int32_t>(event.eventId) << " name:" << event.name
            << " value:" << event.value << endl;
    }
}

void OnDemandHelper::UpdateOnDemandPolicy(int32_t systemAbilityId, OnDemandPolicyType type,
    std::vector<SystemAbilityOnDemandEvent>& abilityOnDemandEvents)
{
    SamMockPermission::MockProcess("listen_test");
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sm == nullptr) {
        cout << "GetSystemAbilityManager samgr object null!" << endl;
        return;
    }
    for (auto& event : abilityOnDemandEvents) {
        cout << "update eventId: " << static_cast<int32_t>(event.eventId) << " name:" << event.name
            << " value:" << event.value << endl;
    }
    int32_t ret = sm->UpdateOnDemandPolicy(systemAbilityId, type, abilityOnDemandEvents);
    if (ret != ERR_OK) {
        cout << "UpdateOnDemandPolicy failed" << endl;
        return;
    }
    cout << "UpdateOnDemandPolicy success" << endl;
}

void OnDemandHelper::OnLoadSystemAbility(int32_t systemAbilityId)
{
}

void OnDemandHelper::OnDemandLoadCallback::OnLoadSystemAbilitySuccess(int32_t systemAbilityId,
    const sptr<IRemoteObject>& remoteObject)
{
    cout << "OnLoadSystemAbilitySuccess systemAbilityId:" << systemAbilityId << " IRemoteObject result:" <<
        ((remoteObject != nullptr) ? "succeed" : "failed") << endl;
    OnDemandHelper::GetInstance().OnLoadSystemAbility(systemAbilityId);
}

void OnDemandHelper::OnDemandLoadCallback::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    cout << "OnLoadSystemAbilityFail systemAbilityId:" << systemAbilityId << endl;
}

void OnDemandHelper::OnDemandLoadCallback::OnLoadSACompleteForRemote(const std::string& deviceId,
    int32_t systemAbilityId, const sptr<IRemoteObject>& remoteObject)
{
    cout << "OnLoadSACompleteForRemote systemAbilityId:" << systemAbilityId << " IRemoteObject result:" <<
        ((remoteObject != nullptr) ? "succeed" : "failed") << endl;
}

int32_t OnDemandHelper::GetExtensionSaIds(const std::string& extension, std::vector<int32_t> &saIds)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    int32_t ret;
    if ((ret = sm->GetExtensionSaIds(extension, saIds)) != ERR_OK) {
        return ret;
    }
    cout << __func__ << "saIds size: " << saIds.size() << endl;
    if (saIds.size() != 0) {
        cout << __func__ << "saIds: ";
        for (uint32_t loop = 0; loop < saIds.size(); ++loop) {
            cout << saIds[loop] << ", ";
        }
        cout << endl;
    }
    return ERR_OK;
}

int32_t OnDemandHelper::GetExtensionRunningSaList(const std::string& extension,
    std::vector<sptr<IRemoteObject>>& saList)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    int32_t ret;
    if ((ret = sm->GetExtensionRunningSaList(extension, saList)) != ERR_OK) {
        return ret;
    }
    cout << __func__ << "saList size: " << saList.size() << endl;
    if (saList.size() != 0) {
        cout << __func__ << "saIds: ";
        for (uint32_t loop = 0; loop < saList.size(); ++loop) {
            cout << (saList[loop] != nullptr) << ", ";
        }
        cout << endl;
    }
    return ERR_OK;
}

void OnDemandHelper::GetCommonEventExtraId(int32_t saId, const std::string& eventName)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sm == nullptr) {
        cout << "GetCommonEventExtraId get samgr failed" << endl;
        return;
    }
    std::vector<int64_t> extraDataIdList;
    int32_t ret = sm->GetCommonEventExtraDataIdlist(saId, extraDataIdList, eventName);
    if (ret != ERR_OK) {
        cout << "GetCommonEventExtraDataIdlist failed ret is " << ret << endl;
        return;
    }
    cout << __func__ << "extra id size: " << extraDataIdList.size() << endl;
    for (auto& item : extraDataIdList) {
        cout << item << ", ";
        MessageParcel extraDataParcel;
        ret = sm->GetOnDemandReasonExtraData(item, extraDataParcel);
        if (ret != ERR_OK) {
            cout << "get extra data failed" << endl;
            continue;
        }
        auto extraData = extraDataParcel.ReadParcelable<OnDemandReasonExtraData>();
        if (extraData == nullptr) {
            cout << "get extra data read parcel fail" << endl;
            continue;
        }
        auto want = extraData->GetWant();
        cout << "get extra data event name is " << want["common_event_action_name"] << endl;
    }
    cout << endl;
    return;
}
}

static void TestProcess(OHOS::OnDemandHelper& ondemandHelper)
{
    std::string cmd = "";
    cout << "please input proc test case(getp/initp/subp/unsubp)" << endl;
    cin >> cmd;
    if (cmd == "getp") {
        SamMockPermission::MockProcess("resource_schedule_service");
        ondemandHelper.GetSystemProcess();
    } else if (cmd == "subp") {
        SamMockPermission::MockProcess("resource_schedule_service");
        ondemandHelper.SubscribeSystemProcess();
    } else if (cmd == "unsubp") {
        SamMockPermission::MockProcess("resource_schedule_service");
        ondemandHelper.UnSubscribeSystemProcess();
    } else if (cmd == "initp") {
        ondemandHelper.InitSystemProcessStatusChange();
    } else {
        cout << "invalid input" << endl;
    }
}

static void TestSystemAbility(OHOS::OnDemandHelper& ondemandHelper)
{
    std::string cmd = "";
    cout << "please input sa test case(get/load/unload/syncload/getinfo)" << endl;
    cin >> cmd;
    int32_t systemAbilityId = 0;
    std::string deviceId = ondemandHelper.GetFirstDevice();
    cout << "please input systemAbilityId for " << cmd << " operation" << endl;
    cin >> systemAbilityId;
    if (cmd == "get") {
        ondemandHelper.GetSystemAbility(systemAbilityId);
    } else if (cmd == "load") {
        ondemandHelper.OnDemandAbility(systemAbilityId);
    } else if (cmd == "device") { // get remote networkid
        ondemandHelper.GetDeviceList();
    } else if (cmd == "loadrmt1") { // single thread with one device, one system ability, one callback
        ondemandHelper.LoadRemoteAbility(systemAbilityId, deviceId, nullptr);
    } else if (cmd == "loadrmt2") { // one device, one system ability, one callback, three threads
        ondemandHelper.LoadRemoteAbilityMuti(systemAbilityId, deviceId);
    } else if (cmd == "loadrmt3") { // one device, one system ability, three callbacks, three threads
        ondemandHelper.LoadRemoteAbilityMutiCb(systemAbilityId, deviceId);
    } else if (cmd == "loadrmt4") { // one device, three system abilities, one callback, three threads
        ondemandHelper.LoadRemoteAbilityMutiSA(systemAbilityId, deviceId);
    } else if (cmd == "loadrmt5") { // one device, three system abilities, three callbacks, three threads
        ondemandHelper.LoadRemoteAbilityMutiSACb(systemAbilityId, deviceId);
    } else if (cmd == "loadrmt6") { // two devices
        int32_t otherSystemAbilityId = 0;
        cout << "please input another systemabilityId for " << cmd << " operation" << endl;
        cin >> otherSystemAbilityId;
        cout << "please input another deviceId for " << cmd << " operation" << endl;
        std::string otherDevice;
        cin >> otherDevice;
        ondemandHelper.LoadRemoteAbility(systemAbilityId, deviceId, nullptr);
        ondemandHelper.LoadRemoteAbility(otherSystemAbilityId, otherDevice, nullptr);
    } else if (cmd == "loadmuti") {
        ondemandHelper.LoadRemoteAbilityPressure(systemAbilityId, deviceId);
    } else if (cmd == "unload") {
        ondemandHelper.UnloadSystemAbility(systemAbilityId);
    } else if (cmd == "getinfo") {
        ondemandHelper.GetSystemProcessInfo(systemAbilityId);
    } else if (cmd == "syncload") {
        ondemandHelper.TestSyncOnDemandAbility(systemAbilityId);
    } else {
        cout << "invalid input" << endl;
    }
}

static void TestParamPlugin(OHOS::OnDemandHelper& ondemandHelper)
{
    cout << "please input param's value" << endl;
    string value = "false";
    cin >> value;
    if (value == "false") {
        int ret = SetParameter("persist.samgr.deviceparam", "false");
        cout << ret;
    } else if (value == "true") {
        int ret = SetParameter("persist.samgr.deviceparam", "true");
        cout << ret;
    } else {
        cout << "invalid input" << endl;
    }
}

static void CreateOnDemandStartPolicy(SystemAbilityOnDemandEvent& event)
{
    int eventId = 1;
    cout << "please input on demand event id(1,2,3,4,5)" << endl;
    cin >> eventId;
    if (eventId == static_cast<int32_t>(OnDemandEventId::DEVICE_ONLINE)) {
        event.eventId = OnDemandEventId::DEVICE_ONLINE;
        event.name = "deviceonline";
        event.value = "on";
    } else if (eventId == static_cast<int32_t>(OnDemandEventId::SETTING_SWITCH)) {
        event.eventId = OnDemandEventId::SETTING_SWITCH;
        event.name = "wifi_status";
        event.value = "on";
    } else if (eventId == static_cast<int32_t>(OnDemandEventId::PARAM)) {
        event.eventId = OnDemandEventId::PARAM;
        event.name = "persist.samgr.deviceparam";
        event.value = "true";
    } else if (eventId == static_cast<int32_t>(OnDemandEventId::COMMON_EVENT)) {
        event.eventId = OnDemandEventId::COMMON_EVENT;
        event.name = "usual.event.SCREEN_ON";
        event.value = "";
    } else if (eventId == static_cast<int32_t>(OnDemandEventId::TIMED_EVENT)) {
        event.eventId = OnDemandEventId::TIMED_EVENT;
        event.name = "loopevent";
        event.value = "60";
    }
}

static void CreateOnDemandStopPolicy(SystemAbilityOnDemandEvent& event)
{
    int eventId = 1;
    cout << "please input on demand event id(1,2,3,4,5)" << endl;
    cin >> eventId;
    if (eventId == static_cast<int32_t>(OnDemandEventId::DEVICE_ONLINE)) {
        event.eventId = OnDemandEventId::DEVICE_ONLINE;
        event.name = "deviceonline";
        event.value = "off";
    } else if (eventId == static_cast<int32_t>(OnDemandEventId::SETTING_SWITCH)) {
        event.eventId = OnDemandEventId::SETTING_SWITCH;
        event.name = "wifi_status";
        event.value = "off";
    } else if (eventId == static_cast<int32_t>(OnDemandEventId::PARAM)) {
        event.eventId = OnDemandEventId::PARAM;
        event.name = "persist.samgr.deviceparam";
        event.value = "false";
    } else if (eventId == static_cast<int32_t>(OnDemandEventId::COMMON_EVENT)) {
        event.eventId = OnDemandEventId::COMMON_EVENT;
        event.name = "usual.event.SCREEN_OFF";
        event.value = "";
    } else if (eventId == static_cast<int32_t>(OnDemandEventId::TIMED_EVENT)) {
        event.eventId = OnDemandEventId::TIMED_EVENT;
        event.name = "loopevent";
        event.value = "70";
    }
}

static void TestOnDemandPolicy(OHOS::OnDemandHelper& ondemandHelper)
{
    std::string cmd = "";
    cout << "please input on demand policy test case(get/update)" << endl;
    cin >> cmd;
    std::string type = "";
    cout << "please input on demand type test case(start/stop)" << endl;
    cin >> type;
    int32_t systemAbilityId = 0;
    cout << "please input systemAbilityId for " << cmd << " operation" << endl;
    cin >> systemAbilityId;
    if (cmd == "get" && type == "start") {
        ondemandHelper.GetOnDemandPolicy(systemAbilityId, OnDemandPolicyType::START_POLICY);
    } else if (cmd == "get" && type == "stop") {
        ondemandHelper.GetOnDemandPolicy(systemAbilityId, OnDemandPolicyType::STOP_POLICY);
    } else if (cmd == "update" && type == "start") {
        SystemAbilityOnDemandEvent event;
        CreateOnDemandStartPolicy(event);
        std::vector<SystemAbilityOnDemandEvent> abilityOnDemandEvents;
        abilityOnDemandEvents.push_back(event);
        ondemandHelper.UpdateOnDemandPolicy(systemAbilityId, OnDemandPolicyType::START_POLICY, abilityOnDemandEvents);
    } else if (cmd == "update" && type == "start_multi") {
        SystemAbilityOnDemandEvent event;
        CreateOnDemandStartPolicy(event);
        SystemAbilityOnDemandEvent event2;
        CreateOnDemandStartPolicy(event2);
        std::vector<SystemAbilityOnDemandEvent> abilityOnDemandEvents;
        abilityOnDemandEvents.push_back(event);
        abilityOnDemandEvents.push_back(event2);
        ondemandHelper.UpdateOnDemandPolicy(systemAbilityId, OnDemandPolicyType::START_POLICY, abilityOnDemandEvents);
    } else if (cmd == "update" && type == "stop") {
        SystemAbilityOnDemandEvent event;
        CreateOnDemandStopPolicy(event);
        std::vector<SystemAbilityOnDemandEvent> abilityOnDemandEvents;
        abilityOnDemandEvents.push_back(event);
        ondemandHelper.UpdateOnDemandPolicy(systemAbilityId, OnDemandPolicyType::STOP_POLICY, abilityOnDemandEvents);
    } else if (cmd == "update" && type == "stop_multi") {
        SystemAbilityOnDemandEvent event;
        CreateOnDemandStopPolicy(event);
        SystemAbilityOnDemandEvent event2;
        CreateOnDemandStopPolicy(event2);
        std::vector<SystemAbilityOnDemandEvent> abilityOnDemandEvents;
        abilityOnDemandEvents.push_back(event);
        abilityOnDemandEvents.push_back(event2);
        ondemandHelper.UpdateOnDemandPolicy(systemAbilityId, OnDemandPolicyType::STOP_POLICY, abilityOnDemandEvents);
    } else {
        cout << "invalid input" << endl;
    }
}

static void CreateOnDemandStartPolicy1(SystemAbilityOnDemandEvent& event)
{
    int eventId = 1;
    cout << "please input on demand event id(1,2,3,4,5)" << endl;
    cin >> eventId;
    if (eventId == static_cast<int32_t>(OnDemandEventId::DEVICE_ONLINE)) {
        event.eventId = OnDemandEventId::DEVICE_ONLINE;
        event.name = "deviceonline";
        event.value = "on";
    } else if (eventId == static_cast<int32_t>(OnDemandEventId::SETTING_SWITCH)) {
        event.eventId = OnDemandEventId::SETTING_SWITCH;
        event.name = "wifi_status";
        event.value = "on";
    } else if (eventId == static_cast<int32_t>(OnDemandEventId::PARAM)) {
        event.eventId = OnDemandEventId::PARAM;
        event.name = "persist.samgr.deviceparam";
        event.value = "true";
    } else if (eventId == static_cast<int32_t>(OnDemandEventId::COMMON_EVENT)) {
        event.eventId = OnDemandEventId::COMMON_EVENT;
        event.name = "usual.event.SCREEN_ON";
        event.value = "";
    } else if (eventId == static_cast<int32_t>(OnDemandEventId::TIMED_EVENT)) {
        event.eventId = OnDemandEventId::TIMED_EVENT;
        event.name = "timedevent";
        event.value = g_inputTimeStr;
        event.persistence = true;
    }
}

static void CreateOnDemandStopPolicy1(SystemAbilityOnDemandEvent& event)
{
    int eventId = 1;
    cout << "please input on demand event id(1,2,3,4,5)" << endl;
    cin >> eventId;
    if (eventId == static_cast<int32_t>(OnDemandEventId::DEVICE_ONLINE)) {
        event.eventId = OnDemandEventId::DEVICE_ONLINE;
        event.name = "deviceonline";
        event.value = "off";
    } else if (eventId == static_cast<int32_t>(OnDemandEventId::SETTING_SWITCH)) {
        event.eventId = OnDemandEventId::SETTING_SWITCH;
        event.name = "wifi_status";
        event.value = "off";
    } else if (eventId == static_cast<int32_t>(OnDemandEventId::PARAM)) {
        event.eventId = OnDemandEventId::PARAM;
        event.name = "persist.samgr.deviceparam";
        event.value = "false";
    } else if (eventId == static_cast<int32_t>(OnDemandEventId::COMMON_EVENT)) {
        event.eventId = OnDemandEventId::COMMON_EVENT;
        event.name = "usual.event.SCREEN_OFF";
        event.value = "";
    } else if (eventId == static_cast<int32_t>(OnDemandEventId::TIMED_EVENT)) {
        event.eventId = OnDemandEventId::TIMED_EVENT;
        event.name = "timedevent";
        event.value = g_inputTimeStr;
        event.persistence = true;
    }
}

static void TestOnDemandPolicy1(OHOS::OnDemandHelper& ondemandHelper)
{
    cout << "please input time" << endl;
    cin >> g_inputTimeStr;
    std::string cmd = "";
    cout << "please input on demand policy test case(get/update)" << endl;
    cin >> cmd;
    std::string type = "";
    cout << "please input on demand type test case(start/stop)" << endl;
    cin >> type;
    int32_t systemAbilityId = 0;
    cout << "please input systemAbilityId for " << cmd << " operation" << endl;
    cin >> systemAbilityId;
    if (cmd == "get" && type == "start") {
        ondemandHelper.GetOnDemandPolicy(systemAbilityId, OnDemandPolicyType::START_POLICY);
    } else if (cmd == "get" && type == "stop") {
        ondemandHelper.GetOnDemandPolicy(systemAbilityId, OnDemandPolicyType::STOP_POLICY);
    } else if (cmd == "update" && type == "start") {
        SystemAbilityOnDemandEvent event;
        CreateOnDemandStartPolicy1(event);
        std::vector<SystemAbilityOnDemandEvent> abilityOnDemandEvents;
        abilityOnDemandEvents.push_back(event);
        ondemandHelper.UpdateOnDemandPolicy(systemAbilityId, OnDemandPolicyType::START_POLICY, abilityOnDemandEvents);
    } else if (cmd == "update" && type == "start_multi") {
        SystemAbilityOnDemandEvent event;
        CreateOnDemandStartPolicy1(event);
        SystemAbilityOnDemandEvent event2;
        CreateOnDemandStartPolicy1(event2);
        std::vector<SystemAbilityOnDemandEvent> abilityOnDemandEvents;
        abilityOnDemandEvents.push_back(event);
        abilityOnDemandEvents.push_back(event2);
        ondemandHelper.UpdateOnDemandPolicy(systemAbilityId, OnDemandPolicyType::START_POLICY, abilityOnDemandEvents);
    } else if (cmd == "update" && type == "stop") {
        SystemAbilityOnDemandEvent event;
        CreateOnDemandStopPolicy1(event);
        std::vector<SystemAbilityOnDemandEvent> abilityOnDemandEvents;
        abilityOnDemandEvents.push_back(event);
        ondemandHelper.UpdateOnDemandPolicy(systemAbilityId, OnDemandPolicyType::STOP_POLICY, abilityOnDemandEvents);
    } else if (cmd == "update" && type == "stop_multi") {
        SystemAbilityOnDemandEvent event;
        CreateOnDemandStopPolicy1(event);
        SystemAbilityOnDemandEvent event2;
        CreateOnDemandStopPolicy1(event2);
        std::vector<SystemAbilityOnDemandEvent> abilityOnDemandEvents;
        abilityOnDemandEvents.push_back(event);
        abilityOnDemandEvents.push_back(event2);
        ondemandHelper.UpdateOnDemandPolicy(systemAbilityId, OnDemandPolicyType::STOP_POLICY, abilityOnDemandEvents);
    } else {
        cout << "invalid input" << endl;
    }
}

static void TestCommonEvent(OHOS::OnDemandHelper& ondemandHelper)
{
    std::string cmd = "";
    cout << "please input common event test case(1 get/2 get_with_event)" << endl;
    cin >> cmd;
    int32_t saId = 0;
    cout << "please input systemAbilityId for " << cmd << " operation" << endl;
    cin >> saId;
    if (cmd == "1") {
        ondemandHelper.GetCommonEventExtraId(saId, "");
    } else if (cmd == "2") {
        cout << "please input common event name" << endl;
        std::string eventName;
        cin >> eventName;
        ondemandHelper.GetCommonEventExtraId(saId, eventName);
    } else {
        cout << "invalid input" << endl;
    }
}

static void TestGetExtension(OHOS::OnDemandHelper& ondemandHelper)
{
    std::string extension;
    cin >> extension;

    std::vector<int32_t> saIds;
    if (ondemandHelper.GetExtensionSaIds(extension, saIds) != ERR_OK) {
        cout << "get extension: " << extension << " failed" << endl;
        return;
    }
    std::vector<sptr<IRemoteObject>> saList;
    if (ondemandHelper.GetExtensionRunningSaList(extension, saList) != ERR_OK) {
        cout << "get handle extension: " << extension << " failed" << endl;
        return;
    }
    return;
}

static void TestCheckSystemAbility(OHOS::OnDemandHelper& ondemandHelper)
{
    std::string cmd = "";
    cout << "please input check case(local/remote)" << endl;
    cin >> cmd;
    int32_t saId = 0;
    cout << "please input systemAbilityId for " << cmd << " operation" << endl;
    cin >> saId;
    if (cmd == "local") {
        ondemandHelper.CheckSystemAbility(saId);
    } else if (cmd == "remote") {
        std::string deviceId = ondemandHelper.GetFirstDevice();
        ondemandHelper.CheckSystemAbility(saId, deviceId);
    } else {
        cout << "invalid input" << endl;
    }
}

static void TestIntCommand(OHOS::OnDemandHelper& ondemandHelper, string& cmd)
{
    if (cmd == "1") {
        TestParamPlugin(ondemandHelper);
    } else if (cmd == "2") {
        TestSystemAbility(ondemandHelper);
    } else if (cmd == "3") {
        TestProcess(ondemandHelper);
    } else if (cmd == "4") {
        TestOnDemandPolicy(ondemandHelper);
    } else if (cmd == "5") {
        TestGetExtension(ondemandHelper);
    } else if (cmd == "6") {
        TestCommonEvent(ondemandHelper);
    } else if (cmd == "7") {
        TestCheckSystemAbility(ondemandHelper);
    } else if (cmd == "8") {
        TestOnDemandPolicy1(ondemandHelper);
    } else if (cmd == "9") {
        TestScheduler(ondemandHelper);
    } else if (cmd == "10") {
        ondemandHelper.UnloadAllIdleSystemAbility();
    } else {
        cout << "invalid input" << endl;
    }
}

static void TestStringCommand(OHOS::OnDemandHelper& ondemandHelper, string& cmd)
{
    if (cmd == "param") {
        TestParamPlugin(ondemandHelper);
    } else if (cmd == "sa") {
        TestSystemAbility(ondemandHelper);
    } else if (cmd == "proc") {
        TestProcess(ondemandHelper);
    } else if (cmd == "policy") {
        TestOnDemandPolicy(ondemandHelper);
    } else if (cmd == "getExtension") {
        TestGetExtension(ondemandHelper);
    } else if (cmd == "getEvent") {
        TestCommonEvent(ondemandHelper);
    } else if (cmd == "check") {
        TestCheckSystemAbility(ondemandHelper);
    } else if (cmd == "policy_time") {
        TestOnDemandPolicy1(ondemandHelper);
    } else if (cmd == "test") {
        TestScheduler(ondemandHelper);
    } else if (cmd == "memory") {
        ondemandHelper.UnloadAllIdleSystemAbility();
    } else {
        cout << "invalid input" << endl;
    }
}

int main(int argc, char* argv[])
{
    SamMockPermission::MockPermission();
    OHOS::OnDemandHelper& ondemandHelper = OnDemandHelper::GetInstance();
    string cmd = "load";
    do {
        cout << "please input operation(1-param/2-sa/3-proc/4-policy/5-getExtension)" << endl;
        cout << "please input operation(6-getEvent/7-check/8-policy_time/9-test/10-memory)" << endl;
        cmd.clear();
        cin.clear();
        cin >> cmd;
        int32_t value = atoi(cmd.c_str());
        if (value == 0) {
            TestStringCommand(ondemandHelper, cmd);
        } else {
            TestIntCommand(ondemandHelper, cmd);
        }
        cout << "-----Input q or Q to quit" << endl;
        cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
    } while (cmd[0] != 'q' && cmd[0] != 'Q');
    return 0;
}
