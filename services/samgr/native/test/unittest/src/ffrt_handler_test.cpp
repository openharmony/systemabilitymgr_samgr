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

#include "ffrt_handler_test.h"

#include <chrono>
#include <functional>
#include <limits>
#include <string>
#include <thread>

#include "test_log.h"

#define private public
#include "ffrt_handler.h"

using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS;

namespace OHOS {
namespace {
    constexpr int32_t MAX_WAIT_TIME = 2000;
    constexpr uint64_t TEST_DELAY_TIME = 100; // ms
    const std::string TEST_TASK_NAME = "test_task";
}
bool FFRTHandlerTest::isCaseDone_ = false;
std::mutex FFRTHandlerTest::caseDoneLock_;
std::condition_variable FFRTHandlerTest::caseDoneCondition_;

void FFRTHandlerTest::SetUpTestCase()
{
    DTEST_LOG << "SetUpTestCase" << std::endl;
}

void FFRTHandlerTest::TearDownTestCase()
{
    DTEST_LOG << "TearDownTestCase" << std::endl;
}

void FFRTHandlerTest::SetUp()
{
    DTEST_LOG << "SetUp" << std::endl;
}

void FFRTHandlerTest::TearDown()
{
    DTEST_LOG << "TearDown" << std::endl;
}

static void WaitCaseDone()
{
    std::unique_lock<std::mutex> lock(FFRTHandlerTest::caseDoneLock_);
    FFRTHandlerTest::caseDoneCondition_.wait_for(lock, std::chrono::milliseconds(MAX_WAIT_TIME),
        [&] () { return FFRTHandlerTest::isCaseDone_; });
    FFRTHandlerTest::isCaseDone_ = false;
}

static void NotifyCaseDone()
{
    std::lock_guard<std::mutex> autoLock(FFRTHandlerTest::caseDoneLock_);
    FFRTHandlerTest::isCaseDone_ = true;
    FFRTHandlerTest::caseDoneCondition_.notify_all();
}

/**
 * @tc.name: PostTask001
 * @tc.desc: test PostTask with valid function
 * @tc.type: FUNC
 */
HWTEST_F(FFRTHandlerTest, PostTask001, TestSize.Level1)
{
    FFRTHandler handler("PostTask001");
    auto doneTask = []() { NotifyCaseDone(); };
    bool ret = handler.PostTask(doneTask);
    EXPECT_TRUE(ret);
    WaitCaseDone();
    handler.CleanFfrt();
}

/**
 * @tc.name: PostTask002
 * @tc.desc: test PostTask with null function
 * @tc.type: FUNC
 */
HWTEST_F(FFRTHandlerTest, PostTask002, TestSize.Level1)
{
    FFRTHandler handler("PostTask002");
    std::function<void()> nullTask = nullptr;
    bool ret = handler.PostTask(nullTask);
    EXPECT_FALSE(ret);
    handler.CleanFfrt();
}

/**
 * @tc.name: PostTask003
 * @tc.desc: test PostTask when queue_ is nullptr (after CleanFfrt)
 * @tc.type: FUNC
 */
HWTEST_F(FFRTHandlerTest, PostTask003, TestSize.Level1)
{
    FFRTHandler handler("PostTask003");
    handler.CleanFfrt();
    EXPECT_EQ(handler.queue_, nullptr);
    auto doneTask = []() { NotifyCaseDone(); };
    bool ret = handler.PostTask(doneTask);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: PostTaskDelay001
 * @tc.desc: test PostTask with valid function and delay
 * @tc.type: FUNC
 */
HWTEST_F(FFRTHandlerTest, PostTaskDelay001, TestSize.Level1)
{
    FFRTHandler handler("PostTaskDelay001");
    auto doneTask = []() { NotifyCaseDone(); };
    bool ret = handler.PostTask(doneTask, TEST_DELAY_TIME);
    EXPECT_TRUE(ret);
    WaitCaseDone();
    handler.CleanFfrt();
}

/**
 * @tc.name: PostTaskDelay002
 * @tc.desc: test PostTask with null function and delay
 * @tc.type: FUNC
 */
HWTEST_F(FFRTHandlerTest, PostTaskDelay002, TestSize.Level1)
{
    FFRTHandler handler("PostTaskDelay002");
    std::function<void()> nullTask = nullptr;
    bool ret = handler.PostTask(nullTask, TEST_DELAY_TIME);
    EXPECT_FALSE(ret);
    handler.CleanFfrt();
}

/**
 * @tc.name: PostTaskDelay003
 * @tc.desc: test PostTask with invalid delay time (overflow)
 * @tc.type: FUNC
 */
HWTEST_F(FFRTHandlerTest, PostTaskDelay003, TestSize.Level1)
{
    FFRTHandler handler("PostTaskDelay003");
    auto doneTask = []() { NotifyCaseDone(); };
    uint64_t invalidDelay = (std::numeric_limits<uint64_t>::max() / 1000) + 1;
    bool ret = handler.PostTask(doneTask, invalidDelay);
    EXPECT_FALSE(ret);
    handler.CleanFfrt();
}

/**
 * @tc.name: PostTaskDelay004
 * @tc.desc: test PostTask with delay when queue_ is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(FFRTHandlerTest, PostTaskDelay004, TestSize.Level1)
{
    FFRTHandler handler("PostTaskDelay004");
    handler.CleanFfrt();
    EXPECT_EQ(handler.queue_, nullptr);
    auto doneTask = []() { NotifyCaseDone(); };
    bool ret = handler.PostTask(doneTask, TEST_DELAY_TIME);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: PostTaskName001
 * @tc.desc: test PostTask with valid function, name and delay
 * @tc.type: FUNC
 */
HWTEST_F(FFRTHandlerTest, PostTaskName001, TestSize.Level1)
{
    FFRTHandler handler("PostTaskName001");
    auto doneTask = []() { NotifyCaseDone(); };
    bool ret = handler.PostTask(doneTask, TEST_TASK_NAME, TEST_DELAY_TIME);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(handler.HasInnerEvent(TEST_TASK_NAME));
    WaitCaseDone();
    handler.CleanFfrt();
}

/**
 * @tc.name: PostTaskName002
 * @tc.desc: test PostTask with null function, name and delay
 * @tc.type: FUNC
 */
HWTEST_F(FFRTHandlerTest, PostTaskName002, TestSize.Level1)
{
    FFRTHandler handler("PostTaskName002");
    std::function<void()> nullTask = nullptr;
    bool ret = handler.PostTask(nullTask, TEST_TASK_NAME, TEST_DELAY_TIME);
    EXPECT_FALSE(ret);
    handler.CleanFfrt();
}

/**
 * @tc.name: PostTaskName003
 * @tc.desc: test PostTask with name and invalid delay time
 * @tc.type: FUNC
 */
HWTEST_F(FFRTHandlerTest, PostTaskName003, TestSize.Level1)
{
    FFRTHandler handler("PostTaskName003");
    auto doneTask = []() { NotifyCaseDone(); };
    uint64_t invalidDelay = (std::numeric_limits<uint64_t>::max() / 1000) + 1;
    bool ret = handler.PostTask(doneTask, TEST_TASK_NAME, invalidDelay);
    EXPECT_FALSE(ret);
    handler.CleanFfrt();
}

/**
 * @tc.name: PostTaskName004
 * @tc.desc: test PostTask with name when queue_ is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(FFRTHandlerTest, PostTaskName004, TestSize.Level1)
{
    FFRTHandler handler("PostTaskName004");
    handler.CleanFfrt();
    EXPECT_EQ(handler.queue_, nullptr);
    auto doneTask = []() { NotifyCaseDone(); };
    bool ret = handler.PostTask(doneTask, TEST_TASK_NAME, TEST_DELAY_TIME);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: CleanFfrt001
 * @tc.desc: test CleanFfrt sets queue_ to nullptr (lock-scope change)
 * @tc.type: FUNC
 */
HWTEST_F(FFRTHandlerTest, CleanFfrt001, TestSize.Level1)
{
    FFRTHandler handler("CleanFfrt001");
    EXPECT_NE(handler.queue_, nullptr);
    handler.CleanFfrt();
    EXPECT_EQ(handler.queue_, nullptr);
}

/**
 * @tc.name: CleanFfrt002
 * @tc.desc: test CleanFfrt idempotent (double clean does not crash)
 * @tc.type: FUNC
 */
HWTEST_F(FFRTHandlerTest, CleanFfrt002, TestSize.Level1)
{
    FFRTHandler handler("CleanFfrt002");
    handler.CleanFfrt();
    EXPECT_EQ(handler.queue_, nullptr);
    handler.CleanFfrt();
    EXPECT_EQ(handler.queue_, nullptr);
}

/**
 * @tc.name: CleanFfrt003
 * @tc.desc: test CleanFfrt clears taskMap_
 * @tc.type: FUNC
 */
HWTEST_F(FFRTHandlerTest, CleanFfrt003, TestSize.Level1)
{
    FFRTHandler handler("CleanFfrt003");
    auto doneTask = []() { NotifyCaseDone(); };
    handler.PostTask(doneTask, TEST_TASK_NAME, TEST_DELAY_TIME);
    EXPECT_FALSE(handler.taskMap_.empty());
    handler.CleanFfrt();
    EXPECT_TRUE(handler.taskMap_.empty());
    EXPECT_EQ(handler.queue_, nullptr);
}

/**
 * @tc.name: SetFfrt001
 * @tc.desc: test SetFfrt destroys old queue and creates new one
 * @tc.type: FUNC
 */
HWTEST_F(FFRTHandlerTest, SetFfrt001, TestSize.Level1)
{
    FFRTHandler handler("SetFfrt001_old");
    ffrt_queue_t oldQueue = handler.queue_;
    EXPECT_NE(oldQueue, nullptr);
    handler.SetFfrt("SetFfrt001_new");
    EXPECT_NE(handler.queue_, nullptr);
    EXPECT_NE(handler.queue_, oldQueue);
    handler.CleanFfrt();
}

/**
 * @tc.name: SetFfrt002
 * @tc.desc: test SetFfrt clears taskMap_ before creating new queue
 * @tc.type: FUNC
 */
HWTEST_F(FFRTHandlerTest, SetFfrt002, TestSize.Level1)
{
    FFRTHandler handler("SetFfrt002");
    auto doneTask = []() { NotifyCaseDone(); };
    handler.PostTask(doneTask, TEST_TASK_NAME, TEST_DELAY_TIME);
    EXPECT_FALSE(handler.taskMap_.empty());
    handler.SetFfrt("SetFfrt002_new");
    EXPECT_TRUE(handler.taskMap_.empty());
    EXPECT_NE(handler.queue_, nullptr);
    handler.CleanFfrt();
}

/**
 * @tc.name: SetFfrt003
 * @tc.desc: test SetFfrt after CleanFfrt (queue_ is nullptr, skip destroy)
 * @tc.type: FUNC
 */
HWTEST_F(FFRTHandlerTest, SetFfrt003, TestSize.Level1)
{
    FFRTHandler handler("SetFfrt003");
    handler.CleanFfrt();
    EXPECT_EQ(handler.queue_, nullptr);
    handler.SetFfrt("SetFfrt003_new");
    EXPECT_NE(handler.queue_, nullptr);
    handler.CleanFfrt();
}

/**
 * @tc.name: Destructor001
 * @tc.desc: test destructor calls CleanFfrt, queue_ destroyed safely
 * @tc.type: FUNC
 */
HWTEST_F(FFRTHandlerTest, Destructor001, TestSize.Level1)
{
    ffrt_queue_t queueHandle = nullptr;
    {
        FFRTHandler handler("Destructor001");
        EXPECT_NE(handler.queue_, nullptr);
        queueHandle = handler.queue_;
    }
    EXPECT_NE(queueHandle, nullptr);
}

/**
 * @tc.name: Destructor002
 * @tc.desc: test destructor is safe after CleanFfrt (queue_ already nullptr, idempotent)
 * @tc.type: FUNC
 */
HWTEST_F(FFRTHandlerTest, Destructor002, TestSize.Level1)
{
    FFRTHandler* handler = new FFRTHandler("Destructor002");
    handler->CleanFfrt();
    EXPECT_EQ(handler->queue_, nullptr);
    delete handler;
}

/**
 * @tc.name: RemoveTask001
 * @tc.desc: test RemoveTask removes existing named task
 * @tc.type: FUNC
 */
HWTEST_F(FFRTHandlerTest, RemoveTask001, TestSize.Level1)
{
    FFRTHandler handler("RemoveTask001");
    auto pendingTask = []() { NotifyCaseDone(); };
    handler.PostTask(pendingTask, TEST_TASK_NAME, MAX_WAIT_TIME * 10);
    EXPECT_TRUE(handler.HasInnerEvent(TEST_TASK_NAME));
    handler.RemoveTask(TEST_TASK_NAME);
    EXPECT_FALSE(handler.HasInnerEvent(TEST_TASK_NAME));
    handler.CleanFfrt();
}

/**
 * @tc.name: RemoveTask002
 * @tc.desc: test RemoveTask with non-existing name
 * @tc.type: FUNC
 */
HWTEST_F(FFRTHandlerTest, RemoveTask002, TestSize.Level1)
{
    FFRTHandler handler("RemoveTask002");
    handler.RemoveTask("non_existing_task");
    EXPECT_FALSE(handler.HasInnerEvent("non_existing_task"));
    handler.CleanFfrt();
}

/**
 * @tc.name: DelTask001
 * @tc.desc: test DelTask with existing name
 * @tc.type: FUNC
 */
HWTEST_F(FFRTHandlerTest, DelTask001, TestSize.Level1)
{
    FFRTHandler handler("DelTask001");
    auto pendingTask = []() { NotifyCaseDone(); };
    handler.PostTask(pendingTask, TEST_TASK_NAME, MAX_WAIT_TIME * 10);
    EXPECT_TRUE(handler.HasInnerEvent(TEST_TASK_NAME));
    handler.DelTask(TEST_TASK_NAME);
    handler.CleanFfrt();
}

/**
 * @tc.name: DelTask002
 * @tc.desc: test DelTask with non-existing name
 * @tc.type: FUNC
 */
HWTEST_F(FFRTHandlerTest, DelTask002, TestSize.Level1)
{
    FFRTHandler handler("DelTask002");
    handler.DelTask("non_existing_task");
    EXPECT_FALSE(handler.HasInnerEvent("non_existing_task"));
    handler.CleanFfrt();
}

/**
 * @tc.name: HasInnerEvent001
 * @tc.desc: test HasInnerEvent returns true for existing task
 * @tc.type: FUNC
 */
HWTEST_F(FFRTHandlerTest, HasInnerEvent001, TestSize.Level1)
{
    FFRTHandler handler("HasInnerEvent001");
    auto pendingTask = []() { NotifyCaseDone(); };
    handler.PostTask(pendingTask, TEST_TASK_NAME, MAX_WAIT_TIME * 10);
    EXPECT_TRUE(handler.HasInnerEvent(TEST_TASK_NAME));
    handler.CleanFfrt();
}

/**
 * @tc.name: HasInnerEvent002
 * @tc.desc: test HasInnerEvent returns false for non-existing task
 * @tc.type: FUNC
 */
HWTEST_F(FFRTHandlerTest, HasInnerEvent002, TestSize.Level1)
{
    FFRTHandler handler("HasInnerEvent002");
    EXPECT_FALSE(handler.HasInnerEvent("non_existing_task"));
    handler.CleanFfrt();
}
} // namespace OHOS
