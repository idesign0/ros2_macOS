# ROS 2 macOS CI — autofix status

_updated **2026-08-31 19:26 UTC** · refreshed every autofix cycle_

## 🔧 What the autofixer is doing

> **💤 idle — all completed shards drained, waiting for more to finish — 2026-08-31 19:26 UTC**

> ⚠️ **Self-check: parity drift detected** in `ci/skip-list.txt` — should be byte-identical across the 3 distros. Needs a look.

## Distro runners

| distro | status | shards done | conclusion |
|---|---|--:|---|
| humble | completed | 20/20 | success |
| jazzy | completed | 21/21 | failure |
| kilted | completed | 21/21 | success |

## Auto-fixed: **96** packages tracked

> Fixes are committed **locally on the Mac** with an `[auto]` tag and are **never pushed** —
> review and push them at the Mac. This status page is the only thing pushed (to this branch).

### Recent `[auto]` commits (humble tree)
```
f9ff8952 ci(humble): sync_tooling_msgs — protobuf Module->CONFIG mode + Abseil link [auto] [skip ci]
93606a87 ci(humble): autoware_test_utils — bare yaml-cpp linker name (x2) [auto] [skip ci]
d9ce15e6 ci(humble): autoware_map_projection_loader — bare yaml-cpp linker name [auto] [skip ci]
21bc2852 ci(humble): autoware_downsample_filters — add tl_expected/expected.hpp include shim [auto] [skip ci]
67f8012f ci(humble): fmilibrary_vendor — fix CMP0026 fallout (tests/doxygen/expatex/implicit-decl) + .so->dylib [auto] [skip ci]
47aab403 ci(humble): rmf_traffic_ros2 - bare yaml-cpp target_link_libraries link fix [auto] [skip ci]
f7a6b8fb ci(DISTRO): skip-list += husarion_asset_server, kortex_api [auto] [skip ci]
85c4cd2d ci(humble): boost_plugin_loader, as2_platform_crazyflie, libmavconn, ros2_medkit_serialization, om_gravity/spring_actuator_controller — 5 new-root fixes [auto] [skip ci]
```

### Latest cycle detail
```
  Last cycle: 2026-08-27 22:15 UTC
  autoware_downsample_filters (humble+jazzy, absent kilted) - fixed — #include &lt;tl_expected/expected.hpp&gt; not found on jazzy/kilted (brew tl-expected installs at tl/expected.hpp); same latent bug affects 6+ other packages project-wide, fixed once via a redirect-shim include dir on the shared Findtl_expected.cmake bridge target; compile-tested (real configure+build+run, tl::expected&lt;int,int&gt; round-trip) (committed, not pushed)
  autoware_map_projection_loader (humble+jazzy, absent kilted) - fixed — 4th instance of the bare-yaml-cpp linker-name bug, rewrote to ${YAML_CPP_LIBRARIES}; verified structurally + idempotency-checked (committed, not pushed)
  autoware_test_utils (humble+jazzy, absent kilted) - fixed — same bare-yaml-cpp bug, 2 occurrences in one file; verified structurally + idempotency-checked (committed, not pushed)
  sync_tooling_msgs (humble+jazzy, absent kilted) - fixed — google/protobuf/runtime_version.h not found; the obvious fix compiled but then failed at link with ~35 undefined Abseil symbols (caught by locally building all 27 real .proto files, not just trusting the CI error) — real fix switches to find_package(Protobuf REQUIRED CONFIG) + links protobuf::libprotobuf directly; compile and link-tested end-to-end against real brew protobuf/Abseil, verified via otool -L (committed, not pushed)
  96 packages auto-fixed so far
```

**All three runners are DONE** — next: review the `[auto]` commits, push, and dispatch a fresh round.
