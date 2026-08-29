// Copyright (C) 2026 Huawei Device Co., Ltd.
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

pub(crate) use ffi::*;

#[cxx::bridge(namespace = "OHOS::SamgrRust")]
mod ffi {
    unsafe extern "C++" {
        include!("wrapper.rs.h");
        include!("system_ability_manager_wrapper.h");
        include!("status_change_wrapper.h");

        #[namespace = "OHOS"]
        type SptrIRemoteObject = crate::wrapper::SptrIRemoteObject;

        type UnSubscribeSystemAbilityHandler = crate::wrapper::UnSubscribeSystemAbilityHandler;
        type UnSubscribeSystemProcessHandler = crate::wrapper::UnSubscribeSystemProcessHandler;
        type SystemProcessInfo = crate::wrapper::SystemProcessInfo;

        fn LoadSystemAbilityByUserId(
            said: i32,
            timeout: i32,
            user_id: i32,
        ) -> UniquePtr<SptrIRemoteObject>;

        fn LoadSystemAbilityWithCallbackByUserId(
            said: i32,
            on_success: fn(),
            on_fail: fn(),
            user_id: i32,
        ) -> i32;

        fn GetSystemAbilityByUserId(
            said: i32,
            user_id: i32,
        ) -> UniquePtr<SptrIRemoteObject>;

        fn CheckSystemAbilityByUserId(
            said: i32,
            user_id: i32,
        ) -> UniquePtr<SptrIRemoteObject>;

        fn SubscribeSystemAbilityByUserId(
            said: i32,
            on_add: fn(i32, &str),
            on_remove: fn(i32, &str),
            user_id: i32,
        ) -> UniquePtr<UnSubscribeSystemAbilityHandler>;

        fn UnSubscribeSystemAbilityByUserId(
            self: Pin<&mut UnSubscribeSystemAbilityHandler>,
            user_id: i32,
        ) -> i32;

        fn GetSystemProcessInfoByUserId(said: i32, user_id: i32) -> SystemProcessInfo;

        fn SubscribeSystemProcessByUserId(
            on_start: fn(&SystemProcessInfo),
            on_stop: fn(&SystemProcessInfo),
            user_id: i32,
        ) -> UniquePtr<UnSubscribeSystemProcessHandler>;

        fn UnSubscribeSystemProcessByUserId(
            self: Pin<&mut UnSubscribeSystemProcessHandler>,
            user_id: i32,
        ) -> i32;
    }
}
