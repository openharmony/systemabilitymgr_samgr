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

namespace OHOS {
namespace Samgr {
namespace {
    constexpr size_t THRESHOLD = 10;
    constexpr const char* HIDUMPER_PROCESS_NAME = "hidumper_service";
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
    SystemAbilityManagerDumper::GetFfrtDumpInfoProc(scheduler, args, result);
    SystemAbilityManagerDumper::GetSAMgrFfrtInfo(result);
    int32_t pid = BuildInt32FromData(data, size);
    SystemAbilityManagerDumper::DumpFfrtInfoByProcName(pid, Str8ToStr16(processName), result);

    std::shared_ptr<SystemAbilityManager> manager = std::make_shared<SystemAbilityManager>();
    manager->abilityStateScheduler_ = std::make_shared<SystemAbilityStateScheduler>();
    manager->Dump(fd, argsWithStr16);
    manager->IpcDumpProc(fd, args);
    manager->IpcDumpAllProcess(fd, cmd);
    manager->IpcDumpSamgrProcess(fd, cmd);
    manager->IpcDumpSingleProcess(fd, cmd, processName);
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
    
    return 0;
}

