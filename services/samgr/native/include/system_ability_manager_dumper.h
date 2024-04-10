/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef SERVICES_SAMGR_NATIVE_INCLUDE_SYSTEM_ABILITY_MANAGER_DUMPER_H
#define SERVICES_SAMGR_NATIVE_INCLUDE_SYSTEM_ABILITY_MANAGER_DUMPER_H

#include <string>
#include <vector>

#include "schedule/system_ability_state_scheduler.h"

namespace OHOS {
enum {
    IPC_STAT_PREFIX_INDEX = 0,
    IPC_STAT_PROCESS_INDEX = 1,
    IPC_STAT_CMD_INDEX = 2
};

constexpr int32_t IPC_STAT_CMD_LEN 3

class SystemAbilityManagerDumper {
public:
    static bool Dump(std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler,
        const std::vector<std::string>& args, std::string& result);
    static bool IpcDumpCmdParser(int32_t& cmd, const std::vector<std::string>& args);
    static bool IpcDumpIsAllProcess(const std::string& processName);
private:
    SystemAbilityManagerDumper() = default;
    ~SystemAbilityManagerDumper() = default;
    static bool CanDump();
    static void ShowHelp(std::string& result);
    static void ShowAllSystemAbilityInfo(std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler,
        std::string& result);
    static void ShowSystemAbilityInfo(int32_t said,
        std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler, std::string& result);
    static void ShowProcessInfo(const std::string& processName,
        std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler, std::string& result);
    static void ShowAllSystemAbilityInfoInState(const std::string& state,
        std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler, std::string& result);
    static void IllegalInput(std::string& result);
};
} // namespace OHOS
#endif // SERVICES_SAMGR_NATIVE_INCLUDE_SYSTEM_ABILITY_MANAGER_DUMPER_H