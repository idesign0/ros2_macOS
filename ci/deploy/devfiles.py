#!/usr/bin/env python3
"""
Make a packaged ROS 2 tree usable as an UNDERLAY: an overlay workspace must be able to
find_package() any package in it and compile and link against the same headers and
libraries the tree runs with.

package_tree.py makes the tree RUN without the user's Homebrew by vendoring the runtime
closure (dylibs and frameworks). It ships none of their development files, and the tree's
own exported CMake/pkg-config files still name the machine that built it. Measured on
kilted 26.11.12.16 (find_package() of each of 1484 packages from a clean overlay):
301 failed outright (Boost 129, Qt5 ~110, PCL 34, ...), and most of the rest only
configured by picking up whatever Homebrew had installed at the time -- opencv 5 headers
against the tree's opencv@4 runtime -- which an overlay then mixes in one process.

Runs on the machine that built or packaged the tree (it reads Homebrew and the workspace
checkout), in three steps around a SECOND package_tree.py pass:

  stage  -- work out which external prefixes the tree's build interface reaches (Homebrew
            formulae and workspace prefixes such as ros-commondep/boost-1.89) and mirror
            each one's development layout into <tree>/vendor/opt/<name>/: include/, the
            CMake and pkg-config files, static archives, the tools those CMake files
            import (Qt's moc/uic/rcc), and every dylib/framework/plugin the runtime pass
            did NOT vendor (a CMake targets file refuses to load when any file it lists
            is missing).
  (package_tree.py again: vendors the runtime closure of the binaries just staged and
            rewrites their Homebrew references, exactly as for the rest of the tree.)
  link   -- point the mirrored dylibs/frameworks at the vendored runtime copies, rewrite
            every absolute path in the tree's CMake/pkg-config files to the install
            prefix, drop the build machine's SDK paths, and prepend vendor/opt/* to
            CMAKE_PREFIX_PATH / PKG_CONFIG_PATH through a colcon package so an overlay
            resolves the tree's versions before the user's Homebrew.

Usage (the workflows call it right after package_tree.py, with the same flags):
  devfiles.py install --install-prefix /opt/homebrew/opt/ros2-kilted [--tolerate-stale-input]
  devfiles.py install --install-prefix ... --report-only     # audit, change nothing
"""
import argparse, collections, glob, io, json, os, re, shutil, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
HBROOT = "/opt/homebrew"
HB = HBROOT + "/opt"
CELLAR = HBROOT + "/Cellar"
LINKED = ("include", "lib", "share", "bin", "Frameworks", "etc")
# Build machines: GitHub's macOS runners. Their workspace is the checkout the tree was
# built from; their Xcode lives under a versioned name no user has.
RUNNER_WS = "/Users/runner/work/ros2_macOS/ros2_macOS"
SDK_RE = re.compile(r"/(?:Applications/Xcode[^/]*\.app/Contents/Developer|"
                    r"Library/Developer/CommandLineTools)/[^;\"'\s)]*")
TEXT_SUFFIXES = (".cmake", ".pc", ".prl", ".pri")
DEV_PKG = "ros2_macos_devfiles"
MACHO_MAGIC = {bytes.fromhex(h) for h in
               ("feedface", "cefaedfe", "feedfacf", "cffaedfe", "cafebabe", "bebafeca")}
# A path token as it appears in CMake lists, pkg-config flags and plain assignments.
ABS = re.compile(r"(?:(?<=-[LIF])|(?<![\w.$}/@-]))(/(?:opt/homebrew|Users/runner)/[^;\"'\s)<>,]*)")
# the bare Homebrew root as a search hint (PCLConfig: set(FLANN_ROOT "/opt/homebrew"))
BARE = re.compile(r"(?<=[\"\s;=])/opt/homebrew(?=[\"\s;)])")


def is_macho(p):
    try:
        with open(p, "rb") as fh:
            return fh.read(4) in MACHO_MAGIC
    except OSError:
        return False


def text_files(root, skip=()):
    for r, dirs, names in os.walk(root):
        if any(r == s or r.startswith(s + "/") for s in skip):
            dirs[:] = []
            continue
        for n in names:
            if n.endswith(TEXT_SUFFIXES):
                p = os.path.join(r, n)
                if not os.path.islink(p):
                    yield p


def read(p):
    try:
        return io.open(p, encoding="utf-8", errors="surrogateescape").read()
    except OSError:
        return ""


# ------------------------------------------------------------------ external prefixes
class Prefix:
    """One external prefix the tree's build interface reaches: a Homebrew formula
    (src = its Cellar keg) or a workspace prefix (src = its checkout dir)."""
    def __init__(self, name, src, kind):
        self.name, self.src, self.kind = name, os.path.realpath(src), kind

    @property
    def dest_rel(self):
        return f"vendor/opt/{self.name}"


DRIFT = {}


def formula_of(path):
    """(formula, keg root) for a Homebrew path in any of its spellings, else None."""
    m = re.match(rf"^{re.escape(HB)}/([^/]+)(/.*)?$", path)
    if m:
        root = os.path.realpath(f"{HB}/{m.group(1)}")
        m2 = re.match(rf"^{re.escape(CELLAR)}/([^/]+)/[^/]+$", root)
        return (m2.group(1), root) if m2 else None
    m = re.match(rf"^{re.escape(CELLAR)}/([^/]+)/([^/]+)", path)
    if m:
        root = f"{CELLAR}/{m.group(1)}/{m.group(2)}"
        if not os.path.isdir(root):
            # the tree names a version this machine no longer has (Homebrew moved after
            # the build): mirror the installed one, as package_tree tolerates drift
            alt = os.path.realpath(f"{HB}/{m.group(1)}")
            if os.path.isdir(alt):
                DRIFT[m.group(1)] = (m.group(2), os.path.basename(alt))
                return m.group(1), alt
        return m.group(1), root
    m = re.match(rf"^{re.escape(HBROOT)}/(?:{'|'.join(LINKED)})/", path)
    if m:
        # linked path: whichever keg its realpath lands in (walk up until something
        # exists, so a dangling leaf still names its keg)
        q = path
        while q and q != HBROOT and not os.path.lexists(q):
            q = os.path.dirname(q)
        rp = os.path.realpath(q)
        m2 = re.match(rf"^{re.escape(CELLAR)}/([^/]+)/([^/]+)", rp)
        if m2:
            return m2.group(1), f"{CELLAR}/{m2.group(1)}/{m2.group(2)}"
    return None


def on_fs(path, ws):
    """Where a build-machine path is on THIS machine (the same path in CI; a local
    checkout when testing elsewhere)."""
    return ws + path[len(RUNNER_WS):] if path.startswith(RUNNER_WS + "/") else path


def ws_prefix_of(path, ws):
    """A workspace path that sits inside a self-contained PREFIX (include/ + lib/, like
    ros-commondep/boost-1.89) -> that prefix. Source trees of ROS packages are not
    prefixes; they are mapped onto the installed tree instead (see source_map)."""
    path = on_fs(path, ws)
    if not path.startswith(ws + "/"):
        return None
    q = path
    while q.startswith(ws + "/"):
        if (os.path.isdir(os.path.join(q, "include")) and os.path.isdir(os.path.join(q, "lib"))
                and os.path.exists(os.path.join(q, "COLCON_IGNORE"))):
            return q
        q = os.path.dirname(q)
    return None


def config_names(root, depth=3):
    """CMake package names whose config file lives under root (lib/cmake, share/...)."""
    out = set()
    for sub in ("lib/cmake", "share/cmake", "share", "lib"):
        d = os.path.join(root, sub)
        if not os.path.isdir(d):
            continue
        for r, dirs, names in os.walk(d):
            if r.count("/") - d.count("/") >= depth:
                dirs[:] = []
            for n in names:
                m = re.match(r"^(.+?)(?:Config|-config)\.cmake$", n)
                if m:
                    out.add(m.group(1).lower())
    return out


def cmake_names_index():
    """CMake package name -> every formula providing a config for it. Several can
    (OpenCV: opencv and opencv@4; Eigen3: eigen and eigen@3; Boost), which is exactly
    where a name alone must not decide."""
    idx = collections.defaultdict(list)
    for f in sorted(os.listdir(HB)) if os.path.isdir(HB) else []:
        root = os.path.realpath(os.path.join(HB, f))
        m = re.match(rf"^{re.escape(CELLAR)}/([^/]+)/[^/]+$", root)
        if not m or m.group(1) != f:       # aliases (openssl -> openssl@4): canonical only
            continue
        for n in config_names(root):
            idx[n].append(f)
    return idx


def pc_index():
    """pkg-config module name -> formula."""
    idx = {}
    for f in sorted(os.listdir(HB)) if os.path.isdir(HB) else []:
        root = os.path.realpath(os.path.join(HB, f))
        m = re.match(rf"^{re.escape(CELLAR)}/([^/]+)/[^/]+$", root)
        if not m or m.group(1) != f:
            continue
        for sub in ("lib/pkgconfig", "share/pkgconfig"):
            d = os.path.join(root, sub)
            if os.path.isdir(d):
                for n in os.listdir(d):
                    if n.endswith(".pc"):
                        idx.setdefault(n[:-3], f)
    return idx


PCIDX = {}


def keg_dylibs(root):
    names = set()
    for sub in ("lib", "Frameworks"):
        d = os.path.join(root, sub)
        if not os.path.isdir(d):
            continue
        for n in os.listdir(d):
            if n.endswith((".dylib", ".framework")):
                names.add(n)
    return names


_RT = {}
_VLOC = {}


def vendored_at(tree, name, scope):
    """Where the runtime pass put a library or framework called `name`: vendor/lib/<name>,
    or vendor/lib/<keg>/<name> when two kegs claimed it (prefer this formula's scope)."""
    if tree not in _VLOC:
        loc = collections.defaultdict(list)
        v = os.path.join(tree, "vendor", "lib")
        for r, dirs, names in os.walk(v):
            for d in list(dirs):
                if d.endswith(".framework"):
                    loc[d].append(os.path.join(r, d))
                    dirs.remove(d)
            for n in names:
                loc[n].append(os.path.join(r, n))
        _VLOC[tree] = loc
    cands = _VLOC[tree].get(name, [])
    for c in cands:
        if os.path.basename(os.path.dirname(c)) == scope:
            return c
    flat = [c for c in cands if os.path.dirname(c) == os.path.join(tree, "vendor", "lib")]
    return (flat or cands or [None])[0]



def runtime_names(tree):
    """Basenames the runtime pass vendored (dylibs and framework bundles)."""
    if tree in _RT:
        return _RT[tree]
    out = _RT[tree] = set()
    v = os.path.join(tree, "vendor", "lib")
    for r, dirs, names in os.walk(v):
        for d in list(dirs):
            if d.endswith(".framework"):
                out.add(d)
                dirs.remove(d)
        out.update(n for n in names if n.endswith((".dylib", ".so")))
    return out


FINDDEP = re.compile(r"\b(?:find_dependency|find_package)\s*\(\s*([A-Za-z0-9_.+-]+)", re.I)
EXPORTED_DEPS = re.compile(r'set\(_exported_dependencies\s+"([^"]*)"\)')
INCLUDE = re.compile(rb'^\s*#\s*include\s*[<"]([^>"]+)[>"]', re.M)


def tree_refs(tree, skip):
    """Every absolute path and every dependency NAME in the tree's own build interface."""
    paths, names = collections.Counter(), collections.Counter()
    for p in text_files(tree, skip):
        t = read(p)
        for m in ABS.finditer(t):
            paths[m.group(1).rstrip("/.")] += 1
        if p.endswith(".cmake") and not os.path.basename(p).startswith("Find"):
            for m in FINDDEP.finditer(t):
                names[m.group(1)] += 1
            for m in EXPORTED_DEPS.finditer(t):
                for d in m.group(1).split(";"):
                    if d:
                        names[d] += 1
    return paths, names


def header_includes(dirs, limit_bytes=1 << 20):
    incs = set()
    for d in dirs:
        for r, _, names in os.walk(d):
            for n in names:
                if not n.endswith((".h", ".hpp", ".hh", ".hxx", ".inl", ".ipp", ".tcc")):
                    continue
                try:
                    with open(os.path.join(r, n), "rb") as fh:
                        data = fh.read(limit_bytes)
                except OSError:
                    continue
                for m in INCLUDE.finditer(data):
                    incs.add(m.group(1).decode("utf-8", "replace"))
    return incs


def source_map(paths, ws, tree):
    """Workspace SOURCE/BUILD dirs named by the tree's build interface (a package's source
    include/, gtsam's source dir, a vendor package's build-time install dir) -> the
    installed dir with the same contents. Matched by file suffix: list up to 200 files
    under the source dir and find the tree dir whose files carry the same relative paths.
    Needs the checkout, which exists where packaging runs."""
    out, unmatched = {}, []
    tree_index = None
    for orig in sorted(paths):
        p = on_fs(orig, ws)
        if not p.startswith(ws + "/") or not os.path.isdir(p):
            unmatched.append(orig)
            continue
        # a ROS package's own source root (mrt_cmake_modules' MRT_CMAKE_MODULES_ROOT_PATH)
        px = os.path.join(p, "package.xml")
        if os.path.isfile(px):
            m = re.search(r"<name>\s*([^<\s]+)\s*</name>", read(px))
            if m and os.path.isdir(os.path.join(tree, "share", m.group(1))):
                out[orig] = os.path.join(tree, "share", m.group(1))
                continue
        rels = []
        for r, dirs, names in os.walk(p):
            dirs[:] = [d for d in dirs if not d.startswith((".", "test", "doc", "build"))]
            for n in names:
                if not n.endswith((".h", ".hpp", ".hh", ".hxx", ".inl", ".cmake")):
                    continue
                rels.append(os.path.relpath(os.path.join(r, n), p))
                if len(rels) >= 200:
                    break
            if len(rels) >= 200:
                break
        hdr = [r for r in rels if not r.endswith(".cmake")]
        rels = hdr or rels          # an include dir is what such a path is used as
        if not rels:
            unmatched.append(orig)
            continue
        if tree_index is None:
            tree_index = collections.defaultdict(list)
            for r, dirs, names in os.walk(tree):
                if "/vendor" in r[len(tree):][:7]:
                    dirs[:] = []
                    continue
                for n in names:
                    tree_index[n].append(os.path.join(r, n))
        votes = collections.Counter()
        for rel in rels:
            for cand in tree_index.get(os.path.basename(rel), ()):
                if cand.endswith("/" + rel):
                    votes[cand[: -len(rel) - 1]] += 1
        if votes:
            best, n = votes.most_common(1)[0]
            if n >= max(1, len(rels) // 2):
                out[orig] = best
                continue
        unmatched.append(orig)
    return out, unmatched


# ------------------------------------------------------------------ stage
DEV_DIRS = ("include", "lib", "share", "mkspecs", "plugins", "Frameworks", "libexec")


SMALL_KEG = 25 << 20
# a library the prefix's own config files name (protobuf's libutf8_range) ships even
# when the tree does not load it, as long as it is small
NAMED_DYLIB_MAX = 4 << 20
NAMED_DYLIB_COUNT = 8
# a bin/ this small ships whole when the prefix's targets import its tools (GeographicLib)
TINY_BIN = 5 << 20
RUNTIME_DEV_MAX = 40 << 20
TREE_FILES = set()      # file names the tree's own build interface references


def named_in_configs(src):
    """Every file name the prefix's own CMake/pkg-config files mention."""
    out = set()
    for p in text_files(src):
        for m in re.finditer(r"([A-Za-z0-9_.+-]+\.(?:dylib|a|framework|so))", read(p)):
            out.add(m.group(1))
        for m in re.finditer(r"\b(Qt[A-Za-z0-9]+)\b", read(p)):
            out.add(m.group(1) + ".framework")
    return out


def mirror(prefix, tree, tools, whole_ok=True):
    """Copy prefix's development layout to tree/vendor/opt/<name>. Mach-o files are
    copied as real files here; link() later swaps any the runtime pass vendored for a
    symlink to that copy."""
    dst_root = os.path.join(tree, prefix.dest_rel)
    n = 0
    named = named_in_configs(prefix.src) | TREE_FILES

    alias_cache = {}

    def aliases(sp):
        """Every name in sp's directory that resolves to the same file: package_tree
        vendors a library under the name it was REFERENCED by (libtinyxml2.11.dylib),
        not its real name (libtinyxml2.11.0.0.dylib)."""
        d = os.path.dirname(sp)
        if d not in alias_cache:
            m = collections.defaultdict(set)
            for x in os.listdir(d):
                m[os.path.realpath(os.path.join(d, x))].add(x)
            alias_cache[d] = m
        return alias_cache[d].get(os.path.realpath(sp), {os.path.basename(sp)})
    vendored = runtime_names(tree)
    # A keg whose unloaded libraries are small ships whole: its CMake/pkg-config files then
    # resolve inside the tree instead of falling through to the user's Homebrew (flann,
    # lz4, libsodium...). Big ones (vtk, Qt, opencv) ship what the tree runs with.
    extra = 0
    ld = os.path.join(prefix.src, "lib")
    if os.path.isdir(ld):
        for x in os.listdir(ld):
            q = os.path.join(ld, x)
            if x.endswith(".dylib") and not os.path.islink(q) and x not in vendored:
                extra += os.path.getsize(q)
    small = whole_ok and extra < SMALL_KEG
    # the few unloaded libraries a prefix's link interface names (protobuf -> utf8_range);
    # a prefix naming dozens (vtk, opencv modules) only DEFINES them, nobody links them all
    few_named = set()
    if os.path.isdir(ld):
        cand = {x for x in os.listdir(ld) if x.endswith(".dylib") and x in named
                and x not in vendored and os.path.getsize(os.path.join(ld, x)) < NAMED_DYLIB_MAX}
        if len(cand) <= NAMED_DYLIB_COUNT:
            few_named = cand

    def wanted(sp, nm):
        """Binaries are mirrored only when something will look for them: the runtime
        loads them (link() turns those into symlinks) or the prefix's own config files
        name them (a CMake targets file refuses to load when a listed file is missing).
        Without this, llvm alone would bring gigabytes of archives nobody links."""
        if ".framework/" in sp:
            return True
        if nm.endswith(".a"):
            stem = nm[:-2]
            return nm in named or not os.path.exists(os.path.join(os.path.dirname(sp),
                                                                  stem + ".dylib"))
        if nm.endswith((".dylib", ".so")) or is_macho(sp):
            top_lib = os.path.dirname(sp) == os.path.join(prefix.src, "lib")
            if prefix.kind == "ws" or (small and top_lib) or (
                    top_lib and aliases(sp) & few_named):
                # our own prefix / a small keg's lib/ (not variant subdirs such as
                # bullet's lib/bullet/double, whose names collide with lib/): pass 2 relinks
                return True
            return bool(aliases(sp) & vendored)
        return True
    for top in DEV_DIRS:
        s = os.path.join(prefix.src, top)
        if not os.path.isdir(s):
            continue
        for r, dirs, names in os.walk(s):
            rel_r = os.path.relpath(r, prefix.src)
            # share/: only build-interface pieces (cmake, pkgconfig, aclocal, eigen's
            # cmake dir...) -- not docs, man pages or data
            if top == "share" and rel_r.count("/") >= 2 and "cmake" not in rel_r.lower() \
                    and "pkgconfig" not in rel_r:
                if not any(x.lower().endswith(("cmake", "config.cmake")) for x in names):
                    dirs[:] = [d for d in dirs if "cmake" in d.lower()]
            for d in list(dirs):
                sp = os.path.join(r, d)
                if d.endswith(".framework"):
                    dirs.remove(d)
                    at = vendored_at(tree, d, prefix.name) if d in vendored else None
                    dp = os.path.join(dst_root, rel_r, d)
                    if at and not os.path.lexists(dp):
                        os.makedirs(os.path.dirname(dp), exist_ok=True)
                        os.symlink(os.path.relpath(at, os.path.dirname(dp)), dp)
                        n += 1
                    continue
                if os.path.islink(sp):
                    dp = os.path.join(dst_root, rel_r, d)
                    os.makedirs(os.path.dirname(dp), exist_ok=True)
                    if not os.path.lexists(dp):
                        os.symlink(os.readlink(sp), dp)
                    dirs.remove(d)
            for nm in names:
                sp = os.path.join(r, nm)
                rel = os.path.join(rel_r, nm)
                if top == "share" and not (nm.endswith((".cmake", ".pc", ".m4"))
                                           or "cmake" in rel_r.lower()):
                    continue
                if top == "lib" and nm.endswith((".la",)):
                    continue
                if not wanted(sp, nm):
                    continue
                dp = os.path.join(dst_root, rel)
                if os.path.lexists(dp):
                    continue
                os.makedirs(os.path.dirname(dp), exist_ok=True)
                if os.path.islink(sp):
                    os.symlink(os.readlink(sp), dp)
                elif nm.endswith((".dylib", ".so")) or is_macho(sp):
                    # the runtime copy, not a second one: overlay and tree must share it
                    at = next((x for x in (vendored_at(tree, al, prefix.name)
                                           for al in sorted(aliases(sp))) if x), None)
                    if at:
                        os.symlink(os.path.relpath(at, os.path.dirname(dp)), dp)
                    else:
                        shutil.copy2(sp, dp)
                else:
                    shutil.copy2(sp, dp)
                n += 1
    for top in ("bin", "include", "lib", "share"):
        if os.path.isdir(os.path.join(prefix.src, top)):
            os.makedirs(os.path.join(dst_root, top), exist_ok=True)
    # tools the prefix's CMake files import (Qt: moc, uic, rcc, qmake...) -- only those
    # that run on the shipped runtime alone: qhelpgenerator needs QtHelp, which the tree
    # does not ship, and would drag a whole Qt module set in with it
    extra_tools = set()
    bd = os.path.join(prefix.src, "bin")
    tiny_bin = os.path.isdir(bd) and sum(
        os.path.getsize(os.path.join(bd, x)) for x in os.listdir(bd)
        if os.path.isfile(os.path.join(bd, x))) < TINY_BIN
    if small or tiny_bin:
        # plain command-line tools only (GeographicLib's CartConvert...): a Qt tool brings
        # the Qt6 plugin-path patching along, which is not built for it
        extra_tools = imported_tools(prefix.src, targets=True) - set(tools)
        tools = set(tools) | extra_tools
    # Qt6::Platform puts share/qt/mkspecs/<spec> on the include path
    for spec in sorted({m.group(2) for p in text_files(prefix.src)
                        for m in re.finditer(r"(share/qt/mkspecs)/([\w+.-]+)", read(p))} | (
                           {"common"} if os.path.isdir(os.path.join(prefix.src, "share/qt/mkspecs/common")) else set())):
        sp = os.path.join(prefix.src, "share/qt/mkspecs", spec)
        dp = os.path.join(dst_root, "share/qt/mkspecs", spec)
        # skip when share/qt is itself a link into another keg (the qt meta-formula)
        if os.path.isdir(sp) and not os.path.lexists(dp) and \
                not os.path.islink(os.path.join(prefix.src, "share/qt")):
            shutil.copytree(sp, dp, symlinks=True)
            n += 1
    for t in sorted(tools):
        sp = os.path.join(prefix.src, "bin", t)
        dp = os.path.join(dst_root, "bin", t)
        if not os.path.isfile(sp) or os.path.lexists(dp):
            continue
        deps = subprocess.run(["otool", "-L", sp], capture_output=True, text=True).stdout
        need = set()
        for line in deps.splitlines()[1:]:
            ref = line.strip().split(" ")[0]
            if ref.startswith(HBROOT + "/"):
                m = re.search(r"/([^/]+\.framework)/", ref)
                need.add(m.group(1) if m else os.path.basename(os.path.realpath(ref)))
        # plain dylibs the second package_tree pass vendors like any other; a FRAMEWORK
        # not already shipped means a Qt module the tree does not run with (QtHelp...)
        if t in extra_tools and any(x.endswith(".framework") for x in need):
            continue
        if {x for x in need - vendored if x.endswith(".framework")}:
            SKIPPED_TOOLS.append(f"{prefix.name}/bin/{t} (needs {', '.join(sorted(need - vendored))})")
            continue
        if True:
            os.makedirs(os.path.dirname(dp), exist_ok=True)
            shutil.copy2(sp, dp)
            n += 1
    return n


SKIPPED_TOOLS = []


def imported_tools(src, targets=False):
    """bin/ executables a prefix's own CMake files reference."""
    tools = set()
    for p in text_files(src):
        # targets files import every tool a formula builds (vtkpython, h5dump...) and
        # their existence check is relaxed below; Config/ConfigExtras files hard-require
        # the few an overlay build actually runs (Qt: moc, rcc, uic, qmake)
        if not targets and "argets" in os.path.basename(p):
            continue
        for m in re.finditer(r"/bin/([A-Za-z0-9_.+-]+)", read(p)):
            if os.path.isfile(os.path.join(src, "bin", m.group(1))):
                tools.add(m.group(1))
    return tools


def plan(tree, ws, skip):
    PCIDX.update(pc_index())
    paths, names = tree_refs(tree, skip)
    TREE_FILES.update(os.path.basename(p) for p in paths)
    idx = cmake_names_index()
    have_in_tree = config_names(os.path.join(tree, "share"), 2) | config_names(tree, 3)
    vendored = runtime_names(tree)

    prefixes, why, skipped = {}, collections.defaultdict(set), collections.defaultdict(set)
    kinfo = {}

    def info(f):
        if f not in kinfo:
            root = os.path.realpath(os.path.join(HB, f))
            libs = keg_dylibs(root)
            # runtime evidence: the tree loads one of this keg's libraries
            real = {os.path.basename(os.path.realpath(os.path.join(root, "lib", n)))
                    for n in libs if not n.endswith(".framework")}
            kinfo[f] = (root, bool(libs), bool((libs | real) & vendored))
        return kinfo[f]

    def add(f, root, kind, reason):
        if f.startswith("ros2-"):
            return False
        if f not in prefixes:
            prefixes[f] = Prefix(f, root, kind)
        why[f].add(reason)
        return True

    def providers_of(name):
        n = name.lower()
        out = [x for x, p in prefixes.items() if p.kind == "ws" and n in p.names]
        return out + [f for f in idx.get(n, []) if f not in out]

    def want_name(name, reason):
        """Resolve a CMake package NAME to one provider, or report why not."""
        n = name.lower()
        if n in have_in_tree:
            return None
        cands = providers_of(name)
        if not cands:
            return None
        chosen = [c for c in cands if c in prefixes]
        if chosen:
            why[chosen[0]].add(reason)
            return None
        ok = []
        for c in cands:
            root, has_libs, used = info(c)
            if used or not has_libs:
                ok.append(c)
        if len(ok) >= 1:
            # prefer the one the runtime actually loads
            ok.sort(key=lambda c: (not info(c)[2], c))
            c = ok[0]
            if add(c, info(c)[0], "brew", reason):
                return prefixes[c]
        skipped[name].add(f"{reason}: {'/'.join(cands)} (none loaded by the tree)")
        return None

    # 1. absolute paths: definite
    for p in paths:
        fr = formula_of(p)
        if fr:
            add(fr[0], fr[1], "brew", "path")
            continue
        wp = ws_prefix_of(p, ws)
        if wp:
            nm = os.path.basename(wp)
            if add(nm, wp, "ws", "path"):
                prefixes[nm].names = config_names(wp)
    for pre in prefixes.values():
        if not hasattr(pre, "names"):
            pre.names = set()
    # 2. dependency names the tree's configs ask for
    for n in sorted(names):
        want_name(n, f"find:{n}")
    # 3. headers the tree installs include from a keg (header-only deps, keg-only ones)
    incs = header_includes([os.path.join(tree, "include")])
    search = [os.path.join(HBROOT, "include")] + [
        os.path.join(p.src, "include") for p in prefixes.values()]
    tree_inc = os.path.join(tree, "include")
    for inc in sorted(incs):
        if os.path.exists(os.path.join(tree_inc, inc)):
            continue
        for s_ in search:
            q = os.path.join(s_, inc)
            if os.path.exists(q):
                fr = formula_of(q) if q.startswith(HBROOT) else None
                if fr and fr[0] not in prefixes:
                    root, has_libs, used = info(fr[0])
                    if used or not has_libs:
                        add(fr[0], fr[1], "brew", "include")
                    else:
                        skipped[fr[0]].add(f"include <{inc}> (keg not loaded by the tree)")
                break
    # 3b. every formula whose libraries the tree RUNS with ships its headers and configs
    #     too: a Find module (FindFreeImage, FindFLANN...) locates them by default paths,
    #     and without a copy in the tree it lands on the user's Homebrew -- another version
    for f in sorted(os.listdir(HB)) if os.path.isdir(HB) else []:
        if f in prefixes or f.startswith("ros2-"):
            continue
        root = os.path.realpath(os.path.join(HB, f))
        m = re.match(rf"^{re.escape(CELLAR)}/([^/]+)/[^/]+$", root)
        if not m or m.group(1) != f:
            continue
        if not info(f)[2]:
            continue
        # not a second provider of a package already shipped (brew boost 1.92 and
        # boost@1.85 are loaded by brew pcl/ompl, but the tree builds on boost-1.89), and
        # not a toolchain-sized keg (llvm, gcc: loaded by mesa/gfortran users, never built
        # against by a ROS overlay)
        shipped = set().union(*(getattr(q, "names", set()) or config_names(q.src)
                                for q in prefixes.values()))
        clash = config_names(root) & shipped
        if clash:
            skipped[f].add(f"runtime: would duplicate {', '.join(sorted(clash)[:3])}")
            continue
        size = 0
        for sub in ("include", "lib/cmake", "lib/pkgconfig"):
            for r_, _, ns in os.walk(os.path.join(root, sub)):
                size += sum(os.path.getsize(os.path.join(r_, n)) for n in ns
                            if not os.path.islink(os.path.join(r_, n)))
        if size > RUNTIME_DEV_MAX:
            skipped[f].add(f"runtime: dev files {size >> 20} MB (> {RUNTIME_DEV_MAX >> 20})")
            continue
        add(f, root, "brew", "runtime")
    # 4. transitive: what the mirrored prefixes' own CMake files reach
    frontier = [p for p in prefixes.values()]
    for p in frontier:
        p.names = getattr(p, "names", set())
    while frontier:
        nxt = []
        for pre in frontier:
            for p in text_files(pre.src):
                t = read(p)
                for m in ABS.finditer(t):
                    fr = formula_of(m.group(1))
                    if fr and fr[0] not in prefixes:
                        clash = config_names(fr[1]) & set().union(
                            *(getattr(q, "names", set()) or config_names(q.src)
                              for q in prefixes.values()))
                        if clash:
                            skipped[fr[0]].add(f"via:{pre.name} would duplicate "
                                               f"{', '.join(sorted(clash)[:3])} already shipped")
                            continue
                        if add(fr[0], fr[1], "brew", f"via:{pre.name}"):
                            prefixes[fr[0]].names = set()
                            nxt.append(prefixes[fr[0]])
                if p.endswith(".pc"):
                    for m in re.finditer(r"^Requires(?:\.private)?:(.*)$", t, re.M):
                        for mod in re.findall(r"([A-Za-z0-9_.+-]+)(?:\s*[<>=]+\s*[0-9.]+)?",
                                              m.group(1)):
                            f = PCIDX.get(mod)
                            if f and f not in prefixes:
                                root, has_libs, used = info(f)
                                if add(f, root, "brew", f"via:{pre.name}"):
                                    prefixes[f].names = set()
                                    nxt.append(prefixes[f])
                for m in (FINDDEP.finditer(t) if not os.path.basename(p).startswith("Find")
                          else ()):
                    got = want_name(m.group(1), f"via:{pre.name}")
                    if got:
                        got.names = set()
                        nxt.append(got)
        frontier = nxt
    src, unmatched = source_map([p for p in paths if (p.startswith(RUNNER_WS + "/")
                                 or p.startswith(ws + "/")) and not ws_prefix_of(p, ws)
                                 and not re.search(r"/build/[^/]+/[^/]+_install(/|$)", p)],
                                ws, tree)
    return prefixes, why, src, unmatched, skipped


def stage(a, tree, skip):
    prefixes, why, src, unmatched, skipped = plan(tree, a.ws, skip)
    print(f"  dev prefixes         : {len(prefixes)}", flush=True)
    total = 0
    for name, pre in sorted(prefixes.items()):
        n = 0 if a.report_only else mirror(pre, tree, imported_tools(pre.src),
                                           whole_ok=bool(why[name] - {"runtime"}))
        total += n
        print(f"     {name:28s} {pre.kind:4s} {os.path.relpath(pre.src, '/'):60s} "
              f"{n:6d} files  ({', '.join(sorted(why[name]))[:70]})")
    if DRIFT:
        print(f"  version drift        : {len(DRIFT)} formulae differ from the build: "
              + ", ".join(f"{k} {a}->{b}" for k, (a, b) in sorted(DRIFT.items())))
    print(f"  source dirs mapped   : {len(src)}")
    for s, d in sorted(src.items()):
        print(f"     {s} -> {os.path.relpath(d, tree)}")
    if skipped:
        print(f"  NOT shipped          : {len(skipped)} dependency names/kegs the tree "
              f"asks for but does not run with (overlays get the user's own, if any)")
        for k, v in sorted(skipped.items())[:40]:
            print(f"     {k}: {'; '.join(sorted(v))[:150]}")
    if unmatched:
        print(f"  source dirs UNMATCHED: {len(unmatched)} (left as they are)")
        for s in unmatched[:20]:
            print(f"     {s}")
    state = {"prefixes": {n: {"src": p.src, "kind": p.kind, "dest": p.dest_rel}
                          for n, p in prefixes.items()},
             "source_map": {s: os.path.relpath(d, tree) for s, d in src.items()}}
    sp = os.path.join(tree, "vendor", ".devfiles.json")
    if os.path.exists(sp):
        # a re-run reads files the first link already rewrote, and would see less
        old = json.load(open(sp))
        for k, v in old.get("prefixes", {}).items():
            state["prefixes"].setdefault(k, v)
        for k, v in old.get("source_map", {}).items():
            state["source_map"].setdefault(k, v)
    if not a.report_only:
        os.makedirs(os.path.join(tree, "vendor"), exist_ok=True)
        json.dump(state, open(os.path.join(tree, "vendor", ".devfiles.json"), "w"), indent=1)
    if SKIPPED_TOOLS:
        print(f"  tools not shipped    : {len(SKIPPED_TOOLS)} (need libraries the tree does not run "
              f"with): " + "; ".join(SKIPPED_TOOLS[:8]))
    print(f"  staged               : {total} files", flush=True)
    return state


# ------------------------------------------------------------------ link
def link(a, tree):
    state = json.load(open(os.path.join(tree, "vendor", ".devfiles.json")))
    man = {}
    mp = os.path.join(tree, "vendor", ".manifest.json")
    if os.path.exists(mp):
        man = json.load(open(mp))           # realpath(source) -> path under vendor/lib
    vlib = os.path.join(tree, "vendor", "lib")
    P = a.install_prefix.rstrip("/")

    # 1. mirrored mach-o files the runtime pass vendored -> symlink to that copy, so the
    #    overlay links the very file the tree loads (one copy, one install name).
    swapped = 0
    fw_done = set()
    for name, info in state["prefixes"].items():
        droot = os.path.join(tree, info["dest"])
        for r, dirs, names in os.walk(droot):
            # whole framework bundles first
            for d in list(dirs):
                if not d.endswith(".framework"):
                    continue
                src_fw = os.path.realpath(os.path.join(info["src"], os.path.relpath(
                    os.path.join(r, d), droot)))
                hit = next((v for k, v in man.items()
                            if k.startswith(src_fw + "/")), None)
                if hit:
                    fw_rel = hit.split(".framework/")[0] + ".framework"
                    dp = os.path.join(r, d)
                    target = os.path.join(vlib, fw_rel)
                    if os.path.isdir(target) and not os.path.islink(dp):
                        shutil.rmtree(dp)
                        os.symlink(os.path.relpath(target, r), dp)
                        swapped += 1
                        fw_done.add(dp)
                    dirs.remove(d)
            for n in names:
                dp = os.path.join(r, n)
                if os.path.islink(dp) or not is_macho(dp):
                    continue
                sp = os.path.realpath(os.path.join(info["src"], os.path.relpath(dp, droot)))
                v = man.get(sp)
                if v and os.path.exists(os.path.join(vlib, v)):
                    os.remove(dp)
                    os.symlink(os.path.relpath(os.path.join(vlib, v), r), dp)
                    swapped += 1
    print(f"  linked to runtime    : {swapped} mirrored libs/frameworks now point at "
          f"vendor/lib copies", flush=True)

    # 0. merged vendor/include for the linked spelling (/opt/homebrew/include/...): a
    #    per-FILE merge, since kegs share directories there (GL: glew + mesa + freeglut)
    vinc = os.path.join(tree, "vendor", "include")
    os.makedirs(vinc, exist_ok=True)
    clashes = []

    def merge(src_dir, dst_dir, who):
        for e in sorted(os.listdir(src_dir)):
            sp, dp = os.path.join(src_dir, e), os.path.join(dst_dir, e)
            if not os.path.exists(sp):
                continue        # dangling (Qt's include/<Module> of a module not shipped)
            if os.path.isdir(sp):
                if os.path.islink(dp) and os.path.isdir(dp):   # one provider so far: split
                    first = os.path.realpath(dp)
                    os.remove(dp)
                    os.makedirs(dp)
                    merge(first, dp, "?")
                if os.path.isdir(dp):
                    merge(sp, dp, who)
                    continue
            if os.path.lexists(dp):
                if not os.path.isdir(dp):
                    clashes.append((os.path.relpath(dp, vinc), who))
                continue
            os.symlink(os.path.relpath(sp, dst_dir), dp)

    def linked_first(item):
        """/opt/homebrew/include/<x> means the LINKED keg's x (eigen, not keg-only
        eigen@3), so linked kegs claim the merged dir first."""
        name, info = item
        src_inc = os.path.join(info["src"], "include")
        linked = False
        if os.path.isdir(src_inc):
            for e in os.listdir(src_inc)[:5]:
                q = os.path.join(HBROOT, "include", e)
                if os.path.lexists(q) and os.path.realpath(q).startswith(info["src"] + "/"):
                    linked = True
                    break
        return (not linked, name)

    vlibd = os.path.join(tree, "vendor", "lib")
    libview = 0
    for name, info in sorted(state["prefixes"].items(), key=linked_first):
        inc = os.path.join(tree, info["dest"], "include")
        if os.path.isdir(inc):
            merge(inc, vinc, name)
        # vendor/lib doubles as the linked /opt/homebrew/lib: add what it lacks
        ld = os.path.join(tree, info["dest"], "lib")
        if os.path.isdir(ld):
            for e in os.listdir(ld):
                q = os.path.join(ld, e)
                if e.endswith((".dylib", ".a")) and os.path.exists(q) \
                        and not os.path.lexists(os.path.join(vlibd, e)):
                    os.symlink(os.path.relpath(q, vlibd), os.path.join(vlibd, e))
                    libview += 1
    print(f"  vendor/lib view      : {libview} dev libraries linked in", flush=True)
    if clashes:
        print(f"  vendor/include       : {len(clashes)} files provided twice, first kept: "
              + ", ".join(f"{e}<-{n}" for e, n in clashes[:6]))

    # 2. rewrite absolute paths in every build-interface text file of the tree
    rules = []          # (old prefix, new prefix), longest first
    for name, info in state["prefixes"].items():
        new = f"{P}/{info['dest']}"
        if info["kind"] == "brew":
            rules += [(info["src"], new), (f"{HB}/{name}", new)]
            # other installed versions of this formula map to the shipped one too
            cd = os.path.join(CELLAR, name)
            if os.path.isdir(cd):
                rules += [(os.path.join(cd, v), new) for v in os.listdir(cd)]
        else:
            rules += [(info["src"], new)]
            rel = os.path.relpath(info["src"], os.path.realpath(a.ws))
            rules += [(os.path.join(a.ws, rel), new), (os.path.join(RUNNER_WS, rel), new)]
    for s, d in state["source_map"].items():
        rules.append((s, f"{P}/{d}"))
    rules = sorted(set(rules), key=lambda kv: -len(kv[0]))

    def map_path(p):
        for old, new in rules:
            if p == old or p.startswith(old + "/"):
                return new + p[len(old):]
        fr = formula_of(p)
        if fr and fr[0] in state["prefixes"]:
            # linked spelling (/opt/homebrew/include/foo): via its keg
            q = p
            while q != HBROOT and not os.path.lexists(q):
                q = os.path.dirname(q)
            rp = os.path.realpath(q) + p[len(q):]
            for old, new in rules:
                if rp == old or rp.startswith(old + "/"):
                    return new + rp[len(old):]
        if p in (f"{HBROOT}/include", f"{HBROOT}/lib"):
            return f"{P}/vendor/{p.rsplit('/', 1)[1]}"
        # another version of a shipped formula (a .pc written against glib 2.88.1 while
        # 2.90.1 is what ships)
        m = re.match(rf"^{re.escape(CELLAR)}/([^/]+)/[^/]+(/.*)?$", p)
        if m and m.group(1) in state["prefixes"]:
            return f"{P}/{state['prefixes'][m.group(1)]['dest']}{m.group(2) or ''}"
        # a directory several kegs link into (/opt/homebrew/include/GL: glew, mesa,
        # freeglut): the merged vendor/include holds all of them
        m = re.match(rf"^{re.escape(HBROOT)}/(include|lib)(/.*)$", p)
        if m and os.path.lexists(os.path.join(tree, "vendor", m.group(1), m.group(2)[1:])):
            return f"{P}/vendor/{m.group(1)}{m.group(2)}"
        # an ament_vendor package's staging install (build/<pkg>/<x>_install), which the
        # package then installed into the tree itself
        m = re.match(rf"^(?:{re.escape(RUNNER_WS)}|{re.escape(a.ws)})/build/[^/]+/[^/]+_install(/.*)?$", p)
        if m and (not m.group(1) or os.path.lexists(tree + m.group(1))):
            return P + (m.group(1) or "")
        return None

    changed, unmapped = 0, collections.Counter()
    for f in text_files(tree, skip=(os.path.join(tree, "vendor", "lib"),)):
        t = read(f)
        if "/opt/homebrew" not in t and "/Users/runner" not in t and "/Applications/Xcode" \
                not in t and "/Library/Developer" not in t:
            continue
        orig = t

        def sub(m):
            p = m.group(1).rstrip("/")
            if p.startswith(P + "/") or p == P or p.startswith(os.path.realpath(P)):
                return m.group(0)
            q = map_path(p)
            if q is None and re.match(rf"^{re.escape(HBROOT)}/(etc|var)(/|$)", p):
                return m.group(0)          # runtime state (dbus socket, font cache)
            if q is None:
                unmapped[re.sub(r"^(/opt/homebrew/(?:opt|Cellar)/[^/]+|/Users/runner/work/"
                                r"ros2_macOS/ros2_macOS/[^/]+/[^/]+|/opt/homebrew/[^/]+).*",
                                r"\1", p)] += 1
                return m.group(0)
            return m.group(0).replace(p, q, 1)
        t = ABS.sub(sub, t)
        t = BARE.sub(f"{P}/vendor", t)
        # the build machine's SDK: drop it -- the compiler adds the user's own
        t = re.sub(r"(?:/Applications/Xcode[^/]*\.app/Contents/Developer|/Library/Developer/"
                   r"CommandLineTools)/[^;\"'\s)]*?/System/Library/Frameworks/([A-Za-z0-9_]+)"
                   r"\.framework", r"-framework \1", t)
        t = re.sub(r";" + SDK_RE.pattern, "", t)
        t = re.sub(SDK_RE.pattern + r";", "", t)
        t = re.sub(r"\s*-(?:I|isystem\s*|L|F)" + SDK_RE.pattern, "", t)
        if t != orig:
            st = os.stat(f)
            os.chmod(f, st.st_mode | 0o200)      # some kegs install their configs 0444
            io.open(f, "w", encoding="utf-8", errors="surrogateescape", newline="").write(t)
            os.chmod(f, st.st_mode)
            changed += 1
    print(f"  build files rewritten: {changed}", flush=True)

    # 2b. a mirrored formula ships the libraries the tree RUNS with, not every module it
    #     has. CMake's generated targets files refuse to load if ANY listed file is
    #     missing, so find_package(VTK) would fail over a module nobody here uses. Keep
    #     the check's message but not its FATAL: an overlay that links such a module
    #     still gets a precise error -- at link time, naming the file.
    soft = 0
    for f in text_files(os.path.join(tree, "vendor", "opt")):
        t = read(f)
        if "references the file" not in t:
            continue
        u = re.sub(r'message\(FATAL_ERROR ("The imported target \\"[^\n]*?references the file)',
                   r'message(VERBOSE \1', t)
        if u != t:
            os.chmod(f, os.stat(f).st_mode | 0o200)
            io.open(f, "w", encoding="utf-8", errors="surrogateescape", newline="").write(u)
            soft += 1
    print(f"  import checks relaxed: {soft} mirrored targets files", flush=True)

    # 2c. Qt5 loads EVERY Qt5<Module>_*Plugin.cmake next to its config and hard-fails on
    #     a plugin file that is not there; the tree ships only the plugins it runs with
    #     (vendor/lib/plugins), so drop the plugin configs it does not ship.
    pruned = 0
    for f in text_files(os.path.join(tree, "vendor", "opt")):
        b = os.path.basename(f)
        if not re.match(r"Qt5\w+_\w+\.cmake$", b):
            continue
        m = re.search(r'_populate_\w+_plugin_properties\(\w+ \w+ "([^"]+)"', read(f))
        if not m:
            continue
        qt_prefix = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(f))))
        if not os.path.exists(os.path.join(qt_prefix, "plugins", m.group(1))):
            os.remove(f)
            pruned += 1
    print(f"  qt plugin configs    : {pruned} dropped (plugin not shipped)", flush=True)

    # 2d. upstream exports that do not hold on macOS: an include dir the package never
    #     installs (CMake rejects an imported target naming a non-existent path), and a
    #     hardcoded lib<x>.so where the tree has lib<x>.dylib
    made, so = 0, 0
    libdirs = [os.path.join(tree, "lib")] + [os.path.join(tree, "opt", o, "lib") for o in (
        os.listdir(os.path.join(tree, "opt")) if os.path.isdir(os.path.join(tree, "opt")) else [])]
    for f in text_files(os.path.join(tree, "share")):
        t = read(f)
        for m in re.finditer(r'(?:\$\{_IMPORT_PREFIX\}|\$\{\w+_DIR\}/\.\./\.\./\.\.)/include/([\w.+-]+)(?=["; ])', t):
            d = os.path.join(tree, "include", m.group(1))
            if not os.path.exists(d):
                os.makedirs(d)
                made += 1
        def _dylib(m):
            has = lambda ext: any(os.path.exists(os.path.join(r, f"lib{m.group(2)}{ext}"))
                                  for r in libdirs)
            return m.group(1) + '.dylib"' if not has(".so") and has(".dylib") else m.group(0)
        new = re.sub(r'(/lib([\w.+-]+?))\.so"', _dylib, t)
        if new != t:
            os.chmod(f, os.stat(f).st_mode | 0o200)
            io.open(f, "w").write(new)
            so += 1
    print(f"  export fixes         : {made} empty include dirs, {so} files .so -> .dylib", flush=True)

    # 4. a colcon package that puts the shipped prefixes ahead of the user's Homebrew
    lines = []
    for name, info in sorted(state["prefixes"].items()):
        d = info["dest"]
        lines.append(f"prepend-non-duplicate;CMAKE_PREFIX_PATH;{d}")
        for pc in ("lib/pkgconfig", "share/pkgconfig"):
            if os.path.isdir(os.path.join(tree, d, pc)):
                lines.append(f"prepend-non-duplicate;PKG_CONFIG_PATH;{d}/{pc}")
    # an opt/<vendor> prefix whose config a package re-exports as a dependency
    # (multisensor_calibration -> small_gicp) but whose own hooks leave it off
    # CMAKE_PREFIX_PATH: without this the overlay's find_dependency() cannot see it
    exported = set()
    for f in glob.glob(os.path.join(tree, "share", "*", "cmake",
                                    "ament_cmake_export_dependencies-extras.cmake")):
        m = re.search(r'set\(_exported_dependencies "([^"]*)"\)', read(f))
        if m:
            exported |= {x for x in m.group(1).split(";") if x}
    hooked = set()
    for f in glob.glob(os.path.join(tree, "share", "*", "hook", "*.dsv")) + \
            glob.glob(os.path.join(tree, "share", "*", "environment", "*.dsv")):
        for m in re.finditer(r"CMAKE_PREFIX_PATH;(opt/[^/;\s]+)", read(f)):
            hooked.add(m.group(1))
    exported = {x.lower() for x in exported}
    elsewhere = config_names(tree)
    for o in sorted(os.listdir(os.path.join(tree, "opt")) if os.path.isdir(os.path.join(tree, "opt")) else []):
        rel = f"opt/{o}"
        if rel in hooked:
            continue
        names = config_names(os.path.join(tree, rel))
        if names & (exported - elsewhere):
            lines.append(f"prepend-non-duplicate;CMAKE_PREFIX_PATH;{rel}")
            print(f"  opt prefix exposed   : {rel} ({', '.join(sorted(names & exported))})")
    meta = os.path.join(tree, "share", "colcon-core", "packages")
    os.makedirs(meta, exist_ok=True)
    io.open(os.path.join(meta, DEV_PKG), "w").write("")
    dd = os.path.join(tree, "share", DEV_PKG)
    os.makedirs(dd, exist_ok=True)
    # the interpreter the tree's Python packages were built for (bin/ros2's shebang):
    # without a hint FindPython3 takes whichever python3 is first, often Homebrew's
    try:
        sheb = open(os.path.join(tree, "bin", "ros2")).readline()
        m = re.match(r"#!\s*(/Library/Frameworks/Python\.framework/Versions/[0-9.]+)/bin/python",
                     sheb)
        if m:
            lines.append(f"set-if-unset;Python3_ROOT_DIR;{m.group(1)}")
    except OSError:
        pass
    io.open(os.path.join(dd, "package.dsv"), "w").write("\n".join(lines) + "\n")
    idx = os.path.join(tree, "share", "ament_index", "resource_index", "packages")
    print(f"  env package          : share/{DEV_PKG} ({len(lines)} entries)")

    # 5. what is still absolute and outside the tree
    if unmapped:
        print(f"  NOT rewritten        : {sum(unmapped.values())} refs")
        for k, v in unmapped.most_common(25):
            print(f"     {v:5d}  {k}")
    return unmapped


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("tree")
    ap.add_argument("--install-prefix", required=True)
    ap.add_argument("--ws", default=os.environ.get("GITHUB_WORKSPACE", RUNNER_WS),
                    help="the checkout the tree was built from")
    ap.add_argument("--tolerate-stale-input", action="store_true")
    ap.add_argument("--report-only", action="store_true")
    ap.add_argument("--step", choices=("all", "stage", "link"), default="all")
    ap.add_argument("--strict", action="store_true",
                    help="fail when a build-interface path cannot be rewritten")
    a = ap.parse_args()
    a.ws = a.ws.rstrip("/")
    tree = os.path.abspath(a.tree.rstrip("/"))
    skip = (os.path.join(tree, "vendor"),)

    if a.step in ("all", "stage"):
        print("== devfiles: stage", flush=True)
        stage(a, tree, skip)
        if a.report_only:
            return
    if a.step == "all":
        print("== devfiles: package_tree pass over the staged binaries", flush=True)
        cmd = [sys.executable, "-I", os.path.join(HERE, "package_tree.py"), tree,
               "--install-prefix", a.install_prefix]
        if a.tolerate_stale_input:
            cmd.append("--tolerate-stale-input")
        rc = subprocess.call(cmd)
        if rc:
            sys.exit(rc)
    if a.step in ("all", "link"):
        print("== devfiles: link", flush=True)
        left = link(a, tree)
        if left and a.strict:
            print(f"::error::{sum(left.values())} build-interface path(s) still point outside "
                  f"the tree")
            sys.exit(1)


if __name__ == "__main__":
    main()
