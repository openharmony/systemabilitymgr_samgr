/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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
#include <list>
#include <memory>
#include <thread>
#include <vector>
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

#ifdef SUPPORT_MULTI_INSTANCE
constexpr int32_t LOAD_CALLBACK_TIMEOUT_MS = 5000;
constexpr char LISTEN_PROCESS_NAME[] = "listen_test";
constexpr char MULTI_INSTANCE_PROCESS_NAME[] = "multi_instance_test_probe";

enum class TestSaRequestCode : uint32_t {
    TRIGGER_UNLOAD = 3,
    TRIGGER_UNLOAD_CANCEL = 4,
    UPDATE_ON_DEMAND_POLICY = 5,
    GET_ON_DEMAND_POLICY = 6,
};

class WaitableLoadCallback final : public SystemAbilityLoadCallbackStub {
public:
    explicit WaitableLoadCallback(int32_t targetSaId) : targetSaId_(targetSaId) {}

    void OnLoadSystemAbilitySuccess(int32_t saId, const sptr<IRemoteObject>& remoteObject) override
    {
        if (saId != targetSaId_) {
            return;
        }
        std::lock_guard<std::mutex> lock(mutex_);
        remoteObject_ = remoteObject;
        completed_ = true;
        condition_.notify_all();
    }

    void OnLoadSystemAbilityFail(int32_t saId) override
    {
        if (saId != targetSaId_) {
            return;
        }
        std::lock_guard<std::mutex> lock(mutex_);
        completed_ = true;
        condition_.notify_all();
    }

    void OnLoadSACompleteForRemote(const std::string& deviceId, int32_t saId,
        const sptr<IRemoteObject>& remoteObject) override
    {
        (void)deviceId;
        OnLoadSystemAbilitySuccess(saId, remoteObject);
    }

    sptr<IRemoteObject> WaitForResult()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        bool ready = condition_.wait_for(lock, std::chrono::milliseconds(LOAD_CALLBACK_TIMEOUT_MS),
            [this]() { return completed_; });
        return ready ? remoteObject_ : nullptr;
    }

private:
    const int32_t targetSaId_;
    bool completed_ = false;
    sptr<IRemoteObject> remoteObject_;
    std::mutex mutex_;
    std::condition_variable condition_;
};

sptr<ISystemAbilityManager> GetSystemAbilityManager()
{
    return SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
}

const char* GetTestProcessName(int32_t saId)
{
    return saId == DISTRIBUTED_SCHED_TEST_LISTEN_ID ? LISTEN_PROCESS_NAME : MULTI_INSTANCE_PROCESS_NAME;
}

int32_t SendTestRequest(const sptr<IRemoteObject>& target, TestSaRequestCode code, MessageParcel& data,
    MessageParcel& reply)
{
    if (target == nullptr) {
        return ERR_NULL_OBJECT;
    }
    MessageOption option;
    return target->SendRequest(static_cast<uint32_t>(code), data, reply, option);
}
#endif
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

void OnDemandHelper::SubscribeLowMemSystemProcess()
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sm == nullptr) {
        cout << "GetSystemAbilityManager samgr object null!" << endl;
        return;
    }
    int32_t ret = sm->SubscribeLowMemSystemProcess(systemProcessStatusChange_);
    if (ret != ERR_OK) {
        cout << "SubscribeLowMemSystemProcess failed" << endl;
        return;
    }
    cout << "SubscribeLowMemSystemProcess success" << endl;
}

void OnDemandHelper::UnSubscribeLowMemSystemProcess()
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sm == nullptr) {
        cout << "GetSystemAbilityManager samgr object null!" << endl;
        return;
    }
    int32_t ret = sm->UnSubscribeLowMemSystemProcess(systemProcessStatusChange_);
    if (ret != ERR_OK) {
        cout << "UnSubscribeLowMemSystemProcess failed" << endl;
        return;
    }
    cout << "UnSubscribeLowMemSystemProcess success" << endl;
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

void OnDemandHelper::SystemProcessStatusChange::OnSystemProcessActivated(SystemProcessInfo& systemProcessInfo)
{
    std::unique_lock lock(mutex_);
    cout << endl << "OnSystemProcessActivated, processName: " << systemProcessInfo.processName << " pid:"
        << systemProcessInfo.pid << " uid:" << systemProcessInfo.uid << endl;
    eventFired_ = OnDemandHelper::ProcessStatusChangeEvent::Active;
    cv_.notify_all();
}

void OnDemandHelper::SystemProcessStatusChange::OnSystemProcessIdled(SystemProcessInfo& systemProcessInfo)
{
    std::unique_lock lock(mutex_);
    cout << endl << "OnSystemProcessIdled, processName: " << systemProcessInfo.processName << " pid:"
        << systemProcessInfo.pid << " uid:" << systemProcessInfo.uid << endl;
    eventFired_ = OnDemandHelper::ProcessStatusChangeEvent::Idle;
    cv_.notify_all();
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

int32_t OnDemandHelper::SetSamgrIpcPrior(bool enalbe)
{
    SamMockPermission::MockProcess("resource_schedule_service");
    int64_t begin = GetTickCount();
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sm == nullptr) {
        cout << "SetSamgrIpcPrior samgr object null!" << endl;
        return ERR_NULL_OBJECT;
    }
    int32_t result = sm->SetSamgrIpcPrior(enalbe);
    if (result != ERR_OK) {
        cout << "SetSamgrIpcPrior failed, result code:" << result << endl;
        return result;
    }
    cout << "SetSamgrIpcPrior result:" << result << " spend:" << (GetTickCount() - begin) << " ms" << endl;
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

int32_t OnDemandHelper::OnUserStateChanged(int32_t userId, SamgrUserState userState)
{
    SamMockPermission::MockProcess("accountmgr");
    int64_t begin = GetTickCount();
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sm == nullptr) {
        cout << "OnUserStateChanged samgr object null!" << endl;
        return ERR_NULL_OBJECT;
    }
    int32_t result = sm->OnUserStateChanged(userId, userState);
    if (result != ERR_OK) {
        cout << "OnUserStateChanged failed, result code:" << result << endl;
        return result;
    }
    cout << "OnUserStateChanged result:" << result << " spend:" << (GetTickCount() - begin) << " ms"
        << " userId:" << userId << " userState:" << static_cast<int32_t>(userState) << endl;
    return ERR_OK;
}

#ifdef SUPPORT_MULTI_INSTANCE
sptr<IRemoteObject> OnDemandHelper::GetSystemAbility(int32_t systemAbilityId, int32_t userId)
{
    sptr<ISystemAbilityManager> manager = GetSystemAbilityManager();
    sptr<IRemoteObject> object = manager == nullptr ? nullptr : manager->GetSystemAbility(systemAbilityId, userId);
    cout << "GetSystemAbility systemAbilityId:" << systemAbilityId << " userId:" << userId
        << " result:" << (object == nullptr ? "failed" : "success") << endl;
    return object;
}

sptr<IRemoteObject> OnDemandHelper::CheckSystemAbility(int32_t systemAbilityId, bool& isExist)
{
    sptr<ISystemAbilityManager> manager = GetSystemAbilityManager();
    sptr<IRemoteObject> object = manager == nullptr ? nullptr : manager->CheckSystemAbility(systemAbilityId, isExist);
    cout << "CheckSystemAbility isExist:" << isExist << " systemAbilityId:" << systemAbilityId
        << " result:" << (object == nullptr ? "failed" : "success") << endl;
    return object;
}

sptr<IRemoteObject> OnDemandHelper::CheckSystemAbility(int32_t systemAbilityId, int32_t userId)
{
    sptr<ISystemAbilityManager> manager = GetSystemAbilityManager();
    sptr<IRemoteObject> object = manager == nullptr ? nullptr : manager->CheckSystemAbility(systemAbilityId, userId);
    cout << "CheckSystemAbility systemAbilityId:" << systemAbilityId << " userId:" << userId
        << " result:" << (object == nullptr ? "failed" : "success") << endl;
    return object;
}

sptr<IRemoteObject> OnDemandHelper::CheckSystemAbilityByUserId(int32_t systemAbilityId, bool& isExist,
    int32_t userId)
{
    sptr<ISystemAbilityManager> manager = GetSystemAbilityManager();
    sptr<IRemoteObject> object = manager == nullptr ? nullptr :
        manager->CheckSystemAbilityByUserId(systemAbilityId, isExist, userId);
    cout << "CheckSystemAbility isExist:" << isExist << " systemAbilityId:" << systemAbilityId
        << " userId:" << userId << " result:" << (object == nullptr ? "failed" : "success") << endl;
    return object;
}

int32_t OnDemandHelper::LoadSystemAbility(int32_t systemAbilityId,
    const sptr<ISystemAbilityLoadCallback>& callback, int32_t userId)
{
    sptr<ISystemAbilityManager> manager = GetSystemAbilityManager();
    return manager == nullptr ? ERR_NULL_OBJECT : manager->LoadSystemAbility(systemAbilityId, callback, userId);
}

sptr<IRemoteObject> OnDemandHelper::LoadSystemAbility(int32_t systemAbilityId, int32_t timeoutSeconds)
{
    sptr<ISystemAbilityManager> manager = GetSystemAbilityManager();
    sptr<IRemoteObject> object = manager == nullptr ? nullptr : manager->LoadSystemAbility(systemAbilityId,
        timeoutSeconds);
    cout << "SyncLoadSystemAbility saId:" << systemAbilityId << " timeout:" << timeoutSeconds
        << " result:" << (object == nullptr ? "failed" : "success") << endl;
    return object;
}

sptr<IRemoteObject> OnDemandHelper::LoadSystemAbility(int32_t systemAbilityId, int32_t timeoutSeconds, int32_t userId)
{
    sptr<ISystemAbilityManager> manager = GetSystemAbilityManager();
    sptr<IRemoteObject> object = manager == nullptr ? nullptr :
        manager->LoadSystemAbility(systemAbilityId, timeoutSeconds, userId);
    cout << "SyncLoadSystemAbility saId:" << systemAbilityId << " userId:" << userId
        << " timeout:" << timeoutSeconds << " result:" << (object == nullptr ? "failed" : "success") << endl;
    return object;
}

int32_t OnDemandHelper::LoadSystemAbilityByCallback(int32_t systemAbilityId)
{
    sptr<WaitableLoadCallback> callback = new WaitableLoadCallback(systemAbilityId);
    int32_t result = LoadSystemAbility(systemAbilityId, callback);
    if (result != ERR_OK) {
        return result;
    }
    int32_t callbackResult = callback->WaitForResult() == nullptr ? ERR_NULL_OBJECT : ERR_OK;
    cout << "LoadSystemAbility(callback) saId:" << systemAbilityId << " result:" << callbackResult << endl;
    return callbackResult;
}

int32_t OnDemandHelper::LoadSystemAbilityByCallback(int32_t systemAbilityId, int32_t userId)
{
    sptr<WaitableLoadCallback> callback = new WaitableLoadCallback(systemAbilityId);
    int32_t result = LoadSystemAbility(systemAbilityId, callback, userId);
    if (result != ERR_OK) {
        return result;
    }
    int32_t callbackResult = callback->WaitForResult() == nullptr ? ERR_NULL_OBJECT : ERR_OK;
    cout << "LoadSystemAbility(callback) saId:" << systemAbilityId << " userId:" << userId
        << " result:" << callbackResult << endl;
    return callbackResult;
}

int32_t OnDemandHelper::GetSystemProcessInfo(int32_t systemAbilityId, SystemProcessInfo& processInfo)
{
    sptr<ISystemAbilityManager> manager = GetSystemAbilityManager();
    int32_t result = manager == nullptr ? ERR_NULL_OBJECT : manager->GetSystemProcessInfo(systemAbilityId, processInfo);
    cout << "GetSystemProcessInfo saId:" << systemAbilityId << " processName:" << processInfo.processName
        << " pid:" << processInfo.pid << " result:" << result << endl;
    return result;
}

int32_t OnDemandHelper::GetSystemProcessInfo(int32_t systemAbilityId, SystemProcessInfo& processInfo, int32_t userId)
{
    sptr<ISystemAbilityManager> manager = GetSystemAbilityManager();
    int32_t result = manager == nullptr ? ERR_NULL_OBJECT :
        manager->GetSystemProcessInfo(systemAbilityId, processInfo, userId);
    cout << "GetSystemProcessInfo saId:" << systemAbilityId << " userId:" << userId
        << " processName:" << processInfo.processName << " pid:" << processInfo.pid << " result:" << result << endl;
    return result;
}

sptr<IRemoteObject> OnDemandHelper::GetLocalAbilityManagerProxy(int32_t systemAbilityId)
{
    sptr<ISystemAbilityManager> manager = GetSystemAbilityManager();
    sptr<IRemoteObject> object = manager == nullptr ? nullptr : manager->GetLocalAbilityManagerProxy(systemAbilityId);
    cout << "GetLocalAbilityManagerProxy saId:" << systemAbilityId
        << " result:" << (object == nullptr ? "null" : "ok") << endl;
    return object;
}

sptr<IRemoteObject> OnDemandHelper::GetLocalAbilityManagerProxy(int32_t systemAbilityId, int32_t userId)
{
    sptr<ISystemAbilityManager> manager = GetSystemAbilityManager();
    sptr<IRemoteObject> object = manager == nullptr ? nullptr :
        manager->GetLocalAbilityManagerProxy(systemAbilityId, userId);
    cout << "GetLocalAbilityManagerProxy saId:" << systemAbilityId << " userId:" << userId
        << " result:" << (object == nullptr ? "null" : "ok") << endl;
    return object;
}

int32_t OnDemandHelper::SubscribeSystemAbility(int32_t systemAbilityId,
    const sptr<ISystemAbilityStatusChange>& listener)
{
    sptr<ISystemAbilityManager> manager = GetSystemAbilityManager();
    return manager == nullptr ? ERR_NULL_OBJECT : manager->SubscribeSystemAbility(systemAbilityId, listener);
}

int32_t OnDemandHelper::SubscribeSystemAbility(int32_t systemAbilityId,
    const sptr<ISystemAbilityStatusChange>& listener, int32_t userId)
{
    sptr<ISystemAbilityManager> manager = GetSystemAbilityManager();
    return manager == nullptr ? ERR_NULL_OBJECT : manager->SubscribeSystemAbility(systemAbilityId, listener, userId);
}

int32_t OnDemandHelper::UnSubscribeSystemAbility(int32_t systemAbilityId,
    const sptr<ISystemAbilityStatusChange>& listener)
{
    sptr<ISystemAbilityManager> manager = GetSystemAbilityManager();
    return manager == nullptr ? ERR_NULL_OBJECT : manager->UnSubscribeSystemAbility(systemAbilityId, listener);
}

int32_t OnDemandHelper::UnSubscribeSystemAbility(int32_t systemAbilityId,
    const sptr<ISystemAbilityStatusChange>& listener, int32_t userId)
{
    sptr<ISystemAbilityManager> manager = GetSystemAbilityManager();
    return manager == nullptr ? ERR_NULL_OBJECT : manager->UnSubscribeSystemAbility(systemAbilityId, listener, userId);
}

int32_t OnDemandHelper::SubscribeSystemProcess(const sptr<ISystemProcessStatusChange>& listener)
{
    sptr<ISystemAbilityManager> manager = GetSystemAbilityManager();
    return manager == nullptr ? ERR_NULL_OBJECT : manager->SubscribeSystemProcess(listener);
}

int32_t OnDemandHelper::SubscribeSystemProcess(const sptr<ISystemProcessStatusChange>& listener, int32_t userId)
{
    sptr<ISystemAbilityManager> manager = GetSystemAbilityManager();
    return manager == nullptr ? ERR_NULL_OBJECT : manager->SubscribeSystemProcess(listener, userId);
}

int32_t OnDemandHelper::UnSubscribeSystemProcess(const sptr<ISystemProcessStatusChange>& listener)
{
    sptr<ISystemAbilityManager> manager = GetSystemAbilityManager();
    return manager == nullptr ? ERR_NULL_OBJECT : manager->UnSubscribeSystemProcess(listener);
}

int32_t OnDemandHelper::UnSubscribeSystemProcess(const sptr<ISystemProcessStatusChange>& listener, int32_t userId)
{
    sptr<ISystemAbilityManager> manager = GetSystemAbilityManager();
    return manager == nullptr ? ERR_NULL_OBJECT : manager->UnSubscribeSystemProcess(listener, userId);
}

int32_t OnDemandHelper::RemoveSystemAbility(int32_t systemAbilityId)
{
    SamMockPermission::MockProcess(GetTestProcessName(systemAbilityId));
    sptr<ISystemAbilityManager> manager = GetSystemAbilityManager();
    int32_t result = manager == nullptr ? ERR_NULL_OBJECT : manager->RemoveSystemAbility(systemAbilityId);
    cout << "RemoveSystemAbility systemAbilityId:" << systemAbilityId << " result:" << result << endl;
    return result;
}

int32_t OnDemandHelper::CancelUnloadSystemAbility(int32_t systemAbilityId)
{
    SamMockPermission::MockProcess(GetTestProcessName(systemAbilityId));
    sptr<ISystemAbilityManager> manager = GetSystemAbilityManager();
    int32_t result = manager == nullptr ? ERR_NULL_OBJECT : manager->CancelUnloadSystemAbility(systemAbilityId);
    cout << "CancelUnloadSystemAbility systemAbilityId:" << systemAbilityId << " result:" << result << endl;
    return result;
}

int32_t OnDemandHelper::AddOnDemandSystemAbilityInfo(int32_t systemAbilityId, const std::string& processName)
{
    sptr<ISystemAbilityManager> manager = GetSystemAbilityManager();
    int32_t result = manager == nullptr ? ERR_NULL_OBJECT :
        manager->AddOnDemandSystemAbilityInfo(systemAbilityId, Str8ToStr16(processName));
    cout << "AddOnDemandSystemAbilityInfo saId:" << systemAbilityId << " processName:" << processName
        << " result:" << result << endl;
    return result;
}

int32_t OnDemandHelper::TriggerUnloadSystemAbility(const OnDemandTarget& target, bool cancelUnload)
{
    sptr<IRemoteObject> object = target.useUserIdApi ? GetSystemAbility(target.saId, target.userId) :
        GetSystemAbility(target.saId);
    if (object == nullptr) {
        cout << "TriggerUnloadSystemAbility saId:" << target.saId << " userId:" << target.userId
            << " cancelUnload:" << cancelUnload << " result:" << ERR_NULL_OBJECT << endl;
        return ERR_NULL_OBJECT;
    }
    MessageParcel data;
    MessageParcel reply;
    TestSaRequestCode code = cancelUnload ? TestSaRequestCode::TRIGGER_UNLOAD_CANCEL :
        TestSaRequestCode::TRIGGER_UNLOAD;
    int32_t sendResult = SendTestRequest(object, code, data, reply);
    int32_t result = sendResult == ERR_OK ? reply.ReadInt32() : sendResult;
    cout << "TriggerUnloadSystemAbility accepted, saId:" << target.saId;
    cout << " userId:" << target.userId;
    cout << " cancelUnload:" << cancelUnload << " result:" << result << endl;
    return result;
}

int32_t OnDemandHelper::GetRunningSystemProcess(std::list<SystemProcessInfo>& processInfos)
{
    sptr<ISystemAbilityManager> manager = GetSystemAbilityManager();
    int32_t result = manager == nullptr ? ERR_NULL_OBJECT : manager->GetRunningSystemProcess(processInfos);
    cout << "GetRunningSystemProcess size:" << processInfos.size() << " result:" << result << endl;
    for (const auto& processInfo : processInfos) {
        cout << "processName:" << processInfo.processName << " pid:" << processInfo.pid
            << " uid:" << processInfo.uid << endl;
    }
    return result;
}

int32_t OnDemandHelper::SendStrategy(int32_t type, std::vector<int32_t>& systemAbilityIds, int32_t level,
    const std::string& action)
{
    sptr<ISystemAbilityManager> manager = GetSystemAbilityManager();
    if (manager == nullptr) {
        return ERR_NULL_OBJECT;
    }
    std::string mutableAction = action;
    int32_t result = manager->SendStrategy(type, systemAbilityIds, level, mutableAction);
    cout << "SendStrategy result:" << result << " saCount:" << systemAbilityIds.size() << endl;
    return result;
}

int32_t OnDemandHelper::GetRunningSaExtensionInfoList(const std::string& extension,
    std::vector<ISystemAbilityManager::SaExtensionInfo>& infoList)
{
    SamMockPermission::MockPermission();
    sptr<ISystemAbilityManager> manager = GetSystemAbilityManager();
    int32_t result = manager == nullptr ? ERR_NULL_OBJECT : manager->GetRunningSaExtensionInfoList(extension, infoList);
    cout << "GetRunningSaExtensionInfoList extension:" << extension << " size:" << infoList.size()
        << " result:" << result << endl;
    for (size_t index = 0; index < infoList.size(); ++index) {
        cout << "[" << index << "] saId:" << infoList[index].saId << " processObj:"
            << (infoList[index].processObj == nullptr ? "null" : "valid") << endl;
    }
    return result;
}

int32_t OnDemandHelper::UpdateOnDemandPolicyBySa(const PolicyUpdateRequest& request)
{
    sptr<IRemoteObject> target = GetSystemAbility(request.target.saId);
    if (request.target.useUserIdApi) {
        target = GetSystemAbility(request.target.saId, request.target.userId);
    }
    MessageParcel data;
    MessageParcel reply;
    bool written = data.WriteInt32(static_cast<int32_t>(request.policyType)) &&
        data.WriteInt32(static_cast<int32_t>(request.event.eventId)) && data.WriteString(request.event.name) &&
        data.WriteString(request.event.value) && data.WriteBool(request.event.persistence);
    int32_t sendResult = written ? SendTestRequest(target, TestSaRequestCode::UPDATE_ON_DEMAND_POLICY, data, reply) :
        ERR_FLATTEN_OBJECT;
    int32_t result = sendResult == ERR_OK ? reply.ReadInt32() : sendResult;
    std::cout << "SA_INTERNAL_POLICY_UPDATE saId=" << request.target.saId
              << " userId=" << request.target.userId
              << " type=" << static_cast<int32_t>(request.policyType)
              << " event=" << request.event.name << " result=" << result << std::endl;
    return result;
}

int32_t OnDemandHelper::GetOnDemandPolicyBySa(const PolicyQueryRequest& request)
{
    sptr<IRemoteObject> target = GetSystemAbility(request.target.saId);
    if (request.target.useUserIdApi) {
        target = GetSystemAbility(request.target.saId, request.target.userId);
    }
    MessageParcel data;
    MessageParcel reply;
    bool written = data.WriteInt32(static_cast<int32_t>(request.policyType));
    int32_t sendResult = written ? SendTestRequest(target, TestSaRequestCode::GET_ON_DEMAND_POLICY, data, reply) :
        ERR_FLATTEN_OBJECT;
    if (sendResult != ERR_OK) {
        return sendResult;
    }
    int32_t result = reply.ReadInt32();
    std::string policy = reply.ReadString();
    std::cout << "SA_INTERNAL_POLICY_GET saId=" << request.target.saId
              << " userId=" << request.target.userId
              << " type=" << static_cast<int32_t>(request.policyType)
              << " policy=" << policy << " result=" << result << std::endl;
    return result;
}
#endif
}
