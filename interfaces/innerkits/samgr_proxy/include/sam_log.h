/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef SAMGR_PROXY_INCLUDE_SAM_LOG_H
#define SAMGR_PROXY_INCLUDE_SAM_LOG_H

#include "hilog/log.h"

namespace OHOS {
#undef LOG_DOMAIN
#ifdef SAMGR_PROXY
#define LOG_DOMAIN 0xD001810
#else
#define LOG_DOMAIN 0xD001800
#endif

#undef LOG_TAG
#ifdef SAMGR_PROXY
#define LOG_TAG "SA_CLIENT"
#else
#define LOG_TAG "SAMGR"
#endif

#ifdef HILOGF
#undef HILOGF
#endif

#ifdef HILOGE
#undef HILOGE
#endif

#ifdef HILOGW
#undef HILOGW
#endif

#ifdef HILOGI
#undef HILOGI
#endif

#ifdef HILOGD
#undef HILOGD
#endif

#define HILOGF(...) HILOG_FATAL(LOG_CORE, __VA_ARGS__)
#define HILOGE(...) HILOG_ERROR(LOG_CORE, __VA_ARGS__)
#define HILOGW(...) HILOG_WARN(LOG_CORE, __VA_ARGS__)
#define HILOGI(...) HILOG_INFO(LOG_CORE, __VA_ARGS__)
#define HILOGD(...) HILOG_DEBUG(LOG_CORE, __VA_ARGS__)
} // namespace OHOS

#endif // #ifndef SAMGR_PROXY_INCLUDE_SAM_LOG_H
