/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef SAMGR_INTERFACE_INNERKITS_COMMOM_INCLUDE_SAMGR_ERR_CODE_H
#define SAMGR_INTERFACE_INNERKITS_COMMOM_INCLUDE_SAMGR_ERR_CODE_H

namespace OHOS {
enum SamgrErrCode {
    SAMGR_OK = 0,

// common
    INVALID_SYSTEM_ABILITY_ID = 1000,
    INVALID_INPUT_PARA,
    PROFILE_NOT_EXIST,
    CALLBACK_NULL,
    CALLBACK_MAP_SIZE_LIMIT,
    INVALID_CALL_PROC,
    STATE_SCHEDULER_NULL,
    PROC_NOT_EXIST,
    SA_NOT_EXIST,
    ONDEMAND_SIZE_LIMIT,
    SUBSCRIBE_SIZE_LIMIT,
    ABILITY_MAP_SIZE_LIMIT,
    PROC_MAP_SIZE_LIMIT,
    PEND_LOAD_EVENT_SIZE_LIMIT,
    POST_TASK_FAIL,
    POST_TIMEOUT_TASK_FAIL,
    SAVE_FD_FAIL,
    CHECK_CALL_PROC_FAIL,
    ONDEMAND_SA_LIST_EMPTY,
    ACTIVE_SA_FAIL,
    IDLE_SA_FAIL,
    NOT_ONDEMAND_SA,
    SA_OBJ_NULL,
    LISTENER_NULL,
    SA_NOT_DISTRIBUTED,

// scheduler
    GET_SA_CONTEXT_FAIL = 2000,
    GET_PROC_CONTEXT_FAIL,
    PROC_STATE_NOT_STARTED,
    INVALID_SA_STATE,
    INVALID_PROC_STATE,
    SA_CONTEXT_NULL,
    SA_STATE_HANDLER_NULL,
    INVALID_SA_NEXT_STATE,
    INVALID_PROC_NEXT_STATE,
    TRANSIT_PROC_STATE_FAIL,
    TRANSIT_SA_STATE_FAIL,
    UPDATE_STATE_COUNT_FAIL,
    UNLOAD_EVENT_HANDLER_NULL,
    UNLOAD_REQUEST_NULL,
    SEND_EVENT_FAIL,
    INVALID_TIMED_EVENT_NAME,
    INVALID_TIMED_EVENT_PERSISTENCE,
    INVALID_TIMED_EVENT_INTERVAL,
    INVALID_SA_STATE_EVENT,
    INVALID_PROC_STATE_EVENT,
    INVALID_POLICY_TYPE,
    INVALID_SWITCH_EVENTID,
    INVALID_SWITCH_EVENT_NAME,
    ADD_COLLECT_FAIL,
    CREATE_EVENT_SUBSCRIBER_FAIL,
    GET_DEVICE_LIST_FAIL,
    INIT_DEVICE_MANAGER_FAIL,
    GET_EXTRA_DATA_FAIL,
    COMMON_EVENT_COLLECT_NULL,
    ADD_COLLECT_EVENT_FAIL,
    SUBSCRIBE_SWITCH_FAIL,
    UNSUBSCRIBE_SWITCH_FAIL,
    SWITCH_SUBSCRIBER_NULL,
    COLLECT_MANAGER_NULL,
    PROC_CONTEXT_NULL,
    PROC_STATE_HANDLER_NULL,
};
}
#endif // SAMGR_INTERFACE_INNERKITS_COMMOM_INCLUDE_SAMGR_ERR_CODE_H