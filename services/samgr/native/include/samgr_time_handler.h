/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_SAMGR_TIME_HANDLER_H
#define OHOS_SAMGR_TIME_HANDLER_H

#include <thread>
#include <string>
#include <functional>
#include <sys/epoll.h>
#include <signal.h>
#include <sys/timerfd.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#include <mutex>
#include <concurrent_map.h>
#include <atomic>
namespace OHOS {
class SamgrTimeHandler {
public:
    typedef std::function<void()> TaskType;
    ~SamgrTimeHandler();
    static SamgrTimeHandler* GetInstance();
    bool PostTask(TaskType func, uint64_t delayTime);

private:
    SamgrTimeHandler();
    class Deletor {
    public:
        ~Deletor()
        {
            if (nullptr != SamgrTimeHandler::singleton) {
                delete SamgrTimeHandler::singleton;
                SamgrTimeHandler::singleton = nullptr;
            }
        }
    };
    void StartThread();
    void OnTime(SamgrTimeHandler &handle, int number, struct epoll_event events[]);
    int CreateAndRetry();

private:
    int epollfd = -1;
    std::atomic<bool> flag = false;
    ConcurrentMap <uint32_t, TaskType> timeFunc;
    static SamgrTimeHandler* volatile singleton;
    static Deletor deletor;
};
} // namespace OHOS
#endif // OHOS_SAMGR_TIME_HANDLER_H