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

#include "samgr_rust_test_mem.h"

#include <cstdint>
#include <iostream>
#include <memory>

#include "refbase.h"
namespace OHOS {
extern "C" {
int32_t g_testDrop = 0;
}

Foo::~Foo()
{
    g_testDrop += 1;
}

Keeper::Keeper(sptr<Foo> inner) : inner_(inner)
{
}

std::unique_ptr<sptr<Foo>> GetUniqueSptr()
{
    auto foo = sptr<Foo>::MakeSptr();
    return std::make_unique<sptr<Foo>>(std::move(foo));
}

std::unique_ptr<Keeper> KeepUniqueSptr(std::unique_ptr<sptr<Foo>> foo)
{
    return std::make_unique<Keeper>(std::move(*foo.release()));
}
} // namespace OHOS