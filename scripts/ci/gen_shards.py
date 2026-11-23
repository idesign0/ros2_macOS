#!/usr/bin/env python3
"""
Generate the layered-build shard lists for this distro's workspace from the
real colcon dependency graph.

Rule: a package depended on by >=2 domains sinks into the shared BASE layer;
every other package stays private to its own domain shard. This makes the
domain shards mutually independent (0 cross-shard edges) so they can build in
parallel on top of BASE.

Outputs (relative to workspace root):
  ci/shards/00_base_core.txt      core base (build tools, interfaces, client libs, middleware)
  ci/shards/01_base_vendors.txt   heavy compiled third-party (*_vendor) — split out so a
                                  vendor rebuild does not re-run the base spine
  ci/shards/dom_<domain>.txt      one file per domain shard
  ci/shards/_domains.txt          newline list of domain shard names (drives the CI matrix)
  ci/shards/COVERAGE.txt          manifest: proves union(shards) == (all packages - skip-list)

Skip-list: ci/skip-list.txt (one package name per line, '#' comments allowed) lists
packages excluded from the macOS build entirely (Lane 7 — unsupported platform, e.g.
proprietary DDS). They are removed from the graph before layering, so nothing is
assigned to depend on them.

Usage:  python3 scripts/ci/gen_shards.py [workspace_root]   (default: cwd)
Requires: colcon (colcon-common-extensions) on PATH.
"""
import os, sys, re, subprocess, json
from collections import defaultdict, deque

ROOT = os.path.abspath(sys.argv[1] if len(sys.argv) > 1 else ".")
SHARD_DIR = os.path.join(ROOT, "ci", "shards")
SKIP_FILE = os.path.join(ROOT, "ci", "skip-list.txt")
os.makedirs(SHARD_DIR, exist_ok=True)

def run(args):
    return subprocess.run(args, cwd=ROOT, check=True, capture_output=True, text=True).stdout

def load_skip():
    skip = set()
    if os.path.exists(SKIP_FILE):
        for line in open(SKIP_FILE):
            line = line.split("#", 1)[0].strip()
            if line:
                skip.add(line)
    return skip

def main():
    skip = load_skip()
    # authoritative package list: name <tab> path <tab> type
    listing = run(["colcon", "list", "--base-paths", ROOT])
    dom = {}
    for line in listing.splitlines():
        parts = line.split("\t")
        if len(parts) < 2:
            continue
        name, path = parts[0], parts[1]
        p = os.path.relpath(path, ROOT)
        domain = p.split(os.sep)[0] if os.sep in p else "(root)"
        dom[name] = domain
    # dependency edges (u depends on v)
    # colcon >=0.21 emits dot node ids as "<pkgname>_<objectaddress>"; map those
    # back to the real package name (colcon 0.19 emitted the plain name).
    known = set(dom)
    def canon(node):
        if node in known:
            return node
        stripped = re.sub(r'_\d+$', '', node)
        return stripped if stripped in known else node
    dot = run(["colcon", "graph", "--base-paths", ROOT, "--dot"])
    deps = defaultdict(set)
    rdeps = defaultdict(set)
    er = re.compile(r'^\s*"([^"]+)"\s*->\s*"([^"]+)"')
    for line in dot.splitlines():
        m = er.match(line)
        if not m:
            continue
        u, v = canon(m.group(1)), canon(m.group(2))
        dom.setdefault(u, "(external)")
        dom.setdefault(v, "(external)")
        if u != v:
            deps[u].add(v); rdeps[v].add(u)
    # drop skipped packages (and their edges) from the graph
    nodes = set(dom) - skip
    for p in skip:
        dom.pop(p, None)
    def clean(d):
        for k in list(d):
            if k in skip: del d[k]; continue
            d[k] = {x for x in d[k] if x not in skip}
    clean(deps); clean(rdeps)

    # domains that (transitively) depend on each package
    def dep_domains(p):
        seen = {p}; dq = deque([p]); doms = {dom[p]}
        while dq:
            x = dq.popleft()
            for u in rdeps.get(x, ()):
                if u not in seen:
                    seen.add(u); doms.add(dom[u]); dq.append(u)
        return doms
    shared = {p for p in nodes if len(dep_domains(p)) >= 2}
    private = nodes - shared

    base_vendors = sorted(p for p in shared if p.endswith("_vendor"))
    base_core = sorted(shared - set(base_vendors))
    shard = defaultdict(list)
    for p in private:
        shard[dom[p]].append(p)

    def write(fn, names):
        with open(os.path.join(SHARD_DIR, fn), "w") as f:
            f.write("\n".join(sorted(names)) + ("\n" if names else ""))
    write("00_base_core.txt", base_core)
    write("01_base_vendors.txt", base_vendors)
    domains = sorted(shard)
    for d in domains:
        write(f"dom_{d.replace('/', '_')}.txt", shard[d])
    write("_domains.txt", [])  # placeholder, rewritten below with order preserved
    with open(os.path.join(SHARD_DIR, "_domains.txt"), "w") as f:
        f.write("\n".join(domains) + "\n")

    written = set(base_core) | set(base_vendors)
    for d in domains: written |= set(shard[d])
    missing = nodes - written
    extra = written - nodes
    upward = [(p, v) for p in private for v in deps.get(p, ())
              if v in private and dom[v] != dom[p]]
    with open(os.path.join(SHARD_DIR, "COVERAGE.txt"), "w") as f:
        f.write(f"total_packages_after_skip={len(nodes)}\n")
        f.write(f"skipped={len(skip)}  ({', '.join(sorted(skip)) or '-'})\n")
        f.write(f"base_core={len(base_core)}\nbase_vendors={len(base_vendors)}\n")
        f.write(f"domain_shards={len(domains)}\nprivate_packages={len(private)}\n")
        f.write(f"written={len(written)}\nmissing={len(missing)}  ({', '.join(sorted(missing)) or '-'})\n")
        f.write(f"extra={len(extra)}\n")
        f.write(f"cross_shard_upward_edges={len(upward)}\n")
        f.write(f"COVERAGE_OK={missing == set() and written == nodes}\n")
        f.write(f"SHARDS_INDEPENDENT={len(upward) == 0}\n")
    ok = (missing == set() and written == nodes and not upward)
    print(f"[{os.path.basename(ROOT.rstrip('/')) or ROOT}] total={len(nodes)} "
          f"base_core={len(base_core)} base_vendors={len(base_vendors)} "
          f"shards={len(domains)} skipped={len(skip)} "
          f"COVERAGE_OK={missing==set()} INDEPENDENT={not upward}")
    if not ok:
        sys.exit(1)

if __name__ == "__main__":
    main()
