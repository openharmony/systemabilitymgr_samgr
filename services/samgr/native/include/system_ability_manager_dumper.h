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
#include "system_ability_manager.h"
#include "if_local_ability_manager.h"

namespace OHOS {
enum {
    IPC_STAT_PREFIX_INDEX = 0,
    IPC_STAT_PROCESS_INDEX = 1,
    IPC_STAT_CMD_INDEX = 2
};

enum {
    FFRT_STAT_PREFIX_INDEX = 0,
    FFRT_STAT_PROCESS_INDEX = 1,
    FFRT_STAT_CMD_INDEX = 2
};
constexpr int32_t IPC_STAT_CMD_LEN = 3;

class SystemAbilityManagerDumper {
public:
    static bool Dump(std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler,
        const std::vector<std::string>& args, std::string& result);
    static bool IpcDumpCmdParser(int32_t& cmd, const std::vector<std::string>& args);
    static bool IpcDumpIsAllProcess(const std::string& processName);
    static bool IpcDumpIsSamgr(const std::string& processName);
    static bool StartSamgrIpcStatistics(std::string& result);
    static bool StopSamgrIpcStatistics(std::string& result);
    static bool GetSamgrIpcStatistics(std::string& result);
    static bool FfrtDumpParser(std::vector<int32_t>& processIds, const std::string& processIdsStr);
    static int32_t FfrtDumpProc(std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler,
        int32_t fd, const std::vector<std::string>& args);
    static void GetFfrtLoadMetrics(std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler,
        int32_t fd, const std::vector<std::string>& args, std::string& result);
    static bool FfrtStatCmdParser(int32_t& cmd, const std::vector<std::string>& args);
    static void CollectFfrtMetricInfoInProcs(int32_t fd, const std::vector<int32_t>& processIds,
        std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler, int32_t cmd, std::string& result);
    static sptr<ILocalAbilityManager> GetProcByProcessId(
        std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler, int32_t processId);
    static bool GetFfrtDumpInfoProc(std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler,
        const std::vector<std::string>& args, std::string& result);
    static int32_t ListenerDumpProc(std::map<int32_t, std::list<SAListener>>& listeners,
        int32_t fd, const std::vector<std::string>& args);

private:
    SystemAbilityManagerDumper() = default;
    ~SystemAbilityManagerDumper() = default;
    static bool CollectFfrtStatistics(int32_t cmd, std::string& result);
    static bool StartFfrtStatistics(std::string& result);
    static bool StopFfrtStatistics(std::string& result);
    static bool GetFfrtStatistics(std::string& result);
    static void FfrtStatisticsParser(std::string& result);
    static void ClearFfrtStatisticsBufferLocked();
    static void ClearFfrtStatistics();
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
    static void DumpFfrtInfoInProc(
        std::shared_ptr<SystemAbilityStateScheduler> abilityStateScheduler, int32_t pid, std::string& result);
    static int32_t SaveDumpResultToFd(int32_t fd, const std::string& result);
    static void GetSAMgrFfrtInfo(std::string& result);
    static void GetListenerDumpProc(std::map<int32_t, std::list<SAListener>>& listeners,
        const std::vector<std::string>& args, std::string& result);
    static void ShowAllBySA(std::map<int32_t, std::list<SAListener>>& listeners, std::string& result);
    static void ShowAllByCallingPid(std::map<int32_t, std::list<SAListener>>& listeners,
        std::string& result);
    static void ShowCallingPidBySA(std::map<int32_t, std::list<SAListener>>& listeners,
        int32_t said, std::string& result);
    static void ShowSAByCallingPid(std::map<int32_t, std::list<SAListener>>& listeners,
        int32_t pid, std::string& result);
    static void ShowListenerHelp(std::string& result);
    static std::shared_ptr<FFRTHandler> handler_;
};
} // namespace OHOS
#endif // SERVICES_SAMGR_NATIVE_INCLUDE_SYSTEM_ABILITY_MANAGER_DUMPER_H