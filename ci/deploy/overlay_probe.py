#!/usr/bin/env python3
"""
Can an overlay workspace build against this tree? For every package that ships a CMake
config, run find_package() from a fresh overlay project with only the tree sourced, and
check what it hands the overlay: every include dir, library and imported file must exist
and must live in the tree (or the system / python.org SDK), never in the build machine's
checkout or a Homebrew keg the tree does not ship.

Packages are probed in batches (one CMake run each, every package in its own
subdirectory so their imported targets do not collide); a failing batch is split until
the failing package is isolated. ~1500 packages take a few minutes.

  overlay_probe.py <prefix> [--jobs N] [--allow FILE] [--out DIR]

Exit 1 if any package fails and is not listed in --allow (one name per line, # comments).
"""
import argparse, concurrent.futures as cf, os, re, shutil, subprocess, sys, tempfile

PROBE = r"""
cmake_minimum_required(VERSION 3.22)
project(overlay_probe_@N@ C CXX)
set(_keg "$ENV{PROBE_PREFIX}")
set(_kegreal "$ENV{PROBE_PREFIX_REAL}")
set(_devdir "$ENV{PROBE_DEVDIR}")
string(REGEX REPLACE "/+$" "" _sdk "${CMAKE_OSX_SYSROOT}")
function(_probe_check pkg)
  # What an overlay that uses <pkg> compiles and links against: the package's own
  # targets/variables and, transitively, every target their link interface names. Targets
  # a dependency merely DEFINES (all 170 VTK modules) are not checked -- only what is used.
  set(_bad "")
  set(_vals "${${pkg}_INCLUDE_DIRS};${${pkg}_LIBRARIES}")
  get_property(_tg DIRECTORY PROPERTY IMPORTED_TARGETS)
  set(_todo "")
  foreach(_t ${_tg})
    if(_t MATCHES "^${pkg}::" OR _t STREQUAL "${pkg}")
      list(APPEND _todo ${_t})
    endif()
  endforeach()
  foreach(_x ${${pkg}_LIBRARIES} ${${pkg}_TARGETS})
    if(TARGET "${_x}")
      list(APPEND _todo ${_x})
    endif()
  endforeach()
  set(_seen "")
  while(_todo)
    list(POP_FRONT _todo _t)
    if(_t IN_LIST _seen OR NOT TARGET "${_t}")
      continue()
    endif()
    list(APPEND _seen ${_t})
    get_target_property(_al ${_t} ALIASED_TARGET)
    if(_al)
      list(APPEND _todo ${_al})
    endif()
    foreach(_p INTERFACE_INCLUDE_DIRECTORIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES
               INTERFACE_LINK_DIRECTORIES IMPORTED_LOCATION IMPORTED_LOCATION_RELEASE
               IMPORTED_LOCATION_NONE IMPORTED_LOCATION_NOCONFIG)
      get_target_property(_v ${_t} ${_p})
      if(_v)
        list(APPEND _vals ${_v})
      endif()
    endforeach()
    get_target_property(_ll ${_t} INTERFACE_LINK_LIBRARIES)
    foreach(_l ${_ll})
      string(REGEX REPLACE "^\\$<LINK_ONLY:(.*)>$" "\\1" _l "${_l}")
      if(TARGET "${_l}")
        list(APPEND _todo ${_l})
      else()
        list(APPEND _vals ${_l})
      endif()
    endforeach()
  endwhile()
  foreach(_x ${_vals})
    if(_x MATCHES "^\\$<" OR _x MATCHES "^-" OR NOT IS_ABSOLUTE "${_x}")
      continue()
    endif()
    # the tree's own real location and the SDK this configure picked are fine wherever
    # they are (on CI both live under /Users/runner and /Applications/Xcode_*)
    if(_x MATCHES "^${_kegreal}/" OR (CMAKE_OSX_SYSROOT AND _x MATCHES "^${_sdk}")
       OR (_devdir AND _x MATCHES "^${_devdir}/"))
      continue()
    endif()
    if(_x MATCHES "^/Users/runner/" OR _x MATCHES "^/Applications/Xcode_")
      list(APPEND _bad "BUILDMACHINE ${_x}")
    elseif(NOT EXISTS "${_x}")
      list(APPEND _bad "MISSING ${_x}")
    elseif(_x MATCHES "^/opt/homebrew/" AND NOT _x MATCHES "^${_keg}" AND NOT _x MATCHES "^${_kegreal}")
      list(APPEND _bad "HOMEBREW ${_x}")
    endif()
  endforeach()
  list(REMOVE_DUPLICATES _bad)
  foreach(_b ${_bad})
    message(STATUS "PROBE|${pkg}|${_b}")
  endforeach()
  message(STATUS "PROBE|${pkg}|OK")
endfunction()
@SUBDIRS@
"""
SUB = "find_package(ament_cmake QUIET)\nfind_package(@P@ REQUIRED)\n_probe_check(@P@)\n"


def packages(prefix):
    out = set()
    share = os.path.join(prefix, "share")
    for p in os.listdir(share):
        d = os.path.join(share, p, "cmake")
        if os.path.isfile(os.path.join(d, f"{p}Config.cmake")) or \
                os.path.isfile(os.path.join(d, f"{p}-config.cmake")):
            out.add(p)
    # opt/<vendor> prefixes count only where the tree's own hooks put them on
    # CMAKE_PREFIX_PATH (gz vendors do; zenoh_cpp_vendor keeps zenohc to itself)
    cpp = {os.path.realpath(x) for x in os.environ.get("CMAKE_PREFIX_PATH", "").split(":") if x}
    opts = [o for o in (os.listdir(os.path.join(prefix, "opt"))
                        if os.path.isdir(os.path.join(prefix, "opt")) else [])
            if os.path.realpath(os.path.join(prefix, "opt", o)) in cpp]
    for base in [os.path.join(prefix, "lib", "cmake")] + [
            os.path.join(prefix, "opt", o, "lib", "cmake") for o in opts]:
        if not os.path.isdir(base):
            continue
        for p in os.listdir(base):
            d = os.path.join(base, p)
            if os.path.isfile(os.path.join(d, f"{p}Config.cmake")) or \
                    os.path.isfile(os.path.join(d, f"{p}-config.cmake")):
                out.add(p)
    return sorted(out)


def run_batch(pkgs, work, env):
    d = tempfile.mkdtemp(dir=work)
    subs = []
    for i, p in enumerate(pkgs):
        os.makedirs(os.path.join(d, f"p{i}"))
        open(os.path.join(d, f"p{i}", "CMakeLists.txt"), "w").write(SUB.replace("@P@", p))
        subs.append(f"add_subdirectory(p{i})")
    open(os.path.join(d, "CMakeLists.txt"), "w").write(
        PROBE.replace("@N@", "x").replace("@SUBDIRS@", "\n".join(subs)))
    r = subprocess.run(["cmake", "-S", d, "-B", os.path.join(d, "b"), "-DCMAKE_BUILD_TYPE=Release"],
                       capture_output=True, text=True, env=env)
    shutil.rmtree(d, ignore_errors=True)
    return r.returncode, r.stdout + r.stderr


def probe(pkgs, work, env, jobs):
    """-> {pkg: (ok, [problems], first error line)}"""
    res = {}
    todo = [pkgs[i:i + 25] for i in range(0, len(pkgs), 25)]
    with cf.ThreadPoolExecutor(jobs) as ex:
        while todo:
            futs = {ex.submit(run_batch, b, work, env): b for b in todo}
            todo = []
            for f in cf.as_completed(futs):
                b = futs[f]
                rc, out = f.result()
                if rc == 0 or len(b) == 1:
                    probs = {}
                    for m in re.finditer(r"PROBE\|([^|]+)\|(.+)", out):
                        if m.group(2) != "OK":
                            probs.setdefault(m.group(1), []).append(m.group(2).strip())
                    for p in b:
                        if rc == 0:
                            res[p] = (not probs.get(p), probs.get(p, []), "")
                        else:
                            err = re.search(r"CMake Error[^\n]*\n((?:.*\n){1,4})", out)
                            res[p] = (False, [], " ".join((err.group(0) if err else out[-300:])
                                                          .split())[:300])
                else:
                    h = len(b) // 2
                    todo += [b[:h], b[h:]]
    return res


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("prefix")
    ap.add_argument("--jobs", type=int, default=os.cpu_count() or 4)
    ap.add_argument("--allow", help="file of package names allowed to fail")
    ap.add_argument("--out", default=None)
    a = ap.parse_args()
    prefix = a.prefix.rstrip("/")
    real = os.path.realpath(prefix)
    pkgs = packages(real)
    devdir = subprocess.run(["xcode-select", "-p"], capture_output=True, text=True).stdout.strip()
    # /Applications/Xcode_26.6.app/Contents/Developer -> the .app; CommandLineTools as is
    if devdir.endswith("/Contents/Developer"):
        devdir = devdir[:-len("/Contents/Developer")]
    env = dict(os.environ, PROBE_PREFIX=prefix, PROBE_PREFIX_REAL=real, PROBE_DEVDIR=devdir)
    work = tempfile.mkdtemp(prefix="overlay_probe.")
    res = probe(pkgs, work, env, a.jobs)
    shutil.rmtree(work, ignore_errors=True)
    allow = set()
    if a.allow and os.path.exists(a.allow):
        allow = {l.split("#")[0].strip() for l in open(a.allow)} - {""}
    fail = {p: r for p, r in res.items() if not r[0]}
    hard = {p: r for p, r in fail.items() if r[2]}
    soft = {p: r for p, r in fail.items() if not r[2]}
    print(f"overlay probe: {len(pkgs)} packages, {len(pkgs) - len(fail)} clean, "
          f"{len(hard)} find_package() errors, {len(soft)} with paths outside the tree")
    kinds = {}
    for p, (_, probs, err) in sorted(fail.items()):
        tag = " (allowed)" if p in allow else ""
        if err:
            print(f"  FAIL {p}{tag}: {err[:220]}")
        for x in probs[:3]:
            print(f"  PATH {p}{tag}: {x}")
            kinds[x.split()[0]] = kinds.get(x.split()[0], 0) + 1
    if a.out:
        os.makedirs(a.out, exist_ok=True)
        with open(os.path.join(a.out, "overlay_probe.tsv"), "w") as fh:
            for p, (ok, probs, err) in sorted(res.items()):
                fh.write(f"{p}\t{'ok' if ok else 'fail'}\t{err}\t{' | '.join(probs)}\n")
    bad = [p for p in fail if p not in allow]
    if bad:
        print(f"::error::{len(bad)} package(s) cannot be used from an overlay: "
              + ", ".join(sorted(bad)[:20]))
        sys.exit(1)


if __name__ == "__main__":
    main()
