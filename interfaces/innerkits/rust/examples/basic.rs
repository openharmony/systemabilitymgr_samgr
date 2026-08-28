// Copyright (C) 2024-2026 Huawei Device Co., Ltd.
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
use std::ptr;
use std::sync::atomic::{AtomicI32, Ordering};
use std::thread;
use std::time::Duration;

use ipc::parcel::MsgParcel;
use ipc::remote::RemoteObj;
use samgr::manage::{SystemAbilityManager, UnsubscribeHandler};

#[repr(C)]
struct NativeTokenInfoParams {
    dcaps_num: i32,
    perms_num: i32,
    acls_num: i32,
    dcaps: *const *const c_char,
    perms: *const *const c_char,
    acls: *const *const c_char,
    process_name: *const c_char,
    apl_str: *const c_char,
    uid: i32,
}

extern "C" {
    fn GetAccessTokenId(token_info: *mut NativeTokenInfoParams) -> u64;
    fn SetSelfTokenID(token_id: u64) -> i32;
}

#[derive(Clone, Copy)]
enum FlowApi {
    Legacy,
    ByUserId(i32),
}

struct ProcessSnapshot {
    process_name: String,
    pid: i32,
    uid: i32,
}

static SA_ADDED: AtomicI32 = AtomicI32::new(0);
static SA_REMOVED: AtomicI32 = AtomicI32::new(0);
static PROCESS_STARTED: AtomicI32 = AtomicI32::new(0);
static PROCESS_STOPPED: AtomicI32 = AtomicI32::new(0);
static LOAD_SUCCEEDED: AtomicI32 = AtomicI32::new(0);
static LOAD_FAILED: AtomicI32 = AtomicI32::new(0);

fn set_native_token() {
    let process_name = CString::new("samgr_rust_basic").expect("process name");
    let apl = CString::new("system_core").expect("apl");
    let mut token_info = NativeTokenInfoParams {
        dcaps_num: 0,
        perms_num: 0,
        acls_num: 0,
        dcaps: ptr::null(),
        perms: ptr::null(),
        acls: ptr::null(),
        process_name: process_name.as_ptr(),
        apl_str: apl.as_ptr(),
        uid: -1,
    };
    unsafe {
        let token_id = GetAccessTokenId(&mut token_info);
        let _ = SetSelfTokenID(token_id);
    }
}

fn reset_counts() {
    for counter in [
        &SA_ADDED,
        &SA_REMOVED,
        &PROCESS_STARTED,
        &PROCESS_STOPPED,
        &LOAD_SUCCEEDED,
        &LOAD_FAILED,
    ] {
        counter.store(0, Ordering::SeqCst);
    }
}

fn on_sa_added(said: i32, _device_id: &str) {
    SA_ADDED.fetch_add(1, Ordering::SeqCst);
    println!("SA added: sa_id={}", said);
}

fn on_sa_removed(said: i32, _device_id: &str) {
    SA_REMOVED.fetch_add(1, Ordering::SeqCst);
    println!("SA removed: sa_id={}", said);
}

fn on_load_success() {
    LOAD_SUCCEEDED.fetch_add(1, Ordering::SeqCst);
    println!("load callback: success");
}

fn on_load_fail() {
    LOAD_FAILED.fetch_add(1, Ordering::SeqCst);
    println!("load callback: fail");
}

fn subscribe_sa(api: FlowApi, said: i32) -> UnsubscribeHandler {
    match api {
        FlowApi::Legacy => SystemAbilityManager::subscribe_system_ability(said, on_sa_added, on_sa_removed),
        FlowApi::ByUserId(user_id) => SystemAbilityManager::subscribe_system_ability_by_user_id(
            said,
            on_sa_added,
            on_sa_removed,
            user_id,
        ),
    }
}

fn subscribe_process(api: FlowApi) -> UnsubscribeHandler {
    match api {
        FlowApi::Legacy => SystemAbilityManager::subscribe_system_process(
            |info| {
                PROCESS_STARTED.fetch_add(1, Ordering::SeqCst);
                println!("system process started: name={}, pid={}, uid={}", info.processName, info.pid, info.uid);
            },
            |info| {
                PROCESS_STOPPED.fetch_add(1, Ordering::SeqCst);
                println!("system process stopped: name={}, pid={}, uid={}", info.processName, info.pid, info.uid);
            },
        ),
        FlowApi::ByUserId(user_id) => SystemAbilityManager::subscribe_system_process_by_user_id(
            |info| {
                PROCESS_STARTED.fetch_add(1, Ordering::SeqCst);
                println!("system process started: name={}, pid={}, uid={}", info.processName, info.pid, info.uid);
            },
            |info| {
                PROCESS_STOPPED.fetch_add(1, Ordering::SeqCst);
                println!("system process stopped: name={}, pid={}, uid={}", info.processName, info.pid, info.uid);
            },
            user_id,
        ),
    }
}

fn load(api: FlowApi, said: i32, timeout: i32) -> Option<RemoteObj> {
    match api {
        FlowApi::Legacy => SystemAbilityManager::load_system_ability(said, timeout),
        FlowApi::ByUserId(user_id) => {
            SystemAbilityManager::load_system_ability_by_user_id(said, timeout, user_id)
        }
    }
}

fn check(api: FlowApi, said: i32) -> bool {
    match api {
        FlowApi::Legacy => SystemAbilityManager::check_system_ability(said).is_some(),
        FlowApi::ByUserId(user_id) => {
            SystemAbilityManager::check_system_ability_by_user_id(said, user_id).is_some()
        }
    }
}

fn get(api: FlowApi, said: i32) -> bool {
    match api {
        FlowApi::Legacy => SystemAbilityManager::get_system_ability(said).is_some(),
        FlowApi::ByUserId(user_id) => {
            SystemAbilityManager::get_system_ability_by_user_id(said, user_id).is_some()
        }
    }
}

fn process_info(api: FlowApi, said: i32) -> ProcessSnapshot {
    let info = match api {
        FlowApi::Legacy => SystemAbilityManager::get_system_process_info(said),
        FlowApi::ByUserId(user_id) => {
            SystemAbilityManager::get_system_process_info_by_user_id(said, user_id)
        }
    };
    ProcessSnapshot {
        process_name: info.processName,
        pid: info.pid,
        uid: info.uid,
    }
}

fn load_with_callback(api: FlowApi, said: i32) -> i32 {
    match api {
        FlowApi::Legacy => SystemAbilityManager::load_system_ability_with_callback(
            said,
            on_load_success,
            on_load_fail,
        ),
        FlowApi::ByUserId(user_id) => {
            SystemAbilityManager::load_system_ability_with_callback_by_user_id(
                said,
                on_load_success,
                on_load_fail,
                user_id,
            )
        }
    }
}

fn trigger_unload(remote: &RemoteObj) -> i32 {
    let mut request = MsgParcel::new();
    match remote.send_request(3, &mut request) {
        Ok(mut reply) => reply.read::<i32>().unwrap_or(-1),
        Err(_) => -1,
    }
}

fn wait_for_callbacks(before_removed: i32, before_stopped: i32) {
    for _ in 0..50 {
        if SA_REMOVED.load(Ordering::SeqCst) > before_removed
            && PROCESS_STOPPED.load(Ordering::SeqCst) > before_stopped
        {
            return;
        }
        thread::sleep(Duration::from_millis(100));
    }
}

fn empty_for_api(api: FlowApi, said: i32) -> bool {
    // GetSystemProcessInfo keeps the process context after stop; use CheckSystemAbility for unload state.
    !check(api, said)
}

fn wait_until_empty(api: FlowApi, said: i32) -> bool {
    // The default delayed-unload interval is 30 seconds; allow a small margin for callbacks.
    for _ in 0..350 {
        if empty_for_api(api, said) {
            return true;
        }
        thread::sleep(Duration::from_millis(100));
    }
    false
}

fn run_full_flow(api: FlowApi, said: i32, invalid_user_id: Option<i32>, timeout: i32) -> bool {
    reset_counts();
    let label = match api {
        FlowApi::Legacy => "LEGACY_FULL_FLOW",
        FlowApi::ByUserId(_) => "FULL_FLOW",
    };
    println!("{} phase=subscribe", label);
    let sa_subscription = subscribe_sa(api, said);
    let process_subscription = subscribe_process(api);

    let loaded_remote = load(api, said, timeout);
    let loaded = loaded_remote.is_some();
    println!("load: {}", loaded);
    let checked = check(api, said);
    println!("check: {}", checked);
    let fetched = get(api, said);
    println!("get: {}", fetched);
    let info = process_info(api, said);
    println!("process: name={}, pid={}, uid={}", info.process_name, info.pid, info.uid);

    let callback_request = load_with_callback(api, said);
    println!("load callback request result: {}", callback_request);
    thread::sleep(Duration::from_millis(300));
    let before_removed = SA_REMOVED.load(Ordering::SeqCst);
    let before_stopped = PROCESS_STOPPED.load(Ordering::SeqCst);
    let unload_request = loaded_remote.as_ref().map(trigger_unload).unwrap_or(-1);
    println!("unload request result: {}", unload_request);
    drop(loaded_remote);
    wait_for_callbacks(before_removed, before_stopped);

    let sa_added = SA_ADDED.load(Ordering::SeqCst);
    let sa_removed = SA_REMOVED.load(Ordering::SeqCst);
    let process_started = PROCESS_STARTED.load(Ordering::SeqCst);
    let process_stopped = PROCESS_STOPPED.load(Ordering::SeqCst);
    let (sa_unsubscribe, process_unsubscribe) = match api {
        FlowApi::Legacy => {
            sa_subscription.unsubscribe();
            process_subscription.unsubscribe();
            (0, 0)
        }
        FlowApi::ByUserId(_) => (
            sa_subscription.unsubscribe_by_user_id(),
            process_subscription.unsubscribe_by_user_id(),
        ),
    };
    println!(
        "{} subscriptions: sa_added={} sa_removed={} process_started={} process_stopped={} sa_unsubscribe={} process_unsubscribe={}",
        label,
        sa_added,
        sa_removed,
        process_started,
        process_stopped,
        sa_unsubscribe,
        process_unsubscribe
    );

    let counts = (sa_added, sa_removed, process_started, process_stopped);
    let second_remote = load(api, said, timeout);
    let second_unload = second_remote.as_ref().map(trigger_unload).unwrap_or(-1);
    println!("post-unsubscribe unload request result: {}", second_unload);
    drop(second_remote);
    thread::sleep(Duration::from_millis(500));
    let callbacks_unchanged = counts
        == (
            SA_ADDED.load(Ordering::SeqCst),
            SA_REMOVED.load(Ordering::SeqCst),
            PROCESS_STARTED.load(Ordering::SeqCst),
            PROCESS_STOPPED.load(Ordering::SeqCst),
        );
    println!("{} callbacks_unchanged_after_unsubscribe: {}", label, callbacks_unchanged);

    let empty_after_unload = wait_until_empty(api, said);
    println!("{} empty_after_unload: {}", label, empty_after_unload);
    let invalid_user_empty = invalid_user_id.map_or(true, |user_id| {
        empty_for_api(FlowApi::ByUserId(user_id), said)
    });
    println!("{} invalid_user_empty: {}", label, invalid_user_empty);

    let pass = loaded
        && checked
        && fetched
        && info.process_name == "multi_instance_test_probe"
        && info.pid > 0
        && info.uid == 1000
        && callback_request == 0
        && LOAD_SUCCEEDED.load(Ordering::SeqCst) > 0
        && LOAD_FAILED.load(Ordering::SeqCst) == 0
        && sa_added > 0
        && sa_removed > 0
        && process_started > 0
        && process_stopped > 0
        && sa_unsubscribe == 0
        && process_unsubscribe == 0
        && callbacks_unchanged
        && empty_after_unload
        && invalid_user_empty;
    println!("{}_RESULT:{}", label, if pass { "PASS" } else { "FAIL" });
    println!("{}", if pass { "PASS" } else { "FAIL" });
    pass
}

fn main() {
    set_native_token();
    let args: Vec<String> = std::env::args().collect();
    let result = match args.get(1).map(String::as_str) {
        Some("full-flow") if args.len() == 6 => {
            match (args[2].parse::<i32>(), args[3].parse::<i32>(), args[4].parse::<i32>(), args[5].parse::<i32>()) {
                (Ok(said), Ok(user_id), Ok(invalid_user_id), Ok(timeout)) => {
                    Some(run_full_flow(FlowApi::ByUserId(user_id), said, Some(invalid_user_id), timeout))
                }
                _ => None,
            }
        }
        Some("legacy-full-flow") if args.len() == 4 => {
            match (args[2].parse::<i32>(), args[3].parse::<i32>()) {
                (Ok(said), Ok(timeout)) => Some(run_full_flow(FlowApi::Legacy, said, None, timeout)),
                _ => None,
            }
        }
        _ => None,
    };
    match result {
        Some(true) => {}
        Some(false) => std::process::exit(1),
        None => {
            eprintln!("usage: samgr_rust_basic full-flow <said> <user_id> <invalid_user_id> <timeout>");
            eprintln!("       samgr_rust_basic legacy-full-flow <said> <timeout>");
            std::process::exit(2);
        }
    }
}
