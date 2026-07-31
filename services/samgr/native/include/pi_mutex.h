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

#ifndef SAMGR_PI_MUTEX_H
#define SAMGR_PI_MUTEX_H

#include <pthread.h>
#include <type_traits>
#include <mutex>

namespace OHOS {
namespace PiMutex {

template<class, class = std::void_t<>>
struct HasType : std::false_type {};

template<class T>
struct HasType<T, std::void_t<typename T::native_handle_type>> : std::true_type {};

template<class Mutex>
class PiMutex {
public:
    PiMutex();

    PiMutex(const PiMutex&) = delete;
    PiMutex& operator=(const PiMutex&) = delete;

    void lock() { mutex_.lock(); }
    void unlock() { mutex_.unlock(); }
    bool try_lock() { return mutex_.try_lock(); }
    auto native_handle() { return mutex_.native_handle(); }

private:
    Mutex mutex_;
};

extern template class PiMutex<std::mutex>;

} // namespace PiMutex
} // namespace OHOS

#endif // SAMGR_PI_MUTEX_H