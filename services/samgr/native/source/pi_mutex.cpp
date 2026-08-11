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

#include "pi_mutex.h"
#include <mutex>
#include "system_ability_manager_util.h"

namespace OHOS {
namespace PiMutex {

template<class Mutex>
PiMutex<Mutex>::PiMutex() : mutex_()
{
    if constexpr (!HasType<Mutex>::value ||
                  !std::is_same_v<typename Mutex::native_handle_type, pthread_mutex_t *>) {
        return;
    }
    if (!SamgrUtil::CheckLockPriorityInheritEnable()) {
        return;
    }
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
    pthread_mutex_init(mutex_.native_handle(), &attr);
}
template class PiMutex<std::mutex>;

} // namespace PiMutex
} // namespace OHOS