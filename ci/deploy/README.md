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
left to repair.

Simplest arrangement: ONE vendor dir in base, every binary in every domain rpath'd into
it. base links 47 formulae, the whole set links 92, and 10 of the 21 domains need nothing
beyond base -- so per-domain vendor dirs would be twelve mechanisms for no gain.

Cost: ~687 MB, taking base from 1.6 GB to ~2.3 GB (+43%). Keep `depends_on` anyway --
vendoring fixes runtime; anyone compiling against the keg still needs headers and CMake
configs from /opt/homebrew/opt/<f>/include.

Detection needs BOTH passes, and that is measurable rather than theoretical:

  1. static  -- brewclosure.py. Catches everything linked, and everything dlopen'd from
     INSIDE the keg: the 699 gz engine-plugins and 208 pluginlib plugins ship as files, so
     otool sees them.
  2. runtime -- `DYLD_PRINT_LIBRARIES=1`, filtered to /opt/homebrew. Note dyld prints
     **Cellar** paths, not opt, because it resolves the symlink -- match both forms.

Loading one library (librviz_common) under DYLD showed 20 brew formulae, of which **llvm
and md4c appear in neither the static closure nor the curated list** (mesa dlopens llvm
for software GL). So runtime finds what static cannot see, and static finds what a given
run did not exercise. Union them.

Runtime coverage equals what you actually run: no Qt plugin appeared in a headless test,
because plugins load only when a GUI initialises. `QT_QPA_PLATFORM=offscreen` makes Qt
load a platform plugin headlessly, and `gz sim -s` exercises the server path -- use those
so the smoke job reaches the plugin loaders. The keg currently ships no libqcocoa, so a
GUI tool today loads Qt's platform plugin out of brew.

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

* decide vendoring yes/no with the ~687 MB number in hand
* wire brewclosure.py into packaging with `missing` non-empty = fail
* fix the under-declared depends_on (ros2-kilted.rb has 57; direct refs are ~89)
* the DYLD smoke job (QT_QPA_PLATFORM=offscreen, gz sim -s)
* `data_tamer_msgs` is in two shards (dom_moveit + dom_ros-visualization), so two kegs
  would ship it; give it one owner
* `osrf` and `eclipse-iceoryx` deltas have zero ament packages -- they should not become
  formulae as-is
* third-party taps need `brew trust <tap>` on Homebrew 7; the install docs must say so
