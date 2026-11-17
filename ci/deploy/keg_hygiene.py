#!/usr/bin/env python3
"""keg_hygiene.py <tree> --prefix <install prefix>

Runs after package_tree.py / devfiles.py. Those relocate OUR build's paths; this cleans
what third parties put in the tree, so check_keg_paths.py can hold the keg to "every
path points inside the keg or at the system":

 1. text files, whole tree (vendor/ included):
    /opt/homebrew/Cellar/<f>/<ver>/x  -> <prefix>/vendor/opt/<f>/x when <f> is vendored,
                                         else the stable /opt/homebrew/opt/<f>/x
    a versioned SDK (CommandLineTools MacOSX26.sdk, Xcode_26.6.app ... MacOSX.sdk)
                                      -> /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk
                                         (Homebrew requires the Command Line Tools)
    /private/tmp/<sandbox>/..., /private/var/folders/..., /tmp/buildd/...
                                      -> removed: a build sandbox that no longer exists
                                         anywhere (qmake .prl build dirs, Python sysconfig,
                                         *.settings, mesh "authoring" fields)
 1b. Doxygen tag files and vendor/.devfiles.json only record where sources were: the
    build workspace root (/Users/runner/work/<repo>/<repo>/) is made relative there.
 2. mach-o: an @rpath/ or @loader_path/ load that resolves to nothing inside the keg is
    pointed at the one in-keg file with that name (webots_ros2_driver's
    @rpath/Contents/lib/controller/*, boost-1.89 siblings, Qt6 plugins -> Qt frameworks).
    Two or more candidates with different content -> error, never a guess.
 3. *.dSYM bundles are dropped (debug info only, full of build-machine paths), and so is a
    vendored formula's bin/ or libexec/ tool whose own libraries are not in the keg
    (VTK's wrapping generators, a stray protoc) -- it could never run.
Binary files are never length-changed; check_keg_paths.py reports what remains in them.
"""
import argparse, collections, hashlib, io, os, re, shutil, subprocess, sys

ap = argparse.ArgumentParser()
ap.add_argument("tree")
ap.add_argument("--prefix", required=True)
a = ap.parse_args()
tree = os.path.realpath(a.tree)
P = a.prefix.rstrip("/")
MAGIC = {b"\xcf\xfa\xed\xfe", b"\xca\xfe\xba\xbe"}
END = r"[^\s\"'`;:,()<>\[\]{}|\\]*"
CLT_SDK = "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk"

# ---- 3. dSYM bundles
dsyms = []
for root, dirs, names in os.walk(tree):
    for d in list(dirs):
        if d.endswith(".dSYM"):
            dsyms.append(os.path.join(root, d)); dirs.remove(d)
for d in dsyms:
    shutil.rmtree(d)

vendored = set(os.listdir(os.path.join(tree, "vendor", "opt"))) if os.path.isdir(os.path.join(tree, "vendor", "opt")) else set()
self_name = os.path.basename(P)          # ros2-jazzy

def cellar(m):
    f, rest = m.group(1), m.group(3) or ""
    if f == self_name:
        return P + rest
    if f in vendored:
        return f"{P}/vendor/opt/{f}{rest}"
    return f"/opt/homebrew/opt/{f}{rest}"

RULES = [
    (re.compile(r"/opt/homebrew/Cellar/([^/\s\"']+)/([^/\s\"']+)(/" + END + r")?"), cellar),
    (re.compile(r"/Library/Developer/CommandLineTools/SDKs/MacOSX\d[\d.]*\.sdk"), lambda m: CLT_SDK),
    (re.compile(r"/Applications/Xcode[\w.-]*\.app/Contents/Developer/Platforms/MacOSX\.platform/Developer/SDKs/MacOSX[\d.]*\.sdk"),
     lambda m: CLT_SDK),
    (re.compile(r"(?:/private/tmp/|/private/var/folders/|/tmp/buildd/)" + END), lambda m: ""),
]
# Files that only RECORD where sources were (Doxygen tag files cross-referencing docs,
# devfiles.py's own staging manifest): the build workspace root becomes relative.
RELATIVE_ONLY = re.compile(r"(\.tag|\.tag\.xml|^\.devfiles\.json)$")
WS_ROOT = re.compile(r"/Users/runner/work/[^/\s\"']+/[^/\s\"']+/")
QUICK = (b"/Users/runner/work/", b"/opt/homebrew/Cellar/", b"/SDKs/MacOSX", b"/private/tmp/", b"/private/var/folders/", b"/tmp/buildd/")

text_fixed, machos = 0, []
for root, dirs, names in os.walk(tree):
    for n in names:
        f = os.path.join(root, n)
        if os.path.islink(f) or not os.path.isfile(f):
            continue
        try:
            with open(f, "rb") as h:
                head = h.read(4)
                if head in MAGIC:
                    machos.append(f); continue
                raw = head + h.read()
        except OSError:
            continue
        if b"\0" in raw or not any(q in raw for q in QUICK) or n.endswith((".pyc", ".pyo")):
            continue
        try:
            txt = raw.decode("utf-8")
        except UnicodeDecodeError:
            continue
        new = txt
        for rx, fn in RULES:
            new = rx.sub(fn, new)
        if RELATIVE_ONLY.search(n):
            new = WS_ROOT.sub("", new)
        if new != txt:
            st = os.stat(f)
            os.chmod(f, st.st_mode | 0o200)
            io.open(f, "w", encoding="utf-8", newline="").write(new)
            os.chmod(f, st.st_mode)
            text_fixed += 1

# ---- 1b2. ament_export_dependencies(Qt5): the exported dependency list makes every
# overlay run find_package(Qt5 QUIET REQUIRED) with no component, which Qt5Config rejects
# ("The Qt5 package requires at least one component") -- nav2_rviz_plugins, rviz_imu_plugin
# and 7 more (upstream). Qt5Widgets brings Core and Gui and is what those plugins link.
qt5_fixed = 0
for root, dirs, names in os.walk(os.path.join(tree, "share")):
    if "ament_cmake_export_dependencies-extras.cmake" in names:
        f = os.path.join(root, "ament_cmake_export_dependencies-extras.cmake")
        t = open(f).read()
        m = re.search(r'set\(_exported_dependencies "([^"]*)"\)', t)
        if m and "Qt5" in m.group(1).split(";"):
            deps = ["Qt5Widgets" if d == "Qt5" else d for d in m.group(1).split(";")]
            t = t[:m.start(1)] + ";".join(dict.fromkeys(deps)) + t[m.end(1):]
            st = os.stat(f); os.chmod(f, st.st_mode | 0o200)
            io.open(f, "w", encoding="utf-8", newline="").write(t); os.chmod(f, st.st_mode)
            qt5_fixed += 1
if qt5_fixed:
    print(f"  hygiene: {qt5_fixed} exported dependency list(s): Qt5 -> Qt5Widgets")

# ---- 1c. CMake import blocks for files that are not shipped.
# devfiles.py mirrors a big formula's CMake package (g2o, arrow, libkml, ...) but only the
# libraries the tree runs with; its <Pkg>Targets-<config>.cmake still lists every library,
# and CMake refuses the WHOLE package when one listed file is missing. Drop the import
# block (location + import check) of exactly those targets: find_package() works, the
# shipped targets work, an unshipped one is defined without a location.
IMPORT_BLOCK = re.compile(
    r'# Import target "([^"]+)" for configuration "[^"]*"\n.*?'
    r'list\(APPEND _(?:cmake_import_check_files_for_|IMPORT_CHECK_FILES_FOR_)\1 [^\n]*\)\n', re.S)
def import_prefix_levels(d):
    for x in os.listdir(d):
        if not x.endswith(".cmake"):
            continue
        t = open(os.path.join(d, x), errors="replace").read()
        if 'get_filename_component(_IMPORT_PREFIX "${CMAKE_CURRENT_LIST_FILE}" PATH)' in t:
            # the first call yields the file's own dir; each further PATH goes one level up
            return t.count('get_filename_component(_IMPORT_PREFIX "${_IMPORT_PREFIX}" PATH)')
    return None
def kegpath(path, import_prefix):
    path = path.replace("${_IMPORT_PREFIX}", import_prefix)
    if path.startswith(P + "/"):
        path = tree + path[len(P):]
    return path
pruned = collections.Counter()
for root, dirs, names in os.walk(tree):
    for n in names:
        if not re.search(r"Targets-[\w]+\.cmake$", n):
            continue
        f = os.path.join(root, n)
        txt = open(f, errors="replace").read()
        lv = import_prefix_levels(root)
        ip = root
        for _ in range(lv if lv is not None else 0):
            ip = os.path.dirname(ip)
        def keep(m):
            files = re.findall(r'IMPORTED_LOCATION(?:_\w+)? "([^"]+)"', m.group(0))
            if files and lv is None and any("${_IMPORT_PREFIX}" in x for x in files):
                return m.group(0)
            if files and not all(os.path.exists(kegpath(x, ip)) for x in files):
                pruned[os.path.relpath(f, tree)] += 1
                return ""
            return m.group(0)
        new = IMPORT_BLOCK.sub(keep, txt)
        if new != txt:
            st = os.stat(f)
            os.chmod(f, st.st_mode | 0o200)
            io.open(f, "w", encoding="utf-8", newline="").write(new)
            os.chmod(f, st.st_mode)

# ---- 1c2. exported include dirs that do not exist. CMake rejects an imported target
# whose INTERFACE_INCLUDE_DIRECTORIES names a missing directory ("contains non-existent
# path"), so the whole package fails to load: stomp exports ${_IMPORT_PREFIX}/examples,
# which it never installs. Drop such entries (plain paths inside the keg only).
inc_fixed = collections.Counter()
INC = re.compile(r'(INTERFACE_INCLUDE_DIRECTORIES ")([^"]*)(")')
for root, dirs, names in os.walk(tree):
    for n in names:
        if not re.search(r"([Tt]argets|Export)\.cmake$", n):
            continue
        f = os.path.join(root, n)
        try:
            txt = open(f).read()
        except (OSError, UnicodeDecodeError):
            continue
        if "INTERFACE_INCLUDE_DIRECTORIES" not in txt:
            continue
        lv = import_prefix_levels(root)
        ip = root
        for _ in range(lv if lv is not None else 0):
            ip = os.path.dirname(ip)
        def fix(m):
            keep = []
            for e in m.group(2).split(";"):
                plain = "$<" not in e and (e.startswith("${_IMPORT_PREFIX}/") and lv is not None
                                           or e.startswith(P + "/"))
                if plain and not os.path.isdir(kegpath(e, ip)):
                    inc_fixed[os.path.relpath(f, tree)] += 1
                    continue
                keep.append(e)
            return m.group(1) + ";".join(keep) + m.group(3)
        new = INC.sub(fix, txt)
        if new != txt:
            st = os.stat(f); os.chmod(f, st.st_mode | 0o200)
            io.open(f, "w", encoding="utf-8", newline="").write(new); os.chmod(f, st.st_mode)
for f, k in sorted(inc_fixed.items()):
    print(f"  hygiene: {f}: dropped {k} exported include dir(s) that do not exist")

# ---- 1d. scripts whose interpreter is a keg path that is not shipped (openvino's
# libexec/bin tools run libexec/bin/python, a venv Homebrew builds and we do not vendor).
dead_scripts = []
for root, dirs, names in os.walk(os.path.join(tree, "vendor")):
    for n in names:
        f = os.path.join(root, n)
        if os.path.islink(f) or not os.access(f, os.X_OK):
            continue
        try:
            first = open(f, "rb").readline(512)
        except OSError:
            continue
        m = re.match(rb"#!\s*(\S+)", first)
        if m:
            interp = m.group(1).decode(errors="replace")
            if interp.startswith(P + "/") and not os.path.exists(tree + interp[len(P):]):
                dead_scripts.append(f)
for f in dead_scripts:
    os.chmod(os.path.dirname(f), os.stat(os.path.dirname(f)).st_mode | 0o200)
    os.remove(f)
    print(f"  hygiene: removed {os.path.relpath(f, tree)} (its interpreter is not shipped)")
for f, k in sorted(pruned.items()):
    print(f"  hygiene: {f}: dropped {k} import block(s) for files not shipped")

# ---- 2. unresolved @rpath / @loader_path loads
def in_keg(p):
    if not (p == P or p.startswith(P + "/")):
        return False
    return os.path.exists(tree + p[len(P):])

by_name = {}
for f in machos:
    by_name.setdefault(os.path.basename(f), []).append(f)
# frameworks: index "X.framework/Versions/A/X" style tails too
def digest(f):
    return hashlib.sha1(open(f, "rb").read()).hexdigest()

fixed, failed, drop = 0, [], set()
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
        ldir = os.path.join(P, os.path.dirname(rel)).rstrip("/")
        rps = [r.replace("@loader_path", ldir).replace("@executable_path", ldir) for r in rpaths]
        change = []
        for q in loads:
            if q.startswith("@rpath/"):
                cands = [os.path.normpath(os.path.join(r, q[7:])) for r in rps]
            elif q.startswith(("@loader_path/", "@executable_path/")):
                cands = [os.path.normpath(q.replace("@loader_path", ldir).replace("@executable_path", ldir))]
            else:
                continue
            if any(in_keg(c) or c.startswith(("/usr/lib/", "/System/")) for c in cands):
                continue
            tail = q.split("/", 1)[1]
            fw = re.search(r"([^/]+\.framework/.*)$", tail)
            hits = [h for h in machos if h.endswith("/" + (fw.group(1) if fw else os.path.basename(tail)))]
            if not hits:
                if re.match(r"vendor/opt/[^/]+/(bin|libexec)/", rel):
                    drop.add(f); continue       # a formula's build tool whose libs we do not ship
                failed.append((rel, q, "no file of that name in the keg")); continue
            # same name in several places (webots' lib/controller/libdriver.dylib vs
            # webots_ros2_driver's lib/libdriver.dylib): the longest shared path suffix wins
            def suffix_len(h):
                a_, b_ = os.path.relpath(h, tree).split("/")[::-1], tail.split("/")[::-1]
                n = 0
                while n < min(len(a_), len(b_)) and a_[n] == b_[n]:
                    n += 1
                return n
            best = max(map(suffix_len, hits))
            hits = [h for h in hits if suffix_len(h) == best]
            if len({digest(h) for h in hits}) > 1:
                failed.append((rel, q, f"{len(hits)} different candidates")); continue
            change += ["-change", q, P + "/" + os.path.relpath(hits[0], tree)]
        if f in drop:
            continue
        if change:
            st = os.stat(f); os.chmod(f, st.st_mode | 0o200)
            r = subprocess.run(["install_name_tool"] + change + [f], capture_output=True, text=True)
            if r.returncode == 0 and subprocess.run(["codesign", "--force", "-s", "-", f],
                                                   capture_output=True).returncode == 0:
                fixed += 1
            else:
                failed.append((rel, " ".join(change[1::3]), r.stderr.strip()[:200]))
            os.chmod(f, st.st_mode)

for f in sorted(drop):
    os.remove(f)
    print(f"  hygiene: removed {os.path.relpath(f, tree)} (build tool; its libraries are not shipped)")
print(f"  hygiene: dropped {len(dsyms)} .dSYM bundle(s); cleaned {text_fixed} text file(s); "
      f"repointed unresolved loads in {fixed} mach-o file(s)")
if failed:
    # Not fatal here: check_keg_paths.py is the gate and holds the allow list (boost-1.89
    # cannot be repointed without a relink); it fails the job for anything not listed.
    print(f"::warning::{len(failed)} load(s) resolve to nothing in the keg and could not be repointed:")
    for rel, q, why in failed[:30]:
        print(f"     {rel}: {q} ({why})")
