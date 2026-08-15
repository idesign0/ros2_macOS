# ROS 2 macOS CI — autofix status

_updated **2026-08-15 09:32 UTC** · refreshed every autofix cycle_

## 🔧 What the autofixer is doing

> **🔧 fixing 20 package(s): as2_gazebo_assets as2_platform_crazyflie boost_plugin_loader data_tamer_cpp husarion_asset_server kortex_api libmavconn moveit_hybrid_planning om_gravity_compensation_controller om_spring_actuator_controller rmf_battery rmf_robot_sim_gz_classic_plugins rmf_robot_sim_gz_plugins rmf_traffic_editor_test_maps rmf_traffic_examples rmf_visualization_navgraphs rmf_visualization_rviz2_plugins robotiq_driver ros2_medkit_serialization webots_ros2_driver  — 2026-08-15 09:32 UTC**

> ✅ Self-check: shared patch files byte-identical across all 3 distros; nothing pushed to release branches.

## Distro runners

| distro | status | shards done | conclusion |
|---|---|--:|---|
| humble | queued | 6/20 |  |
| jazzy | queued | 12/21 |  |
| kilted | completed | 21/21 | success |

## Auto-fixed: **66** packages tracked

> Fixes are committed **locally on the Mac** with an `[auto]` tag and are **never pushed** —
> review and push them at the Mac. This status page is the only thing pushed (to this branch).

### Recent `[auto]` commits (humble tree)
```
212f86b0 ci(humble): lely_core_libraries — fix macOS clockid_t typedef conflict [auto] [skip ci]
9fd083d7 ci(humble): ardrone_sdk — skip-list (avahi-client is Linux/systemd-only, no macOS bottle) [auto] [skip ci]
6e4a56ce ci(humble): audio_common_msgs, ecal, naoqi_libqi, rmf_traffic, rmf_traffic_editor, rmw_stats_shim, sick_safetyscanners_base — 7-package autofix cycle [auto]
421933f4 ci(humble): canboat_vendor + fmilibrary_vendor — Linux-only tool drop + CMP0026 LOCATION fix [auto]
24d26927 ci(humble): menge_vendor — install osrf/simulation/tinyxml1 (brew removed bare tinyxml) [auto]
```

### Latest cycle detail
```
  Last cycle: 2026-08-15 08:31 UTC
  lely_core_libraries - fixed macOS clockid_t typedef conflict (Apple SDK enum clockid_t vs lely's own shim) — new git-apply patch wired into the package's existing UPDATE_COMMAND chain in apply_source_patches.sh, compile-tested against both pinned lely-core commits (committed, not pushed, all 3 distros)
  mujoco_3d_lidar - cascade, not fixed — find_package(mujoco_vendor) fails because mujoco_vendor is already-catalogued unsupported on Darwin (upstream hard-rejects macOS)
  gz_ros2_control_demos / leo_gz_plugins / turtlebot4_gz_gui_plugins - needs human — all 3 hit gz/msgs10 protobuf gencode/runtime version mismatch; root cause looks like a stale cached gz_msgs_vendor codegen artifact, not a bug in these packages (confirmed same brew protobuf version on both jobs via gh api) — cache invalidation is outside the 3 workspaces
  ign_rviz_common - needs human — depends on old-naming ignition-gui6; project only vendors the renamed gz-* stack, no legacy vendor exists anywhere in the tree — needs a real fork/port, not a patch
  rosgraph_monitor - needs human — generate_parameter_library_cpp_BIN empty; confirmed via gh api that the "ros-planning" domain shard (which builds the console-script it needs) was still queued while the shard that needed it ran and failed — a cross-domain build-order race in the layered-matrix scheduling, not a source bug
  66 packages auto-fixed so far
```
