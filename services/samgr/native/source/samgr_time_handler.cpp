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

#include "samgr_time_handler.h"
#include "sam_log.h"
#include <cinttypes>

using namespace std;
namespace OHOS {
namespace {
constexpr uint32_t INIT_NUM = 4;
constexpr uint32_t MAX_EVENT = 8;
constexpr int32_t RETRY_TIMES = 3;
}
SamgrTimeHandler* volatile SamgrTimeHandler::singleton = nullptr;
SamgrTimeHandler::Deletor SamgrTimeHandler::deletor;
static mutex mtx;

SamgrTimeHandler* SamgrTimeHandler::GetInstance()
{
    if (singleton == nullptr) {
        lock_guard<mutex> autoLock(mtx);
        if (singleton == nullptr) {
            singleton = new SamgrTimeHandler;
        }
    }
    return singleton;
}

SamgrTimeHandler::SamgrTimeHandler()
{
    HILOGI("SamgrTimeHandler init start");
    epollfd = epoll_create(INIT_NUM);
    if (epollfd == -1) {
        HILOGE("SamgrTimeHandler epoll_create error");
    }
    flag = false;
    StartThread();
}

void SamgrTimeHandler::StartThread()
{
    std::function<void()> func = [this]() {
        HILOGI("SamgrTimeHandler thread start");
        struct epoll_event events[MAX_EVENT];
        while (!this->timeFunc.IsEmpty()) {
            int number = epoll_wait(this->epollfd, events, MAX_EVENT, -1);
            OnTime((*this), number, events);
        }
        this->flag = false;
        HILOGI("SamgrTimeHandler thread end");
    };
    std::thread t(func);
    this->flag = true;
    t.detach();
}

void SamgrTimeHandler::OnTime(SamgrTimeHandler &handle, int number, struct epoll_event events[])
{
    if (number > 0) {
        HILOGI("SamgrTimeHandler OnTime: %{public}d", number);
    }
    for (int i = 0; i < number; i++) {
        uint32_t timerfd = events[i].data.u32;
        uint64_t unused = 0;
        HILOGI("SamgrTimeHandler timerfd: %{public}u", timerfd);
        int ret = read(timerfd, &unused, sizeof(unused));
        if (ret == sizeof(uint64_t)) {
            TaskType funcTime;
            bool hasFunction = handle.timeFunc.Find(timerfd, funcTime);
            HILOGI("SamgrTimeHandler hasFunction: %{public}d", hasFunction);
            funcTime();
            handle.timeFunc.Erase(timerfd);
            epoll_ctl(this->epollfd, EPOLL_CTL_DEL, timerfd, nullptr);
            ::close(timerfd);
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
}

int SamgrTimeHandler::CreateAndRetry()
{
    for (int32_t i = 0; i < RETRY_TIMES; i++) {
        int timerfd = timerfd_create(CLOCK_BOOTTIME_ALARM, 0);
        if (timerfd != -1) {
            return timerfd;
        }
        HILOGE("timerfd_create set alarm err: %{public}s", strerror(errno));
    }
    return -1;
}

bool SamgrTimeHandler::PostTask(TaskType func, uint64_t delayTime)
{
    HILOGI("SamgrTimeHandler postTask start: %{public}" PRId64 "s", delayTime);
    int timerfd = CreateAndRetry();
    if (timerfd == -1) {
        timerfd = timerfd_create(CLOCK_MONOTONIC, 0);
        if (timerfd == -1) {
            HILOGE("timerfd_create fail : %{public}s", strerror(errno));
            return false;
        }
    }
    epoll_event event {};
    event.events = EPOLLIN | EPOLLWAKEUP;
    event.data.u32 = static_cast<uint32_t>(timerfd);
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, timerfd, &event) == -1) {
        HILOGE("epoll_ctl(EPOLL_CTL_ADD) failed : %{public}s", strerror(errno));
        ::close(timerfd);
        return false;
    }
    struct itimerspec newValue = {};
    newValue.it_value.tv_sec = static_cast<int64_t>(delayTime);
    newValue.it_value.tv_nsec = 0;
    newValue.it_interval.tv_sec = 0;
    newValue.it_interval.tv_nsec = 0;

    if (timerfd_settime(timerfd, 0, &newValue, NULL) == -1) {
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