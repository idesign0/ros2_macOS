#!/usr/bin/env python3
"""
Package a built ROS 2 install tree so it does not depend on the user's Homebrew.

Does, at BUILD time, the two jobs the formula's post_install used to do at INSTALL time
(Homebrew 7 deprecates `post_install` and its replacement `post_install_steps` is a closed
set of built-in actions, so custom Ruby there has no future), plus the vendoring that
makes the stale-reference repair unnecessary:

  1. VENDOR  -- copy every Homebrew dylib AND framework bundle in the tree's closure into
                <tree>/vendor/lib, rewrite each reference to @rpath/<name>, and give
                every mach-o an absolute rpath to that directory. After this the tree
                references no /opt/homebrew/opt path at all, so it cannot be broken by the
                user's brew being newer, older, or missing a library upstream deleted.
  2. RPATHS   -- the surviving half of post_install: absolute rpaths for the in-tree
                opt/*/lib vendor dirs. macOS Hardened Runtime does not expand @loader_path
                for a plugin loaded via dlopen, so gz/pluginlib plugins need the absolute
                path baked in.

ABSOLUTE rpaths, not @loader_path: that is what the existing post_install settled on for
exactly the dlopen reason above, and the files here sit at many different depths.

Flat WHERE IT CAN BE, scoped where it cannot. Measured across base + all 21 kilted domains
the 478-dylib closure had 478 distinct basenames, so flattening was safe. It is not safe in
general and is no longer true: brew's `vtk` bottle links Qt6 (qtbase, qtsvg, qtdeclarative)
while rviz and the rqt stack link qt@5, so QtCore, QtGui, QtOpenGL, QtWidgets, QtNetwork
and QtSvg each arrive twice from two incompatible majors. A formula whose soname omits the
version (libcrypto.3.dylib, libdart.6.19.dylib) can do the same, unlike abseil whose soname
carries it (libabsl_hash.2608.0.0.dylib).

A name claimed by two DIFFERENT files is therefore not flattened: each claimant goes to
vendor/lib/<keg>/<name> and every reference is rewritten to the scoped path of the file it
actually named. ONE rpath still serves the whole tree -- what became unique is the names
under it, so there is nothing to resolve per consumer. Same-file duplicates (the opt/ vs
realpath'd Cellar/ routes to one library) still collapse onto a single copy.

Scoping preserves what each binary linked; it reconciles nothing. A single mach-o naming
two kegs for one library was already broken before packaging -- two Qt majors cannot share
a process -- so that is reported and left exactly as built, not silently decided.

Run it in the `assemble` job between "Merge shard deltas" and "Pack full workspace
install".

Usage:
  package_tree.py <tree> --install-prefix /opt/homebrew/opt/ros2-kilted
  package_tree.py <tree> --install-prefix ... --tolerate-stale-input   # smoke only
  package_tree.py <tree> --install-prefix ... --allow-missing          # local only
  package_tree.py <tree> --install-prefix ... --verify-only
"""
import argparse, collections, io, json, os, re, shutil, subprocess, sys

HB = "/opt/homebrew/opt"
# A reference is either a plain dylib/so, or a binary INSIDE a .framework bundle. Qt is
# the reason the second alternative exists: qt@5 ships 72 frameworks and ZERO dylibs, so a
# dylib-only pattern never matched Qt at all, nothing Qt-related was ever vendored, and the
# shipped tree kept pointing at /opt/homebrew/opt/qt@5/.... That is why qt@5 was still
# loading from Homebrew after packaging in smoke run 41055576065, and it means RViz, rqt and
# gz-gui stayed exposed to the user's Qt version exactly as gz-sim was to abseil.
CELLAR = "/opt/homebrew/Cellar"
# Both roots. Qt5's own frameworks name each OTHER by absolute Cellar path
# (QtGui -> /opt/homebrew/Cellar/qt@5/5.15.19/lib/QtCore.framework/Versions/5/QtCore), and
# so do glib, mesa, ffmpeg, apache-arrow and others. With an opt-only pattern those refs
# were never rewritten: the vendored QtGui kept loading the user's brew QtCore beside our
# vendored one, and two QtCores in one process abort (exit 134). That was every gz plugin
# in the smoke sample, on both distros. vendor_unit() keys on realpath, so an opt route and
# a Cellar route into one keg still collapse onto a single vendored copy.
REF = re.compile(
    rf"(?:{re.escape(HB)}/[^/\s]+|{re.escape(CELLAR)}/[^/\s]+/[^/\s]+)/"
    r"(?:\S+\.(?:dylib|so)|\S*?[^/]+\.framework/\S+)")
MACHO_EXT = (".dylib", ".so", ".bundle")
MACHO_MAGIC = {bytes.fromhex(h) for h in
               ("feedface", "cefaedfe", "feedfacf", "cffaedfe", "cafebabe", "bebafeca")}

# .../QtCore.framework/Versions/5/QtCore -> (framework dir, "QtCore", "Versions/5/QtCore")
FW = re.compile(r"^(?P<fwdir>.*?/(?P<name>[^/]+)\.framework)/(?P<rest>.+)$")


def fw_parts(p):
    m = FW.match(p)
    return (m.group("fwdir"), m.group("name"), m.group("rest")) if m else None


def rpath_name(p):
    """What this reference becomes under @rpath. A framework keeps its bundle-relative
    path, because dyld resolves @rpath/QtCore.framework/Versions/5/QtCore against the
    rpath -- flattening it to the bare basename would break the bundle layout and the
    Resources/Info.plist lookups that come with it."""
    f = fw_parts(p)
    if f:
        _, name, rest = f
        return f"{name}.framework/{rest}"
    return os.path.basename(p)


def keg_of(p):
    """The formula a brew path belongs to -- the scope dir name when a name is claimed
    twice. Both routes into a keg are recognised, because the closure reaches libraries
    by /opt/homebrew/opt/<formula> (absolute refs) and by realpath'd Cellar/ paths
    (@rpath siblings), and those must score the same scope."""
    m = re.match(rf"^{re.escape(HB)}/([^/]+)/", p)
    if m:
        return m.group(1)
    m = re.match(r"^/opt/homebrew/Cellar/([^/]+)/", p)
    return m.group(1) if m else "other"


def keg_version(p):
    """Cellar version of a brew path, for the case where ONE formula has two versions
    installed and the formula name alone cannot separate them (proven with two libssh2
    kegs: both ship libssh2.1.dylib, different contents)."""
    for q in (p, os.path.realpath(p)):
        m = re.match(r"^/opt/homebrew/Cellar/[^/]+/([^/]+)/", q)
        if m:
            return m.group(1)
    return None


def vendor_key(p):
    """The name this closure member competes for in the vendor dir: a framework competes
    as its whole BUNDLE (the bundle dir is what gets copied), a dylib as its basename."""
    f = fw_parts(p)
    return f"{f[1]}.framework" if f else os.path.basename(p)


def vendor_unit(p):
    """Identity of the thing being vendored, so two paths to one library collapse and two
    libraries never do. Keyed on realpath: refs_of() yields /opt/homebrew/opt/... while
    rpath_refs() realpaths to /opt/homebrew/Cellar/..., and those are one file."""
    f = fw_parts(p)
    return ("fw", os.path.realpath(f[0])) if f else ("lib", os.path.realpath(p))


def run(cmd):
    return subprocess.run(cmd, capture_output=True, text=True)


def is_macho(p):
    if p.endswith(MACHO_EXT):
        return True
    return os.access(p, os.X_OK) and os.path.isfile(p) and "." not in os.path.basename(p)


def walk_macho(tree):
    out = []
    for root, _, names in os.walk(tree):
        for n in names:
            p = os.path.join(root, n)
            if not os.path.islink(p) and os.path.isfile(p) and is_macho(p):
                out.append(p)
    return out


# The keg's OWN install prefix. When it lives under /opt/homebrew/opt (it always does:
# /opt/homebrew/opt/ros2-<distro>), a reference the relocation step just pointed at the
# keg's own libraries -- .../opt/ros2-kilted/lib/liblely-libc.2.dylib -- matches REF like
# any brew keg would. It is not Homebrew: it is this tree, at the place it will be
# installed. Smoke run 42504451826 failed its verify on exactly 21 such self-references
# (lely, gtsam, the mola CLIs). Set once in main(); every caller of refs_of() skips them.
SELF_PREFIXES = ()


def _not_self(ref):
    return not any(ref == p or ref.startswith(p + "/") for p in SELF_PREFIXES)


def refs_of(paths):
    """path -> set of /opt/homebrew/{opt,Cellar}/... references, excluding the keg itself."""
    out = {}
    for i in range(0, len(paths), 150):
        batch = paths[i:i + 150]
        r = run(["otool", "-L", *batch])
        cur = None
        for line in r.stdout.splitlines():
            if not line.startswith("\t") and line.rstrip(":") in batch:
                cur = line.rstrip(":")
                out.setdefault(cur, set())
            elif cur is not None:
                m = REF.search(line)
                if m and _not_self(m.group(0)):
                    out[cur].add(m.group(0))
        # single-file batches print no header line
        if len(batch) == 1 and batch[0] not in out:
            out[batch[0]] = {m.group(0) for m in REF.finditer(r.stdout)
                             if _not_self(m.group(0))}
    return out


def has_rpath(path, want):
    """Whether `path` already carries `want` as an LC_RPATH. Parsed, not substring-matched
    against the whole `otool -l` dump: that dump opens with the file's OWN path, so every
    file inside the vendor dir matched its own vendor rpath and was skipped. Nothing
    under vendor/lib ever got the rpath, which only stayed invisible because dyld
    resolves a nested @rpath against the LC_RPATHs of the loading chain -- so the in-tree
    consumer's rpath covered it. It stops being true the moment something dlopens a
    vendored dylib directly."""
    return want in [m.group(1).strip() for m in re.finditer(
        r"^\s*path (.+?) \(offset \d+\)$", run(["otool", "-l", path]).stdout, re.M)]


def rpath_deps(path):
    """dep TEXT -> the brew file it resolves to, for every @rpath/@loader_path dep of
    `path`. The text matters as well as the target: a scoped library is no longer at the
    name such a dep spells, so step 2b has to rewrite the dep itself."""
    out = {}
    d = os.path.dirname(path)
    lc = run(["otool", "-l", path]).stdout
    rpaths = [m.group(1).strip() for m in
              re.finditer(r"^\s*path (.+?) \(offset \d+\)$", lc, re.M)]
    rpaths = [r.replace("@loader_path", d) for r in rpaths]
    deps = run(["otool", "-L", path]).stdout.splitlines()[2:]  # [0] header, [1] own ID
    for line in deps:
        t = line.strip().split(" ")[0]
        if t.startswith("@loader_path/"):
            cand = [t.replace("@loader_path", d)]
        elif t.startswith("@rpath/"):
            suf = t[len("@rpath/"):]
            cand = [os.path.join(r, suf) for r in rpaths]
        else:
            continue
        for c in cand:
            c = os.path.normpath(c)
            if c.startswith(HB) or c.startswith("/opt/homebrew/Cellar"):
                if os.path.exists(c):
                    out[t] = os.path.realpath(c)
                    break
    return out


def rpath_refs(path):
    """Brew dylibs reference their SIBLINGS via @rpath/@loader_path, resolved against
    their own LC_RPATH -- typically @loader_path/../lib, i.e. their own keg's lib dir.
    Measured: 187 of 406 vendored brew dylibs do this. An absolute-path-only scan misses
    every one of them, and copying such a dylib into a flat vendor dir silently breaks it
    because @loader_path now points somewhere else. That was 20 regressions out of 102
    dylibs that had loaded before packaging (libgeos_c -> @rpath/libgeos.3.15.0.dylib).
    So resolve them here and pull the real files into the closure too."""
    return set(rpath_deps(path).values())


_GONE = {}


def formula_retired(name):
    """Reason string if Homebrew has DISABLED this formula or dropped it from core,
    else None.

    Such a reference can never resolve again on any machine, so it is the terminal form
    of stale input -- NOT an undeclared dependency. Treating the two alike was wrong:
    icu4c@77 was disabled on 2026-10-30 ("versioned_formula"), and from that morning on
    every job asking for it failed in setup, which is how run 41046526636 died before it
    reached packaging at all. The formula was installed when the tree was built and is
    now uninstallable, which is drift, not a missing declaration.

    Reads the local homebrew-core tap (the jobs set HOMEBREW_NO_INSTALL_FROM_API=1), so
    no network. If brew itself is unavailable we return None and stay conservative: the
    ref keeps its fatal classification rather than being waved through.
    """
    if name in _GONE:
        return _GONE[name]
    try:
        r = run(["brew", "info", "--json=v2", name])
    except OSError:
        _GONE[name] = None
        return None
    if r.returncode != 0:
        # No such formula in core. Either Homebrew removed it, or it came from a tap
        # that is not tapped here -- distinguishable only by a human, so say which.
        _GONE[name] = "unknown to brew (removed from core, or an untapped tap)"
    else:
        try:
            f = json.loads(r.stdout)["formulae"][0]
            _GONE[name] = (f"disabled: {f.get('disable_reason') or 'no reason given'}"
                           if f.get("disabled") else None)
        except (ValueError, KeyError, IndexError):
            _GONE[name] = None
    return _GONE[name]


def closure(tree_files):
    seen, frontier = set(), set()
    for s in refs_of(tree_files).values():
        frontier |= s
    seen |= frontier
    while frontier:
        existing = [p for p in frontier if os.path.exists(p)]
        new = set()
        for s in refs_of(existing).values():
            new |= s
        for p in existing:
            new |= rpath_refs(p)
        new -= seen
        seen |= new
        frontier = new
    return seen


def resign(path):
    run(["codesign", "--force", "--sign", "-", path])


def build_prefixes(tree, extra):
    """The absolute path(s) the tree was BUILT at. colcon and ament bake it into the
    tree -- ament's generated local_setup.sh carries it as the fallback
    `: ${AMENT_CURRENT_PREFIX:="<prefix>"}`, which makes that file the authoritative
    record even when the tree is packaged somewhere else (a local test, a different
    runner path). The tree's own current location is included too: in CI they are the
    same path."""
    out = {os.path.abspath(tree)}
    out.update(os.path.abspath(e) for e in extra)
    share = os.path.join(tree, "share")
    pat = re.compile(r'AMENT_CURRENT_PREFIX:="([^"]+)"')
    try:
        for pkg in sorted(os.listdir(share))[:400]:
            f = os.path.join(share, pkg, "local_setup.sh")
            if os.path.isfile(f):
                m = pat.search(io.open(f, encoding="utf-8", errors="replace").read())
                if m:
                    out.add(m.group(1).rstrip("/"))
    except OSError:
        pass
    return sorted(out, key=len, reverse=True)   # longest first: never half-rewrite a nest


LC_NAME = re.compile(r"^\s*cmd (LC_ID_DYLIB|LC_LOAD_DYLIB|LC_LOAD_WEAK_DYLIB|"
                     r"LC_REEXPORT_DYLIB|LC_LAZY_LOAD_DYLIB|LC_RPATH)$")


def load_paths(path):
    """(command, path) for every dylib-naming and rpath load command of a mach-o."""
    out, cur = [], None
    for line in run(["otool", "-l", path]).stdout.splitlines():
        m = LC_NAME.match(line)
        if m:
            cur = m.group(1)
            continue
        if cur:
            m = re.match(r"^\s*(?:name|path) (.+?) \(offset \d+\)$", line)
            if m:
                out.append((cur, m.group(1)))
                cur = None
    return out


ENV_PKG = "ros2_macos_install_env"


def write_install_prefix_env(tree):
    """Tell each gz library where it ACTUALLY lives, via the override gz built in for it.

    gz libraries compile their install prefix in, and derive every plugin, engine,
    world and GUI-config directory from it. Relocating text and load commands cannot
    reach those strings, and on a user's machine they name the build machine: measured
    on the jazzy tree, gz sim then starts and loads NONE of its default systems --
    physics, user-commands, scene-broadcaster all "Could not find shared library" -- so
    the server runs and simulates nothing. (No CI test can see it: on the runner the
    compiled-in path exists.)

    gz-cmake gives each project an override for exactly this: getInstallPrefix() honours
    GZ_<PROJECT>_INSTALL_PREFIX before the compiled-in value. So read which of those
    variables each library actually defines, from the binary itself, and set it to the
    directory that library sits in. A library under opt/<pkg>/lib was built for prefix
    opt/<pkg>; one under lib/ for the tree root. The ROS vendor copy (opt/) wins over
    the top-level gz build where both define a variable, because that is the copy the
    gz CLI loads (gz_*_vendor put their share/gz first on GZ_CONFIG_PATH).

    Written as colcon `set` lines with RELATIVE values -- colcon resolves those against
    the prefix at source time -- in a small package colcon discovers like any other, so
    nothing absolute is baked in and the tree stays relocatable."""
    pat = re.compile(rb"\x00(GZ_[A-Z0-9]+_INSTALL_PREFIX)\x00")
    found = {}
    dirs = [os.path.join(tree, "lib")] + sorted(
        os.path.join(tree, "opt", d, "lib") for d in
        (os.listdir(os.path.join(tree, "opt")) if os.path.isdir(os.path.join(tree, "opt")) else []))
    for d in dirs:
        if not os.path.isdir(d):
            continue
        rel = os.path.relpath(os.path.dirname(d), tree)
        rel = "" if rel == "." else rel
        for n in sorted(os.listdir(d)):
            f = os.path.join(d, n)
            if not n.endswith(".dylib") or os.path.islink(f) or not os.path.isfile(f):
                continue
            try:
                raw = open(f, "rb").read()
            except OSError:
                continue
            for m in pat.finditer(raw):
                var = m.group(1).decode()
                if var not in found or (rel and not found[var]):   # opt/ beats the root
                    found[var] = rel
    if not found:
        return {}
    meta = os.path.join(tree, "share", "colcon-core", "packages")
    deps = sorted({v.split("/")[1] for v in found.values() if v.startswith("opt/")})
    deps = [x for x in deps if os.path.exists(os.path.join(meta, x))]
    os.makedirs(meta, exist_ok=True)
    io.open(os.path.join(meta, ENV_PKG), "w").write(":".join(deps))
    d = os.path.join(tree, "share", ENV_PKG)
    os.makedirs(d, exist_ok=True)
    io.open(os.path.join(d, "package.dsv"), "w").write(
        "".join(f"set;{var};{rel}\n" for var, rel in sorted(found.items())))
    return found


def add_intree_rpaths(tree, install_prefix, targets):
    """Make every in-tree @rpath dependency resolvable through rpaths ALONE.

    Without this, a lot of the tree only loads when DYLD_LIBRARY_PATH is set -- and on
    macOS that variable does not survive: dyld strips every DYLD_* variable from a
    SIP-protected binary, and /usr/bin/env, /bin/sh and /usr/bin/ruby all are. So
    `#!/usr/bin/env ruby` (bin/gz) loses it before any of our code runs. Measured on the
    jazzy tree: opt/gz_sim_vendor/lib/libgz-sim8-gz needs @rpath/libgz-fuel_tools9.9,
    and gz-transport-topic needs @rpath/libgz-utils2.2. Both live in the tree's TOP-LEVEL
    lib/, which none of their @loader_path rpaths reach, and which the previous version of
    this step never considered -- it only walked opt/*/lib.

    For each file, resolve its @rpath/ deps against the rpaths it already has, and append
    the installed directory of the first provider for any that do not resolve,
    preferring the file's own opt/<pkg> and then the top-level lib/. Appending keeps the
    existing rpaths first, so nothing that resolves today changes which copy it gets.

    Also O(files): one otool per file. The old loop ran otool -L and otool -l once per
    (directory x file) pair -- on the order of 10^5 process spawns per packaging run."""
    dirs = set()
    if os.path.isdir(os.path.join(tree, "lib")):
        dirs.add(os.path.join(tree, "lib"))
    for r, _, ns in os.walk(os.path.join(tree, "opt")):
        if any(n.endswith(".dylib") for n in ns):
            dirs.add(r)
    provider = collections.defaultdict(list)          # leaf -> [tree dir, ...]
    for d in sorted(dirs):
        for n in os.listdir(d):
            if n.endswith(".dylib"):
                provider[n].append(d)

    def to_tree(path_):
        if path_ == install_prefix or path_.startswith(install_prefix + "/"):
            return os.path.join(tree, os.path.relpath(path_, install_prefix))
        return path_

    added = 0
    for p in targets:
        lp = load_paths(p)
        rps = [q for c, q in lp if c == "LC_RPATH"]
        here = os.path.dirname(p)
        rdirs = [to_tree(r.replace("@loader_path", here).replace("@executable_path", here))
                 for r in rps]
        want = []
        own = os.path.relpath(p, tree).split(os.sep)
        own_pkg = os.path.join(tree, "opt", own[1]) if own[0] == "opt" and len(own) > 1 else None
        for c, q in lp:
            if c == "LC_RPATH" or c == "LC_ID_DYLIB" or not q.startswith("@rpath/"):
                continue
            leaf = q[len("@rpath/"):]
            if "/" in leaf or leaf not in provider:
                continue          # frameworks / scoped vendor names, or not ours
            if any(os.path.exists(os.path.join(d, leaf)) for d in rdirs):
                continue
            cands = provider[leaf]
            pick = next((d for d in cands if own_pkg and d.startswith(own_pkg + os.sep)), None) \
                or next((d for d in cands if d == os.path.join(tree, "lib")), None) or cands[0]
            inst = os.path.join(install_prefix, os.path.relpath(pick, tree))
            if inst not in rps and inst not in want:
                want.append(inst)
            rdirs.append(pick)
        if not want:
            continue
        cmd = ["install_name_tool"]
        for w in want:
            cmd += ["-add_rpath", w]
        if run(cmd + [p]).returncode == 0:
            added += len(want)
    return added


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("tree")
    ap.add_argument("--install-prefix", required=True,
                    help="where the tree will live once installed, e.g. "
                         "/opt/homebrew/opt/ros2-kilted")
    ap.add_argument("--vendor-subdir", default="vendor/lib")
    ap.add_argument("--tolerate-stale-input", action="store_true",
                    help="tolerate ONLY the stale-input class of missing ref: the formula "
                         "is installed but at a version that no longer ships that soname, "
                         "i.e. Homebrew moved after the tree was built. A ref whose "
                         "formula is not installed at all still fails. For the package "
                         "smoke job, whose subject is the packager and not the tree.")
    ap.add_argument("--allow-missing", action="store_true",
                    help="tolerate every missing ref, including an absent formula "
                         "(LOCAL EXPERIMENTS ONLY -- implies --tolerate-stale-input)")
    ap.add_argument("--verify-only", action="store_true")
    ap.add_argument("--build-prefix", action="append", default=[],
                    help="an absolute path the tree was built at, to relocate to "
                         "--install-prefix. Normally auto-detected from the tree's ament "
                         "local_setup.sh; repeatable. For packaging a tree away from the "
                         "machine that built it.")
    a = ap.parse_args()

    tree = a.tree.rstrip("/")
    vendor_abs = os.path.join(tree, a.vendor_subdir)
    global SELF_PREFIXES
    SELF_PREFIXES = (os.path.normpath(a.install_prefix),)
    rpath_installed = os.path.join(a.install_prefix, a.vendor_subdir)

    files = walk_macho(tree)
    print(f"  mach-o files in tree : {len(files)}", flush=True)

    if a.verify_only:
        bad = {p: s for p, s in refs_of(files).items() if s}
        print(f"  files still referencing {HB}: {len(bad)}")
        for p, s in list(bad.items())[:10]:
            print(f"     {p[len(tree)+1:]}  -> {sorted(s)[:2]}")
        sys.exit(1 if bad else 0)

    cl = closure(files)
    present = sorted(p for p in cl if os.path.exists(p))
    missing = sorted(p for p in cl if not os.path.exists(p))
    print(f"  closure              : {len(cl)} dylibs "
          f"({len(present)} present, {len(missing)} missing)", flush=True)

    # Two very different things make a closure ref missing, and only one of them is a
    # bug in this script or in the dependency set:
    #
    #   DRIFTED -- the formula is installed, but at a version that no longer ships that
    #              soname. Homebrew moved between the build and now. Nothing is wrong
    #              with the closure walk; the INPUT TREE is stale. Measured 2026-10-29:
    #              a kilted tree two days old already wanted libdraco.9 and
    #              librealsense2.2.58, and core had moved to draco 1.5.9 / librealsense
    #              2.59.1. This is the abseil-2601 failure mode and the reason the tree
    #              gets vendored at all.
    #   GONE    -- the formula is not installed AND Homebrew has disabled it or dropped
    #              it from core, so it can never be installed again. Still drift, just
    #              the terminal kind: it WAS there at build time. icu4c@77, disabled
    #              2026-10-30.
    #   ABSENT  -- the formula is not installed and is still perfectly installable.
    #              /opt/homebrew/opt/<formula> is kept even by `brew unlink`, so an
    #              absent dir here means it was never installed on this machine: an
    #              undeclared dependency, or a closure walk that invented a path.
    #              Always fatal -- only --allow-missing waves it through.
    tolerated = set()
    if missing:
        byf = collections.Counter(p.split("/")[4] for p in missing)
        drifted, gone, absent = [], [], []
        for p in missing:
            f = p.split("/")[4]
            if os.path.isdir(os.path.join(HB, f)):
                drifted.append(p)
            elif formula_retired(f):
                gone.append(p)
            else:
                absent.append(p)
        for p in missing[:12]:
            print(f"     MISSING {p}")
        print(f"     ... across {len(byf)} formulae: "
              f"{', '.join(f'{k}({v})' for k, v in byf.most_common(6))}")

        if drifted:
            vers = {}
            for p in drifted:
                f = p.split("/")[4]
                vers.setdefault(f, os.path.basename(
                    os.path.realpath(os.path.join(HB, f))))
            print(f"  stale-input refs     : {len(drifted)} across "
                  f"{len(vers)} formulae -- Homebrew moved after the build")
            for f, v in sorted(vers.items()):
                print(f"     {f}: installed {v}, tree was built against an older soname")
        if gone:
            gf = sorted(set(p.split("/")[4] for p in gone))
            print(f"  retired formulae     : {len(gone)} refs across {len(gf)} formulae "
                  f"Homebrew has disabled or removed")
            for f in gf:
                print(f"     {f}: {formula_retired(f)}")
            print(f"     these can never resolve again on any machine -- the tree MUST "
                  f"be rebuilt before it is shipped")
        if absent:
            af = sorted(set(p.split("/")[4] for p in absent))
            print(f"  absent formulae      : {len(absent)} refs across {len(af)} "
                  f"formulae not installed here but still installable: {', '.join(af)}")

        if absent and not a.allow_missing:
            print("::error::the closure references formulae that are not installed on "
                  "this machine yet are still installable. That is not Homebrew drift "
                  "-- it is an undeclared dependency or a bad closure walk. Refusing to "
                  "package.")
            sys.exit(1)

        if (drifted or gone) and not (a.tolerate_stale_input or a.allow_missing):
            print("::error::the tree references Homebrew dylibs that do not exist. It "
                  "cannot load as built -- this is the abseil 2601 failure. Refusing to "
                  "package. Rebuild against the current brew.")
            sys.exit(1)

        tolerated = set(drifted) | set(gone) | (set(absent) if a.allow_missing else set())
        print(f"::warning::packaging over {len(tolerated)} missing reference(s): the "
              f"vendor dir will be incomplete and anything that needed them could not "
              f"load before packaging either. Load-test results stay comparable; the "
              f"shipped tarball must come from a tree built against the current brew.")

    # ---- 1. plan the vendor layout, THEN copy.
    # A name wanted by exactly one library stays flat -- vendor/lib/<name>, as it always
    # was. A name wanted by two DIFFERENT libraries cannot: dropping either breaks
    # whatever linked it, and nothing here can know which caller mattered. Both go to
    # vendor/lib/<keg>/<name> instead. Six names need this on every distro right now
    # (QtCore, QtGui, QtOpenGL, QtWidgets, QtNetwork, QtSvg) because brew's vtk bottle
    # pulls Qt6 in while rviz and rqt link qt@5 -- transitively, through a prebuilt
    # bottle, which is why CMAKE_PREFIX_PATH ordering cannot prevent it.
    units = collections.defaultdict(dict)   # key -> {unit: first source path seen}
    for src in present:
        units[vendor_key(src)].setdefault(vendor_unit(src), src)

    dest_rel = {}         # unit -> its path relative to the vendor dir
    scoped = {}           # key -> {scope: src}, for the report
    clashes = []
    for key, members in sorted(units.items()):
        if len(members) == 1:
            dest_rel[next(iter(members))] = key
            continue
        # group by keg first: only a keg wanted twice needs its version in the name, so
        # the common case reads vendor/lib/qt@5/QtCore.framework, not qt@5-5.15.19/...
        by_keg = collections.defaultdict(list)
        for unit, src in members.items():
            by_keg[keg_of(src)].append((unit, src))
        used = {}
        for keg, group in sorted(by_keg.items()):
            for unit, src in group:
                scope = keg
                if len(group) > 1:
                    v = keg_version(src)
                    scope = f"{keg}-{v}" if v else keg
                if scope in used:
                    clashes.append((key, used[scope], src))
                    continue
                used[scope] = src
                dest_rel[unit] = f"{scope}/{key}"
        scoped[key] = used

    if clashes:
        print(f"::error::{len(clashes)} vendor name(s) cannot be made unique: two different "
              f"files share one name AND one keg at one version, so scoping has nothing left "
              f"to separate them by. Refusing to package.")
        for key, first, second in clashes[:20]:
            print(f"     {key}\n        {first}\n        {second}")
        sys.exit(1)

    os.makedirs(vendor_abs, exist_ok=True)
    base_of = {}          # source path -> vendored path relative to the vendor dir
    origin_of = {}        # vendored file -> the source it was copied from
    done_fw, done_lib = set(), set()
    for src in present:
        unit = vendor_unit(src)
        rel = dest_rel[unit]
        fp = fw_parts(src)
        if fp:
            # Copy the ENTIRE bundle once (symlinks preserved: Versions/Current is one,
            # and Qt's own dylib-style aliases depend on it), then treat the binary inside
            # it as the vendored file for every later step.
            fwdir, _, rest = fp
            if unit not in done_fw:
                done_fw.add(unit)
                dstdir = os.path.join(vendor_abs, rel)
                os.makedirs(os.path.dirname(dstdir), exist_ok=True)
                if not os.path.exists(dstdir):
                    shutil.copytree(fwdir, dstdir, symlinks=True)
            base_of[src] = f"{rel}/{rest}"
        else:
            if unit not in done_lib:
                done_lib.add(unit)
                dst = os.path.join(vendor_abs, rel)
                os.makedirs(os.path.dirname(dst), exist_ok=True)
                if not os.path.exists(dst):
                    shutil.copy2(src, dst)
                    os.chmod(dst, 0o755)
            base_of[src] = rel
        origin_of.setdefault(os.path.join(vendor_abs, base_of[src]), src)

    def vendor_rel(p):
        """Where a closure member ended up, relative to the vendor dir. Resolved by UNIT,
        not by string, so a reference naming the bundle's top-level alias
        (QtCore.framework/QtCore) lands in the same scope dir as one naming
        Versions/5/QtCore, and either route into a keg gives the same answer."""
        if p in base_of:
            return base_of[p]
        fp = fw_parts(p)
        rel = dest_rel.get(vendor_unit(p))
        if rel is None:
            return rpath_name(p)
        return f"{rel}/{fp[2]}" if fp else rel

    # only plain dylibs can duplicate: two binaries inside ONE framework bundle are
    # two different files, not two routes to one.
    dupes = sum(1 for p in present if not fw_parts(p)) - len(done_lib)
    print(f"  vendored             : {len(done_lib)} dylibs"
          + (f" + {len(done_fw)} framework bundle(s)" if done_fw else "")
          + f" into {a.vendor_subdir}"
          + (f" ({dupes} duplicate path(s) of the same file collapsed)" if dupes else ""),
          flush=True)
    if scoped:
        print(f"  scoped               : {len(scoped)} name(s) claimed by more than one "
              f"keg, kept apart in per-keg subdirs")
        for key, used in sorted(scoped.items()):
            print(f"     {key}: {', '.join(sorted(used))}")

    vendored = [os.path.join(vendor_abs, b) for b in sorted(set(base_of.values()))]

    # vendored copies: own install_name -> @rpath, and their refs rewritten too
    for v in vendored:
        # relpath, not basename: a framework binary's id must stay bundle-relative
        # (@rpath/QtCore.framework/Versions/5/QtCore). For a plain dylib the two are equal.
        run(["install_name_tool", "-id",
             f"@rpath/{os.path.relpath(v, vendor_abs)}", v])

    # ---- 1b. in-tree COPIES of brew dylibs carry the brew path as their OWN
    # install_name (LC_ID_DYLIB). The tree ships such copies deliberately -- the
    # abseil-compat/ dir holds brew abseil dylibs so two sonames can coexist, per the
    # abseil soname-drift fix. otool -L reports a dylib's own ID as its first line, so
    # leaving these alone means the tree still "references" /opt/homebrew even though
    # the file is local. Re-identify them to @rpath so they resolve inside the keg.
    reid = 0
    for p in files:
        out = run(["otool", "-D", p]).stdout.splitlines()
        own = out[1].strip() if len(out) >= 2 else ""
        if own.startswith(HB) or own.startswith(CELLAR):
            if run(["install_name_tool", "-id",
                    f"@rpath/{rpath_name(p)}", p]).returncode == 0:
                reid += 1
    print(f"  re-identified         : {reid} in-tree copies of brew dylibs", flush=True)

    # ---- 1c. relocate the BUILD prefix to the INSTALL prefix.
    # Everything above takes the tree off the user's Homebrew. This takes it off the
    # machine that BUILT it. colcon, ament, CMake and autotools all write the build-time
    # install prefix into what they install, and in CI that is
    # /Users/runner/work/ros2_macOS/ros2_macOS/install. Measured on the jazzy tree of run
    # 42025234330: 3855 text files (ament local_setup.sh, .cmake, .pc, .dsv hooks) and 39
    # mach-o files carry it -- among them gz_ros2_control's GZ_SIM_SYSTEM_PLUGIN_PATH,
    # python_orocos_kdl_vendor's PYTHONPATH (PyKDL) and lely, whose liblely-util loads
    # liblely-libc by that absolute path. None of it is visible to a CI test, because on
    # the runner the path exists; on a user's machine it does not.
    # The install prefix is always the SHORTER string (/opt/homebrew/opt/ros2-<distro>,
    # 28-29 chars, vs 45), so install_name_tool never needs header room it lacks.
    olds = [b for b in build_prefixes(tree, a.build_prefix)
            if os.path.normpath(b) != os.path.normpath(a.install_prefix)]
    vendor_root = os.path.join(tree, a.vendor_subdir.split("/")[0])
    text_fixed = 0
    for root, dirs, names in os.walk(tree):
        if os.path.abspath(root).startswith(os.path.abspath(vendor_root)):
            dirs[:] = []
            continue
        for n in names:
            f = os.path.join(root, n)
            if os.path.islink(f) or n.endswith((".pyc", ".pyo")):
                continue
            try:
                raw = open(f, "rb").read()
            except OSError:
                continue
            # by MAGIC, not is_macho(): that one calls every extensionless executable a
            # binary, which would skip bin/ entry-point scripts. A NUL byte also rules out
            # every mach-o and every other binary format -- never length-change those.
            if raw[:4] in MACHO_MAGIC or b"\0" in raw or not any(o.encode() in raw for o in olds):
                continue
            try:
                txt = raw.decode("utf-8")
            except UnicodeDecodeError:
                continue            # not text: never length-change binary data
            for o in olds:
                txt = txt.replace(o, a.install_prefix)
            st = os.stat(f)
            io.open(f, "w", encoding="utf-8", newline="").write(txt)
            os.chmod(f, st.st_mode)
            text_fixed += 1
    macho_fixed, macho_failed = 0, []
    for p in files:
        cmd = ["install_name_tool"]
        have_rp = {q for c, q in load_paths(p) if c == "LC_RPATH"}
        for c, q in load_paths(p):
            o = next((o for o in olds if q == o or q.startswith(o + "/")), None)
            if not o:
                continue
            new = a.install_prefix + q[len(o):]
            if c == "LC_ID_DYLIB":
                cmd += ["-id", new]
            elif c == "LC_RPATH":
                cmd += (["-delete_rpath", q] if new in have_rp else ["-rpath", q, new])
                have_rp.add(new)
            else:
                cmd += ["-change", q, new]
        if len(cmd) == 1:
            continue
        if run(cmd + [p]).returncode == 0:
            macho_fixed += 1
        else:
            macho_failed.append(p)
    print(f"  relocated            : {' | '.join(olds) or '(none)'} -> {a.install_prefix}")
    print(f"                         {text_fixed} text files, {macho_fixed} mach-o files",
          flush=True)
    envs = write_install_prefix_env(tree)
    if envs:
        print(f"  gz install prefixes  : {len(envs)} via share/{ENV_PKG}: "
              + ", ".join(f"{k}={v or '.'}" for k, v in sorted(envs.items())))
    if macho_failed:
        print(f"::error::{len(macho_failed)} mach-o file(s) rejected the build->install "
              f"relocation; they would load from the build machine's path:")
        for p in macho_failed[:10]:
            print(f"     {p[len(tree)+1:]}")
        sys.exit(1)

    # ---- 2. rewrite every reference, in the tree AND in the vendored copies
    targets = files + vendored
    allrefs = refs_of(targets)
    changed = 0
    unwritable = []
    for p, s in allrefs.items():
        if not s:
            continue
        cmd = ["install_name_tool"]
        for r in sorted(s):
            if not os.path.exists(r):
                # never rewrite a reference we could not vendor -- that converts a
                # resolvable absolute path into a dangling @rpath
                continue
            cmd += ["-change", r, f"@rpath/{vendor_rel(r)}"]
        if len(cmd) == 1:
            continue
        cmd.append(p)
        if run(cmd).returncode == 0:
            changed += 1
        else:
            unwritable.append(p)
    print(f"  references rewritten : {changed} files", flush=True)
    if unwritable:
        # Almost always header padding: install_name_tool cannot grow the load commands
        # of a binary linked without -headerpad_max_install_names. Scoping does not cause
        # it -- @rpath/<keg>/<name> is 15 characters SHORTER than the
        # /opt/homebrew/opt/<keg>/lib/<name> it replaces, every time -- but say which
        # files failed, because step 6 would otherwise report them only as "still
        # references /opt/homebrew" with no cause.
        print(f"::warning::{len(unwritable)} file(s) rejected an install_name_tool change "
              f"(load commands cannot grow -- built without -headerpad_max_install_names?)")
        for p in unwritable[:10]:
            print(f"     {p[len(tree)+1:]}")

    # ---- 2b. sibling refs that resolve INTO a scope dir.
    # A brew dylib reaches its keg-mates by @rpath/<basename> against its own LC_RPATH
    # (@loader_path/../lib), not by absolute path -- 187 of 406 vendored dylibs did when
    # it was measured. Flattening kept that working by accident: the vendor dir is itself
    # named lib, so @loader_path/../lib from inside it IS the vendor dir. A scoped file is
    # no longer at the name those deps spell, so rewrite the deps too -- otherwise the
    # only broken references in the shipped tree would be the ones scoping created.
    # Resolve from the ORIGINAL keg location, since inside the vendor dir the sibling is
    # no longer where the dep points, which is the whole problem.
    scoped_units = {u for u, rel in dest_rel.items() if "/" in rel}
    fixed = 0
    if scoped_units:
        for p in targets:
            cmd = ["install_name_tool"]
            for text, real in rpath_deps(origin_of.get(p, p)).items():
                if vendor_unit(real) in scoped_units:
                    cmd += ["-change", text, f"@rpath/{vendor_rel(real)}"]
            if len(cmd) == 1:
                continue
            cmd.append(p)
            if run(cmd).returncode == 0:
                fixed += 1
        print(f"  scoped sibling refs  : {fixed} files", flush=True)

    # One mach-o naming two kegs for ONE library was already broken before packaging --
    # two Qt majors cannot coexist in a process. Scoping leaves each reference pointing
    # where it pointed, which is the honest outcome, but say so.
    mixed = []
    for p, s in allrefs.items():
        per = collections.defaultdict(set)
        for r in s:
            rel = vendor_rel(r)
            if "/" in rel:
                per[vendor_key(r)].add(rel.split("/")[0])
        mixed += [(p, k, sorted(v)) for k, v in per.items() if len(v) > 1]
    if mixed:
        print(f"::warning::{len(mixed)} file(s) name two different kegs for one library. "
              f"Each reference keeps the keg it had -- this is how the tree was BUILT, not "
              f"something packaging introduced, and it cannot load both at once.")
        for p, k, scopes in mixed[:10]:
            print(f"     {p[len(tree)+1:]}  {k} <- {', '.join(scopes)}")

    # ---- 3. absolute rpath to the vendor dir, for everything
    added = 0
    for p in targets:
        if has_rpath(p, rpath_installed):
            continue
        if run(["install_name_tool", "-add_rpath", rpath_installed, p]).returncode == 0:
            added += 1
    print(f"  vendor rpath added   : {added} files", flush=True)

    # ---- 4. absolute rpaths for every in-tree lib dir a file actually needs.
    inner = add_intree_rpaths(tree, a.install_prefix, targets)
    print(f"  in-tree lib rpaths   : {inner} additions", flush=True)

    # ---- 5. re-sign everything we touched
    for p in targets:
        resign(p)
    print(f"  re-signed            : {len(targets)} files", flush=True)

    # ---- 6. verify
    # Step 2 deliberately does not rewrite a reference it could not vendor, because that
    # would turn a resolvable absolute path into a dangling @rpath. Those survivors are
    # exactly `tolerated`, so they are not verify failures -- anything else is.
    left = {p: (s - tolerated) for p, s in refs_of(targets).items()}
    left = {p: s for p, s in left.items() if s}
    if tolerated:
        print(f"  left in place        : refs to {len(tolerated)} un-vendorable dylib(s), "
              f"un-rewritten on purpose")
    if left:
        print(f"::error::{len(left)} files still reference Homebrew (opt/ or Cellar/) "
              f"after packaging")
        for p, s in list(left.items())[:10]:
            print(f"     {p[len(tree)+1:]} -> {sorted(s)[:2]}")
        sys.exit(1)
    print(f"  VERIFIED             : 0 remaining Homebrew references (opt/ and Cellar/)")

    # Anything still naming the build machine. A mach-o load command doing so is fatal --
    # that is exactly lely's failure. Text that names OTHER workspace dirs (build/, a
    # vendored source tree's include/) is reported, not rewritten: there is no installed
    # equivalent to map it to, and it only matters when compiling against the keg.
    ws = sorted({os.path.dirname(o) for o in olds}, key=len, reverse=True)
    bad = [(p, q) for p in targets for c, q in load_paths(p)
           if any(q.startswith(o + "/") or q == o for o in olds)]
    if bad:
        print(f"::error::{len(bad)} load command(s) still name the build prefix:")
        for p, q in bad[:10]:
            print(f"     {p[len(tree)+1:]} -> {q}")
        sys.exit(1)
    if ws:
        left = collections.Counter()
        for root, dirs, names in os.walk(tree):
            if os.path.abspath(root).startswith(os.path.abspath(vendor_root)):
                dirs[:] = []
                continue
            for n in names:
                f = os.path.join(root, n)
                if os.path.islink(f):
                    continue
                try:
                    raw = open(f, "rb").read()
                except OSError:
                    continue
                for w in ws:
                    for m in re.finditer(re.escape(w.encode()) + rb"/([A-Za-z0-9_.@+-]+)", raw):
                        left[m.group(1).decode("utf-8", "replace")] += 1
        if left:
            print(f"  build-machine paths left (outside the install prefix, not rewritten): "
                  + ", ".join(f"{k}/ x{v}" for k, v in left.most_common(8)))


if __name__ == "__main__":
    main()
