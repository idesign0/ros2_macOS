# ROS 2 macOS CI — autofix status

_updated **2026-08-15 07:10 UTC** · refreshed every autofix cycle_

## 🔧 What the autofixer is doing

> **⬆️ pushing 3 [auto] commit(s) to GitHub — 2026-08-15 07:10 UTC**

> ✅ Self-check: shared patch files byte-identical across all 3 distros; nothing pushed to release branches.

## Distro runners

| distro | status | shards done | conclusion |
|---|---|--:|---|
| humble | queued | 0/20 |  |
| jazzy | queued | 7/21 |  |
| kilted | queued | 21/21 |  |

## Auto-fixed: **59** packages tracked

> Fixes are committed **locally on the Mac** with an `[auto]` tag and are **never pushed** —
> review and push them at the Mac. This status page is the only thing pushed (to this branch).

### Recent `[auto]` commits (humble tree)
```
9fd083d7 ci(humble): ardrone_sdk — skip-list (avahi-client is Linux/systemd-only, no macOS bottle) [auto] [skip ci]
6e4a56ce ci(humble): audio_common_msgs, ecal, naoqi_libqi, rmf_traffic, rmf_traffic_editor, rmw_stats_shim, sick_safetyscanners_base — 7-package autofix cycle [auto]
421933f4 ci(humble): canboat_vendor + fmilibrary_vendor — Linux-only tool drop + CMP0026 LOCATION fix [auto]
24d26927 ci(humble): menge_vendor — install osrf/simulation/tinyxml1 (brew removed bare tinyxml) [auto]
```

### Latest cycle detail
```
  Last cycle: 2026-08-15 07:10 UTC
  ardrone_sdk - skip-listed: needs avahi-client (Linux D-Bus mDNS), which has no macOS Homebrew bottle at all (systemd-only formula); its discovery code would need a real Bonjour/dns_sd.h port, not a patch — humble+jazzy only, not in kilted's tree (committed, not pushed)
  compass_interfaces triaged, not fixed — confirmed cascade of cras_cpp_common (already catalogued), no own error to fix
  59 packages auto-fixed so far
```
