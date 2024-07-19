/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "samgr_time_handler.h"
#include "sam_log.h"

namespace OHOS {
namespace {
constexpr uint64_t CONVERSION_FACTOR = 1000; // ms to us
}
SamgrTimeHandler* volatile SamgrTimeHandler::singleton = nullptr;
static std::recursive_mutex mtx;

SamgrTimeHandler* SamgrTimeHandler::GetInstance()
{
    if (singleton == nullptr) {
        mtx.lock();
        if (singleton == nullptr) {
            singleton = new SamgrTimeHandler;
        }
        mtx.unlock();
    }
    return singleton;
}

SamgrTimeHandler::SamgrTimeHandler()
{
    HILOGI("SamgrTimeHandler init start");
    epollfd = epoll_create(INIT_NUM);
    StartThread();
}

void SamgrTimeHandler::StartThread()
{
    std::function<void()> func = [this]() {
        struct epoll_event events[this->MAX_EVENT];
        int number;
        while (!this->timeFunc.IsEmpty()) {
            number = epoll_wait(this->epollfd, events, this->MAX_EVENT, -1);
            OnTime((*this), number, events);
        }
        this->flag = false;
    };
    std::thread t(func);
    this->flag = true;
    t.detach();
}

void SamgrTimeHandler::OnTime(SamgrTimeHandler &handle, int number, struct epoll_event events[])
{
    for (int i = 0; i < number; i++) {
        uint32_t timerfd = events[i].data.u32;
        uint64_t unused;
        int ret = read(timerfd, &unused, sizeof(unused));
        if (ret == sizeof(uint64_t)) {
            TaskType func_time;
            handle.timeFunc.Find(timerfd, func_time);
            func_time();
            handle.timeFunc.Erase(timerfd);
            epoll_ctl(this->epollfd, EPOLL_CTL_DEL, timerfd, nullptr);
        }
    }
}

SamgrTimeHandler::~SamgrTimeHandler()
{
    auto closeFunc = [this](uint32_t fd) {
        epoll_ctl(this->epollfd, EPOLL_CTL_DEL, fd, nullptr);
        ::close(fd);
    };
    timeFunc.Clear(closeFunc);
    ::close(epollfd);
    if (singleton != nullptr) {
        delete singleton;
    }
}


bool SamgrTimeHandler::PostTask(TaskType func, uint64_t delayTime)
{
    HILOGD("SamgrTimeHandler postTask start: %{public}d", (int)delayTime);
    int timerfd = timerfd_create(CLOCK_BOOTTIME_ALARM, 0);
    if (timerfd == -1) {
        HILOGD("timerfd_create CLOCK_BOOTTIME_ALARM not support: %{public}s", strerror(errno));
        timerfd = timerfd_create(CLOCK_MONOTONIC, 0);
        if (timerfd == -1) {
            HILOGE("timerfd_create fail : %{public}s", strerror(errno));
            return false;
        }
    }
    epoll_event event {};
    event.events = EPOLLIN | EPOLLWAKEUP;
    event.data.u32 = timerfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, timerfd, &event) == -1) {
        HILOGE("epoll_ctl(EPOLL_CTL_ADD) failed : %{public}s", strerror(errno));
        ::close(timerfd);
        return false;
    }
    struct itimerspec new_value = {};
    new_value.it_value.tv_sec = delayTime;
    new_value.it_value.tv_nsec = 0;
    new_value.it_interval.tv_sec = 0;
    new_value.it_interval.tv_nsec = 0;

    if (timerfd_settime(timerfd, 0, &new_value, NULL) == -1) {
        HILOGE("timerfd_settime failed : %{public}s", strerror(errno));
        ::close(timerfd);
        return false;
    }
    timeFunc.EnsureInsert(timerfd, func);
    if (!flag) {
        StartThread();
    }
    return true;
}
} // namespace OHOS