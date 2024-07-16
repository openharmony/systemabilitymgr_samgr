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
    cout << "please input sa test case(get/load/unload)" << endl;
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

int main(int argc, char* argv[])
{
    SamMockPermission::MockPermission();
    OHOS::OnDemandHelper& ondemandHelper = OnDemandHelper::GetInstance();
    string cmd = "load";
    do {
        cout << "please input operation(sa/proc/param/policy/getExtension/check/6 getEvent)" << endl;
        cmd.clear();
        cin.clear();
        cin >> cmd;
        if (cmd == "param") {
            TestParamPlugin(ondemandHelper);
        } else if (cmd == "sa") {
            TestSystemAbility(ondemandHelper);
        } else if (cmd == "proc") {
            TestProcess(ondemandHelper);
        } else if (cmd == "policy") {
            TestOnDemandPolicy(ondemandHelper);
        } else if (cmd == "6") {
            TestCommonEvent(ondemandHelper);
        } else if (cmd == "getExtension") {
            TestGetExtension(ondemandHelper);
        } else if (cmd == "check") {
            TestCheckSystemAbility(ondemandHelper);
        } else {
            cout << "invalid input" << endl;
        }
        cout << "-----Input q or Q to quit" << endl;
        cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
    } while (cmd[0] != 'q' && cmd[0] != 'Q');
    return 0;
}
