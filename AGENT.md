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

> If a subdirectory contains its own `AGENTS.md`, it takes precedence for tasks scoped to that directory.

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

**Where to look by task type**:

| Task | Start here |
|---|---|
| Add/register a new SA | `system_ability_definition.h` → `if_system_ability_manager.h` |
| Fix IPC dispatch bug | `samgr_ipc_interface_code.h` → `system_ability_manager_stub.cpp` |
| Add on-demand event collector | `collect/icollect_plugin.h` → `collect/device_status_collect_manager.cpp` |
| Change state machine logic | `schedule/system_ability_state_machine.cpp` → `schedule/system_ability_state_scheduler.cpp` |
| Add HiSysEvent | `hisysevent.yaml` → `services/dfx/include/hisysevent_adapter.h` |

**Before editing any file, state**: (1) the task category, (2) which documents you read, (3) which constraints apply.

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
11. **IPC code stability (ask before changing)**: Do not add, remove, or reorder existing entries in `SamgrInterfaceCode` enum. New codes are append-only at the end. Reordering silently breaks cross-process IPC system-wide.
12. **Generated files**: Rust bindings (`interfaces/innerkits/rust/src/cxx/`), hidumper saNameMap_ (`base/hiviewdfx/hidumper/`), and `hisysevent.yaml` generated configs are derived — edit the source, never patch generated output directly.
13. **DFX required**: New SA load/unload/error paths must call the HiSysEvent adapter (`services/dfx/include/hisysevent_adapter.h`). New blocking IPC or scheduler operations must wrap with `SamgrXCollie` (`samgr_xcollie.h`, 60s default). Event names/params must match `hisysevent.yaml` schema.

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

**Device debug** (all via `hidumper -s 0 -a "<args>"`, SA ID 0 = samgr):
```bash
hidumper -s 0 -a "-l"                          # List all SA state info
hidumper -s 0 -a "-sa <said>"                  # Query specific SA state
hidumper -s 0 -a "-sm <ACTIVE|IDLE|NOT_LOADED>" # Query SAs by state
hidumper -s 0 -a "-p <processname>"            # Query process state info
hidumper -s 0 -a "--listener -l -sa"           # All SA subscribe-listener relationships
hidumper -s 0 -a "--listener -sa <said>"       # Who is listening to this SA
hidumper -s 0 -a "--ipc <procname|all> --start-stat"  # Start IPC profiling (then --stat / --stop-stat)
hidumper -s 0 -a "--ffrt <pid1|pid2> --start-stat"    # Start FFRT profiling (then --stat / --stop-stat)
hilog -T SAMGR                                 # hilog tag: SAMGR, domain: 0xD001800
param get bootevent.samgr.ready                # "true" = samgr fully initialized
```

**Static analysis**: CFI (`cfi=true`, `cfi_cross_dso=true`) and PAC (`branch_protector_ret="pac_ret"`) are enforced in every BUILD.gn `sanitize` block. If the build fails with CFI linker errors, add only to `cfi_blocklist.txt` with justification. For ASAN debugging build: `./build.sh --product-name {product} --gn-args is_asan=true --build-target samgr`.

**Done criteria**: Before reporting completion: (1) show build exit code 0, (2) show at least the affected `SystemAbilityMgr*` test suite passing, (3) if DFX paths were touched, verify HiSysEvent adapter calls match `hisysevent.yaml` schema, (4) if validation cannot run, state explicitly what was and was not verified and list remaining risks.
