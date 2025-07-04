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

#ifndef SAMGR_TEST_UNITTEST_REF_COUNT_COLLECT_TEST_H
#define SAMGR_TEST_UNITTEST_REF_COUNT_COLLECT_TEST_H

#include "gtest/gtest.h"
#include "ref_count_collect.h"

namespace OHOS {
class RefCountCollectTest : public testing::Test {
protected:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};
} // namespace OHOS
#endif /* SAMGR_TEST_UNITTEST_REF_COUNT_COLLECT_TEST_H */