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

#include "system_ability_manager_dumper.h"

#include "accesstoken_kit.h"
#include "ipc_skeleton.h"
#include "system_ability_manager.h"
#include "if_local_ability_manager.h"

namespace OHOS {
namespace {
const std::string HIDUMPER_PROCESS_NAME = "hidumper_service";
const std::string ARGS_QUERY_SA_STATE = "-sa";
const std::string ARGS_QUERY_PROCESS_STATE = "-p";
const std::string ARGS_QUERY_SA_IN_CURRENT_STATE = "-sm";
const std::string ARGS_HELP = "-h";
const std::string ARGS_QUERY_ALL_SA_STATE = "-l";
constexpr size_t MIN_ARGS_SIZE = 1;
constexpr size_t MAX_ARGS_SIZE = 2;

const std::string IPC_STAT_STR_START = "--start-stat";
const std::string IPC_STAT_STR_STOP = "--stop-stat";
const std::string IPC_STAT_STR_GET = "--stat";
const std::string IPC_STAT_STR_ALL = "all";
}

bool SystemAbilityManagerDumper::IpcDumpIsAllProcess(const std::string& processName)
{
    return processName == IPC_STAT_STR_ALL;
}

bool SystemAbilityManagerDumper::IpcDumpCmdParser(int32_t& cmd, const std::vector<std::string>& args)
{
    if (!CanDump()) {
        HILOGE("IPC Dump failed, not allowed");
        return false;
    }

    if (args.size() < IPC_STAT_CMD_LEN) {
        HILOGE("IPC Dump failed, length error");
        return false;
    }

    if (args[IPC_STAT_CMD_INDEX] == IPC_STAT_STR_START) {
        cmd = IPC_STAT_CMD_START;
    } else if (args[IPC_STAT_CMD_INDEX] == IPC_STAT_STR_STOP) {
        cmd = IPC_STAT_CMD_STOP;
    } else if (args[IPC_STAT_CMD_INDEX] == IPC_STAT_STR_GET){
        cmd = IPC_STAT_CMD_GET;
    } else {
        return false;
    }
    return true;
}

bool SystemAbilityManagerDumper::Dump(std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler,
    const std::vector<std::string>& args, std::string& result)
{
    if (!CanDump()) {
        HILOGE("Dump failed, not allowed");
        return false;
    }
    if (args.size() == MIN_ARGS_SIZE) {
        // -l
        if (args[0] == ARGS_QUERY_ALL_SA_STATE) {
            ShowAllSystemAbilityInfo(abilityStateScheduler, result);
            return true;
        }
        // -h
        if (args[0] == ARGS_HELP) {
            ShowHelp(result);
            return true;
        }
    }
    if (args.size() == MAX_ARGS_SIZE) {
        // -sa said
        if (args[0] == ARGS_QUERY_SA_STATE) {
            int said = atoi(args[1].c_str());
            ShowSystemAbilityInfo(said, abilityStateScheduler, result);
            return true;
        }
        // -p processname
        if (args[0] == ARGS_QUERY_PROCESS_STATE) {
            ShowProcessInfo(args[1], abilityStateScheduler, result);
            return true;
        }
        // -sm state
        if (args[0] == ARGS_QUERY_SA_IN_CURRENT_STATE) {
            ShowAllSystemAbilityInfoInState(args[1], abilityStateScheduler, result);
            return true;
        }
    }
    IllegalInput(result);
    return false;
}

bool SystemAbilityManagerDumper::CanDump()
{
    uint32_t accessToken = IPCSkeleton::GetCallingTokenID();
    Security::AccessToken::NativeTokenInfo nativeTokenInfo;
    int32_t result = Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(accessToken, nativeTokenInfo);
    if (result == ERR_OK && nativeTokenInfo.processName == HIDUMPER_PROCESS_NAME) {
        return true;
    }
    return false;
}

void SystemAbilityManagerDumper::ShowHelp(std::string& result)
{
    result.append("SystemAbilityManager Dump options:\n")
        .append("  [-h] [cmd]...\n")
        .append("cmd maybe one of:\n")
        .append("  -sa said: query sa state infos.\n")
        .append("  -p processname: query process state infos.\n")
        .append("  -sm state: query all sa based on state infos.\n")
        .append("  -l: query all sa state infos.\n");
}

void SystemAbilityManagerDumper::ShowAllSystemAbilityInfo(
    std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler, std::string& result)
{
    if (abilityStateScheduler == nullptr) {
        HILOGE("abilityStateScheduler is nullptr");
        return;
    }
    abilityStateScheduler->GetAllSystemAbilityInfo(result);
}

void SystemAbilityManagerDumper::ShowSystemAbilityInfo(int32_t said,
    std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler, std::string& result)
{
    if (abilityStateScheduler == nullptr) {
        HILOGE("abilityStateScheduler is nullptr");
        return;
    }
    abilityStateScheduler->GetSystemAbilityInfo(said, result);
}

void SystemAbilityManagerDumper::ShowProcessInfo(const std::string& processName,
    std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler, std::string& result)
{
    if (abilityStateScheduler == nullptr) {
        HILOGE("abilityStateScheduler is nullptr");
        return;
    }
    abilityStateScheduler->GetProcessInfo(processName, result);
}

void SystemAbilityManagerDumper::ShowAllSystemAbilityInfoInState(const std::string& state,
    std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler, std::string& result)
{
    if (abilityStateScheduler == nullptr) {
        HILOGE("abilityStateScheduler is nullptr");
        return;
    }
    abilityStateScheduler->GetAllSystemAbilityInfoByState(state, result);
}

void SystemAbilityManagerDumper::IllegalInput(std::string& result)
{
    result.append("The arguments are illegal and you can enter '-h' for help.\n");
}
} // namespace OHOS