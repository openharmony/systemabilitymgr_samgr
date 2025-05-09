/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "errors.h"
#include "ipc_skeleton.h"
#include "memory_guard.h"
#include "parameter.h"
#include "refbase.h"
#include "sam_log.h"
#include "system_ability_manager.h"
#include "samgr_xcollie.h"

using namespace OHOS;

int main(int argc, char *argv[])
{
    KHILOGI("%{public}s called, enter System Ability Manager ", __func__);
    Samgr::MemoryGuard cacheGuard;
    OHOS::sptr<OHOS::SystemAbilityManager> manager = OHOS::SystemAbilityManager::GetInstance();
    manager->Init();
    KHILOGI("System Ability Manager enter init");
    OHOS::sptr<OHOS::IRemoteObject> serv = manager->AsObject();

    {
        SamgrXCollie samgrXCollie("samgr--main_setContextObject");
        if (!IPCSkeleton::SetContextObject(serv)) {
            KHILOGE("set context fail!");
        }
    }

    manager->AddSamgrToAbilityMap();
    int result = SetParameter("bootevent.samgr.ready", "true");
    KHILOGI("set samgr ready ret : %{public}s", result == 0 ? "succeed" : "failed");
    manager->StartDfxTimer();
    OHOS::IPCSkeleton::JoinWorkThread();
    KHILOGI("JoinWorkThread error, samgr main exit!");
    return -1;
}