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

use samgr::manage::SystemAbilityManager;
use samgr::DumpFlagPriority;

fn init() {
    #[cfg(gn_test)]
    super::init_access_token();
}

#[test]
fn basic() {
    init();
    let abilities = SystemAbilityManager::list_system_abilities();
    assert!(!abilities.is_empty());

    let abilities =
        SystemAbilityManager::list_system_abilities_with_dump_flag(DumpFlagPriority::High);
    assert!(!abilities.is_empty());
    let info = SystemAbilityManager::get_system_process_info(3706);
    assert_eq!("download_server", info.processName);
    let infos = SystemAbilityManager::get_running_system_process();
    assert!(!infos.is_empty());
}

#[test]
fn common_event() {
    init();
    let obj_j = SystemAbilityManager::check_system_ability(1494);
    if let Some(obj) = obj_j {
        let mut id_list:Vec<i64> = vec![];
        let ret = SystemAbilityManager::get_common_event_extra_data_id_list(1494, &mut id_list, "");
        assert_eq!(ret, 0);
        for id in id_list {
            println!("all extra id is {}", id)
        }

        let mut id_list:Vec<i64> = vec![];
        let ret = 
            SystemAbilityManager::get_common_event_extra_data_id_list(1494, &mut id_list, "usual.event.SCREEN_ON");
        assert_eq!(ret, 0);
        for id in id_list {
            println!("usual.event.SCREEN_ON event extra id is {}", id)
        }
    }
}
