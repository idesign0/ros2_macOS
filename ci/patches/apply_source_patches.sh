#!/bin/bash
# Build-time source patches for submodule packages — applied in the composite
# setup AFTER submodule checkout, BEFORE colcon build. No forks needed; edits
# the submodule working tree in place. Idempotent (safe to re-run per job).
#
# Only put CLEAN, verified transforms here. Packages needing a real port
# (io_service::work -> executor_work_guard, resolver::iterator -> range resolve,
# etc.) get their own reviewed patch, not a blind sed.
set -u
ROOT="${1:-.}"

# BSD (macOS) vs GNU sed in-place flag
if sed --version >/dev/null 2>&1; then SEDI=(-i); else SEDI=(-i ''); fi

_pkg_dir() {
  find "$ROOT" -type d -name "$1" -not -path '*/build/*' -not -path '*/install/*' 2>/dev/null | head -1
}

# --- Lane 2 (Boost 1.90): boost::asio::io_service was removed (== io_context).
#     Only for packages that use the bare TYPE (no ::work / resolver::iterator). ---
patch_io_service_typeonly() {
  local dir; dir="$(_pkg_dir "$1")"; [ -z "$dir" ] && { echo "skip $1 (absent)"; return 0; }
  # bail if this package also uses the removed ::work API — needs a real port
  if grep -rq 'io_service::work' "$dir" 2>/dev/null; then
    echo "skip $1 (uses io_service::work — needs full port, not this sed)"; return 0
  fi
  local files; files="$(grep -rl 'io_service' "$dir" \
      --include='*.h' --include='*.hpp' --include='*.ipp' --include='*.cpp' --include='*.cc' 2>/dev/null)"
  [ -z "$files" ] && return 0
  echo "$files" | while IFS= read -r f; do
    sed "${SEDI[@]}" 's/asio::io_service/asio::io_context/g' "$f"
  done
  echo "patched io_service -> io_context in $1 ($dir)"
}

patch_io_service_typeonly hls_lfcd_lds_driver
patch_io_service_typeonly libcreate
# nao_lola + nao_lola_client: bare `boost::asio::io_service` member only (no ::work,
# no deadline_timer, no resolver) — type-only rename is safe. The variable is also
# named `io_service`; only the `asio::io_service` TYPE token is rewritten.
patch_io_service_typeonly nao_lola
patch_io_service_typeonly nao_lola_client

# --- add an #include to a file if missing (idempotent). Inserts after the first
#     existing #include so it lands in the header block. ---
_add_include() {  # $1=file  $2='#include <x>'
  local f="$1" inc="$2"
  [ -f "$f" ] || return 0
  grep -qF "$inc" "$f" && return 0
  awk -v inc="$inc" 'BEGIN{d=0} /^[[:space:]]*#include/ && !d {print; print inc; d=1; next} {print} END{if(!d) print inc}' "$f" > "$f.__p" && mv "$f.__p" "$f"
  echo "  + $inc  ->  ${f#$ROOT/}"
}

# --- Lane 5: missing standard includes (clang/libc++ is stricter than libstdc++) ---
# rcdiscover: several headers use std::string with only <stdexcept> — add <string>
# to ALL of its headers that reference std::string (idempotent; skips ones with it)
d="$(_pkg_dir rcdiscover)"; [ -n "$d" ] && for f in $(grep -rl 'std::string' "$d" 2>/dev/null | grep '\.h$'); do _add_include "$f" '#include <string>'; done
# urg_node: urg_c_wrapper.cpp uses read()/write() (POSIX) undeclared without <unistd.h>
d="$(_pkg_dir urg_node)"; [ -n "$d" ] && for f in $(find "$d" -name urg_c_wrapper.cpp 2>/dev/null); do _add_include "$f" '#include <unistd.h>'; done
# swri_console_util: progress_bar.cpp calls select()/fd_set undeclared without <sys/select.h>
d="$(_pkg_dir swri_console_util)"; [ -n "$d" ] && for f in $(find "$d" -name progress_bar.cpp 2>/dev/null); do _add_include "$f" '#include <sys/select.h>'; done

# --- Lane 3: yaml_cpp_vendor consumers fail (find_package(yaml-cpp) not found ->
#     ld: -lyaml-cpp not found). Root cause: the vendor's *-extras.cmake.in sets
#     yaml-cpp_DIR to opt/yaml_cpp_vendor/SHARE/cmake/yaml-cpp, but yaml-cpp 0.8.0
#     installs its config to CMAKE_INSTALL_LIBDIR/cmake = opt/yaml_cpp_vendor/LIB/
#     cmake/yaml-cpp (matches cmake/toolchain.cmake's yaml-cpp_DIR). The extras'
#     plain set() shadows the toolchain cache value in the consumer scope. Fix path. ---
#     VERSION-SPECIFIC: only yaml-cpp 0.8.0 (jazzy/kilted) installs its cmake config
#     to CMAKE_INSTALL_LIBDIR/cmake = lib/. humble builds yaml-cpp 0.7.0, which installs
#     to CMAKE_INSTALL_DATADIR/cmake = share/ — there the extras `share` path is CORRECT,
#     so guard on VCS_VERSION 0.8.x and never touch the 0.7.0 (humble) tree.
d="$(_pkg_dir yaml_cpp_vendor)"
if [ -n "$d" ] && grep -qE 'VCS_VERSION[[:space:]]+0\.8' "$d/CMakeLists.txt" 2>/dev/null; then
  for f in $(find "$d" -name '*-extras.cmake.in' 2>/dev/null); do
    sed "${SEDI[@]}" 's#/share/cmake/yaml-cpp#/lib/cmake/yaml-cpp#g' "$f"
    echo "  yaml_cpp_vendor(0.8.0) extras: share->lib cmake dir: ${f#$ROOT/}"
  done
fi

# --- Lane 4 cluster: fmt >= 11 (brew ships fmt 12) moved fmt::format out of
#     <fmt/core.h> into <fmt/format.h>. Packages written for fmt <= 10 include
#     only core.h and fail with "no member named 'format' in namespace 'fmt'".
#     Add <fmt/format.h> to every file that uses fmt::format* with only core.h.
#     Clears control_toolbox (ros2_control root), pick_ik, moveit_task_constructor,
#     motion_capture_tracking, plotjuggler_ros, the autoware localization cluster, … ---
for f in $(grep -rlE '#include <fmt/core.h>' "$ROOT" \
    --include='*.hpp' --include='*.h' --include='*.cpp' --include='*.cc' 2>/dev/null \
    | grep -vE '/build/|/install/|/thirdparty/|/bundled/'); do
  if grep -qE 'fmt::(format|format_to|join|print)' "$f" && ! grep -qF '<fmt/format.h>' "$f"; then
    _add_include "$f" '#include <fmt/format.h>'
  fi
done

# --- Lane 2: naoqi_libqi — Boost split process into v1/v2; v1 header moved ---
d="$(_pkg_dir naoqi_libqi)"; [ -n "$d" ] && for f in $(grep -rl 'boost/process/search_path.hpp' "$d" 2>/dev/null); do
  sed "${SEDI[@]}" 's#boost/process/search_path.hpp#boost/process/v1/search_path.hpp#g' "$f"; echo "  naoqi_libqi process v1: ${f#$ROOT/}"
done

# --- Lane 4: rt_usb_9axisimu_driver — the out-of-line ctor definition carries a
#     default arg (`std::string port = ""`) while the header declares it `explicit`
#     with no default; ill-formed under clang. The only caller passes an arg. ---
d="$(_pkg_dir rt_usb_9axisimu_driver)"; [ -n "$d" ] && for f in $(grep -rl 'RtUsb9axisimuRosDriver(std::string port = ""' "$d" 2>/dev/null); do
  sed "${SEDI[@]}" 's/RtUsb9axisimuRosDriver(std::string port = "")/RtUsb9axisimuRosDriver(std::string port)/' "$f"
  echo "  rt_usb default-arg removed: ${f#$ROOT/}"
done

# --- Lane 2: sick_safetyscanners_base — full Boost-1.90 asio port.
#     io_service::work was removed; replace with executor_work_guard and build it
#     from the io_context's executor. Order matters: do ::work BEFORE the bare
#     type rename, and fix the construction to pass an executor. ---
d="$(_pkg_dir sick_safetyscanners_base)"
if [ -n "$d" ]; then
  files="$(grep -rl 'io_service' "$d" --include='*.h' --include='*.hpp' --include='*.cpp' --include='*.cc' 2>/dev/null)"
  echo "$files" | while IFS= read -r f; do
    [ -z "$f" ] && continue
    # 1) work type -> executor_work_guard  2) construct from executor  3) bare type rename
    sed "${SEDI[@]}" \
      -e 's#boost::asio::io_service::work#boost::asio::executor_work_guard<boost::asio::io_context::executor_type>#g' \
      -e 's#make_unique<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>>(m_io_service)#make_unique<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>>(m_io_service.get_executor())#g' \
      -e 's#boost::asio::io_service#boost::asio::io_context#g' \
      -e 's#asio::io_service#asio::io_context#g' \
      "$f"
  done
  echo "  sick_safetyscanners_base: io_service->io_context + work->executor_work_guard ($d)"
  # deadline_timer.hpp exists in vendored 1.89 but is no longer pulled in by the
  # <boost/asio.hpp> umbrella — include it explicitly where deadline_timer is used.
  for f in $(grep -rl 'boost::asio::deadline_timer' "$d" 2>/dev/null); do
    _add_include "$f" '#include <boost/asio/deadline_timer.hpp>'
  done
  # Boost 1.87 removed the deprecated static ip::address*::from_string(); the
  # replacement free functions ip::make_address*() exist since 1.66 (safe on old
  # Boost too). Applies to CommSettings.h and ConfigData.cpp.
  for f in $(grep -rlE 'ip::address(_v4|_v6)?::from_string' "$d" \
      --include='*.h' --include='*.hpp' --include='*.cpp' --include='*.cc' 2>/dev/null); do
    sed "${SEDI[@]}" \
      -e 's#ip::address_v4::from_string#ip::make_address_v4#g' \
      -e 's#ip::address_v6::from_string#ip::make_address_v6#g' \
      -e 's#ip::address::from_string#ip::make_address#g' \
      "$f"
    echo "  sick_safetyscanners_base: address_v4::from_string -> make_address_v4: ${f#$ROOT/}"
  done
fi

echo "source patches applied."
