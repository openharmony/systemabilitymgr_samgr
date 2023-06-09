/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "parse_util.h"

#include <cinttypes>
#include <dlfcn.h>
#include <fstream>
#include <memory>
#include <sstream>
#include <vector>

#include "datetime_ex.h"
#include "hisysevent_adapter.h"
#include "hitrace_meter.h"
#include "sam_log.h"
#include "string_ex.h"

namespace OHOS {
using std::string;

namespace {
constexpr const char* EVENT_TYPE = "eventId";
constexpr const char* EVENT_NAME = "name";
constexpr const char* EVENT_VALUE = "value";
constexpr const char* SA_TAG_SYSTEM_ABILITY = "systemability";
constexpr const char* SA_TAG_PROCESS = "process";
constexpr const char* SA_TAG_LIB_PATH = "libpath";
constexpr const char* SA_TAG_NAME = "name";
constexpr const char* SA_TAG_DEPEND = "depend";
constexpr const char* SA_TAG_DEPEND_TIMEOUT = "depend-time-out";
constexpr const char* SA_TAG_RUN_ON_CREATE = "run-on-create";
constexpr const char* SA_TAG_AUTO_RESTART = "auto-restart";
constexpr const char* SA_TAG_DISTRIBUTED = "distributed";
constexpr const char* SA_TAG_DUMP_LEVEL = "dump-level";
constexpr const char* SA_TAG_CAPABILITY = "capability";
constexpr const char* SA_TAG_PERMISSION = "permission";
constexpr const char* SA_TAG_BOOT_PHASE = "bootphase";
constexpr const char* SA_TAG_SAID = "said";
constexpr const char* SA_TAG_START_ON_DEMAND = "start-on-demand";
constexpr const char* SA_TAG_STOP_ON_DEMAND = "stop-on-demand";
constexpr const char* SA_TAG_ALLOW_UPDATE = "allow-update";
constexpr const char* SA_TAG_RECYCLE_DELAYTIME = "recycle-delaytime";
constexpr const char* SA_TAG_DEVICE_ON_LINE = "deviceonline";
constexpr const char* SA_TAG_SETTING_SWITCH = "settingswitch";
constexpr const char* SA_TAG_COMMON_EVENT = "commonevent";
constexpr const char* SA_TAG_PARAM = "param";
constexpr const char* SA_TAG_TIEMD_EVENT = "timedevent";
constexpr int32_t MAX_JSON_OBJECT_SIZE = 50 * 1024;
constexpr int32_t MAX_JSON_STRING_LENGTH = 128;
constexpr int32_t FIRST_SYS_ABILITY_ID = 0x00000001;
constexpr int32_t LAST_SYS_ABILITY_ID = 0x00ffffff;
const string BOOT_START_PHASE = "BootStartPhase";
const string CORE_START_PHASE = "CoreStartPhase";

enum {
    BOOT_START = 1,
    CORE_START = 2,
    OTHER_START = 3,
};

void ParseLibPath(const string& libPath, string& fileName, string& libDir)
{
    std::vector<string> libPathVec;
    SplitStr(libPath, "/", libPathVec);
    if ((libPathVec.size() > 0)) {
        int fileNameIndex = static_cast<int>(libPathVec.size()) - 1;
        fileName = libPathVec[fileNameIndex];
        int libDirIndex = fileNameIndex - 1;
        if (libDirIndex >= 0) {
            libDir = libPathVec[libDirIndex];
        }
    }
    if (libDir.empty()) {
#ifdef __aarch64__
        libDir = "lib64";
#else
        libDir = "lib";
#endif
    }
}
}

ParseUtil::~ParseUtil()
{
    ClearResource();
}

void ParseUtil::CloseHandle(SaProfile& saProfile)
{
    if (saProfile.handle == nullptr) {
        return;
    }
    int32_t ret = dlclose(saProfile.handle);
    if (ret) {
        HILOGW("close handle failed with errno:%{public}d!", errno);
    }
    saProfile.handle = nullptr;
}

void ParseUtil::CloseSo()
{
    for (auto& saProfile : saProfiles_) {
        CloseHandle(saProfile);
    }
}

void ParseUtil::CloseSo(int32_t systemAbilityId)
{
    for (auto& saProfile : saProfiles_) {
        if (saProfile.saId == systemAbilityId) {
            CloseHandle(saProfile);
            break;
        }
    }
}

void ParseUtil::ClearResource()
{
    CloseSo();
    saProfiles_.clear();
}

void ParseUtil::OpenSo(uint32_t bootPhase)
{
    for (auto& saProfile : saProfiles_) {
        if (saProfile.runOnCreate && saProfile.bootPhase == bootPhase) {
            OpenSo(saProfile);
        }
    }
}

void ParseUtil::OpenSo(SaProfile& saProfile)
{
    if (saProfile.handle == nullptr) {
        string fileName;
        string libDir;
        ParseLibPath(saProfile.libPath, fileName, libDir);
        if (libDir != "lib64" && libDir != "lib") {
            HILOGE("invalid libDir %{public}s", libDir.c_str());
            return;
        }
        bool loadFromModuleUpdate = false;
        Dl_namespace dlNs;
        string updateLibPath = GetRealPath("/module_update/" + ToString(saProfile.saId) + "/" + libDir + "/"
            + fileName);
        if (CheckPathExist(updateLibPath)) {
            Dl_namespace currentNs;
            string nsName = "module_update_" + ToString(saProfile.saId);
            dlns_init(&dlNs, nsName.c_str());
            dlns_get(nullptr, &currentNs);

            string libLdPath = GetRealPath("/module_update/" + ToString(saProfile.saId) + "/" + libDir);
            if (!libLdPath.empty()) {
                dlns_create(&dlNs, libLdPath.c_str());
                dlns_inherit(&dlNs, &currentNs, nullptr);
                loadFromModuleUpdate = true;
            }
        }

        string dlopenTag = ToString(saProfile.saId) + "_DLOPEN";
        HITRACE_METER_NAME(HITRACE_TAG_SAMGR, dlopenTag);
        int64_t begin = GetTickCount();
        DlHandle handle = nullptr;
        if (loadFromModuleUpdate) {
            handle = dlopen_ns(&dlNs, updateLibPath.c_str(), RTLD_NOW);
        }
        if (handle == nullptr) {
            handle = dlopen(saProfile.libPath.c_str(), RTLD_NOW);
        }
        HILOGI("[PerformanceTest] SA:%{public}d OpenSo spend %{public}" PRId64 " ms",
            saProfile.saId, GetTickCount() - begin);
        if (handle == nullptr) {
            ReportAddSystemAbilityFailed(saProfile.saId, fileName);
            HILOGE("dlopen %{public}s failed with errno:%{public}s!", fileName.c_str(), dlerror());
            return;
        }
        saProfile.handle = handle;
    } else {
        HILOGI("SA:%{public}d handle is not null", saProfile.saId);
    }
}

bool ParseUtil::LoadSaLib(int32_t systemAbilityId)
{
    for (auto& saProfile : saProfiles_) {
        if (saProfile.saId == systemAbilityId) {
            OpenSo(saProfile);
            return true;
        }
    }
    return false;
}

const std::list<SaProfile>& ParseUtil::GetAllSaProfiles() const
{
    return saProfiles_;
}

bool ParseUtil::GetProfile(int32_t saId, SaProfile& saProfile)
{
    auto iter = std::find_if(saProfiles_.begin(), saProfiles_.end(), [saId](auto saProfile) {
        return saProfile.saId == saId;
    });
    if (iter != saProfiles_.end()) {
        saProfile = *iter;
        return true;
    }
    return false;
}

void ParseUtil::RemoveSaProfile(int32_t saId)
{
    saProfiles_.remove_if([saId] (auto saInfo) -> bool { return saInfo.saId == saId; });
}

uint32_t ParseUtil::GetBootPriorityPara(const std::string& bootPhase)
{
    if (bootPhase == BOOT_START_PHASE) {
        return static_cast<uint32_t>(BOOT_START);
    } else if (bootPhase == CORE_START_PHASE) {
        return static_cast<uint32_t>(CORE_START);
    } else {
        return static_cast<uint32_t>(OTHER_START);
    }
}

bool ParseUtil::ParseSaProfiles(const string& profilePath)
{
    HILOGD("profilePath:%{private}s", profilePath.c_str());
    string realPath = GetRealPath(profilePath);
    if (!CheckPathExist(realPath.c_str())) {
        HILOGE("bad profile path!");
        return false;
    }

    if (Endswith(realPath, ".json")) {
        return ParseJsonFile(realPath);
    } else {
        HILOGE("Invalid file format, please use json file!");
        return false;
    }
}

bool ParseUtil::Endswith(const std::string& src, const std::string& sub)
{
    return (src.length() >= sub.length() && (src.rfind(sub) == (src.length() - sub.length())));
}

std::unordered_map<std::string, std::string> ParseUtil::StringToMap(const std::string& eventStr)
{
    nlohmann::json eventJson = StringToJsonObj(eventStr);
    std::unordered_map<std::string, std::string> eventMap = JsonObjToMap(eventJson);
    return eventMap;
}

nlohmann::json ParseUtil::StringToJsonObj(const std::string& eventStr)
{
    nlohmann::json jsonObj = nlohmann::json::object();
    if (eventStr.empty()) {
        return jsonObj;
    }
    nlohmann::json eventJson = nlohmann::json::parse(eventStr, nullptr, false);
    if (eventJson.is_discarded()) {
        HILOGE("parse eventStr to json failed");
        return jsonObj;
    }
    if (!eventJson.is_object()) {
        HILOGE("eventStr converted result is not a jsonObj");
        return jsonObj;
    }
    return eventJson;
}

std::unordered_map<std::string, std::string> ParseUtil::JsonObjToMap(const nlohmann::json& eventJson)
{
    std::unordered_map<std::string, std::string> eventMap;
    if (eventJson.contains(EVENT_TYPE) && eventJson[EVENT_TYPE].is_string()) {
        eventMap[EVENT_TYPE] = eventJson[EVENT_TYPE];
    } else {
        eventMap[EVENT_TYPE] = "";
    }
    if (eventJson.contains(EVENT_NAME) && eventJson[EVENT_NAME].is_string()) {
        eventMap[EVENT_NAME] = eventJson[EVENT_NAME];
    } else {
        eventMap[EVENT_NAME] = "";
    }
    if (eventJson.contains(EVENT_VALUE) && eventJson[EVENT_VALUE].is_string()) {
        eventMap[EVENT_VALUE] = eventJson[EVENT_VALUE];
    } else {
        eventMap[EVENT_VALUE] = "";
    }
    return eventMap;
}

bool ParseUtil::ParseJsonFile(const string& realPath)
{
    nlohmann::json profileJson;
    bool result = ParseJsonObj(profileJson, realPath);
    if (!result) {
        HILOGE("json file parse error!");
        return false;
    }
    HILOGI("profileJson:%{private}s", profileJson.dump().c_str());
    string process;
    GetStringFromJson(profileJson, SA_TAG_PROCESS, process);
    if (process.empty()) {
        HILOGE("profile format error: no process tag");
        return false;
    }
    if (process.length() > MAX_JSON_STRING_LENGTH) {
        HILOGE("profile format error: process is too long");
        return false;
    }
    procName_ = Str8ToStr16(process);
    if (profileJson.find(SA_TAG_SYSTEM_ABILITY) == profileJson.end()) {
        HILOGE("system ability parse error!");
        return false;
    }
    nlohmann::json& systemAbilityJson = profileJson.at(SA_TAG_SYSTEM_ABILITY);
    HILOGI("systemAbilityJson:%{private}s", systemAbilityJson.dump().c_str());
    if (!systemAbilityJson.is_array()) {
        HILOGE("system ability is not array!");
        return false;
    }
    size_t size = systemAbilityJson.size();
    for (size_t i = 0; i < size; i++) {
        SaProfile saProfile = { procName_ };
        if (!ParseSystemAbility(saProfile, systemAbilityJson[i])) {
            continue;
        }
        saProfiles_.emplace_back(saProfile);
    }
    return !saProfiles_.empty();
}

bool ParseUtil::ParseSystemAbility(SaProfile& saProfile, nlohmann::json& systemAbilityJson)
{
    HILOGD("ParseSystemAbility begin");
    GetInt32FromJson(systemAbilityJson, SA_TAG_NAME, saProfile.saId);
    if (saProfile.saId == 0) {
        HILOGE("profile format error: no name tag");
        return false;
    }
    if (saProfile.saId < FIRST_SYS_ABILITY_ID || saProfile.saId > LAST_SYS_ABILITY_ID) {
        HILOGE("profile format error: saId error");
        return false;
    }
    GetStringFromJson(systemAbilityJson, SA_TAG_LIB_PATH, saProfile.libPath);
    if (saProfile.libPath.empty()) {
        HILOGE("profile format error: no libPath tag");
        return false;
    }
    if (saProfile.libPath.length() > MAX_JSON_STRING_LENGTH) {
        HILOGE("profile format error: libPath is too long");
        return false;
    }
    GetBoolFromJson(systemAbilityJson, SA_TAG_RUN_ON_CREATE, saProfile.runOnCreate);
    GetBoolFromJson(systemAbilityJson, SA_TAG_AUTO_RESTART, saProfile.autoRestart);
    GetBoolFromJson(systemAbilityJson, SA_TAG_DISTRIBUTED, saProfile.distributed);
    GetIntArrayFromJson(systemAbilityJson, SA_TAG_DEPEND, saProfile.dependSa);
    GetInt32FromJson(systemAbilityJson, SA_TAG_DEPEND_TIMEOUT, saProfile.dependTimeout);
    GetInt32FromJson(systemAbilityJson, SA_TAG_DUMP_LEVEL, saProfile.dumpLevel);
    string capability;
    GetStringFromJson(systemAbilityJson, SA_TAG_CAPABILITY, capability);
    saProfile.capability = capability.length() <= MAX_JSON_STRING_LENGTH ? Str8ToStr16(capability) : u"";
    string permission;
    GetStringFromJson(systemAbilityJson, SA_TAG_PERMISSION, permission);
    saProfile.permission = permission.length() <= MAX_JSON_STRING_LENGTH ? Str8ToStr16(permission) : u"";
    string bootPhase;
    GetStringFromJson(systemAbilityJson, SA_TAG_BOOT_PHASE, bootPhase);
    saProfile.bootPhase = GetBootPriorityPara(bootPhase);
    // parse start-on-demand tag
    ParseStartOndemandTag(systemAbilityJson, SA_TAG_START_ON_DEMAND, saProfile.startOnDemand);
    // parse stop-on-demand tag
    ParseStopOndemandTag(systemAbilityJson, SA_TAG_STOP_ON_DEMAND, saProfile.stopOnDemand);
    HILOGD("ParseSystemAbility end");
    return true;
}

bool ParseUtil::ParseJsonTag(const nlohmann::json& systemAbilityJson, const std::string& jsonTag,
    nlohmann::json& onDemandJson)
{
    if (systemAbilityJson.find(jsonTag) == systemAbilityJson.end()) {
        return false;
    }
    onDemandJson = systemAbilityJson.at(jsonTag);
    if (!onDemandJson.is_object()) {
        HILOGE("parse ondemand tag error");
        return false;
    }
    return true;
}

void ParseUtil::ParseOndemandTag(const nlohmann::json& onDemandJson, std::vector<OnDemandEvent>& onDemandEvents)
{
    GetOnDemandArrayFromJson(DEVICE_ONLINE, onDemandJson, SA_TAG_DEVICE_ON_LINE, onDemandEvents);
    GetOnDemandArrayFromJson(SETTING_SWITCH, onDemandJson, SA_TAG_SETTING_SWITCH, onDemandEvents);
    GetOnDemandArrayFromJson(COMMON_EVENT, onDemandJson, SA_TAG_COMMON_EVENT, onDemandEvents);
    GetOnDemandArrayFromJson(PARAM, onDemandJson, SA_TAG_PARAM, onDemandEvents);
    GetOnDemandArrayFromJson(TIMED_EVENT, onDemandJson, SA_TAG_TIEMD_EVENT, onDemandEvents);
}

void ParseUtil::ParseStartOndemandTag(const nlohmann::json& systemAbilityJson,
    const std::string& jsonTag, StartOnDemand& startOnDemand)
{
    nlohmann::json onDemandJson;
    if (!ParseJsonTag(systemAbilityJson, jsonTag, onDemandJson)) {
        return;
    }
    ParseOndemandTag(onDemandJson, startOnDemand.onDemandEvents);
    GetBoolFromJson(onDemandJson, SA_TAG_ALLOW_UPDATE, startOnDemand.allowUpdate);
}

void ParseUtil::ParseStopOndemandTag(const nlohmann::json& systemAbilityJson,
    const std::string& jsonTag, StopOnDemand& stopOnDemand)
{
    nlohmann::json onDemandJson;
    if (!ParseJsonTag(systemAbilityJson, jsonTag, onDemandJson)) {
        return;
    }
    ParseOndemandTag(onDemandJson, stopOnDemand.onDemandEvents);
    GetBoolFromJson(onDemandJson, SA_TAG_ALLOW_UPDATE, stopOnDemand.allowUpdate);
    GetInt32FromJson(onDemandJson, SA_TAG_RECYCLE_DELAYTIME, stopOnDemand.delayTime);
}

void ParseUtil::GetOnDemandArrayFromJson(int32_t eventId, const nlohmann::json& obj,
    const std::string& key, std::vector<OnDemandEvent>& out)
{
    if (obj.find(key.c_str()) != obj.end() && obj[key.c_str()].is_array()) {
        for (auto& item : obj[key.c_str()]) {
            std::string name;
            GetStringFromJson(item, "name", name);
            std::string value;
            GetStringFromJson(item, "value", value);
            std::vector<OnDemandCondition> conditions;
            GetOnDemandConditionsFromJson(item, "conditions", conditions);
            HILOGD("conditions size: %{public}zu", conditions.size());
            bool enableOnce = false;
            GetBoolFromJson(item, "enable-once", enableOnce);
            if (!name.empty() && name.length() <= MAX_JSON_STRING_LENGTH &&
                value.length() <= MAX_JSON_STRING_LENGTH) {
                OnDemandEvent event = {eventId, name, value, -1, conditions, enableOnce};
                out.emplace_back(event);
            }
        }
    }
}

void ParseUtil::GetOnDemandConditionsFromJson(const nlohmann::json& obj,
    const std::string& key, std::vector<OnDemandCondition>& out)
{
    nlohmann::json conditionsJson;
    if (obj.find(key.c_str()) == obj.end() || !obj[key.c_str()].is_array()) {
        return;
    }
    conditionsJson = obj.at(key.c_str());
    for (auto& condition : conditionsJson) {
        std::string type;
        GetStringFromJson(condition, "eventId", type);
        std::string name;
        GetStringFromJson(condition, "name", name);
        std::string value;
        GetStringFromJson(condition, "value", value);
        int32_t eventId = 0;
        if (type == SA_TAG_DEVICE_ON_LINE) {
            eventId = DEVICE_ONLINE;
        } else if (type == SA_TAG_SETTING_SWITCH) {
            eventId = SETTING_SWITCH;
        } else if (type == SA_TAG_COMMON_EVENT) {
            eventId = COMMON_EVENT;
        } else if (type == SA_TAG_PARAM) {
            eventId = PARAM;
        } else if (type == SA_TAG_TIEMD_EVENT) {
            eventId = TIMED_EVENT;
        } else {
            HILOGW("invalid condition eventId: %{public}s", type.c_str());
            continue;
        }
        OnDemandCondition conditionEvent = {eventId, name, value};
        out.emplace_back(conditionEvent);
    }
}

std::u16string ParseUtil::GetProcessName() const
{
    return procName_;
}

string ParseUtil::GetRealPath(const string& profilePath) const
{
    char path[PATH_MAX] = {'\0'};
    if (realpath(profilePath.c_str(), path) == nullptr) {
        HILOGE("get real path fail");
        return "";
    }
    string realPath(path);
    return realPath;
}

bool ParseUtil::CheckPathExist(const string& profilePath)
{
    std::ifstream profileStream(profilePath.c_str());
    return profileStream.good();
}

bool ParseUtil::ParseTrustConfig(const string& profilePath,
    std::map<std::u16string, std::set<int32_t>>& values)
{
    HILOGD("config path:%{private}s", profilePath.c_str());
    string realPath = GetRealPath(profilePath);
    if (!CheckPathExist(realPath.c_str())) {
        HILOGE("bad profile path!");
        return false;
    }
    nlohmann::json trustSaIdJson;
    bool result = ParseJsonObj(trustSaIdJson, realPath);
    if (!result) {
        HILOGE("trust json file parse error!");
        return false;
    }
    string process;
    GetStringFromJson(trustSaIdJson, SA_TAG_PROCESS, process);
    if (process.empty()) {
        HILOGE("trust profile format error: no process tag");
        return false;
    }
    if (process.length() > MAX_JSON_STRING_LENGTH) {
        HILOGE("trust profile format error: process is too long");
        return false;
    }
    auto& saIds = values[Str8ToStr16(process)];
    GetIntArrayFromJson(trustSaIdJson, SA_TAG_SAID, saIds);
    HILOGI("ParseTrustConfig realPath:%s, saIds size = %{public}zu", realPath.c_str(), saIds.size());
    return true;
}

bool ParseUtil::ParseJsonObj(nlohmann::json& jsonObj, const string& jsonPath)
{
    std::ifstream jsonFileStream;
    jsonFileStream.open(jsonPath.c_str(), std::ios::in);
    if (!jsonFileStream.is_open()) {
        HILOGE("open json file error!!");
        return false;
    }
    std::ostringstream buffer;
    char ch;
    int32_t readSize = 0;
    while (buffer && jsonFileStream.get(ch)) {
        readSize++;
        if (readSize < MAX_JSON_OBJECT_SIZE) {
            buffer.put(ch);
        } else {
            jsonFileStream.close();
            HILOGE("too big json file error!!");
            return false;
        }
    }
    jsonFileStream.close();
    string jsonStr = buffer.str();
    jsonObj = nlohmann::json::parse(jsonStr, nullptr, false);
    if (jsonObj.is_discarded()) {
        HILOGE("parse json obj error!!");
        return false;
    }
    return true;
}
} // namespace OHOS
