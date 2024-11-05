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
#include <vector>

#include "datetime_ex.h"
#include "iremote_object.h"
#include "parameter.h"
#include "refbase.h"
#include "sam_log.h"
#include "sysparam_errno.h"

namespace OHOS {
using namespace std;
class DynamicCache : public IRemoteObject::DeathRecipient {
public:
    void OnRemoteDied(const wptr<IRemoteObject>& remote) override
    {
        HILOGD("DynamicCache OnRemoteDied called");
        ClearCache();
    }
    sptr<IRemoteObject> QueryResult(int32_t querySaId, int32_t code)
    {
        int32_t waterLineLength = 128;
        char waterLine[128] = {0};
        string defaultValue = "default";
        GetParameter(key_.c_str(), defaultValue.c_str(), waterLine, waterLineLength);
        {
            std::lock_guard<std::mutex> autoLock(queryCacheLock_);
            if (CanUseCache(querySaId, waterLine, defaultValue)) {
                HILOGD("DynamicCache QueryResult Return Cache");
                return lastQuerySaProxy_;
            }
        }
        HILOGD("DynamicCache QueryResult Recompute");
        sptr<IRemoteObject> res = Recompute(querySaId, code);
        if (res == nullptr) {
            return nullptr;
        }
        {
            std::lock_guard<std::mutex> autoLock(queryCacheLock_);
            localPara_[key_] = waterLine;
            if (lastQuerySaProxy_ != nullptr) {
                HILOGD("DynamicCache RemoveDeathRecipient");
                lastQuerySaProxy_->RemoveDeathRecipient(this);
            }
            lastQuerySaId_ = querySaId;
            lastQuerySaProxy_ = res;
            res->AddDeathRecipient(this);
        }
        return res;
    }

    bool CanUseCache(int32_t querySaId, char* waterLine, string defaultValue)
    {
        return localPara_.count(key_) != 0 && lastQuerySaId_ == querySaId &&
            defaultValue != string(waterLine) && string(waterLine) == localPara_[key_] &&
            lastQuerySaProxy_ != nullptr && !lastQuerySaProxy_->IsObjectDead();
    }

    __attribute__((no_sanitize("cfi")))
    void ClearCache()
    {
        std::lock_guard<std::mutex> autoLock(queryCacheLock_);
        if (lastQuerySaProxy_ != nullptr) {
            HILOGD("DynamicCache RemoveDeathRecipient");
            lastQuerySaProxy_->RemoveDeathRecipient(this);
        }
        lastQuerySaId_ = -1;
        lastQuerySaProxy_ = nullptr;
    }

    bool InvalidateCache()
    {
        HILOGD("DynamicCache InvalidateCache Begin");
        string tickCount = to_string(GetTickCount());
        int32_t ret = SetParameter(key_.c_str(), tickCount.c_str());
        if (ret != EC_SUCCESS) {
            HILOGE("DynamicCache InvalidateCache SetParameter error:%{public}d!", ret);
            return false;
        }
        HILOGD("DynamicCache InvalidateCache End");
        return true;
    }

    bool SetKey(const string& key)
    {
        int32_t maxLength = 256;
        if (key.size() == 0 || (int32_t)key.size() > maxLength) {
            HILOGE("DynamicCache SetKey size error:%{public}zu!", key.size());
            return false;
        }
        key_ = key;
        return true;
    }

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
    map<string, string> localPara_;
    string key_;
    int32_t lastQuerySaId_;
    sptr<IRemoteObject> lastQuerySaProxy_;
};
}
#endif /* DYNAMIC_CACHE_H */