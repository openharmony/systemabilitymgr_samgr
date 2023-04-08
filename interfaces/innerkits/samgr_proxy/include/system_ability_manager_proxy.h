/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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


#ifndef INTERFACES_INNERKITS_SAMGR_INCLUDE_SYSTEM_ABILITY_MANAGER_PROXY_H
#define INTERFACES_INNERKITS_SAMGR_INCLUDE_SYSTEM_ABILITY_MANAGER_PROXY_H

#include <string>
#include "if_system_ability_manager.h"

namespace OHOS {
class SystemAbilityManagerProxy : public IRemoteProxy<ISystemAbilityManager> {
public:
    explicit SystemAbilityManagerProxy(const sptr<IRemoteObject>& impl)
        : IRemoteProxy<ISystemAbilityManager>(impl) {}
    ~SystemAbilityManagerProxy() = default;
    /**
     * ListSystemAbilities, Return list of all existing abilities.
     *
     * @param dumpFlags, dump all
     * @return Returns the sa where the current samgr exists.
     */
    std::vector<std::u16string> ListSystemAbilities(unsigned int dumpFlags) override;

    /**
     * GetSystemAbility, Retrieve an existing ability, retrying and blocking for a few seconds if it doesn't exist.
     *
     * @param systemAbilityId, Need to obtain the said of sa.
     * @return nullptr indicates acquisition failure.
     */
    sptr<IRemoteObject> GetSystemAbility(int32_t systemAbilityId) override;

    /**
     * CheckSystemAbility, Retrieve an existing ability, no-blocking.
     *
     * @param systemAbilityId, Need to obtain the said of sa.
     * @return nullptr indicates acquisition failure.
     */
    sptr<IRemoteObject> CheckSystemAbility(int32_t systemAbilityId) override;

    /**
     * RemoveSystemAbility, Remove an ability.
     *
     * @param systemAbilityId, Need to remove the said of sa.
     * @return ERR_OK indicates remove success.
     */
    int32_t RemoveSystemAbility(int32_t systemAbilityId) override;

    /**
     * SubscribeSystemAbility, Subscribe a system ability status, and inherit from ISystemAbilityStatusChange class.
     *
     * @param systemAbilityId, Need to subscribe the said of sa.
     * @param listener, Need to implement OnAddSystemAbility, OnRemoveSystemAbility.
     * @return ERR_OK indicates SubscribeSystemAbility success.
     */
    int32_t SubscribeSystemAbility(int32_t systemAbilityId, const sptr<ISystemAbilityStatusChange>& listener) override;
    
    /**
     * UnSubscribeSystemAbility, UnSubscribe a system ability status, and inherit from ISystemAbilityStatusChange class.
     *
     * @param systemAbilityId, Need to UnSubscribe the said of sa.
     * @param listener, Need to implement OnAddSystemAbility, OnRemoveSystemAbility.
     * @return ERR_OK indicates SubscribeSystemAbility success.
     */
    int32_t UnSubscribeSystemAbility(int32_t systemAbilityId,
        const sptr<ISystemAbilityStatusChange> &listener) override;

    /**
     * GetSystemAbility, Retrieve an existing ability, blocking for a few seconds if it doesn't exist.
     *
     * @param systemAbilityId, Need to get the said of sa.
     * @param deviceId, If the device id is empty, it indicates that it is a local get.
     * @return nullptr indicates acquisition failure.
     */
    sptr<IRemoteObject> GetSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    
    /**
     * CheckSystemAbility, Retrieve an existing ability, no-blocking.
     *
     * @param systemAbilityId, Need to get the said of sa.
     * @param deviceId, If the device id is empty, it indicates that it is a local get.
     * @return nullptr indicates acquisition failure.
     */
    sptr<IRemoteObject> CheckSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    
    /**
     * AddOnDemandSystemAbilityInfo, Add ondemand ability info.
     *
     * @param systemAbilityId, Need to add info the said of sa.
     * @param localAbilityManagerName, Process Name.
     * @return ERR_OK indicates AddOnDemandSystemAbilityInfo success.
     */
    int32_t AddOnDemandSystemAbilityInfo(int32_t systemAbilityId,
        const std::u16string& localAbilityManagerName) override;
    
    /**
     * CheckSystemAbility, Retrieve an ability, no-blocking.
     *
     * @param systemAbilityId, Need to check the said of sa.
     * @param isExist, Issue parameters, and a result of true indicates success.
     * @return nullptr indicates acquisition failure.
     */
    sptr<IRemoteObject> CheckSystemAbility(int32_t systemAbilityId, bool& isExist) override;
    
    /**
     * AddSystemAbility, add an ability to samgr
     *
     * @param systemAbilityId, Need to add the said of sa.
     * @param ability, SA to be added.
     * @param extraProp, Additional parameters for sa, such as whether it is distributed.
     * @return ERR_OK indicates successful add.
     */
    int32_t AddSystemAbility(int32_t systemAbilityId, const sptr<IRemoteObject>& ability,
        const SAExtraProp& extraProp) override;

    /**
     * AddSystemProcess, add an process.
     *
     * @param procName, Need to add the procName of process.
     * @param procObject, Remoteobject of procName.
     * @return ERR_OK indicates successful add.
     */
    int32_t AddSystemProcess(const std::u16string& procName, const sptr<IRemoteObject>& procObject) override;
    
    /**
     * LoadSystemAbility, Load sa.
     *
     * @param systemAbilityId, Need to load the said of sa.
     * @param callback, OnLoadSystemAbilityFail and OnLoadSystemAbilitySuccess need be rewritten.
     * @return ERR_OK It does not mean that the load was successful, but a callback function is.
     required to confirm whether it was successful.
     */
    int32_t LoadSystemAbility(int32_t systemAbilityId, const sptr<ISystemAbilityLoadCallback>& callback) override;
    
    /**
     * LoadSystemAbility, Load sa.
     *
     * @param systemAbilityId, Need to load the said of sa.
     * @param deviceId, if deviceId is empty, it indicates local load.
     * @param callback, OnLoadSystemAbilityFail and OnLoadSystemAbilitySuccess need be rewritten.
     * @return ERR_OK It does not mean that the load was successful
     */
    int32_t LoadSystemAbility(int32_t systemAbilityId, const std::string& deviceId,
        const sptr<ISystemAbilityLoadCallback>& callback) override;
    
    /**
     * UnloadSystemAbility, UnLoad sa.
     *
     * @param systemAbilityId, Need to UnLoad the said of sa.
     * @return ERR_OK It does not mean that the unload was successful.
     */
    int32_t UnloadSystemAbility(int32_t systemAbilityId) override;
    
    /**
     * CancelUnloadSystemAbility, CancelUnload sa.
     *
     * @param systemAbilityId, Need to CancelUnload the said of sa.
     * @return ERR_OK indicates that the uninstall was canceled successfully.
     */
    int32_t CancelUnloadSystemAbility(int32_t systemAbilityId) override;
    
    /**
     * GetRunningSystemProcess, Get all processes currently running.
     *
     * @param systemProcessInfos, Issue a parameter and return it as a result.
     * @return ERR_OK indicates that the get successfully.
     */
    int32_t GetRunningSystemProcess(std::list<SystemProcessInfo>& systemProcessInfos) override;
    
    /**
     * SubscribeSystemProcess, Subscribe the status of process.
     *
     * @param listener, callback.
     * @return ERR_OK indicates that the Subscribe successfully.
     */
    int32_t SubscribeSystemProcess(const sptr<ISystemProcessStatusChange>& listener) override;
    
    /**
     * UnSubscribeSystemProcess, UnSubscribe the status of process.
     *
     * @param listener, callback.
     * @return ERR_OK indicates that the UnSubscribe successfully.
     */
    int32_t UnSubscribeSystemProcess(const sptr<ISystemProcessStatusChange>& listener) override;
private:
    sptr<IRemoteObject> GetSystemAbilityWrapper(int32_t systemAbilityId, const std::string& deviceId = "");
    sptr<IRemoteObject> CheckSystemAbilityWrapper(int32_t code, MessageParcel& data);
    int32_t MarshalSAExtraProp(const SAExtraProp& extraProp, MessageParcel& data) const;
    int32_t AddSystemAbilityWrapper(int32_t code, MessageParcel& data);
    int32_t RemoveSystemAbilityWrapper(int32_t code, MessageParcel& data);
    int32_t ReadSystemProcessFromParcel(std::list<SystemProcessInfo>& systemProcessInfos, MessageParcel& reply);
private:
    static inline BrokerDelegator<SystemAbilityManagerProxy> delegator_;
};
} // namespace OHOS

#endif // !defined(INTERFACES_INNERKITS_SAMGR_INCLUDE_SYSTEM_ABILITY_MANAGER_PROXY_H)
