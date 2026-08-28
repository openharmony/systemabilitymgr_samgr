/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "ondemand_command.h"

#ifdef SUPPORT_MULTI_INSTANCE
#include <array>
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <ctime>
#include <fcntl.h>
#include <iomanip>
#include <iostream>
#include <limits>
#include <list>
#include <sstream>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>
#include <vector>

#include "errors.h"
#include "ondemand_flow.h"
#include "string_ex.h"
#include "system_ability_status_change_stub.h"
#include "system_process_status_change_stub.h"

namespace OHOS {
enum {
    HM_SET_USERID = 12,
    HM_GET_USERID = 13,
    HM_GET_USERINFO = 14,
};
#define ACCESS_TOKENID_DEV "/dev/access_token_id"
#define ACCESS_TOKENID_IOCTL_BASE 'A'
#define ACCESS_TOKENID_SET_USERID _IOW(ACCESS_TOKENID_IOCTL_BASE, HM_SET_USERID, uint32_t)
#define ACCESS_TOKENID_GET_USERID _IOR(ACCESS_TOKENID_IOCTL_BASE, HM_GET_USERID, uint32_t)
namespace {
constexpr size_t TOP_COMMAND_INDEX = 0;
constexpr size_t SA_COMMAND_INDEX = 1;
constexpr size_t SA_ID_INDEX = 2;
constexpr size_t FIRST_OPTION_INDEX = 3;
constexpr size_t SECOND_OPTION_INDEX = 4;
constexpr size_t THIRD_OPTION_INDEX = 5;
constexpr size_t TOP_LEVEL_FIRST_OPTION_INDEX = 1;
constexpr size_t TOP_LEVEL_SECOND_OPTION_INDEX = 2;
constexpr size_t EVENT_NAME_OFFSET = 1;
constexpr size_t EVENT_VALUE_OFFSET = 2;
constexpr size_t EVENT_PERSISTENCE_OFFSET = 3;
constexpr size_t POLICY_EVENT_OFFSET = 1;
constexpr size_t SET_USER_ID_PREFIX_SIZE = 2;
constexpr size_t TIMED_POLICY_TARGET_COUNT = 3;
constexpr size_t FIRST_TARGET_INDEX = 0;
constexpr size_t SECOND_TARGET_INDEX = 1;
constexpr size_t THIRD_TARGET_INDEX = 2;
constexpr int32_t DEFAULT_LOAD_TIMEOUT_SECONDS = 4;
constexpr int32_t DEFAULT_STRATEGY_TYPE = 0;
constexpr int32_t DEFAULT_STRATEGY_LEVEL = 0;
constexpr int32_t FIRST_TIMED_OFFSET_SECONDS = 60;
constexpr int32_t SECOND_TIMED_OFFSET_SECONDS = 120;
constexpr int32_t THIRD_TIMED_OFFSET_SECONDS = 180;
constexpr char DEFAULT_STRATEGY_ACTION[] = "mock";
constexpr int32_t MAX_WAIT_MILLISECONDS = 10 * 60 * 1000;
constexpr useconds_t MICROSECONDS_PER_MILLISECOND = 1000;
constexpr char GET_COMMAND_ALIAS[] = "1";
constexpr char LOAD_COMMAND_ALIAS[] = "2";
constexpr char UNLOAD_COMMAND_ALIAS[] = "3";
constexpr char GET_INFO_COMMAND_ALIAS[] = "4";
constexpr char SYNC_LOAD_COMMAND_ALIAS[] = "5";
constexpr char GET_BY_USER_COMMAND_ALIAS[] = "6";
constexpr char CHECK_EXIST_COMMAND_ALIAS[] = "7";
constexpr char CHECK_BY_USER_COMMAND_ALIAS[] = "8";
constexpr char CHECK_EXIST_BY_USER_COMMAND_ALIAS[] = "9";
constexpr char REMOVE_COMMAND_ALIAS[] = "10";
constexpr char CANCEL_UNLOAD_COMMAND_ALIAS[] = "11";
constexpr char ON_DEMAND_INFO_COMMAND_ALIAS[] = "12";
constexpr char UNLOAD_ALL_IDLE_COMMAND_ALIAS[] = "13";
constexpr char UNLOAD_PROCESS_COMMAND_ALIAS[] = "14";
constexpr char GET_LRU_COMMAND_ALIAS[] = "15";
constexpr char GET_RUNNING_PROCESS_COMMAND_ALIAS[] = "16";
constexpr char SEND_STRATEGY_COMMAND_ALIAS[] = "17";
constexpr char GET_EXTENSION_COMMAND_ALIAS[] = "18";
constexpr char GET_EXTENSION_INFO_COMMAND_ALIAS[] = "19";
constexpr char GET_POLICY_COMMAND_ALIAS[] = "20";
constexpr char COMMON_EVENT_COMMAND_ALIAS[] = "21";
constexpr char SUBSCRIBE_COMMAND_ALIAS[] = "22";
constexpr char UNSUBSCRIBE_COMMAND_ALIAS[] = "23";
constexpr char SUBSCRIBE_PROCESS_COMMAND_ALIAS[] = "24";
constexpr char UNSUBSCRIBE_PROCESS_COMMAND_ALIAS[] = "25";
constexpr char SUBSCRIBE_BY_USER_COMMAND_ALIAS[] = "26";
constexpr char UNSUBSCRIBE_BY_USER_COMMAND_ALIAS[] = "27";
constexpr char SUBSCRIBE_PROCESS_BY_USER_COMMAND_ALIAS[] = "28";
constexpr char UNSUBSCRIBE_PROCESS_BY_USER_COMMAND_ALIAS[] = "29";
constexpr char GET_INFO_BY_USER_COMMAND_ALIAS[] = "30";
constexpr char GET_PROXY_COMMAND_ALIAS[] = "31";
constexpr char GET_PROXY_BY_USER_COMMAND_ALIAS[] = "32";
constexpr char SYNC_LOAD_BY_USER_COMMAND_ALIAS[] = "33";
constexpr char LOAD_BY_USER_COMMAND_ALIAS[] = "34";
constexpr char LOAD_TIMEOUT_COMMAND_ALIAS[] = "35";
constexpr char LOAD_TIMEOUT_BY_USER_COMMAND_ALIAS[] = "36";
constexpr char TRIGGER_UNLOAD_COMMAND_ALIAS[] = "37";
constexpr char TRIGGER_UNLOAD_BY_USER_COMMAND_ALIAS[] = "38";
constexpr char TRIGGER_UNLOAD_CANCEL_COMMAND_ALIAS[] = "39";
constexpr char TRIGGER_UNLOAD_CANCEL_BY_USER_COMMAND_ALIAS[] = "40";
constexpr char SUBSCRIPTION_REJECT_BY_USER_COMMAND_ALIAS[] = "45";
constexpr char POLICY_UPDATE_BY_USER_COMMAND_ALIAS[] = "48";
constexpr char POLICY_GET_BY_USER_COMMAND_ALIAS[] = "49";
constexpr char POLICY_UPDATE_COMMAND_ALIAS[] = "50";
constexpr char POLICY_GET_COMMAND_ALIAS[] = "51";
constexpr char REMOVE_FLOW_COMMAND_ALIAS[] = "52";
constexpr char REMOVE_FLOW_BY_USER_COMMAND_ALIAS[] = "53";

bool CommandMatches(const std::string& actual, const char* name, const char* numericAlias)
{
    return actual == name || (numericAlias != nullptr && actual == numericAlias);
}

bool ParseInt32(const std::string& input, const char* name, bool allowZero, int32_t& value)
{
    errno = 0;
    char* end = nullptr;
    long parsed = std::strtol(input.c_str(), &end, 10);
    bool invalid = errno != 0 || end == input.c_str() || *end != '\0' || parsed > std::numeric_limits<int32_t>::max() ||
                   parsed < 0 || (!allowZero && parsed == 0);
    if (invalid) {
        std::cout << "invalid " << name << ":" << input << std::endl;
        return false;
    }
    value = static_cast<int32_t>(parsed);
    return true;
}

bool ParseBoolean(const std::string& input, bool& value)
{
    if (input == "true" || input == "1") {
        value = true;
        return true;
    }
    if (input == "false" || input == "0") {
        value = false;
        return true;
    }
    std::cout << "invalid boolean:" << input << std::endl;
    return false;
}

bool SetCallingUserId(const std::string& input)
{
    int32_t parsedUserId = ONDEMAND_BASE_USER_ID;
    if (!ParseInt32(input, "userId", true, parsedUserId)) {
        return false;
    }
    uint32_t userId = static_cast<uint32_t>(parsedUserId);
    int fileDescriptor = open(ACCESS_TOKENID_DEV, O_RDWR | O_CLOEXEC);
    if (fileDescriptor < 0) {
        std::cout << "set user id open failed" << std::endl;
        return false;
    }
    int32_t result = ioctl(fileDescriptor, ACCESS_TOKENID_SET_USERID, &userId);
    uint32_t actualUserId = 0;
    if (result == 0) {
        result = ioctl(fileDescriptor, ACCESS_TOKENID_GET_USERID, &actualUserId);
    }
    close(fileDescriptor);
    if (result != 0 || actualUserId != userId) {
        std::cout << "set access token user id failed" << std::endl;
        return false;
    }
    std::cout << "set user id = " << actualUserId << std::endl;
    return true;
}

std::string FormatFutureTime(int32_t offsetSeconds)
{
    std::time_t target = std::time(nullptr) + offsetSeconds;
    std::tm localTime{};
    localtime_r(&target, &localTime);
    std::ostringstream value;
    value << std::put_time(&localTime, "%Y-%m-%d-%H:%M:%S");
    return value.str();
}

class DirectAbilityListener final : public SystemAbilityStatusChangeStub {
public:
    void OnAddSystemAbility(int32_t saId, const std::string& deviceId) override
    {
        (void)saId;
        (void)deviceId;
    }

    void OnRemoveSystemAbility(int32_t saId, const std::string& deviceId) override
    {
        (void)saId;
        (void)deviceId;
    }
};

class DirectProcessListener final : public SystemProcessStatusChangeStub {
public:
    void OnSystemProcessStarted(SystemProcessInfo& processInfo) override
    {
        (void)processInfo;
    }

    void OnSystemProcessStopped(SystemProcessInfo& processInfo) override
    {
        (void)processInfo;
    }
};

class CommandDispatcher final {
public:
    explicit CommandDispatcher(OnDemandHelper& helper) : helper_(helper)
    {
    }
    ~CommandDispatcher() = default;

    CommandDispatcher(const CommandDispatcher&) = delete;
    CommandDispatcher& operator=(const CommandDispatcher&) = delete;
    CommandDispatcher(CommandDispatcher&&) = delete;
    CommandDispatcher& operator=(CommandDispatcher&&) = delete;

    bool Run(int argc, char* argv[], int32_t& exitCode)
    {
        arguments_.clear();
        for (int32_t index = 1; index < argc; ++index) {
            arguments_.emplace_back(argv[index]);
        }
        if (arguments_.empty()) {
            return false;
        }
        if (arguments_.front() == "setuserid") {
            userPrefixApplied_ = true;
            if (arguments_.size() < SET_USER_ID_PREFIX_SIZE ||
                !SetCallingUserId(arguments_[TOP_LEVEL_FIRST_OPTION_INDEX])) {
                exitCode = EXIT_FAILURE;
                return true;
            }
            arguments_.erase(arguments_.begin(), arguments_.begin() + SET_USER_ID_PREFIX_SIZE);
            if (arguments_.empty()) {
                exitCode = EXIT_SUCCESS;
                return true;
            }
        }
        bool handled = DispatchTopLevel();
        if (!handled && userPrefixApplied_) {
            std::cout << "unsupported command after setuserid" << std::endl;
            handled = true;
        }
        exitCode = EXIT_SUCCESS;
        return handled;
    }

private:
    const std::string* GetArgument(size_t index) const
    {
        return index < arguments_.size() ? &arguments_[index] : nullptr;
    }

    bool ParseArgument(size_t index, const char* name, bool allowZero, int32_t& value) const
    {
        const std::string* argument = GetArgument(index);
        if (argument == nullptr) {
            std::cout << "missing " << name << std::endl;
            return false;
        }
        return ParseInt32(*argument, name, allowZero, value);
    }

    bool ParsePolicyType(size_t index, OnDemandPolicyType& policyType) const
    {
        const std::string* argument = GetArgument(index);
        if (argument != nullptr && *argument == "start") {
            policyType = OnDemandPolicyType::START_POLICY;
            return true;
        }
        if (argument != nullptr && *argument == "stop") {
            policyType = OnDemandPolicyType::STOP_POLICY;
            return true;
        }
        std::cout << "invalid policy type, expected start or stop" << std::endl;
        return false;
    }

    bool ParsePolicyPersistence(size_t index, bool defaultValue, bool& persistence) const
    {
        const std::string* argument = GetArgument(index);
        if (argument == nullptr) {
            persistence = defaultValue;
            return true;
        }
        return ParseBoolean(*argument, persistence);
    }

    bool ParseStructuredEvent(size_t index, PolicyEventSpec& event) const
    {
        const std::string* kind = GetArgument(index);
        const std::string* name = GetArgument(index + EVENT_NAME_OFFSET);
        if (kind == nullptr || name == nullptr) {
            std::cout << "missing policy event type or name" << std::endl;
            return false;
        }
        bool timedEvent = *kind == "timed" || *kind == "timed_event" || *kind == "TIMED_EVENT";
        bool paramEvent = *kind == "param" || *kind == "PARAM";
        bool commonEvent = *kind == "common" || *kind == "common_event" || *kind == "COMMON_EVENT";
        if (!timedEvent && !paramEvent && !commonEvent) {
            return false;
        }
        event.name = *name;
        event.eventId = timedEvent ? OnDemandEventId::TIMED_EVENT
                                   : (paramEvent ? OnDemandEventId::PARAM : OnDemandEventId::COMMON_EVENT);
        const std::string* value = GetArgument(index + EVENT_VALUE_OFFSET);
        if ((timedEvent || paramEvent) && value == nullptr) {
            std::cout << "missing policy event value" << std::endl;
            return false;
        }
        event.value = value == nullptr ? "1" : *value;
        return ParsePolicyPersistence(index + EVENT_PERSISTENCE_OFFSET, timedEvent, event.persistence);
    }

    bool ParsePolicyEvent(size_t index, PolicyEventSpec& event) const
    {
        const std::string* argument = GetArgument(index);
        if (argument == nullptr) {
            std::cout << "missing policy event" << std::endl;
            return false;
        }
        bool structured = *argument == "timed" || *argument == "timed_event" || *argument == "TIMED_EVENT" ||
                          *argument == "param" || *argument == "PARAM" || *argument == "common" ||
                          *argument == "common_event" || *argument == "COMMON_EVENT";
        if (structured) {
            return ParseStructuredEvent(index, event);
        }
        event.eventId = OnDemandEventId::COMMON_EVENT;
        event.name = *argument;
        event.value = "1";
        event.persistence = false;
        return true;
    }

    bool ParseScope(size_t index, bool& subscribeProcess, std::string& processName) const
    {
        const std::string* scope = GetArgument(index);
        if (scope == nullptr || (*scope != "sa" && *scope != "both")) {
            std::cout << "invalid subscription scope, expected sa or both" << std::endl;
            return false;
        }
        subscribeProcess = *scope == "both";
        const std::string* process = GetArgument(index + 1);
        if (subscribeProcess && process == nullptr) {
            std::cout << "missing subscription processName" << std::endl;
            return false;
        }
        processName = process == nullptr ? "" : *process;
        return true;
    }

    bool DispatchTopLevel()
    {
        const std::string& command = arguments_[TOP_COMMAND_INDEX];
        if (command == "sa") {
            return DispatchSaCommand();
        }
        if (command == "userstate") {
            return HandleUserState();
        }
        if (command == "wait") {
            return HandleWait();
        }
        if (command == "check") {
            return HandleCheck();
        }
        return false;
    }

    bool HandleUserState()
    {
        int32_t userId = ONDEMAND_BASE_USER_ID;
        int32_t stateValue = 0;
        if (!ParseArgument(TOP_LEVEL_FIRST_OPTION_INDEX, "userId", false, userId) ||
            !ParseArgument(TOP_LEVEL_SECOND_OPTION_INDEX, "userState", true, stateValue)) {
            return true;
        }
        SamgrUserState state = static_cast<SamgrUserState>(stateValue);
        bool valid = state == SamgrUserState::USER_STATE_ACTIVATING || state == SamgrUserState::USER_STATE_SWITCHING ||
                     state == SamgrUserState::USER_STATE_STOPPING;
        if (!valid) {
            std::cout << "invalid userState:" << stateValue << std::endl;
            return true;
        }
        (void)helper_.OnUserStateChanged(userId, state);
        return true;
    }

    bool HandleWait()
    {
        int32_t waitMilliseconds = 0;
        if (!ParseArgument(1, "waitMilliseconds", false, waitMilliseconds)) {
            return true;
        }
        if (waitMilliseconds > MAX_WAIT_MILLISECONDS) {
            std::cout << "waitMilliseconds exceeds limit" << std::endl;
            return true;
        }
        usleep(static_cast<useconds_t>(waitMilliseconds) * MICROSECONDS_PER_MILLISECOND);
        return true;
    }

    bool HandleCheck()
    {
        const std::string* scope = GetArgument(TOP_LEVEL_FIRST_OPTION_INDEX);
        int32_t saId = 0;
        if (scope == nullptr || *scope != "local" ||
            !ParseArgument(TOP_LEVEL_SECOND_OPTION_INDEX, "saId", false, saId)) {
            return false;
        }
        (void)helper_.CheckSystemAbility(saId);
        return true;
    }

    bool DispatchSaCommand()
    {
        const std::string* command = GetArgument(SA_COMMAND_INDEX);
        int32_t saId = 0;
        if (command == nullptr || !ParseArgument(SA_ID_INDEX, "saId", true, saId)) {
            return userPrefixApplied_;
        }
        return HandleBaseQuery(*command, saId) || HandleUserQuery(*command, saId) || HandleBaseLoad(*command, saId) ||
               HandleUserLoad(*command, saId) || HandleAbilityChange(*command, saId) || HandleTrigger(*command, saId) ||
               HandleResource(*command, saId) || HandlePolicy(*command, saId) ||
               HandleDirectSubscription(*command, saId) || HandleSubscriptionFlow(*command, saId) ||
               HandleApiFlow(*command, saId) || HandleConcurrentLoadFlow(*command, saId) ||
               HandleTimedPolicyFlow(*command, saId) || HandleGlobalFlow(*command, saId) ||
               HandleRemoveFlow(*command, saId);
    }

    bool HandleBaseQuery(const std::string& command, int32_t saId)
    {
        if (CommandMatches(command, "get", GET_COMMAND_ALIAS)) {
            (void)helper_.GetSystemAbility(saId);
        } else if (CommandMatches(command, "checkexist", CHECK_EXIST_COMMAND_ALIAS)) {
            bool isExist = false;
            (void)helper_.CheckSystemAbility(saId, isExist);
        } else if (CommandMatches(command, "getinfo", GET_INFO_COMMAND_ALIAS)) {
            SystemProcessInfo processInfo;
            (void)helper_.GetSystemProcessInfo(saId, processInfo);
        } else if (CommandMatches(command, "getproxy", GET_PROXY_COMMAND_ALIAS)) {
            (void)helper_.GetLocalAbilityManagerProxy(saId);
        } else {
            return false;
        }
        return true;
    }

    bool HandleUserQuery(const std::string& command, int32_t saId)
    {
        bool userCommand = CommandMatches(command, "getbyuser", GET_BY_USER_COMMAND_ALIAS) ||
                           CommandMatches(command, "checkbyuser", CHECK_BY_USER_COMMAND_ALIAS) ||
                           CommandMatches(command, "checkexistbyuser", CHECK_EXIST_BY_USER_COMMAND_ALIAS) ||
                           CommandMatches(command, "getinfobyuser", GET_INFO_BY_USER_COMMAND_ALIAS) ||
                           CommandMatches(command, "getproxybyuser", GET_PROXY_BY_USER_COMMAND_ALIAS);
        if (!userCommand) {
            return false;
        }
        OnDemandTarget target{saId, ONDEMAND_BASE_USER_ID, true};
        if (!ParseArgument(FIRST_OPTION_INDEX, "userId", true, target.userId)) {
            return true;
        }
        if (CommandMatches(command, "getbyuser", GET_BY_USER_COMMAND_ALIAS)) {
            (void)helper_.GetSystemAbility(saId, target.userId);
        } else if (CommandMatches(command, "checkbyuser", CHECK_BY_USER_COMMAND_ALIAS)) {
            (void)helper_.CheckSystemAbility(saId, target.userId);
        } else if (CommandMatches(command, "checkexistbyuser", CHECK_EXIST_BY_USER_COMMAND_ALIAS)) {
            bool isExist = false;
            (void)helper_.CheckSystemAbilityByUserId(saId, isExist, target.userId);
        } else if (CommandMatches(command, "getinfobyuser", GET_INFO_BY_USER_COMMAND_ALIAS)) {
            SystemProcessInfo processInfo;
            (void)helper_.GetSystemProcessInfo(saId, processInfo, target.userId);
        } else if (CommandMatches(command, "getproxybyuser", GET_PROXY_BY_USER_COMMAND_ALIAS)) {
            (void)helper_.GetLocalAbilityManagerProxy(saId, target.userId);
        } else {
            return false;
        }
        return true;
    }

    bool HandleBaseLoad(const std::string& command, int32_t saId)
    {
        if (CommandMatches(command, "load", LOAD_COMMAND_ALIAS)) {
            (void)helper_.LoadSystemAbilityByCallback(saId);
            return true;
        }
        int32_t timeoutSeconds = DEFAULT_LOAD_TIMEOUT_SECONDS;
        bool syncLoad = CommandMatches(command, "syncload", SYNC_LOAD_COMMAND_ALIAS);
        if (!syncLoad && CommandMatches(command, "loadtimeout", LOAD_TIMEOUT_COMMAND_ALIAS)) {
            if (!ParseArgument(FIRST_OPTION_INDEX, "timeoutSeconds", false, timeoutSeconds)) {
                return true;
            }
        } else if (!syncLoad) {
            return false;
        }
        (void)helper_.LoadSystemAbility(saId, timeoutSeconds);
        return true;
    }

    bool HandleUserLoad(const std::string& command, int32_t saId)
    {
        OnDemandTarget target{saId, ONDEMAND_BASE_USER_ID, true};
        int32_t timeoutSeconds = DEFAULT_LOAD_TIMEOUT_SECONDS;
        bool timeoutLoad = false;
        if (CommandMatches(command, "loadtimeoutbyuser", LOAD_TIMEOUT_BY_USER_COMMAND_ALIAS)) {
            timeoutLoad = true;
            if (!ParseArgument(FIRST_OPTION_INDEX, "timeoutSeconds", false, timeoutSeconds) ||
                !ParseArgument(SECOND_OPTION_INDEX, "userId", true, target.userId)) {
                return true;
            }
        } else if (CommandMatches(command, "syncloadbyuser", SYNC_LOAD_BY_USER_COMMAND_ALIAS)) {
            timeoutLoad = true;
            if (!ParseArgument(FIRST_OPTION_INDEX, "userId", true, target.userId)) {
                return true;
            }
        } else if (CommandMatches(command, "loadbyuser", LOAD_BY_USER_COMMAND_ALIAS)) {
            if (!ParseArgument(FIRST_OPTION_INDEX, "userId", true, target.userId)) {
                return true;
            }
        } else {
            return false;
        }
        if (timeoutLoad) {
            (void)helper_.LoadSystemAbility(saId, timeoutSeconds, target.userId);
        } else {
            (void)helper_.LoadSystemAbilityByCallback(saId, target.userId);
        }
        return true;
    }

    bool HandleAbilityChange(const std::string& command, int32_t saId)
    {
        if (CommandMatches(command, "unload", UNLOAD_COMMAND_ALIAS)) {
            (void)helper_.UnloadSystemAbility(saId);
        } else if (CommandMatches(command, "remove", REMOVE_COMMAND_ALIAS)) {
            (void)helper_.RemoveSystemAbility(saId);
        } else if (CommandMatches(command, "cancelUnload", CANCEL_UNLOAD_COMMAND_ALIAS)) {
            (void)helper_.CancelUnloadSystemAbility(saId);
        } else if (CommandMatches(command, "ondemandinfo", ON_DEMAND_INFO_COMMAND_ALIAS)) {
            const std::string* processName = GetArgument(FIRST_OPTION_INDEX);
            if (processName == nullptr) {
                std::cout << "missing processName" << std::endl;
            } else {
                (void)helper_.AddOnDemandSystemAbilityInfo(saId, *processName);
            }
        } else {
            return false;
        }
        return true;
    }

    bool HandleTrigger(const std::string& command, int32_t saId)
    {
        OnDemandTarget target{saId, ONDEMAND_BASE_USER_ID, false};
        bool cancelUnload = false;
        bool triggerUnload = CommandMatches(command, "triggerunload", TRIGGER_UNLOAD_COMMAND_ALIAS);
        if (CommandMatches(command, "triggerunloadcancel", TRIGGER_UNLOAD_CANCEL_COMMAND_ALIAS)) {
            cancelUnload = true;
        } else if (CommandMatches(command, "triggerunloadbyuser", TRIGGER_UNLOAD_BY_USER_COMMAND_ALIAS) ||
                   CommandMatches(command, "triggerunloadcancelbyuser", TRIGGER_UNLOAD_CANCEL_BY_USER_COMMAND_ALIAS)) {
            target.useUserIdApi = true;
            cancelUnload =
                CommandMatches(command, "triggerunloadcancelbyuser", TRIGGER_UNLOAD_CANCEL_BY_USER_COMMAND_ALIAS);
            if (!ParseArgument(FIRST_OPTION_INDEX, "userId", true, target.userId)) {
                return true;
            }
        } else if (!triggerUnload) {
            return false;
        }
        (void)helper_.TriggerUnloadSystemAbility(target, cancelUnload);
        return true;
    }

    bool HandleResource(const std::string& command, int32_t saId)
    {
        const std::string* option = GetArgument(FIRST_OPTION_INDEX);
        if (CommandMatches(command, "unloadallidle", UNLOAD_ALL_IDLE_COMMAND_ALIAS)) {
            (void)helper_.UnloadAllIdleSystemAbility();
        } else if (CommandMatches(command, "unloadprocess", UNLOAD_PROCESS_COMMAND_ALIAS)) {
            if (option == nullptr) {
                std::cout << "missing processName" << std::endl;
            } else {
                std::vector<std::u16string> processList{Str8ToStr16(*option)};
                (void)helper_.UnloadProcess(processList);
            }
        } else if (CommandMatches(command, "getlru", GET_LRU_COMMAND_ALIAS)) {
            (void)helper_.GetLruIdleSystemAbilityProc();
        } else if (CommandMatches(command, "getrunningprocess", GET_RUNNING_PROCESS_COMMAND_ALIAS)) {
            std::list<SystemProcessInfo> processInfos;
            (void)helper_.GetRunningSystemProcess(processInfos);
        } else if (CommandMatches(command, "sendstrategy", SEND_STRATEGY_COMMAND_ALIAS)) {
            std::vector<int32_t> saIds{saId};
            (void)helper_.SendStrategy(DEFAULT_STRATEGY_TYPE, saIds, DEFAULT_STRATEGY_LEVEL, DEFAULT_STRATEGY_ACTION);
        } else {
            return HandleExtensionOrEvent(command, saId, option);
        }
        return true;
    }

    bool HandleExtensionOrEvent(const std::string& command, int32_t saId, const std::string* option)
    {
        if (CommandMatches(command, "getextension", GET_EXTENSION_COMMAND_ALIAS)) {
            if (option != nullptr) {
                std::vector<sptr<IRemoteObject>> saList;
                (void)helper_.GetExtensionRunningSaList(*option, saList);
            }
        } else if (CommandMatches(command, "getextensioninfo", GET_EXTENSION_INFO_COMMAND_ALIAS)) {
            if (option != nullptr) {
                std::vector<ISystemAbilityManager::SaExtensionInfo> infoList;
                (void)helper_.GetRunningSaExtensionInfoList(*option, infoList);
            }
        } else if (CommandMatches(command, "getpolicy", GET_POLICY_COMMAND_ALIAS)) {
            OnDemandPolicyType policyType = OnDemandPolicyType::START_POLICY;
            if (ParsePolicyType(FIRST_OPTION_INDEX, policyType)) {
                helper_.GetOnDemandPolicy(saId, policyType);
            }
        } else if (CommandMatches(command, "commonevent", COMMON_EVENT_COMMAND_ALIAS)) {
            std::string eventName = option == nullptr || *option == "all" ? "" : *option;
            helper_.GetCommonEventExtraId(saId, eventName);
        } else {
            return false;
        }
        return true;
    }

    bool HandlePolicy(const std::string& command, int32_t saId)
    {
        bool update = CommandMatches(command, "policyupdate", POLICY_UPDATE_COMMAND_ALIAS) ||
                      CommandMatches(command, "policyupdatebyuser", POLICY_UPDATE_BY_USER_COMMAND_ALIAS);
        bool query = CommandMatches(command, "policyget", POLICY_GET_COMMAND_ALIAS) ||
                     CommandMatches(command, "policygetbyuser", POLICY_GET_BY_USER_COMMAND_ALIAS);
        if (!update && !query) {
            return false;
        }
        bool byUser = CommandMatches(command, "policyupdatebyuser", POLICY_UPDATE_BY_USER_COMMAND_ALIAS) ||
                      CommandMatches(command, "policygetbyuser", POLICY_GET_BY_USER_COMMAND_ALIAS);
        size_t policyIndex = byUser ? SECOND_OPTION_INDEX : FIRST_OPTION_INDEX;
        int32_t userId = ONDEMAND_BASE_USER_ID;
        if (byUser && !ParseArgument(FIRST_OPTION_INDEX, "userId", false, userId)) {
            return true;
        }
        OnDemandPolicyType policyType = OnDemandPolicyType::START_POLICY;
        if (!ParsePolicyType(policyIndex, policyType)) {
            return true;
        }
        return update ? RunPolicyUpdate(saId, userId, byUser, policyType, policyIndex + POLICY_EVENT_OFFSET)
                      : RunPolicyQuery(saId, userId, byUser, policyType);
    }

    bool RunPolicyUpdate(int32_t saId, int32_t userId, bool byUser, OnDemandPolicyType policyType, size_t eventIndex)
    {
        PolicyUpdateRequest request;
        request.target = {saId, userId, byUser};
        request.policyType = policyType;
        if (ParsePolicyEvent(eventIndex, request.event)) {
            (void)helper_.UpdateOnDemandPolicyBySa(request);
        }
        return true;
    }

    bool RunPolicyQuery(int32_t saId, int32_t userId, bool byUser, OnDemandPolicyType policyType)
    {
        PolicyQueryRequest request;
        request.target = {saId, userId, byUser};
        request.policyType = policyType;
        (void)helper_.GetOnDemandPolicyBySa(request);
        return true;
    }

    bool HandleDirectSubscription(const std::string& command, int32_t saId)
    {
        OnDemandTarget target{saId, ONDEMAND_BASE_USER_ID, false};
        bool abilityCommand = CommandMatches(command, "subscribe", SUBSCRIBE_COMMAND_ALIAS) ||
                              CommandMatches(command, "unsubscribe", UNSUBSCRIBE_COMMAND_ALIAS) ||
                              CommandMatches(command, "subscribebyuser", SUBSCRIBE_BY_USER_COMMAND_ALIAS) ||
                              CommandMatches(command, "unsubscribebyuser", UNSUBSCRIBE_BY_USER_COMMAND_ALIAS);
        bool processCommand =
            CommandMatches(command, "subscribeproc", SUBSCRIBE_PROCESS_COMMAND_ALIAS) ||
            CommandMatches(command, "unsubscribeproc", UNSUBSCRIBE_PROCESS_COMMAND_ALIAS) ||
            CommandMatches(command, "subscribeprocbyuser", SUBSCRIBE_PROCESS_BY_USER_COMMAND_ALIAS) ||
            CommandMatches(command, "unsubscribeprocbyuser", UNSUBSCRIBE_PROCESS_BY_USER_COMMAND_ALIAS);
        if (!abilityCommand && !processCommand) {
            return false;
        }
        bool byUser = CommandMatches(command, "subscribebyuser", SUBSCRIBE_BY_USER_COMMAND_ALIAS) ||
                      CommandMatches(command, "unsubscribebyuser", UNSUBSCRIBE_BY_USER_COMMAND_ALIAS) ||
                      CommandMatches(command, "subscribeprocbyuser", SUBSCRIBE_PROCESS_BY_USER_COMMAND_ALIAS) ||
                      CommandMatches(command, "unsubscribeprocbyuser", UNSUBSCRIBE_PROCESS_BY_USER_COMMAND_ALIAS);
        target.useUserIdApi = byUser;
        if (byUser && !ParseArgument(FIRST_OPTION_INDEX, "userId", false, target.userId)) {
            return true;
        }
        bool unsubscribe = CommandMatches(command, "unsubscribe", UNSUBSCRIBE_COMMAND_ALIAS) ||
                           CommandMatches(command, "unsubscribebyuser", UNSUBSCRIBE_BY_USER_COMMAND_ALIAS) ||
                           CommandMatches(command, "unsubscribeproc", UNSUBSCRIBE_PROCESS_COMMAND_ALIAS) ||
                           CommandMatches(command, "unsubscribeprocbyuser", UNSUBSCRIBE_PROCESS_BY_USER_COMMAND_ALIAS);
        if (abilityCommand) {
            if (target.useUserIdApi) {
                (void)(unsubscribe ? helper_.UnSubscribeSystemAbility(saId, abilityListener_, target.userId)
                                   : helper_.SubscribeSystemAbility(saId, abilityListener_, target.userId));
            } else {
                (void)(unsubscribe ? helper_.UnSubscribeSystemAbility(saId, abilityListener_)
                                   : helper_.SubscribeSystemAbility(saId, abilityListener_));
            }
        } else {
            if (target.useUserIdApi) {
                (void)(unsubscribe ? helper_.UnSubscribeSystemProcess(processListener_, target.userId)
                                   : helper_.SubscribeSystemProcess(processListener_, target.userId));
            } else {
                (void)(unsubscribe ? helper_.UnSubscribeSystemProcess(processListener_)
                                   : helper_.SubscribeSystemProcess(processListener_));
            }
        }
        return true;
    }

    bool HandleSubscriptionFlow(const std::string& command, int32_t saId)
    {
        bool callbackFlow = command == "callbackFlow";
        bool timeoutFlow = command == "timeoutFlow";
        bool baseFlow = callbackFlow || timeoutFlow;
        bool userFlow = CommandMatches(command, "subscriptionrejectbyuser", SUBSCRIPTION_REJECT_BY_USER_COMMAND_ALIAS);
        if (!baseFlow && !userFlow) {
            return false;
        }
        SubscriptionFlowOptions options;
        options.target.saId = saId;
        size_t scopeIndex = FIRST_OPTION_INDEX;
        const std::string* firstOption = GetArgument(FIRST_OPTION_INDEX);
        bool optionalUser =
            (callbackFlow || timeoutFlow) && firstOption != nullptr && *firstOption != "sa" && *firstOption != "both";
        userFlow = userFlow || optionalUser;
        if (userFlow) {
            options.target.useUserIdApi = true;
            if (!ParseArgument(FIRST_OPTION_INDEX, "userId", false, options.target.userId)) {
                return true;
            }
            scopeIndex = SECOND_OPTION_INDEX;
        }
        if (!ParseScope(scopeIndex, options.subscribeProcess, options.processName)) {
            return true;
        }
        options.timeoutLoad = timeoutFlow;
        options.expectRejected =
            CommandMatches(command, "subscriptionrejectbyuser", SUBSCRIPTION_REJECT_BY_USER_COMMAND_ALIAS);
        (void)RunSubscriptionFlow(helper_, options);
        return true;
    }

    bool HandleGlobalFlow(const std::string& command, int32_t saId)
    {
        bool routeFlow = command == "globalrouteflow";
        bool backgroundFlow = command == "globalbackgroundflow";
        bool newUserFlow = command == "globalnewuserflow";
        if (!routeFlow && !backgroundFlow && !newUserFlow) {
            return false;
        }
        GlobalSubscriptionOptions options;
        options.saId = saId;
        options.switchForeground = routeFlow;
        options.activateNewUser = newUserFlow;
        const std::string* processName = GetArgument(THIRD_OPTION_INDEX);
        if (!ParseArgument(FIRST_OPTION_INDEX, "foregroundUserId", false, options.foregroundUserId) ||
            !ParseArgument(SECOND_OPTION_INDEX, "otherUserId", false, options.otherUserId) || processName == nullptr) {
            std::cout << "invalid global subscription arguments" << std::endl;
            return true;
        }
        options.processName = *processName;
        (void)RunGlobalSubscriptionFlow(helper_, options);
        return true;
    }

    bool HandleApiFlow(const std::string& command, int32_t saId)
    {
        bool allApis = command == "allApiFlow";
        bool rejectApis = command == "rejectApiFlow";
        if (!allApis && !rejectApis) {
            return false;
        }
        ApiFlowOptions options;
        options.target.saId = saId;
        options.target.useUserIdApi = true;
        options.expectRejected = rejectApis;
        const std::string* processName = GetArgument(SECOND_OPTION_INDEX);
        if (!ParseArgument(FIRST_OPTION_INDEX, "userId", true, options.target.userId) || processName == nullptr) {
            std::cout << "invalid API flow arguments" << std::endl;
            return true;
        }
        options.processName = *processName;
        (void)RunApiFlow(helper_, options);
        return true;
    }

    bool HandleConcurrentLoadFlow(const std::string& command, int32_t saId)
    {
        if (command != "concurrentLoadFlow") {
            return false;
        }
        ConcurrentLoadOptions options;
        options.saId = saId;
        if (!ParseArgument(FIRST_OPTION_INDEX, "firstUserId", false, options.firstUserId) ||
            !ParseArgument(SECOND_OPTION_INDEX, "secondUserId", false, options.secondUserId)) {
            return true;
        }
        (void)RunConcurrentLoadFlow(helper_, options);
        return true;
    }

    bool HandleTimedPolicyFlow(const std::string& command, int32_t baseSaId)
    {
        if (command != "timedPolicyFlow") {
            return false;
        }
        int32_t multiSaId = 0;
        int32_t firstUserId = 0;
        int32_t secondUserId = 0;
        if (!ParseArgument(FIRST_OPTION_INDEX, "multiSaId", false, multiSaId) ||
            !ParseArgument(SECOND_OPTION_INDEX, "firstUserId", false, firstUserId) ||
            !ParseArgument(THIRD_OPTION_INDEX, "secondUserId", false, secondUserId)) {
            return true;
        }
        std::array<PolicyUpdateRequest, TIMED_POLICY_TARGET_COUNT> requests;
        requests[FIRST_TARGET_INDEX].target = {baseSaId, ONDEMAND_BASE_USER_ID, false};
        requests[SECOND_TARGET_INDEX].target = {multiSaId, firstUserId, true};
        requests[THIRD_TARGET_INDEX].target = {multiSaId, secondUserId, true};
        const std::array<int32_t, TIMED_POLICY_TARGET_COUNT> offsets{
            FIRST_TIMED_OFFSET_SECONDS, SECOND_TIMED_OFFSET_SECONDS, THIRD_TIMED_OFFSET_SECONDS};
        bool passed = helper_.LoadSystemAbility(baseSaId, DEFAULT_LOAD_TIMEOUT_SECONDS) != nullptr &&
                      helper_.LoadSystemAbility(multiSaId, DEFAULT_LOAD_TIMEOUT_SECONDS, firstUserId) != nullptr &&
                      helper_.LoadSystemAbility(multiSaId, DEFAULT_LOAD_TIMEOUT_SECONDS, secondUserId) != nullptr;
        for (size_t index = 0; index < requests.size(); ++index) {
            requests[index].event = {OnDemandEventId::TIMED_EVENT, "timedevent", FormatFutureTime(offsets[index]),
                                     true};
            passed = helper_.UpdateOnDemandPolicyBySa(requests[index]) == ERR_OK && passed;
            PolicyQueryRequest query{requests[index].target, OnDemandPolicyType::START_POLICY};
            passed = helper_.GetOnDemandPolicyBySa(query) == ERR_OK && passed;
        }
        std::cout << "TIMED_POLICY_FLOW_RESULT:" << (passed ? "PASS" : "FAIL") << std::endl;
        return true;
    }

    bool HandleRemoveFlow(const std::string& command, int32_t saId)
    {
        bool baseFlow = CommandMatches(command, "removeflow", REMOVE_FLOW_COMMAND_ALIAS);
        bool userFlow = CommandMatches(command, "removeflowbyuser", REMOVE_FLOW_BY_USER_COMMAND_ALIAS);
        if (!baseFlow && !userFlow) {
            return false;
        }
        RemoveFlowOptions options;
        options.target.saId = saId;
        if (baseFlow) {
            const std::string* processName = GetArgument(FIRST_OPTION_INDEX);
            if (processName != nullptr) {
                options.processName = *processName;
            }
        } else {
            options.target.useUserIdApi = true;
            const std::string* processName = GetArgument(THIRD_OPTION_INDEX);
            if (!ParseArgument(FIRST_OPTION_INDEX, "userId", false, options.target.userId) ||
                !ParseArgument(SECOND_OPTION_INDEX, "isolationUserId", false, options.isolationUserId) ||
                processName == nullptr) {
                std::cout << "invalid remove flow arguments" << std::endl;
                return true;
            }
            options.processName = *processName;
        }
        if (options.processName.empty()) {
            std::cout << "missing processName" << std::endl;
            return true;
        }
        (void)RunRemoveFlow(helper_, options);
        return true;
    }

    OnDemandHelper& helper_;
    sptr<DirectAbilityListener> abilityListener_ = new DirectAbilityListener();
    sptr<DirectProcessListener> processListener_ = new DirectProcessListener();
    std::vector<std::string> arguments_;
    bool userPrefixApplied_ = false;
};
} // namespace

bool TryRunOnDemandCommand(int argc, char* argv[], OnDemandHelper& helper, int32_t& exitCode)
{
    CommandDispatcher dispatcher(helper);
    return dispatcher.Run(argc, argv, exitCode);
}

} // namespace OHOS
#endif
