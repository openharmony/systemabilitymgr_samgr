/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
constexpr int32_t FIRST_NUM = 1;
constexpr int32_t SECOND_NUM = 2;
constexpr int32_t THIRD_NUM = 3;
constexpr int32_t FOURTH_NUM = 4;
constexpr int32_t FIFTH_NUM = 5;
constexpr int32_t SIXTH_NUM = 6;
constexpr int32_t SEVENTH_NUM = 7;
}

void TestLoad(OHOS::OnDemandHelper& ondemandHelper)
{
    cout << "input any word to start load test case 1" << endl;
    ondemandHelper.LoadOndemandAbilityCase1();

    cout << "input any word to start load test case 2" << endl;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.LoadOndemandAbilityCase2();

    cout << "input any word to start load test case 3" << endl;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.LoadOndemandAbilityCase3();

    cout << "input any word to start load test case 4" << endl;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.LoadOndemandAbilityCase4();

    cout << "input any word to start load test case 6" << endl;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.LoadOndemandAbilityCase6();

    cout << "input any word to start load test case 7" << endl;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.LoadOndemandAbilityCase7();

    cout << "input any word to start load test case 8" << endl;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.LoadOndemandAbilityCase8();

    cout << "input any word to start load test case 9" << endl;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.LoadOndemandAbilityCase9();

    cout << "input any word to start load test case 10" << endl;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.LoadOndemandAbilityCase10();

    cout << "input any word to start load test case 11" << endl;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.LoadOndemandAbilityCase11();
}

void TestUnload(OHOS::OnDemandHelper& ondemandHelper)
{
    cout << "input any word to start unload test case 1" << endl;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.UnloadOndemandAbilityCase1();
    
    cout << "input any word to start unload test case 2" << endl;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.UnloadOndemandAbilityCase2();

    cout << "input any word to start unload test case 3" << endl;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.UnloadOndemandAbilityCase3();

    cout << "input any word to start unload test case 4" << endl;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.UnloadOndemandAbilityCase4();

    cout << "input any word to start unload test case 5" << endl;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.UnloadOndemandAbilityCase5();

    cout << "input any word to start unload test case 6" << endl;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.UnloadOndemandAbilityCase6();

    cout << "input any word to start unload test case 7" << endl;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.UnloadOndemandAbilityCase7();

    cout << "input any word to start unload test case 8" << endl;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.UnloadOndemandAbilityCase8();

    cout << "input any word to start unload test case 9" << endl;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.UnloadOndemandAbilityCase9();
}

void TestGet(OHOS::OnDemandHelper& ondemandHelper)
{
    cout << "input any word to start get test case 1" << endl;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.GetOndemandAbilityCase1();

    cout << "input any word to start get test case 2" << endl;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.GetOndemandAbilityCase2();
    
    cout << "input any word to start get test case 3" << endl;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.GetOndemandAbilityCase3();

    cout << "input any word to start get test case 4" << endl;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.GetOndemandAbilityCase4();

    cout << "input any word to start get test case 5" << endl;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.GetOndemandAbilityCase5();

    cout << "input any word to start get test case 6" << endl;
    ::system("kill -9 `pidof listen_test`");
    usleep(SLEEP_3_SECONDS);
    ondemandHelper.GetOndemandAbilityCase6();
}

void TestScheduler(OHOS::OnDemandHelper& ondemandHelper)
{
    SamMockPermission::MockProcess("listen_test");
    std::string pause;
    
    TestLoad(ondemandHelper);
    TestUnload(ondemandHelper);
    TestGet(ondemandHelper);
    
    cout << "all test case done" << endl;
    ::system("kill -9 `pidof listen_test`");
}
}

static void TestProcess(OHOS::OnDemandHelper& ondemandHelper, char* inputcmd)
{
    std::string cmd = "";
    cout << "please input proc test case(1-getp/2-initp)" << endl;
    if (strcmp(inputcmd, "getp") == 0 || strcmp(inputcmd, "1") == 0) {
        SamMockPermission::MockProcess("resource_schedule_service");
        ondemandHelper.GetSystemProcess();
    } else if (strcmp(inputcmd, "initp") == 0 || strcmp(inputcmd, "2") == 0) {
        ondemandHelper.InitSystemProcessStatusChange();
        SamMockPermission::MockProcess("resource_schedule_service");
        ondemandHelper.SubscribeSystemProcess();
        ::system("ps -e|grep media_service");
        ::system("kill -9 $(pidof media_service)");
        ::system("ps -e|grep media_service");

        sleep(1);
        ondemandHelper.UnSubscribeSystemProcess();
        ::system("ps -e|grep media_service");
        ::system("kill -9 $(pidof media_service)");
        ::system("ps -e|grep media_service");
    } else {
        cout << "invalid input" << endl;
    }
}

static void TestSystemAbility(OHOS::OnDemandHelper& ondemandHelper, char* inputcmd, char* inputsaid,
    char* inputOtherSaid, char* inputOtherDevice)
{
    cout << "please input sa test case(1-get/2-load/3-unload/4-getinfo/5-syncload)" << endl;
#ifdef SUPPORT_SOFTBUS
    std::string deviceId = ondemandHelper.GetFirstDevice();
#endif
    int32_t systemAbilityId = atoi(inputsaid);
    if (strcmp(inputcmd, "get") == 0 || strcmp(inputcmd, "1") == 0) {
        ondemandHelper.GetSystemAbility(systemAbilityId);
    } else if (strcmp(inputcmd, "load") == 0 || strcmp(inputcmd, "2") == 0) {
        ondemandHelper.OnDemandAbility(systemAbilityId);
#ifdef SUPPORT_SOFTBUS
    } else if (strcmp(inputcmd, "device") == 0) { // get remote networkid
        ondemandHelper.GetDeviceList();
    } else if (strcmp(inputcmd, "loadrmt1") == 0) { // single thread with one device, one system ability, one callback
        ondemandHelper.LoadRemoteAbility(systemAbilityId, deviceId, nullptr);
    } else if (strcmp(inputcmd, "loadrmt2") == 0) { // one device, one system ability, one callback, three threads
        ondemandHelper.LoadRemoteAbilityMuti(systemAbilityId, deviceId);
    } else if (strcmp(inputcmd, "loadrmt3") == 0) { // one device, one system ability, three callbacks, three threads
        ondemandHelper.LoadRemoteAbilityMutiCb(systemAbilityId, deviceId);
    } else if (strcmp(inputcmd, "loadrmt4") == 0) { // one device, three system abilities, one callback, three threads
        ondemandHelper.LoadRemoteAbilityMutiSA(systemAbilityId, deviceId);
    } else if (strcmp(inputcmd, "loadrmt5") == 0) {
        // one device, three system abilities, three callbacks, three threads
        ondemandHelper.LoadRemoteAbilityMutiSACb(systemAbilityId, deviceId);
    } else if (strcmp(inputcmd, "loadrmt6") == 0) { // two devices
        int32_t otherSystemAbilityId = atoi(inputOtherSaid);
        std::string otherDevice = inputOtherDevice;
        ondemandHelper.LoadRemoteAbility(systemAbilityId, deviceId, nullptr);
        ondemandHelper.LoadRemoteAbility(otherSystemAbilityId, otherDevice, nullptr);
    } else if (strcmp(inputcmd, "loadmuti") == 0) {
        ondemandHelper.LoadRemoteAbilityPressure(systemAbilityId, deviceId);
#endif
    } else if (strcmp(inputcmd, "unload") == 0 || strcmp(inputcmd, "3") == 0) {
        ondemandHelper.UnloadSystemAbility(systemAbilityId);
    } else if (strcmp(inputcmd, "getinfo") == 0 || strcmp(inputcmd, "4") == 0) {
        ondemandHelper.GetSystemProcessInfo(systemAbilityId);
    } else if (strcmp(inputcmd, "syncload") == 0 || strcmp(inputcmd, "5") == 0) {
        ondemandHelper.TestSyncOnDemandAbility(systemAbilityId);
    } else {
        cout << "invalid inputcmd" << endl;
    }
}

static void TestParamPlugin(OHOS::OnDemandHelper& ondemandHelper, char* input)
{
    cout << "please input param's value" << endl;
    if (strcmp(input, "false") == 0) {
        int ret = SetParameter("persist.samgr.deviceparam", "false");
        cout << "ret = " << ret <<endl;
    } else if (strcmp(input, "true") == 0) {
        int ret = SetParameter("persist.samgr.deviceparam", "true");
        cout << "ret = " << ret <<endl;
    } else {
        cout << "invalid input" << endl;
    }
}

static void CreateOnDemandStartPolicy(SystemAbilityOnDemandEvent& event, int eventId)
{
    cout << "please input on demand event id:" << endl;
    cout << "1:deviceonline on" << endl;
    cout << "2:wifi_status on" << endl;
    cout << "3:persist.samgr.deviceparam true" << endl;
    cout << "4:usual.event.SCREEN_ON" << endl;
    cout << "5:loopevent 60" << endl;
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

static void CreateOnDemandStopPolicy(SystemAbilityOnDemandEvent& event, int eventId)
{
    cout << "please input on demand event id:" << endl;
    cout << "1:deviceonline off" << endl;
    cout << "2:wifi_status off" << endl;
    cout << "3:persist.samgr.deviceparam false" << endl;
    cout << "4:usual.event.SCREEN_OFF" << endl;
    cout << "5:loopevent 70" << endl;
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

static void TestOnDemandPolicy(OHOS::OnDemandHelper& ondemandHelper, char *argv[])
{
    char* inputcmd = argv[SECOND_NUM];
    char* inputtype = argv[THIRD_NUM];
    char* inputsaid = argv[FOURTH_NUM];
    char* inputeventid = argv[FIFTH_NUM];
    char* anotherinputeventid = argv[SIXTH_NUM];
    cout << "please input on demand policy test case(get/update)" << endl;
    cout << "please input on demand type test case(start/stop/start_multi/stop_multi)" << endl;
    SystemAbilityOnDemandEvent event;
    SystemAbilityOnDemandEvent event2;
    std::vector<SystemAbilityOnDemandEvent> abilityOnDemandEvents;
    int32_t systemAbilityId = atoi(inputsaid);
    if (strcmp(inputcmd, "get") == 0 && strcmp(inputtype, "start") == 0) {
        ondemandHelper.GetOnDemandPolicy(systemAbilityId, OnDemandPolicyType::START_POLICY);
    } else if (strcmp(inputcmd, "get") == 0 && strcmp(inputtype, "stop") == 0) {
        ondemandHelper.GetOnDemandPolicy(systemAbilityId, OnDemandPolicyType::STOP_POLICY);
    } else if (strcmp(inputcmd, "update") == 0 && strcmp(inputtype, "start") == 0) {
        int eventId = atoi(inputeventid);
        CreateOnDemandStartPolicy(event, eventId);
        abilityOnDemandEvents.push_back(event);
        ondemandHelper.UpdateOnDemandPolicy(systemAbilityId, OnDemandPolicyType::START_POLICY, abilityOnDemandEvents);
    } else if (strcmp(inputcmd, "update") == 0 && strcmp(inputtype, "start_multi") == 0) {
        int eventId = atoi(inputeventid);
        int anothereventId = atoi(anotherinputeventid);
        CreateOnDemandStartPolicy(event, eventId);
        CreateOnDemandStartPolicy(event2, anothereventId);
        abilityOnDemandEvents.push_back(event);
        abilityOnDemandEvents.push_back(event2);
        ondemandHelper.UpdateOnDemandPolicy(systemAbilityId, OnDemandPolicyType::START_POLICY, abilityOnDemandEvents);
    } else if (strcmp(inputcmd, "update") == 0 && strcmp(inputtype, "stop") == 0) {
        int eventId = atoi(inputeventid);
        CreateOnDemandStopPolicy(event, eventId);
        abilityOnDemandEvents.push_back(event);
        ondemandHelper.UpdateOnDemandPolicy(systemAbilityId, OnDemandPolicyType::STOP_POLICY, abilityOnDemandEvents);
    } else if (strcmp(inputcmd, "update") == 0 && strcmp(inputtype, "stop_multi") == 0) {
        int eventId = atoi(inputeventid);
        int anothereventId = atoi(anotherinputeventid);
        CreateOnDemandStopPolicy(event, eventId);
        CreateOnDemandStopPolicy(event2, anothereventId);
        abilityOnDemandEvents.push_back(event);
        abilityOnDemandEvents.push_back(event2);
        ondemandHelper.UpdateOnDemandPolicy(systemAbilityId, OnDemandPolicyType::STOP_POLICY, abilityOnDemandEvents);
    } else {
        cout << "invalid input" << endl;
    }
}

static void CreateOnDemandStartPolicy1(SystemAbilityOnDemandEvent& event, int eventId)
{
    cout << "please input on demand event id:" << endl;
    cout << "1:deviceonline on" << endl;
    cout << "2:wifi_status on" << endl;
    cout << "3:persist.samgr.deviceparam true" << endl;
    cout << "4:usual.event.SCREEN_ON" << endl;
    cout << "5:timedevent" << endl;
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

static void CreateOnDemandStopPolicy1(SystemAbilityOnDemandEvent& event, int eventId)
{
    cout << "please input on demand event id:" << endl;
    cout << "1:deviceonline off" << endl;
    cout << "2:wifi_status off" << endl;
    cout << "3:persist.samgr.deviceparam false" << endl;
    cout << "4:usual.event.SCREEN_OFF" << endl;
    cout << "5:timedevent" << endl;
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

static void TestOnDemandPolicy1(OHOS::OnDemandHelper& ondemandHelper, char *argv[])
{
    g_inputTimeStr = argv[SECOND_NUM];
    char* inputcmd = argv[THIRD_NUM];
    char* inputtype = argv[FOURTH_NUM];
    char* inputsaid = argv[FIFTH_NUM];
    char* inputeventid = argv[SIXTH_NUM];
    char* anotherinputeventid = argv[SEVENTH_NUM];
    cout << "please input on demand policy test case(get/update)" << endl;
    cout << "please input on demand type test case(start/stop)" << endl;
    int32_t systemAbilityId = atoi(inputsaid);
    SystemAbilityOnDemandEvent event;
    SystemAbilityOnDemandEvent event2;
    std::vector<SystemAbilityOnDemandEvent> abilityOnDemandEvents;
    if (strcmp(inputcmd, "get") == 0 && strcmp(inputtype, "start") == 0) {
        ondemandHelper.GetOnDemandPolicy(systemAbilityId, OnDemandPolicyType::START_POLICY);
    } else if (strcmp(inputcmd, "get") == 0 && strcmp(inputtype, "stop") == 0) {
        ondemandHelper.GetOnDemandPolicy(systemAbilityId, OnDemandPolicyType::STOP_POLICY);
    } else if (strcmp(inputcmd, "update") == 0 && strcmp(inputtype, "start") == 0) {
        int eventId = atoi(inputeventid);
        CreateOnDemandStartPolicy1(event, eventId);
        abilityOnDemandEvents.push_back(event);
        ondemandHelper.UpdateOnDemandPolicy(systemAbilityId, OnDemandPolicyType::START_POLICY, abilityOnDemandEvents);
    } else if (strcmp(inputcmd, "update") == 0 && strcmp(inputtype, "start_multi") == 0) {
        int eventId = atoi(inputeventid);
        int anothereventId = atoi(anotherinputeventid);
        CreateOnDemandStartPolicy1(event, eventId);
        CreateOnDemandStartPolicy1(event2, anothereventId);
        abilityOnDemandEvents.push_back(event);
        abilityOnDemandEvents.push_back(event2);
        ondemandHelper.UpdateOnDemandPolicy(systemAbilityId, OnDemandPolicyType::START_POLICY, abilityOnDemandEvents);
    } else if (strcmp(inputcmd, "update") == 0 && strcmp(inputtype, "stop") == 0) {
        int eventId = atoi(inputeventid);
        CreateOnDemandStopPolicy1(event, eventId);
        abilityOnDemandEvents.push_back(event);
        ondemandHelper.UpdateOnDemandPolicy(systemAbilityId, OnDemandPolicyType::STOP_POLICY, abilityOnDemandEvents);
    } else if (strcmp(inputcmd, "update") == 0 && strcmp(inputtype, "stop_multi") == 0) {
        int eventId = atoi(inputeventid);
        int anothereventId = atoi(anotherinputeventid);
        CreateOnDemandStopPolicy1(event, eventId);
        CreateOnDemandStopPolicy1(event2, anothereventId);
        abilityOnDemandEvents.push_back(event);
        abilityOnDemandEvents.push_back(event2);
        ondemandHelper.UpdateOnDemandPolicy(systemAbilityId, OnDemandPolicyType::STOP_POLICY, abilityOnDemandEvents);
    } else {
        cout << "invalid input" << endl;
    }
}

static void TestCommonEvent(OHOS::OnDemandHelper& ondemandHelper, char* inputcmd, char* inputsaid, char* inputEvent)
{
    cout << "please input common event test case(1 get/2 get_with_event)" << endl;
    cout << "please input systemAbilityId for 1/2 operation" << endl;
    int32_t saId = atoi(inputsaid);
    if (strcmp(inputcmd, "1") == 0) {
        ondemandHelper.GetCommonEventExtraId(saId, "");
    } else if (strcmp(inputcmd, "2") == 0) {
        cout << "please input common event name" << endl;
        std::string eventName = inputEvent;
        ondemandHelper.GetCommonEventExtraId(saId, eventName);
    } else {
        cout << "invalid input" << endl;
    }
}

static void TestGetExtension(OHOS::OnDemandHelper& ondemandHelper, char* input)
{
    std::string extension = input;
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

static void TestCheckSystemAbility(OHOS::OnDemandHelper& ondemandHelper, char* inputcmd, char* inputsaid)
{
#ifdef SUPPORT_SOFTBUS
    std::string cmd = "";
    cout << "please input check case(local/remote)" << endl;
#endif
    int32_t saId = 0;
    cout << "please input systemAbilityId" << endl;
    saId = atoi(inputsaid);
#ifdef SUPPORT_SOFTBUS
    if (strcmp(inputcmd, "local") == 0) {
        ondemandHelper.CheckSystemAbility(saId);
    } else if (strcmp(inputcmd, "remote") == 0) {
        std::string deviceId = ondemandHelper.GetFirstDevice();
        ondemandHelper.CheckSystemAbility(saId, deviceId);
    } else {
        cout << "invalid input" << endl;
    }
#else
    ondemandHelper.CheckSystemAbility(saId);
#endif
}

static void TestMemory(OHOS::OnDemandHelper& ondemandHelper, char* argv[])
{
    if (strcmp(argv[SECOND_NUM], "unoadall") == 0 || strcmp(argv[SECOND_NUM], "1") == 0) {
        ondemandHelper.UnloadAllIdleSystemAbility();
    } else if (strcmp(argv[SECOND_NUM], "getlru") == 0 || strcmp(argv[SECOND_NUM], "2") == 0) {
        ondemandHelper.GetLruIdleSystemAbilityProc();
    } else if (strcmp(argv[SECOND_NUM], "unloadprocess") == 0 || strcmp(argv[SECOND_NUM], "3") == 0) {
        vector<u16string> processList;
        for (int i = THIRD_NUM; i < ondemandHelper.argc_; ++i) {
            processList.push_back(Str8ToStr16(string(argv[i])));
        }
        ondemandHelper.UnloadProcess(processList);
    } else {
        cout << "invalid input" << endl;
    }
}

static void TestIntCommand(OHOS::OnDemandHelper& ondemandHelper, char* argv[])
{
    if (strcmp(argv[FIRST_NUM], "1") == 0) {
        TestParamPlugin(ondemandHelper, argv[SECOND_NUM]);
    } else if (strcmp(argv[FIRST_NUM], "2") == 0) {
        TestSystemAbility(ondemandHelper, argv[SECOND_NUM], argv[THIRD_NUM], argv[FOURTH_NUM], argv[FIFTH_NUM]);
    } else if (strcmp(argv[FIRST_NUM], "3") == 0) {
        TestProcess(ondemandHelper, argv[SECOND_NUM]);
    } else if (strcmp(argv[FIRST_NUM], "4") == 0) {
        TestOnDemandPolicy(ondemandHelper, argv);
    } else if (strcmp(argv[FIRST_NUM], "5") == 0) {
        TestGetExtension(ondemandHelper, argv[SECOND_NUM]);
    } else if (strcmp(argv[FIRST_NUM], "6") == 0) {
        TestCommonEvent(ondemandHelper, argv[SECOND_NUM], argv[THIRD_NUM], argv[FOURTH_NUM]);
    } else if (strcmp(argv[FIRST_NUM], "7") == 0) {
        TestCheckSystemAbility(ondemandHelper, argv[SECOND_NUM], argv[THIRD_NUM]);
    } else if (strcmp(argv[FIRST_NUM], "8") == 0) {
        TestOnDemandPolicy1(ondemandHelper, argv);
    } else if (strcmp(argv[FIRST_NUM], "9") == 0) {
        TestScheduler(ondemandHelper);
    } else if (strcmp(argv[FIRST_NUM], "10") == 0) {
        TestMemory(ondemandHelper, argv);
    } else {
        cout << "invalid input" << endl;
    }
}

static void TestStringCommand(OHOS::OnDemandHelper& ondemandHelper, char* argv[])
{
    if (strcmp(argv[FIRST_NUM], "param") == 0) {
        TestParamPlugin(ondemandHelper, argv[SECOND_NUM]);
    } else if (strcmp(argv[FIRST_NUM], "sa") == 0) {
        TestSystemAbility(ondemandHelper, argv[SECOND_NUM], argv[THIRD_NUM], argv[FOURTH_NUM], argv[FIFTH_NUM]);
    } else if (strcmp(argv[FIRST_NUM], "proc") == 0) {
        TestProcess(ondemandHelper, argv[SECOND_NUM]);
    } else if (strcmp(argv[FIRST_NUM], "policy") == 0) {
        TestOnDemandPolicy(ondemandHelper, argv);
    } else if (strcmp(argv[FIRST_NUM], "getExtension") == 0) {
        TestGetExtension(ondemandHelper, argv[SECOND_NUM]);
    } else if (strcmp(argv[FIRST_NUM], "getEvent") == 0) {
        TestCommonEvent(ondemandHelper, argv[SECOND_NUM], argv[THIRD_NUM], argv[FOURTH_NUM]);
    } else if (strcmp(argv[FIRST_NUM], "check") == 0) {
        TestCheckSystemAbility(ondemandHelper, argv[SECOND_NUM], argv[THIRD_NUM]);
    } else if (strcmp(argv[FIRST_NUM], "policy_time") == 0) {
        TestOnDemandPolicy1(ondemandHelper, argv);
    } else if (strcmp(argv[FIRST_NUM], "test") == 0) {
        TestScheduler(ondemandHelper);
    } else if (strcmp(argv[FIRST_NUM], "memory") == 0) {
        TestMemory(ondemandHelper, argv);
    } else {
        cout << "invalid input" << endl;
    }
}

int main(int argc, char* argv[])
{
    SamMockPermission::MockPermission();
    OHOS::OnDemandHelper& ondemandHelper = OnDemandHelper::GetInstance();
    ondemandHelper.argc_ = argc;
    cout << "please input operation(1-param/2-sa/3-proc/4-policy/5-getExtension)" << endl;
    cout << "please input operation(6-getEvent/7-check/8-policy_time/9-test/10-memory)" << endl;
    int32_t cmd = atoi(argv[FIRST_NUM]);
    if (cmd == 0) {
        TestStringCommand(ondemandHelper, argv);
    } else {
        TestIntCommand(ondemandHelper, argv);
    }
    return 0;
}
