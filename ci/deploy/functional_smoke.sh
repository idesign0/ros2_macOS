#!/usr/bin/env bash
# Headless functional checks on an INSTALLED ROS 2 tree -- the packaged tree in the
# package-smoke workflows, and the keg `brew install` produced in brew-install-test.yml,
# so both run exactly the same checks.
#
#   usage: functional_smoke.sh <prefix>     (the dir holding setup.bash)
#
# Exit status = number of failed checks. Logs go to /tmp/fsmoke and report/functional/.
set -uo pipefail
PREFIX="${1:?usage: functional_smoke.sh <prefix>}"
PREFIX="${PREFIX%/}"
# gz-transport picks a network interface on its own. A CI runner has several, discovery
# never comes back over the chosen one, and the server then looks dead (no /stats, no
# poses). Loopback is all a single-host check needs; a caller's own setting wins.
export GZ_IP="${GZ_IP:-127.0.0.1}"
export GZ_PARTITION="${GZ_PARTITION:-fsmoke-$$}"
# Everything here runs HEADLESS. The GUI / Ogre2 / Metal render that joaaosc verified by
# hand (homebrew-ros2#3) is deliberately NOT attempted: GitHub's macOS runners have no
# display and no GPU, so a Metal render cannot be exercised in CI at all.
#
# What this adds over the dlopen sample above, which the closure check in package_tree.py
# already covers statically:
#   - gz loads system plugins through its OWN loader, not ctypes.CDLL, so a plugin that
#     dlopen's a dependency is invisible to both otool and the sample.
#   - a SILENT ABI break -- same soname, changed struct layout -- loads fine and then
#     computes the wrong answer with exit code 0. Only running physics catches it.
#   - ros_gz_bridge is where two independently-built ABI worlds actually meet.

# macOS ships no GNU `timeout`; brew coreutils installs it as gtimeout. And `gz topic -e`
# blocks forever when no message ever arrives, so every call here must be bounded.
TO=""
for c in timeout gtimeout; do command -v "$c" >/dev/null 2>&1 && { TO="$c"; break; }; done
bounded() {
  local s=$1; shift
  if [ -n "$TO" ]; then "$TO" -k 5 "$s" "$@"; return $?; fi
  "$@" & local p=$!
  ( sleep "$s"; kill -9 "$p" 2>/dev/null ) & local w=$!
  wait "$p" 2>/dev/null; local rc=$?
  kill -9 "$w" 2>/dev/null
  return $rc
}
L=/tmp/fsmoke; mkdir -p "$L"; fails=0
note() { echo "  $1"; }
bad()  { echo "::warning::functional check FAILED: $1"; fails=$((fails+1)); }

# Test the PACKAGED tree, from where it lies -- that is what ships.
# Source TOLERANTLY and SHOW what breaks. This line used to be
#   . install/setup.bash 2>/dev/null || { ...; exit 1; }
# and on jazzy smoke 42409015421 it ended the step in 6 seconds with exit 127 and
# no output at all -- a broken package env hook (husarion_ugv_gazebo's
# setup_envs.sh calls ament_prepend_unique_value, which colcon's sourcing context
# does not define) aborted the source under -e, and 2>/dev/null hid the reason.
# Advisory mode then reported the whole step as a pass having checked nothing.
# An interactive user shell survives such a hook, so the checks below should too.
set +eu
. "$PREFIX/setup.bash" 2> "$L/source_err.txt"
src_rc=$?
set -eu
if [ -s "$L/source_err.txt" ]; then
  echo "::warning::sourcing $PREFIX/setup.bash printed errors (rc=$src_rc) -- broken package env hooks:"
  sed 's/^/    /' "$L/source_err.txt" | head -25
fi
command -v gz >/dev/null 2>&1 || { echo "::error::no gz binary on PATH after sourcing"; exit 1; }
note "gz: $(command -v gz)"

WORLD="$(find "$PREFIX/share" -name shapes.sdf -path '*worlds*' 2>/dev/null | head -1)"
[ -n "$WORLD" ] || { echo "::error::no worlds/shapes.sdf in the tree"; exit 1; }
note "world: $WORLD"

# ---------------------------------------------------------------- server
# Unbounded in SIM time, bounded in wall time, so the topic checks below have something
# live to query. Killed explicitly at the end.
( bounded 240 gz sim -s -r -v 2 "$WORLD" > "$L/server.log" 2>&1 ) &
SRV=$!
up=0
for _ in $(seq 1 30); do
  sleep 2
  if bounded 10 gz topic -l 2>/dev/null | grep -q '/stats'; then up=1; break; fi
  kill -0 "$SRV" 2>/dev/null || break
done
if [ "$up" != 1 ]; then
  echo "::warning::gz server never advertised /stats; see server.log"
  sed -n '1,60p' "$L/server.log" | sed 's/^/    /'
  fails=$((fails+1))
fi
bounded 15 gz topic -l > "$L/topics.txt" 2>/dev/null || true
note "topics advertised: $(wc -l < "$L/topics.txt" | tr -d ' ')"

# ---------------------------------------------------------------- 1. physics steps
# If the DART physics plugin fails to load -- joaaosc's incomplete-DART-closure case --
# the server either aborts or never steps, so iterations stay put.
if [ "$up" = 1 ]; then
  bounded 45 gz topic -e -t /stats -n 2 > "$L/stats.txt" 2>&1 || true
  if python3 -I - "$L/stats.txt" <<'PY'
import re, sys
vals = [int(m) for m in re.findall(r"^\s*iterations:\s*(\d+)", open(sys.argv[1]).read(), re.M)]
print(f"    iterations seen: {vals[:4]}")
sys.exit(0 if len(vals) >= 2 and vals[1] > vals[0] else 1)
PY
  then note "sim loop is running (iterations increase) -- NOT proof of physics, see the drop test"
  else bad "iterations did not increase -- the sim loop is not running"
  fi

  # Bodies exist and are being updated.
  POSE_T="$(grep -m1 'dynamic_pose/info' "$L/topics.txt" || true)"
  if [ -n "$POSE_T" ]; then
    bounded 45 gz topic -e -t "$POSE_T" -n 1 > "$L/pose.txt" 2>&1 || true
    if [ -s "$L/pose.txt" ] && grep -q 'position\|pose' "$L/pose.txt"; then
      note "poses exposed on $POSE_T"
    else bad "no pose message on $POSE_T"; fi
  else bad "no dynamic_pose/info topic advertised"; fi

  # 2. clock, the check joaaosc ran: two messages with advancing sim time.
  bounded 45 gz topic -e -t /clock -n 2 > "$L/clock.txt" 2>&1 || true
  if python3 -I - "$L/clock.txt" <<'PY'
import re, sys

def field(block, key):
    m = re.search(r"\b" + key + r":\s*(\d+)", block)
    return int(m.group(1)) if m else 0

# \bsim\s*\{ matches the Clock message's `sim {` and deliberately NOT `sim_time {`.
blocks = re.findall(r"\bsim\s*\{([^}]*)\}", open(sys.argv[1]).read())
times = [field(b, "sec") * 10**9 + field(b, "nsec") for b in blocks]
print(f"    sim times (ns): {times[:4]}")
sys.exit(0 if len(times) >= 2 and times[1] > times[0] else 1)
PY
  then note "gz clock advances"
  else bad "gz clock did not advance over two messages"; fi
fi

# ---------------------------------------------------------------- 1b. a dropped ball falls
# The iteration count above advances with NO physics loaded at all. Measured on a
# relocated jazzy tree: gz sim ran with physics, user-commands and scene-broadcaster
# ALL failing to load, and iterations still went 1613 -> 1614. So drop a ball from
# 5 m under gz's default engine (DART -- what a user gets) and require it to have
# fallen. Nothing short of a working physics engine can pass this.
cat > "$L/drop.sdf" <<'SDF'
<?xml version="1.0"?>
<sdf version="1.9"><world name="drop">
  <physics name="1ms" type="ignored"><max_step_size>0.001</max_step_size><real_time_factor>1.0</real_time_factor></physics>
  <plugin filename="gz-sim-physics-system" name="gz::sim::systems::Physics"/>
  <plugin filename="gz-sim-user-commands-system" name="gz::sim::systems::UserCommands"/>
  <plugin filename="gz-sim-scene-broadcaster-system" name="gz::sim::systems::SceneBroadcaster"/>
  <gravity>0 0 -9.8</gravity>
  <model name="ground"><static>true</static><link name="l"><collision name="c"><geometry><plane><normal>0 0 1</normal></plane></geometry></collision></link></model>
  <model name="ball"><pose>0 0 5 0 0 0</pose><link name="l">
    <inertial><mass>1</mass><inertia><ixx>0.1</ixx><iyy>0.1</iyy><izz>0.1</izz></inertia></inertial>
    <collision name="c"><geometry><sphere><radius>0.2</radius></sphere></geometry></collision></link></model>
</world></sdf>
SDF
( bounded 90 gz sim -s -r -v 3 "$L/drop.sdf" > "$L/drop_server.log" 2>&1 ) &
DROP=$!
dup=0
for _ in $(seq 1 30); do
  sleep 2
  if bounded 10 gz topic -l 2>/dev/null | grep -q '/world/drop/dynamic_pose/info'; then dup=1; break; fi
  kill -0 "$DROP" 2>/dev/null || break
done
if [ "$dup" = 1 ]; then
  sleep 3
  bounded 30 gz topic -e -t /world/drop/dynamic_pose/info -n 1 > "$L/drop_pose.txt" 2>&1 || true
  if python3 -I - "$L/drop_pose.txt" <<'PY'
import re, sys
t = open(sys.argv[1]).read()
m = re.search(r'name:\s*"ball".*?position\s*\{([^}]*)\}', t, re.S)
z = None
if m:
    zm = re.search(r"z:\s*([-0-9.e]+)", m.group(1))
    z = float(zm.group(1)) if zm else 0.0
print(f"    ball z ~3 s after a 5 m drop: {z}")
sys.exit(0 if z is not None and z < 4.5 else 1)
PY
  then note "physics WORKS: a dropped ball fell under the default engine"
  else
    bad "a ball dropped from 5 m did not fall -- no working physics engine"
    grep -nE 'Error|Failed|not loaded' "$L/drop_server.log" | head -10 | sed 's/^/    /'
  fi
else
  bad "the drop-test world never published poses"
  sed -n '1,30p' "$L/drop_server.log" | sed 's/^/    /'
fi
kill "$DROP" 2>/dev/null || true

# ---------------------------------------------------------------- 3. ros_gz_bridge
if ros2 pkg prefix ros_gz_bridge >/dev/null 2>&1 && [ "$up" = 1 ]; then
  ros2 run ros_gz_bridge parameter_bridge \
    '/clock@rosgraph_msgs/msg/Clock[gz.msgs.Clock' > "$L/bridge.log" 2>&1 &
  BR=$!
  sleep 8
  bounded 45 ros2 topic echo /clock rosgraph_msgs/msg/Clock --once > "$L/ros_clock.txt" 2>&1 || true
  kill "$BR" 2>/dev/null || true
  if grep -qE 'sec:' "$L/ros_clock.txt" 2>/dev/null; then
    note "ros_gz_bridge delivered a ROS clock message"
  else
    bad "ros_gz_bridge produced no ROS clock message"
    sed -n '1,30p' "$L/bridge.log" 2>/dev/null | sed 's/^/    /'
  fi
else
  note "ros_gz_bridge not in this tree (or server down) -- bridge check skipped"
fi

# ---------------------------------------------------------------- plugin loader errors
kill "$SRV" 2>/dev/null || true
pkill -f 'gz sim' 2>/dev/null || true
if [ -s "$L/server.log" ]; then
  if cat "$L/server.log" "$L/drop_server.log" 2>/dev/null \
       | grep -nE 'Unable to load|Failed to load (system )?plugin|dlopen|image not found|Library not loaded|Symbol not found' \
       > "$L/loaderr.txt"; then
    bad "gz reported plugin/library load errors"
    sed -n '1,25p' "$L/loaderr.txt" | sed 's/^/    /'
  else
    note "no plugin/library load errors in the gz server log"
  fi
fi

# ---------------------------------------------------------------- 4. Qt GUI runtime
# Qt loads its platform plugin (cocoa/offscreen), image formats and QML modules by dlopen
# from a directory derived at run time, so neither otool nor the closure sees them. Before
# package_tree.py staged them, a keg without brew Qt could not start rviz2 at all, and one
# WITH brew Qt loaded brew's plugins -- and through them a second QtCore. Offscreen, so
# no display is needed; rviz2 may still die later creating its GL context, which is not
# what this checks: only WHERE the Qt runtime came from.
RV="$PREFIX/lib/rviz2/rviz2"
if [ -x "$RV" ]; then
  # launched directly, not through bounded(): DYLD_PRINT_LIBRARIES would otherwise also
  # list the timeout wrapper's own images (brew coreutils) as Homebrew loads.
  QT_QPA_PLATFORM=offscreen DYLD_PRINT_LIBRARIES=1 "$RV" > "$L/rviz.log" 2>&1 &
  rvp=$!
  for _ in $(seq 30); do kill -0 "$rvp" 2>/dev/null || break; sleep 1; done
  kill -9 "$rvp" 2>/dev/null || true
  wait "$rvp" 2>/dev/null || true
  plat=$(grep -oE '/[^ ]*/platforms/libqoffscreen\.dylib' "$L/rviz.log" | head -1)
  grep -E '^dyld\[[0-9]+\]: <[^>]*> /opt/homebrew/(opt|Cellar)/' "$L/rviz.log" \
    | grep -vE '/opt/homebrew/(opt|Cellar)/ros2-' > "$L/rviz_brew.txt" || true
  ncore=$(grep -cE '/QtCore$' "$L/rviz.log" || true)
  if grep -qiE 'could not find the Qt platform plugin|no Qt platform plugin could be initialized' "$L/rviz.log"; then
    bad "rviz2: no Qt platform plugin found"
  elif [ -z "$plat" ]; then
    bad "rviz2 never loaded a Qt platform plugin"
    tail -15 "$L/rviz.log" | sed 's/^/    /'
  elif [ -s "$L/rviz_brew.txt" ]; then
    bad "rviz2 loaded $(wc -l < "$L/rviz_brew.txt" | tr -d ' ') image(s) from Homebrew"
    sed -E 's/^dyld\[[0-9]+\]: <[^>]*> /    /' "$L/rviz_brew.txt" | head -10
  elif [ "$ncore" -gt 1 ]; then
    bad "rviz2 loaded QtCore $ncore times"
  else
    note "rviz2: Qt platform plugin from ${plat##*/vendor/}, one QtCore, 0 Homebrew images"
  fi
else
  note "rviz2 not in this tree -- Qt check skipped"
fi

# ---------------------------------------------------------------- 5. tree completeness
# Every indexed package must ship its package.xml, and every in-prefix library a mach-o
# names must exist. Both went missing for domain-shard packages (CMake keeps the SOURCE
# mtime on installed files, and the shard delta selected by mtime and skipped symlinks),
# and nothing above exercised them: 811 packages without package.xml, mola without
# libgtsam.4.dylib.
IDX="$PREFIX/share/ament_index/resource_index/packages"
if [ -d "$IDX" ]; then
  nopx=0; tot=0
  for m in "$IDX"/*; do
    tot=$((tot+1)); [ -f "$PREFIX/share/$(basename "$m")/package.xml" ] || { nopx=$((nopx+1)); [ $nopx -le 5 ] && echo "    no package.xml: $(basename "$m")"; }
  done
  if [ "$nopx" -eq 0 ]; then note "all $tot indexed packages ship package.xml"
  else bad "$nopx of $tot indexed packages have no package.xml"; fi
fi
REAL="$(cd "$PREFIX" && pwd -P)"
dangling=$(find "$REAL/lib" "$REAL/bin" -type f \( -name '*.dylib' -o -name '*.so' -o -perm -u+x \) 2>/dev/null \
  | while read -r f; do otool -L "$f" 2>/dev/null | tail -n +3; done \
  | awk '{print $1}' | grep -E "^($PREFIX|$REAL)/" | sort -u | while read -r r; do [ -e "$r" ] || echo "$r"; done)
if [ -z "$dangling" ]; then note "every in-prefix library reference resolves"
else bad "$(echo "$dangling" | wc -l | tr -d ' ') in-prefix library references point at nothing"; echo "$dangling" | head -5 | sed 's/^/    /'; fi

echo "functional checks failed: $fails"
# The report step below does `mkdir -p report` too, so this is order-independent;
# step 9 uploads report/ with if: always(), so these survive a failed check.
mkdir -p report/functional
cp -p "$L"/*.txt "$L"/*.log report/functional/ 2>/dev/null || true
exit "$fails"
