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

using namespace std;
namespace OHOS {
namespace {
constexpr const char* HIDUMPER_PROCESS_NAME = "hidumper_service";
constexpr const char* ARGS_QUERY_SA = "-sa";
constexpr const char* ARGS_QUERY_PROCESS = "-p";
constexpr const char* ARGS_QUERY_SA_IN_CURRENT_STATE = "-sm";
constexpr const char* ARGS_HELP = "-h";
constexpr const char* ARGS_QUERY_ALL = "-l";
constexpr const char* ARGS_FFRT_SEPARATOR = "|";
constexpr size_t MIN_ARGS_SIZE = 1;
constexpr size_t MAX_ARGS_SIZE = 2;
constexpr int32_t FFRT_DUMP_PROC_LEN = 2;
constexpr int32_t FFRT_DUMP_PIDS_INDEX = 1;
constexpr int FFRT_BUFFER_SIZE = 512 * 1024;
constexpr int LISTENER_BASE_INDEX = 1;
constexpr int32_t FFRT_METRIC_CMD_INDEX = 2;
constexpr int32_t FFRT_DUMP_METRIC_LEN = 3;
constexpr int32_t COLLECT_FFRT_METRIC_MAX_SIZE = 5000;
constexpr int32_t FFRT_STAT_SIZE = sizeof(ffrt_stat);
constexpr int32_t BUFFER_SIZE = FFRT_STAT_SIZE * COLLECT_FFRT_METRIC_MAX_SIZE;
constexpr int32_t DELAY_TIME = 60 * 1000;
constexpr const char* FFRT_STAT_STR_START = "--start-stat";
constexpr const char* FFRT_STAT_STR_STOP = "--stop-stat";
constexpr const char* FFRT_STAT_STR_GET = "--stat";
constexpr const char* IPC_STAT_STR_START = "--start-stat";
constexpr const char* IPC_STAT_STR_STOP = "--stop-stat";
constexpr const char* IPC_STAT_STR_GET = "--stat";
constexpr const char* IPC_STAT_STR_ALL = "all";
constexpr const char* IPC_STAT_STR_SAMGR = "samgr";
constexpr const char* IPC_DUMP_SUCCESS = " success\n";
constexpr const char* IPC_DUMP_FAIL = " fail\n";

}

std::shared_ptr<FFRTHandler> SystemAbilityManagerDumper::handler_ = nullptr;
char* SystemAbilityManagerDumper::ffrtMetricBuffer = nullptr;
bool SystemAbilityManagerDumper::collectEnable = false;
samgr::mutex SystemAbilityManagerDumper::ffrtMetricLock;

void SystemAbilityManagerDumper::ShowListenerHelp(string& result)
{
    result.append("SystemAbilityManager Listener Dump options:\n")
        .append("  [-h] [cmd]...\n")
        .append("cmd maybe one of:\n")
        .append("  -sa [said]: query sa listener infos.\n")
        .append("  -p [pid]: query process listener infos.\n")
        .append("  -l [-sa | -p]: query all sa listener infos by [sa | process].\n");
}

int32_t SystemAbilityManagerDumper::ListenerDumpProc(map<int32_t, list<SAListener>>& listeners,
    int32_t fd, const vector<string>& args)
{
    if (!CanDump()) {
        HILOGE("Dump failed, not allowed");
        return ERR_PERMISSION_DENIED;
    }
    string result;
    GetListenerDumpProc(listeners, args, result);
    return SaveDumpResultToFd(fd, result);
}

void SystemAbilityManagerDumper::GetListenerDumpProc(map<int32_t, list<SAListener>>& listeners,
    const vector<string>& args, string& result)
{
    if (args.size() == MIN_ARGS_SIZE + 1) {
        // -h
        if (args[LISTENER_BASE_INDEX] == ARGS_HELP) {
            ShowListenerHelp(result);
            return;
        }
    } else if (args.size() == MAX_ARGS_SIZE + 1) {
        // -l
        if (args[LISTENER_BASE_INDEX] == ARGS_QUERY_ALL) {
            // -sa
            if (args[LISTENER_BASE_INDEX + 1] == ARGS_QUERY_SA) {
                ShowAllBySA(listeners, result);
                return;
            }
            // -p
            if (args[LISTENER_BASE_INDEX + 1] == ARGS_QUERY_PROCESS) {
                ShowAllByCallingPid(listeners, result);
                return;
            }
        }
        // -sa said
        if (args[LISTENER_BASE_INDEX] == ARGS_QUERY_SA) {
            int said = atoi(args[LISTENER_BASE_INDEX + 1].c_str());
            ShowCallingPidBySA(listeners, said, result);
            return;
        }
        // -p pid
        if (args[LISTENER_BASE_INDEX] == ARGS_QUERY_PROCESS) {
            int callingPid = atoi(args[LISTENER_BASE_INDEX + 1].c_str());
            ShowSAByCallingPid(listeners, callingPid, result);
            return;
        }
    }
    IllegalInput(result);
}

void SystemAbilityManagerDumper::ShowAllBySA(map<int32_t, list<SAListener>>& listeners,
    string& result)
{
    result += "********************************ShowAllBySA********************************";
    set<sptr<IRemoteObject>> listenerSet;
    for (auto iter : listeners) {
        if (iter.second.size() == 0) {
            continue;
        }
        result += "\n\n--------------------------------SA:";
        result += to_string(iter.first);
        result += ", SubCnt:";
        result += to_string(iter.second.size());
        result += "--------------------------------";
        map<int32_t, int32_t> pidCnt;
        for (auto saListener : iter.second) {
            pidCnt[saListener.callingPid]++;
            listenerSet.insert(saListener.listener->AsObject());
        }
        for (auto iter : pidCnt) {
            result += "\ncallingPid:";
            result += to_string(iter.first);
            result += ", cnt:";
            result += to_string(iter.second);
        }
    }
    result += "\n--------------------------------TotalListenerCnt:";
    result += to_string(listenerSet.size());
    result += "--------------------------------";
    result += "\n***************************************************************************\n";
}

void SystemAbilityManagerDumper::ShowAllByCallingPid(map<int32_t, list<SAListener>>& listeners,
    string& result)
{
    result += "********************************ShowAllByCallingPid********************************";
    map<int32_t, list<pair<int32_t, sptr<ISystemAbilityStatusChange>>>> subscribeMap;
    map<int32_t, set<sptr<IRemoteObject>>> pidListenerMap;
    size_t totalSum = 0;
    for (auto iter : listeners) {
        int32_t said = iter.first;
        for (auto saListener : iter.second) {
            subscribeMap[saListener.callingPid].push_back({said, saListener.listener});
            pidListenerMap[saListener.callingPid].insert(saListener.listener->AsObject());
        }
    }
    vector<pair<int32_t, set<sptr<IRemoteObject>>>> vec(pidListenerMap.begin(), pidListenerMap.end());
    auto cmp = [](const pair<int32_t, set<sptr<IRemoteObject>>>& p1,
        const pair<int32_t, set<sptr<IRemoteObject>>>& p2) {
        return p1.second.size() > p2.second.size();
    };
    sort(vec.begin(), vec.end(), cmp);
    for (auto iter : vec) {
        result += "\n\n--------------------------------CallingPid:";
        result += to_string(iter.first);
        result += ", ListenerCnt:";
        result += to_string(iter.second.size());
        result += "--------------------------------";
        totalSum += iter.second.size();
        map<int32_t, int32_t> saCnt;
        for (auto p : subscribeMap[iter.first]) {
            saCnt[p.first]++;
        }
        for (auto iter : saCnt) {
            result += "\nSA:";
            result += to_string(iter.first);
            result += ", cnt:";
            result += to_string(iter.second);
        }
    }
    result += "\n--------------------------------TotalListenerCnt:";
    result += to_string(totalSum);
    result += "--------------------------------";
    result += "\n***********************************************************************************\n";
}

void SystemAbilityManagerDumper::ShowCallingPidBySA(map<int32_t, list<SAListener>>& listeners,
    int32_t said, string& result)
{
    result += "********************************ShowCallingPidBySA********************************";
    result += "\n--------------------------------SA:";
    result += to_string(said);
    result += "--------------------------------";
    map<int32_t, int32_t> pidCnt;
    for (auto iter : listeners) {
        if (iter.first == said) {
            for (auto saListener : iter.second) {
                pidCnt[saListener.callingPid]++;
            }
            break;
        }
    }
    int32_t totalSum = 0;
    for (auto iter : pidCnt) {
        result += "\ncallingPid:";
        result += to_string(iter.first);
        result += ", cnt:";
        result += to_string(iter.second);
        totalSum += iter.second;
    }
    result += "\n--------------------------------TotalSubCnt:";
    result += to_string(totalSum);
    result += "--------------------------------";
    result += "\n**********************************************************************************\n";
}

void SystemAbilityManagerDumper::ShowSAByCallingPid(map<int32_t, list<SAListener>>& listeners,
    int32_t callingPid, string& result)
{
    result += "********************************ShowSAByCallingPid********************************";
    result += "\n--------------------------------CallingPid:";
    result += to_string(callingPid);
    result += "--------------------------------";
    map<int32_t, int32_t> saCnt;
    set<sptr<IRemoteObject>> listenerSet;
    for (auto iter : listeners) {
        int32_t said = iter.first;
        for (auto saListener : iter.second) {
            if (saListener.callingPid == callingPid) {
                saCnt[said]++;
                listenerSet.insert(saListener.listener->AsObject());
            }
        }
    }
    for (auto iter : saCnt) {
        result += "\nSA:";
        result += to_string(iter.first);
        result += ", cnt:";
        result += to_string(iter.second);
    }
    result += "\n--------------------------------ListenerCnt:";
    result += to_string(listenerSet.size());
    result += "--------------------------------";
    result += "\n**********************************************************************************\n";
}

int32_t SystemAbilityManagerDumper::FfrtDumpProc(std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler,
    int32_t fd, const std::vector<std::string>& args)
{
    if (!CanDump()) {
        HILOGE("Dump failed, not allowed");
        return ERR_PERMISSION_DENIED;
    }
    std::string result;
    if (args.size() < FFRT_DUMP_PROC_LEN || args[FFRT_DUMP_PIDS_INDEX].empty()) {
        HILOGE("FfrtDump param pid not exist");
        IllegalInput(result);
        return SaveDumpResultToFd(fd, result);
    }
    if (args.size() == FFRT_DUMP_PROC_LEN) {
        GetFfrtDumpInfoProc(abilityStateScheduler, args, result);
        return SaveDumpResultToFd(fd, result);
    }
    if (args.size() == FFRT_DUMP_METRIC_LEN) {
        GetFfrtLoadMetrics(abilityStateScheduler, fd, args, result);
        return SaveDumpResultToFd(fd, result);
    }
    IllegalInput(result);
    return SaveDumpResultToFd(fd, result);
}

void SystemAbilityManagerDumper::GetFfrtLoadMetrics(std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler,
    int32_t fd, const std::vector<std::string>& args, std::string& result)
{
    std::string pidStr = args[FFRT_DUMP_PIDS_INDEX];
    std::vector<int32_t> processIds;
    FfrtDumpParser(processIds, pidStr);
    if (processIds.empty()) {
        HILOGE("FfrtDumpParser parse failed, illegal input processIdsStr %{public}s ", pidStr.c_str());
        IllegalInput(result);
        return;
    }
    int32_t cmd = -1;
    if (!FfrtStatCmdParser(cmd, args)) {
        IllegalInput(result);
        return;
    }
    CollectFfrtMetricInfoInProcs(fd, processIds, abilityStateScheduler, cmd, result);
}

bool SystemAbilityManagerDumper::FfrtStatCmdParser(int32_t& cmd, const std::vector<std::string>& args)
{
    if (args[FFRT_METRIC_CMD_INDEX] == FFRT_STAT_STR_START) {
        cmd = FFRT_STAT_CMD_START;
    } else if (args[FFRT_METRIC_CMD_INDEX] == FFRT_STAT_STR_STOP) {
        cmd = FFRT_STAT_CMD_STOP;
    } else if (args[FFRT_METRIC_CMD_INDEX] == FFRT_STAT_STR_GET) {
        cmd = FFRT_STAT_CMD_GET;
    } else {
        return false;
    }
    return true;
}

void SystemAbilityManagerDumper::CollectFfrtMetricInfoInProcs(int32_t fd, const std::vector<int32_t>& processIds,
    std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler, int32_t cmd, std::string& result)
{
    for (const int32_t pid : processIds) {
        if (pid == getpid()) {
            CollectFfrtStatistics(cmd, result);
            continue;
        }
        sptr<ILocalAbilityManager> obj = GetProcByProcessId(abilityStateScheduler, pid);
        if (obj == nullptr) {
            HILOGE("CollectFfrtMetricInfoInProcs GetSystemProcess failed");
            result.append("process " + std::to_string(pid) + " not found!\n");
            continue;
        }
        obj->FfrtStatCmdProc(fd, cmd);
    }
}

bool SystemAbilityManagerDumper::StartFfrtStatistics(std::string& result)
{
    if (collectEnable) {
        result.append("collect has been started\n");
        return false;
    }
    ClearFfrtStatisticsBufferLocked();
    ffrtMetricBuffer = new char[BUFFER_SIZE]();
    auto ret = ffrt_dump(ffrt_dump_cmd_t::DUMP_START_STAT, ffrtMetricBuffer, BUFFER_SIZE);
    if (ret != ERR_OK) {
        ClearFfrtStatisticsBufferLocked();
        result.append("collect start failed\n");
        return false;
    }
    collectEnable = true;
    result.append("collect start success\n");
    if (handler_ == nullptr) {
        handler_ = std::make_shared<FFRTHandler>("ffrtDumpHandler");
    }
    HILOGI("StartFfrtStatistics PostTask delayTime:%{public}dms", DELAY_TIME);
    handler_->PostTask(ClearFfrtStatistics, "ClearFfrtStatistics", DELAY_TIME);
    return true;
}

bool SystemAbilityManagerDumper::StopFfrtStatistics(std::string& result)
{
    if (!collectEnable) {
        result.append("collect has not been started\n");
        return false;
    }
    collectEnable = false;
    auto ret = ffrt_dump(ffrt_dump_cmd_t::DUMP_STOP_STAT, ffrtMetricBuffer, BUFFER_SIZE);
    if (ret != ERR_OK) {
        ClearFfrtStatisticsBufferLocked();
        result.append("collect stop failed\n");
        return false;
    }
    result.append("collect stop success\n");
    return true;
}

bool SystemAbilityManagerDumper::GetFfrtStatistics(std::string& result)
{
    if (collectEnable) {
        result.append("collect has not been stopped\n");
        return false;
    }
    if (ffrtMetricBuffer == nullptr) {
        result.append("info not collected\n");
        return false;
    }
    FfrtStatisticsParser(result);
    ClearFfrtStatisticsBufferLocked();
    handler_ = nullptr;
    return true;
}

void SystemAbilityManagerDumper::FfrtStatisticsParser(std::string& result)
{
    ffrt_stat* currentStat = (ffrt_stat*)ffrtMetricBuffer;
    char* lastStat = ffrtMetricBuffer + BUFFER_SIZE;
    std::string taskInfo;
    uint64_t maxTime = 0;
    uint64_t minTime = std::numeric_limits<uint64_t>::max();
    uint64_t sumTime = 0;
    uint64_t avgTime = 0;
    uint64_t count = 0;
    while ((char*)currentStat < lastStat && std::strcmp(currentStat->taskName, "") != 0) {
        if (currentStat->startTime > currentStat->endTime) {
            currentStat = (ffrt_stat*)((char*)currentStat + FFRT_STAT_SIZE);
            continue;
        }
        auto duration = currentStat->endTime - currentStat->startTime;
        sumTime += duration;
        maxTime = std::max(maxTime, duration);
        minTime = std::min(minTime, duration);
        ++count;
        taskInfo.append(currentStat->taskName);
        taskInfo.append(" " + ToString(currentStat->startTime));
        taskInfo.append(" " + ToString(currentStat->endTime) + "\n");
        currentStat = (ffrt_stat*)((char*)currentStat + FFRT_STAT_SIZE);
    }
    if (count == 0) {
        minTime = 0;
    } else {
        avgTime = sumTime / count;
    }
    result.append("sumTime:" + ToString(sumTime) + " maxTime:" + ToString(maxTime));
    result.append(" minTime:" + ToString(minTime) + " avgTime:" + ToString(avgTime));
    result.append(" cntTime:" + ToString(count) + "\n");
    result.append("-------------------------------------------------------------------------------------------\n");
    result.append("taskName                                                        startTime(us)   endTime(us)\n");
    result.append("-------------------------------------------------------------------------------------------\n");
    result.append(taskInfo);
    result.append("-------------------------------------------------------------------------------------------\n");
}

void SystemAbilityManagerDumper::ClearFfrtStatisticsBufferLocked()
{
    if (ffrtMetricBuffer != nullptr) {
        delete[] ffrtMetricBuffer;
        ffrtMetricBuffer = nullptr;
        HILOGI("ClearFfrtStatisticsBuffer success");
    }
    if (handler_ != nullptr) {
        handler_->RemoveTask("ClearFfrtStatistics");
    }
}

void SystemAbilityManagerDumper::ClearFfrtStatistics()
{
    HILOGW("ClearFfrtStatistics start");
    std::lock_guard<samgr::mutex> autoLock(ffrtMetricLock);
    if (collectEnable) {
        auto ret = ffrt_dump(ffrt_dump_cmd_t::DUMP_STOP_STAT, ffrtMetricBuffer, BUFFER_SIZE);
        if (ret != ERR_OK) {
            HILOGE("ClearFfrtStatistics stop ffrt_dump err:%{public}d", ret);
        }
        collectEnable = false;
    }
    ClearFfrtStatisticsBufferLocked();
}

bool SystemAbilityManagerDumper::CollectFfrtStatistics(int32_t cmd, std::string& result)
{
    std::lock_guard<samgr::mutex> autoLock(ffrtMetricLock);
    result.append("pid:" + ToString(getpid()) + " ");
    auto ret = false;
    switch (cmd) {
        case FFRT_STAT_CMD_START: {
            ret = StartFfrtStatistics(result);
            break;
        }
        case FFRT_STAT_CMD_STOP: {
            ret = StopFfrtStatistics(result);
            break;
        }
        case FFRT_STAT_CMD_GET: {
            ret = GetFfrtStatistics(result);
            break;
        }
        default:
            break;
    }
    return ret;
}

sptr<ILocalAbilityManager> SystemAbilityManagerDumper::GetProcByProcessId(
    std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler, int32_t processId)
{
    std::u16string processName;
    int32_t queryResult = abilityStateScheduler->GetProcessNameByProcessId(processId, processName);
    if (queryResult != ERR_OK) {
        HILOGE("GetProcessNameByProcessId failed, pid %{public}d not exist", processId);
        return nullptr;
    }
    sptr<ILocalAbilityManager> obj =
        iface_cast<ILocalAbilityManager>(SystemAbilityManager::GetInstance()->GetSystemProcess(processName));
    if (obj == nullptr) {
        HILOGE("GetSystemProcess failed, pid:%{public}d processName:%{public}s not exist",
            processId, Str16ToStr8(processName).c_str());
    }
    return obj;
}

bool SystemAbilityManagerDumper::GetFfrtDumpInfoProc(std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler,
    const std::vector<std::string>& args, std::string& result)
{
    std::string pidStr = args[FFRT_DUMP_PIDS_INDEX];
    std::vector<int32_t> processIds;
    FfrtDumpParser(processIds, pidStr);
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
        DumpFfrtInfoInProc(abilityStateScheduler, pid, result);
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

void SystemAbilityManagerDumper::DumpFfrtInfoInProc(
    std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler, int32_t pid, std::string& result)
{
    sptr<ILocalAbilityManager> obj = GetProcByProcessId(abilityStateScheduler, pid);
    if (obj == nullptr) {
        HILOGE("DumpFfrtInfoInProc GetSystemProcess failed");
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
        if (args[0] == ARGS_QUERY_ALL) {
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
        if (args[0] == ARGS_QUERY_SA) {
            int said = atoi(args[1].c_str());
            ShowSystemAbilityInfo(said, abilityStateScheduler, result);
            return true;
        }
        // -p processname
        if (args[0] == ARGS_QUERY_PROCESS) {
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
        .append("  -l: query all sa state infos.\n")
        .append("  --listener -h: help text for listener.\n")
        .append("  --ffrt [pid1|pid2] --start-stat/--stop-stat/--stat: start/stop/get")
        .append(" the FFRT load statistics of a process.\n")
        .append("  --ffrt [pid1|pid2]: query the FFRT dump infos of a process.\n")
        .append("  --ipc procname/all --start-stat/--stop-stat/--stat: start/stop/get")
        .append(" the IPC load statistics of a process.\n");
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