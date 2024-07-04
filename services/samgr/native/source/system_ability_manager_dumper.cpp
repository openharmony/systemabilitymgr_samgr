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
#include "ffrt_inner.h"
#include "file_ex.h"
#include "ipc_skeleton.h"
#include "system_ability_manager.h"
#include "if_local_ability_manager.h"
#include "ipc_payload_statistics.h"
#include "samgr_err_code.h"

namespace OHOS {
namespace {
constexpr const char* HIDUMPER_PROCESS_NAME = "hidumper_service";
constexpr const char* ARGS_QUERY_SA_STATE = "-sa";
constexpr const char* ARGS_QUERY_PROCESS_STATE = "-p";
constexpr const char* ARGS_QUERY_SA_IN_CURRENT_STATE = "-sm";
constexpr const char* ARGS_HELP = "-h";
constexpr const char* ARGS_QUERY_ALL_SA_STATE = "-l";
constexpr const char* ARGS_FFRT_SEPARATOR = "|";
constexpr size_t MIN_ARGS_SIZE = 1;
constexpr size_t MAX_ARGS_SIZE = 2;
constexpr int32_t FFRT_DUMP_PROC_LEN = 2;
constexpr int32_t FFRT_DUMP_PIDS_INDEX = 1;
constexpr int FFRT_BUFFER_SIZE = 512 * 1024;

constexpr const char* IPC_STAT_STR_START = "--start-stat";
constexpr const char* IPC_STAT_STR_STOP = "--stop-stat";
constexpr const char* IPC_STAT_STR_GET = "--stat";
constexpr const char* IPC_STAT_STR_ALL = "all";
constexpr const char* IPC_STAT_STR_SAMGR = "samgr";
constexpr const char* IPC_DUMP_SUCCESS = " success\n";
constexpr const char* IPC_DUMP_FAIL = " fail\n";
}

int32_t SystemAbilityManagerDumper::FfrtDumpProc(std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler,
    int32_t fd, const std::vector<std::string>& args)
{
    if (!CanDump()) {
        HILOGE("Dump failed, not allowed");
        return ERR_PERMISSION_DENIED;
    }
    std::string result;
    GetFfrtDumpInfoProc(abilityStateScheduler, args, result);
    return SaveDumpResultToFd(fd, result);
}

bool SystemAbilityManagerDumper::GetFfrtDumpInfoProc(std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler,
    const std::vector<std::string>& args, std::string& result)
{
    if (args.size() < FFRT_DUMP_PROC_LEN || args[FFRT_DUMP_PIDS_INDEX].empty()) {
        HILOGE("FfrtDump param pid not exist");
        IllegalInput(result);
        return false;
    }
    std::string pidStr = args[FFRT_DUMP_PIDS_INDEX];
    std::vector<int32_t> processIds;
    SystemAbilityManagerDumper::FfrtDumpParser(processIds, pidStr);
    if (processIds.empty()) {
        HILOGE("FfrtDumpParser parse failed, illegal input processIdsStr %{public}s ", pidStr.c_str());
        IllegalInput(result);
        return false;
    }
    HILOGD("FfrtDumpProc: processIdsSize=%{public}zu", processIds.size());
    for (const int32_t pid : processIds) {
        if (pid == getpid()) {
            GetSAMgrFfrtInfo(result);
            continue;
        }
        std::u16string processName;
        int32_t queryResult = abilityStateScheduler->GetProcessNameByProcessId(pid, processName);
        if (queryResult != ERR_OK) {
            HILOGE("GetProcessNameByProcessId failed, pid %{public}d not exist", pid);
            result.append("process " + std::to_string(pid) + " not found!\n");
            continue;
        }
        DumpFfrtInfoByProcName(pid, processName, result);
    }
    return true;
}

bool SystemAbilityManagerDumper::FfrtDumpParser(std::vector<int32_t>& processIds, const std::string& processIdsStr)
{
    std::string processIdsVecStr = processIdsStr + ARGS_FFRT_SEPARATOR;
    std::size_t pos = processIdsVecStr.find(ARGS_FFRT_SEPARATOR);
    while (pos != std::string::npos) {
        std::string processIdStr = processIdsVecStr.substr(0, pos);
        processIdsVecStr = processIdsVecStr.substr(pos + 1, processIdsVecStr.size() - pos - 1);
        pos = processIdsVecStr.find(ARGS_FFRT_SEPARATOR);
        int32_t processId = -1;
        if (!StrToInt(processIdStr, processId)) {
            HILOGE("StrToInt processIdStr %{public}s error", processIdStr.c_str());
            continue;
        }
        if (processId > 0) {
            processIds.emplace_back(processId);
        }
    }
    return true;
}

void SystemAbilityManagerDumper::GetSAMgrFfrtInfo(std::string& result)
{
    char* buffer = new char[FFRT_BUFFER_SIZE + 1]();
    buffer[FFRT_BUFFER_SIZE] = 0;
    ffrt_dump(ffrt_dump_cmd_t::DUMP_INFO_ALL, buffer, FFRT_BUFFER_SIZE);
    if (strlen(buffer) == 0) {
        HILOGE("get samgr FfrtDumperInfo failed");
        delete[] buffer;
        return;
    }
    std::string ffrtDumpInfoStr(buffer);
    result.append(ffrtDumpInfoStr + "\n");
    delete[] buffer;
}

void SystemAbilityManagerDumper::DumpFfrtInfoByProcName(int32_t pid, const std::u16string processName,
    std::string& result)
{
    sptr<ILocalAbilityManager> obj =
        iface_cast<ILocalAbilityManager>(SystemAbilityManager::GetInstance()->GetSystemProcess(processName));
    if (obj == nullptr) {
        HILOGE("GetSystemProcess failed, pid:%{public}d processName:%{public}s not exist",
            pid, Str16ToStr8(processName).c_str());
        result.append("process " + std::to_string(pid) + " not found!\n");
        return;
    }
    std::string resultForProcess;
    if (!obj->FfrtDumperProc(resultForProcess)) {
        HILOGE("safwk FfrtDumperProc execute failed");
        return;
    }
    result.append(resultForProcess + "\n");
}

int32_t SystemAbilityManagerDumper::SaveDumpResultToFd(int32_t fd, const std::string& result)
{
    if (!SaveStringToFd(fd, result)) {
        HILOGE("save to fd failed");
        return SAVE_FD_FAIL;
    }
    HILOGD("save to fd success");
    return ERR_OK;
}

bool SystemAbilityManagerDumper::StartSamgrIpcStatistics(std::string& result)
{
    result = std::string("StartIpcStatistics pid:") + std::to_string(getpid());
    bool ret = IPCPayloadStatistics::StartStatistics();
    result += ret ? IPC_DUMP_SUCCESS : IPC_DUMP_FAIL;
    return ret;
}

bool SystemAbilityManagerDumper::StopSamgrIpcStatistics(std::string& result)
{
    result = std::string("StopSamgrIpcStatistics pid:") + std::to_string(getpid());
    bool ret = IPCPayloadStatistics::StopStatistics();
    result += ret ? IPC_DUMP_SUCCESS : IPC_DUMP_FAIL;
    return ret;
}

bool SystemAbilityManagerDumper::GetSamgrIpcStatistics(std::string& result)
{
    result += "********************************GlobalStatisticsInfo********************************";
    result += "\nCurrentPid:";
    result += std::to_string(getpid());
    result += "\nTotalCount:";
    result += std::to_string(IPCPayloadStatistics::GetTotalCount());
    result += "\nTotalTimeCost:";
    result += std::to_string(IPCPayloadStatistics::GetTotalCost());
    std::vector<int32_t> pids;
    pids = IPCPayloadStatistics::GetPids();
    for (unsigned int i = 0; i < pids.size(); i++) {
        result += "\n--------------------------------ProcessStatisticsInfo-------------------------------";
        result += "\nCallingPid:";
        result += std::to_string(pids[i]);
        result += "\nCallingPidTotalCount:";
        result += std::to_string(IPCPayloadStatistics::GetCount(pids[i]));
        result += "\nCallingPidTotalTimeCost:";
        result += std::to_string(IPCPayloadStatistics::GetCost(pids[i]));
        std::vector<IPCInterfaceInfo> intfs;
        intfs = IPCPayloadStatistics::GetDescriptorCodes(pids[i]);
        for (unsigned int j = 0; j < intfs.size(); j++) {
            result += "\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~InterfaceStatisticsInfo~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
            result += "\nDescriptorCode:";
            result += Str16ToStr8(intfs[j].desc) + std::string("_") + std::to_string(intfs[j].code);
            result += "\nDescriptorCodeCount:";
            result += std::to_string(
                IPCPayloadStatistics::GetDescriptorCodeCount(pids[i], intfs[j].desc, intfs[j].code));
            result += "\nDescriptorCodeTimeCost:";
            result += "\nTotal:";
            result += std::to_string(
                IPCPayloadStatistics::GetDescriptorCodeCost(pids[i], intfs[j].desc, intfs[j].code).totalCost);
            result += " | Max:";
            result += std::to_string(
                IPCPayloadStatistics::GetDescriptorCodeCost(pids[i], intfs[j].desc, intfs[j].code).maxCost);
            result += " | Min:";
            result += std::to_string(
                IPCPayloadStatistics::GetDescriptorCodeCost(pids[i], intfs[j].desc, intfs[j].code).minCost);
            result += " | Avg:";
            result += std::to_string(
                IPCPayloadStatistics::GetDescriptorCodeCost(pids[i], intfs[j].desc, intfs[j].code).averCost);
            result += "\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
        }
        result += "\n------------------------------------------------------------------------------------";
    }
    result += "\n************************************************************************************\n";
    return true;
}

bool SystemAbilityManagerDumper::IpcDumpIsAllProcess(const std::string& processName)
{
    return processName == IPC_STAT_STR_ALL;
}

bool SystemAbilityManagerDumper::IpcDumpIsSamgr(const std::string& processName)
{
    return processName == IPC_STAT_STR_SAMGR;
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
    } else if (args[IPC_STAT_CMD_INDEX] == IPC_STAT_STR_GET) {
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