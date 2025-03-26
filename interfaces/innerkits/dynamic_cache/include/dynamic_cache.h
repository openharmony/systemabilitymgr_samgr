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

#ifndef DYNAMIC_CACHE_H
#define DYNAMIC_CACHE_H

#include <map>
#include <string>
#include "iremote_object.h"

namespace OHOS {
using namespace std;
class DynamicCache : public IRemoteObject::DeathRecipient {
public:

    void OnRemoteDied(const wptr<IRemoteObject>& remote) override
    {
        ClearCache();
    }

    sptr<IRemoteObject> QueryResult(int32_t querySaId, int32_t code);
    bool CanUseCache(int32_t querySaId, char* waterLine, std::string defaultValue);

    void __attribute__((no_sanitize("cfi"))) ClearCache()
    {
        std::lock_guard<std::mutex> autoLock(queryCacheLock_);
        if (lastQuerySaProxy_ != nullptr) {
            lastQuerySaProxy_->RemoveDeathRecipient(this);
        }
        lastQuerySaId_ = -1;
        lastQuerySaProxy_ = nullptr;
    }
    
    bool InvalidateCache();
    bool SetKey(const std::string& key);
    virtual sptr<IRemoteObject> Recompute(int32_t querySaId, int32_t code)
    {
        std::lock_guard<std::mutex> autoLock(queryCacheLock_);
        if (lastQuerySaId_ != querySaId) {
            return nullptr;
        }
        return lastQuerySaProxy_;
    }
    
private:
    std::mutex queryCacheLock_;
    std::map<std::string, std::string> localPara_;
    std::string key_;
    int32_t lastQuerySaId_;
    sptr<IRemoteObject> lastQuerySaProxy_;
};
} // namespace OHOS
#endif /* DYNAMIC_CACHE_H */