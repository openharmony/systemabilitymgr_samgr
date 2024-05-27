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

use std::pin::Pin;

pub(crate) use ffi::*;
use ipc::cxx_share::RemoteStubWrapper;
use ipc::remote::RemoteStub;

#[cxx::bridge(namespace = "OHOS::SamgrRust")]
mod ffi {

    #[derive(Debug)]
    struct SystemProcessInfo {
        processName: String,
        pid: i32,
        uid: i32,
    }

    struct AddSystemAbilityConfig {
        is_distributed: bool,
        dump_flags: u32,
        capability: String,
        permission: String,
    }

    extern "Rust" {
        type AbilityStub;

        fn on_remote_request(
            self: &mut AbilityStub,
            code: u32,
            data: Pin<&mut MessageParcel>,
            reply: Pin<&mut MessageParcel>,
        ) -> i32;

        fn dump(self: &mut AbilityStub, fd: i32, args: Vec<String>) -> i32;

    }

    unsafe extern "C++" {
        include!("system_ability_manager_wrapper.h");
        include!("status_change_wrapper.h");

        #[namespace = OHOS]
        type SptrIRemoteObject = ipc::cxx_share::SptrIRemoteObject;

        #[namespace = "OHOS"]
        type MessageParcel = ipc::cxx_share::MessageParcel;

        pub type UnSubscribeSystemAbilityHandler;
        pub type UnSubscribeSystemProcessHandler;
        fn LoadSystemAbility(said: i32, timeout: i32) -> UniquePtr<SptrIRemoteObject>;

        fn LoadSystemAbilityWithCallback(said: i32, on_success: fn(), on_fail: fn()) -> i32;

        fn GetSystemAbility(said: i32) -> UniquePtr<SptrIRemoteObject>;

        fn GetContextManager() -> UniquePtr<SptrIRemoteObject>;

        fn CheckSystemAbility(said: i32) -> UniquePtr<SptrIRemoteObject>;

        fn GetSystemAbilityWithDeviceId(
            said: i32,
            deviceId: &CxxString,
        ) -> UniquePtr<SptrIRemoteObject>;

        fn RemoveSystemAbility(said: i32) -> i32;

        fn CheckSystemAbilityWithDeviceId(
            said: i32,
            deviceId: &CxxString,
        ) -> UniquePtr<SptrIRemoteObject>;

        fn ListSystemAbilities() -> Vec<String>;
        fn ListSystemAbilitiesWithDumpFlag(dump_flag: u32) -> Vec<String>;

        fn SubscribeSystemAbility(
            said: i32,
            on_add: fn(i32, &str),
            on_remove: fn(i32, &str),
        ) -> UniquePtr<UnSubscribeSystemAbilityHandler>;

        fn UnSubscribe(self: Pin<&mut UnSubscribeSystemAbilityHandler>);
        fn AddOnDemandSystemAbilityInfo(said: i32, localAbilityManagerName: &str) -> i32;

        fn UnloadSystemAbility(said: i32) -> i32;
        fn CancelUnloadSystemAbility(said: i32) -> i32;
        fn UnloadAllIdleSystemAbility() -> i32;
        fn GetSystemProcessInfo(said: i32) -> SystemProcessInfo;

        fn GetRunningSystemProcess() -> Vec<SystemProcessInfo>;
        fn SendStrategy(
            s_type: i32,
            saids: Vec<i32>,
            level: i32,
            action: Pin<&mut CxxString>,
        ) -> i32;

        fn GetCommonEventExtraDataIdlist(
            said: i32,
            extraids: &mut Vec<i64>,
            event_name: &CxxString,
        ) -> i32;

        fn AddSystemAbility(
            said: i32,
            ability: Box<AbilityStub>,
            config: AddSystemAbilityConfig,
        ) -> i32;

        fn SubscribeSystemProcess(
            on_start: fn(&SystemProcessInfo),
            on_stop: fn(&SystemProcessInfo),
        ) -> UniquePtr<UnSubscribeSystemProcessHandler>;

        fn UnSubscribe(self: Pin<&mut UnSubscribeSystemProcessHandler>);

        fn GetOnDemandReasonExtraData(extra_data_id: i64, parcel: Pin<&mut MessageParcel>) -> i32;
    }
}

pub struct AbilityStub {
    remote: RemoteStubWrapper,
}

impl AbilityStub {
    pub fn new<A: RemoteStub + 'static>(remote: A) -> Self {
        Self {
            remote: RemoteStubWrapper::new(remote),
        }
    }

    fn on_remote_request(
        &mut self,
        code: u32,
        data: Pin<&mut MessageParcel>,
        reply: Pin<&mut MessageParcel>,
    ) -> i32 {
        self.remote.on_remote_request(code, data, reply)
    }

    fn dump(&mut self, fd: i32, args: Vec<String>) -> i32 {
        self.remote.dump(fd, args)
    }
}
