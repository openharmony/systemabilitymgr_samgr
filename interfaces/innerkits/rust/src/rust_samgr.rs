/*
* Copyright (C) 2023 Huawei Device Co., Ltd.
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

extern crate ipc_rust;
use ipc_rust::{
    IRemoteObj, RemoteObjRef, FromRemoteObj, IRemoteBroker, MsgParcel,
    RemoteObj, InterfaceToken, String16, Result, get_context_object
};
use std::ffi::{c_char, CString};
use std::default::Default;
use hilog_rust::{info, error, hilog, HiLogLabel, LogType};
const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xd001800,
    tag: "rustSA"
};

pub enum ISystemAbilityManagerCode {
    /// get systemability code
    CodeGetSystemAbility = 2,
    /// add systemability code
    CodeAddSystemAbility = 3,
}

/// SAExtraProp is used to add_systemability
pub struct SAExtraProp {
    /// Set whether SA is distributed
    pub is_distributed: bool,
    /// Additional parameters for SA, default is 8
    pub dump_flags: u32,
    /// Additional parameters for SA, default is ""
    pub capability: String16,
    /// Additional parameters for SA, default is ""
    pub permission: String16,
}

impl Default for SAExtraProp {
    fn default() -> SAExtraProp {
        SAExtraProp {
            is_distributed: false,
            dump_flags: 8,
            capability: String16::new(""),
            permission: String16::new(""),
        }
    }
}

/// samgr interface
pub trait ISystemAbilityManager: IRemoteBroker {
    /// add_systemability
    fn add_systemability(&self, service: &RemoteObj, said: i32,  extra_prop: SAExtraProp) -> Result<()>;
    /// get_systemability
    fn get_systemability(&self, said: i32) -> Result<RemoteObj>;
}

impl FromRemoteObj for dyn ISystemAbilityManager {
    /// For example, convert RemoteObj to RemoteObjRef<dyn ITest>
    fn try_from(object: RemoteObj) -> Result<RemoteObjRef<dyn ISystemAbilityManager>> {
        Ok(RemoteObjRef::new(Box::new(SystemAbilityManagerProxy::from_remote_object(object)?)))
    }
}

/// get_service_proxy
pub fn get_service_proxy<T: FromRemoteObj + ?Sized>(said: i32) -> Result<RemoteObjRef<T>>
{
    let samgr_proxy = get_systemability_manager();
    let object = samgr_proxy.get_systemability(said)?;
    <T as FromRemoteObj>::try_from(object)
}

/// get_systemability_manager - get samgr proxy
pub fn get_systemability_manager() -> RemoteObjRef<dyn ISystemAbilityManager>
{
    let object = get_context_object().expect("samgr is null");
    let remote = <dyn ISystemAbilityManager as FromRemoteObj>::try_from(object);
    let remote = match remote {
        Ok(x) => x,
        Err(error) => {
            error!(LOG_LABEL, "convert RemoteObj to SystemAbilityManagerProxy failed: {}", error);
            panic!();
        }
    };
    remote
}

/// define rust samgr proxy
pub struct SystemAbilityManagerProxy {
    remote: RemoteObj,
}

impl SystemAbilityManagerProxy {
    fn from_remote_object(remote: RemoteObj) -> Result<Self> {
        Ok(Self {remote})
    }
}

impl IRemoteBroker for SystemAbilityManagerProxy {
    /// Get Remote object from proxy
    fn as_object(&self) -> Option<RemoteObj> {
        Some(self.remote.clone())
    }
}

impl ISystemAbilityManager for SystemAbilityManagerProxy {
    fn add_systemability(&self, service: &RemoteObj, said: i32,  extra_prop: SAExtraProp) -> Result<()>
    {    
        let mut data = MsgParcel::new().expect("MsgParcel is null");
        data.write(&InterfaceToken::new("ohos.samgr.accessToken"))?;
        data.write(&said)?;
        data.write(service)?;
        data.write(&extra_prop.is_distributed)?;
        data.write(&extra_prop.dump_flags)?;
        data.write(&extra_prop.capability)?;
        data.write(&extra_prop.permission)?;
        let reply = self.remote.send_request(
            ISystemAbilityManagerCode::CodeAddSystemAbility as u32, &data, false)?;
        let reply_value: i32 = reply.read()?;
        info!(LOG_LABEL, "register service result: {}", reply_value);
        if reply_value == 0 { Ok(())} else { Err(reply_value) }
    }

    fn get_systemability(&self, said: i32) -> Result<RemoteObj>
    {
        let mut data = MsgParcel::new().expect("MsgParcel is null");
        data.write(&InterfaceToken::new("ohos.samgr.accessToken"))?;
        data.write(&said)?;
        let reply = self.remote.send_request(
            ISystemAbilityManagerCode::CodeGetSystemAbility as u32, &data, false)?;
        let remote: RemoteObj = reply.read()?;
        Ok(remote)
    }
}