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

enum Shift {
    Critical = 0,
    High,
    Normal,
    Default,
    Proto,
}

const DUMP_FLAG_PRIORITY_CRITICAL: u32 = 1 << Shift::Critical as usize;
const DUMP_FLAG_PRIORITY_HIGH: u32 = 1 << Shift::High as usize;
const DUMP_FLAG_PRIORITY_NORMAL: u32 = 1 << Shift::Normal as usize;

const DUMP_FLAG_PRIORITY_DEFAULT: u32 = 1 << Shift::Default as usize;
const DUMP_FLAG_PRIORITY_ALL: u32 = DUMP_FLAG_PRIORITY_CRITICAL
    | DUMP_FLAG_PRIORITY_HIGH
    | DUMP_FLAG_PRIORITY_NORMAL
    | DUMP_FLAG_PRIORITY_DEFAULT;
const DUMP_FLAG_PROTO: u32 = 1 << Shift::Proto as usize;

#[repr(u32)]
pub enum DumpFlagPriority {
    Critical = DUMP_FLAG_PRIORITY_CRITICAL,
    High = DUMP_FLAG_PRIORITY_HIGH,
    Normal = DUMP_FLAG_PRIORITY_NORMAL,
    Default = DUMP_FLAG_PRIORITY_DEFAULT,
    All = DUMP_FLAG_PRIORITY_ALL,
    Proto = DUMP_FLAG_PROTO,
}
