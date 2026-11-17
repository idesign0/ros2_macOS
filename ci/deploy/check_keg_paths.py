#!/usr/bin/env python3
"""check_keg_paths.py <tree> --prefix <install prefix> [--allow FILE] [--report FILE]

Release gate: every path the keg carries must point inside the keg, at something that
exists, or at the system. <tree> is the keg's files (the packaged install/ dir, or the
installed Cellar keg); <prefix> is where the formula puts them
(/opt/homebrew/opt/ros2-<distro>), so a reference to <prefix>/x is checked as <tree>/x.

Checks, over EVERY regular file (text and Mach-O alike):
  runner   /Users/runner/...              build machine's checkout, build or install dir
  tmp      /private/tmp, /private/var/folders, /tmp/buildd   build sandboxes (a plain
           /tmp/<name> is a runtime default the program really uses, not a leak)
  cellar   /opt/homebrew/Cellar/...       versioned keg paths; stale after any upgrade
  sdk      a VERSIONED SDK (MacOSX26.sdk) or /Applications/Xcode*   machine-specific
  python   a python.org framework other than 3.11 (the formula's documented requirement)
  kegref   <prefix>/... that does not exist in the keg (text files only)
A hit inside a binary (compiled-in string, static archive) is reported as <category>-bin:
a config is wrong if it names the build machine; a compiled-in string is only wrong if the
program looks it up, and bundled Homebrew bottles carry their own build records there.
and for Mach-O load commands:
  load     absolute load path outside the keg, the system and python.org 3.11
  rpath    @rpath/@loader_path/@executable_path load that resolves to nothing in the keg

An earlier gate asked "does this path exist on this machine?", which the CI runner
answered yes for python.org 3.12 and Homebrew simdjson, so jazzy 26.11.16.7 shipped
both. Nothing here consults the host filesystem except /usr/lib and /System.

--allow: lines "<category> <fnmatch glob on the keg-relative file> # reason". A finding is
allowed only when both match, so every exception is per category, per file and visible.
Exit 1 if any finding is not allowed.
"""
import argparse, collections, fnmatch, mmap, os, re, subprocess, sys

ap = argparse.ArgumentParser()
ap.add_argument("tree")
ap.add_argument("--prefix", required=True)
ap.add_argument("--allow")
ap.add_argument("--report")
a = ap.parse_args()
tree = os.path.realpath(a.tree)
prefix = a.prefix.rstrip("/")

MAGIC = {b"\xcf\xfa\xed\xfe", b"\xca\xfe\xba\xbe"}
END = rb"[^\s\"'`;:,()<>\[\]{}|\\\x00-\x1f\x7f-\xff]*"
PATTERNS = {
    "runner": re.compile(rb"/Users/runner/" + END),
    "tmp": re.compile(rb"(?:/private/tmp/|/private/var/folders/|/tmp/buildd/)" + END),
    "cellar": re.compile(rb"/opt/homebrew/Cellar/" + END),
    "sdk": re.compile(rb"(?:/[\w./-]*/SDKs/MacOSX\d[\d.]*\.sdk|/Applications/Xcode[\w.-]*\.app)" + END),
    "python": re.compile(rb"/Library/Frameworks/Python\.framework/Versions/(?!3\.11\b)\d+\.\d+" + END),
    "kegref": re.compile(re.escape(prefix.encode()) + rb"(?:/" + END + rb")?"),
}
PY311 = "/Library/Frameworks/Python.framework/Versions/3.11/"

allow = []
if a.allow:
    for line in open(a.allow):
        line = line.split("#", 1)[0].split()
        if len(line) >= 2:
            allow.append((line[0], line[1]))
def allowed(cat, rel):
    return any(c == cat and fnmatch.fnmatch(rel, g) for c, g in allow)

def in_keg(p):
    """True/False for a <prefix> path, None for anything else."""
    if p != prefix and not p.startswith(prefix + "/"):
        return None
    t = tree + p[len(prefix):]
    if os.path.lexists(t):
        return True
    # a prefix the consumer completes (gz's cmdfuel9 -> cmdfuel9.rb, lib/libfoo -> .dylib)
    d, b = os.path.split(t)
    return bool(b) and os.path.isdir(d) and any(n.startswith(b) for n in os.listdir(d))

findings = collections.defaultdict(lambda: collections.defaultdict(set))  # cat -> value -> files
def add(cat, value, rel):
    if not allowed(cat, rel):
        findings[cat][value].add(rel)

machos, n_files = [], 0
for root, dirs, names in os.walk(tree):
    for n in names:
        f = os.path.join(root, n)
        if os.path.islink(f) or not os.path.isfile(f):
            continue
        n_files += 1
        rel = os.path.relpath(f, tree)
        try:
            with open(f, "rb") as h:
                if os.fstat(h.fileno()).st_size == 0:
                    continue
                with mmap.mmap(h.fileno(), 0, access=mmap.ACCESS_READ) as m:
                    is_macho = m[:4] in MAGIC
                    is_text = not is_macho and b"\0" not in m[:8192]
                    if is_macho:
                        machos.append(f)
                    for cat, rx in PATTERNS.items():
                        if cat == "kegref" and not is_text:
                            continue
                        for hit in rx.finditer(m):
                            v = hit.group().decode("utf-8", "replace").rstrip(".")
                            if cat == "kegref":
                                v = v.rstrip("/")
                                if re.search(r"[$%*@]", v) or in_keg(v) is not False:
                                    continue
                            add(cat if is_text else cat + "-bin", v, rel)
        except (OSError, ValueError):
            continue

for i in range(0, len(machos), 200):
    batch = machos[i:i + 200]
    out = subprocess.run(["otool", "-l"] + batch, capture_output=True, text=True).stdout
    info, cur, cmd = {}, None, None
    for line in out.splitlines():
        if line.endswith(":") and line[:-1] in batch:
            cur = line[:-1]; info[cur] = ([], []); continue
        s = line.strip()
        if s.startswith("cmd "):
            cmd = s.split()[1]
        elif cur and s.startswith(("name ", "path ")):
            p = s.split(None, 1)[1].rsplit(" (offset", 1)[0]
            if cmd in ("LC_LOAD_DYLIB", "LC_REEXPORT_DYLIB", "LC_LOAD_UPWARD_DYLIB"):
                info[cur][0].append(p)
            elif cmd == "LC_RPATH":
                info[cur][1].append(p)
    for f, (loads, rpaths) in info.items():
        rel = os.path.relpath(f, tree)
        ldir = prefix + "/" + os.path.dirname(rel) if os.path.dirname(rel) else prefix
        rps = [r.replace("@loader_path", ldir).replace("@executable_path", ldir) for r in rpaths]
        for p in loads:
            if p.startswith(("/usr/lib/", "/System/")) or p.startswith(PY311):
                continue
            if p.startswith("/"):
                if in_keg(p) is not True:
                    add("load", p, rel)
                continue
            if p.startswith(("@loader_path/", "@executable_path/")):
                cands = [p.replace("@loader_path", ldir).replace("@executable_path", ldir)]
            elif p.startswith("@rpath/"):
                cands = [os.path.join(r, p[len("@rpath/"):]) for r in rps]
            else:
                continue
            ok = False
            for c in cands:
                c = os.path.normpath(c)
                if in_keg(c) or c.startswith(("/usr/lib/", "/System/")) or c.startswith(PY311):
                    ok = True; break
            if not ok:
                add("rpath", p, rel)

lines = [f"files scanned: {n_files}   mach-o: {len(machos)}   allow rules: {len(allow)}"]
total = 0
for cat in ("runner", "tmp", "cellar", "sdk", "python", "kegref", "load", "rpath",
            "runner-bin", "tmp-bin", "cellar-bin", "sdk-bin", "python-bin"):
    vals = findings.get(cat, {})
    files = collections.defaultdict(set)
    for v, fs in vals.items():
        for f in fs:
            files[f].add(v)
    total += len(files)
    lines.append(f"\n## {cat}: {len(files)} files, {len(vals)} distinct values")
    for f in sorted(files):
        vs = sorted(files[f])
        lines.append(f"  {f}\n      {vs[0][:200]}" + (f"   (+{len(vs) - 1} more)" if len(vs) > 1 else ""))
text = "\n".join(lines) + "\n"
if a.report:
    open(a.report, "w").write(text)
summary = [l for l in lines if l.startswith(("files", "\n## "))]
print("\n".join(s.strip() for s in summary))
if total:
    print(f"::error::{total} file(s) carry a path that does not point inside the keg"
          + (f" (full list: {a.report})" if a.report else ""))
    sys.exit(1)
print("every path in the keg points inside the keg or at the system")
