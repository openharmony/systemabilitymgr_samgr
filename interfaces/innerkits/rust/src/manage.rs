// Copyright (C) 2024 Huawei Device Co., Ltd.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

use cxx::{let_cxx_string, UniquePtr};
use ipc::parcel::MsgParcel;
use ipc::remote::{RemoteObj, RemoteStub};

use crate::wrapper::{
    GetCommonEventExtraDataIdlist,
    AbilityStub, AddOnDemandSystemAbilityInfo, AddSystemAbility, AddSystemAbilityConfig,
    CancelUnloadSystemAbility, CheckSystemAbility, CheckSystemAbilityWithDeviceId,
    GetContextManager, GetOnDemandReasonExtraData, GetRunningSystemProcess, GetSystemAbility,
    GetSystemAbilityWithDeviceId, GetSystemProcessInfo, ListSystemAbilities,
    ListSystemAbilitiesWithDumpFlag, LoadSystemAbility, LoadSystemAbilityWithCallback,
    RemoveSystemAbility, SendStrategy, SubscribeSystemAbility, SubscribeSystemProcess,
    SystemProcessInfo, UnSubscribeSystemAbilityHandler, UnSubscribeSystemProcessHandler,
    UnloadAllIdleSystemAbility, UnloadSystemAbility,
};
use crate::DumpFlagPriority;

pub struct SystemAbilityManager;

unsafe impl Sync for SystemAbilityManager {}
unsafe impl Send for SystemAbilityManager {}

impl SystemAbilityManager {
    /// # Example
    /// ```rust
    /// use samgr::manage::SystemAbilityManager;
    ///
    /// let context = SystemAbilityManager::get_context_manager();
    /// ```
    pub fn get_context_manager() -> Option<RemoteObj> {
        info!("get context manager");
        RemoteObj::from_sptr(GetContextManager())
    }

    /// let abilities = sysm.list_system_ability();
    ///
    ///
    /// # Example
    /// ```rust
    /// ```
    pub fn list_system_abilities() -> Vec<String> {
        info!("list system ability");
        ListSystemAbilities()
    }
    /// let abilities = sysm.list_system_ability();
    ///
    ///
    /// # Example
    /// ```rust
    /// ```
    pub fn list_system_abilities_with_dump_flag(dump_flag: DumpFlagPriority) -> Vec<String> {
        info!("list system ability");
        ListSystemAbilitiesWithDumpFlag(dump_flag as u32)
    }

    /// # Example
    /// ```rust
    /// use samgr::manage::SystemAbilityManager;
    ///
    /// let request_abilty = SystemAbilityManager::get_system_ability(3706, None);
    /// ```
    pub fn get_system_ability(said: i32) -> Option<RemoteObj> {
        info!("get system ability {}", said);
        RemoteObj::from_sptr(GetSystemAbility(said))
    }

    /// # Example
    /// ```rust
    /// use samgr::manage::SystemAbilityManager;
    /// ```
    pub fn get_system_ability_with_device_id(said: i32, device_id: &str) -> Option<RemoteObj> {
        info!("get system ability {} with device id", said);
        let_cxx_string!(id = device_id);
        RemoteObj::from_sptr(GetSystemAbilityWithDeviceId(said, &id))
    }

    /// # Example
    /// ```rust
    /// use samgr::definition::DOWNLOAD_SERVICE_ID;
    /// use samgr::manage::SystemAbilityManager;
    ///
    /// let download_service = SystemAbilityManager::check_system_ability(DOWNLOAD_SERVICE_ID).unwrap();
    /// ```
    pub fn check_system_ability(said: i32) -> Option<RemoteObj> {
        info!("check system ability {}", said);

        RemoteObj::from_sptr(CheckSystemAbility(said))
    }

    pub fn check_system_ability_with_ability(said: i32, device_id: &str) -> Option<RemoteObj> {
        info!("check system ability {} with device id", said);
        let_cxx_string!(id = device_id);
        RemoteObj::from_sptr(CheckSystemAbilityWithDeviceId(said, &id))
    }

    /// # Example
    /// ```rust
    /// use samgr::definition::DOWNLOAD_SERVICE_ID;
    /// use samgr::manage::SystemAbilityManager;
    ///
    /// SystemAbilityManager::remove_system_ability(DOWNLOAD_SERVICE_IDD);
    /// ```
    pub fn remove_system_ability(said: i32) -> i32 {
        info!("remove system ability {}", said);
        RemoveSystemAbility(said)
    }

    /// Adds a new system ability
    pub fn add_systemability<A: RemoteStub + 'static>(said: i32, ability: A) -> i32 {
        info!("add system ability {}", said);
        let is_distributed = false;
        let dump_flags = DumpFlagPriority::Default;
        let capability = "";
        let permission = "";
        let stub = AbilityStub::new(ability);
        AddSystemAbility(
            said,
            Box::new(stub),
            AddSystemAbilityConfig {
                is_distributed,
                dump_flags: dump_flags as u32,
                capability: capability.to_string(),
                permission: permission.to_string(),
            },
        )
    }

    pub fn add_systemability_with_extra<A: RemoteStub + 'static>(
        said: i32,
        ability: A,
        is_distributed: bool,
        dump_flags: DumpFlagPriority,
        capability: &str,
        permission: &str,
    ) -> i32 {
        info!("add system ability {}", said);
        let stub = AbilityStub::new(ability);
        AddSystemAbility(
            said,
            Box::new(stub),
            AddSystemAbilityConfig {
                is_distributed,
                dump_flags: dump_flags as u32,
                capability: capability.to_string(),
                permission: permission.to_string(),
            },
        )
    }

    pub fn load_system_ability(said: i32, timeout: i32) -> Option<RemoteObj> {
        info!("load system ability {}", said);
        RemoteObj::from_sptr(LoadSystemAbility(said, timeout))
    }

    pub fn load_system_ability_with_callback(said: i32, on_success: fn(), on_fail: fn()) -> i32 {
        info!("load system ability {}", said);
        LoadSystemAbilityWithCallback(said, on_success, on_fail)
    }

    pub fn subscribe_system_ability(
        said: i32,
        on_add: fn(i32, &str),
        on_remove: fn(i32, &str),
    ) -> UnsubscribeHandler {
        info!("subscribe system ability {}", said);
        UnsubscribeHandler::new(Unsubscribe::Ability(SubscribeSystemAbility(
            said, on_add, on_remove,
        )))
    }

    /// # Example
    /// ```rust
    /// use samgr::definition::DOWNLOAD_SERVICE_ID;
    /// use samgr::manage::SystemAbilityManager;
    ///
    /// SystemAbilityManager::remove_system_ability(DOWNLOAD_SERVICE_IDD);
    /// ```
    pub fn add_ondemand_system_ability_info(said: i32, local_ability_manager_name: &str) -> i32 {
        info!("add ondemand system ability {} info", said);

        AddOnDemandSystemAbilityInfo(said, local_ability_manager_name)
    }

    pub fn unload_system_ability(said: i32) -> i32 {
        info!("unload system ability {}", said);
        UnloadSystemAbility(said)
    }

    pub fn cancel_unload_system_ability(said: i32) -> i32 {
        info!("cancel unload system ability {}", said);
        CancelUnloadSystemAbility(said)
    }

    /// # Example
    /// ```rust
    /// use samgr::definition::DOWNLOAD_SERVICE_ID;
    ///
    /// SystemAbilityManager::unload_all_idle_system_ability();
    /// ```
    pub fn unload_all_idle_system_ability(&self) -> i32 {
        info!("unload all idle system ability");
        UnloadAllIdleSystemAbility()
    }

    pub fn get_system_process_info(said: i32) -> SystemProcessInfo {
        info!("get system ability {} process info", said);
        GetSystemProcessInfo(said)
    }

    pub fn get_running_system_process() -> Vec<SystemProcessInfo> {
        info!("get running system ability process info");
        GetRunningSystemProcess()
    }

    ///
    pub fn send_strategy(s_type: i32, saids: Vec<i32>, level: i32, action: &str) -> i32 {
        let_cxx_string!(action = action);
        SendStrategy(s_type, saids, level, action)
    }

    pub fn get_common_event_extra_data_id_list(said: i32, extraids: &mut Vec<i64>, event_name: &str) -> i32 {
        let_cxx_string!(event_name = event_name);
        GetCommonEventExtraDataIdlist(said, extraids, &event_name)
    }

    pub fn subscribe_system_process(
        on_start: fn(&SystemProcessInfo),
        on_stop: fn(&SystemProcessInfo),
    ) -> UnsubscribeHandler {
        UnsubscribeHandler::new(Unsubscribe::Process(SubscribeSystemProcess(
            on_start, on_stop,
        )))
    }

    pub fn get_on_demand_reason_extra_date(extra_data_id: i64, parcel: &mut MsgParcel) -> i32 {
        GetOnDemandReasonExtraData(extra_data_id, parcel.pin_mut().unwrap())
    }
}

enum Unsubscribe {
    Ability(UniquePtr<UnSubscribeSystemAbilityHandler>),
    Process(UniquePtr<UnSubscribeSystemProcessHandler>),
}

pub struct UnsubscribeHandler {
    inner: Unsubscribe,
}

impl UnsubscribeHandler {
    fn new(inner: Unsubscribe) -> Self {
        Self { inner }
    }

    pub fn unsubscribe(self) {
        match self.inner {
            Unsubscribe::Ability(mut p) => p.pin_mut().UnSubscribe(),
            Unsubscribe::Process(mut p) => p.pin_mut().UnSubscribe(),
        }
    }
}
