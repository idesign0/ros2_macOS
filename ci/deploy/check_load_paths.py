#!/usr/bin/env python3
"""check_load_paths.py <keg>: every absolute load command in the installed keg must
exist on THIS machine. The smoke job's load test runs on the runner that built the
tree, where /Users/runner/work/.../build still exists, so a library linked by its
build-dir install name (libzenohc, kilted 26.11.15.20) loads there and nowhere else.
On the clean install-test runner such a path is missing, and this names it."""
import os, subprocess, sys
from collections import defaultdict

MAGIC = {b"\xcf\xfa\xed\xfe", b"\xce\xfa\xed\xfe", b"\xca\xfe\xba\xbe", b"\xbe\xba\xfe\xca"}
keg = os.path.realpath(sys.argv[1])
machos = []
for root, dirs, names in os.walk(keg):
    for n in names:
        f = os.path.join(root, n)
        if os.path.islink(f) or n.endswith((".o", ".a")):
            continue
        try:
            with open(f, "rb") as fh:
                if fh.read(4) in MAGIC:
                    machos.append(f)
        except OSError:
            pass
missing = defaultdict(set)
for i in range(0, len(machos), 200):
    out = subprocess.run(["otool", "-L"] + machos[i:i + 200], capture_output=True,
                         text=True).stdout
    cur = None
    for line in out.splitlines():
        if not line.startswith("\t"):
            cur = line.split(" (architecture")[0].rstrip(":")
            continue
        dep = line.strip().split(" (compatibility")[0]
        if dep.startswith("/") and not dep.startswith(("/usr/lib/", "/System/")) \
                and not os.path.exists(dep):
            missing[dep].add(os.path.relpath(cur, keg))
print(f"mach-o files checked: {len(machos)}")
if not missing:
    print("every absolute load path exists on this machine")
    sys.exit(0)
print(f"::error::{len(missing)} absolute load path(s) do not exist on this machine:")
for dep, users in sorted(missing.items()):
    print(f"  {dep}\n      used by {len(users)}: " + ", ".join(sorted(users)[:6]))
sys.exit(1)
