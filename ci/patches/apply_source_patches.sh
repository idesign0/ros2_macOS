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
# nebula_velodyne_hw_interfaces: velodyne_hw_interface.cpp uses boost::format without <boost/format.hpp>
d="$(_pkg_dir nebula_velodyne_hw_interfaces)"; [ -n "$d" ] && for f in $(find "$d" -name velodyne_hw_interface.cpp 2>/dev/null); do _add_include "$f" '#include <boost/format.hpp>'; done
# event_camera_tools: trigger_delay.cpp uses getopt/optarg/optind undeclared without <unistd.h>
d="$(_pkg_dir event_camera_tools)"; [ -n "$d" ] && for f in $(find "$d" -name trigger_delay.cpp 2>/dev/null); do _add_include "$f" '#include <unistd.h>'; done
# ros2_ouster: declare_parameter(name) with no default/type was removed from rclcpp; the
# no-default calls (lidar_ip, computer_ip) need an explicit type. Add rclcpp::ParameterType.
# NOTE: PR-material (real rclcpp API-drift fix) — fork ros2_ouster_drivers when opening PRs.
d="$(_pkg_dir ros2_ouster)"
if [ -n "$d" ] && [ -f "$d/src/ouster_driver.cpp" ]; then
  perl -0pi -e 's/declare_parameter\(("(?:lidar_ip|computer_ip)")\);/declare_parameter($1, rclcpp::ParameterType::PARAMETER_STRING);/g' "$d/src/ouster_driver.cpp"
  echo "  ros2_ouster: declare_parameter(name) -> +rclcpp::ParameterType::PARAMETER_STRING"
fi

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
  # Boost removed address_v4::to_ulong() (deprecated 1.71, gone 1.87). The only
  # use is on an ip::address_v4 (ChangeCommSettingsCommand.cpp) -> to_uint().
  for f in $(grep -rlE '\.to_ulong\(\)' "$d" --include='*.cpp' --include='*.cc' 2>/dev/null); do
    sed "${SEDI[@]}" 's#\.to_ulong()#.to_uint()#g' "$f"
    echo "  sick_safetyscanners_base: to_ulong -> to_uint: ${f#$ROOT/}"
  done
fi

# --- Lane 4/6: rc_dynamics_api forces C++11, but its abseil dependency requires
#     C++17 (`C++ versions less than C++17 are not supported`). Bump to 17. ---
d="$(_pkg_dir rc_dynamics_api)"; [ -n "$d" ] && for f in $(find "$d" -name CMakeLists.txt 2>/dev/null); do
  sed "${SEDI[@]}" -e 's/set(CMAKE_CXX_STANDARD 11)/set(CMAKE_CXX_STANDARD 17)/' -e 's/-std=c++11/-std=c++17/g' "$f"
  echo "  rc_dynamics_api: C++11 -> C++17: ${f#$ROOT/}"
done

# --- Lane 6: hash_library_vendor FetchContent's stbrumme/hash-library, whose
#     crc32.cpp includes <endian.h> (glibc-only; absent on macOS) and is compiled
#     by the vendor with -Werror. (1) drop -Werror on third-party source; (2) shim
#     <endian.h> -> <machine/endian.h> for this target on APPLE. The include is only
#     used to optionally define __BYTE_ORDER; arm64 is little-endian so the guarded
#     big-endian path stays off. ---
d="$(_pkg_dir hash_library_vendor)"
if [ -n "$d" ] && [ -f "$d/CMakeLists.txt" ]; then
  sed "${SEDI[@]}" 's/ -Werror//g' "$d/CMakeLists.txt"
  if ! grep -q 'MACOS_ENDIAN_SHIM' "$d/CMakeLists.txt"; then
    cat >> "$d/CMakeLists.txt" <<'EOF2'

# MACOS_ENDIAN_SHIM (ci patch): macOS has no <endian.h>; shim to <machine/endian.h>.
if(APPLE)
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/compat/endian.h" "#pragma once\n#include <machine/endian.h>\n")
  target_include_directories(hash_library_vendor BEFORE PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/compat")
endif()
EOF2
    echo "  hash_library_vendor: -Werror dropped + <endian.h> shim added"
  fi
fi

# --- Lane 6: qpoases_vendor ExternalProject uses the dead coin-or SVN
#     (svn E170013). Repoint to the GitHub mirror at the same 3.2 release. ---
d="$(_pkg_dir qpoases_vendor)"
if [ -n "$d" ] && [ -f "$d/CMakeLists.txt" ]; then
  sed "${SEDI[@]}" \
    -e 's#SVN_REPOSITORY https://projects.coin-or.org/svn/qpOASES/stable/3.2#GIT_REPOSITORY https://github.com/coin-or/qpOASES.git#' \
    -e 's#SVN_TRUST_CERT TRUE#GIT_TAG releases/3.2.2#' \
    "$d/CMakeLists.txt"
  echo "  qpoases_vendor: dead coin-or SVN -> github releases/3.2.2"
fi

# --- Lane 4: nebula_core_common/util/errno.hpp assumes the GNU strerror_r
#     (returns char*); macOS/BSD use the XSI form (returns int, fills the buffer).
#     Guard both. Clears the nebula errno cluster (decoders, hw_interfaces, …). ---
for f in $(find "$ROOT" -path '*nebula_core_common*util/errno.hpp' -not -path '*/build/*' 2>/dev/null); do
  perl -0pi -e 's{std::string_view msg = strerror_r\(err_no, msg_buf\.data\(\), msg_buf\.size\(\)\);}{#if defined(__GLIBC__) \&\& defined(_GNU_SOURCE)\n  std::string_view msg = strerror_r(err_no, msg_buf.data(), msg_buf.size());\n#else\n  strerror_r(err_no, msg_buf.data(), msg_buf.size());  /* XSI/macOS: int, fills buf */\n  std::string_view msg = msg_buf.data();\n#endif}' "$f"
  echo "  nebula errno.hpp: XSI strerror_r guard: ${f#$ROOT/}"
done

# --- Lane 7-guard: nebula udp.hpp uses SO_RXQ_OVFL (Linux-only rx-overflow
#     counter). Guard the setsockopt and the cmsg case so macOS just skips the
#     drop-reporting feature instead of failing to compile. ---
for f in $(find "$ROOT" -path '*nebula_core_hw_interfaces*connections/udp.hpp' -not -path '*/build/*' 2>/dev/null); do
  perl -0pi -e 's{(\n[ \t]*sock_fd_\.setsockopt\(SOL_SOCKET, SO_RXQ_OVFL, 1\)\.value_or_throw\(\);)}{\n#ifdef SO_RXQ_OVFL$1\n#endif}' "$f"
  perl -0pi -e 's!(case SO_RXQ_OVFL: \{[^}]*\})!#ifdef SO_RXQ_OVFL\n        $1\n#endif!s' "$f"
  echo "  nebula udp.hpp: guarded SO_RXQ_OVFL: ${f#$ROOT/}"
done

# --- Lane 6: lely_core_libraries (ros2_canopen) builds lely-core via autotools
#     with -Werror; its libc/time.h shim redefines CLOCK_MONOTONIC (already defined
#     on macOS) -> -Wmacro-redefined error. Pass CFLAGS to suppress that warning. ---
d="$(_pkg_dir lely_core_libraries)"
if [ -n "$d" ] && [ -f "$d/CMakeLists.txt" ] && ! grep -q 'Wno-macro-redefined' "$d/CMakeLists.txt"; then
  sed "${SEDI[@]}" 's#<SOURCE_DIR>/configure --prefix#<SOURCE_DIR>/configure "CFLAGS=-O2 -Wno-macro-redefined" --prefix#' "$d/CMakeLists.txt"
  echo "  lely_core_libraries: configure CFLAGS -Wno-macro-redefined"
fi

# --- Lane 2: boost-python component version. mrt_cmake_modules FindBoostPython
#     derives the component from find_package(Python3), which resolves the runner's
#     newest Python (3.14) -> boost_python314, absent from the vendored boost-1.89
#     (ships python311/313, built against the toolchain Python 3.11). Pin to 311 when
#     the vendored 311 lib is present. Fixes lanelet2_python, libfranka, etc. ---
d="$(_pkg_dir mrt_cmake_modules)"
if [ -n "$d" ] && [ -f "$d/cmake/Modules/FindBoostPython.cmake" ] && ! grep -q 'ci-pin-python311' "$d/cmake/Modules/FindBoostPython.cmake"; then
  perl -0pi -e 's{(\n[ \t]*find_package\(Boost COMPONENTS python\$\{_python_version\} numpy\$\{_python_version\}\))}{\n    # ci-pin-python311: vendored boost-1.89 ships python311/313 (built vs Python 3.11);\n    # find_package(Python3) may resolve 3.14 -> boost_python314 which does not exist.\n    if(DEFINED BOOST_LIBRARYDIR AND EXISTS "\$\{BOOST_LIBRARYDIR\}/libboost_python311.dylib")\n      set(_python_version 311)\n    endif()$1}' "$d/cmake/Modules/FindBoostPython.cmake"
  echo "  mrt_cmake_modules FindBoostPython: pin python component -> 311 (vendored boost)"
fi

# --- Lane 2: websocketpp 0.8.2 (brew, header-only) vs Boost 1.87+ removed APIs.
#     rmf_websocket includes brew websocketpp, whose asio transport uses a pile of
#     APIs deleted in Boost 1.87: io_service, io_service::strand, strand::wrap,
#     io_context::post/reset, io_service::work, socket_base::max_connections,
#     basic_waitable_timer::expires_from_now, and resolver::iterator/query.
#     Patch the installed brew headers in place (ephemeral runner; header-only).
#     VERIFIED: patched tree compiles clean vs Boost 1.89 (server + client connect
#     paths, clang -std=c++17). Idempotent via the ci-wspp-boost187 marker.
#     Cascade root: rmf_traffic_ros2 (aborts) -> rmf_battery,
#     rmf_visualization_rviz2_plugins, rmf_traffic_editor, rmf_traffic_examples. ---
WSPP="$(cd /opt/homebrew/include/websocketpp 2>/dev/null && pwd -P || true)"
if [ -n "${WSPP:-}" ] && [ -f "$WSPP/common/asio.hpp" ] && ! grep -q 'ci-wspp-boost187' "$WSPP/common/asio.hpp" 2>/dev/null; then
  _AS="$WSPP/common/asio.hpp"
  _CX="$WSPP/transport/asio/connection.hpp"
  _EP="$WSPP/transport/asio/endpoint.hpp"
  _NO="$WSPP/transport/asio/security/none.hpp"
  _TL="$WSPP/transport/asio/security/tls.hpp"
  # io_service -> io_context alias (fixes every bare io_service type reference)
  perl -0pi -e 's/(using namespace boost::asio;)/$1  \/\/ ci-wspp-boost187\n        using io_service = io_context;/' "$_AS"
  # strand: construction (needs executor) then member type
  perl -0pi -e 's/new lib::asio::io_service::strand\(\*io_service\)/new lib::asio::strand<lib::asio::io_context::executor_type>(io_service->get_executor())/g' "$_CX"
  perl -0pi -e 's/lib::asio::io_service::strand/lib::asio::strand<lib::asio::io_context::executor_type>/g' "$_CX" "$_NO" "$_TL"
  # strand::wrap(h) -> bind_executor(strand, h)  (two exact receivers)
  perl -0pi -e 's/m_strand->wrap\(/lib::asio::bind_executor(*m_strand, /g' "$_CX" "$_TL"
  perl -0pi -e 's/tcon->get_strand\(\)->wrap\(/lib::asio::bind_executor(*tcon->get_strand(), /g' "$_EP"
  # io_context::post(h) -> asio::post(io_context, h)
  perl -0pi -e 's/m_io_service->post\(/lib::asio::post(*m_io_service, /g' "$_CX"
  # timer->expires_from_now() -> remaining duration via expiry()
  perl -0pi -e 's/->expires_from_now\(\)/->expiry() - lib::asio::steady_timer::clock_type::now()/g' "$_CX" "$_EP"
  # io_service::work -> executor_work_guard (construction then type)
  perl -0pi -e 's/new lib::asio::io_service::work\(\*m_io_service\)/new lib::asio::executor_work_guard<lib::asio::io_context::executor_type>(m_io_service->get_executor())/g' "$_EP"
  perl -0pi -e 's/lib::asio::io_service::work/lib::asio::executor_work_guard<lib::asio::io_context::executor_type>/g' "$_EP"
  # socket_base::max_connections -> max_listen_connections
  perl -0pi -e 's/socket_base::max_connections/socket_base::max_listen_connections/g' "$_EP"
  # resolver::iterator -> results_type (sync listen path, async handler param, debug loop)
  perl -0777 -pi -e 's/tcp::resolver::iterator endpoint_iterator = r\.resolve\(query\);\n(\s*)tcp::resolver::iterator end;/tcp::resolver::results_type results = r.resolve(query);\n$1tcp::resolver::results_type::const_iterator endpoint_iterator = results.begin();\n$1tcp::resolver::results_type::const_iterator end = results.end();/' "$_EP"
  perl -0pi -e 's/lib::asio::ip::tcp::resolver::iterator iterator\)/lib::asio::ip::tcp::resolver::results_type iterator)/g' "$_EP"
  perl -0777 -pi -e 's/lib::asio::ip::tcp::resolver::iterator it, end;\n(\s*)for \(it = iterator; it != end; \+\+it\) \{/for (auto it = iterator.begin(); it != iterator.end(); ++it) {/' "$_EP"
  # resolver::query removed -> host/service string overloads
  perl -0777 -pi -e 's/^[ \t]*tcp::resolver::query query\([^)]*\);\n//mg' "$_EP"
  perl -0pi -e 's/r\.resolve\(query\)/r.resolve(host, service)/g' "$_EP"
  perl -0777 -pi -e 's/(async_resolve\(\n[ \t]*)query,/${1}host, port,/g' "$_EP"
  # io_context::reset() renamed to restart()
  perl -0pi -e 's/m_io_service->reset\(\)/m_io_service->restart()/g' "$_EP" "$_CX"
  echo "  websocketpp 0.8.2: patched for Boost 1.87+ (io_service/strand/work/post/resolver/timer)"
fi

echo "source patches applied."
