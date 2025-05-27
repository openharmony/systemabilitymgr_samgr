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

#include "nlohmann/json.hpp"
#include "system_ability_manager.h"
#include "system_ability_manager_util.h"
#include "parameter.h"
#include "parameters.h"
#include "accesstoken_kit.h"
#include "ipc_skeleton.h"
#include "string_ex.h"
#include "tools.h"
#include "sam_log.h"

namespace OHOS {
namespace fs = std::filesystem;
using namespace std;
constexpr int32_t MAX_NAME_SIZE = 200;
constexpr int32_t SPLIT_NAME_VECTOR_SIZE = 2;
constexpr int32_t UID_ROOT = 0;
constexpr int32_t UID_SYSTEM = 1000;
constexpr int32_t SHFIT_BIT = 32;

constexpr const char* EVENT_TYPE = "eventId";
constexpr const char* EVENT_NAME = "name";
constexpr const char* EVENT_VALUE = "value";
constexpr const char* EVENT_EXTRA_DATA_ID = "extraDataId";
constexpr const char* MODULE_UPDATE_PARAM = "persist.samgr.moduleupdate";
constexpr const char* PENG_LAI_PARAM = "ohos.boot.minisys.mode";
constexpr const char* PENG_LAI = "penglai";
constexpr const char* PENGLAI_PATH = "/sys_prod/profile/penglai";
std::shared_ptr<FFRTHandler> SamgrUtil::setParmHandler_ = make_shared<FFRTHandler>("setParmHandler");

bool SamgrUtil::IsNameInValid(const std::u16string& name)
{
    HILOGD("%{public}s called:name = %{public}s", __func__, Str16ToStr8(name).c_str());
    bool ret = false;
    if (name.empty() || name.size() > MAX_NAME_SIZE || DeleteBlank(name).empty()) {
        ret = true;
    }

    return ret;
}

void SamgrUtil::ParseRemoteSaName(const std::u16string& name, std::string& deviceId,
    std::u16string& saName)
{
    vector<string> strVector;
    SplitStr(Str16ToStr8(name), "_", strVector);
    if (strVector.size() == SPLIT_NAME_VECTOR_SIZE) {
        deviceId = strVector[0];
        saName = Str8ToStr16(strVector[1]);
    }
}

bool SamgrUtil::CheckDistributedPermission()
{
    auto callingUid = IPCSkeleton::GetCallingUid();
    if (callingUid != UID_ROOT && callingUid != UID_SYSTEM) {
        return false;
    }
    return true;
}

bool SamgrUtil::IsSameEvent(const OnDemandEvent& event, std::list<OnDemandEvent>& enableOnceList)
{
    for (auto iter = enableOnceList.begin(); iter != enableOnceList.end(); iter++) {
        if (event.eventId == iter->eventId && event.name == iter->name && event.value == iter->value) {
            HILOGI("event already exits in enable-once list");
            return true;
        }
    }
    return false;
}

std::string SamgrUtil::EventToStr(const OnDemandEvent& event)
{
    nlohmann::json eventJson;
    eventJson[EVENT_TYPE] = event.eventId;
    eventJson[EVENT_NAME] = event.name;
    eventJson[EVENT_VALUE] = event.value;
    eventJson[EVENT_EXTRA_DATA_ID] = event.extraDataId;
    std::string eventStr = eventJson.dump();
    return eventStr;
}

std::string SamgrUtil::TransformDeviceId(const std::string& deviceId, int32_t type, bool isPrivate)
{
    return isPrivate ? std::string() : deviceId;
}

bool SamgrUtil::CheckCallerProcess(const CommonSaProfile& saProfile)
{
    if (!CheckCallerProcess(Str16ToStr8(saProfile.process))) {
        HILOGE("can't operate SA: %{public}d by proc:%{public}s",
            saProfile.saId, Str16ToStr8(saProfile.process).c_str());
        return false;
    }
    return true;
}

bool SamgrUtil::CheckCallerProcess(const std::string& callProcess)
{
    uint32_t accessToken = IPCSkeleton::GetCallingTokenID();
    Security::AccessToken::NativeTokenInfo nativeTokenInfo;
    int32_t tokenInfoResult = Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(accessToken, nativeTokenInfo);
    if (tokenInfoResult != ERR_OK) {
        HILOGE("get token info failed");
        return false;
    }

    if (nativeTokenInfo.processName != callProcess) {
        HILOGE("can't operate by proc:%{public}s", nativeTokenInfo.processName.c_str());
        return false;
    }
    return true;
}

bool SamgrUtil::CheckAllowUpdate(OnDemandPolicyType type, const CommonSaProfile& saProfile)
{
    if (type == OnDemandPolicyType::START_POLICY && saProfile.startAllowUpdate) {
        return true;
    } else if (type == OnDemandPolicyType::STOP_POLICY && saProfile.stopAllowUpdate) {
        return true;
    }
    return false;
}

void SamgrUtil::ConvertToOnDemandEvent(const SystemAbilityOnDemandEvent& from, OnDemandEvent& to)
{
    to.eventId = static_cast<int32_t>(from.eventId);
    to.name = from.name;
    to.value = from.value;
    to.persistence = from.persistence;
    for (auto& item : from.conditions) {
        OnDemandCondition condition;
        condition.eventId = static_cast<int32_t>(item.eventId);
        condition.name = item.name;
        condition.value = item.value;
        to.conditions.push_back(condition);
    }
    to.enableOnce = from.enableOnce;
}

void SamgrUtil::ConvertToSystemAbilityOnDemandEvent(const OnDemandEvent& from,
    SystemAbilityOnDemandEvent& to)
{
    to.eventId = static_cast<OnDemandEventId>(from.eventId);
    to.name = from.name;
    to.value = from.value;
    to.persistence = from.persistence;
    for (auto& item : from.conditions) {
        SystemAbilityOnDemandCondition condition;
        condition.eventId = static_cast<OnDemandEventId>(item.eventId);
        condition.name = item.name;
        condition.value = item.value;
        to.conditions.push_back(condition);
    }
    to.enableOnce = from.enableOnce;
}

uint64_t SamgrUtil::GenerateFreKey(int32_t uid, int32_t saId)
{
    uint32_t uSaid = static_cast<uint32_t>(saId);
    uint64_t key = static_cast<uint64_t>(uid);
    return (key << SHFIT_BIT) | uSaid;
}

std::list<int32_t> SamgrUtil::GetCacheCommonEventSa(const OnDemandEvent& event,
    const std::list<SaControlInfo>& saControlList)
{
    std::list<int32_t> saList;
    if (event.eventId != COMMON_EVENT || event.extraDataId == -1) {
        return saList;
    }
    for (auto& item : saControlList) {
        if (item.cacheCommonEvent) {
            saList.emplace_back(item.saId);
        }
    }
    return saList;
}

void SamgrUtil::SetModuleUpdateParam(const std::string& key, const std::string& value)
{
    auto SetParamTask = [=] () {
        int ret = SetParameter(key.c_str(), value.c_str());
        if (ret != 0) {
            HILOGE("SetModuleUpdateParam SetParameter error:%{public}d!", ret);
            return;
        }
    };
    setParmHandler_->PostTask(SetParamTask);
}

void SamgrUtil::SendUpdateSaState(int32_t systemAbilityId, const std::string& updateSaState)
{
    if (SystemAbilityManager::GetInstance()->IsModuleUpdate(systemAbilityId)) {
        std::string startKey = std::string(MODULE_UPDATE_PARAM) + ".start";
        std::string saKey = std::string(MODULE_UPDATE_PARAM) + "." + std::to_string(systemAbilityId);
        SamgrUtil::SetModuleUpdateParam(startKey, "true");
        SamgrUtil::SetModuleUpdateParam(saKey, updateSaState);
    }
}

void SamgrUtil::InvalidateSACache()
{
    auto invalidateCacheTask = [] () {
        SystemAbilityManager::GetInstance()->InvalidateCache();
    };
    setParmHandler_->PostTask(invalidateCacheTask);
}

void SamgrUtil::FilterCommonSaProfile(const SaProfile& oldProfile, CommonSaProfile& newProfile)
{
    newProfile.process = oldProfile.process;
    newProfile.saId = oldProfile.saId;
    newProfile.moduleUpdate = oldProfile.moduleUpdate;
    newProfile.distributed = oldProfile.distributed;
    newProfile.cacheCommonEvent = oldProfile.cacheCommonEvent;
    newProfile.startAllowUpdate = oldProfile.startOnDemand.allowUpdate;
    newProfile.stopAllowUpdate = oldProfile.stopOnDemand.allowUpdate;
    newProfile.recycleStrategy = oldProfile.recycleStrategy;
    newProfile.extension.assign(oldProfile.extension.begin(), oldProfile.extension.end());
}

bool SamgrUtil::CheckPengLai()
{
    std::string defaultValue = "";
    std::string paramValue = system::GetParameter(PENG_LAI_PARAM, defaultValue);
    return paramValue == PENG_LAI;
}

void SamgrUtil::GetFilesByPriority(const std::string& path, std::vector<std::string>& fileNames)
{
    if (SamgrUtil::CheckPengLai()) {
        HILOGI("GetFilesByPriority penglai!");
        GetDirFiles(PENGLAI_PATH, fileNames);
    } else {
        std::map<std::string, std::string> fileNamesMap;
        CfgFiles* filePaths = GetCfgFiles(path.c_str());
        for (int i = 0; filePaths && i < MAX_CFG_POLICY_DIRS_CNT; i++) {
            if (filePaths->paths[i] == nullptr) {
                continue;
            }
            HILOGI("GetFilesByPriority filePaths : %{public}s!", filePaths->paths[i]);
            std::vector<std::string> files;
            GetDirFiles(filePaths->paths[i], files);
            for (const auto& file : files) {
                HILOGD("GetFilesByPriority file : %{public}s!", file.c_str());
                fileNamesMap[fs::path(file).filename().string()] = file;
            }
        }

        for (const auto& pair : fileNamesMap) {
            HILOGD("GetFilesByPriority files : %{public}s!", pair.second.c_str());
            fileNames.push_back(pair.second);
        }

        FreeCfgFiles(filePaths);
    }
}
}
