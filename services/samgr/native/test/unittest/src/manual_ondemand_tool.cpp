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
constexpr int32_t SLEEP_3_SECONDS = 3 * 1000 * 1000;
std::string g_inputTimeStr = "2023-10-9-10:00:00"; // time format
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
}

static void TestProcess(OHOS::OnDemandHelper& ondemandHelper)
{
    std::string cmd = "";
    cout << "please input proc test case(1-getp/2-initp/3-subp/4-unsubp)" << endl;
    cin >> cmd;
    if (cmd == "getp" || cmd == "1") {
        SamMockPermission::MockProcess("resource_schedule_service");
    } else if (cmd == "initp" || cmd == "2") {
        ondemandHelper.InitSystemProcessStatusChange();
        ondemandHelper.GetSystemProcess();
    } else if (cmd == "subp" || cmd == "3") {
        SamMockPermission::MockProcess("resource_schedule_service");
        ondemandHelper.SubscribeSystemProcess();
    } else if (cmd == "unsubp" || cmd == "4") {
        SamMockPermission::MockProcess("resource_schedule_service");
        ondemandHelper.UnSubscribeSystemProcess();
    } else {
        cout << "invalid input" << endl;
    }
}

static void TestSystemAbility(OHOS::OnDemandHelper& ondemandHelper)
{
    std::string cmd = "";
    cout << "please input sa test case(1-get/2-load/3-unload/4-syncload/5-getinfo)" << endl;
    cin >> cmd;
    int32_t systemAbilityId = 0;
#ifdef SUPPORT_SOFTBUS
    std::string deviceId = ondemandHelper.GetFirstDevice();
#endif
    cout << "please input systemAbilityId for " << cmd << " operation" << endl;
    cin >> systemAbilityId;
    if (cmd == "get" || cmd == "1") {
        ondemandHelper.GetSystemAbility(systemAbilityId);
    } else if (cmd == "load" || cmd == "2") {
        ondemandHelper.OnDemandAbility(systemAbilityId);
#ifdef SUPPORT_SOFTBUS
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
#endif
    } else if (cmd == "unload" || cmd == "3") {
        ondemandHelper.UnloadSystemAbility(systemAbilityId);
    } else if (cmd == "syncload" || cmd == "4") {
        ondemandHelper.TestSyncOnDemandAbility(systemAbilityId);
    } else if (cmd == "getinfo" || cmd == "5") {
        ondemandHelper.GetSystemProcessInfo(systemAbilityId);
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
    cout << "please input on demand event id:" << endl;
    cout << "1:deviceonline on" << endl;
    cout << "2:wifi_status on" << endl;
    cout << "3:persist.samgr.deviceparam true" << endl;
    cout << "4:usual.event.SCREEN_ON" << endl;
    cout << "5:timedevent" << endl;
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
        cout << "please input timeEventId(1,2,3) 1:loopevent 2:awakeloopevent 3: timedevent" << endl;
        int timeEventId = 1;
        cin >> timeEventId;
        event.eventId = OnDemandEventId::TIMED_EVENT;
        if (timeEventId == static_cast<int32_t>(TimeEventId::LOOP_EVENT)) {
            event.name = "loopevent";
        } else if (timeEventId == static_cast<int32_t>(TimeEventId::AWAKE_LOOP_EVENT)) {
            event.name = "awakeloopevent";
        } else if (timeEventId == static_cast<int32_t>(TimeEventId::ORDER_TIMED_EVENT)) {
            event.name = "timedevent";
        }
        cout << "please input the duration";
        string duration = "60";
        cin >> duration;
        event.value = duration;
    }
}

static void CreateOnDemandStopPolicy(SystemAbilityOnDemandEvent& event)
{
    int eventId = 1;
    cout << "please input on demand event id:" << endl;
    cout << "1:deviceonline off" << endl;
    cout << "2:wifi_status off" << endl;
    cout << "3:persist.samgr.deviceparam false" << endl;
    cout << "4:usual.event.SCREEN_OFF" << endl;
    cout << "5:timedevent" << endl;
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
        cout << "please input timeEventId(1,2,3) 1:loopevent 2:awakeloopevent 3: timedevent" << endl;
        int timeEventId = 1;
        cin >> timeEventId;
        event.eventId = OnDemandEventId::TIMED_EVENT;
        if (timeEventId == static_cast<int32_t>(TimeEventId::LOOP_EVENT)) {
            event.name = "loopevent";
        } else if (timeEventId == static_cast<int32_t>(TimeEventId::AWAKE_LOOP_EVENT)) {
            event.name = "awakeloopevent";
        } else if (timeEventId == static_cast<int32_t>(TimeEventId::ORDER_TIMED_EVENT)) {
            event.name = "timedevent";
        }
        cout << "please input the duration";
        string duration = "70";
        cin >> duration;
        event.value = duration;
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
    cout << "please input on demand event id:" << endl;
    cout << "1:deviceonline on" << endl;
    cout << "2:wifi_status on" << endl;
    cout << "3:persist.samgr.deviceparam true" << endl;
    cout << "4:usual.event.SCREEN_ON" << endl;
    cout << "5:timedevent" << endl;
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
    cout << "please input on demand event id:" << endl;
    cout << "1:deviceonline off" << endl;
    cout << "2:wifi_status off" << endl;
    cout << "3:persist.samgr.deviceparam false" << endl;
    cout << "4:usual.event.SCREEN_OFF" << endl;
    cout << "5:timedevent" << endl;
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
#ifdef SUPPORT_SOFTBUS
    std::string cmd = "";
    cout << "please input check case(local/remote)" << endl;
    cin >> cmd;
#endif
    int32_t saId = 0;
    cout << "please input systemAbilityId" << endl;
    cin >> saId;
#ifdef SUPPORT_SOFTBUS
    if (cmd == "local") {
        ondemandHelper.CheckSystemAbility(saId);
    } else if (cmd == "remote") {
        std::string deviceId = ondemandHelper.GetFirstDevice();
        ondemandHelper.CheckSystemAbility(saId, deviceId);
    } else {
        cout << "invalid input" << endl;
    }
#else
    ondemandHelper.CheckSystemAbility(saId);
#endif
}

static void TestMemory(OHOS::OnDemandHelper& ondemandHelper)
{
    std::string cmd = "";
    cout << "please input memory test(1-unloadall/2-getlru/3-unloadprocess)" << endl;
    cin >> cmd;
    if (cmd == "unloadall" || cmd == "1")  {
        ondemandHelper.UnloadAllIdleSystemAbility();
    } else if (cmd == "getlru" || cmd == "2") {
        ondemandHelper.GetLruIdleSystemAbilityProc();
    } else if (cmd == "unloadprocess" || cmd == "3") {
        vector<u16string> processList;
        string proc = "";
        do {
            cout << "please input unload process name(input '-1' to end)" << endl;
            cin >> proc;
            processList.push_back(Str8ToStr16(proc));
        } while (proc != "-1");
        
        ondemandHelper.UnloadProcess(processList);
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
        TestMemory(ondemandHelper);
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
        TestMemory(ondemandHelper);
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
