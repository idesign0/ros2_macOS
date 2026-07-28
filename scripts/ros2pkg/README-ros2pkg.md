# ros2pkg — domain-based package placement for ros2_macOS

Decides **where** each official ROS 2 repo should live in `src/`, grouped by
**use-case domain** (not GitHub owner), and tracks which repos are **not yet
added as submodules**.

## Pipeline

```
rsdistro.py        rosdistro  ->  ./manifests/<distro>_{packages,repos}.json
                                  (upstream url + branch + owner + package list)
        |
        v
ros2pkg.py         *_repos.json + your .gitmodules
                   -> domain[/subdomain] for every repo
                   -> git submodule add commands  (never run automatically)
```

**Closed system:** everything lives under `humble-ros2/src/scripts/` — the
scripts *and* all generated data (in `./manifests/`). Nothing depends on
`~/Downloads`. The only external requirement is the `rosdistro` Python package
(the data source for new packages). `manifests/` is git-ignored; delete it any
time and `refresh.sh` regenerates it.

- **Repo, not package, is the unit.** One git repo = one submodule = many colcon
  packages. A repo is joined to your tree by a normalized key (strips `.git`,
  the `id_` fork prefix, `-release`, unifies `-`/`_`).
- **"Added" = present in `.gitmodules`.** Nothing else counts as added.

## Files

| File | Role |
|------|------|
| `rsdistro.py` | regenerate manifests from rosdistro (run on a schedule) |
| `ros2pkg.py` | the engine — you rarely edit this |
| `ros2pkg.config.json` | **what you maintain**: domains, rules, overrides |
| `refresh.sh` | scheduler entry point: rsdistro → index → per-distro pending report |
| `refresh.log` | appended output of each scheduled run |

Generated into `./manifests/` (git-ignored): `<distro>_{packages,repos}.json`,
`submodule_index.json` (lookup table), `<distro>_domains.json` (full
classification), `<distro>_pending.json` (not-yet-added), `.seen_<distro>.json`
(baseline for NEW detection), and `adds/<distro>/<domain>.sh` (backlog scripts).

## Placement precedence

1. **`overrides`** — hard per-repo pin, beats everything. Use for repos that
   span domains (e.g. `ur_robot_driver` ships a moveit_config but is a driver).
2. **`domain_rules`** — ordered; first match wins. A rule matches on repo-name
   regex, *any* package-name regex, or owner.
3. **`_incoming/`** — nothing matched. Review bucket; never guessed.

Repos already in `.gitmodules` are left exactly where they are — domains only
route *new* placements.

## Commands

```bash
python3 ros2pkg.py index                 # rebuild the lookup table
python3 ros2pkg.py classify humble --write        # domain breakdown + <distro>_domains.json
python3 ros2pkg.py classify humble --show-incoming # list unmatched repos to triage
python3 ros2pkg.py new humble            # repos NOT yet added as submodules (pending)
python3 ros2pkg.py new humble --new-only # only those that appeared since last refresh
python3 ros2pkg.py place slam_toolbox -d humble   # where would one repo go?
python3 ros2pkg.py diff humble --write adds.sh    # full backlog as add-commands
python3 ros2pkg.py add apriltag -d humble         # print the add command (dry-run)
python3 ros2pkg.py add apriltag -d humble --yes   # actually run git submodule add
python3 ros2pkg.py add --all -d humble --yes      # add every pending classified repo
python3 ros2pkg.py add --all -d humble --yes --retry-default-branch  # + salvage bad-branch repos
python3 ros2pkg.py sync --from humble             # repos in one distro missing from others
```

Nothing writes to your tree unless you pass `--yes` to `add`.

**Broken packages are never half-added.** If a `git submodule add` fails (branch
missing, repo 404/moved), the tool auto-cleans any partial state (orphan dir,
`.git/modules` entry, stray `.gitmodules` section) so the tree stays consistent,
and records the repo in `manifests/<distro>_failed.json`. With
`--retry-default-branch`, a branch-not-found failure is retried on the repo's
actual default branch and reported as "salvaged" (review the branch — it may not
match the distro; the tolerant `-new` CI build will confirm whether it compiles).

## Adding a package (the loop)

1. `./refresh.sh` (or scheduled) reports what's pending / newly appeared.
2. If a new repo landed in `_incoming/`, add a rule or an `overrides` entry.
3. `python3 ros2pkg.py add <repo> -d <distro> --yes` when ready.
4. Next refresh: it's in `.gitmodules`, so it drops off the pending list.

## Tuning classification

Edit `ros2pkg.config.json` only:

- **Wrong domain for a whole owner/pattern** → adjust that domain's rule.
- **One repo misfiled** (spans domains) → add it to `overrides`.
- **New capability area** → add a `domains` entry + a rule (e.g. `aerial`,
  `autonomous_driving` were added this way).

Re-run `classify` to see the effect instantly — no code changes.

## Scheduling

Point cron or launchd at `refresh.sh`. Example (weekly, Monday 9am) via cron:

```
0 9 * * 1 /Users/dhruvpatel29/humble-ros2/src/scripts/refresh.sh
```

`refresh.sh` only *reports*; it never adds submodules. Review the pending list,
then add what you want by hand.
