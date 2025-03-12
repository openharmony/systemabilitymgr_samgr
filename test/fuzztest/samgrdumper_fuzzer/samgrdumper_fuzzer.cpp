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

#include "samgrdumper_fuzzer.h"

#define private public
#include "system_ability_manager.h"
#include "system_ability_manager_dumper.h"
#include "schedule/system_ability_state_scheduler.h"
#include "sam_mock_permission.h"
#include "sa_status_change_mock.h"
#include "securec.h"

namespace OHOS {
namespace Samgr {
namespace {
    constexpr size_t THRESHOLD = 10;
    constexpr const char* HIDUMPER_PROCESS_NAME = "hidumper_service";
    const uint8_t *g_baseFuzzData = nullptr;
    size_t g_baseFuzzSize = 0;
    size_t g_baseFuzzPos;
    constexpr int32_t SAID = 1493;
}

template <class T> T GetData()
{
    T object{};
    size_t objectSize = sizeof(object);
    if (g_baseFuzzData == nullptr || objectSize > g_baseFuzzSize - g_baseFuzzPos) {
        return object;
    }
    errno_t ret = memcpy_s(&object, objectSize, g_baseFuzzData + g_baseFuzzPos, objectSize);
    if (ret != EOK) {
        return {};
    }
    g_baseFuzzPos += objectSize;
    return object;
}

int32_t BuildInt32FromData(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return 0;
    }
    int32_t int32Val = *reinterpret_cast<const int32_t *>(data);
    return int32Val;
}

std::string BuildStringFromData(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return "";
    }
    std::string strVal(reinterpret_cast<const char *>(data), size);
    return strVal;
}

void SamgrDumperFuzzTest(const uint8_t* data, size_t size)
{
    SamMockPermission::MockProcess(HIDUMPER_PROCESS_NAME);
    std::string strVal = BuildStringFromData(data, size);
    std::vector<std::string> args;
    SplitStr(strVal, " ", args);
    std::vector<std::u16string> argsWithStr16;
    for (size_t i = 0; i < args.size(); i++) {
        argsWithStr16.emplace_back(Str8ToStr16(args[i]));
    }
    std::string result;
    std::shared_ptr<SystemAbilityStateScheduler> scheduler = std::make_shared<SystemAbilityStateScheduler>();
    int32_t fd = -1;
    SystemAbilityManagerDumper::FfrtDumpProc(scheduler, fd, args);
    SystemAbilityManagerDumper::Dump(scheduler, args, result);
    int32_t cmd = -1;
    SystemAbilityManagerDumper::IpcDumpCmdParser(cmd, args);
    std::string processName = BuildStringFromData(data, size);
    SystemAbilityManagerDumper::IpcDumpIsSamgr(processName);
    SystemAbilityManagerDumper::IpcDumpIsAllProcess(processName);
    SystemAbilityManagerDumper::GetSamgrIpcStatistics(result);
    SystemAbilityManagerDumper::StopSamgrIpcStatistics(result);
    SystemAbilityManagerDumper::StartSamgrIpcStatistics(result);
    SystemAbilityManagerDumper::GetSAMgrFfrtInfo(result);
    int32_t pid = BuildInt32FromData(data, size);
    SystemAbilityManagerDumper::DumpFfrtInfoInProc(scheduler, pid, result);

    std::shared_ptr<SystemAbilityManager> manager = std::make_shared<SystemAbilityManager>();
    manager->abilityStateScheduler_ = std::make_shared<SystemAbilityStateScheduler>();
    manager->Dump(fd, argsWithStr16);
    manager->IpcDumpProc(fd, args);
    manager->IpcDumpAllProcess(fd, cmd);
    manager->IpcDumpSamgrProcess(fd, cmd);
    manager->IpcDumpSingleProcess(fd, cmd, processName);
}

void FuzzListenerDumpProc(const uint8_t* data, size_t size)
{
    SamMockPermission::MockProcess(HIDUMPER_PROCESS_NAME);
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    int32_t pid1 = GetData<int32_t>();
    int32_t pid2 = GetData<int32_t>();
    std::map<int32_t, std::list<SAListener>> dumpListeners;
    std::list<SAListener> listeners;
    sptr<SaStatusChangeMock> listener(new SaStatusChangeMock());
    listeners.emplace_back(listener, pid1);
    listeners.emplace_back(listener, pid2);
    dumpListeners[SAID] = listeners;
    int32_t fd = -1;
    std::vector<std::string> args;
    args.push_back("test");
    args.push_back("-h");
    SystemAbilityManagerDumper::ListenerDumpProc(dumpListeners, fd, args);
    args.clear();

    args.push_back("test");
    args.push_back("-l");
    args.push_back("-sa");
    SystemAbilityManagerDumper::ListenerDumpProc(dumpListeners, fd, args);
    args.clear();

    args.push_back("test");
    args.push_back("-l");
    args.push_back("-p");
    SystemAbilityManagerDumper::ListenerDumpProc(dumpListeners, fd, args);
    args.clear();

    args.push_back("test");
    args.push_back("-sa");
    args.push_back(ToString(SAID));
    SystemAbilityManagerDumper::ListenerDumpProc(dumpListeners, fd, args);
    args.clear();

    args.push_back("test");
    args.push_back("-p");
    args.push_back(ToString(pid1));
    SystemAbilityManagerDumper::ListenerDumpProc(dumpListeners, fd, args);
    args.clear();
}

void FuzzFfrtLoadMetrics(const uint8_t* data, size_t size)
{
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    int32_t pid = GetData<int32_t>();
    std::shared_ptr<SystemAbilityStateScheduler> scheduler = std::make_shared<SystemAbilityStateScheduler>();
    int32_t fd = -1;
    std::vector<std::string> args;
    args.push_back("--ffrt");
    args.push_back(ToString(pid));
    args.push_back("--start-stat");
    std::string result;
    SystemAbilityManagerDumper::GetFfrtLoadMetrics(scheduler, fd, args, result);
    SystemAbilityManagerDumper::ClearFfrtStatistics();
    SystemAbilityManagerDumper::GetFfrtLoadMetrics(scheduler, fd, args, result);
    args.pop_back();
    args.push_back("--stop-stat");
    SystemAbilityManagerDumper::GetFfrtLoadMetrics(scheduler, fd, args, result);
    args.pop_back();
    args.push_back("--stat");
    SystemAbilityManagerDumper::GetFfrtLoadMetrics(scheduler, fd, args, result);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::Samgr::THRESHOLD) {
        return 0;
    }

    OHOS::Samgr::SamgrDumperFuzzTest(data, size);
    OHOS::Samgr::FuzzListenerDumpProc(data, size);
    OHOS::Samgr::FuzzFfrtLoadMetrics(data, size);

    return 0;
}

