# ci/deploy -- split Homebrew distribution

Tools and findings for shipping the built workspace as per-domain Homebrew formulae
instead of one 1.8 GB keg. Nothing here runs in CI yet; it is validated offline against
the artifacts of kilted run 40491414514.

## Layout

One tap, one formula per domain, plus base and a metaformula:

    idesign0/homebrew-ros2
      ros2-<distro>              metaformula: depends_on every domain (today's UX, full stack)
      ros2-<distro>-base         531 ament packages, 263 MB compressed
      ros2-<distro>-<domain>     21 of them; depends_on ros2-<distro>-base

ONE tap, not one per domain: formulae in a single tap update in a single commit, so the
set is always self-consistent. Across N repos there is no atomic update, so there would
always be a window where a domain wants a base its own tap has not published.

**The dependency graph is a star.** Every domain depends on base and on nothing else
(0 cross-domain edges, measured). So a failing domain blocks no other domain -- which is
what makes "stop that domain's release, ship the rest" actually work. The only coupling
left is base's ABI.

Verified reconstruction: union of parts = 1334 packages = the full tree exactly, nothing
missing and nothing invented.

## Why keg_only matters

The existing formula is deliberately `keg_only` and is used by sourcing `setup.zsh`, not
by linking (see its own comment: linking hands protoc/check_urdf/binvox2bt/gz and the
GeographicLib/benchmark headers to this keg -- idesign0/ros2_macOS#8). That is precisely
what makes splitting safe: nothing is symlinked into HOMEBREW_PREFIX, so separate kegs
can never collide on setup.bash, COLCON_IGNORE or ament index files.

Chaining is colcon's own mechanism and was tested end to end:

    COLCON_CURRENT_PREFIX=$BASE source $BASE/setup.zsh
    COLCON_CURRENT_PREFIX=$DOM  source $DOM/local_setup.zsh

`local_setup.sh` enumerates packages dynamically via `_local_setup_util_sh.py`, so each
keg discovers only its own. In a clean shell (`env -i ... zsh -f`) base + planning gave
AMENT_PREFIX_PATH with exactly 2 entries and `ros2 pkg list` = 733 = 531 + 202.

## Tools

### genformulae.py
Generates the formulae. Derives everything from the artifacts so the graph cannot drift
from what was built:
  * packages provided      <- ament index
  * cross-domain edges     <- package_run_dependencies, via a provider index
  * Homebrew deps          <- otool -L

Two lessons learned the hard way:
  * **otool is a FLOOR, not the list.** Against the curated 61 deps it found 42, missing
    26 real ones that are not link-time: ogre1.9/ogre2.3 (dlopen'd gz rendering),
    gpsd, gts, the X11 set, pinocchio. Union otool WITH the curated list; use otool as an
    auditor that flags undeclared linkage.
  * **Do not deny-list the formulae CI unlinks.** `brew unlink` removes symlinks from
    {bin,lib,include} but KEEPS /opt/homebrew/opt/<formula>, which is the path these
    binaries reference. They are genuine runtime deps and must be declared; what belongs
    in caveats is which ones a user may safely unlink.

### brewclosure.py
Computes the full Homebrew dylib closure of a built tree. Emits three things:
  * `formulae` -> the depends_on list
  * `dylibs`   -> exactly what a vendor dir must contain
  * `missing`  -> references that do not resolve. **In CI this must be empty; a non-empty
    missing list should fail the release.**

Measured on base + all 21 kilted domains (5923 mach-o files, converged in 5 rounds):

    one otool pass : 354 dylibs,  89 formulae, 446 MB
    full closure   : 478 dylibs, 172 formulae, 590 MB measured / ~687 MB expected in CI

A single pass under-counts badly: the closure adds the whole aws-sdk-cpp family (via
gdal/arrow), nss/nspr, poppler, krb5, gpgme, libheif. For `depends_on` use the ~89 direct
refs -- brew resolves transitive deps itself. For a vendor dir you need all 478, because
a vendored dylib with unvendored dependencies is useless.

## The bug this was built to prevent

The shipped `ros2-jazzy` keg cannot load Gazebo Sim:

    dlopen(libgz-sim8.8.14.0.dylib): Library not loaded:
      /opt/homebrew/opt/abseil/lib/libabsl_crc_cpu_detect.2601.0.0.dylib

236 dylibs and 5 executables (all gz-*, gzserver_component, protoc-32.0.0,
protoc-gen-upb*) reference `libabsl_crc_cpu_detect` and `libabsl_borrowed_fixup_buffer`,
which abseil DELETED between soname 2601 and 2608. The gz job's output was cached under a
key that tracked brew not at all, so gz and protobuf stayed on 2601 while the rest of the
workspace built against 2608. `post_install` could not repair it -- its `next unless
actual` finds no replacement to point at. `brewclosure.py` would have listed both in
`missing` and failed the release.

Fixed by adding brew-packages/matched_packages.txt to the gz cache key and by
.github/workflows/update-brew.yml (on the default branch) refreshing it weekly.

## Vendoring, and how to detect what needs it

Vendoring = ship our own copy of each brew dylib inside the keg and point the binaries at
it with @rpath, so the user's brew state is irrelevant. This REPLACES post_install's
stale-reference repair rather than notifying about it: there are no external references
left to repair. `package_tree.py` implements it and has been run end to end on the base
tree.

Simplest arrangement: ONE vendor dir in base, every binary in every domain rpath'd into
it. base links 47 formulae, the whole set links 92, and 10 of the 21 domains need nothing
beyond base, so per-domain vendor dirs would be twelve mechanisms for no gain.

MEASURED on the base tree (2239 mach-o files):

    vendored                478 dylibs, 743 MB   (base keg 1.6 GB -> ~2.3 GB)
    dylibs loading before   102 / 120 sampled
    dylibs loading after    106 / 120            <- MORE: some were already broken
    real regressions        0

Keep `depends_on` anyway: vendoring fixes runtime; anyone compiling against the keg still
needs headers and CMake configs from /opt/homebrew/opt/<f>/include.

### The trap: brew dylibs hide dependencies behind @rpath

**187 of 406 brew dylibs reference their SIBLINGS via @rpath/@loader_path**, resolved
against their own LC_RPATH (typically `@loader_path/../lib`, i.e. their own keg's lib dir):

    libgeos_c.1.dylib -> @rpath/libgeos.3.15.0.dylib,  LC_RPATH @loader_path/../lib

An absolute-path-only closure misses every one of them. Copy such a dylib into a flat
vendor dir and @loader_path now points somewhere that has none of them. The first
implementation did exactly this and **broke 20 of 102 dylibs that had loaded before**,
while reporting "0 remaining /opt/homebrew/opt references" -- a check that was true and
useless simultaneously. Fixing the closure to resolve @rpath siblings took the vendor set
from 362 dylibs/486 MB to 478/743 MB and the regressions from 20 to 0.

Second trap, same run: never rewrite a reference you could not vendor. Doing so converts a
working absolute path into a dangling @rpath -- actively worse than leaving it.

### Vendoring must be COMPLETE or it can be worse than none

Libraries with process-global state -- gflags, glog, protobuf, abseil -- abort or
misbehave when two instances coexist in one process:

    ERROR: something wrong with flag 'flagfile' ... being linked both statically and
    dynamically into this executable

That happens whenever a process holds both the vendored copy and the brew copy. Real
routes to it: a user's app links brew gflags and also loads this keg; or a brew-RESIDENT
plugin is dlopen'd and drags brew's copies of shared deps in alongside ours. The keg ships
no libqcocoa, so a GUI tool loads Qt's platform plugin from brew today -- exactly that
situation.

### Detection needs BOTH passes, and the gate must be LOAD SUCCESS

  1. static  -- brewclosure.py / package_tree.py's closure. Catches everything linked and
     everything dlopen'd from INSIDE the keg: the 699 gz engine-plugins and 208 pluginlib
     plugins ship as files, so otool sees them. Must resolve @rpath siblings (see above).
  2. runtime -- `DYLD_PRINT_LIBRARIES=1`, filtered to /opt/homebrew. dyld prints **Cellar**
     paths, not opt, because it resolves the symlink -- match both forms.

Loading ONE library (librviz_common) under DYLD showed 20 brew formulae, of which **llvm
and md4c appear in neither the static closure nor the curated list** (mesa dlopens llvm for
software GL). So runtime finds what static cannot see, and static finds what a given run
did not exercise. Union them.

Runtime coverage equals what you actually run: no Qt plugin appeared headless, because
plugins load only when a GUI initialises. `QT_QPA_PLATFORM=offscreen` makes Qt load a
platform plugin headlessly and `gz sim -s` exercises the server path -- use those so the
smoke job reaches the plugin loaders.

**Do not gate on "zero brew references".** It passed on a tree with 20 broken libraries.
Gate on dlopen success, measured before and after.

### A missing closure reference has two causes, and only one is a bug

Measured 2026-10-29 by the first `package-smoke-kilted` run, against an `install-full-kilted`
artifact built **two days earlier** on `kilted-new`:

| | |
|---|---|
| closure | 739 dylibs, 737 present, **2 missing** |
| missing | `draco/libdraco.9.dylib`, `librealsense/librealsense2.2.58.dylib` |
| why | core moved to `draco 1.5.9` / `librealsense 2.59.1` in those two days |
| sample load test, before packaging | 123 of 150 loaded |

So abseil-2601 was not an unlucky one-off. Two sonames moved under a tree in 48 hours;
every tarball decays from the hour it is built, and `matched_packages.txt` never protected
against this because it is an install LIST, not a constraint. That is the argument for
vendoring, and it is also why the input to a packaging run has to be fresh.

The drift is not only between a build and a later packaging run -- it happens WITHIN one
run, between its own jobs. Measured 2026-10-30 on humble run 41083360164: `gz` and `base-1`
disagreed on **91 of 379** formulae, including qtbase 6.12.0_2 vs 6.11.2_1 (plus 35 other
`qt*`), dartsim 6.20.3 vs 6.20.0 and boost 1.92.0_1 vs 1.92.0. Jazzy run 41083372266 showed
77. Both runs died in `base-1` at ~75 minutes.

Two facts from that measurement decide the fix:

* The pool is **heterogeneous at a single instant.** The `gz` jobs of the humble and jazzy
  runs were launched in the same second and still disagreed on 12 of those formulae, so this
  is not a rotation that can be waited out and a plain re-run is not reliably self-consistent.
* **70 of the 91** drifted formulae were poured by the job itself (tap resolution), but **21
  were not** -- openssl@3, harfbuzz, freetype, sqlite, python@3.14, ca-certificates and
  friends ship pre-installed in the runner image, and `install_brew_packages.sh` skips
  anything already present regardless of version.

So prevention is two-part, both in `setup-ros2-macos`:

| mechanism | covers | on failure |
|---|---|---|
| `brew-core-sha` -- `plan` resolves one homebrew-core commit per run via `git ls-remote`; every macOS job fetches that SHA (depth 1) and detaches its tap onto it | the 70 the job installs | warns, runs unpinned |
| reconcile -- after pinning, any of `abseil protobuf qt@5 qtbase boost openssl@3 opencv@4 dartsim suite-sparse` already installed at a version the pinned tap disagrees with is reinstalled from it | the 21 image preinstalls, incl. openssl@3 | warns, leaves the image version |

Downgrading is safe: every superseded bottle revision is retained in ghcr (checked
2026-10-30 -- qtbase 6.11.2_1/6.12.0_2, openssl@3 3.5.9/3.6.5, dartsim 6.20.0/6.20.3 and
boost 1.92.0/1.92.0_1 all return HTTP 200). The reconcile list is deliberately just the
load-bearing set the skew check gates on; a full reconcile would reinstall ~300 formulae in
each of 24 jobs. Pinning is prevention and the `brew-state-run` diff is detection -- keep
both, because the pin is best-effort by design and must never kill a five-hour run.

`package_tree.py` therefore classifies each missing reference:

| class | test | meaning | flag that tolerates it |
|---|---|---|---|
| DRIFTED | `/opt/homebrew/opt/<formula>` exists, the file does not | installed at a version that dropped that soname -- Homebrew moved after the build. The INPUT is stale; the closure walk is fine | `--tolerate-stale-input` |
| RETIRED | keg absent, and `brew info` says `disabled`, or brew does not know the name | Homebrew has withdrawn the formula. Still drift, just terminal: it WAS installable at build time and never will be again | `--tolerate-stale-input` |
| ABSENT | keg absent, formula still installable | never installed here -- an undeclared dependency, or a closure walk that invented a path. `brew unlink` keeps the `opt` symlink, so an absent dir is conclusive | `--allow-missing` only (local experiments) |

RETIRED is not a hypothetical third case, it is how the second smoke run (41046526636)
died -- not in packaging but in `setup-ros2-macos`, 65 minutes in:

```
icu4c@77 has been disabled because it is a versioned formula!
::error::brew: 1 formula(e) failed to install and are not in allow_install_failures.txt: icu4c@77
```

`disable_date: 2026-10-30`, the morning of that run. Homebrew writes `disable!` into the
formula with a FUTURE date, so the trap is committed to core months ahead and springs on
a calendar day with no change on our side. Nothing depended on `icu4c@77` (checked every
`dependencies` list in `matched_packages.txt`) and `icu4c@78` was already being installed
as a `boost@1.85` dependency, so the line was simply dropped.

Auditing the whole list the same way, `brew info --json=v2 $(awk '{print $1}' matched_packages.txt)`:

| state | formulae |
|---|---|
| disabled | `boost@1.85` (2026-04-05), `icu4c@77` (2026-10-30), `tinyxml1` (2026-10-26, tapped as `osrf/simulation/tinyxml1`, already in `allow_install_failures.txt`), `websocketpp` (2026-04-10) |
| deprecated, still installable | `qt@5` and `pyqt@5`/`qwt-qt5` (2026-05-19), `opencv@4` (2026-10-09), `python@3.10` (2026-10-01), `lua@5.4`, `assimp@5`, `freeimage`, `postgresql@14`, `icu4c@78` (2026-10-30), `llvm@14`, `pcre` |

`boost@1.85` and `websocketpp` read as disabled here yet still install on the runner, so
they are left alone until a run actually rejects them -- `allow_install_failures.txt` is
evidence-driven by design and this is not evidence. The deprecation list is the one worth
watching: Homebrew disables roughly a year after deprecating, which puts **`qt@5` around
2027-05-19**, and the whole base stack is Qt5.

Who passes what:

| caller | flags | rationale |
|---|---|---|
| `package` job in `macos-<distro>-layered.yml` | none | runs in the same run as the build, so a stale input is a real failure worth stopping on |
| `package-smoke-<distro>.yml` | `--tolerate-stale-input` | its subject is the PACKAGER, not the tree. A stale artifact must not stop it from answering whether anything regressed. Consumers of an un-vendorable lib could not load before packaging either, so before/after stays comparable |
| local | `--allow-missing` | a dev box legitimately lacks formulae (no `rtabmap` -> no `librealsense`) |

A tolerated reference is deliberately **not rewritten** -- rewriting a path that cannot be
vendored turns a resolvable absolute path into a dangling `@rpath` -- so the final verify
subtracts exactly that set and fails on anything else.

## post_install is deprecated -- move it into CI packaging

Homebrew 7 raises `odeprecated "post_install", "post_install_steps"`, and
`post_install_steps` is a DECLARATIVE DSL with a closed set of built-in actions
(configure_gcc_runtime, install_gzipped_executable, bootstrap_cpython, ...). There is no
"run arbitrary Ruby" step, and zero homebrew-core formulae still define post_install. So
this is a withdrawn capability, not a rename.

What the current post_install does, and where each part belongs:

| job | nature | move to |
|---|---|---|
| absolute vendor RPATHs for dlopen under Hardened Runtime | pure function of the tree | CI packaging -- do it once, ship a correct tree |
| stale versioned dylib refs -> install_name_tool + codesign | depends on the USER's brew | obviated by vendoring; until then, a package-time CHECK that fails the release |

Doing the rpath work in CI also removes an otool walk over ~100k files from every user's
machine on every upgrade (ros-visualization alone is 30502 files, ros-drivers 21588,
ros-planning 17840).

Note the stale-ref repair rewrites a reference to a DIFFERENT soname, which assumes an ABI
compatibility nothing verified -- and it silently hid a tree that cannot load. Prefer
failing the package step.

## Open items

* decide vendoring yes/no: measured 743 MB, 0 regressions on the base tree
* have `base` upload its `brew-state.txt` and every later job diff against it: `package` is a separate job on a fresh runner hours after `assemble`, so core can move mid-run and the strict gate would then reject a tree that was correct when built
* fix the under-declared depends_on (ros2-kilted.rb has 57; direct refs are ~89)
* the DYLD smoke job (QT_QPA_PLATFORM=offscreen, gz sim -s)
* `data_tamer_msgs` is in two shards (dom_moveit + dom_ros-visualization), so two kegs
  would ship it; give it one owner
* `osrf` and `eclipse-iceoryx` deltas have zero ament packages -- they should not become
  formulae as-is
* third-party taps need `brew trust <tap>` on Homebrew 7; the install docs must say so
