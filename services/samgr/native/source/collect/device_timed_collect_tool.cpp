/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *e
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "device_timed_collect_tool.h"

#include <sys/stat.h>
#include <sys/types.h>

#include "preferences_errno.h"
#include "preferences_helper.h"
#include "preferences_value.h"
#include "sam_log.h"
#include "system_ability_manager.h"
using namespace std;

namespace OHOS {
namespace {
constexpr char PREFERENCES_PATH[] = "/data/samgr/samgr.xml";
constexpr char MULTI_USER_PREFERENCES_PATH_PREFIX[] = "/data/samgr/samgr_";
constexpr char PREFERENCES_PATH_SUFFIX[] = ".xml";
}

PreferencesUtil::PreferencesUtil() : path_(GetPathForUser(BASE_USER)) {}

PreferencesUtil::PreferencesUtil(int32_t userId) : path_(GetPathForUser(userId)) {}

shared_ptr<PreferencesUtil> PreferencesUtil::GetInstance()
{
    static std::shared_ptr<PreferencesUtil> instance = make_shared<PreferencesUtil>();
    return instance;
}

std::string PreferencesUtil::GetPathForUser(int32_t userId)
{
    if (userId == BASE_USER) {
        return PREFERENCES_PATH;
    }
    return std::string(MULTI_USER_PREFERENCES_PATH_PREFIX) + std::to_string(userId) + PREFERENCES_PATH_SUFFIX;
}

int32_t PreferencesUtil::DeleteUserPreferences(int32_t userId)
{
    if (userId == BASE_USER || userId == SAMGR_INVALID_USER_ID) {
        HILOGE("DeleteUserPreferences invalid userId:%{public}d", userId);
        return ERR_INVALID_VALUE;
    }
    int32_t ret = NativePreferences::PreferencesHelper::DeletePreferences(GetPathForUser(userId));
    if (ret != NativePreferences::E_OK) {
        HILOGE("DeleteUserPreferences failed, userId:%{public}d, ret:%{public}d", userId, ret);
        return ERR_INVALID_OPERATION;
    }
    HILOGD("DeleteUserPreferences done, userId:%{public}d", userId);
    return ERR_OK;
}

bool PreferencesUtil::GetPreference()
{
    if (ptr_ != nullptr) {
        return true;
    }
    int32_t errCode = ERR_INVALID_VALUE;
    ptr_ = NativePreferences::PreferencesHelper::GetPreferences(path_, errCode);
    if (ptr_ == nullptr) {
        HILOGE("GetPreference error code is %{public}d", errCode);
        return false;
    }
    return true;
}

bool PreferencesUtil::SaveLong(const std::string& key, int64_t value)
{
    return Save(key, value);
}

bool PreferencesUtil::SaveString(const std::string& key, std::string value)
{
    return Save(key, value);
}

template <typename T>
bool PreferencesUtil::Save(const std::string& key, const T& value)
{
    if (!GetPreference()) {
        return false;
    }
    if (!SaveInner(ptr_, key, value)) {
        HILOGE("SaveInner error");
        return false;
    }
    return RefreshSync();
}

bool PreferencesUtil::SaveInner(
    std::shared_ptr<NativePreferences::Preferences> ptr, const std::string& key, const int64_t& value)
{
    if (ptr == nullptr) {
        HILOGE("ptr is nullptr");
        return false;
    }
    return ptr->PutLong(key, value) == NativePreferences::E_OK;
}

bool PreferencesUtil::SaveInner(
    std::shared_ptr<NativePreferences::Preferences> ptr, const std::string& key, const string& value)
{
    if (ptr == nullptr) {
        HILOGE("ptr is nullptr");
        return false;
    }
    return ptr->PutString(key, value) == NativePreferences::E_OK;
}

std::map<std::string, NativePreferences::PreferencesValue> PreferencesUtil::ObtainAll()
{
    if (!GetPreference()) {
        return {};
    }
    if (ptr_ == nullptr) {
        HILOGE("ptr_ is nullptr");
        return {};
    }
    return ptr_->GetAll();
}

int64_t PreferencesUtil::ObtainLong(const std::string& key, int64_t defValue)
{
    return Obtain(key, defValue);
}

string PreferencesUtil::ObtainString(const std::string& key, std::string defValue)
{
    return Obtain(key, defValue);
}

template <typename T>
T PreferencesUtil::Obtain(const std::string& key, const T& defValue)
{
    if (!GetPreference()) {
        HILOGI("Obtain GetPreference failed");
        return defValue;
    }
    return ObtainInner(ptr_, key, defValue);
}

int64_t PreferencesUtil::ObtainInner(
    std::shared_ptr<NativePreferences::Preferences> ptr, const std::string& key, const int64_t& defValue)
{
    return ptr->GetLong(key, defValue);
}

std::string PreferencesUtil::ObtainInner(
    std::shared_ptr<NativePreferences::Preferences> ptr, const std::string& key, const std::string& defValue)
{
    return ptr->GetString(key, defValue);
}

bool PreferencesUtil::RefreshSync()
{
    if (!GetPreference()) {
        HILOGI("RefreshSync GetPreference failed");
        return false;
    }
    if (ptr_ == nullptr) {
        HILOGE("ptr_ is nullptr");
        return false;
    }
    if (ptr_->FlushSync() != NativePreferences::E_OK) {
        HILOGE("RefreshSync error");
        return false;
    }
    return true;
}

bool PreferencesUtil::IsExist(const std::string& key)
{
    if (!GetPreference()) {
        HILOGI("IsExist GetPreference failed");
        return false;
    }
    if (ptr_ == nullptr) {
        HILOGE("ptr_ is nullptr");
        return false;
    }
    return ptr_->HasKey(key);
}

bool PreferencesUtil::Remove(const std::string &key)
{
    if (!GetPreference()) {
        HILOGI("Remove GetPreference failed");
        return false;
    }
    if (ptr_ == nullptr) {
        HILOGE("ptr_ is nullptr");
        return false;
    }
    if (ptr_->Delete(key) != NativePreferences::E_OK) {
        return false;
    }
    return RefreshSync();
}
}