# ROS 2 macOS CI — autofix status

_updated **2026-08-15 07:05 UTC** · refreshed every autofix cycle_

## 🔧 What the autofixer is doing

> **🔧 fixing 2 package(s): ardrone_sdk compass_interfaces  — 2026-08-15 07:05 UTC**

> ✅ Self-check: shared patch files byte-identical across all 3 distros; nothing pushed to release branches.

## Distro runners

| distro | status | shards done | conclusion |
|---|---|--:|---|
| humble | queued | 0/20 |  |
| jazzy | queued | 7/21 |  |
| kilted | in_progress | 20/21 |  |

## Auto-fixed: **57** packages tracked

> Fixes are committed **locally on the Mac** with an `[auto]` tag and are **never pushed** —
> review and push them at the Mac. This status page is the only thing pushed (to this branch).

### Recent `[auto]` commits (humble tree)
```
6e4a56ce ci(humble): audio_common_msgs, ecal, naoqi_libqi, rmf_traffic, rmf_traffic_editor, rmw_stats_shim, sick_safetyscanners_base — 7-package autofix cycle [auto]
421933f4 ci(humble): canboat_vendor + fmilibrary_vendor — Linux-only tool drop + CMP0026 LOCATION fix [auto]
24d26927 ci(humble): menge_vendor — install osrf/simulation/tinyxml1 (brew removed bare tinyxml) [auto]
```

### Latest cycle detail
```
  Last cycle: 2026-08-15 04:15 UTC
  audio_common_msgs - repointed .gitmodules from branch=master (ROS1 catkin) to branch=ros2 (ament_cmake) — wrong-branch case (committed, not pushed)
  ecal - find_package(CMakeFunctions) → direct include() of the same macros, bypassing a CMake dependency provider colcon never activates (committed, not pushed)
  naoqi_libqi - dropped a stray boost/asio/io_service.hpp include already superseded by the package's own compat shim (committed, not pushed)
  rmf_traffic - fixed 3 mistyped std::size_t overrides (should be ParticipantId) that only break where uint64_t != size_t (macOS); humble+jazzy only, kilted already fine upstream (committed, not pushed)
  rmf_traffic_editor - bare yaml-cpp link target → ${YAML_CPP_LIBRARIES}, matching a pattern already proven elsewhere in-tree (committed, not pushed)
  rmw_stats_shim - guarded a GNU-ld-only -Wl,--no-undefined flag with NOT APPLE (committed, not pushed)
  sick_safetyscanners_base - added a missing posix_time_types.hpp include (sibling headers in the same package already had it) (committed, not pushed)
  menge_vendor and zenoh_bridge_dds investigated, left for human review (Homebrew formula build break; pinned Rust crate needs a version bump I can't verify without local cargo)
  57 packages auto-fixed so far
```
