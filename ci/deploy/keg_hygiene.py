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
 2. mach-o: an @rpath/ or @loader_path/ load that resolves to nothing inside the keg is
    pointed at the one in-keg file with that name (webots_ros2_driver's
    @rpath/Contents/lib/controller/*, boost-1.89 siblings, Qt6 plugins -> Qt frameworks).
    Two or more candidates with different content -> error, never a guess.
 3. *.dSYM bundles are dropped (debug info only, full of build-machine paths), and so is a
    vendored formula's bin/ or libexec/ tool whose own libraries are not in the keg
    (VTK's wrapping generators, a stray protoc) -- it could never run.
Binary files are never length-changed; check_keg_paths.py reports what remains in them.
"""
import argparse, hashlib, io, os, re, shutil, subprocess, sys

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
QUICK = (b"/opt/homebrew/Cellar/", b"/SDKs/MacOSX", b"/private/tmp/", b"/private/var/folders/", b"/tmp/buildd/")

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
        if new != txt:
            st = os.stat(f)
            os.chmod(f, st.st_mode | 0o200)
            io.open(f, "w", encoding="utf-8", newline="").write(new)
            os.chmod(f, st.st_mode)
            text_fixed += 1

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
    print(f"::error::{len(failed)} load(s) resolve to nothing in the keg and could not be repointed:")
    for rel, q, why in failed[:30]:
        print(f"     {rel}: {q} ({why})")
    sys.exit(1)
