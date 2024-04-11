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

#ifndef SAMGR_RUST_TEST_MEM_H
#define SAMGR_RUST_TEST_MEM_H

#include <cstdint>
#include <memory>

#include "refbase.h"

namespace OHOS {

struct Foo : RefBase {
    Foo() = default;
    ~Foo();
    bool inner_ = true;
};

typedef sptr<Foo> SptrFoo;

class Keeper {
public:
    Keeper(sptr<Foo> inner);
    ~Keeper() = default;
    sptr<Foo> inner_;
};
std::unique_ptr<sptr<Foo>> GetUniqueSptr();
std::unique_ptr<Keeper> KeepUniqueSptr(std::unique_ptr<SptrFoo> foo);

} // namespace OHOS
#endif