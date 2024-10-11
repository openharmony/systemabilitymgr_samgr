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

#ifndef SAMGR_SERVICES_DFX_INCLUDE__HISYSEVENT_ADAPTER_H
#define SAMGR_SERVICES_DFX_INCLUDE__HISYSEVENT_ADAPTER_H

#include <string>
namespace OHOS {
void ReportSaMainExit(const std::string& reason);

void ReportAddSystemAbilityFailed(int32_t said, int32_t pid, int32_t uid, const std::string& filaName);

void ReportGetSAFrequency(uint32_t callerUid, uint32_t said, int32_t count);

void WatchDogSendEvent(int32_t pid, uint32_t uid, const std::string& sendMsg,
    const std::string& eventName);

void ReportSaCrash(int32_t saId);

void ReportSamgrSaLoadFail(int32_t said, int32_t pid, int32_t uid, const std::string& reason);

void ReportSamgrSaLoad(int32_t said, int32_t pid, int32_t uid, int32_t eventId);

void ReportSamgrSaUnload(int32_t said, int32_t pid, int32_t uid, int32_t eventId);

void ReportSaUnLoadFail(int32_t saId, int32_t pid, int32_t uid, const std::string& reason);

void ReportSaLoadDuration(int32_t saId, int32_t keyStage, int64_t duration);

void ReportSaUnLoadDuration(int32_t saId, int32_t keyStage, int64_t duration);

void ReportProcessStartFail(const std::string& processName, int32_t pid, int32_t uid, const std::string& reason);

void ReportProcessStopFail(const std::string& processName, int32_t pid, int32_t uid, const std::string& reason);

void ReportProcessStartDuration(const std::string& processName, int32_t pid, int32_t uid, int64_t duration);

void ReportProcessStopDuration(const std::string& processName, int32_t pid, int32_t uid, int64_t duration);
} // OHOS
#endif // SAMGR_SERVICES_DFX_INCLUDE__HISYSEVENT_ADAPTER_H
