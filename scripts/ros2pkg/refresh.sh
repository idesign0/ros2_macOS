#!/usr/bin/env bash
# ------------------------------------------------------------------
# refresh.sh — the scheduled entry point.
#
#   1. rsdistro.py   re-pull rosdistro -> ~/Downloads/<distro>_{packages,repos}.json
#   2. ros2pkg index rebuild the submodule lookup table from every .gitmodules
#   3. ros2pkg classify/new  report, per distro, what is NOT yet added as a submodule
#
# "Added" means present in .gitmodules. A repo stays in the pending report on
# every run until it is really submoduled — this script never adds anything.
# The snapshot is updated at the END so the next run can flag what newly appeared.
#
# Point cron/launchd at this file. Output is appended to refresh.log.
# ------------------------------------------------------------------
set -euo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
LOG="$HERE/refresh.log"
DISTROS=(humble jazzy kilted)
PY="${PYTHON:-python3}"

log() { echo "[$(date '+%Y-%m-%d %H:%M:%S')] $*" | tee -a "$LOG"; }

log "=== refresh start ==="

# 1) regenerate the rosdistro manifests (network)
if "$PY" "$HERE/rsdistro.py" >>"$LOG" 2>&1; then
    log "rsdistro.py ok"
else
    log "rsdistro.py FAILED — keeping previous manifests"
fi

# 2) rebuild the lookup table from current .gitmodules
"$PY" "$HERE/ros2pkg.py" index >>"$LOG" 2>&1
log "index rebuilt"

# 3) per-distro pending report (does NOT modify your tree)
for d in "${DISTROS[@]}"; do
    "$PY" "$HERE/ros2pkg.py" classify "$d" --write >>"$LOG" 2>&1 || true
    "$PY" "$HERE/ros2pkg.py" backlog "$d"  >>"$LOG" 2>&1 || true
    log "--- $d ---"
    "$PY" "$HERE/ros2pkg.py" new "$d" --write --update-snapshot | tee -a "$LOG"
done

log "=== refresh done ==="
echo "Full log: $LOG"
