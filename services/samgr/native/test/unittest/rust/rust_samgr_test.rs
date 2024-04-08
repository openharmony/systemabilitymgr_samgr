// Copyright (C) 2023 Huawei Device Co., Ltd.
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

extern crate audio_rust_ipc;
extern crate audio_rust_sa;
extern crate ipc_rust;
extern crate rust_samgr;
extern crate test_access_token;
use std::ffi::{c_char, CString};

use hilog_rust::{error, hilog, HiLogLabel, LogType};
use ipc_rust::{IRemoteBroker, RemoteObjRef};
const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xd001800,
    tag: "rustSA",
};
use audio_rust_ipc::{ITest, TestStub};
use audio_rust_sa::{TestService, TEST_AUDIO_ID};
use rust_samgr::{get_service_proxy, get_systemability_manager, SAExtraProp};
use test_access_token::init_access_token;

fn get_test_service() -> RemoteObjRef<dyn ITest> {
    let object = get_service_proxy::<dyn ITest>(TEST_AUDIO_ID);
    let object = match object {
        Ok(object) => object,
        Err(error) => {
            error!(
                LOG_LABEL,
                "convert RemoteObj to TestProxy failed: {}", error
            );
            panic!();
        }
    };
    object
}

#[test]
fn test_add_access_token() {
    init_access_token();
}

#[test]
fn add_systemability_test() {
    init_access_token();
    let service = TestStub::new_remote_stub(TestService).expect("create TestService failed");
    let sapro = SAExtraProp::default();
    let samgr_proxy = get_systemability_manager();
    samgr_proxy
        .add_systemability(&service.as_object().unwrap(), TEST_AUDIO_ID, sapro)
        .expect("add_systemability failed");
}

#[test]
fn unload_systemability_test() {
    init_access_token();
    let service = TestStub::new_remote_stub(TestService).expect("create TestService failed");
    let sapro = SAExtraProp::default();
    let samgr_proxy = get_systemability_manager();
    samgr_proxy
        .add_systemability(&service.as_object().unwrap(), TEST_AUDIO_ID, sapro)
        .expect("add_systemability failed");
    samgr_proxy
        .unload_systemability(TEST_AUDIO_ID)
        .expect("unload_systemability failed");
}

#[test]
fn get_service_test() {
    init_access_token();
    let service = TestStub::new_remote_stub(TestService).expect("create TestService failed");
    let sapro = SAExtraProp::default();
    let samgr_proxy = get_systemability_manager();
    samgr_proxy
        .add_systemability(&service.as_object().unwrap(), TEST_AUDIO_ID, sapro)
        .expect("add_systemability failed");
    get_test_service();
}

#[test]
fn test_request_concurent() {
    init_access_token();
    let service = TestStub::new_remote_stub(TestService).expect("create TestService failed");
    let sapro = SAExtraProp::default();
    let samgr_proxy = get_systemability_manager();
    samgr_proxy
        .add_systemability(&service.as_object().unwrap(), TEST_AUDIO_ID, sapro)
        .expect("add_systemability failed");
    let remote = get_test_service();
    let value = remote.request_concurent(false).expect("should return true");
    assert!(value);
}
