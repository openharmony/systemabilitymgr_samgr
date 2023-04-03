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

#ifndef INTERFACES_INNERKITS_SAMGR_INCLUDE_IF_SYSTEM_ABILITY_MANAGER_H
#define INTERFACES_INNERKITS_SAMGR_INCLUDE_IF_SYSTEM_ABILITY_MANAGER_H

#include <string>
#include <list>

#include "iremote_broker.h"
#include "iremote_object.h"
#include "iremote_proxy.h"
#include "isystem_ability_load_callback.h"
#include "isystem_ability_status_change.h"
#include "isystem_process_status_change.h"

namespace OHOS {
class ISystemAbilityManager : public IRemoteBroker {
public:
    /**
     * ListSystemAbilities, Return list of all existing abilities.
     *
     * @param dumpFlags,.dump all
     * @return Returns the sa where the current samgr exists
     */
    virtual std::vector<std::u16string> ListSystemAbilities(unsigned int dumpFlags = DUMP_FLAG_PRIORITY_ALL) = 0;

    enum {
        SHEEFT_CRITICAL = 0,
        SHEEFT_HIGH,
        SHEEFT_NORMAL,
        SHEEFT_DEFAULT,
        SHEEFT_PROTO,
    };

    static const unsigned int DUMP_FLAG_PRIORITY_CRITICAL = 1 << SHEEFT_CRITICAL;
    static const unsigned int DUMP_FLAG_PRIORITY_HIGH = 1 << SHEEFT_HIGH;
    static const unsigned int DUMP_FLAG_PRIORITY_NORMAL = 1 << SHEEFT_NORMAL;

    static const unsigned int DUMP_FLAG_PRIORITY_DEFAULT = 1 << SHEEFT_DEFAULT;
    static const unsigned int DUMP_FLAG_PRIORITY_ALL = DUMP_FLAG_PRIORITY_CRITICAL |
        DUMP_FLAG_PRIORITY_HIGH | DUMP_FLAG_PRIORITY_NORMAL | DUMP_FLAG_PRIORITY_DEFAULT;
    static const unsigned int DUMP_FLAG_PROTO = 1 << SHEEFT_PROTO;

    enum {
        GET_SYSTEM_ABILITY_TRANSACTION = 1,
        CHECK_SYSTEM_ABILITY_TRANSACTION = 2,
        ADD_SYSTEM_ABILITY_TRANSACTION = 3,
        REMOVE_SYSTEM_ABILITY_TRANSACTION = 4,
        LIST_SYSTEM_ABILITY_TRANSACTION = 5,
        SUBSCRIBE_SYSTEM_ABILITY_TRANSACTION = 6,
        LOAD_SYSTEM_ABILITY_TRANSACTION = 7,
        LOAD_REMOTE_SYSTEM_ABILITY_TRANSACTION = 8,
        CHECK_REMOTE_SYSTEM_ABILITY_TRANSACTION = 9,
        ADD_ONDEMAND_SYSTEM_ABILITY_TRANSACTION = 10,
        CHECK_SYSTEM_ABILITY_IMMEDIATELY_TRANSACTION = 12,
        CHECK_ONDEMAND_SYSTEM_ABILITY_TRANSACTION = 15,
        GET_SYSTEM_ABILITYINFOLIST_TRANSACTION = 17,
        UNSUBSCRIBE_SYSTEM_ABILITY_TRANSACTION = 18,
        ADD_SYSTEM_PROCESS_TRANSACTION = 20,
        UNLOAD_SYSTEM_ABILITY_TRANSACTION = 21,
        CANCEL_UNLOAD_SYSTEM_ABILITY_TRANSACTION = 22,
        GET_RUNNING_SYSTEM_PROCESS_TRANSACTION = 23,
        SUBSCRIBE_SYSTEM_PROCESS_TRANSACTION = 24,
        UNSUBSCRIBE_SYSTEM_PROCESS_TRANSACTION = 25,
    };
    /**
     * GetSystemAbility, Retrieve an existing ability, retrying and blocking for a few seconds if it doesn't exist.
     *
     * @param systemAbilityId,.Need to obtain the said of sa
     * @return nullptr indicates acquisition failure
     */
    virtual sptr<IRemoteObject> GetSystemAbility(int32_t systemAbilityId) = 0;

    /**
     * CheckSystemAbility, Retrieve an existing ability, no-blocking.
     *
     * @param systemAbilityId,.Need to obtain the said of sa
     * @return nullptr indicates acquisition failure
     */
    virtual sptr<IRemoteObject> CheckSystemAbility(int32_t systemAbilityId) = 0;

    /**
     * RemoveSystemAbility, Remove an ability.
     *
     * @param systemAbilityId,.Need to remove the said of sa
     * @return ERR_OK indicates remove success
     */
    virtual int32_t RemoveSystemAbility(int32_t systemAbilityId) = 0;

    /**
     * SubscribeSystemAbility, Subscribe a system ability status, and inherit from ISystemAbilityStatusChange class.
     *
     * @param systemAbilityId,.Need to subscribe the said of sa
     * @param listener,.Need to implement OnAddSystemAbility, OnRemoveSystemAbility
     * @return ERR_OK indicates SubscribeSystemAbility success
     */
    virtual int32_t SubscribeSystemAbility(int32_t systemAbilityId,
        const sptr<ISystemAbilityStatusChange>& listener) = 0;

    /**
     * UnSubscribeSystemAbility, UnSubscribe a system ability status, and inherit from ISystemAbilityStatusChange class.
     *
     * @param systemAbilityId,.Need to UnSubscribe the said of sa
     * @param listener,.Need to implement OnAddSystemAbility, OnRemoveSystemAbility
     * @return ERR_OK indicates SubscribeSystemAbility success
     */
    virtual int32_t UnSubscribeSystemAbility(int32_t systemAbilityId,
        const sptr<ISystemAbilityStatusChange>& listener) = 0;

    /**
     * GetSystemAbility, Retrieve an existing ability, blocking for a few seconds if it doesn't exist.
     *
     * @param systemAbilityId,.Need to get the said of sa
     * @param deviceId,.If the device id is empty, it indicates that it is a local get
     * @return nullptr indicates acquisition failure
     */
    virtual sptr<IRemoteObject> GetSystemAbility(int32_t systemAbilityId, const std::string& deviceId) = 0;

    /**
     * CheckSystemAbility, Retrieve an existing ability, no-blocking
     *
     * @param systemAbilityId,.Need to get the said of sa
     * @param deviceId,.If the device id is empty, it indicates that it is a local get
     * @return nullptr indicates acquisition failure
     */
    virtual sptr<IRemoteObject> CheckSystemAbility(int32_t systemAbilityId, const std::string& deviceId) = 0;

    /**
     * AddOnDemandSystemAbilityInfo, Add ondemand ability info.
     *
     * @param systemAbilityId,.Need to add info the said of sa
     * @param localAbilityManagerName,.Process Name
     * @return ERR_OK indicates AddOnDemandSystemAbilityInfo success
     */
    virtual int32_t AddOnDemandSystemAbilityInfo(int32_t systemAbilityId,
        const std::u16string& localAbilityManagerName) = 0;

    /**
     * CheckSystemAbility, Retrieve an ability, no-blocking.
     *
     * @param systemAbilityId,.Need to check the said of sa
     * @param isExist,.Issue parameters, and a result of true indicates success
     * @return nullptr indicates acquisition failure
     */
    virtual sptr<IRemoteObject> CheckSystemAbility(int32_t systemAbilityId, bool& isExist) = 0;

    struct SAExtraProp {
        SAExtraProp() = default;
        SAExtraProp(bool isDistributed, unsigned int dumpFlags, const std::u16string& capability,
            const std::u16string& permission)
        {
            this->isDistributed = isDistributed;
            this->dumpFlags = dumpFlags;
            this->capability = capability;
            this->permission = permission;
        }

        bool isDistributed = false;
        unsigned int dumpFlags = DUMP_FLAG_PRIORITY_DEFAULT;
        std::u16string capability;
        std::u16string permission;
    };

    /**
     * AddSystemAbility, add an ability to samgr
     *
     * @param systemAbilityId,.Need to add the said of sa
     * @param ability, .SA to be added
     * @param extraProp, .Additional parameters for sa, such as whether it is distributed
     * @return ERR_OK indicates successful add
     */
    virtual int32_t AddSystemAbility(int32_t systemAbilityId, const sptr<IRemoteObject>& ability,
        const SAExtraProp& extraProp = SAExtraProp(false, DUMP_FLAG_PRIORITY_DEFAULT, u"", u"")) = 0;

    /**
     * AddSystemProcess, add an process
     *
     * @param procName,.Need to add the procName of process
     * @param procObject, .Remoteobject of procName
     * @return ERR_OK indicates successful add
     */
    virtual int32_t AddSystemProcess(const std::u16string& procName, const sptr<IRemoteObject>& procObject) = 0;
    
    /**
     * LoadSystemAbility, Load sa
     *
     * @param systemAbilityId,.Need to load the said of sa
     * @param callback, .OnLoadSystemAbilityFail and OnLoadSystemAbilitySuccess need be rewritten
     * @return ERR_OK It does not mean that the load was successful, but a callback function is
     required to confirm whether it was successful
     */
    virtual int32_t LoadSystemAbility(int32_t systemAbilityId, const sptr<ISystemAbilityLoadCallback>& callback) = 0;
    
    /**
     * LoadSystemAbility, Load sa
     *
     * @param systemAbilityId,.Need to load the said of sa
     * @param deviceId,.if deviceId is empty, it indicates local load
     * @param callback, .OnLoadSystemAbilityFail and OnLoadSystemAbilitySuccess need be rewritten
     * @return ERR_OK It does not mean that the load was successful, but a callback function is
     required to confirm whether it was successful
     */
    virtual int32_t LoadSystemAbility(int32_t systemAbilityId, const std::string& deviceId,
        const sptr<ISystemAbilityLoadCallback>& callback) = 0;

    /**
     * UnloadSystemAbility, UnLoad sa
     *
     * @param systemAbilityId,.Need to UnLoad the said of sa
     * @return ERR_OK It does not mean that the unload was successful, but sa entered an idle state
     */
    virtual int32_t UnloadSystemAbility(int32_t systemAbilityId) = 0;

    /**
     * CancelUnloadSystemAbility, CancelUnload sa
     *
     * @param systemAbilityId,.Need to CancelUnload the said of sa
     * @return ERR_OK indicates that the uninstall was canceled successfully
     */
    virtual int32_t CancelUnloadSystemAbility(int32_t systemAbilityId) = 0;

    /**
     * GetRunningSystemProcess, Get all processes currently running
     *
     * @param systemProcessInfos,.Issue a parameter and return it as a result.
     * @return ERR_OK indicates that the get successfully
     */
    virtual int32_t GetRunningSystemProcess(std::list<SystemProcessInfo>& systemProcessInfos) = 0;

    /**
     * SubscribeSystemProcess, Subscribe the status of process
     *
     * @param listener,.callback, add listener to processListeners_, OnSystemProcessStarted will be
     called when the process starts, OnSystemProcessStopped will be called when the process end.
     * @return ERR_OK indicates that the Subscribe successfully
     */
    virtual int32_t SubscribeSystemProcess(const sptr<ISystemProcessStatusChange>& listener) = 0;
    
    /**
     * UnSubscribeSystemProcess, UnSubscribe the status of process
     *
     * @param listener,.callback, remove listener to processListeners_, OnSystemProcessStarted will be
     called when the process starts, OnSystemProcessStopped will be called when the process end.
     * @return ERR_OK indicates that the UnSubscribe successfully
     */
    virtual int32_t UnSubscribeSystemProcess(const sptr<ISystemProcessStatusChange>& listener) = 0;
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.ISystemAbilityManager");
protected:
    static constexpr int32_t FIRST_SYS_ABILITY_ID = 0x00000001;
    static constexpr int32_t LAST_SYS_ABILITY_ID = 0x00ffffff;
    bool CheckInputSysAbilityId(int32_t sysAbilityId) const
    {
        if (sysAbilityId >= FIRST_SYS_ABILITY_ID && sysAbilityId <= LAST_SYS_ABILITY_ID) {
            return true;
        }
        return false;
    }
    static inline const std::u16string SAMANAGER_INTERFACE_TOKEN = u"ohos.samgr.accessToken";
};
} // namespace OHOS

#endif // !defined(INTERFACES_INNERKITS_SAMGR_INCLUDE_IF_SYSTEM_ABILITY_MANAGER_H )
