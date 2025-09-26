/*
* Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "ffrt_handler.h"

#include <limits>

#include "sam_log.h"

namespace OHOS {
using namespace ffrt;
namespace {
    constexpr uint64_t CONVERSION_FACTOR = 1000; // ms to us
}

FFRTHandler::FFRTHandler(const std::string& name)
{
    queue_ = std::make_shared<queue>(name.c_str());
}

void FFRTHandler::CleanFfrt()
{
    std::unique_lock<samgr::shared_mutex> lock(mutex_);
    if (queue_ == nullptr) {
        return;
    }
    for (auto iter = taskMap_.begin(); iter != taskMap_.end(); ++iter) {
        HILOGI("CleanFfrt taskMap_ %{public}s", iter->first.c_str());
        auto& handlerQueue = iter->second;
        while (!handlerQueue.empty()) {
            auto& handler = handlerQueue.front();
            handlerQueue.pop();
            if (handler == nullptr) {
                continue;
            }
            auto ret = queue_->cancel(handler);
            if (ret != 0) {
                HILOGE("cancel task failed, error code %{public}d", ret);
            }
        }
    }
    taskMap_.clear();
    queue_.reset();
}

void FFRTHandler::SetFfrt(const std::string& name)
{
    std::unique_lock<samgr::shared_mutex> lock(mutex_);
    queue_ = std::make_shared<queue>(name.c_str());
}

bool FFRTHandler::PostTask(std::function<void()> func)
{
    if (!func) {
        HILOGE("FFRTHandler post task failed, func is null");
        return false;
    }
    std::unique_lock<samgr::shared_mutex> lock(mutex_);
    if (queue_ == nullptr) {
        return false;
    }
    task_handle handler = queue_->submit_h(func);
    if (handler == nullptr) {
        HILOGE("FFRTHandler post task failed");
        return false;
    }
    return true;
}

bool FFRTHandler::PostTask(std::function<void()> func, uint64_t delayTime)
{
    if (!func) {
        HILOGE("FFRTHandler post delay task failed, func is null");
        return false;
    }
    if (delayTime > std::numeric_limits<uint64_t>::max() / CONVERSION_FACTOR) {
        HILOGE("invalid delay time");
        return false;
    }
    std::unique_lock<samgr::shared_mutex> lock(mutex_);
    if (queue_ == nullptr) {
        return false;
    }
    task_handle handler = queue_->submit_h(func, task_attr().delay(delayTime * CONVERSION_FACTOR));
    if (handler == nullptr) {
        HILOGE("FFRTHandler post task failed");
        return false;
    }
    return true;
}

bool FFRTHandler::PostTask(std::function<void()> func, const std::string& name, uint64_t delayTime)
{
    if (!func) {
        HILOGE("FFRTHandler post delay task with name failed, func is null");
        return false;
    }
    if (delayTime > std::numeric_limits<uint64_t>::max() / CONVERSION_FACTOR) {
        HILOGE("invalid delay time");
        return false;
    }
    std::unique_lock<samgr::shared_mutex> lock(mutex_);
    if (queue_ == nullptr) {
        return false;
    }
    task_handle handler = queue_->submit_h(func, task_attr().delay(delayTime * CONVERSION_FACTOR));
    if (handler == nullptr) {
        HILOGE("FFRTHandler post task failed");
        return false;
    }
    auto& handlerQueue = taskMap_[name];
    handlerQueue.push(std::move(handler));
    return true;
}

void FFRTHandler::RemoveTask(const std::string& name)
{
    std::unique_lock<samgr::shared_mutex> lock(mutex_);
    auto item = taskMap_.find(name);
    if (item == taskMap_.end()) {
        HILOGW("rm task %{public}s NF", name.c_str());
        return;
    }
    if (queue_ == nullptr) {
        return;
    }
    auto& handlerQueue = item->second;
    while (!handlerQueue.empty()) {
        auto& handler = handlerQueue.front();
        if (handler != nullptr) {
            auto ret = queue_->cancel(handler);
            if (ret != 0) {
                HILOGE("cancel task failed, error code %{public}d", ret);
            }
            handler = nullptr;
        }
        handlerQueue.pop();
    }
    taskMap_.erase(name);
}

void FFRTHandler::DelTask(const std::string& name)
{
    std::unique_lock<samgr::shared_mutex> lock(mutex_);
    auto item = taskMap_.find(name);
    if (item == taskMap_.end()) {
        HILOGW("del task %{public}s NF", name.c_str());
        return;
    }
    auto& handlerQueue = item->second;
    if (!handlerQueue.empty()) {
        handlerQueue.pop();
    }
    if (handlerQueue.empty()) {
        HILOGD("erase task %{public}s ", name.c_str());
        taskMap_.erase(name);
    }
}

bool FFRTHandler::HasInnerEvent(const std::string name)
{
    std::unique_lock<samgr::shared_mutex> lock(mutex_);
    auto item = taskMap_.find(name);
    if (item == taskMap_.end()) {
        return false;
    }
    return true;
}
} // namespace OHOS