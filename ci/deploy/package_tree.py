#!/usr/bin/env python3
"""
Package a built ROS 2 install tree so it does not depend on the user's Homebrew.

Does, at BUILD time, the two jobs the formula's post_install used to do at INSTALL time
(Homebrew 7 deprecates `post_install` and its replacement `post_install_steps` is a closed
set of built-in actions, so custom Ruby there has no future), plus the vendoring that
makes the stale-reference repair unnecessary:

  1. VENDOR  -- copy every Homebrew dylib in the tree's transitive closure into
                <tree>/vendor/lib, rewrite each reference to @rpath/<basename>, and give
                every mach-o an absolute rpath to that directory. After this the tree
                references no /opt/homebrew/opt path at all, so it cannot be broken by the
                user's brew being newer, older, or missing a library upstream deleted.
  2. RPATHS   -- the surviving half of post_install: absolute rpaths for the in-tree
                opt/*/lib vendor dirs. macOS Hardened Runtime does not expand @loader_path
                for a plugin loaded via dlopen, so gz/pluginlib plugins need the absolute
                path baked in.

ABSOLUTE rpaths, not @loader_path: that is what the existing post_install settled on for
exactly the dlopen reason above, and the files here sit at many different depths.

Flat vendor dir: measured across base + all 21 kilted domains, the 478-dylib closure had
478 distinct basenames. That is NOT guaranteed -- a formula whose soname omits the version
(libcrypto.3.dylib, libdart.6.19.dylib) would put two versions on one filename, unlike
abseil whose soname carries it (libabsl_hash.2608.0.0.dylib). Step 1 now refuses to package
when two DIFFERENT dylibs claim one basename, and reports same-file duplicates (the
opt/ vs realpath'd Cellar/ routes) as collapsed instead.

Run it in the `assemble` job between "Merge shard deltas" and "Pack full workspace
install".

Usage:
  package_tree.py <tree> --install-prefix /opt/homebrew/opt/ros2-kilted
  package_tree.py <tree> --install-prefix ... --tolerate-stale-input   # smoke only
  package_tree.py <tree> --install-prefix ... --allow-missing          # local only
  package_tree.py <tree> --install-prefix ... --verify-only
"""
import argparse, collections, json, os, re, shutil, subprocess, sys

HB = "/opt/homebrew/opt"
REF = re.compile(rf"{re.escape(HB)}/([^/]+)/(\S+\.(?:dylib|so))")
MACHO_EXT = (".dylib", ".so", ".bundle")


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


def refs_of(paths):
    """path -> set of /opt/homebrew/opt/... references."""
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
                if m:
                    out[cur].add(f"{HB}/{m.group(1)}/{m.group(2)}")
        # single-file batches print no header line
        if len(batch) == 1 and batch[0] not in out:
            out[batch[0]] = {f"{HB}/{m.group(1)}/{m.group(2)}"
                             for m in REF.finditer(r.stdout)}
    return out


def rpath_refs(path):
    """Brew dylibs reference their SIBLINGS via @rpath/@loader_path, resolved against
    their own LC_RPATH -- typically @loader_path/../lib, i.e. their own keg's lib dir.
    Measured: 187 of 406 vendored brew dylibs do this. An absolute-path-only scan misses
    every one of them, and copying such a dylib into a flat vendor dir silently breaks it
    because @loader_path now points somewhere else. That was 20 regressions out of 102
    dylibs that had loaded before packaging (libgeos_c -> @rpath/libgeos.3.15.0.dylib).
    So resolve them here and pull the real files into the closure too."""
    out = set()
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
                    out.add(os.path.realpath(c))
                    break
    return out


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
    a = ap.parse_args()

    tree = a.tree.rstrip("/")
    vendor_abs = os.path.join(tree, a.vendor_subdir)
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

    # ---- 1. vendor
    os.makedirs(vendor_abs, exist_ok=True)
    base_of = {}
    claimed = {}          # basename -> the source path that produced the vendored copy
    collisions = []       # (basename, first src, second src) with DIFFERENT contents
    for src in present:
        b = os.path.basename(src)
        dst = os.path.join(vendor_abs, b)
        if b in claimed:
            # The vendor dir is FLAT, so two sources with one basename are one file. That is
            # fine when they are the same physical dylib reached by two routes -- refs_of()
            # yields /opt/homebrew/opt/... while rpath_refs() realpaths to
            # /opt/homebrew/Cellar/... -- and a silent ABI swap when they are not.
            # Soname decides which: libabsl_hash.2608.0.0.dylib carries its version, so two
            # abseils coexist as two files; libcrypto.3.dylib and libdart.6.19.dylib do not,
            # so two versions would land on one name and every consumer would be rewritten to
            # whichever arrived first. Proven locally with two installed libssh2 kegs: both
            # ship libssh2.1.dylib with different md5s. Refuse rather than ship that.
            if os.path.realpath(claimed[b]) != os.path.realpath(src):
                collisions.append((b, claimed[b], src))
            continue
        claimed[b] = src
        if not os.path.exists(dst):
            shutil.copy2(src, dst)
            os.chmod(dst, 0o755)
        base_of[src] = b

    if collisions:
        print(f"::error::{len(collisions)} basename collision(s) in the flat vendor dir: two "
              f"DIFFERENT dylibs share one filename, so vendoring would bind every consumer "
              f"of that name to whichever was copied first -- a silent ABI swap. Refusing to "
              f"package.")
        for b, first, second in collisions[:20]:
            print(f"     {b}\n        kept    {first}\n        dropped {second}")
        sys.exit(1)

    dupes = len(present) - len(claimed)
    print(f"  vendored             : {len(claimed)} dylibs into {a.vendor_subdir}"
          + (f" ({dupes} duplicate path(s) of the same file collapsed)" if dupes else ""),
          flush=True)

    vendored = [os.path.join(vendor_abs, b) for b in sorted(set(base_of.values()))]

    # vendored copies: own install_name -> @rpath, and their refs rewritten too
    for v in vendored:
        run(["install_name_tool", "-id", f"@rpath/{os.path.basename(v)}", v])

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
        if own.startswith(HB):
            if run(["install_name_tool", "-id",
                    f"@rpath/{os.path.basename(p)}", p]).returncode == 0:
                reid += 1
    print(f"  re-identified         : {reid} in-tree copies of brew dylibs", flush=True)

    # ---- 2. rewrite every reference, in the tree AND in the vendored copies
    targets = files + vendored
    allrefs = refs_of(targets)
    changed = 0
    for p, s in allrefs.items():
        if not s:
            continue
        cmd = ["install_name_tool"]
        for r in sorted(s):
            if not os.path.exists(r):
                # never rewrite a reference we could not vendor -- that converts a
                # resolvable absolute path into a dangling @rpath
                continue
            cmd += ["-change", r, f"@rpath/{os.path.basename(r)}"]
        if len(cmd) == 1:
            continue
        cmd.append(p)
        if run(cmd).returncode == 0:
            changed += 1
    print(f"  references rewritten : {changed} files", flush=True)

    # ---- 3. absolute rpath to the vendor dir, for everything
    added = 0
    for p in targets:
        have = run(["otool", "-l", p]).stdout
        if rpath_installed in have:
            continue
        if run(["install_name_tool", "-add_rpath", rpath_installed, p]).returncode == 0:
            added += 1
    print(f"  vendor rpath added   : {added} files", flush=True)

    # ---- 4. the surviving post_install job: in-tree opt/*/lib vendor dirs.
    # Hardened Runtime will not expand @loader_path for a dlopen'd plugin, so the
    # absolute installed path has to be baked in.
    inner = 0
    for d in sorted(set(os.path.dirname(x) for x in
                        (os.path.join(r, n)
                         for r, _, ns in os.walk(os.path.join(tree, "opt"))
                         for n in ns) if x.endswith(".dylib"))):
        libs = {n for n in os.listdir(d) if n.endswith(".dylib")}
        if not libs:
            continue
        installed_dir = os.path.join(a.install_prefix, os.path.relpath(d, tree))
        for p, s in allrefs.items():
            o = run(["otool", "-L", p]).stdout
            if not any(f"@rpath/{l}" in o for l in libs):
                continue
            if installed_dir in run(["otool", "-l", p]).stdout:
                continue
            if run(["install_name_tool", "-add_rpath", installed_dir, p]).returncode == 0:
                inner += 1
    print(f"  in-tree opt/ rpaths  : {inner} additions", flush=True)

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
        print(f"::error::{len(left)} files still reference {HB} after packaging")
        for p, s in list(left.items())[:10]:
            print(f"     {p[len(tree)+1:]} -> {sorted(s)[:2]}")
        sys.exit(1)
    print(f"  VERIFIED             : 0 remaining {HB} references")


if __name__ == "__main__":
    main()
