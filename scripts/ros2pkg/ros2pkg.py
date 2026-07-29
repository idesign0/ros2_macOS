#!/usr/bin/env python3
"""
ros2pkg — placement & sync manager for the ros2_macOS submodule tree.

Pipeline it fits into:
    rsdistro.py  -->  <distro>_repos.json   (official upstream url+branch+owner)
    ros2pkg.py   -->  reads those + your .gitmodules, decides where each repo goes

Core idea
---------
A ROS 2 git repo (one submodule) contains many colcon packages, so the unit of
placement is the *repository*, joined to your tree by a normalized repo-name key
(strips `.git`, the `id_` fork prefix, `-release`, and unifies `-`/`_`).

Folder rule (your rule, automated):
    1. repo already a submodule            -> leave it (report ALREADY)
    2. owner has an override in config     -> <mapped_root>/<repo>
    3. owner already a folder in src/      -> <owner>/<repo>
    4. brand-new owner                     -> <owner>/<repo>   (new root folder)
    5. no upstream url (release-only)      -> NEEDS_MANUAL (never guessed)

Commands
--------
    ros2pkg index                 build submodule_index.json (lookup table, all distros)
    ros2pkg diff  <distro>        official repos not yet installed + proposed placement
    ros2pkg place <repo> -d <d>   resolve one repo's target path
    ros2pkg add   <repo> -d <d>   print (or with --yes, run) the git submodule add
    ros2pkg sync  --from <d>      repos in one distro's tree missing from the others

Nothing is written to your tree unless you pass --yes to `add`/`sync`.
"""

import argparse
import json
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
CONFIG_PATH = os.path.join(HERE, "ros2pkg.config.json")


# ── config ──────────────────────────────────────────────────────────────────
def load_config():
    with open(CONFIG_PATH) as f:
        cfg = json.load(f)
    exp = os.path.expanduser
    md = exp(cfg["manifest_dir"])
    # relative manifest_dir -> resolve against the scripts folder, so all
    # generated data stays inside the repo (closed system, no ~/Downloads).
    cfg["manifest_dir"] = md if os.path.isabs(md) else os.path.join(HERE, md)
    os.makedirs(cfg["manifest_dir"], exist_ok=True)
    cfg["home_dir"] = exp(cfg["home_dir"])
    cfg["_src"] = {d: os.path.join(cfg["home_dir"], rel)
                   for d, rel in cfg["distros"].items()}
    # pre-compile domain rules once (case-insensitive)
    compiled = []
    for rule in cfg.get("domain_rules", []):
        compiled.append({
            "domain": rule["domain"],
            "subdomain": rule.get("subdomain"),
            "pkg": re.compile(rule["pkg"], re.I) if rule.get("pkg") else None,
            "repo": re.compile(rule["repo"], re.I) if rule.get("repo") else None,
            "owners": set(rule.get("owners", [])),
        })
    cfg["_rules"] = compiled
    return cfg


def classify(cfg, repo_entry):
    """Return (domain, subdomain) for one repo. First matching rule wins;
    falls through to _incoming when nothing matches."""
    name = repo_entry["repository"]
    owner = repo_entry.get("owner")
    pkgs = repo_entry.get("packages", []) or []
    pin = cfg.get("overrides", {}).get(name)
    if pin:
        d, _, s = pin.partition("/")
        return d, (s or None)
    for r in cfg["_rules"]:
        if r["repo"] and r["repo"].search(name):
            return r["domain"], r["subdomain"]
        if r["owners"] and owner in r["owners"]:
            return r["domain"], r["subdomain"]
        if r["pkg"] and any(r["pkg"].search(p) for p in pkgs):
            return r["domain"], r["subdomain"]
    return "_incoming", None


def domain_path(cfg, domain, subdomain, repo_name):
    folder = cfg["domains"].get(domain, {}).get("folder", domain)
    parts = [folder] + ([subdomain] if subdomain else []) + [repo_name]
    return "/".join(parts)


# ── the one normalization every join relies on ───────────────────────────────
def repo_key(name):
    """Canonical key so upstream, your `id_` forks, and `-release` all collide."""
    if not name:
        return None
    n = name.strip().lower()
    n = re.sub(r"\.git$", "", n)
    n = n.rsplit("/", 1)[-1]         # basename if a url slipped in
    n = re.sub(r"^id_", "", n)       # your personal fork prefix
    n = re.sub(r"-release$", "", n)
    n = re.sub(r"[^a-z0-9]", "", n)  # collapse ALL separators: Fast-CDR/fast_cdr/fastcdr -> fastcdr
    return n


def entry_keys(repo_entry):
    """All keys an official repo could match an installed submodule by:
    its repository name AND its source-URL basename (so a repo whose upstream
    was forked/renamed — fastrtps -> Fast-DDS -> id_Fast-DDS — still matches)."""
    keys = {repo_key(repo_entry.get("repository"))}
    url = repo_entry.get("source_url")
    if url:
        keys.add(repo_key(url))
    return {k for k in keys if k}


# ── .gitmodules parser (tolerant; no configparser pitfalls) ───────────────────
def parse_gitmodules(path):
    """Return list of {name, path, url, branch} from a .gitmodules file."""
    subs, cur = [], None
    if not os.path.exists(path):
        return subs
    with open(path) as f:
        for raw in f:
            line = raw.strip()
            m = re.match(r'\[submodule "(.+)"\]', line)
            if m:
                cur = {"name": m.group(1), "path": None, "url": None, "branch": None}
                subs.append(cur)
                continue
            if cur is None:
                continue
            m = re.match(r'(path|url|branch)\s*=\s*(.+)', line)
            if m:
                cur[m.group(1)] = m.group(2).strip()
    return subs


# ── lookup table ──────────────────────────────────────────────────────────────
def build_index(cfg):
    """{distro: {key: {path,url,branch,root,name}}} plus alias keys for matching."""
    index = {}
    for distro, src in cfg["_src"].items():
        gm = os.path.join(src, ".gitmodules")
        table = {}
        for s in parse_gitmodules(gm):
            if not s["path"]:
                continue
            leaf = s["path"].rsplit("/", 1)[-1]
            root = s["path"].split("/", 1)[0]
            entry = {"path": s["path"], "url": s["url"],
                     "branch": s["branch"], "root": root, "name": leaf}
            # register under both url-basename key and path-leaf key so forks
            # and custom folder names both resolve.
            for k in {repo_key(s["url"]), repo_key(leaf)}:
                if k:
                    table.setdefault(k, entry)
        index[distro] = table
    return index


def load_repos(cfg, distro):
    path = os.path.join(cfg["manifest_dir"], f"{distro}_repos.json")
    if not os.path.exists(path):
        sys.exit(f"missing {path} — run rsdistro.py first")
    with open(path) as f:
        return json.load(f)


# ── placement resolver (the heart) ───────────────────────────────────────────
def resolve(cfg, index, distro, repo_entry):
    """Return dict: status, path, root, cmd for one official repo entry."""
    name = repo_entry["repository"]
    url = repo_entry.get("source_url")
    branch = repo_entry.get("source_branch")
    owner = repo_entry.get("owner")

    hit = next((index[distro][k] for k in entry_keys(repo_entry) if k in index[distro]), None)
    if hit:
        return {"status": "ALREADY", "repo": name, "path": hit["path"],
                "root": hit["root"], "cmd": None}

    if not url or not owner:
        return {"status": "NEEDS_MANUAL", "repo": name, "path": None,
                "root": None, "cmd": None,
                "reason": "no upstream source_url in rosdistro"}

    src = cfg["_src"][distro]
    domain, subdomain = classify(cfg, repo_entry)
    path = domain_path(cfg, domain, subdomain, name)
    root = path.split("/", 1)[0]
    new_root = not os.path.isdir(os.path.join(src, root))
    cmd = f"git -C {src} submodule add -b {branch} {url} {path}"
    return {"status": "TO_ADD", "repo": name, "path": path, "root": root,
            "domain": domain, "subdomain": subdomain, "new_root": new_root,
            "branch": branch, "url": url, "cmd": cmd}


# ── commands ──────────────────────────────────────────────────────────────────
def cmd_index(cfg, args):
    index = build_index(cfg)
    out = os.path.join(cfg["manifest_dir"], "submodule_index.json")
    with open(out, "w") as f:
        json.dump(index, f, indent=2)
    print(f"wrote {out}")
    for d, t in index.items():
        # distinct submodules = distinct paths (aliases share entries)
        n = len({e["path"] for e in t.values()})
        print(f"  {d}: {n} submodules indexed")


def cmd_diff(cfg, args):
    distro = args.distro
    index = build_index(cfg)
    repos = load_repos(cfg, distro)
    buckets = {"ALREADY": [], "TO_ADD": [], "NEEDS_MANUAL": []}
    for entry in repos.values():
        r = resolve(cfg, index, distro, entry)
        buckets[r["status"]].append(r)

    print(f"\n{distro}: {len(repos)} official repos")
    print(f"  ALREADY installed : {len(buckets['ALREADY'])}")
    print(f"  TO ADD            : {len(buckets['TO_ADD'])}")
    print(f"  NEEDS MANUAL      : {len(buckets['NEEDS_MANUAL'])}")

    to_add = sorted(buckets["TO_ADD"], key=lambda r: (r["root"], r["repo"]))
    if not args.quiet:
        print("\n  new root folders that would be created:")
        newroots = sorted({r["root"] for r in to_add if r.get("new_root")})
        print("   ", ", ".join(newroots) if newroots else "(none)")
        print(f"\n  TO_ADD (showing {min(args.limit, len(to_add))} of {len(to_add)}):")
        for r in to_add[:args.limit]:
            flag = " [NEW ROOT]" if r.get("new_root") else ""
            print(f"    {r['path']}{flag}")

    if args.write:
        with open(args.write, "w") as f:
            f.write("#!/usr/bin/env bash\nset -euo pipefail\n")
            for r in to_add:
                f.write(r["cmd"] + "\n")
        os.chmod(args.write, 0o755)
        print(f"\n  wrote {len(to_add)} add-commands to {args.write}")

    if buckets["NEEDS_MANUAL"] and not args.quiet:
        print("\n  NEEDS_MANUAL (release-only, no upstream url):")
        for r in sorted(buckets["NEEDS_MANUAL"], key=lambda r: r["repo"]):
            print(f"    {r['repo']}")


def cmd_classify(cfg, args):
    """Classify every official repo into domain[/subdomain] and report coverage."""
    distro = args.distro
    index = build_index(cfg)
    repos = load_repos(cfg, distro)
    installed = set(index[distro].keys())

    tree, mapping = {}, {}
    counts = {"total": 0, "already": 0, "to_add": 0, "incoming": 0}
    for name, entry in repos.items():
        domain, subdomain = classify(cfg, entry)
        state = "already" if (entry_keys(entry) & installed) else "to_add"
        counts["total"] += 1
        counts[state] += 1
        if domain == "_incoming":
            counts["incoming"] += 1
        tree.setdefault(domain, {}).setdefault(subdomain or "", []).append(name)
        mapping[name] = {"domain": domain, "subdomain": subdomain,
                         "state": state,
                         "path": domain_path(cfg, domain, subdomain, name)}

    print(f"\n{distro}: {counts['total']} repos  "
          f"({counts['already']} already installed, {counts['to_add']} to add)")
    covered = counts["total"] - counts["incoming"]
    pct = 100 * covered / counts["total"] if counts["total"] else 0
    print(f"classified: {covered}/{counts['total']} ({pct:.0f}%)   "
          f"_incoming: {counts['incoming']}\n")

    for domain in sorted(tree, key=lambda d: (d == "_incoming", d)):
        subs = tree[domain]
        n = sum(len(v) for v in subs.values())
        folder = cfg["domains"].get(domain, {}).get("folder", domain)
        print(f"  {domain:<14} -> {folder:<16} {n:>4} repos")
        for sub in sorted(subs):
            if sub:
                print(f"      {sub:<12} {len(subs[sub]):>4}")

    if args.write:
        out = os.path.join(cfg["manifest_dir"], f"{distro}_domains.json")
        with open(out, "w") as f:
            json.dump(mapping, f, indent=2)
        print(f"\n  wrote {out}")

    if args.show_incoming and "_incoming" in tree:
        print("\n  _incoming (no rule matched — add a rule or triage):")
        for name in sorted(x for v in tree["_incoming"].values() for x in v):
            print(f"    {name}  ({repos[name].get('owner')})")


def cmd_new(cfg, args):
    """Report repos NOT yet added as submodules (the authoritative pending set).

    'Added' == present in .gitmodules. A repo stays pending across every refresh
    until it is really submoduled — appearing in the rosdistro manifest is not
    enough. The snapshot only distinguishes NEW (appeared in rosdistro since the
    last refresh) from PENDING (seen before, still not added); it never removes
    anything from the pending list.
    """
    distro = args.distro
    index = build_index(cfg)
    repos = load_repos(cfg, distro)
    installed = set(index[distro].keys())     # <- the ONLY definition of "added"

    snap_path = os.path.join(cfg["manifest_dir"], f".seen_{distro}.json")
    seen = set()
    if os.path.exists(snap_path):
        with open(snap_path) as f:
            seen = set(json.load(f))
    have_baseline = len(seen) > 0

    pending = []
    for name, entry in repos.items():
        if entry_keys(entry) & installed:
            continue                          # already a submodule -> done, skip
        r = resolve(cfg, index, distro, entry)
        r["tag"] = "NEW" if (have_baseline and name not in seen) else "PENDING"
        pending.append(r)

    pending.sort(key=lambda r: (r["tag"] != "NEW", r.get("path") or "~", r["repo"]))
    new_n = sum(1 for r in pending if r["tag"] == "NEW")
    manual_n = sum(1 for r in pending if r["status"] == "NEEDS_MANUAL")
    print(f"{distro}: {len(pending)} not-yet-added "
          f"({new_n} new since last refresh, {len(pending) - new_n} still pending, "
          f"{manual_n} need manual url)")
    if not have_baseline:
        print("  (first run — no baseline yet, so nothing is flagged NEW)")

    shown = [r for r in pending if not args.new_only or r["tag"] == "NEW"]
    for r in shown[:args.limit]:
        path = r.get("path") or f"(manual: {r['repo']})"
        print(f"  [{r['tag']:<7}] {path}")
    if len(shown) > args.limit:
        print(f"  ... and {len(shown) - args.limit} more (raise --limit or see *_pending.json)")

    if args.write:
        out = os.path.join(cfg["manifest_dir"], f"{distro}_pending.json")
        with open(out, "w") as f:
            json.dump([{k: r.get(k) for k in
                        ("repo", "tag", "status", "domain", "subdomain", "path", "cmd")}
                       for r in pending], f, indent=2)
        print(f"  wrote {out}")

    if args.update_snapshot:
        with open(snap_path, "w") as f:
            json.dump(sorted(repos.keys()), f)
        print(f"  snapshot updated ({len(repos)} rosdistro repos recorded)")


def cmd_backlog(cfg, args):
    """Write per-domain add-command scripts under manifest_dir/adds/<distro>/."""
    distro = args.distro
    index = build_index(cfg)
    repos = load_repos(cfg, distro)
    by = {}
    for entry in repos.values():
        r = resolve(cfg, index, distro, entry)
        if r["status"] != "TO_ADD" or not r.get("cmd"):
            continue
        dom = r["domain"] + ("/" + r["subdomain"] if r.get("subdomain") else "")
        by.setdefault(dom, []).append(r)
    outdir = os.path.join(cfg["manifest_dir"], "adds", distro)
    os.makedirs(outdir, exist_ok=True)
    for dom, rows in sorted(by.items()):
        fn = os.path.join(outdir, dom.replace("/", "__") + ".sh")
        with open(fn, "w") as f:
            f.write("#!/usr/bin/env bash\nset -euo pipefail\n")
            f.write(f"# {distro}: {len(rows)} repos for domain '{dom}'\n")
            for r in sorted(rows, key=lambda r: r["repo"]):
                f.write(f"# {r['repo']}\n{r['cmd']}\n")
        os.chmod(fn, 0o755)
    total = sum(len(v) for v in by.values())
    print(f"  {distro}: {total} add-commands across {len(by)} scripts -> {outdir}/")


def cmd_place(cfg, args):
    index = build_index(cfg)
    repos = load_repos(cfg, args.distro)
    entry = repos.get(args.repo)
    if not entry:
        # allow lookup by normalized key too
        k = repo_key(args.repo)
        entry = next((v for v in repos.values() if v.get("key") == k), None)
    if not entry:
        sys.exit(f"'{args.repo}' not found in {args.distro}_repos.json")
    r = resolve(cfg, index, args.distro, entry)
    print(json.dumps(r, indent=2))


def _run(cmd):
    return subprocess.run(cmd, shell=True, capture_output=True, text=True)


def _cleanup_partial(src, path):
    """Undo any partial state a failed `git submodule add` may have left, so a
    broken package is never half-added: stray gitlink, orphan dir, .git/modules
    entry, and any leftover .gitmodules section."""
    _run(f"git -C {src} submodule deinit -f {path}")
    _run(f"git -C {src} rm -f {path}")
    _run(f"git -C {src} config -f .gitmodules --remove-section 'submodule.{path}'")
    import shutil
    shutil.rmtree(os.path.join(src, path), ignore_errors=True)
    shutil.rmtree(os.path.join(src, ".git", "modules", path), ignore_errors=True)


def _default_branch(url):
    p = _run(f"git ls-remote --symref {url} HEAD")
    m = re.search(r"ref:\s+refs/heads/(\S+)\s+HEAD", p.stdout)
    return m.group(1) if m else None


def add_one(src, path, url, branch, retry_default=False):
    """Add one submodule robustly. Returns (status, branch_used, error).
    status: 'added' | 'added-default' | 'failed'. On any failure the tree is
    cleaned so nothing broken remains."""
    p = _run(f"git -C {src} submodule add -b {branch} {url} {path}")
    if p.returncode == 0:
        return ("added", branch, None)
    err = (p.stderr.strip().splitlines() or ["?"])[-1]
    _cleanup_partial(src, path)
    if retry_default:
        db = _default_branch(url)
        if db and db != branch:
            p2 = _run(f"git -C {src} submodule add -b {db} {url} {path}")
            if p2.returncode == 0:
                return ("added-default", db, None)
            _cleanup_partial(src, path)
            err = (p2.stderr.strip().splitlines() or [err])[-1]
    return ("failed", branch, err)


def scan_pkg_names(src, subpaths):
    """Map colcon package name -> list of providers {sub, dir, depth}, by
    scanning every package.xml. `sub` = owning submodule path, `dir` =
    directory holding the package.xml, `depth` = levels below the submodule
    root (0 = the submodule IS that package; >0 = a nested/vendored copy).
    This catches duplicate PACKAGE names across repos AND distinguishes a
    dedicated submodule from vendored source (e.g. ROS1 tree inside a repo)."""
    name_re = re.compile(r"<name>\s*([^< ]+)\s*</name>")
    out = subprocess.run(f"find {src} -name package.xml -not -path '*/.git/*'",
                         shell=True, capture_output=True, text=True).stdout
    roots = sorted(subpaths, key=len, reverse=True)  # longest prefix wins
    names = {}
    for pxml in out.splitlines():
        rel = os.path.relpath(pxml, src)
        root = next((p for p in roots if rel == p or rel.startswith(p + "/")), None)
        if not root:
            continue
        try:
            m = name_re.search(open(pxml, encoding="utf-8", errors="ignore").read())
        except OSError:
            continue
        if not m:
            continue
        inner = os.path.dirname(rel)[len(root):].strip("/")   # path below submodule root
        depth = inner.count("/") + 1 if inner else 0
        names.setdefault(m.group(1), []).append(
            {"sub": root, "dir": os.path.dirname(pxml), "depth": depth})
    return names


def _base_paths(src, base):
    """Submodule paths present in <base>:.gitmodules (the 'original' set)."""
    p = _run(f"git -C {src} show {base}:.gitmodules")
    if p.returncode != 0:
        return None
    return {m.group(1) for m in re.finditer(r"path\s*=\s*(.+)", p.stdout)}


def cmd_dedupe_packages(cfg, args):
    """Remove submodules that duplicate a colcon PACKAGE name already provided
    by another submodule. Keeps the 'original' (present on the base branch),
    removes the newly-added duplicate — exactly colcon's 'Duplicate package
    names not supported' error."""
    distro = args.distro
    src = cfg["_src"][distro]
    cur = _run(f"git -C {src} branch --show-current").stdout.strip()
    base = args.base or (cur[:-4] if cur.endswith("-new") else cur)
    orig = _base_paths(src, base)
    if orig is None:
        sys.exit(f"cannot read {base}:.gitmodules — pass --base <branch>")

    subpaths = [l.split(None, 1)[1] for l in
                _run(f"git -C {src} config -f .gitmodules --get-regexp '\\.path$'").stdout.splitlines()]
    names = scan_pkg_names(src, subpaths)

    # Only a DEDICATED submodule (package.xml at/near its root, depth <= 1)
    # counts as "providing" a workspace package. Deeply nested/vendored copies
    # (thirdparty/, a bundled ros1 tree, ...) are the submodule's internal
    # business and are NOT workspace duplicates — colcon in CI doesn't treat
    # them as such. Flag a conflict only when >= 2 dedicated submodules collide.
    DEDICATED = 1
    remove_subs = []
    for name, ps in sorted(names.items()):
        best = {}
        for p in ps:
            best[p["sub"]] = min(best.get(p["sub"], 99), p["depth"])
        ded = sorted(s for s, d in best.items() if d <= DEDICATED)
        if len(ded) < 2:
            continue
        keep = next((s for s in ded if s in orig), ded[0])   # prefer original(base)
        for s in ded:
            if s != keep:
                remove_subs.append((name, s, keep))

    remove_subs = sorted(set(remove_subs))
    print(f"{distro}: {len(remove_subs)} duplicate submodule(s) to remove")
    for name, sub, keep in remove_subs:
        print(f"  '{name}': rm {sub}  (keep {keep})")
    if not remove_subs:
        return
    if not args.yes:
        print("(dry-run; pass --yes to remove)")
        return
    for _, sub, _ in remove_subs:
        _cleanup_partial(src, sub)
    print(f"removed {len(remove_subs)} duplicate submodule(s)")


def cmd_add(cfg, args):
    index = build_index(cfg)
    repos = load_repos(cfg, args.distro)
    src = cfg["_src"][args.distro]

    if args.all:
        return _add_all(cfg, index, repos, src, args)

    entry = repos.get(args.repo) or next(
        (v for v in repos.values() if repo_key(args.repo) in entry_keys(v)), None) if args.repo else None
    if not entry:
        sys.exit(f"'{args.repo}' not found in {args.distro}_repos.json")
    r = resolve(cfg, index, args.distro, entry)
    if r["status"] != "TO_ADD":
        sys.exit(f"{args.repo}: status is {r['status']} — nothing to add")
    print(r["cmd"])
    if not args.yes:
        print("(dry-run; pass --yes to execute)")
        return
    status, br, err = add_one(src, r["path"], r["url"], r["branch"], args.retry_default_branch)
    if status == "failed":
        sys.exit(f"FAILED (cleaned up, nothing added): {r['path']}  [{r['branch']}]  {err}")
    print(f"{status} {r['path']}  [{br}]")


def _add_all(cfg, index, repos, src, args):
    """Bulk add every pending classified repo. Broken ones are skipped cleanly
    (auto-cleanup) and written to <distro>_failed.json — never left half-added."""
    # package names already provided by the installed tree (colcon-equivalent);
    # reserve as we go so we also avoid new-vs-new package collisions.
    subpaths = [l.split(None, 1)[1] for l in
                _run(f"git -C {src} config -f .gitmodules --get-regexp '\\.path$'").stdout.splitlines()]
    # only names provided by a DEDICATED submodule (shallow package.xml) count;
    # vendored/nested copies aren't workspace packages.
    provided = {n for n, ps in scan_pkg_names(src, subpaths).items()
                if min(p["depth"] for p in ps) <= 1}

    todo, skipped_dup = [], []
    for entry in repos.values():
        r = resolve(cfg, index, args.distro, entry)
        if r["status"] != "TO_ADD":
            continue
        if r["domain"] == "_incoming" and not args.include_incoming:
            continue
        clash = [p for p in (entry.get("packages") or []) if p in provided]
        if clash:
            skipped_dup.append({"path": r["path"], "packages": clash})
            continue
        provided.update(entry.get("packages") or [])
        todo.append(r)
    print(f"{args.distro}: {len(todo)} to add, {len(skipped_dup)} skipped (duplicate package name)"
          + ("" if args.yes else "  (dry-run; pass --yes to execute)"))
    if not args.yes:
        return

    added, salvaged, failed = [], [], []
    for i, r in enumerate(todo, 1):
        status, br, err = add_one(src, r["path"], r["url"], r["branch"], args.retry_default_branch)
        if status == "added":
            added.append(r["path"])
        elif status == "added-default":
            salvaged.append({"path": r["path"], "branch": br})
            print(f"[{i}/{len(todo)}] salvaged {r['path']} on default branch '{br}'")
        else:
            failed.append({"repo": r["repo"], "path": r["path"], "url": r["url"],
                           "branch": r["branch"], "error": err})
            print(f"[{i}/{len(todo)}] FAILED {r['path']}  [{r['branch']}]  {err}")

    report = os.path.join(cfg["manifest_dir"], f"{args.distro}_failed.json")
    with open(report, "w") as f:
        json.dump(failed, f, indent=2)
    print(f"\nadded={len(added)}  salvaged={len(salvaged)}  failed={len(failed)}")
    print(f"broken packages (skipped clean, not added) -> {report}")


def cmd_sync(cfg, args):
    index = build_index(cfg)
    src_distro = args.src
    targets = args.to or [d for d in cfg["distros"] if d != src_distro]
    src_paths = {e["path"]: e for e in index[src_distro].values()}
    for tgt in targets:
        tgt_keys = set(index[tgt].keys())
        missing = [e for k, e in {repo_key(v["name"]): v for v in src_paths.values()}.items()
                   if k not in tgt_keys]
        print(f"\n{src_distro} -> {tgt}: {len(missing)} repos present in {src_distro} but missing in {tgt}")
        for e in sorted(missing, key=lambda e: e["path"])[:args.limit]:
            tgt_src = cfg["_src"][tgt]
            branch = e["branch"] or "<branch>"
            print(f"    git -C {tgt_src} submodule add -b {branch} {e['url']} {e['path']}")


def main():
    cfg = load_config()
    p = argparse.ArgumentParser(prog="ros2pkg", description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = p.add_subparsers(dest="cmd", required=True)

    sub.add_parser("index", help="build submodule_index.json").set_defaults(fn=cmd_index)

    c = sub.add_parser("classify", help="classify official repos into domains")
    c.add_argument("distro")
    c.add_argument("--write", action="store_true", help="write <distro>_domains.json")
    c.add_argument("--show-incoming", action="store_true")
    c.set_defaults(fn=cmd_classify)

    n = sub.add_parser("new", help="repos not yet added as submodules (pending set)")
    n.add_argument("distro")
    n.add_argument("--limit", type=int, default=40)
    n.add_argument("--new-only", action="store_true", help="show only NEW-since-last-refresh")
    n.add_argument("--write", action="store_true", help="write <distro>_pending.json")
    n.add_argument("--update-snapshot", action="store_true",
                   help="record current rosdistro repo set as the new baseline")
    n.set_defaults(fn=cmd_new)

    b = sub.add_parser("backlog", help="write per-domain add scripts under manifests/adds/")
    b.add_argument("distro")
    b.set_defaults(fn=cmd_backlog)

    dp = sub.add_parser("dedupe-packages",
                        help="remove submodules with a colcon package name already provided elsewhere")
    dp.add_argument("distro")
    dp.add_argument("--base", help="branch holding the 'original' set (default: <distro>)")
    dp.add_argument("--yes", action="store_true", help="actually remove")
    dp.set_defaults(fn=cmd_dedupe_packages)

    d = sub.add_parser("diff", help="official repos not yet installed")
    d.add_argument("distro")
    d.add_argument("--limit", type=int, default=60)
    d.add_argument("--write", metavar="FILE", help="write add-commands to a script")
    d.add_argument("--quiet", action="store_true")
    d.set_defaults(fn=cmd_diff)

    pl = sub.add_parser("place", help="resolve one repo's target path")
    pl.add_argument("repo")
    pl.add_argument("-d", "--distro", default="humble")
    pl.set_defaults(fn=cmd_place)

    a = sub.add_parser("add", help="add submodule(s); broken ones are skipped clean + reported")
    a.add_argument("repo", nargs="?", help="single repo; omit with --all")
    a.add_argument("-d", "--distro", default="humble")
    a.add_argument("--yes", action="store_true", help="actually run it")
    a.add_argument("--all", action="store_true", help="add every pending classified repo")
    a.add_argument("--include-incoming", action="store_true", help="also add _incoming repos")
    a.add_argument("--retry-default-branch", action="store_true",
                   help="if the configured branch is missing, retry the repo's default branch")
    a.set_defaults(fn=cmd_add)

    s = sub.add_parser("sync", help="repos in one distro missing from others")
    s.add_argument("--from", dest="src", default="humble")
    s.add_argument("--to", nargs="*")
    s.add_argument("--limit", type=int, default=60)
    s.set_defaults(fn=cmd_sync)

    args = p.parse_args()
    args.fn(cfg, args)


if __name__ == "__main__":
    main()
