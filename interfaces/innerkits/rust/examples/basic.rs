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

#![allow(missing_docs, unused)]

use std::ffi::{c_char, CString};

use hilog_rust::{debug, hilog, info};
use ipc::parcel::{MsgOption, MsgParcel};
use ipc::remote::RemoteStub;
use samgr::manage::SystemAbilityManager;

const LOG_LABEL: hilog_rust::HiLogLabel = hilog_rust::HiLogLabel {
    log_type: hilog_rust::LogType::LogCore,
    domain: 0xD001810,
    tag: "SamgrRust",
};

struct Request;

impl RemoteStub for Request {
    fn on_remote_request(
        &self,
        code: u32,
        data: &mut ipc::parcel::MsgParcel,
        reply: &mut ipc::parcel::MsgParcel,
    ) -> i32 {
        debug!(LOG_LABEL, "ReqeustStub On Remote");
        1
    }
}

fn main() {
    use samgr::definition::DOWNLOAD_SERVICE_ID;
    use samgr::manage::SystemAbilityManager;
}
