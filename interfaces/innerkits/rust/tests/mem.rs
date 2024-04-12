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

use ffi::{GetUniqueSptr, KeepUniqueSptr};

#[cxx::bridge(namespace = "OHOS")]
mod ffi {
    unsafe extern "C++" {
        include!("samgr_rust_test_mem.h");
        type Keeper;
        type SptrFoo;
        fn GetUniqueSptr() -> UniquePtr<SptrFoo>;
        fn KeepUniqueSptr(s: UniquePtr<SptrFoo>) -> UniquePtr<Keeper>;
    }
}

#[test]
fn test() {
    let w = GetUniqueSptr();
    unsafe {
        assert_eq!(g_testDrop, 0);
    }
    drop(w);
    unsafe {
        assert_eq!(g_testDrop, 1);
    }
    let w = GetUniqueSptr();
    let keeper = KeepUniqueSptr(w);
    unsafe {
        assert_eq!(g_testDrop, 1);
    }
    drop(keeper);
    unsafe {
        assert_eq!(g_testDrop, 2);
    }
}

#[link(name = "samgr_rust_test_c")]
extern "C" {
    static g_testDrop: i32;
}
