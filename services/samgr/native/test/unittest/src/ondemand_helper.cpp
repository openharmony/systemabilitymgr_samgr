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

#include "ondemand_helper.h"

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
#ifdef SUPPORT_ACCESS_TOKEN
#include "nativetoken_kit.h"
#include "token_setproc.h"
#endif
#include "sam_mock_permission.h"
#ifdef SUPPORT_SOFTBUS
#include "softbus_bus_center.h"
#endif
#include "system_ability_ondemand_reason.h"
#include "system_ability_definition.h"
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
    cout << "processName: " << processInfo.processName << " pid: " << processInfo.pid << endl;
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
        cout << "systemAbilityId:" << systemAbilityId << " syncload failed, result code:" << result << endl;
        return nullptr;
    }
    cout << "SyncLoadSystemAbility result:" << result << " spend:" << (GetTickCount() - begin) << "ms"
        << " systemAbilityId:" << systemAbilityId << endl;
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
        cout << "transact failed, errCode = " << errCode;
        return errCode;
    }
    int32_t ret = reply.ReadInt32();
    cout << "ret = " << ret;
    return ret;
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
    cout << "UnloadAllIdleSystemAbility result:" << result << " spend:" << (GetTickCount() - begin) << "ms" << endl;
    return ERR_OK;
}

int32_t OnDemandHelper::GetLruIdleSystemAbilityProc()
{
    SamMockPermission::MockProcess("memmgrservice");
    int64_t begin = GetTickCount();
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sm == nullptr) {
        cout << "GetLruIdleSystemAbilityProc samgr object null!" << endl;
        return ERR_NULL_OBJECT;
    }
    vector<IdleProcessInfo> procInfos;
    int32_t result = sm->GetLruIdleSystemAbilityProc(procInfos);
    if (result != ERR_OK) {
        cout << "GetLruIdleSystemAbilityProc failed, result code:" << result << endl;
        return result;
    }
    cout << "GetLruIdleSystemAbilityProc result:" << result << " spend:" << (GetTickCount() - begin) << "ms" << endl;
    for (const auto& proc:procInfos) {
        cout << "pid:" << proc.pid << ", "
            << "processName:" << Str16ToStr8(proc.processName) << ", "
            << "lastIdleTime:" << proc.lastIdleTime << endl;
    }
    return ERR_OK;
}

int32_t OnDemandHelper::UnloadProcess(const vector<u16string>& processList)
{
    SamMockPermission::MockProcess("memmgrservice");
    int64_t begin = GetTickCount();
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sm == nullptr) {
        cout << "UnloadProcess samgr object null!" << endl;
        return ERR_NULL_OBJECT;
    }
    int32_t result = sm->UnloadProcess(processList);
    if (result != ERR_OK) {
        cout << "UnloadProcess failed, result code:" << result << endl;
        return result;
    }
    cout << "UnloadProcess result:" << result << " spend:" << (GetTickCount() - begin) << "ms" << endl;
    return ERR_OK;
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

#ifdef SUPPORT_SOFTBUS
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
#endif

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
