# Samgr
## Introduction

The System Ability Manager (Samgr) component is a core component of OpenHarmony. It provides functions related to system abilities (also called system services), including the startup, registration, and query.

## System Architecture

Figure 1 Architecture of Samgr


![](figures/en-us_image_0000001115820566.png)

## Directory Structure

```
/foundation/systemabilitymgr
├── samgr
│   ├── bundle.json  # Description and build file of Samgr
│   ├── frameworks   # Framework implementation
│   ├── interfaces   # APIs
│   ├── services     # Directory for the Samgr service
│   ├── test         # Test code
│   ├── utils        # Utils
```

## Description

1.  Upon receiving a registration message from the system ability framework, the Samgr service saves the system ability information in the local cache.

    ```
    int32_t SystemAbilityManager::AddSystemAbility(int32_t systemAbilityId, const sptr<IRemoteObject>& ability,
        const SAExtraProp& extraProp)
    {
        if (!CheckInputSysAbilityId(systemAbilityId) || ability == nullptr) {
            HILOGE("AddSystemAbilityExtra input params is invalid.");
            return ERR_INVALID_VALUE;
        }
        {
            unique_lock<shared_mutex> writeLock(abilityMapLock_);
            auto saSize = abilityMap_.size();
            if (saSize >= MAX_SERVICES) {
                HILOGE("map size error, (Has been greater than %zu)", saSize);
                return ERR_INVALID_VALUE;
            }
            SAInfo saInfo;
            saInfo.remoteObj = ability;
            saInfo.isDistributed = extraProp.isDistributed;
            saInfo.capability = extraProp.capability;
            saInfo.permission = Str16ToStr8(extraProp.permission);
            abilityMap_[systemAbilityId] = std::move(saInfo);
            HILOGI("insert %{public}d. size : %{public}zu", systemAbilityId, abilityMap_.size());
        }
        RemoveCheckLoadedMsg(systemAbilityId);
        if (abilityDeath_ != nullptr) {
            ability->AddDeathRecipient(abilityDeath_);
        }

        u16string strName = Str8ToStr16(to_string(systemAbilityId));
        if (extraProp.isDistributed && dBinderService_ != nullptr) {
            dBinderService_->RegisterRemoteProxy(strName, systemAbilityId);
            HILOGD("AddSystemAbility RegisterRemoteProxy, serviceId is %{public}d", systemAbilityId);
        }
        if (systemAbilityId == SOFTBUS_SERVER_SA_ID && !isDbinderStart_) {
            if (dBinderService_ != nullptr && rpcCallbackImp_ != nullptr) {
                bool ret = dBinderService_->StartDBinderService(rpcCallbackImp_);
                HILOGI("start result is %{public}s", ret ? "succeed" : "fail");
                isDbinderStart_ = true;
            }
        }
        SendSystemAbilityAddedMsg(systemAbilityId, ability);
        return ERR_OK;
    }
    ```

2.  If the system service requested by the system ability framework is a local service, the Samgr service locates the proxy object of the system service based on the service ID and returns the proxy object to the system ability framework.

    ```
    sptr<IRemoteObject> SystemAbilityManager::CheckSystemAbility(int32_t systemAbilityId)
    {
        if (!CheckInputSysAbilityId(systemAbilityId)) {
            HILOGW("CheckSystemAbility CheckSystemAbility invalid!");
            return nullptr;
        }

        shared_lock<shared_mutex> readLock(abilityMapLock_);
        auto iter = abilityMap_.find(systemAbilityId);
        if (iter != abilityMap_.end()) {
            HILOGI("found service : %{public}d.", systemAbilityId);
            return iter->second.remoteObj;
        }
        HILOGI("NOT found service : %{public}d", systemAbilityId);
        return nullptr;
    }
    ```

3. The Samgr service dynamically loads the system service process and system ability. Instead of starting upon system startup, the process starts when the system ability is accessed and loads the specified system ability.
    
    3.1 Inherit from the **SystemAbilityLoadCallbackStub** class and override the **OnLoadSystemAbilitySuccess(int32_t systemAbilityId, const sptr<IRemoteObject>& remoteObject)** and **OnLoadSystemAbilityFail(int32_t systemAbilityId)** methods.
    
    ```
    class OnDemandLoadCallback : public SystemAbilityLoadCallbackStub {
    public:
        void OnLoadSystemAbilitySuccess(int32_t systemAbilityId, const sptr<IRemoteObject>& remoteObject) override;
        void OnLoadSystemAbilityFail(int32_t systemAbilityId) override;
    };
    
    void OnDemandLoadCallback::OnLoadSystemAbilitySuccess(int32_t systemAbilityId,
        const sptr<IRemoteObject>& remoteObject) // systemAbilityId is the ID of the system ability to be loaded, and remoteObject indicates the proxy object of the system ability.
    {
        cout << "OnLoadSystemAbilitySuccess systemAbilityId:" << systemAbilityId << " IRemoteObject result:" <<
            ((remoteObject != nullptr) ? "succeed" : "failed") << endl;
    }
    
    void OnDemandLoadCallback::OnLoadSystemAbilityFail(int32_t systemAbilityId) // systemAbilityId is the ID of the system ability to be loaded.
    {
        cout << "OnLoadSystemAbilityFail systemAbilityId:" << systemAbilityId << endl;
    }
    ```
    
    3.2 Call **LoadSystemAbility(int32_t systemAbilityId, const sptr<ISystemAbilityLoadCallback>& callback)** provided by Samgr.
    
    ```
    // Construct a SystemAbilityLoadCallbackStub instance (mentioned in step 3.1).
    sptr<OnDemandLoadCallback> loadCallback_ = new OnDemandLoadCallback();
    // Call the LoadSystemAbility method.
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sm == nullptr) {
        cout << "GetSystemAbilityManager samgr object null!" << endl;
        return;
    }
    int32_t result = sm->LoadSystemAbility(systemAbilityId, loadCallback_);
    if (result != ERR_OK) {
        cout << "systemAbilityId:" << systemAbilityId << " load failed, result code:" << result << endl;
        return;
    }
    ```
>NOTE
>
>1. After **LoadSystemAbility** is called, the **OnLoadSystemAbilitySuccess** callback will be invoked if the specified system ability is successfully loaded and the **OnLoadSystemAbilityFail** callback will be invoked if the system ability fails to be loaded.
>
>2. The dynamically loaded process cannot start upon system startup. You must set **"ondemand": true** in the .cfg file. The following is an example:
>
>```
>{
>   "services" : [{
>           "name" : "listen_test",
>           "path" : ["/system/bin/sa_main", "/system/profile/listen_test.json"],
>           "ondemand" : true,
>           "uid" : "system",
>           "gid" : ["system", "shell"]
>       }
>   ]
>}
>```
>3. The **LoadSystemAbility** method applies to dynamic loading of system abilities. In other scenarios, use the **CheckSystemAbility** method to obtain a system ability.
>4. The process name in the .cfg file must be the same as that in the .json configuration file of the system ability.

## Repositories Involved

Samgr

[systemabilitymgr\_safwk](https://gitee.com/openharmony/systemabilitymgr_safwk)

[**systemabilitymgr\_samgr**](https://gitee.com/openharmony/systemabilitymgr_samgr)

[systemabilitymgr\_safwk\_lite](https://gitee.com/openharmony/systemabilitymgr_safwk_lite)

[systemabilitymgr\_samgr\_lite](https://gitee.com/openharmony/systemabilitymgr_samgr_lite)
