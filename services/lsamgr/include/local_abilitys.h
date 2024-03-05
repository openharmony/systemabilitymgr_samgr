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

#ifndef LOCAL_ABILITYS_H_
#define LOCAL_ABILITYS_H_

#include <map>
#include <mutex>
#include <string>
#include "hilog/log.h"
#include "iremote_object.h"

namespace OHOS {
class LocalAbilitys {
public:
    static LocalAbilitys& GetInstance();
    void AddAbility(int32_t systemAbilityId, const sptr<IRemoteObject>& ability);
    sptr<IRemoteObject> GetAbility(int32_t systemAbilityId);
    void RemoveAbility(int32_t systemAbilityId);

private:
    LocalAbilitys() = default;
    ~LocalAbilitys() = default;

    std::mutex localSALock_;
    std::map<int32_t, sptr<IRemoteObject>> localSAMap_;
};
}
#endif // !defined(LOCAL_ABILITYS_H_)
