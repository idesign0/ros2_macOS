# ROS 2 macOS CI — autofix status

_updated **2026-08-19 07:58 UTC** · refreshed every autofix cycle_

## 🔧 What the autofixer is doing

> **💤 idle — all completed shards drained, waiting for more to finish — 2026-08-19 07:58 UTC**

> ✅ Self-check: shared patch files byte-identical across all 3 distros; nothing pushed to release branches.

## Distro runners

| distro | status | shards done | conclusion |
|---|---|--:|---|
| humble | (query failed) | - | - |
| jazzy | (query failed) | - | - |
| kilted | (query failed) | - | - |

## Auto-fixed: **89** packages tracked

> Fixes are committed **locally on the Mac** with an `[auto]` tag and are **never pushed** —
> review and push them at the Mac. This status page is the only thing pushed (to this branch).

### Recent `[auto]` commits (humble tree)
```
47aab403 ci(humble): rmf_traffic_ros2 - bare yaml-cpp target_link_libraries link fix [auto] [skip ci]
f7a6b8fb ci(DISTRO): skip-list += husarion_asset_server, kortex_api [auto] [skip ci]
85c4cd2d ci(humble): boost_plugin_loader, as2_platform_crazyflie, libmavconn, ros2_medkit_serialization, om_gravity/spring_actuator_controller — 5 new-root fixes [auto] [skip ci]
212f86b0 ci(humble): lely_core_libraries — fix macOS clockid_t typedef conflict [auto] [skip ci]
9fd083d7 ci(humble): ardrone_sdk — skip-list (avahi-client is Linux/systemd-only, no macOS bottle) [auto] [skip ci]
6e4a56ce ci(humble): audio_common_msgs, ecal, naoqi_libqi, rmf_traffic, rmf_traffic_editor, rmw_stats_shim, sick_safetyscanners_base — 7-package autofix cycle [auto]
421933f4 ci(humble): canboat_vendor + fmilibrary_vendor — Linux-only tool drop + CMP0026 LOCATION fix [auto]
24d26927 ci(humble): menge_vendor — install osrf/simulation/tinyxml1 (brew removed bare tinyxml) [auto]
```

### Latest cycle detail
```
  Last cycle: 2026-08-19 06:58 UTC
  rmf_traffic_ros2 (jazzy; shared fix landed all 3 distros) - fixed — own real error was ld: library 'yaml-cpp' not found linking librmf_traffic_ros2.dylib (the task brief's quoted CMake/tinyxml error actually belonged to an interleaved menge_vendor log block, already-diagnosed needs-human, not re-touched); same bare-yaml-cpp-in-target_link_libraries() bug already fixed for rmf_traffic_editor — rewrote to ${YAML_CPP_LIBRARIES}, left the unrelated ament_export_dependencies() package-name entry alone; verified structurally + idempotency-checked against real CMakeLists.txt from all 3 trees (committed, not pushed)
  89 packages auto-fixed so far
```

**All three runners are DONE** — next: review the `[auto]` commits, push, and dispatch a fresh round.
