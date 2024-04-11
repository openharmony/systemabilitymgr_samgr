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

#include "system_ability_state_machine_test.h"
#include "samgr_err_code.h"
#include "sa_status_change_mock.h"
#include "test_log.h"
#define private public
#define protected public
#include "schedule/system_ability_state_machine.h"

using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS;

namespace OHOS {
namespace {
constexpr uint32_t SAID = 1234;
constexpr uint32_t INVALID_SAID = 0;
}
void StateMachineListener::OnAbilityNotLoadedLocked(int32_t systemAbilityId)
{
    return;
}
void StateMachineListener::OnAbilityLoadingLocked(int32_t systemAbilityId)
{
    return;
}
void StateMachineListener::OnAbilityLoadedLocked(int32_t systemAbilityId)
{
    return;
}
void StateMachineListener::OnAbilityUnloadableLocked(int32_t systemAbilityId)
{
    return;
}
void StateMachineListener::OnAbilityUnloadingLocked(int32_t systemAbilityId)
{
    return;
}
void StateMachineListener::OnProcessNotStartedLocked(const std::u16string& processName)
{
    return;
}
void StateMachineListener::OnProcessStartedLocked(const std::u16string& processName)
{
    return;
}
void StateMachineListener::OnProcessStoppingLocked(const std::u16string& processName)
{
    return;
}

void SystemAbilityStateMachineTest::SetUpTestCase()
{
    DTEST_LOG << "SetUpTestCase" << std::endl;
}

void SystemAbilityStateMachineTest::TearDownTestCase()
{
    DTEST_LOG << "TearDownTestCase" << std::endl;
}

void SystemAbilityStateMachineTest::SetUp()
{
    DTEST_LOG << "SetUp" << std::endl;
}

void SystemAbilityStateMachineTest::TearDown()
{
    DTEST_LOG << "TearDown" << std::endl;
}


/**
 * @tc.name: AbilityStateTransitionLocked001
 * @tc.desc: test AbilityStateTransitionLocked with context is nullptr
 * @tc.type: FUNC
 * @tc.require: I6KOQV
 */

HWTEST_F(SystemAbilityStateMachineTest, AbilityStateTransitionLocked001, TestSize.Level3)
{
    std::shared_ptr<StateMachineListener> stateListener =
        std::make_shared<StateMachineListener>();
    std::shared_ptr<SystemAbilityStateMachine> systemAbilityStateMachine =
        std::make_shared<SystemAbilityStateMachine>(stateListener);
    int32_t ret = systemAbilityStateMachine->AbilityStateTransitionLocked(nullptr,
        SystemAbilityState::NOT_LOADED);
    EXPECT_EQ(ret, SA_CONTEXT_NULL);
}

/**
 * @tc.name: AbilityStateTransitionLocked002
 * @tc.desc: test AbilityStateTransitionLocked with nextState is not in abilityStateHandlerMap_
 * @tc.type: FUNC
 * @tc.require: I6KOQV
 */

HWTEST_F(SystemAbilityStateMachineTest, AbilityStateTransitionLocked002, TestSize.Level3)
{
    std::shared_ptr<StateMachineListener> stateListener =
        std::make_shared<StateMachineListener>();
    std::shared_ptr<SystemAbilityStateMachine> systemAbilityStateMachine =
        std::make_shared<SystemAbilityStateMachine>(stateListener);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext =
        std::make_shared<SystemAbilityContext>();
    systemAbilityStateMachine->abilityStateHandlerMap_.clear();
    int32_t ret = systemAbilityStateMachine->AbilityStateTransitionLocked(systemAbilityContext,
        SystemAbilityState::NOT_LOADED);
    EXPECT_EQ(ret, INVALID_SA_NEXT_STATE);
}

/**
 * @tc.name: AbilityStateTransitionLocked003
 * @tc.desc: test AbilityStateTransitionLocked with abilityStateHandlerMap_[nextState] is nullptr
 * @tc.type: FUNC
 * @tc.require: I6KOQV
 */

HWTEST_F(SystemAbilityStateMachineTest, AbilityStateTransitionLocked003, TestSize.Level3)
{
    std::shared_ptr<StateMachineListener> stateListener =
        std::make_shared<StateMachineListener>();
    std::shared_ptr<SystemAbilityStateMachine> systemAbilityStateMachine =
        std::make_shared<SystemAbilityStateMachine>(stateListener);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext =
        std::make_shared<SystemAbilityContext>();
    systemAbilityStateMachine->abilityStateHandlerMap_.clear();
    systemAbilityStateMachine->abilityStateHandlerMap_[SystemAbilityState::NOT_LOADED] = nullptr;
    int32_t ret = systemAbilityStateMachine->AbilityStateTransitionLocked(systemAbilityContext,
        SystemAbilityState::NOT_LOADED);
    EXPECT_EQ(ret, SA_STATE_HANDLER_NULL);
}

/**
 * @tc.name: AbilityStateTransitionLocked004
 * @tc.desc: test AbilityStateTransitionLocked with context->state == currentState
 * @tc.type: FUNC
 * @tc.require: I6KOQV
 */

HWTEST_F(SystemAbilityStateMachineTest, AbilityStateTransitionLocked004, TestSize.Level3)
{
    std::shared_ptr<StateMachineListener> stateListener =
        std::make_shared<StateMachineListener>();
    std::shared_ptr<SystemAbilityStateMachine> systemAbilityStateMachine =
        std::make_shared<SystemAbilityStateMachine>(stateListener);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext =
        std::make_shared<SystemAbilityContext>();
    std::shared_ptr<StateMachineListener> stateListener1 =
        std::make_shared<StateMachineListener>();
    std::shared_ptr<NotLoadedStateHandler> notLoadedStateHandler =
        std::make_shared<NotLoadedStateHandler>(stateListener1);
    systemAbilityContext->state = SystemAbilityState::NOT_LOADED;
    systemAbilityStateMachine->abilityStateHandlerMap_.clear();
    systemAbilityStateMachine->abilityStateHandlerMap_[SystemAbilityState::NOT_LOADED] = notLoadedStateHandler;
    int32_t ret = systemAbilityStateMachine->AbilityStateTransitionLocked(systemAbilityContext,
        SystemAbilityState::NOT_LOADED);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: AbilityStateTransitionLocked005
 * @tc.desc: test AbilityStateTransitionLocked with context->state != currentState
 * @tc.type: FUNC
 * @tc.require: I6KOQV
 */

HWTEST_F(SystemAbilityStateMachineTest, AbilityStateTransitionLocked005, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityContext> systemAbilityContext =
        std::make_shared<SystemAbilityContext>();
    std::shared_ptr<StateMachineListener> stateListener =
        std::make_shared<StateMachineListener>();
    std::shared_ptr<SystemAbilityStateMachine> systemAbilityStateMachine =
        std::make_shared<SystemAbilityStateMachine>(stateListener);
    std::shared_ptr<StateMachineListener> stateListener1 =
        std::make_shared<StateMachineListener>();
    std::shared_ptr<NotLoadedStateHandler> notLoadedStateHandler =
        std::make_shared<NotLoadedStateHandler>(stateListener1);
    systemAbilityContext->state = SystemAbilityState::NOT_LOADED;
    systemAbilityStateMachine->abilityStateHandlerMap_.clear();
    systemAbilityStateMachine->abilityStateHandlerMap_[SystemAbilityState::NOT_LOADED] = notLoadedStateHandler;
    int32_t ret = systemAbilityStateMachine->AbilityStateTransitionLocked(systemAbilityContext,
        SystemAbilityState::LOADING);
    EXPECT_EQ(ret, INVALID_SA_NEXT_STATE);
}

/**
 * @tc.name: UpdateStateCount001
 * @tc.desc: test UpdateStateCount with context is nullptr
 * @tc.type: FUNC
 * @tc.require: I6KOQV
 */

HWTEST_F(SystemAbilityStateMachineTest, UpdateStateCount001, TestSize.Level3)
{
    std::shared_ptr<StateMachineListener> stateListener =
        std::make_shared<StateMachineListener>();
    std::shared_ptr<SystemAbilityStateMachine> systemAbilityStateMachine =
        std::make_shared<SystemAbilityStateMachine>(stateListener);
    bool ret = systemAbilityStateMachine->UpdateStateCount(nullptr,
        SystemAbilityState::NOT_LOADED, SystemAbilityState::LOADING);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: UpdateStateCount002
 * @tc.desc: test UpdateStateCount with abilityStateCountMap does not have fromState
 * @tc.type: FUNC
 * @tc.require: I6KOQV
 */

HWTEST_F(SystemAbilityStateMachineTest, UpdateStateCount002, TestSize.Level3)
{
    std::shared_ptr<StateMachineListener> stateListener =
        std::make_shared<StateMachineListener>();
    std::shared_ptr<SystemAbilityStateMachine> systemAbilityStateMachine =
        std::make_shared<SystemAbilityStateMachine>(stateListener);
    std::shared_ptr<SystemProcessContext> systemProcessContext =
        std::make_shared<SystemProcessContext>();
    systemProcessContext->abilityStateCountMap.clear();
    systemProcessContext->abilityStateCountMap[SystemAbilityState::LOADING] = SAID;
    bool ret = systemAbilityStateMachine->UpdateStateCount(systemProcessContext,
        SystemAbilityState::LOADED, SystemAbilityState::LOADING);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: UpdateStateCount003
 * @tc.desc: test UpdateStateCount with abilityStateCountMap does not have toState
 * @tc.type: FUNC
 * @tc.require: I6KOQV
 */

HWTEST_F(SystemAbilityStateMachineTest, UpdateStateCount003, TestSize.Level3)
{
    std::shared_ptr<StateMachineListener> stateListener =
        std::make_shared<StateMachineListener>();
    std::shared_ptr<SystemAbilityStateMachine> systemAbilityStateMachine =
        std::make_shared<SystemAbilityStateMachine>(stateListener);
    std::shared_ptr<SystemProcessContext> systemProcessContext =
        std::make_shared<SystemProcessContext>();
    systemProcessContext->abilityStateCountMap.clear();
    systemProcessContext->abilityStateCountMap[SystemAbilityState::NOT_LOADED] = SAID;
    bool ret = systemAbilityStateMachine->UpdateStateCount(systemProcessContext,
        SystemAbilityState::NOT_LOADED, SystemAbilityState::LOADING);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: UpdateStateCount004
 * @tc.desc: test UpdateStateCount with abilityStateCountMap is empty
 * @tc.type: FUNC
 * @tc.require: I6KOQV
 */

HWTEST_F(SystemAbilityStateMachineTest, UpdateStateCount004, TestSize.Level3)
{
    std::shared_ptr<StateMachineListener> stateListener =
        std::make_shared<StateMachineListener>();
    std::shared_ptr<SystemAbilityStateMachine> systemAbilityStateMachine =
        std::make_shared<SystemAbilityStateMachine>(stateListener);
    std::shared_ptr<SystemProcessContext> systemProcessContext =
        std::make_shared<SystemProcessContext>();
    systemProcessContext->abilityStateCountMap.clear();
    bool ret = systemAbilityStateMachine->UpdateStateCount(systemProcessContext,
        SystemAbilityState::NOT_LOADED, SystemAbilityState::LOADING);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: UpdateStateCount005
 * @tc.desc: test UpdateStateCount with abilityStateCountMap[fromState] is invalid
 * @tc.type: FUNC
 * @tc.require: I6KOQV
 */

HWTEST_F(SystemAbilityStateMachineTest, UpdateStateCount005, TestSize.Level3)
{
    std::shared_ptr<StateMachineListener> stateListener =
        std::make_shared<StateMachineListener>();
    std::shared_ptr<SystemAbilityStateMachine> systemAbilityStateMachine =
        std::make_shared<SystemAbilityStateMachine>(stateListener);
    std::shared_ptr<SystemProcessContext> systemProcessContext =
        std::make_shared<SystemProcessContext>();
    systemProcessContext->abilityStateCountMap.clear();
    systemProcessContext->abilityStateCountMap[SystemAbilityState::NOT_LOADED] = INVALID_SAID;
    systemProcessContext->abilityStateCountMap[SystemAbilityState::LOADING] = SAID;
    bool ret = systemAbilityStateMachine->UpdateStateCount(systemProcessContext,
        SystemAbilityState::NOT_LOADED, SystemAbilityState::LOADING);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: ProcessStateTransitionLocked001
 * @tc.desc: test ProcessStateTransitionLocked with context is nullptr
 * @tc.type: FUNC
 * @tc.require: I6KOQV
 */

HWTEST_F(SystemAbilityStateMachineTest, ProcessStateTransitionLocked001, TestSize.Level3)
{
    std::shared_ptr<StateMachineListener> stateListener =
        std::make_shared<StateMachineListener>();
    std::shared_ptr<SystemAbilityStateMachine> systemAbilityStateMachine =
        std::make_shared<SystemAbilityStateMachine>(stateListener);
    int32_t ret = systemAbilityStateMachine->ProcessStateTransitionLocked(nullptr, SystemProcessState::NOT_STARTED);
    EXPECT_EQ(ret, PROC_CONTEXT_NULL);
}

/**
 * @tc.name: ProcessStateTransitionLocked002
 * @tc.desc: test ProcessStateTransitionLocked with processStateHandlerMap_ does not have nextState
 * @tc.type: FUNC
 * @tc.require: I6KOQV
 */

HWTEST_F(SystemAbilityStateMachineTest, ProcessStateTransitionLocked002, TestSize.Level3)
{
    std::shared_ptr<StateMachineListener> stateListener =
        std::make_shared<StateMachineListener>();
    std::shared_ptr<SystemAbilityStateMachine> systemAbilityStateMachine =
        std::make_shared<SystemAbilityStateMachine>(stateListener);
    std::shared_ptr<SystemProcessContext> systemProcessContext =
        std::make_shared<SystemProcessContext>();
    systemAbilityStateMachine->processStateHandlerMap_.clear();
    int32_t ret = systemAbilityStateMachine->ProcessStateTransitionLocked(systemProcessContext,
        SystemProcessState::NOT_STARTED);
    EXPECT_EQ(ret, INVALID_PROC_NEXT_STATE);
}

/**
 * @tc.name: ProcessStateTransitionLocked003
 * @tc.desc: test ProcessStateTransitionLocked with processStateHandlerMap_[nextState] is nullptr
 * @tc.type: FUNC
 * @tc.require: I6KOQV
 */

HWTEST_F(SystemAbilityStateMachineTest, ProcessStateTransitionLocked003, TestSize.Level3)
{
    std::shared_ptr<StateMachineListener> stateListener =
        std::make_shared<StateMachineListener>();
    std::shared_ptr<SystemAbilityStateMachine> systemAbilityStateMachine =
        std::make_shared<SystemAbilityStateMachine>(stateListener);
    std::shared_ptr<SystemProcessContext> systemProcessContext =
        std::make_shared<SystemProcessContext>();
    systemAbilityStateMachine->processStateHandlerMap_.clear();
    systemAbilityStateMachine->processStateHandlerMap_[SystemProcessState::NOT_STARTED] = nullptr;
    int32_t ret = systemAbilityStateMachine->ProcessStateTransitionLocked(systemProcessContext,
        SystemProcessState::NOT_STARTED);
    EXPECT_EQ(ret, PROC_STATE_HANDLER_NULL);
}

/**
 * @tc.name: ProcessStateTransitionLocked004
 * @tc.desc: test ProcessStateTransitionLocked with context->state == nextState
 * @tc.type: FUNC
 * @tc.require: I6KOQV
 */

HWTEST_F(SystemAbilityStateMachineTest, ProcessStateTransitionLocked004, TestSize.Level3)
{
    std::shared_ptr<StateMachineListener> stateListener =
        std::make_shared<StateMachineListener>();
    std::shared_ptr<SystemAbilityStateMachine> systemAbilityStateMachine =
        std::make_shared<SystemAbilityStateMachine>(stateListener);
    std::shared_ptr<SystemProcessContext> systemProcessContext =
        std::make_shared<SystemProcessContext>();
    std::shared_ptr<StateMachineListener> stateListener1 =
        std::make_shared<StateMachineListener>();
    std::shared_ptr<NotStartedStateHandler> notStartedStateHandler =
        std::make_shared<NotStartedStateHandler>(stateListener1);
    systemProcessContext->state = SystemProcessState::NOT_STARTED;
    systemAbilityStateMachine->processStateHandlerMap_.clear();
    systemAbilityStateMachine->processStateHandlerMap_[SystemProcessState::NOT_STARTED] = notStartedStateHandler;
    int32_t ret = systemAbilityStateMachine->ProcessStateTransitionLocked(systemProcessContext,
        SystemProcessState::NOT_STARTED);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: ProcessStateTransitionLocked005
 * @tc.desc: test ProcessStateTransitionLockedwith context->state != nextState
 * @tc.type: FUNC
 * @tc.require: I6KOQV
 */

HWTEST_F(SystemAbilityStateMachineTest, ProcessStateTransitionLocked005, TestSize.Level3)
{
    std::shared_ptr<StateMachineListener> stateListener =
        std::make_shared<StateMachineListener>();
    std::shared_ptr<SystemAbilityStateMachine> systemAbilityStateMachine =
        std::make_shared<SystemAbilityStateMachine>(stateListener);
    std::shared_ptr<SystemProcessContext> systemProcessContext =
        std::make_shared<SystemProcessContext>();
    std::shared_ptr<StateMachineListener> stateListener1 =
        std::make_shared<StateMachineListener>();
    std::shared_ptr<StartedStateHandler> startedStateHandler =
        std::make_shared<StartedStateHandler>(stateListener1);
    systemProcessContext->state = SystemProcessState::STOPPING;
    systemAbilityStateMachine->processStateHandlerMap_.clear();
    systemAbilityStateMachine->processStateHandlerMap_[SystemProcessState::NOT_STARTED] = startedStateHandler;
    int32_t ret = systemAbilityStateMachine->ProcessStateTransitionLocked(systemProcessContext,
        SystemProcessState::STARTED);
    EXPECT_EQ(ret, INVALID_PROC_NEXT_STATE);
}

/**
 * @tc.name: NotLoadedStateHandlerOnEnter001
 * @tc.desc: test NotLoadedStateHandlerOnEnter with listener_ is nullptr
 * @tc.type: FUNC
 * @tc.require: I6KOQV
 */

HWTEST_F(SystemAbilityStateMachineTest, NotLoadedStateHandlerOnEnter001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityContext> systemAbilityContext =
        std::make_shared<SystemAbilityContext>();
    std::shared_ptr<StateMachineListener> stateListener =
        std::make_shared<StateMachineListener>();
    std::shared_ptr<NotLoadedStateHandler> notLoadedStateHandler =
        std::make_shared<NotLoadedStateHandler>(stateListener);
    stateListener = nullptr;
    notLoadedStateHandler->OnEnter(systemAbilityContext);
    EXPECT_EQ(notLoadedStateHandler->listener_.lock(), nullptr);
}

/**
 * @tc.name: LoadingStateHandlerOnEnter001
 * @tc.desc: test LoadingStateHandlerOnEnter with listener_ is nullptr
 * @tc.type: FUNC
 * @tc.require: I6KOQV
 */

HWTEST_F(SystemAbilityStateMachineTest, LoadingStateHandlerOnEnter001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityContext> systemAbilityContext =
        std::make_shared<SystemAbilityContext>();
    std::shared_ptr<StateMachineListener> stateListener =
        std::make_shared<StateMachineListener>();
    std::shared_ptr<LoadingStateHandler> loadingStateHandler =
        std::make_shared<LoadingStateHandler>(stateListener);
    stateListener = nullptr;
    loadingStateHandler->OnEnter(systemAbilityContext);
    EXPECT_EQ(loadingStateHandler->listener_.lock(), nullptr);
}

/**
 * @tc.name: LoadedStateHandlerCanEnter001
 * @tc.desc: test LoadedStateHandlerCanEnter with fromState == SystemAbilityState::LOADED
 * @tc.type: FUNC
 * @tc.require: I6KOQV
 */

HWTEST_F(SystemAbilityStateMachineTest, LoadedStateHandlerCanEnter001, TestSize.Level3)
{
    std::shared_ptr<StateMachineListener> stateListener =
        std::make_shared<StateMachineListener>();
    std::shared_ptr<LoadedStateHandler> loadedStateHandler =
        std::make_shared<LoadedStateHandler>(stateListener);
    bool ret = loadedStateHandler->CanEnter(SystemAbilityState::LOADED);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: LoadedStateHandlerOnEnter001
 * @tc.desc: test LoadedStateHandlerOnEnter with listener_ is nullptr
 * @tc.type: FUNC
 * @tc.require: I6KOQV
 */

HWTEST_F(SystemAbilityStateMachineTest, LoadedStateHandlerOnEnter001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityContext> systemAbilityContext =
        std::make_shared<SystemAbilityContext>();
    std::shared_ptr<StateMachineListener> stateListener =
        std::make_shared<StateMachineListener>();
    std::shared_ptr<LoadedStateHandler> loadedStateHandler =
        std::make_shared<LoadedStateHandler>(stateListener);
    stateListener = nullptr;
    loadedStateHandler->OnEnter(systemAbilityContext);
    EXPECT_EQ(loadedStateHandler->listener_.lock(), nullptr);
}

/**
 * @tc.name: UnloadableStateHandlerCanEnter001
 * @tc.desc: test UnloadableStateHandlerCanEnter with fromState == SystemAbilityState::LOADED
 * @tc.type: FUNC
 * @tc.require: I6KOQV
 */

HWTEST_F(SystemAbilityStateMachineTest, UnloadableStateHandlerCanEnter001, TestSize.Level3)
{
    std::shared_ptr<StateMachineListener> stateListener =
        std::make_shared<StateMachineListener>();
    std::shared_ptr<UnloadableStateHandler> unloadableStateHandler =
        std::make_shared<UnloadableStateHandler>(stateListener);
    bool ret = unloadableStateHandler->CanEnter(SystemAbilityState::LOADED);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: UnloadableStateHandlerOnEnter001
 * @tc.desc: test UnloadableStateHandlerOnEnter with listener_ is nullptr
 * @tc.type: FUNC
 * @tc.require: I6KOQV
 */

HWTEST_F(SystemAbilityStateMachineTest, UnloadableStateHandlerOnEnter001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityContext> systemAbilityContext =
        std::make_shared<SystemAbilityContext>();
    std::shared_ptr<StateMachineListener> stateListener =
        std::make_shared<StateMachineListener>();
    std::shared_ptr<UnloadableStateHandler> unloadableStateHandler =
        std::make_shared<UnloadableStateHandler>(stateListener);
    stateListener = nullptr;
    unloadableStateHandler->OnEnter(systemAbilityContext);
    EXPECT_EQ(unloadableStateHandler->listener_.lock(), nullptr);
}

/**
 * @tc.name: UnloadingStateHandlerCanEnter001
 * @tc.desc: test UnloadingStateHandlerCanEnter with fromState == SystemAbilityState::UNLOADABLE
 * @tc.type: FUNC
 * @tc.require: I6KOQV
 */

HWTEST_F(SystemAbilityStateMachineTest, UnloadingStateHandlerCanEnter001, TestSize.Level3)
{
    std::shared_ptr<StateMachineListener> stateListener =
        std::make_shared<StateMachineListener>();
    std::shared_ptr<UnloadingStateHandler> unloadingStateHandler =
        std::make_shared<UnloadingStateHandler>(stateListener);
    bool ret = unloadingStateHandler->CanEnter(SystemAbilityState::UNLOADABLE);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: UnloadingStateHandlerOnEnter001
 * @tc.desc: test UnloadingStateHandlerOnEnter with listener_ is nullptr
 * @tc.type: FUNC
 * @tc.require: I6KOQV
 */

HWTEST_F(SystemAbilityStateMachineTest, UnloadingStateHandlerOnEnter001, TestSize.Level3)
{
    std::shared_ptr<SystemAbilityContext> systemAbilityContext =
        std::make_shared<SystemAbilityContext>();
    std::shared_ptr<StateMachineListener> stateListener =
        std::make_shared<StateMachineListener>();
    std::shared_ptr<UnloadingStateHandler> unloadingStateHandler =
        std::make_shared<UnloadingStateHandler>(stateListener);
    stateListener = nullptr;
    unloadingStateHandler->OnEnter(systemAbilityContext);
    EXPECT_EQ(unloadingStateHandler->listener_.lock(), nullptr);
}

/**
 * @tc.name: NotStartedStateHandlerOnEnter001
 * @tc.desc: test NotStartedStateHandlerOnEnter with listener_ is nullptr
 * @tc.type: FUNC
 * @tc.require: I6KOQV
 */

HWTEST_F(SystemAbilityStateMachineTest, NotStartedStateHandlerOnEnter001, TestSize.Level3)
{
    std::shared_ptr<SystemProcessContext> systemProcessContext =
        std::make_shared<SystemProcessContext>();
    std::shared_ptr<StateMachineListener> stateListener =
        std::make_shared<StateMachineListener>();
    std::shared_ptr<NotStartedStateHandler> notStartedStateHandler =
        std::make_shared<NotStartedStateHandler>(stateListener);
    stateListener = nullptr;
    notStartedStateHandler->OnEnter(systemProcessContext);
    EXPECT_EQ(notStartedStateHandler->listener_.lock(), nullptr);
}

/**
 * @tc.name: StartedStateHandlerOnEnter001
 * @tc.desc: test StartedStateHandlerOnEnter with listener_ is nullptr
 * @tc.type: FUNC
 * @tc.require: I6KOQV
 */

HWTEST_F(SystemAbilityStateMachineTest, StartedStateHandlerOnEnter001, TestSize.Level3)
{
    std::shared_ptr<SystemProcessContext> systemProcessContext =
        std::make_shared<SystemProcessContext>();
    std::shared_ptr<StateMachineListener> stateListener =
        std::make_shared<StateMachineListener>();
    std::shared_ptr<StartedStateHandler> startedStateHandler =
        std::make_shared<StartedStateHandler>(stateListener);
    stateListener = nullptr;
    startedStateHandler->OnEnter(systemProcessContext);
    EXPECT_EQ(startedStateHandler->listener_.lock(), nullptr);
}

/**
 * @tc.name: StoppingStateHandlerCanEnter001
 * @tc.desc: test StoppingStateHandlerCanEnter with fromState == SystemProcessState::STARTED
 * @tc.type: FUNC
 * @tc.require: I6KOQV
 */

HWTEST_F(SystemAbilityStateMachineTest, StoppingStateHandlerCanEnter001, TestSize.Level3)
{
    std::shared_ptr<StateMachineListener> stateListener =
        std::make_shared<StateMachineListener>();
    std::shared_ptr<StoppingStateHandler> stoppingStateHandler =
        std::make_shared<StoppingStateHandler>(stateListener);
    bool ret = stoppingStateHandler->CanEnter(SystemProcessState::STARTED);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: StoppingStateHandlerOnEnter001
 * @tc.desc: test StoppingStateHandlerOnEnter with listener_ is nullptr
 * @tc.type: FUNC
 * @tc.require: I6KOQV
 */

HWTEST_F(SystemAbilityStateMachineTest, StoppingStateHandlerOnEnter001, TestSize.Level3)
{
    std::shared_ptr<SystemProcessContext> systemProcessContext =
        std::make_shared<SystemProcessContext>();
    std::shared_ptr<StateMachineListener> stateListener =
        std::make_shared<StateMachineListener>();
    std::shared_ptr<StoppingStateHandler> stoppingStateHandler =
        std::make_shared<StoppingStateHandler>(stateListener);
    stateListener = nullptr;
    stoppingStateHandler->OnEnter(systemProcessContext);
    EXPECT_EQ(stoppingStateHandler->listener_.lock(), nullptr);
}
}