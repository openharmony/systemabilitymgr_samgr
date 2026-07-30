# AGENT.md — samgr（系统服务管理器）

> 路径: `foundation/systemabilitymgr/samgr` | 子系统: `systemabilitymgr` | 仅标准系统
>
> **维护规则**: 使用agent进行代码修改时，如新增代码结构或功能特性，必须同步更新本AGENT文档。

## 代码地图

```
interfaces/innerkits/
  samgr_proxy/include/     # 对外SDK头文件 — if_system_ability_manager.h 是核心接口
  common/include/          # SaProfile结构、错误码(samgr_err_code.h)、parse_util.h
  dynamic_cache/           # SA查询缓存（减少到samgr的IPC）
  rust/                    # Rust绑定(samgr_rust)
frameworks/native/source/  # Proxy/Stub实现（samgr二进制和SDK共用）
services/
  samgr/native/source/     # samgr进程核心 — system_ability_manager.cpp 是枢纽
    schedule/              # 状态机+调度器（按需加载/卸载生命周期）
    collect/               # 事件采集器（参数、定时、网络、公共事件、开关）
  lsamgr/                  # LocalAbilityManager Proxy（samgr→SA进程方向）
  dfx/                     # HiSysEvent适配
  common/                  # parse_util.cpp（共享Profile解析器）
utils/native/              # tools.h, test_log.h
etc/                       # samgr.para, samgr_standard.cfg
```

> 若子目录包含自己的 `AGENTS.md`，则该目录范围内的任务以子目录文档为准。

| 构建目标　　　　　　　　　　　　　　　　　　　　　 | 类型　　　 | 产物　　　　　　　　|
| ----------------------------------------------------| ------------| ---------------------|
| `services/samgr/native:samgr`　　　　　　　　　　　| 可执行文件 | `/system/bin/samgr` |
| `interfaces/innerkits/samgr_proxy:samgr_proxy`　　 | 共享库　　 | 其他子系统依赖的SDK |
| `interfaces/innerkits/common:samgr_common`　　　　 | 共享库　　 | Profile解析、错误码 |
| `interfaces/innerkits/dynamic_cache:dynamic_cache` | 共享库　　 | SA查询缓存　　　　　|
| `interfaces/innerkits/rust:samgr_rust`　　　　　　 | Rust crate | Rust绑定　　　　　　|

## 知识路由

| 问题 | 先读 |
|---|---|
| 注册/查询/卸载SA | `if_system_ability_manager.h` → `system_ability_manager_proxy.cpp` |
| 新增SA ID | `system_ability_definition.h`（枚举）。**必须三处同步**: 此文件 + `hidumper/dump_utils.cpp`的saNameMap_ + `rust/src/cxx/definition.rs` |
| 按需加载流程 | `sa_profiles.h`（OnDemandEvent）→ `schedule/system_ability_state_scheduler.cpp` |
| IPC接口码定义 | `samgr_ipc_interface_code.h`（enum SamgrInterfaceCode） |
| Stub分发逻辑 | `system_ability_manager_stub.cpp` |
| 事件采集插件 | `collect/icollect_plugin.h` → `collect/device_status_collect_manager.cpp` |
| SA死亡处理 | `ability_death_recipient.cpp` |
| 错误码含义 | `samgr_err_code.h` |
| HiSysEvent事件定义 | `hisysevent.yaml` |
| 获取samgr代理 | `iservice_registry.h` → `SystemAbilityManagerClient` |
| dbinder/跨设备SA | `rpc_callback_imp.cpp` |

**关键术语**: `said`（SA ID，范围0x1–0x00ffffff）、ondemand（按需加载/卸载）、LSA（进程内LocalAbilityManager）、BootPhase（启动优先级: BootStart > CoreStart > OtherStart）、dbinder（跨设备RPC）。

**按任务类型定位**:

| 任务 | 起点 |
|---|---|
| 新增/注册SA | `system_ability_definition.h` → `if_system_ability_manager.h` |
| 修复IPC分发bug | `samgr_ipc_interface_code.h` → `system_ability_manager_stub.cpp` |
| 新增按需事件采集器 | `collect/icollect_plugin.h` → `collect/device_status_collect_manager.cpp` |
| 修改状态机逻辑 | `schedule/system_ability_state_machine.cpp` → `schedule/system_ability_state_scheduler.cpp` |
| 新增HiSysEvent | `hisysevent.yaml` → `services/dfx/include/hisysevent_adapter.h` |

**编辑任何文件前，先声明**: (1) 任务类别、(2) 已读文档、(3) 适用约束。

## 专家约束

1. **SA ID三处同步**: 新增SA ID必须同时更新 `system_ability_definition.h`、`hidumper/dump_utils.cpp`、`rust/cxx/definition.rs`。
2. **线程安全**: `abilityMap_` 由 `shared_mutex` 保护 — 读用 `shared_lock`，写用 `unique_lock`，绝不可绕过。
3. **强制FFRT**: 编译定义了 `SAMGR_USE_FFRT`。并发使用 `FfrtHandler`，禁止 `std::thread`。
4. **CFI启用**: 所有目标开启CFI+CFI-cross-DSO。需豁免的函数写入 `cfi_blocklist.txt`。
5. **监听器上限**: 同进程 SubscribeSystemAbility 监听器最多256个。
6. **IPC令牌**: 所有IPC请求必须携带 `u"ohos.samgr.accessToken"`。
7. **MAX_SERVICES**: `abilityMap_` 有硬性容量上限，超过拒绝注册。
8. **safwk ↔ samgr 双向依赖**（设计如此）: samgr → safwk `system_ability_ondemand_reason`；safwk → samgr `samgr_proxy` + `samgr_common`。通过静态库/动态库分层解耦。
9. **samgr禁止include业务SA头文件** — 只通过 `IRemoteObject` + said 交互。
10. **所有写状态的 OnRemoteRequest 路径必须做 AccessToken 校验**。
11. **IPC接口码稳定性（修改前需确认）**: 禁止新增、删除或重排 `SamgrInterfaceCode` 枚举中的已有项。新码只能追加到枚举末尾。重排会静默破坏全系统跨进程IPC。
12. **生成文件**: Rust绑定(`interfaces/innerkits/rust/src/cxx/`)、hidumper saNameMap_(`base/hiviewdfx/hidumper/`)、`hisysevent.yaml`生成配置均为派生产物 — 编辑源文件，禁止直接修改生成输出。
13. **DFX必填**: 新增SA加载/卸载/错误路径必须调用HiSysEvent适配器(`services/dfx/include/hisysevent_adapter.h`)。新增阻塞型IPC或调度操作必须用 `SamgrXCollie`(`samgr_xcollie.h`，默认60秒)包裹。事件名/参数必须与 `hisysevent.yaml` schema一致。

**反模式**: 硬编码SA ID而非用枚举 · `GetSystemAbility`返回值不判空 · 新增Collector不在`DeviceStatusCollectManager`注册 · 不加锁直接操作`abilityMap_`。

## 编译与测试

```bash
# 编译samgr组件
./build.sh --product-name {product} --build-target samgr

# 编译特定目标
./build.sh --product-name {product} \
  --build-target foundation/systemabilitymgr/samgr/services/samgr/native:samgr

# 单元测试（bundle.json → build.test 中定义）
./build.sh --product-name {product} \
  --build-target foundation/systemabilitymgr/samgr/services/samgr/native/test:unittest

# 主要测试目标: SystemAbilityMgrTest, SystemAbilityMgrOndemandTest,
#   SystemAbilityStateMachineTest, DeviceStatusCollectManagerTest

# Fuzz测试
./build.sh --product-name {product} \
  --build-target foundation/systemabilitymgr/samgr/test/fuzztest:fuzztest
# 目标: samgr_fuzzer, systemabilitymanager_fuzzer, samgrdumper_fuzzer
```

**特性开关** (`config.gni`): `samgr_enable_delay_dbinder`(T)、`samgr_enable_extend_load_timeout`(F)、`samgr_support_multi_instance`(F)。

**设备调试**（均通过 `hidumper -s 0 -a "<参数>"`，SA ID 0 = samgr）:
```bash
hidumper -s 0 -a "-l"                          # 列出所有SA状态
hidumper -s 0 -a "-sa <said>"                  # 查询指定SA状态
hidumper -s 0 -a "-sm <ACTIVE|IDLE|NOT_LOADED>" # 按状态筛选SA
hidumper -s 0 -a "-p <进程名>"                  # 查询进程状态
hidumper -s 0 -a "--listener -l -sa"           # 所有SA监听关系
hidumper -s 0 -a "--listener -sa <said>"       # 谁在监听此SA
hidumper -s 0 -a "--ipc <进程名|all> --start-stat"  # 启动IPC统计（后接 --stat / --stop-stat）
hidumper -s 0 -a "--ffrt <pid1|pid2> --start-stat"  # 启动FFRT统计（后接 --stat / --stop-stat）
hilog -T SAMGR                                 # hilog标签: SAMGR，domain: 0xD001800
param get bootevent.samgr.ready                # "true" = samgr初始化完成
```

**静态分析**: CFI(`cfi=true`、`cfi_cross_dso=true`)和PAC(`branch_protector_ret="pac_ret"`)在每个BUILD.gn的 `sanitize` 块中强制开启。编译出现CFI链接错误时，仅限有充分理由时添加到 `cfi_blocklist.txt`。ASAN调试构建: `./build.sh --product-name {product} --gn-args is_asan=true --build-target samgr`。

**完成标准**: 报告完成前: (1) 展示编译退出码0、(2) 展示至少受影响的 `SystemAbilityMgr*` 测试套通过、(3) 若修改了DFX路径，验证HiSysEvent适配器调用与 `hisysevent.yaml` schema一致、(4) 若无法运行验证，明确说明已验证和未验证项并列出残留风险。
