# ROS 2 macOS CI — autofix status

_updated **2026-08-16 08:26 UTC** · refreshed every autofix cycle_

## 🔧 What the autofixer is doing

> **⏳ paused (usage limit or auth) — will resume next hour — 2026-08-16 08:26 UTC**

> ✅ Self-check: shared patch files byte-identical across all 3 distros; nothing pushed to release branches.

## Distro runners

| distro | status | shards done | conclusion |
|---|---|--:|---|
| humble | queued | 11/20 |  |
| jazzy | completed | 21/21 | success |
| kilted | completed | 21/21 | success |

## Auto-fixed: **86** packages tracked

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
  Last cycle: 2026-08-15 09:35 UTC
  boost_plugin_loader (humble) - fixed, missing submodule — humble's tree never had ros_industrial_cmake_boilerplate at all (jazzy/kilted did, same pinned commit); added the submodule + a base-stage shard entry, compile-tested a full configure+build of both packages (committed, not pushed)
  as2_platform_crazyflie (humble) - fixed — FetchContent'd crazyflie_cpp statically links libusb; added the missing CoreFoundation/IOKit/Security framework links for its macOS backend (committed, not pushed)
  libmavconn (all 3) - fixed — own FindASIO.cmake never searched the homebrew prefix for asio.hpp; added the missing PATHS (committed, not pushed, all 3 distros)
  ros2_medkit_serialization (humble+jazzy) - fixed — yaml-cpp fallback lookup missed the vendored yaml-cpp's nested install prefix; added a fallback tier using the toolchain's own yaml-cpp cache vars, compile-tested via a real cmake configure (committed, not pushed)
  om_gravity_compensation_controller / om_spring_actuator_controller (jazzy) - fixed — find_package(tl_expected) (underscore) never matched homebrew's hyphenated tl-expected config; added a small CMake module bridging the name, compile-tested standalone and via the real toolchain file (committed, not pushed, all 3 distros)
  husarion_asset_server (jazzy) / kortex_api (humble+jazzy) - skip-listed — confirmed via GitHub release assets / vendor SDK source that neither has any macOS build path at all
  6 packages (as2_gazebo_assets, rmf_battery, rmf_traffic_examples, rmf_visualization_navgraphs, rmf_visualization_rviz2_plugins, rmf_robot_sim_gz_classic_plugins) - cascades of already-fixed-but-unpushed bugs or known cross-domain scheduling races, no new action needed
  6 packages (rmf_robot_sim_gz_plugins, rmf_traffic_editor_test_maps, moveit_hybrid_planning, data_tamer_cpp, robotiq_driver, webots_ros2_driver) - needs human — legacy-stack ports, a missing dependency with no safe upstream fork to guess, and one shard log that doesn't capture the real error
  86 packages auto-fixed so far
```
