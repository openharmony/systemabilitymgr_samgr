# AGENT.md — samgr (System Ability Manager)

> Path: `foundation/systemabilitymgr/samgr` | Subsystem: `systemabilitymgr` | Standard system only
>
> **Maintenance rule**: When modifying code via agent, if new code structures or features are added, this document must be updated synchronously.

## Code Map

```
interfaces/innerkits/
  samgr_proxy/include/     # Public SDK headers — ISYSTEM_ABILITY_MANAGER_H is the root interface
  common/include/          # SaProfile struct, error codes (samgr_err_code.h), parse_util.h
  dynamic_cache/           # SA query cache (avoids repeated IPC to samgr)
  rust/                    # Rust bindings (samgr_rust)
frameworks/native/source/  # Proxy/Stub impl shared by samgr binary and SDK consumers
services/
  samgr/native/source/     # samgr binary core — system_ability_manager.cpp is the hub
    schedule/              # State machine + scheduler (on-demand load/unload lifecycle)
    collect/               # Event collectors (param, timed, networking, common_event, switch)
  lsamgr/                  # Proxy to LocalAbilityManager (samgr → SA process direction)
  dfx/                     # HiSysEvent adapter
  common/                  # parse_util.cpp (shared profile parser)
utils/native/              # tools.h, test_log.h
etc/                       # samgr.para, samgr_standard.cfg
```

| Build target | Type | Output |
|---|---|---|
| `services/samgr/native:samgr` | executable | `/system/bin/samgr` |
| `interfaces/innerkits/samgr_proxy:samgr_proxy` | shared lib | SDK for other subsystems |
| `interfaces/innerkits/common:samgr_common` | shared lib | Profile parser, error codes |
| `interfaces/innerkits/dynamic_cache:dynamic_cache` | shared lib | SA query cache |
| `interfaces/innerkits/rust:samgr_rust` | rust crate | Rust bindings |

## Knowledge Routing

| Problem | Read first |
|---|---|
| Register/query/unload SA | `if_system_ability_manager.h` → `system_ability_manager_proxy.cpp` |
| Add new SA ID | `system_ability_definition.h` (enum). **Must sync 3 places**: here + `hidumper/dump_utils.cpp` saNameMap_ + `rust/src/cxx/definition.rs` |
| On-demand load flow | `sa_profiles.h` (OnDemandEvent) → `schedule/system_ability_state_scheduler.cpp` |
| IPC code definitions | `samgr_ipc_interface_code.h` (enum SamgrInterfaceCode) |
| Stub dispatch | `system_ability_manager_stub.cpp` |
| Event collector plugins | `collect/icollect_plugin.h` → `collect/device_status_collect_manager.cpp` |
| SA death handling | `ability_death_recipient.cpp` |
| Error codes | `samgr_err_code.h` |
| HiSysEvent definitions | `hisysevent.yaml` |
| Get samgr proxy | `iservice_registry.h` → `SystemAbilityManagerClient` |
| dbinder / cross-device SA | `rpc_callback_imp.cpp` |

**Key terms**: `said` (SA ID, range 0x1–0x00ffffff), ondemand (lazy load/unload), LSA (LocalAbilityManager in each SA process), BootPhase (startup priority: BootStart > CoreStart > OtherStart), dbinder (cross-device RPC).

## Expert Constraints

1. **SA ID three-place sync**: Adding an SA ID requires updating `system_ability_definition.h`, `hidumper/dump_utils.cpp`, and `rust/cxx/definition.rs` simultaneously.
2. **Thread safety**: `abilityMap_` guarded by `shared_mutex` — read with `shared_lock`, write with `unique_lock`. Never bypass.
3. **FFRT mandated**: Compile flag `SAMGR_USE_FFRT` is set. Use `FfrtHandler` for concurrency, not `std::thread`.
4. **CFI enabled**: All targets have CFI+CFI-cross-DSO. Functions needing exemption go in `cfi_blocklist.txt`.
5. **Listener cap**: Max 256 SubscribeSystemAbility listeners per process.
6. **IPC token**: All IPC requests must carry `u"ohos.samgr.accessToken"`.
7. **MAX_SERVICES**: `abilityMap_` has a hard size limit — rejected if exceeded.
8. **safwk ↔ samgr bidirectional dependency** (by design): samgr → safwk `system_ability_ondemand_reason`; safwk → samgr `samgr_proxy` + `samgr_common`. Decoupled via static/shared library split.
9. **samgr must not include business SA headers** — only interacts via `IRemoteObject` + said.
10. **AccessToken check required** in all `OnRemoteRequest` paths that mutate state.

**Anti-patterns**: Hardcoding SA IDs instead of enum constants · Skipping null check on `GetSystemAbility` return · Adding a collector without registering in `DeviceStatusCollectManager` · Directly manipulating `abilityMap_` without lock.

## Build & Test

```bash
# Build samgr component
./build.sh --product-name {product} --build-target samgr

# Build specific target
./build.sh --product-name {product} \
  --build-target foundation/systemabilitymgr/samgr/services/samgr/native:samgr

# Unit tests (targets in bundle.json → build.test)
./build.sh --product-name {product} \
  --build-target foundation/systemabilitymgr/samgr/services/samgr/native/test:unittest

# Key test targets: SystemAbilityMgrTest, SystemAbilityMgrOndemandTest,
#   SystemAbilityStateMachineTest, DeviceStatusCollectManagerTest

# Fuzz tests
./build.sh --product-name {product} \
  --build-target foundation/systemabilitymgr/samgr/test/fuzztest:fuzztest
# Targets: samgr_fuzzer, systemabilitymanager_fuzzer, samgrdumper_fuzzer
```

**Feature switches** (`config.gni`): `samgr_enable_delay_dbinder`(T), `samgr_enable_extend_load_timeout`(F), `samgr_support_multi_instance`(F).

**Device debug**: `hidumper -s 0 -a "-l"` (list all SAs) · `hilog | grep -i samgr` · `param get | grep samgr`.
