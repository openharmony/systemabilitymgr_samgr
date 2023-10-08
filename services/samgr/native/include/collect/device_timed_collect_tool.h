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

#ifndef OHOS_SYSTEM_ABILITY_MANAGER_DEVICE_TIMED_COLLECT_TOOL_H
#define OHOS_SYSTEM_ABILITY_MANAGER_DEVICE_TIMED_COLLECT_TOOL_H

#include <map>
#include <time.h>

#include "preferences.h"
namespace OHOS {
class PreferencesUtil {
public:
    static std::shared_ptr<PreferencesUtil> GetInstance();
    PreferencesUtil() = default;
    ~PreferencesUtil() = default;

    bool SaveLong(const std::string& key, int64_t value);
    bool SaveString(const std::string& key, std::string value);

    int64_t ObtainLong(const std::string& key, int64_t defValue);
    std::string ObtainString(const std::string& key, std::string defValue);
    std::map<std::string, NativePreferences::PreferencesValue> ObtainAll();

    bool IsExist(const std::string& key);
    bool Remove(const std::string &key);
private:
    bool GetPreference();
    bool RefreshSync();

    template <typename T>
    bool Save(const std::string& key, const T& defValue);

    bool SaveInner(
        std::shared_ptr<NativePreferences::Preferences> ptr, const std::string& key, const int64_t& value);
    bool SaveInner(
        std::shared_ptr<NativePreferences::Preferences> ptr, const std::string& key, const std::string& value);

    template <typename T>
    T Obtain(const std::string& key, const T& defValue);
    
    int64_t ObtainInner(
        std::shared_ptr<NativePreferences::Preferences> ptr, const std::string& key, const int64_t& defValue);
    std::string ObtainInner(
        std::shared_ptr<NativePreferences::Preferences> ptr, const std::string& key, const std::string& defValue);
    std::shared_ptr<NativePreferences::Preferences> ptr_ = nullptr;
};

class TimeUtils {
public:
    static int64_t GetTimestamp()
    {
        time_t currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        return static_cast<int64_t>(currentTime);
    }
};
} // namespace OHOS
#endif // OHOS_SYSTEM_ABILITY_MANAGER_DEVICE_TIMED_COLLECT_TOOL_H