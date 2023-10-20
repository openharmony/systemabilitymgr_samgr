/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "hisysevent_adapter.h"

#include <string>

#include "def.h"
#include "hisysevent.h"
#include "sam_log.h"

namespace OHOS {
using namespace OHOS::HiviewDFX;
namespace {
const std::string ADD_SYSTEMABILITY_FAIL = "ADD_SYSTEMABILITY_FAIL";
const std::string CALLER_UID = "CALLER_UID";
const std::string SAID = "SAID";
const std::string COUNT = "COUNT";
const std::string FILE_NAME = "FILE_NAME";
const std::string GETSA__TAG = "GETSA_FREQUENCY";

const std::string REASON = "REASON";
const std::string ONDEMAND_SA_LOAD_FAIL = "ONDEMAND_SA_LOAD_FAIL";
const std::string ONDEMAND_SA_LOAD = "ONDEMAND_SA_LOAD";
const std::string EVENT = "EVENT";
const std::string ONDEMAND_SA_UNLOAD = "ONDEMAND_SA_UNLOAD";
}

void ReportSamgrSaLoadFail(int32_t said, const std::string& reason)
{
    int ret = HiSysEventWrite(HiSysEvent::Domain::SAMGR,
        ONDEMAND_SA_LOAD_FAIL,
        HiSysEvent::EventType::FAULT,
        SAID, said,
        REASON, reason);
    if (ret != 0) {
        HILOGE("hisysevent report samgr sa load fail event failed! ret %{public}d.", ret);
    }
}

void ReportSamgrSaLoad(int32_t said, int32_t eventId)
{
    int ret = HiSysEventWrite(HiSysEvent::Domain::SAMGR,
        ONDEMAND_SA_LOAD,
        HiSysEvent::EventType::BEHAVIOR,
        SAID, said,
<<<<<<< HEAD
        EVENT, eventId);
=======
        EVENT_ID, eventId);
>>>>>>> 6ff4d6fb5c0114ebf93075abca9767764c95d788
    if (ret != 0) {
        HILOGE("hisysevent report samgr sa load event failed! ret %{public}d.", ret);
    }
}

void ReportSamgrSaUnload(int32_t said, int32_t eventId)
{
    int ret = HiSysEventWrite(HiSysEvent::Domain::SAMGR,
        ONDEMAND_SA_UNLOAD,
        HiSysEvent::EventType::BEHAVIOR,
        SAID, said,
<<<<<<< HEAD
        EVENT, eventId);
=======
        EVENT_ID, eventId);
>>>>>>> 6ff4d6fb5c0114ebf93075abca9767764c95d788
    if (ret != 0) {
        HILOGE("hisysevent report samgr sa unload event failed! ret %{public}d.", ret);
    }
}

void ReportAddSystemAbilityFailed(int32_t said, const std::string& filaName)
{
    int ret = HiSysEventWrite(HiSysEvent::Domain::SAMGR,
        ADD_SYSTEMABILITY_FAIL,
        HiSysEvent::EventType::FAULT,
        SAID, said,
        FILE_NAME, filaName);
    if (ret != 0) {
        HILOGE("hisysevent report add system ability event failed! ret %{public}d.", ret);
    }
}

void ReportGetSAFrequency(uint32_t callerUid, uint32_t said, int32_t count)
{
    int ret = HiSysEventWrite(HiSysEvent::Domain::SAMGR,
        GETSA__TAG,
        HiSysEvent::EventType::STATISTIC,
        CALLER_UID, callerUid,
        SAID, said,
        COUNT, count);
    if (ret != 0) {
        HILOGE("hisysevent report get sa frequency failed! ret %{public}d.", ret);
    }
}

void WatchDogSendEvent(int32_t pid, uint32_t uid, const std::string& sendMsg,
    const std::string& eventName)
{
    int ret = HiSysEventWrite(HiSysEvent::Domain::SAMGR,
        eventName,
        HiSysEvent::EventType::FAULT,
        "PID", pid,
        "UID", uid,
        "MSG", sendMsg);
    if (ret != 0) {
        HILOGE("hisysevent report watchdog failed! ret %{public}d.", ret);
    }
}
} // OHOS
