# ROS 2 macOS CI — autofix status

_updated **2026-08-18 21:49 UTC** · refreshed every autofix cycle_

## 🔧 What the autofixer is doing

> **💤 idle — all completed shards drained, waiting for more to finish — 2026-08-18 21:49 UTC**

> ✅ Self-check: shared patch files byte-identical across all 3 distros; nothing pushed to release branches.

## Distro runners

| distro | status | shards done | conclusion |
|---|---|--:|---|
| humble | in_progress | 0/0 |  |
| jazzy | in_progress | 0/0 |  |
| kilted | queued | 0/0 |  |

## Auto-fixed: **88** packages tracked

> Fixes are committed **locally on the Mac** with an `[auto]` tag and are **never pushed** —
> review and push them at the Mac. This status page is the only thing pushed (to this branch).

### Recent `[auto]` commits (humble tree)
```
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
  Last cycle: 2026-08-18 20:52 UTC
  autoware_lanelet2_extension (jazzy+kilted; humble already had it) - fixed — rosidl_runtime_cpp/traits.hpp's u16string-to-yaml helper uses a C++17-deprecated wstring_convert&lt;codecvt_utf8_utf16&gt;, which errors under this package's own CMAKE_COMPILE_WARNING_AS_ERROR setting; added the missing -Wno-error=deprecated-declarations toolchain flag (jazzy/kilted parity with humble), compile-tested with a real clang++ -Werror repro (committed, not pushed)
  multisensor_calibration (jazzy+kilted) - fixed — macOS-only bug: bare find_package(tinyxml2 REQUIRED) tries Module mode first and, on macOS's case-insensitive filesystem, accidentally matches the tinyxml2 vendor's installed FindTinyXML2.cmake instead of falling through to the real Config file, so the wrong _FOUND variable gets set; forced Config mode explicitly, compile-tested via a real cmake configure reproducing both the bug and the fix (committed, not pushed, all 3 distros for shared-file parity)
  93 packages auto-fixed so far
```
