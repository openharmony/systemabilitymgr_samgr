/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef MOCK_DEVICE_MANAGER_H
#define MOCK_DEVICE_MANAGER_H

#include <vector>

#include "device_manager_callback.h"
#include "dm_device_info.h"

namespace OHOS::DistributedHardware {
class DeviceManager {
public:
    static DeviceManager &GetInstance()
    {
        static DeviceManager instance;
        return instance;
    }

    int32_t InitDeviceManager(const std::string &pkgName, std::shared_ptr<DmInitCallback> dmInitCallback)
    {
        return 0;
    }
    int32_t RegisterDevStateCallback(const std::string &pkgName, const std::string &extra,
        std::shared_ptr<DeviceStateCallback> callback)
    {
        return 0;
    }
    int32_t UnRegisterDevStateCallback(const std::string &pkgName)
    {
        return 0;
    }
    int32_t GetTrustedDeviceList(const std::string &pkgName, const std::string &extra,
        std::vector<DmDeviceInfo> &deviceList)
    {
        return 0;
    }

private:
    DeviceManager() = default;
    ~DeviceManager() = default;
    std::shared_ptr<DeviceStateCallback> callback_;
};
} // namespace OHOS::DistributedHardware
#endif // MOCK_DEVICE_MANAGER_H