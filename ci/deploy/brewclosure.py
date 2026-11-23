#!/usr/bin/env python3
"""
Compute the full Homebrew dylib closure of a built ROS 2 tree.

Two outputs, both needed:
  * formulae  -> the depends_on list for the formula
  * dylibs    -> exactly what must go in the vendor dir, with its size

Why a CLOSURE and not one otool pass: a vendored dylib has its own
/opt/homebrew/opt/... references, and those must be vendored too or the vendor dir is
useless. One pass found 354 dylibs / 89 formulae; the closure is what actually ships.

Scans EVERY mach-o in the tree, not just lib/: the 699 gz engine-plugins and the
pluginlib plugins are dlopen'd at runtime but ship as files here, so otool does see
them. What it cannot see is a plugin that lives in a brew keg and is dlopen'd by path
(Qt's libqcocoa is the live example -- the tree ships none). Those need the runtime
pass: unlink the vendored formulae, run the entry points under DYLD_PRINT_LIBRARIES=1,
and assert every loaded image is inside the keg.

Usage: brewclosure.py <tree> [<tree> ...] [--json out.json]
"""
import json, os, re, subprocess, sys

HB = "/opt/homebrew/opt"
REF = re.compile(rf"{re.escape(HB)}/([^/]+)/(\S+\.(?:dylib|so))")
MACHO_EXT = (".dylib", ".so", ".bundle")


def is_macho(p):
    if p.endswith(MACHO_EXT):
        return True
    # executables: cheap filter, then let otool decide
    return os.access(p, os.X_OK) and not os.path.isdir(p) and "." not in os.path.basename(p)


def collect(paths):
    """otool -L a batch, return the set of /opt/homebrew/opt/... references."""
    found = set()
    for i in range(0, len(paths), 200):
        batch = paths[i:i + 200]
        try:
            out = subprocess.run(["otool", "-L", *batch], capture_output=True,
                                 text=True, timeout=1800).stdout
        except (subprocess.SubprocessError, OSError):
            continue
        for m in REF.finditer(out):
            found.add(f"{HB}/{m.group(1)}/{m.group(2)}")
    return found


def walk(tree):
    out = []
    for root, _, names in os.walk(tree):
        for n in names:
            p = os.path.join(root, n)
            if os.path.islink(p) or not os.path.isfile(p):
                continue
            if is_macho(p):
                out.append(p)
    return out


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    jout = None
    if "--json" in sys.argv:
        jout = sys.argv[sys.argv.index("--json") + 1]

    files = []
    for t in args:
        f = walk(t)
        print(f"  {t:<46} {len(f):>6} mach-o files", file=sys.stderr)
        files += f
    print(f"  {'TOTAL scanned':<46} {len(files):>6}", file=sys.stderr)

    # round 0: what the tree itself references
    frontier = collect(files)
    closure = set(frontier)
    rounds = 0
    # rounds 1..n: what the vendored dylibs themselves reference
    while frontier:
        rounds += 1
        existing = [p for p in frontier if os.path.exists(p)]
        new = collect(existing) - closure
        closure |= new
        frontier = new
        print(f"  closure round {rounds}: +{len(new)}", file=sys.stderr)

    present = sorted(p for p in closure if os.path.exists(p))
    missing = sorted(p for p in closure if not os.path.exists(p))
    total = sum(os.path.getsize(p) for p in present)
    formulae = sorted({p.split("/")[4] for p in closure})

    print(f"\n  CLOSURE: {len(closure)} dylibs, {len(formulae)} formulae, {rounds} rounds")
    print(f"  resolvable here : {len(present)}  ({total/1048576:.0f} MB)")
    print(f"  unresolvable    : {len(missing)}  (local brew differs from the build's)")
    print(f"\n  depends_on list ({len(formulae)}):")
    for i in range(0, len(formulae), 6):
        print("     " + "  ".join(f"{x:<18}" for x in formulae[i:i + 6]))

    if jout:
        json.dump({"formulae": formulae, "dylibs": present, "missing": missing,
                   "total_bytes": total, "rounds": rounds}, open(jout, "w"), indent=1)
        print(f"\n  wrote {jout}")


if __name__ == "__main__":
    main()
