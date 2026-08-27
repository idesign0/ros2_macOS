# ROS 2 macOS CI — autofix status

_updated **2026-08-27 11:47 UTC** · refreshed every autofix cycle_

## 🔧 What the autofixer is doing

> **💤 idle — all completed shards drained, waiting for more to finish — 2026-08-27 11:47 UTC**

> ⚠️ **Self-check: parity drift detected** in `ci/skip-list.txt` — should be byte-identical across the 3 distros. Needs a look.

## Distro runners

| distro | status | shards done | conclusion |
|---|---|--:|---|
| humble | in_progress | 0/0 |  |
| jazzy | in_progress | 0/0 |  |
| kilted | in_progress | 0/0 |  |

## Auto-fixed: **92** packages tracked

> Fixes are committed **locally on the Mac** with an `[auto]` tag and are **never pushed** —
> review and push them at the Mac. This status page is the only thing pushed (to this branch).

### Recent `[auto]` commits (humble tree)
```
67f8012f ci(humble): fmilibrary_vendor — fix CMP0026 fallout (tests/doxygen/expatex/implicit-decl) + .so->dylib [auto] [skip ci]
47aab403 ci(humble): rmf_traffic_ros2 - bare yaml-cpp target_link_libraries link fix [auto] [skip ci]
f7a6b8fb ci(DISTRO): skip-list += husarion_asset_server, kortex_api [auto] [skip ci]
85c4cd2d ci(humble): boost_plugin_loader, as2_platform_crazyflie, libmavconn, ros2_medkit_serialization, om_gravity/spring_actuator_controller — 5 new-root fixes [auto] [skip ci]
212f86b0 ci(humble): lely_core_libraries — fix macOS clockid_t typedef conflict [auto] [skip ci]
9fd083d7 ci(humble): ardrone_sdk — skip-list (avahi-client is Linux/systemd-only, no macOS bottle) [auto] [skip ci]
6e4a56ce ci(humble): audio_common_msgs, ecal, naoqi_libqi, rmf_traffic, rmf_traffic_editor, rmw_stats_shim, sick_safetyscanners_base — 7-package autofix cycle [auto]
421933f4 ci(humble): canboat_vendor + fmilibrary_vendor — Linux-only tool drop + CMP0026 LOCATION fix [auto]
```

### Latest cycle detail
```
  Last cycle: 2026-08-19 19:42 UTC
  fmilibrary_vendor (all 3 distros) - fixed — CMP0026 fallout in the same fmi-library ExternalProject (runtime_test.cmake + UseDoxygen.cmake, disabled via CMAKE_ARGS), a broken add_dependencies(expatex ...) call (perl-deleted), an Apple-Clang implicit-function-declaration hard error in vendored minizip (downgraded to warning), and libfmilib_shared.so → ${CMAKE_SHARED_LIBRARY_SUFFIX} (macOS builds .dylib); compile-tested end-to-end against a real offline v2.2.3 clone (cmake --build + make install clean) (committed, not pushed)
  live555_vendor (jazzy only) - fixed — GNU-ld-only --allow-shlib-undefined swapped for Apple ld64's -undefined dynamic_lookup on 3 libraries (a genuine circular HashTable symbol dependency, not decorative), plus a real OpenSSL include-scope bug (PRIVATE→PUBLIC on liveMedia); compile-tested end-to-end with default build options, all libs + demo executables link clean, make install verified (committed, not pushed)
  nebula_velodyne_common (humble+jazzy) - fixed — same bare-yaml-cpp linker-name bug as rmf_traffic_editor/rmf_traffic_ros2, rewrote to ${YAML_CPP_LIBRARIES}; verified structurally + idempotency-checked (committed, not pushed)
  92 packages auto-fixed so far
```
