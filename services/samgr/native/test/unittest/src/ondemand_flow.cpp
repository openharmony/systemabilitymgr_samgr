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

#include "ondemand_flow.h"

#ifdef SUPPORT_MULTI_INSTANCE
#include <algorithm>
#include <array>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <list>
#include <mutex>
#include <sstream>
#include <thread>
#include <unistd.h>
#include <utility>

#include "errors.h"
#include "message_option.h"
#include "message_parcel.h"
#include "sam_mock_permission.h"
#include "system_ability_status_change_stub.h"
#include "system_process_status_change_stub.h"

namespace OHOS {
namespace {
constexpr int32_t CALLBACK_TIMEOUT_MS = 5000;
constexpr int32_t LOAD_TIMEOUT_SECONDS = 4;
constexpr int32_t MICROSECONDS_PER_MILLISECOND = 1000;
constexpr useconds_t POLL_INTERVAL_US = 200 * 1000;
constexpr useconds_t CALLBACK_DRAIN_US = 500 * 1000;
constexpr int32_t NO_CALLBACK = 0;
constexpr int32_t FIRST_CALLBACK = 1;
constexpr int32_t SECOND_CALLBACK = 2;
constexpr size_t CONCURRENT_LOAD_COUNT = 2;
constexpr size_t FIRST_LOAD_INDEX = 0;
constexpr size_t SECOND_LOAD_INDEX = 1;

enum class SubscriptionRequestCode : uint32_t {
    TRIGGER_UNLOAD = 3,
    TRIGGER_REMOVE = 7,
    TRIGGER_REPUBLISH = 8,
};

std::mutex g_subscriptionOutputMutex;

void PrintSubscriptionLine(const std::string& line)
{
    std::lock_guard<std::mutex> lock(g_subscriptionOutputMutex);
    std::cout << line << std::endl;
}

struct CallbackCounts {
    int32_t add = 0;
    int32_t remove = 0;
    int32_t start = 0;
    int32_t stop = 0;

    bool operator==(const CallbackCounts& other) const
    {
        return add == other.add && remove == other.remove && start == other.start && stop == other.stop;
    }
};

class AbilityStatusListener final : public SystemAbilityStatusChangeStub {
public:
    explicit AbilityStatusListener(int32_t targetSaId) : targetSaId_(targetSaId)
    {
    }

    void OnAddSystemAbility(int32_t saId, const std::string& deviceId) override
    {
        (void)deviceId;
        if (saId != targetSaId_) {
            return;
        }
        std::lock_guard<std::mutex> lock(mutex_);
        ++addCount_;
        std::ostringstream line;
        line << "SUBSCRIPTION_CALLBACK event=OnAddSystemAbility saId=" << saId << " count=" << addCount_;
        PrintSubscriptionLine(line.str());
        condition_.notify_all();
    }

    void OnRemoveSystemAbility(int32_t saId, const std::string& deviceId) override
    {
        (void)deviceId;
        if (saId != targetSaId_) {
            return;
        }
        std::lock_guard<std::mutex> lock(mutex_);
        ++removeCount_;
        std::ostringstream line;
        line << "SUBSCRIPTION_CALLBACK event=OnRemoveSystemAbility saId=" << saId << " count=" << removeCount_;
        PrintSubscriptionLine(line.str());
        condition_.notify_all();
    }

    bool WaitFor(int32_t addCount, int32_t removeCount)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        return condition_.wait_for(
            lock, std::chrono::milliseconds(CALLBACK_TIMEOUT_MS),
            [this, addCount, removeCount]() { return addCount_ >= addCount && removeCount_ >= removeCount; });
    }

    CallbackCounts GetCounts()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        CallbackCounts counts;
        counts.add = addCount_;
        counts.remove = removeCount_;
        return counts;
    }

private:
    const int32_t targetSaId_;
    int32_t addCount_ = 0;
    int32_t removeCount_ = 0;
    std::mutex mutex_;
    std::condition_variable condition_;
};

class ProcessStatusListener final : public SystemProcessStatusChangeStub {
public:
    explicit ProcessStatusListener(const std::string& targetProcess) : targetProcess_(targetProcess)
    {
    }

    void OnSystemProcessStarted(SystemProcessInfo& processInfo) override
    {
        if (processInfo.processName != targetProcess_) {
            return;
        }
        std::lock_guard<std::mutex> lock(mutex_);
        ++startCount_;
        std::ostringstream line;
        line << "SUBSCRIPTION_CALLBACK event=OnSystemProcessStarted process=" << processInfo.processName
             << " count=" << startCount_;
        PrintSubscriptionLine(line.str());
        condition_.notify_all();
    }

    void OnSystemProcessStopped(SystemProcessInfo& processInfo) override
    {
        if (processInfo.processName != targetProcess_) {
            return;
        }
        std::lock_guard<std::mutex> lock(mutex_);
        ++stopCount_;
        std::ostringstream line;
        line << "SUBSCRIPTION_CALLBACK event=OnSystemProcessStopped process=" << processInfo.processName
             << " count=" << stopCount_;
        PrintSubscriptionLine(line.str());
        condition_.notify_all();
    }

    bool WaitFor(int32_t startCount, int32_t stopCount)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        return condition_.wait_for(
            lock, std::chrono::milliseconds(CALLBACK_TIMEOUT_MS),
            [this, startCount, stopCount]() { return startCount_ >= startCount && stopCount_ >= stopCount; });
    }

    CallbackCounts GetCounts()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        CallbackCounts counts;
        counts.start = startCount_;
        counts.stop = stopCount_;
        return counts;
    }

private:
    const std::string targetProcess_;
    int32_t startCount_ = 0;
    int32_t stopCount_ = 0;
    std::mutex mutex_;
    std::condition_variable condition_;
};

struct SubscriptionState {
    OnDemandHelper* helper = nullptr;
    int32_t saId = 0;
    int32_t userId = ONDEMAND_BASE_USER_ID;
    bool useUserIdApi = false;
    bool timeoutLoad = false;
    bool subscribeProcess = false;
    bool unsubscribed = false;
    std::string processName;
    sptr<AbilityStatusListener> saListener;
    sptr<ProcessStatusListener> processListener;
};

struct RemoveState {
    SubscriptionState session;
    int32_t isolationUserId = ONDEMAND_BASE_USER_ID;
    sptr<IRemoteObject> target;
    sptr<IRemoteObject> isolation;
};

CallbackCounts MergeCounts(SubscriptionState& state)
{
    CallbackCounts counts = state.saListener->GetCounts();
    CallbackCounts processCounts = state.processListener->GetCounts();
    counts.start = processCounts.start;
    counts.stop = processCounts.stop;
    return counts;
}

void PrintCounts(const char* phase, const CallbackCounts& counts)
{
    std::cout << "SUBSCRIPTION_COUNTS phase=" << phase << " add=" << counts.add << " remove=" << counts.remove
              << " start=" << counts.start << " stop=" << counts.stop << std::endl;
}

sptr<IRemoteObject> LoadAbility(SubscriptionState& state)
{
    int32_t result = ERR_INVALID_OPERATION;
    sptr<IRemoteObject> object;
    if (state.useUserIdApi) {
        result = state.timeoutLoad
                     ? (state.helper->LoadSystemAbility(state.saId, LOAD_TIMEOUT_SECONDS, state.userId) == nullptr
                            ? ERR_NULL_OBJECT
                            : ERR_OK)
                     : state.helper->LoadSystemAbilityByCallback(state.saId, state.userId);
        object = result == ERR_OK ? state.helper->GetSystemAbility(state.saId, state.userId) : nullptr;
    } else {
        result = state.timeoutLoad
                     ? (state.helper->LoadSystemAbility(state.saId, LOAD_TIMEOUT_SECONDS) == nullptr ? ERR_NULL_OBJECT
                                                                                                     : ERR_OK)
                     : state.helper->LoadSystemAbilityByCallback(state.saId);
        object = result == ERR_OK ? state.helper->GetSystemAbility(state.saId) : nullptr;
    }
    std::cout << "SUBSCRIPTION_LIFECYCLE action=load saId=" << state.saId
              << " result=" << (object != nullptr ? "success" : "fail") << " code=" << result << std::endl;
    return object;
}

int32_t GetProcessInfo(SubscriptionState& state, SystemProcessInfo& processInfo)
{
    return state.useUserIdApi ? state.helper->GetSystemProcessInfo(state.saId, processInfo, state.userId)
                              : state.helper->GetSystemProcessInfo(state.saId, processInfo);
}

sptr<IRemoteObject> GetLocalProxy(SubscriptionState& state)
{
    return state.useUserIdApi ? state.helper->GetLocalAbilityManagerProxy(state.saId, state.userId)
                              : state.helper->GetLocalAbilityManagerProxy(state.saId);
}

bool QueryAbilityDetails(SubscriptionState& state)
{
    bool success = false;
    if (state.timeoutLoad) {
        bool isExist = false;
        sptr<IRemoteObject> object = state.useUserIdApi
                                         ? state.helper->CheckSystemAbilityByUserId(state.saId, isExist, state.userId)
                                         : state.helper->CheckSystemAbility(state.saId, isExist);
        success = object != nullptr && isExist;
    } else {
        success = state.useUserIdApi ? state.helper->GetSystemAbility(state.saId, state.userId) != nullptr
                                     : state.helper->GetSystemAbility(state.saId) != nullptr;
    }
    if (!success) {
        return false;
    }
    SystemProcessInfo processInfo;
    int32_t processCode = GetProcessInfo(state, processInfo);
    sptr<IRemoteObject> proxy = GetLocalProxy(state);
    std::cout << "SUBSCRIPTION_QUERY action=get_process_info saId=" << state.saId << " code=" << processCode
              << " processName=" << processInfo.processName << " pid=" << processInfo.pid << " uid=" << processInfo.uid
              << std::endl;
    std::cout << "SUBSCRIPTION_QUERY action=get_local_proxy saId=" << state.saId
              << " result=" << (proxy != nullptr ? "success" : "fail") << std::endl;
    return processCode == ERR_OK && proxy != nullptr;
}

int32_t SendSessionRequest(const sptr<IRemoteObject>& object, SubscriptionRequestCode code, const char* action)
{
    if (object == nullptr) {
        return ERR_NULL_OBJECT;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    int32_t sendCode = object->SendRequest(static_cast<uint32_t>(code), data, reply, option);
    int32_t result = sendCode == ERR_OK ? reply.ReadInt32() : sendCode;
    std::cout << "SUBSCRIPTION_LIFECYCLE action=" << action << " requestCode=" << static_cast<uint32_t>(code)
              << " result=" << result << std::endl;
    return result;
}

int32_t Subscribe(SubscriptionState& state)
{
    int32_t saCode = state.useUserIdApi
                         ? state.helper->SubscribeSystemAbility(state.saId, state.saListener, state.userId)
                         : state.helper->SubscribeSystemAbility(state.saId, state.saListener);
    int32_t processCode = ERR_OK;
    if (state.subscribeProcess) {
        processCode = state.useUserIdApi ? state.helper->SubscribeSystemProcess(state.processListener, state.userId)
                                         : state.helper->SubscribeSystemProcess(state.processListener);
    }
    std::cout << "SUBSCRIPTION_API action=subscribe saCode=" << saCode << " processCode=" << processCode
              << " userId=" << state.userId << " useUserIdApi=" << state.useUserIdApi << std::endl;
    return saCode != ERR_OK ? saCode : processCode;
}

std::pair<int32_t, int32_t> Unsubscribe(SubscriptionState& state)
{
    if (state.unsubscribed) {
        return {ERR_OK, ERR_OK};
    }
    int32_t saCode = state.useUserIdApi
                         ? state.helper->UnSubscribeSystemAbility(state.saId, state.saListener, state.userId)
                         : state.helper->UnSubscribeSystemAbility(state.saId, state.saListener);
    int32_t processCode = ERR_OK;
    if (state.subscribeProcess) {
        processCode = state.useUserIdApi ? state.helper->UnSubscribeSystemProcess(state.processListener, state.userId)
                                         : state.helper->UnSubscribeSystemProcess(state.processListener);
    }
    state.unsubscribed = true;
    std::cout << "SUBSCRIPTION_API action=unsubscribe saCode=" << saCode << " processCode=" << processCode
              << " sameListener=true" << std::endl;
    return {saCode, processCode};
}

bool WaitForCallbacks(SubscriptionState& state, int32_t addCount, int32_t removeCount)
{
    bool abilityReady = state.saListener->WaitFor(addCount, removeCount);
    if (!abilityReady || !state.subscribeProcess) {
        return abilityReady;
    }
    (void)state.processListener->WaitFor(addCount, removeCount);
    return true;
}

bool IsProcessStopped(SubscriptionState& state)
{
    std::list<SystemProcessInfo> infos;
    if (state.helper->GetRunningSystemProcess(infos) != ERR_OK) {
        return false;
    }
    const std::string& processName = state.processName;
    return std::none_of(infos.begin(), infos.end(),
                        [&processName](const SystemProcessInfo& info) { return info.processName == processName; });
}

bool IsStopped(SubscriptionState& state)
{
    bool isExist = false;
    sptr<IRemoteObject> object = state.useUserIdApi
                                     ? state.helper->CheckSystemAbilityByUserId(state.saId, isExist, state.userId)
                                     : state.helper->CheckSystemAbility(state.saId, isExist);
    bool abilityStopped = object == nullptr && !isExist;
    return abilityStopped && (!state.subscribeProcess || IsProcessStopped(state));
}

bool WaitUntilStopped(SubscriptionState& state)
{
    constexpr int32_t pollCount = CALLBACK_TIMEOUT_MS * MICROSECONDS_PER_MILLISECOND / POLL_INTERVAL_US;
    for (int32_t index = 0; index < pollCount; ++index) {
        if (IsStopped(state)) {
            return true;
        }
        usleep(POLL_INTERVAL_US);
    }
    return false;
}

bool RunLifecycleCycle(SubscriptionState& state, bool waitCallbacks, bool verifyDetails)
{
    sptr<IRemoteObject> target = LoadAbility(state);
    if (target == nullptr || (verifyDetails && !QueryAbilityDetails(state))) {
        return false;
    }
    if (SendSessionRequest(target, SubscriptionRequestCode::TRIGGER_UNLOAD, "trigger_unload") != ERR_OK) {
        return false;
    }
    if (waitCallbacks) {
        bool ready = WaitForCallbacks(state, FIRST_CALLBACK, FIRST_CALLBACK);
        std::cout << "SUBSCRIPTION_LIFECYCLE action=trigger_unload result=" << (ready ? ERR_OK : ERR_INVALID_OPERATION)
                  << std::endl;
        return ready;
    }
    return WaitUntilStopped(state);
}

bool ValidateSubscriptionOptions(const SubscriptionFlowOptions& options)
{
    return options.target.saId > 0 &&
           (!options.target.useUserIdApi || options.target.userId >= ONDEMAND_BASE_USER_ID) &&
           (!options.subscribeProcess || !options.processName.empty());
}

bool PrepareSubscription(SubscriptionState& state, OnDemandHelper& helper, const SubscriptionFlowOptions& options)
{
    if (!ValidateSubscriptionOptions(options)) {
        return false;
    }
    state.helper = &helper;
    state.saId = options.target.saId;
    state.userId = options.target.userId;
    state.useUserIdApi = options.target.useUserIdApi;
    state.timeoutLoad = options.timeoutLoad;
    state.subscribeProcess = options.subscribeProcess;
    state.processName = options.processName;
    state.saListener = new AbilityStatusListener(options.target.saId);
    state.processListener = new ProcessStatusListener(options.processName);
    return true;
}

int32_t CompleteRejectedSession(SubscriptionState& state, int32_t subscribeCode)
{
    std::pair<int32_t, int32_t> codes = Unsubscribe(state);
    bool rejected =
        subscribeCode != ERR_OK && codes.first != ERR_OK && (!state.subscribeProcess || codes.second != ERR_OK);
    PrintCounts("rejected", MergeCounts(state));
    std::cout << "SUBSCRIPTION_SESSION_RESULT:" << (rejected ? "PASS" : "FAIL") << " expected=rejected" << std::endl;
    return rejected ? ERR_OK : ERR_INVALID_OPERATION;
}

int32_t CompleteSubscription(SubscriptionState& state, bool verifyUnsubscribe)
{
    CallbackCounts snapshot = MergeCounts(state);
    PrintCounts("snapshot", snapshot);
    std::pair<int32_t, int32_t> codes = Unsubscribe(state);
    bool processCallbacksValid = !state.subscribeProcess ||
        ((snapshot.start == 0 && snapshot.stop == 0) || (snapshot.start > 0 && snapshot.stop > 0));
    bool callbacksReceived = snapshot.add > 0 && snapshot.remove > 0 && processCallbacksValid;
    if (!callbacksReceived || codes.first != ERR_OK || codes.second != ERR_OK) {
        std::cout << "SUBSCRIPTION_SESSION_RESULT:FAIL reason=callback_or_unsubscribe" << std::endl;
        return ERR_INVALID_OPERATION;
    }
    if (!verifyUnsubscribe) {
        std::cout << "SUBSCRIPTION_SESSION_RESULT:PASS mode=active_callbacks" << std::endl;
        return ERR_OK;
    }
    if (!RunLifecycleCycle(state, false, false)) {
        return ERR_INVALID_OPERATION;
    }
    usleep(CALLBACK_DRAIN_US);
    bool unchanged = snapshot == MergeCounts(state);
    std::cout << "SUBSCRIPTION_CALLBACK_UNCHANGED:" << (unchanged ? "true" : "false") << std::endl;
    std::cout << "SUBSCRIPTION_SESSION_RESULT:" << (unchanged ? "PASS" : "FAIL") << " mode=unsubscribe_verification"
              << std::endl;
    return unchanged ? ERR_OK : ERR_INVALID_OPERATION;
}

bool RunUserCycle(SubscriptionState& state, int32_t userId, bool expectCallbacks)
{
    SubscriptionState routedState = state;
    routedState.useUserIdApi = true;
    routedState.userId = userId;
    CallbackCounts before = MergeCounts(state);
    sptr<IRemoteObject> target = LoadAbility(routedState);
    if (target == nullptr || !QueryAbilityDetails(routedState) ||
        SendSessionRequest(target, SubscriptionRequestCode::TRIGGER_UNLOAD, "trigger_unload") != ERR_OK) {
        return false;
    }
    if (expectCallbacks) {
        bool ready = state.saListener->WaitFor(before.add + FIRST_CALLBACK, before.remove + FIRST_CALLBACK);
        return ready && (!state.subscribeProcess ||
                         state.processListener->WaitFor(before.start + FIRST_CALLBACK, before.stop + FIRST_CALLBACK));
    }
    if (!WaitUntilStopped(routedState)) {
        return false;
    }
    usleep(CALLBACK_DRAIN_US);
    return before == MergeCounts(state);
}

void CleanupSubscription(SubscriptionState& state)
{
    (void)Unsubscribe(state);
}

bool ExistsForUser(SubscriptionState& state, int32_t userId, bool useUserIdApi)
{
    SubscriptionState routedState = state;
    routedState.userId = userId;
    routedState.useUserIdApi = useUserIdApi;
    bool isExist = false;
    sptr<IRemoteObject> object =
        routedState.useUserIdApi
            ? routedState.helper->CheckSystemAbilityByUserId(routedState.saId, isExist, routedState.userId)
            : routedState.helper->CheckSystemAbility(routedState.saId, isExist);
    return object != nullptr && isExist;
}

bool ValidateRemoveOptions(const RemoveFlowOptions& options)
{
    bool validTarget = options.target.saId > 0 && !options.processName.empty();
    bool validUser = !options.target.useUserIdApi || options.target.userId > ONDEMAND_BASE_USER_ID;
    bool validIsolation = options.isolationUserId == ONDEMAND_BASE_USER_ID ||
                          (options.target.useUserIdApi && options.isolationUserId != options.target.userId);
    return validTarget && validUser && validIsolation;
}

bool PrepareRemove(RemoveState& state)
{
    if (Subscribe(state.session) != ERR_OK) {
        return false;
    }
    if (state.isolationUserId != ONDEMAND_BASE_USER_ID) {
        SubscriptionState isolationState = state.session;
        isolationState.userId = state.isolationUserId;
        isolationState.useUserIdApi = true;
        state.isolation = LoadAbility(isolationState);
        if (state.isolation == nullptr) {
            return false;
        }
    }
    state.target = LoadAbility(state.session);
    return state.target != nullptr && WaitForCallbacks(state.session, FIRST_CALLBACK, NO_CALLBACK);
}

bool ExecuteRemove(RemoveState& state)
{
    int32_t result = SendSessionRequest(state.target, SubscriptionRequestCode::TRIGGER_REMOVE, "trigger_remove");
    if (result != ERR_OK || !state.session.saListener->WaitFor(FIRST_CALLBACK, FIRST_CALLBACK)) {
        return false;
    }
    bool targetExists = ExistsForUser(state.session, state.session.userId, state.session.useUserIdApi);
    bool isolationExists =
        state.isolationUserId != ONDEMAND_BASE_USER_ID && ExistsForUser(state.session, state.isolationUserId, true);
    if (targetExists || (state.isolationUserId != ONDEMAND_BASE_USER_ID && !isolationExists)) {
        return false;
    }
    if (SendSessionRequest(state.target, SubscriptionRequestCode::TRIGGER_REMOVE, "trigger_remove_again") == ERR_OK) {
        return false;
    }
    result = SendSessionRequest(state.target, SubscriptionRequestCode::TRIGGER_REPUBLISH, "trigger_republish");
    return result == ERR_OK && state.session.saListener->WaitFor(SECOND_CALLBACK, FIRST_CALLBACK) &&
           ExistsForUser(state.session, state.session.userId, state.session.useUserIdApi);
}

void CleanupRemove(RemoveState& state)
{
    (void)Unsubscribe(state.session);
    (void)SendSessionRequest(state.target, SubscriptionRequestCode::TRIGGER_UNLOAD, "cleanup_unload");
    if (state.isolation != nullptr) {
        (void)SendSessionRequest(state.isolation, SubscriptionRequestCode::TRIGGER_UNLOAD, "cleanup_isolation_unload");
    }
}

bool FinishRemove(RemoveState& state)
{
    CallbackCounts snapshot = MergeCounts(state.session);
    std::pair<int32_t, int32_t> codes = Unsubscribe(state.session);
    CleanupRemove(state);
    bool cleaned = WaitUntilStopped(state.session);
    if (state.isolationUserId != ONDEMAND_BASE_USER_ID) {
        cleaned = cleaned && !ExistsForUser(state.session, state.isolationUserId, true);
    }
    usleep(CALLBACK_DRAIN_US);
    bool unchanged = snapshot == MergeCounts(state.session);
    std::cout << "REMOVE_CALLBACK_UNCHANGED:" << (unchanged ? "true" : "false") << std::endl;
    return cleaned && unchanged && codes.first == ERR_OK && codes.second == ERR_OK;
}

int32_t ChangeUserState(SubscriptionState& state, int32_t userId, SamgrUserState userState, const char* stateName)
{
    int32_t result = state.helper->OnUserStateChanged(userId, userState);
    SamMockPermission::MockPermission();
    std::cout << "GLOBAL_NEW_USER_STATE state=" << stateName << " userId=" << userId << " result=" << result
              << std::endl;
    return result;
}

int32_t RunNewUserSession(SubscriptionState& state, const GlobalSubscriptionOptions& options)
{
    CallbackCounts before = MergeCounts(state);
    bool stateChanged =
        ChangeUserState(state, options.otherUserId, SamgrUserState::USER_STATE_ACTIVATING, "ACTIVATING") == ERR_OK;
    stateChanged = stateChanged && ChangeUserState(state, options.otherUserId, SamgrUserState::USER_STATE_SWITCHING,
                                                   "SWITCHING") == ERR_OK;
    bool cyclePassed = stateChanged && RunUserCycle(state, options.otherUserId, true);
    CallbackCounts after = MergeCounts(state);
    CallbackCounts delta{after.add - before.add, after.remove - before.remove, after.start - before.start,
                         after.stop - before.stop};
    PrintCounts("new_user_delta", delta);
    std::pair<int32_t, int32_t> codes = Unsubscribe(state);
    bool oneCycle = delta.add == FIRST_CALLBACK && delta.remove == FIRST_CALLBACK && delta.start == FIRST_CALLBACK &&
                    delta.stop == FIRST_CALLBACK;
    bool passed = cyclePassed && oneCycle && codes.first == ERR_OK && codes.second == ERR_OK;
    std::cout << "GLOBAL_NEW_USER_SUBSCRIPTION_RESULT:" << (passed ? "PASS" : "FAIL")
              << " existingUser=" << options.foregroundUserId << " newUser=" << options.otherUserId << std::endl;
    return passed ? ERR_OK : ERR_INVALID_OPERATION;
}

bool VerifyAllQueries(SubscriptionState& state)
{
    bool isExist = false;
    sptr<IRemoteObject> getObject = state.helper->GetSystemAbility(state.saId, state.userId);
    sptr<IRemoteObject> checkObject = state.helper->CheckSystemAbility(state.saId, state.userId);
    sptr<IRemoteObject> existObject = state.helper->CheckSystemAbilityByUserId(state.saId, isExist, state.userId);
    SystemProcessInfo processInfo;
    int32_t infoCode = state.helper->GetSystemProcessInfo(state.saId, processInfo, state.userId);
    sptr<IRemoteObject> proxy = state.helper->GetLocalAbilityManagerProxy(state.saId, state.userId);
    return getObject != nullptr && checkObject != nullptr && existObject != nullptr && isExist && infoCode == ERR_OK &&
           proxy != nullptr;
}

bool VerifyBothLoads(SubscriptionState& state)
{
    int32_t callbackCode = state.helper->LoadSystemAbilityByCallback(state.saId, state.userId);
    sptr<IRemoteObject> timeoutObject = state.helper->LoadSystemAbility(state.saId, LOAD_TIMEOUT_SECONDS, state.userId);
    return callbackCode == ERR_OK && timeoutObject != nullptr;
}

bool VerifyRejectedApis(SubscriptionState& state)
{
    bool isExist = false;
    bool queriesRejected = state.helper->GetSystemAbility(state.saId, state.userId) == nullptr &&
                           state.helper->CheckSystemAbility(state.saId, state.userId) == nullptr &&
                           state.helper->CheckSystemAbilityByUserId(state.saId, isExist, state.userId) == nullptr &&
                           !isExist;
    SystemProcessInfo processInfo;
    queriesRejected = queriesRejected &&
                      state.helper->GetSystemProcessInfo(state.saId, processInfo, state.userId) != ERR_OK &&
                      state.helper->GetLocalAbilityManagerProxy(state.saId, state.userId) == nullptr;
    bool loadsRejected = state.helper->LoadSystemAbilityByCallback(state.saId, state.userId) != ERR_OK &&
                         state.helper->LoadSystemAbility(state.saId, LOAD_TIMEOUT_SECONDS, state.userId) == nullptr;
    int32_t subscribeCode = Subscribe(state);
    std::pair<int32_t, int32_t> unsubscribeCodes = Unsubscribe(state);
    return queriesRejected && loadsRejected && subscribeCode != ERR_OK && unsubscribeCodes.first != ERR_OK &&
           unsubscribeCodes.second != ERR_OK && MergeCounts(state) == CallbackCounts{};
}

struct ConcurrentResult {
    int32_t code = ERR_INVALID_OPERATION;
    std::chrono::steady_clock::time_point begin;
    std::chrono::steady_clock::time_point end;
};

bool RunConcurrentRound(OnDemandHelper& helper, const ConcurrentLoadOptions& options)
{
    std::mutex mutex;
    std::condition_variable condition;
    size_t readyCount = 0;
    bool start = false;
    std::array<ConcurrentResult, CONCURRENT_LOAD_COUNT> results;
    auto load = [&helper, &mutex, &condition, &readyCount, &start, &results, saId = options.saId](size_t index,
                                                                                                  int32_t userId) {
        {
            std::unique_lock<std::mutex> lock(mutex);
            ++readyCount;
            condition.notify_all();
            condition.wait(lock, [&start]() { return start; });
        }
        results[index].begin = std::chrono::steady_clock::now();
        results[index].code = helper.LoadSystemAbilityByCallback(saId, userId);
        results[index].end = std::chrono::steady_clock::now();
    };
    std::thread first(load, FIRST_LOAD_INDEX, options.firstUserId);
    std::thread second(load, SECOND_LOAD_INDEX, options.secondUserId);
    {
        std::unique_lock<std::mutex> lock(mutex);
        condition.wait(lock, [&readyCount]() { return readyCount == CONCURRENT_LOAD_COUNT; });
        start = true;
        condition.notify_all();
    }
    first.join();
    second.join();
    bool overlap = results[FIRST_LOAD_INDEX].begin < results[SECOND_LOAD_INDEX].end &&
                   results[SECOND_LOAD_INDEX].begin < results[FIRST_LOAD_INDEX].end;
    return results[FIRST_LOAD_INDEX].code == ERR_OK && results[SECOND_LOAD_INDEX].code == ERR_OK && overlap;
}

bool VerifyConcurrentTargets(OnDemandHelper& helper, const ConcurrentLoadOptions& options)
{
    SystemProcessInfo firstInfo;
    SystemProcessInfo secondInfo;
    bool firstReady = helper.GetSystemAbility(options.saId, options.firstUserId) != nullptr &&
                      helper.GetSystemProcessInfo(options.saId, firstInfo, options.firstUserId) == ERR_OK;
    bool secondReady = helper.GetSystemAbility(options.saId, options.secondUserId) != nullptr &&
                       helper.GetSystemProcessInfo(options.saId, secondInfo, options.secondUserId) == ERR_OK;
    return firstReady && secondReady && firstInfo.pid != secondInfo.pid;
}

bool CleanupConcurrentTargets(OnDemandHelper& helper, const ConcurrentLoadOptions& options)
{
    OnDemandTarget first{options.saId, options.firstUserId, true};
    OnDemandTarget second{options.saId, options.secondUserId, true};
    if (helper.TriggerUnloadSystemAbility(first, false) != ERR_OK ||
        helper.TriggerUnloadSystemAbility(second, false) != ERR_OK) {
        return false;
    }
    constexpr int32_t pollCount = CALLBACK_TIMEOUT_MS * MICROSECONDS_PER_MILLISECOND / POLL_INTERVAL_US;
    for (int32_t index = 0; index < pollCount; ++index) {
        bool firstExist = false;
        bool secondExist = false;
        helper.CheckSystemAbilityByUserId(options.saId, firstExist, options.firstUserId);
        helper.CheckSystemAbilityByUserId(options.saId, secondExist, options.secondUserId);
        if (!firstExist && !secondExist) {
            return true;
        }
        usleep(POLL_INTERVAL_US);
    }
    return false;
}
} // namespace

int32_t RunSubscriptionFlow(OnDemandHelper& helper, const SubscriptionFlowOptions& options)
{
    SubscriptionState state;
    if (!PrepareSubscription(state, helper, options)) {
        std::cout << "SUBSCRIPTION_SESSION_RESULT:FAIL reason=invalid_arguments" << std::endl;
        return ERR_INVALID_VALUE;
    }
    int32_t subscribeCode = Subscribe(state);
    if (options.expectRejected) {
        return CompleteRejectedSession(state, subscribeCode);
    }
    if (subscribeCode != ERR_OK || !RunLifecycleCycle(state, true, true)) {
        CleanupSubscription(state);
        std::cout << "SUBSCRIPTION_SESSION_RESULT:FAIL reason=first_cycle" << std::endl;
        return ERR_INVALID_OPERATION;
    }
    return CompleteSubscription(state, options.verifyUnsubscribe);
}

int32_t RunGlobalSubscriptionFlow(OnDemandHelper& helper, const GlobalSubscriptionOptions& options)
{
    SubscriptionFlowOptions sessionOptions;
    sessionOptions.target.saId = options.saId;
    sessionOptions.subscribeProcess = true;
    sessionOptions.processName = options.processName;
    SubscriptionState state;
    bool validUsers = options.foregroundUserId > ONDEMAND_BASE_USER_ID && options.otherUserId > ONDEMAND_BASE_USER_ID &&
                      options.foregroundUserId != options.otherUserId;
    if (!validUsers || !PrepareSubscription(state, helper, sessionOptions) || Subscribe(state) != ERR_OK) {
        std::cout << "GLOBAL_SUBSCRIPTION_RESULT:FAIL reason=prepare" << std::endl;
        return ERR_INVALID_VALUE;
    }
    if (options.activateNewUser) {
        return RunNewUserSession(state, options);
    }
    bool passed = RunUserCycle(state, options.foregroundUserId, true);
    if (passed && options.switchForeground) {
        passed = state.helper->OnUserStateChanged(options.otherUserId, SamgrUserState::USER_STATE_SWITCHING) == ERR_OK;
        SamMockPermission::MockPermission();
    }
    passed = passed && RunUserCycle(state, options.otherUserId, options.switchForeground);
    std::pair<int32_t, int32_t> codes = Unsubscribe(state);
    passed = passed && codes.first == ERR_OK && codes.second == ERR_OK;
    std::cout << "GLOBAL_SUBSCRIPTION_RESULT:" << (passed ? "PASS" : "FAIL")
              << " mode=" << (options.switchForeground ? "foreground_switch" : "background_filtered") << std::endl;
    return passed ? ERR_OK : ERR_INVALID_OPERATION;
}

int32_t RunRemoveFlow(OnDemandHelper& helper, const RemoveFlowOptions& options)
{
    if (!ValidateRemoveOptions(options)) {
        std::cout << "REMOVE_SESSION_RESULT:FAIL reason=invalid_arguments" << std::endl;
        return ERR_INVALID_VALUE;
    }
    RemoveState state;
    state.session.helper = &helper;
    state.session.saId = options.target.saId;
    state.session.userId = options.target.userId;
    state.session.useUserIdApi = options.target.useUserIdApi;
    state.session.subscribeProcess = true;
    state.session.processName = options.processName;
    state.session.saListener = new AbilityStatusListener(options.target.saId);
    state.session.processListener = new ProcessStatusListener(options.processName);
    state.isolationUserId = options.isolationUserId;
    if (!PrepareRemove(state) || !ExecuteRemove(state)) {
        CleanupRemove(state);
        std::cout << "REMOVE_SESSION_RESULT:FAIL reason=remove_lifecycle" << std::endl;
        return ERR_INVALID_OPERATION;
    }
    bool passed = FinishRemove(state);
    std::cout << "REMOVE_SESSION_RESULT:" << (passed ? "PASS" : "FAIL") << std::endl;
    return passed ? ERR_OK : ERR_INVALID_OPERATION;
}

int32_t RunApiFlow(OnDemandHelper& helper, const ApiFlowOptions& options)
{
    SubscriptionFlowOptions subscriptionOptions;
    subscriptionOptions.target = options.target;
    subscriptionOptions.subscribeProcess = true;
    subscriptionOptions.processName = options.processName;
    SubscriptionState state;
    if (!PrepareSubscription(state, helper, subscriptionOptions)) {
        return ERR_INVALID_VALUE;
    }
    if (options.expectRejected) {
        bool rejected = VerifyRejectedApis(state);
        std::cout << "API_FLOW_RESULT:" << (rejected ? "PASS" : "FAIL") << " mode=rejected" << std::endl;
        return rejected ? ERR_OK : ERR_INVALID_OPERATION;
    }
    bool passed = Subscribe(state) == ERR_OK && VerifyBothLoads(state) && VerifyAllQueries(state) &&
                  RunLifecycleCycle(state, true, false);
    int32_t result = passed ? CompleteSubscription(state, true) : ERR_INVALID_OPERATION;
    CleanupSubscription(state);
    std::cout << "API_FLOW_RESULT:" << (result == ERR_OK ? "PASS" : "FAIL") << " mode=all" << std::endl;
    return result;
}

int32_t RunConcurrentLoadFlow(OnDemandHelper& helper, const ConcurrentLoadOptions& options)
{
    bool valid = options.saId > 0 && options.firstUserId > 0 && options.secondUserId > 0 &&
                 options.firstUserId != options.secondUserId;
    bool firstRound = valid && RunConcurrentRound(helper, options) && VerifyConcurrentTargets(helper, options);
    bool cleaned = firstRound && CleanupConcurrentTargets(helper, options);
    bool secondRound = cleaned && RunConcurrentRound(helper, options) && VerifyConcurrentTargets(helper, options);
    bool passed = secondRound && CleanupConcurrentTargets(helper, options);
    std::cout << "CONCURRENT_LOAD_RESULT:" << (passed ? "PASS" : "FAIL") << std::endl;
    return passed ? ERR_OK : ERR_INVALID_OPERATION;
}

} // namespace OHOS
#endif
