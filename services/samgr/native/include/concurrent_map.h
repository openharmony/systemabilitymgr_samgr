/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_SAMGR_CONCURRENT_MAP_H
#define OHOS_SAMGR_CONCURRENT_MAP_H

#include <map>
#include <mutex>
#include <functional>

namespace OHOS {
template <typename K, typename V> class ConcurrentMap {
public:
    ConcurrentMap() {}

    ~ConcurrentMap() {}

    ConcurrentMap(const ConcurrentMap &rhs)
    {
        map_ = rhs.map_;
    }

    ConcurrentMap &operator = (const ConcurrentMap &rhs)
    {
        if (&rhs != this) {
            map_ = rhs.map_;
        }

        return *this;
    }

    // when multithread calling size() return a tmp status, some threads may insert just after size() call
    int Size()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return map_.size();
    }

    // when multithread calling Empty() return a tmp status, some threads may insert just after Empty() call
    bool IsEmpty()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return map_.empty();
    }

    bool Insert(const K &key, const V &value)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto ret = map_.insert(std::pair<K, V>(key, value));
        return ret.second;
    }

    void EnsureInsert(const K &key, const V &value)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto ret = map_.insert(std::pair<K, V>(key, value));
        // find key and cannot insert
        if (!ret.second) {
            map_.erase(ret.first);
            map_.insert(std::pair<K, V>(key, value));
            return;
        }
        return;
    }

    bool Find(const K &key, V &value)
    {
        bool ret = false;
        std::lock_guard<std::mutex> lock(mutex_);

        auto iter = map_.find(key);
        if (iter != map_.end()) {
            value = iter->second;
            ret = true;
        }

        return ret;
    }

    void Erase(const K &key)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        map_.erase(key);
    }

    void Clear()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        map_.clear();
        return;
    }

    void Clear(std::function<void(K)> func)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto iter = map_.begin(); iter != map_.end(); iter++) {
            func(iter->first);
        }
        map_.clear();
        return;
    }

private:
    std::mutex mutex_;
    std::map<K, V> map_;
};
} // namespace OHOS
#endif