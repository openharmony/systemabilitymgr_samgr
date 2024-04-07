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

#![allow(unused, missing_docs)]
use std::ffi::{c_char, CString};
use std::sync::Once;
use std::thread;

use hilog_rust::{debug, hilog};
use system_ability_fwk::cxx_share::SystemAbilityOnDemandReason;
pub const LOG_LABEL: hilog_rust::HiLogLabel = hilog_rust::HiLogLabel {
    log_type: hilog_rust::LogType::LogCore,
    domain: 0xD001800,
    tag: "SAMGR_RUST",
};

const AUDIO_SA: i32 = 1488;

use ipc::parcel::MsgParcel;
use ipc::remote::RemoteStub;
use samgr::manage::SystemAbilityManager;

fn init() {
    #[cfg(gn_test)]
    super::init_access_token();
}

struct TestRemote;

impl RemoteStub for TestRemote {
    fn on_remote_request(
        &self,
        code: u32,
        data: &mut ipc::parcel::MsgParcel,
        reply: &mut ipc::parcel::MsgParcel,
    ) -> i32 {
        0
    }
}
#[test]
fn basic_test() {
    init();
    let audio = SystemAbilityManager::load_system_ability(3706, 100000).unwrap();

    let mut data = MsgParcel::new();
    for i in 0..10 {
        thread::sleep(std::time::Duration::from_secs(1));
        let mut reply = audio.send_request(2, &mut data).unwrap();
    }
}
