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

| 构建目标 | 类型 | 产物 |
|---|---|---|
| `services/samgr/native:samgr` | 可执行文件 | `/system/bin/samgr` |
| `interfaces/innerkits/samgr_proxy:samgr_proxy` | 共享库 | 其他子系统依赖的SDK |
| `interfaces/innerkits/common:samgr_common` | 共享库 | Profile解析、错误码 |
| `interfaces/innerkits/dynamic_cache:dynamic_cache` | 共享库 | SA查询缓存 |
| `interfaces/innerkits/rust:samgr_rust` | Rust crate | Rust绑定 |

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

**设备调试**: `hidumper -s 0 -a "-l"`（列出所有SA）· `hilog | grep -i samgr` · `param get | grep samgr`。
