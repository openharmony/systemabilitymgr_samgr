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

#include "system_ability_event_handler_test.h"
#include "samgr_err_code.h"
#include "test_log.h"
#include <memory>
#define private public
#include "schedule/system_ability_event_handler.h"

using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS;

namespace OHOS {
namespace {}
void StateListener::OnAbilityNotLoadedLocked(int32_t systemAbilityId)
{
}
void StateListener::OnAbilityLoadingLocked(int32_t systemAbilityId)
{
}
void StateListener::OnAbilityLoadedLocked(int32_t systemAbilityId)
{
}
void StateListener::OnAbilityUnloadableLocked(int32_t systemAbilityId)
{
}
void StateListener::OnAbilityUnloadingLocked(int32_t systemAbilityId)
{
}
void StateListener::OnProcessNotStartedLocked(const std::u16string& processName)
{
}
void StateListener::OnProcessStartedLocked(const std::u16string& processName)
{
}
void StateListener::OnProcessStoppingLocked(const std::u16string& processName)
{
}

void SystemAbilityEventHandlerTest::SetUpTestCase()
{
    DTEST_LOG << "SetUpTestCase" << std::endl;
}

void SystemAbilityEventHandlerTest::TearDownTestCase()
{
    DTEST_LOG << "TearDownTestCase" << std::endl;
}

void SystemAbilityEventHandlerTest::SetUp()
{
    DTEST_LOG << "SetUp" << std::endl;
}

void SystemAbilityEventHandlerTest::TearDown()
{
    DTEST_LOG << "TearDown" << std::endl;
}

/**
 * @tc.name: HandleAbilityEventLocked001
 * @tc.desc: test HandleAbilityEventLocked with context is nullptr
 * @tc.type: FUNC
 * @tc.require: I6KOQV
 */

HWTEST_F(SystemAbilityEventHandlerTest, HandleAbilityEventLocked001, TestSize.Level3)
{
    std::shared_ptr<StateListener> stateListener =
        std::make_shared<StateListener>();
    std::shared_ptr<SystemAbilityStateMachine> systemAbilityStateMachine =
        std::make_shared<SystemAbilityStateMachine>(stateListener);
    std::shared_ptr<SystemAbilityEventHandler> systemAbilityEventHandler =
        std::make_shared<SystemAbilityEventHandler>(systemAbilityStateMachine);
    int32_t ret = systemAbilityEventHandler->HandleAbilityEventLocked(nullptr,
        AbilityStateEvent::ABILITY_LOAD_SUCCESS_EVENT);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: HandleAbilityEventLocked002
 * @tc.desc: test HandleAbilityEventLocked with event is in abilityEventHandlerMap_
 * @tc.type: FUNC
 * @tc.require: I6KOQV
 */

HWTEST_F(SystemAbilityEventHandlerTest, HandleAbilityEventLocked002, TestSize.Level3)
{
    std::shared_ptr<StateListener> stateListener =
        std::make_shared<StateListener>();
    std::shared_ptr<SystemAbilityStateMachine> systemAbilityStateMachine =
        std::make_shared<SystemAbilityStateMachine>(stateListener);
    std::shared_ptr<SystemAbilityEventHandler> systemAbilityEventHandler =
        std::make_shared<SystemAbilityEventHandler>(systemAbilityStateMachine);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext =
        std::make_shared<SystemAbilityContext>();
    systemAbilityContext->state = SystemAbilityState::LOADED;
    systemAbilityEventHandler->abilityEventHandlerMap_.clear();
    systemAbilityEventHandler->InitEventHandlerMap();
    int32_t ret = systemAbilityEventHandler->HandleAbilityEventLocked(systemAbilityContext,
        AbilityStateEvent::ABILITY_LOAD_FAILED_EVENT);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: HandleAbilityEventLocked003
 * @tc.desc: test HandleAbilityEventLocked with abilityEventHandlerMap_ is empty
 * @tc.type: FUNC
 * @tc.require: I6KOQV
 */

HWTEST_F(SystemAbilityEventHandlerTest, HandleAbilityEventLocked003, TestSize.Level3)
{
    std::shared_ptr<StateListener> stateListener =
        std::make_shared<StateListener>();
    std::shared_ptr<SystemAbilityStateMachine> systemAbilityStateMachine =
        std::make_shared<SystemAbilityStateMachine>(stateListener);
    std::shared_ptr<SystemAbilityEventHandler> systemAbilityEventHandler =
        std::make_shared<SystemAbilityEventHandler>(systemAbilityStateMachine);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext =
        std::make_shared<SystemAbilityContext>();
    systemAbilityEventHandler->abilityEventHandlerMap_.clear();
    int32_t ret = systemAbilityEventHandler->HandleAbilityEventLocked(systemAbilityContext,
        AbilityStateEvent::ABILITY_LOAD_FAILED_EVENT);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: HandleProcessEventLocked001
 * @tc.desc: test HandleProcessEventLocked with context is nullptr
 * @tc.type: FUNC
 * @tc.require: I6KOQV
 */

HWTEST_F(SystemAbilityEventHandlerTest, HandleProcessEventLocked001, TestSize.Level3)
{
    ProcessInfo processInfo;
    ProcessStateEvent event = ProcessStateEvent::PROCESS_STARTED_EVENT;
    std::shared_ptr<StateListener> stateListener =
        std::make_shared<StateListener>();
    std::shared_ptr<SystemAbilityStateMachine> systemAbilityStateMachine =
        std::make_shared<SystemAbilityStateMachine>(stateListener);
    std::shared_ptr<SystemAbilityEventHandler> systemAbilityEventHandler =
        std::make_shared<SystemAbilityEventHandler>(systemAbilityStateMachine);
    int32_t ret = systemAbilityEventHandler->HandleProcessEventLocked(nullptr,
        processInfo, event);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: HandleProcessEventLocked002
 * @tc.desc: test HandleProcessEventLocked with event is in processEventHandlerMap_
 * @tc.type: FUNC
 * @tc.require: I6KOQV
 */

HWTEST_F(SystemAbilityEventHandlerTest, HandleProcessEventLocked002, TestSize.Level3)
{
    ProcessInfo processInfo;
    ProcessStateEvent event = ProcessStateEvent::PROCESS_STARTED_EVENT;
    std::shared_ptr<StateListener> stateListener =
        std::make_shared<StateListener>();
    std::shared_ptr<SystemAbilityStateMachine> systemAbilityStateMachine =
        std::make_shared<SystemAbilityStateMachine>(stateListener);
    std::shared_ptr<SystemAbilityEventHandler> systemAbilityEventHandler =
        std::make_shared<SystemAbilityEventHandler>(systemAbilityStateMachine);
    std::shared_ptr<SystemProcessContext> systemProcessContext =
        std::make_shared<SystemProcessContext>();
    systemProcessContext->state = SystemProcessState::STARTED;
    systemAbilityEventHandler->processEventHandlerMap_.clear();
    systemAbilityEventHandler->InitEventHandlerMap();
    int32_t ret = systemAbilityEventHandler->HandleProcessEventLocked(systemProcessContext,
        processInfo, event);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: HandleProcessEventLocked003
 * @tc.desc: test HandleProcessEventLocked with processEventHandlerMap_ is empty
 * @tc.type: FUNC
 * @tc.require: I6KOQV
 */

HWTEST_F(SystemAbilityEventHandlerTest, HandleProcessEventLocked003, TestSize.Level3)
{
    ProcessInfo processInfo;
    ProcessStateEvent event = ProcessStateEvent::PROCESS_STARTED_EVENT;
    std::shared_ptr<StateListener> stateListener =
        std::make_shared<StateListener>();
    std::shared_ptr<SystemAbilityStateMachine> systemAbilityStateMachine =
        std::make_shared<SystemAbilityStateMachine>(stateListener);
    std::shared_ptr<SystemAbilityEventHandler> systemAbilityEventHandler =
        std::make_shared<SystemAbilityEventHandler>(systemAbilityStateMachine);
    std::shared_ptr<SystemProcessContext> systemProcessContext =
        std::make_shared<SystemProcessContext>();
    systemAbilityEventHandler->processEventHandlerMap_.clear();
    int32_t ret = systemAbilityEventHandler->HandleProcessEventLocked(systemProcessContext,
        processInfo, event);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: HandleAbilityLoadFailedEventLocked001
 * @tc.desc: test HandleAbilityLoadFailedEventLocked with context's state is LOADING
 * @tc.type: FUNC
 * @tc.require: I6KOQV
 */

HWTEST_F(SystemAbilityEventHandlerTest, HandleAbilityLoadFailedEventLocked001, TestSize.Level3)
{
    std::shared_ptr<StateListener> stateListener =
        std::make_shared<StateListener>();
    std::shared_ptr<SystemAbilityStateMachine> systemAbilityStateMachine =
        std::make_shared<SystemAbilityStateMachine>(stateListener);
    std::shared_ptr<SystemAbilityEventHandler> systemAbilityEventHandler =
        std::make_shared<SystemAbilityEventHandler>(systemAbilityStateMachine);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext =
        std::make_shared<SystemAbilityContext>();
    systemAbilityContext->state = SystemAbilityState::LOADING;
    systemAbilityStateMachine->abilityStateHandlerMap_.clear();
    int32_t ret = systemAbilityEventHandler->HandleAbilityLoadFailedEventLocked(systemAbilityContext);
    EXPECT_EQ(ret, INVALID_SA_NEXT_STATE);
}

/**
 * @tc.name: HandleAbilityLoadFailedEventLocked002
 * @tc.desc: test HandleAbilityLoadFailedEventLocked with context is not PendingEvent::UNLOAD_ABILITY_EVENT
 * @tc.type: FUNC
 * @tc.require: I6KOQV
 */

HWTEST_F(SystemAbilityEventHandlerTest, HandleAbilityLoadFailedEventLocked002, TestSize.Level3)
{
    std::shared_ptr<StateListener> stateListener =
        std::make_shared<StateListener>();
    std::shared_ptr<SystemAbilityStateMachine> systemAbilityStateMachine =
        std::make_shared<SystemAbilityStateMachine>(stateListener);
    std::shared_ptr<SystemAbilityEventHandler> systemAbilityEventHandler =
        std::make_shared<SystemAbilityEventHandler>(systemAbilityStateMachine);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext =
        std::make_shared<SystemAbilityContext>();
    int32_t ret = systemAbilityEventHandler->HandleAbilityLoadFailedEventLocked(systemAbilityContext);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: HandleAbilityLoadFailedEventLocked003
 * @tc.desc: test HandleAbilityLoadFailedEventLocked with context is PendingEvent::UNLOAD_ABILITY_EVENT
 * @tc.type: FUNC
 * @tc.require: I6LQ18
 */

HWTEST_F(SystemAbilityEventHandlerTest, HandleAbilityLoadFailedEventLocked003, TestSize.Level3)
{
    std::shared_ptr<StateListener> stateListener =
        std::make_shared<StateListener>();
    std::shared_ptr<SystemAbilityStateMachine> systemAbilityStateMachine =
        std::make_shared<SystemAbilityStateMachine>(stateListener);
    std::shared_ptr<SystemAbilityEventHandler> systemAbilityEventHandler =
        std::make_shared<SystemAbilityEventHandler>(systemAbilityStateMachine);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext =
        std::make_shared<SystemAbilityContext>();
    systemAbilityContext->pendingEvent = PendingEvent::UNLOAD_ABILITY_EVENT;
    int32_t ret = systemAbilityEventHandler->HandleAbilityLoadFailedEventLocked(systemAbilityContext);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: HandleAbilityLoadSuccessEventLocked001
 * @tc.desc: test HandleAbilityLoadSuccessEventLocked with context's state is LOADED
 * @tc.type: FUNC
 * @tc.require: I6KOQV
 */

HWTEST_F(SystemAbilityEventHandlerTest, HandleAbilityLoadSuccessEventLocked001, TestSize.Level3)
{
    std::shared_ptr<StateListener> stateListener =
        std::make_shared<StateListener>();
    std::shared_ptr<SystemAbilityStateMachine> systemAbilityStateMachine =
        std::make_shared<SystemAbilityStateMachine>(stateListener);
    std::shared_ptr<SystemAbilityEventHandler> systemAbilityEventHandler =
        std::make_shared<SystemAbilityEventHandler>(systemAbilityStateMachine);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext =
        std::make_shared<SystemAbilityContext>();
    systemAbilityContext->state = SystemAbilityState::LOADED;
    int32_t ret = systemAbilityEventHandler->HandleAbilityLoadSuccessEventLocked(systemAbilityContext);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: HandleAbilityUnLoadSuccessEventLocked001
 * @tc.desc: test HandleAbilityUnLoadSuccessEventLocked with context's state is SystemAbilityState::LOADING
 * @tc.type: FUNC
 * @tc.require: I6KOQV
 */

HWTEST_F(SystemAbilityEventHandlerTest, HandleAbilityUnLoadSuccessEventLocked001, TestSize.Level3)
{
    std::shared_ptr<StateListener> stateListener =
        std::make_shared<StateListener>();
    std::shared_ptr<SystemAbilityStateMachine> systemAbilityStateMachine =
        std::make_shared<SystemAbilityStateMachine>(stateListener);
    std::shared_ptr<SystemAbilityEventHandler> systemAbilityEventHandler =
        std::make_shared<SystemAbilityEventHandler>(systemAbilityStateMachine);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext =
        std::make_shared<SystemAbilityContext>();
    systemAbilityContext->state = SystemAbilityState::LOADING;
    systemAbilityStateMachine->abilityStateHandlerMap_.clear();
    int32_t ret = systemAbilityEventHandler->HandleAbilityUnLoadSuccessEventLocked(systemAbilityContext);
    EXPECT_EQ(ret, INVALID_SA_NEXT_STATE);
}

/**
 * @tc.name: HandleAbilityUnLoadSuccessEventLocked002
 * @tc.desc: test HandleAbilityUnLoadSuccessEventLocked with context is PendingEvent::UNLOAD_ABILITY_EVENT
 * @tc.type: FUNC
 * @tc.require: I6LQ18
 */

HWTEST_F(SystemAbilityEventHandlerTest, HandleAbilityUnLoadSuccessEventLocked002, TestSize.Level3)
{
    std::shared_ptr<StateListener> stateListener =
        std::make_shared<StateListener>();
    std::shared_ptr<SystemAbilityStateMachine> systemAbilityStateMachine =
        std::make_shared<SystemAbilityStateMachine>(stateListener);
    std::shared_ptr<SystemAbilityEventHandler> systemAbilityEventHandler =
        std::make_shared<SystemAbilityEventHandler>(systemAbilityStateMachine);
    std::shared_ptr<SystemAbilityContext> systemAbilityContext =
        std::make_shared<SystemAbilityContext>();
    systemAbilityContext->state = SystemAbilityState::LOADING;
    systemAbilityContext->pendingEvent = PendingEvent::UNLOAD_ABILITY_EVENT;
    systemAbilityStateMachine->abilityStateHandlerMap_.clear();
    int32_t ret = systemAbilityEventHandler->HandleAbilityUnLoadSuccessEventLocked(systemAbilityContext);
    EXPECT_EQ(ret, INVALID_SA_NEXT_STATE);
}

/**
 * @tc.name: HandleProcessStartedEventLocked001
 * @tc.desc: test HandleProcessStartedEventLocked with context's state is SystemProcessState::STARTED
 * @tc.type: FUNC
 * @tc.require: I6KOQV
 */

HWTEST_F(SystemAbilityEventHandlerTest, HandleProcessStartedEventLocked001, TestSize.Level3)
{
    ProcessInfo processInfo;
    std::shared_ptr<StateListener> stateListener =
        std::make_shared<StateListener>();
    std::shared_ptr<SystemAbilityStateMachine> systemAbilityStateMachine =
        std::make_shared<SystemAbilityStateMachine>(stateListener);
    std::shared_ptr<SystemAbilityEventHandler> systemAbilityEventHandler =
        std::make_shared<SystemAbilityEventHandler>(systemAbilityStateMachine);
    std::shared_ptr<SystemProcessContext> systemProcessContext =
        std::make_shared<SystemProcessContext>();
    systemProcessContext->state = SystemProcessState::STARTED;
    int32_t ret = systemAbilityEventHandler->HandleProcessStartedEventLocked(systemProcessContext, processInfo);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: HandleProcessStoppedEventLocked001
 * @tc.desc: test HandleProcessStoppedEventLocked with context's state is SystemProcessState::NOT_STARTED
 * @tc.type: FUNC
 * @tc.require: I6KOQV
 */

HWTEST_F(SystemAbilityEventHandlerTest, HandleProcessStoppedEventLocked001, TestSize.Level3)
{
    ProcessInfo processInfo;
    std::shared_ptr<StateListener> stateListener =
        std::make_shared<StateListener>();
    std::shared_ptr<SystemAbilityStateMachine> systemAbilityStateMachine =
        std::make_shared<SystemAbilityStateMachine>(stateListener);
    std::shared_ptr<SystemAbilityEventHandler> systemAbilityEventHandler =
        std::make_shared<SystemAbilityEventHandler>(systemAbilityStateMachine);
    std::shared_ptr<SystemProcessContext> systemProcessContext =
        std::make_shared<SystemProcessContext>();
    systemProcessContext->state = SystemProcessState::NOT_STARTED;
    int32_t ret = systemAbilityEventHandler->HandleProcessStoppedEventLocked(systemProcessContext, processInfo);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

}