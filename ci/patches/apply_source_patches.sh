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
# --- MOVED TO id_ FORKS (fix baked into forked source + submodule repointed; removed
#     from this script so the next run validates the real source for upstream PRs):
#       nebula_velodyne_hw_interfaces -> id_nebula (boost/format.hpp)
#       event_camera_tools           -> id_event_camera_tools (unistd.h)
#       sick_safetyscanners_base     -> id_sick_safetyscanners_base (posix_time_types.hpp)
# libcreate: serial_query.h declares boost::asio::deadline_timer, but on Boost 1.89 the
# umbrella <boost/asio.hpp> no longer pulls in deadline_timer.hpp -> add the explicit
# include (posix_time bits are header-only, no date_time link needed). The io_service->
# io_context rename is already handled above by patch_io_service_typeonly libcreate;
# create.cpp's "no viable overloaded '='" cascades from the incomplete SerialQuery.
d="$(_pkg_dir libcreate)"; [ -n "$d" ] && for f in $(find "$d" -name serial_query.h 2>/dev/null); do _add_include "$f" '#include <boost/asio/deadline_timer.hpp>'; done
# aruco_ros: aruco_ros_utils.cpp includes 'opencv4/opencv2/…' — wrong prefix; OpenCV_INCLUDE_DIRS
# already ends in include/opencv4, so the include path is opencv2/… . Strip the opencv4/ prefix.
d="$(_pkg_dir aruco_ros)"; [ -n "$d" ] && for f in $(grep -rl 'opencv4/opencv2/' "$d" 2>/dev/null); do sed "${SEDI[@]}" 's#opencv4/opencv2/#opencv2/#g' "$f"; done
# schunk_svh_library: Serial.h includes Linux-only <termio.h>; macOS has only <termios.h>, and the
# class uses the termios struct anyway. Swap the legacy header.
d="$(_pkg_dir schunk_svh_library)"; [ -n "$d" ] && for f in $(grep -rl '<termio.h>' "$d" 2>/dev/null); do sed "${SEDI[@]}" 's#<termio\.h>#<termios.h>#g' "$f"; done
# libcreate (peel #2): serial.cpp calls io_context::reset() — renamed to restart() in Boost 1.87.
# (io_service->io_context handled by patch_io_service_typeonly; deadline_timer include above.)
# Compile-tested: io_context::restart() compiles, io.reset() is gone, vs brew boost 1.89.
d="$(_pkg_dir libcreate)"; [ -n "$d" ] && for f in $(find "$d" -name serial.cpp 2>/dev/null); do sed "${SEDI[@]}" 's/io\.reset()/io.restart()/g' "$f"; done
# sick_safetyscanners2: boost::asio::ip::address_v4::from_string removed in Boost 1.87 -> make_address_v4.
# Compile-tested: make_address_v4() compiles, address_v4::from_string is gone, vs brew boost 1.89.
d="$(_pkg_dir sick_safetyscanners2)"; [ -n "$d" ] && for f in $(grep -rl 'address_v4::from_string' "$d" 2>/dev/null); do sed "${SEDI[@]}" 's/address_v4::from_string/make_address_v4/g' "$f"; done
# onnxruntime_vendor: hard-rejects Darwin with FATAL_ERROR, but Microsoft publishes an osx-arm64
# prebuilt with the SAME .tgz layout the Linux path uses (verified onnxruntime-osx-arm64-<ver>.tgz
# exists on the v<ver> release). The vendor's copy/install is platform-agnostic (file(COPY) +
# install(DIRECTORY lib/)), so only the download asset name needs a Darwin case. Not unsupported.
d="$(_pkg_dir onnxruntime_vendor)"
if [ -n "$d" ] && grep -q 'macOS (Darwin) is not supported' "$d/CMakeLists.txt" 2>/dev/null; then
  sed "${SEDI[@]}" 's|.*message(FATAL_ERROR "onnxruntime_vendor: macOS (Darwin) is not supported.*|  set(ORT_ASSET_NAME "onnxruntime-osx-arm64-${ORT_VERSION}.tgz")|' "$d/CMakeLists.txt"
  echo "  onnxruntime_vendor: Darwin -> osx-arm64 prebuilt"
fi
# ros2_medkit_cmake: medkit_find_yaml_cpp() does find_library(yaml-cpp) which fails on macOS (the
# vendored yaml-cpp isn't on the default lib path) -> FATAL, blocking fmilibrary_vendor/clips_vendor
# and other medkit consumers on humble. Add a fallback tier that uses the toolchain's vendored
# YAML_CPP_LIBRARIES/YAML_CPP_INCLUDE_DIRS before the FATAL. Cmake-configure-tested: creates the
# yaml-cpp::yaml-cpp target from those vars.
d="$(_pkg_dir ros2_medkit_cmake)"
if [ -n "$d" ] && [ -f "$d/cmake/ROS2MedkitCompat.cmake" ] && ! grep -q 'ci-medkit-yamlfallback' "$d/cmake/ROS2MedkitCompat.cmake"; then
  perl -0pi -e 's/(\n[ \t]*)else\(\)\n([ \t]*message\(FATAL_ERROR "\[MedkitCompat\] Could not find yaml-cpp)/$1elseif(YAML_CPP_LIBRARIES AND YAML_CPP_INCLUDE_DIRS)  # ci-medkit-yamlfallback$1  add_library(yaml-cpp::yaml-cpp IMPORTED INTERFACE)$1  set_target_properties(yaml-cpp::yaml-cpp PROPERTIES INTERFACE_LINK_LIBRARIES "\$\{YAML_CPP_LIBRARIES\}" INTERFACE_INCLUDE_DIRECTORIES "\$\{YAML_CPP_INCLUDE_DIRS\}")$1else()\n$2/' "$d/cmake/ROS2MedkitCompat.cmake"
  echo "  ros2_medkit_cmake: yaml-cpp vendored-path fallback"
fi
# off_highway_* sensor drivers: rclcpp_components_register_node(... EXECUTABLE <exe>) already
# creates AND installs the standalone executable to lib/${PROJECT_NAME}; a redundant
# install(TARGETS <exe> ...) installs it a SECOND time to the same place, so macOS
# install_name_tool errors on the duplicate LC_RPATH (Linux/patchelf silently tolerates it).
# Drop the redundant install(TARGETS receiver/sender ...) across the off_highway family
# (mm7p10, general_purpose_radar, uss, radar; unblocks off_highway_can + adi_3dtof cascades).
for f in $(find "$ROOT" -path '*off_highway_sensor_drivers/*' -name CMakeLists.txt -not -path '*/build/*' -not -path '*/install/*' 2>/dev/null); do
  grep -q 'rclcpp_components_register_node' "$f" 2>/dev/null && perl -0pi -e 's/\ninstall\(TARGETS (?:receiver|sender)[^\)]*\)\n//g' "$f" && echo "  off_highway: dropped redundant install(TARGETS receiver/sender) in ${f#$ROOT/}"
done
# ros2_ouster: MOVED TO id_ros2_ouster_drivers fork (declare_parameter type) — removed from script.
# kuka_drivers_core: control_node.cpp sets CPU affinity via Linux-only cpu_set_t /
# pthread_setaffinity_np. Guard the block with #ifdef __linux__ (macOS has no cpu_set_t).
# PR-material (portability) — fork kuka_drivers when opening PRs. Cascade root for kuka stack.
d="$(_pkg_dir kuka_drivers_core)"
if [ -n "$d" ] && [ -f "$d/src/control_node.cpp" ] && ! grep -q 'CPU affinity not supported on this platform' "$d/src/control_node.cpp"; then
  perl -0777 -pi -e 's/([ \t]*)(cpu_set_t cpuset;.*?RCLCPP_INFO\(controller_manager->get_logger\(\), "CPU affinity set to core %d", cpu\);\n[ \t]*\})/$1#ifdef __linux__\n$1$2\n$1#else\n$1(void)cpu;\n$1RCLCPP_WARN(controller_manager->get_logger(), "CPU affinity not supported on this platform (non-Linux)");\n$1#endif/s' "$d/src/control_node.cpp"
  echo "  kuka_drivers_core: #ifdef __linux__ guard around cpu_set_t affinity block"
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

# --- Lane 2: naoqi_libqi/src/eventloop.cpp:10 has a stray raw
#     `#include <boost/asio/io_service.hpp>` — removed in Boost 1.87+ (fatal
#     error: file not found). The file already includes the package's own
#     boostasiocompat.hpp shim (line 6, BOOST_VERSION-gated AsioIoService/
#     AsioWork aliases) and uses qi::newAsioWork() from it — the raw include
#     is dead/redundant, just delete it. Compile-tested the shim alone
#     (qi::AsioIoService + qi::newAsioWork) against brew boost 1.89. ---
d="$(_pkg_dir naoqi_libqi)"; [ -n "$d" ] && [ -f "$d/src/eventloop.cpp" ] && \
  grep -q '^#include <boost/asio/io_service.hpp>' "$d/src/eventloop.cpp" && {
  sed "${SEDI[@]}" '/^#include <boost\/asio\/io_service.hpp>$/d' "$d/src/eventloop.cpp"
  echo "  naoqi_libqi eventloop.cpp: dropped stray boost/asio/io_service.hpp include"
}

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
#     on macOS) -> -Wmacro-redefined error. Pass CFLAGS to suppress that warning.
#     ALSO (2026-08-19): lely/libc/threads.h picks its pthread fallback via
#     `_POSIX_THREADS >= 200112L`, but on macOS _POSIX_THREADS is only defined after
#     <unistd.h> (glibc exposes it globally), so the check fails -> no threads backend
#     -> `unknown type name 'once_flag'/'cnd_t'/'mtx_t'`. Apple clang also claims C17
#     without defining __STDC_NO_THREADS__, so LELY_HAVE_THREADS_H could wrongly try the
#     absent system <threads.h>. Force both: THREADS_H off, PTHREAD_H on. Compile-tested
#     on macOS 26.5.2 (arm64), Apple clang 21.0.0, clang -std=c11 vs lely-core fb735b79:
#     fails without these (once_flag/cnd_t undefined), fully clean with them (+ the
#     existing sys/types.h clockid_t __APPLE__ patch below). ---
d="$(_pkg_dir lely_core_libraries)"
if [ -n "$d" ] && [ -f "$d/CMakeLists.txt" ] && ! grep -q 'Wno-macro-redefined' "$d/CMakeLists.txt"; then
  sed "${SEDI[@]}" 's#<SOURCE_DIR>/configure --prefix#<SOURCE_DIR>/configure "CFLAGS=-O2 -Wno-macro-redefined -Wno-keyword-macro -DLELY_HAVE_THREADS_H=0 -DLELY_HAVE_PTHREAD_H=1" --prefix#' "$d/CMakeLists.txt"
  echo "  lely_core_libraries: configure CFLAGS -Wno-macro-redefined + force pthread threads (THREADS_H=0 PTHREAD_H=1)"
fi

# --- Lane 6: lely_core_libraries's vendored libc/sys/types.h only treats
#     _POSIX_C_SOURCE/__MINGW32__/__NEWLIB__ as "has a real <sys/types.h>"; on
#     macOS none of those are defined, so it falls back to `typedef int
#     clockid_t;`, which conflicts with Apple SDK <time.h>'s `enum clockid_t`
#     (Xcode 26.6 / macOS 26) -> "typedef redefinition with different types".
#     Drop a patch file + wire it into the ExternalProject's UPDATE_COMMAND,
#     matching this package's existing git-apply patch mechanism. ---
d="$(_pkg_dir lely_core_libraries)"
if [ -n "$d" ] && [ -f "$d/CMakeLists.txt" ] && ! grep -q '0099-macos-clockid_t.patch' "$d/CMakeLists.txt"; then
  mkdir -p "$d/patches"
  cat > "$d/patches/0099-macos-clockid_t.patch" <<'PATCHEOF'
From 0000000000000000000000000000000000000099 Mon Sep 17 00:00:00 2001
From: Build Fix <noreply@example.com>
Date: Sat, 15 Aug 2026 00:00:00 +0000
Subject: [PATCH] Fix macOS clockid_t redefinition

LELY_HAVE_SYS_TYPES_H only recognizes _POSIX_C_SOURCE, __MINGW32__ and
__NEWLIB__ as "the platform provides a real <sys/types.h>". On macOS
none of those are defined in this build (no -D_POSIX_C_SOURCE), so
lely falls back to its own `typedef int clockid_t;` shim. But Apple's
SDK <time.h> (pulled in separately by lely/libc/time.h) already
declares `clockid_t` as `enum clockid_t` (Xcode 26.6 / macOS 26 SDK),
so the two declarations conflict:

  error: typedef redefinition with different types
  ('enum clockid_t' vs 'int')

macOS has a real, POSIX-conformant <sys/types.h> (with clockid_t and
ssize_t), so treat __APPLE__ the same as the existing __MINGW32__ /
__NEWLIB__ platform cases and use the system header instead of the
shim.
---
 include/lely/libc/sys/types.h | 2 +-
 1 file changed, 1 insertion(+), 1 deletion(-)

diff --git a/include/lely/libc/sys/types.h b/include/lely/libc/sys/types.h
index 9c4dd70d..5802fb39 100644
--- a/include/lely/libc/sys/types.h
+++ b/include/lely/libc/sys/types.h
@@ -26,7 +26,7 @@
 #include <lely/features.h>

 #ifndef LELY_HAVE_SYS_TYPES_H
-#if defined(_POSIX_C_SOURCE) || defined(__MINGW32__) || defined(__NEWLIB__)
+#if defined(_POSIX_C_SOURCE) || defined(__MINGW32__) || defined(__NEWLIB__) || defined(__APPLE__)
 #define LELY_HAVE_SYS_TYPES_H 1
 #endif
 #endif
PATCHEOF
  perl -0pi -e 's{(\n[ \t]*#CONFIGURE step execute autoreconf and configure)}{\n  COMMAND git apply --whitespace=fix --reject \$\{CMAKE_CURRENT_SOURCE_DIR\}/patches/0099-macos-clockid_t.patch$1}' "$d/CMakeLists.txt"
  echo "  lely_core_libraries: added 0099-macos-clockid_t.patch to UPDATE_COMMAND"
fi

# --- Lane 6: lely_core_libraries's src/*/Makefile.am append -Wl,--as-needed to
#     EVERY library's LDFLAGS unconditionally. That flag is GNU-ld-only; Apple's
#     ld64 rejects it outright ("ld: unknown options: --as-needed" -> clang: linker
#     command failed). On Darwin configure.ac's host case matches neither *linux*
#     nor *-*-mingw*, so PLATFORM_LINUX=no. Wrap the flag in the existing
#     `if PLATFORM_LINUX ... endif` automake conditional (same mechanism as the
#     adjacent PLATFORM_WIN32 block), then autoreconf -i (run by CONFIGURE_COMMAND)
#     emits it as @PLATFORM_LINUX_TRUE@ so macOS omits it. Verified vs pinned tag
#     fb735b79: patch applies clean (0 rejects), autoreconf -i -> Makefile.in has
#     `@PLATFORM_LINUX_TRUE@am__append = -Wl,--as-needed`. --as-needed only prunes
#     unused DT_NEEDED entries; dropping it does not change macOS runtime. ---
d="$(_pkg_dir lely_core_libraries)"
if [ -n "$d" ] && [ -f "$d/CMakeLists.txt" ] && ! grep -q '0100-macos-no-as-needed.patch' "$d/CMakeLists.txt"; then
  mkdir -p "$d/patches"
  cat > "$d/patches/0100-macos-no-as-needed.patch" <<'PATCHEOF'
From 0000000000000000000000000000000000000100 Mon Sep 17 00:00:00 2001
From: Build Fix <noreply@example.com>
Date: Wed, 27 Aug 2026 00:00:00 +0000
Subject: [PATCH] Guard -Wl,--as-needed behind PLATFORM_LINUX (macOS ld64)

Each src/*/Makefile.am appends `-Wl,--as-needed` to its library
LDFLAGS unconditionally. `--as-needed` is a GNU ld feature; Apple's
ld64 (the macOS system linker) rejects it outright:

  ld: unknown options: --as-needed
  clang: error: linker command failed with exit code 1

On Darwin, configure.ac's host case matches neither *linux* nor
*-*-mingw*, so PLATFORM_LINUX/PLATFORM_POSIX/PLATFORM_WIN32 are all
"no". Wrap the flag in the existing `if PLATFORM_LINUX ... endif`
automake conditional so it is emitted only where the GNU linker is in
use; on macOS the libraries link without it (identical runtime
behaviour -- --as-needed only prunes unused DT_NEEDED entries).
---
diff --git a/src/can/Makefile.am b/src/can/Makefile.am
index 5ccbb48..2dfce64 100644
--- a/src/can/Makefile.am
+++ b/src/can/Makefile.am
@@ -28,7 +28,9 @@ liblely_can_la_LDFLAGS = -no-undefined -version-number 1:9:2
 if PLATFORM_WIN32
 liblely_can_la_LDFLAGS += -Wl,--output-def,liblely-can-1.def
 endif
+if PLATFORM_LINUX
 liblely_can_la_LDFLAGS += -Wl,--as-needed
+endif
 liblely_can_la_LIBADD =
 liblely_can_la_LIBADD += $(top_builddir)/src/libc/liblely-libc.la
 liblely_can_la_LIBADD += $(top_builddir)/src/util/liblely-util.la
diff --git a/src/co/Makefile.am b/src/co/Makefile.am
index 65b1a77..f606834 100644
--- a/src/co/Makefile.am
+++ b/src/co/Makefile.am
@@ -72,7 +72,9 @@ liblely_co_la_LDFLAGS = -no-undefined -version-number $(VERSION_MAJOR):$(VERSION
 if PLATFORM_WIN32
 liblely_co_la_LDFLAGS += -Wl,--output-def,liblely-co-$(VERSION_MAJOR).def
 endif
+if PLATFORM_LINUX
 liblely_co_la_LDFLAGS += -Wl,--as-needed
+endif
 liblely_co_la_LIBADD =
 liblely_co_la_LIBADD += $(top_builddir)/src/libc/liblely-libc.la
 liblely_co_la_LIBADD += $(top_builddir)/src/util/liblely-util.la
diff --git a/src/coapp/Makefile.am b/src/coapp/Makefile.am
index 311535a..d867153 100644
--- a/src/coapp/Makefile.am
+++ b/src/coapp/Makefile.am
@@ -32,7 +32,9 @@ liblely_coapp_la_LDFLAGS = -no-undefined -version-number $(VERSION_MAJOR):$(VERS
 if PLATFORM_WIN32
 liblely_coapp_la_LDFLAGS += -Wl,--output-def,liblely-coapp-$(VERSION_MAJOR).def
 endif
+if PLATFORM_LINUX
 liblely_coapp_la_LDFLAGS += -Wl,--as-needed
+endif
 liblely_coapp_la_LIBADD =
 liblely_coapp_la_LIBADD += $(top_builddir)/src/libc/liblely-libc.la
 liblely_coapp_la_LIBADD += $(top_builddir)/src/util/liblely-util.la
diff --git a/src/ev/Makefile.am b/src/ev/Makefile.am
index 8844de6..f81fa25 100644
--- a/src/ev/Makefile.am
+++ b/src/ev/Makefile.am
@@ -31,7 +31,9 @@ liblely_ev_la_LDFLAGS = -no-undefined -version-number $(VERSION_MAJOR):$(VERSION
 if PLATFORM_WIN32
 liblely_ev_la_LDFLAGS += -Wl,--output-def,liblely-ev-$(VERSION_MAJOR).def
 endif
+if PLATFORM_LINUX
 liblely_ev_la_LDFLAGS += -Wl,--as-needed
+endif
 liblely_ev_la_LIBADD =
 liblely_ev_la_LIBADD += $(top_builddir)/src/libc/liblely-libc.la
 liblely_ev_la_LIBADD += $(top_builddir)/src/util/liblely-util.la
diff --git a/src/io/Makefile.am b/src/io/Makefile.am
index 871bef8..550bb5d 100644
--- a/src/io/Makefile.am
+++ b/src/io/Makefile.am
@@ -35,7 +35,9 @@ liblely_io_la_LDFLAGS = -no-undefined -version-number 1:9:2
 if PLATFORM_WIN32
 liblely_io_la_LDFLAGS += -Wl,--output-def,liblely-io-1.def
 endif
+if PLATFORM_LINUX
 liblely_io_la_LDFLAGS += -Wl,--as-needed
+endif
 liblely_io_la_LIBADD =
 liblely_io_la_LIBADD += $(top_builddir)/src/libc/liblely-libc.la
 liblely_io_la_LIBADD += $(top_builddir)/src/util/liblely-util.la
diff --git a/src/io2/Makefile.am b/src/io2/Makefile.am
index 2daf658..eba79dc 100644
--- a/src/io2/Makefile.am
+++ b/src/io2/Makefile.am
@@ -86,7 +86,9 @@ liblely_io2_la_LDFLAGS = -no-undefined -version-number $(VERSION_MAJOR):$(VERSIO
 if PLATFORM_WIN32
 liblely_io2_la_LDFLAGS += -Wl,--output-def,liblely-io2-$(VERSION_MAJOR).def
 endif
+if PLATFORM_LINUX
 liblely_io2_la_LDFLAGS += -Wl,--as-needed
+endif
 liblely_io2_la_LIBADD =
 liblely_io2_la_LIBADD += $(top_builddir)/src/libc/liblely-libc.la
 liblely_io2_la_LIBADD += $(top_builddir)/src/util/liblely-util.la
diff --git a/src/libc/Makefile.am b/src/libc/Makefile.am
index dd1afe7..d484602 100644
--- a/src/libc/Makefile.am
+++ b/src/libc/Makefile.am
@@ -43,7 +43,9 @@ liblely_libc_la_LDFLAGS = -no-undefined -version-number $(VERSION_MAJOR):$(VERSI
 if PLATFORM_WIN32
 liblely_libc_la_LDFLAGS += -Wl,--output-def,liblely-libc-$(VERSION_MAJOR).def
 endif
+if PLATFORM_LINUX
 liblely_libc_la_LDFLAGS += -Wl,--as-needed
+endif
 liblely_libc_la_LIBADD = $(RT_LIBS)
 if CODE_COVERAGE_ENABLED
 liblely_libc_la_LIBADD += $(CODE_COVERAGE_LIBS)
diff --git a/src/tap/Makefile.am b/src/tap/Makefile.am
index 6c06b88..a2c3e24 100644
--- a/src/tap/Makefile.am
+++ b/src/tap/Makefile.am
@@ -15,7 +15,9 @@ liblely_tap_la_LDFLAGS = -no-undefined -version-number $(VERSION_MAJOR):$(VERSIO
 if PLATFORM_WIN32
 liblely_tap_la_LDFLAGS += -Wl,--output-def,liblely-tap-$(VERSION_MAJOR).def
 endif
+if PLATFORM_LINUX
 liblely_tap_la_LDFLAGS += -Wl,--as-needed
+endif
 liblely_tap_la_LIBADD =
 liblely_tap_la_LIBADD += $(top_builddir)/src/libc/liblely-libc.la
 if CODE_COVERAGE_ENABLED
diff --git a/src/util/Makefile.am b/src/util/Makefile.am
index 3a77f65..780f4a6 100644
--- a/src/util/Makefile.am
+++ b/src/util/Makefile.am
@@ -67,7 +67,9 @@ liblely_util_la_LDFLAGS = -no-undefined -version-number $(VERSION_MAJOR):$(VERSI
 if PLATFORM_WIN32
 liblely_util_la_LDFLAGS += -Wl,--output-def,liblely-util-$(VERSION_MAJOR).def
 endif
+if PLATFORM_LINUX
 liblely_util_la_LDFLAGS += -Wl,--as-needed
+endif
 liblely_util_la_LIBADD =
 liblely_util_la_LIBADD += $(top_builddir)/src/libc/liblely-libc.la
 if !ECSS_COMPLIANCE
PATCHEOF
  perl -0pi -e 's{(\n[ \t]*#CONFIGURE step execute autoreconf and configure)}{\n  COMMAND git apply --whitespace=fix --reject \$\{CMAKE_CURRENT_SOURCE_DIR\}/patches/0100-macos-no-as-needed.patch$1}' "$d/CMakeLists.txt"
  echo "  lely_core_libraries: added 0100-macos-no-as-needed.patch to UPDATE_COMMAND"
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

# --- Lane 6: canboat_vendor's CANBOAT_TOOLS list unconditionally installs
#     socketcan-writer, but canboat's own Makefile (socketcan-writer/Makefile)
#     gates that binary to Linux only (`ifeq ($(OS),Linux)` -> TARGETS empty
#     elsewhere), so it is never built on macOS and `install(PROGRAMS ...)`
#     fails with "file INSTALL cannot find .../rel/socketcan-writer". This repo
#     only ever builds on macOS, so drop the entry (BUILD_BYPRODUCTS + tool
#     list) rather than add a runtime guard. Present in jazzy/kilted only
#     (same submodule commit fc04c4f, absent from humble) -> no-op there. ---
d="$(_pkg_dir canboat_vendor)"
if [ -n "$d" ] && [ -f "$d/CMakeLists.txt" ] && grep -q 'socketcan-writer' "$d/CMakeLists.txt"; then
  sed "${SEDI[@]}" \
    -e '/<SOURCE_DIR>\/rel\/socketcan-writer/d' \
    -e '/^[[:space:]]*socketcan-writer[[:space:]]*$/d' \
    "$d/CMakeLists.txt"
  echo "  canboat_vendor: dropped Linux-only socketcan-writer from install list"
fi

# --- Lane 6: fmilibrary_vendor's ExternalProject clones modelon-community/
#     fmi-library (own CMake project, cmake_minimum_required 2.8.6). Its
#     Config.cmake/mergestaticlibs.cmake reads the LOCATION target property on
#     not-yet-built targets (pre-3.0 idiom) -> modern CMake enforces CMP0026
#     as a hard error ("The LOCATION property may not be read from target...
#     use $<TARGET_FILE>"), hit on every merge_static_libs() call (fmiimport,
#     fmilib, etc). Not macOS-specific -- same across all 3 distros (identical
#     CMakeLists.txt, same fmilibrary_version 2.2.3 pin, verified byte-equal
#     humble/jazzy/kilted despite differing vendor submodule commits).
#     PATCH_COMMAND runs perl on the freshly-cloned fmi-library source,
#     swapping the LOCATION reads for `set(... "$<TARGET_FILE:...>")`. The
#     replacement is built via chr(36) + string concat (/e) instead of a
#     literal '$<' in the command text: ExternalProject stores *_COMMAND as a
#     CMake list, and any literal '$<...>' genexp syntax embedded in that text
#     -- even inside a [[ ]] bracket-argument, which only suppresses ${VAR}
#     expansion, not genexp scanning -- gets eagerly evaluated by the OUTER
#     (fmilibrary_vendor) project at generate time and collapses to empty,
#     since ${lib}/${outlib} aren't real targets in that scope. Split across
#     two PATCH_COMMAND/COMMAND steps (not one `;`-joined perl -e) because
#     ExternalProject also re-splits command text on bare semicolons.
#     VERIFIED end-to-end against the real files: fetched mergestaticlibs.cmake
#     from the v2.2.3 tag, ran this exact injected CMakeLists.txt through a
#     real (offline, local SOURCE_DIR) externalproject_add PATCH_COMMAND step,
#     diffed the output byte-identical to a version that was then separately
#     configure+build tested (AppleClang) via a probe project calling
#     merge_static_libs() on two dummy static libs -- produced a correctly
#     merged archive with both input symbols present.
#     2026-08-19: getting past CMP0026 unmasked 3 MORE errors in the SAME
#     configure/build (CMake reports several errors per pass, then bails --
#     each earlier fix just uncovers the next one; peeled all the way to a
#     real `make install` locally):
#       1) Config.cmake/runtime_test.cmake:36 `get_property(... TARGET
#          compress_test_fmu_zip PROPERTY LOCATION)` -- same CMP0026 class,
#          only reached when FMILIB_BUILD_TESTS=ON (default). fmi-library's
#          own doc target construction (UseDoxygen.cmake:140
#          `get_target_property(DOC_TARGET doc TYPE)` on a not-yet-defined
#          "doc" target -- modern CMake hard-errors instead of returning
#          NOTFOUND) only reached when FMILIB_GENERATE_DOXYGEN_DOC=ON
#          (default, and this runner has doxygen installed so it's not even
#          skipped by a missing-Doxygen guard). We don't need fmi-library's
#          own test suite or doc target for a vendored lib -- CMAKE_ARGS
#          turns both off, skipping the broken codepaths entirely rather than
#          patching them.
#       2) Config.cmake/fmixml.cmake:185 `add_dependencies(expatex
#          ${CMAKE_BINARY_DIR}/CMakeCache.txt ${FMILIBRARYHOME}/CMakeLists.txt)`
#          passes FILE paths (not target names) to add_dependencies() --
#          modern CMake errors "dependency target ... does not exist";
#          harmless/redundant since the actual file dependency for the
#          expatex re-configure step is already declared correctly via the
#          `DEPENDS ${CMAKE_BINARY_DIR}/CMakeCache.txt` on the
#          ExternalProject_Add_Step right above it. This error was invisible
#          in the CI log because CMake bails out of the configure phase
#          entirely on error (1) before ever reaching the generate phase
#          where this one surfaces -- confirmed locally: fixing only (1)
#          exposes this as the new next failure. A 3rd PATCH_COMMAND step
#          perl-deletes the whole `add_dependencies(expatex ...)` line
#          (matched by literal prefix, no ${VAR}/genexp text needed in the
#          pattern so no CMake-vs-perl escaping is involved).
#       3) ThirdParty/Minizip/minizip/miniunz.c:143 calls bare `mkdir(path)`
#          with no `<sys/stat.h>` include -- ancient (~2013) vendored C code;
#          Apple Clang (Xcode 26.6) treats implicit-function-declaration as a
#          hard error by default in C mode (Clang 15+ behavior change), so
#          this blocks the actual `make` even after configure succeeds.
#          CMAKE_ARGS downgrades it back to a warning for this ExternalProject
#          only (`-DCMAKE_C_FLAGS=-Wno-error=...`) -- scoped to the vendored
#          build, doesn't touch our own toolchain flags.
#     Also: the wrapper's own install()/ament_export_libraries() hardcode
#     `libfmilib_shared.so`, but fmi-library's CMakeLists never overrides
#     OUTPUT_NAME, so on macOS the real built artifact is
#     `libfmilib_shared.dylib` -- install(FILES ...) would fail on a
#     nonexistent path. Fixed below via ${CMAKE_SHARED_LIBRARY_SUFFIX}
#     (portable, Linux .so unaffected).
#     VERIFIED end-to-end (2026-08-19): built a scratch outer CMakeLists.txt
#     driving externalproject_add() with SOURCE_DIR pointed at a real,
#     offline v2.2.3 clone and this exact CMAKE_ARGS/PATCH_COMMAND chain --
#     full `cmake --build` (including the nested expatex sub-build) completed
#     clean, `make install` produced both
#     .../install/lib/libfmilib.a and .../install/lib/libfmilib_shared.dylib. ---
d="$(_pkg_dir fmilibrary_vendor)"
if [ -n "$d" ] && [ -f "$d/CMakeLists.txt" ] && ! grep -q 'ci-fmilib-cmp0026' "$d/CMakeLists.txt"; then
  _FMI_SNIPPET="$(mktemp)"
  cat > "$_FMI_SNIPPET" <<'PATCHEOF'
  CMAKE_ARGS -DFMILIB_BUILD_TESTS=OFF -DFMILIB_GENERATE_DOXYGEN_DOC=OFF -DCMAKE_C_FLAGS=-Wno-error=implicit-function-declaration
  PATCH_COMMAND perl -0pi -e [[s/get_target_property\(libfile (\$\{lib\}) LOCATION\)/'set(libfile "'.chr(36).'<TARGET_FILE:'.$1.'>")'/ge]] <SOURCE_DIR>/Config.cmake/mergestaticlibs.cmake
  COMMAND perl -0pi -e [[s/get_target_property\(outfile (\$\{outlib\}) LOCATION\)/'set(outfile "'.chr(36).'<TARGET_FILE:'.$1.'>")'/ge]] <SOURCE_DIR>/Config.cmake/mergestaticlibs.cmake
  COMMAND perl -0pi -e [[s/^[ \t]*add_dependencies\(expatex[^\n]*\)[ \t]*\n//m]] <SOURCE_DIR>/Config.cmake/fmixml.cmake # ci-fmilib-cmp0026
PATCHEOF
  sed -e "/^[[:space:]]*TIMEOUT 60[[:space:]]*\$/r $_FMI_SNIPPET" "$d/CMakeLists.txt" > "$d/CMakeLists.txt.__p" && mv "$d/CMakeLists.txt.__p" "$d/CMakeLists.txt"
  rm -f "$_FMI_SNIPPET"
  echo "  fmilibrary_vendor: PATCH_COMMAND fixes CMP0026 (mergestaticlibs), disables tests/doxygen, drops broken expatex add_dependencies, downgrades implicit-decl to warning"
fi
d="$(_pkg_dir fmilibrary_vendor)"
if [ -n "$d" ] && [ -f "$d/CMakeLists.txt" ] && grep -q 'libfmilib_shared\.so' "$d/CMakeLists.txt"; then
  sed "${SEDI[@]}" \
    -e 's/libfmilib_shared\.so/libfmilib_shared${CMAKE_SHARED_LIBRARY_SUFFIX}/' \
    "$d/CMakeLists.txt"
  echo "  fmilibrary_vendor: install()/ament_export_libraries() use \${CMAKE_SHARED_LIBRARY_SUFFIX} instead of hardcoded .so (macOS builds libfmilib_shared.dylib)"
fi

# --- Lane 8: live555_vendor (jazzy only -- absent from humble/kilted; a
#     brand-new, 2025, CMake-native live555 port, not an ExternalProject).
#     `add_library(UsageEnvironment ...)` is followed by an unconditional
#     `if(NOT WIN32) target_link_options(... "LINKER:--allow-shlib-undefined")`
#     -- a GNU-ld-only flag; Apple's ld64 rejects it outright ("ld: unknown
#     options: --allow-shlib-undefined"). The flag isn't decorative: it's
#     covering a real circular symbol dependency -- HashTable::create() /
#     HashTable::Iterator::create() are only implemented in
#     BasicHashTable.cpp (the BasicUsageEnvironment target), which itself
#     links UsageEnvironment (not the reverse), so UsageEnvironment.dylib
#     genuinely has unresolved symbols until the final consumer links both
#     libs together. Confirmed locally: just deleting the flag on APPLE (a
#     tempting first guess) makes UsageEnvironment fail with a real
#     "Undefined symbols ... HashTable::Iterator::create" link error --
#     GNU ld's --allow-shlib-undefined and Apple ld64's
#     `-undefined dynamic_lookup` are the platform-equivalent way to permit
#     this, so we swap to the latter under APPLE rather than dropping it.
#     groupsock and liveMedia hit the identical HashTable-symbol pattern
#     (both link UsageEnvironment only, not BasicUsageEnvironment) even
#     though upstream's Linux build never needed the flag there -- macOS's
#     ld64 defaults to strict undefined-symbol checking for dylibs, Linux's
#     default is permissive, so this is a real linker-default gap between
#     platforms, not an upstream omission to fix narrowly. Also fixed one
#     more, platform-independent upstream bug that only surfaces once the
#     linker errors above are cleared: `target_link_libraries(liveMedia
#     PRIVATE ... OpenSSL::SSL)` hides OpenSSL's include dir from liveMedia's
#     consumers, but TLSState.hh -- a *public* liveMedia header, pulled in
#     transitively by the demo executables via RTPInterface.hh -- needs
#     <openssl/ssl.h> directly whenever OpenSSL is present; PRIVATE->PUBLIC.
#     VERIFIED end-to-end: real offline `cmake --build` of the actual
#     submodule source with default options (LIVE555_BUILD_EXECUTABLES ON,
#     i.e. exactly what colcon builds since this package is CMAKE_PROJECT_IS_
#     TOP_LEVEL) -- all 4 libraries AND all 3 demo executables link clean;
#     `make install` produced libUsageEnvironment/libBasicUsageEnvironment/
#     libgroupsock/libliveMedia .dylib + live555Config.cmake under DESTDIR. ---
d="$(_pkg_dir live555_vendor)"
if [ -n "$d" ] && [ -f "$d/CMakeLists.txt" ] && ! grep -q 'ci-live555-apple-undefined' "$d/CMakeLists.txt"; then
  perl -0777 -pi -e '
    s/if\(NOT WIN32\)\n([ \t]*)target_link_options\(UsageEnvironment PRIVATE "LINKER:--allow-shlib-undefined"\)\nendif\(\)/if(APPLE) # ci-live555-apple-undefined\n$1target_link_options(UsageEnvironment PRIVATE "LINKER:-undefined,dynamic_lookup")\nelseif(NOT WIN32)\n$1target_link_options(UsageEnvironment PRIVATE "LINKER:--allow-shlib-undefined")\nendif()/;
    s/(live555\/BasicUsageEnvironment\/BasicHashTable\.cpp\n\))\n/$1\nif(APPLE)\n    target_link_options(BasicUsageEnvironment PRIVATE "LINKER:-undefined,dynamic_lookup")\nendif()\n/;
    s/(live555\/groupsock\/NetInterface\.cpp\n\))\n/$1\nif(APPLE)\n    target_link_options(groupsock PRIVATE "LINKER:-undefined,dynamic_lookup")\nendif()\n/;
    s/(live555\/liveMedia\/WAVAudioFileSource\.cpp\n\))\n/$1\nif(APPLE)\n    target_link_options(liveMedia PRIVATE "LINKER:-undefined,dynamic_lookup")\nendif()\n/;
    s/target_link_libraries\(liveMedia PRIVATE \$<TARGET_NAME_IF_EXISTS:OpenSSL::SSL>\)/target_link_libraries(liveMedia PUBLIC \$<TARGET_NAME_IF_EXISTS:OpenSSL::SSL>)/;
  ' "$d/CMakeLists.txt"
  echo "  live555_vendor: Apple ld64 -undefined,dynamic_lookup for UsageEnvironment/BasicUsageEnvironment/groupsock/liveMedia (was GNU-ld-only --allow-shlib-undefined) + liveMedia's OpenSSL link PRIVATE->PUBLIC"
fi

# --- Lane 6: ecal — CMakeLists.txt:299 `find_package(CMakeFunctions REQUIRED)`
#     only resolves via a CMake dependency provider
#     (cmake_language(SET_DEPENDENCY_PROVIDER ...) in cmake/submodule_dependencies.cmake)
#     that eCAL's own build never activates unless CMAKE_PROJECT_TOP_LEVEL_INCLUDES
#     is set before the top-level project() call — upstream only does that via
#     CMakePresets.json, which colcon does not use, so find_package() falls
#     through to a normal search and fails ("Findcatkin"-style "could not find
#     a package configuration file"). Turning the provider on globally would
#     default-build ~15 more vendored thirdparty libs from source
#     (ecal_submodule_dependencies: asio, HDF5, Protobuf, spdlog, yaml-cpp...)
#     since each ECAL_THIRDPARTY_BUILD_<NAME> option defaults ON — far more
#     than this needs. CMakeFunctionsConfig.cmake.in's @PACKAGE_INIT@ body is
#     itself just `include(.../cmake_functions.cmake)` (plain macro
#     definitions: git_revision_information, msvc_macros,
#     protoc_generate_files, targets_protobuf — no compiled artifact) — so
#     swap the find_package() for that same include directly, bypassing the
#     provider entirely. Same submodule commit (e9ca7cf) + identical
#     CMakeLists.txt across all 3 distros -> shared fix. ---
d="$(find "$ROOT" -maxdepth 4 -type d -path '*/middleware/ecal' -not -path '*/build/*' 2>/dev/null | head -1)"
if [ -n "$d" ] && [ -f "$d/CMakeLists.txt" ] && grep -q 'find_package(CMakeFunctions REQUIRED)' "$d/CMakeLists.txt"; then
  sed "${SEDI[@]}" 's|find_package(CMakeFunctions REQUIRED)|include(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/cmakefunctions/cmake_functions/cmake_functions.cmake)|' "$d/CMakeLists.txt"
  echo "  ecal: find_package(CMakeFunctions) -> direct include (dependency provider never activated by colcon)"
fi

# --- Lane 4 (real bug, per-distro): rmf_traffic's schedule::Database and
#     schedule::Mirror classes override Viewer::get_participant /
#     ItineraryViewer::get_itinerary / get_current_plan_id — the base virtuals
#     take `ParticipantId` (uint64_t), but Database.hpp/Mirror.hpp (and their
#     .cpp definitions) declare these three overrides with `std::size_t`
#     instead. On 64-bit Linux/glibc, uint64_t is `unsigned long` == size_t,
#     so the mismatch is silently invisible there; on macOS/clang, uint64_t is
#     `unsigned long long`, a genuinely distinct type from size_t, so these
#     become non-overriding `final` members that hide the base virtuals ->
#     hard error. Genuine upstream bug (already fixed in the commit kilted
#     pins — grep confirms 0 occurrences of "std::size_t participant_id"
#     there); humble/jazzy pin older commits that still have it -> per-distro
#     fix, no-ops cleanly on kilted. Compile-tested: built a probe
#     ItineraryViewer subclass implementing the 3 overrides with ParticipantId
#     against the real Viewer.hpp/rmf_utils headers -> clean compile. ---
for _rf in \
  "$ROOT/fleet/rmf_traffic/rmf_traffic/include/rmf_traffic/schedule/Database.hpp" \
  "$ROOT/fleet/rmf_traffic/rmf_traffic/include/rmf_traffic/schedule/Mirror.hpp" \
  "$ROOT/fleet/rmf_traffic/rmf_traffic/src/rmf_traffic/schedule/Database.cpp" \
  "$ROOT/fleet/rmf_traffic/rmf_traffic/src/rmf_traffic/schedule/Mirror.cpp"; do
  if [ -f "$_rf" ] && grep -q 'std::size_t participant_id' "$_rf"; then
    sed "${SEDI[@]}" 's/std::size_t participant_id/ParticipantId participant_id/g' "$_rf"
    echo "  rmf_traffic: std::size_t -> ParticipantId in ${_rf#$ROOT/}"
  fi
done

# --- Lane 4 (real bug): rmf_utils/Modular.hpp uses std::to_string (in the modular-
#     distance overflow message) but includes only <stdexcept>/<limits>/<type_traits>.
#     libstdc++ pulls std::to_string transitively through those; libc++ (Apple clang)
#     does NOT -> "no member named 'to_string' in namespace 'std'" when rmf_traffic
#     (and every rmf_utils consumer) compiles against the installed header. Add the
#     missing <string>. Compile-tested Apple clang 21 / libc++ -std=c++17: reproduces
#     the error without it, clean with it. ---
for _mh in $(find "$ROOT" -path '*rmf_utils/include/rmf_utils/Modular.hpp' -not -path '*/build/*' -not -path '*/install/*' 2>/dev/null); do
  if ! grep -qE '#include[[:space:]]*<string>' "$_mh"; then
    perl -0pi -e 's{(#include <type_traits>)}{$1\n#include <string>  // std::to_string (libc++ does not pull it transitively)}' "$_mh"
    echo "  rmf_utils/Modular.hpp: +#include <string> (std::to_string on libc++) in ${_mh#$ROOT/}"
  fi
done

# --- Lane 4 (real bug): cartographer_ros_msgs/CMakeLists.txt falls back to
#     set(CMAKE_CXX_STANDARD 14) when the toolchain has not already pinned it. But
#     toolchain.cmake force-includes cartographer_absl_compat.h into every ^cartographer
#     project, and that shim #include <absl/base/thread_annotations.h> -> Abseil's
#     policy_checks.h hard-errors "C++ versions less than C++17 are not supported" under
#     C++14. Bump the fallback to 17 (harmless for a msgs package). ---
d="$(_pkg_dir cartographer_ros_msgs)"
if [ -n "$d" ] && [ -f "$d/CMakeLists.txt" ] && grep -qE 'set\(CMAKE_CXX_STANDARD 14\)' "$d/CMakeLists.txt"; then
  sed "${SEDI[@]}" 's/set(CMAKE_CXX_STANDARD 14)/set(CMAKE_CXX_STANDARD 17)/' "$d/CMakeLists.txt"
  echo "  cartographer_ros_msgs: CMAKE_CXX_STANDARD 14 -> 17 (absl shim needs C++17)"
fi

# --- Lane 3: rmf_traffic_editor's gui_lib target_link_libraries() lists the
#     bare word `yaml-cpp` (not an imported target in this scope, not
#     ${YAML_CPP_LIBRARIES}) -> CMake emits a raw `-lyaml-cpp` link flag ->
#     "ld: library 'yaml-cpp' not found". toolchain.cmake's global
#     link_directories(.../opt/yaml_cpp_vendor/lib) covers most bare -lyaml-cpp
#     consumers, but this target still fails, so swap the bare name for the
#     toolchain's own ${YAML_CPP_LIBRARIES} absolute-path CACHE variable
#     (set in cmake/toolchain.cmake) — the same idiom already used natively by
#     other packages in this tree (camera_calibration_parsers, aruco_opencv,
#     velodyne_pointcloud, etc), so it's a proven-working substitution, not a
#     novel one. Same bare-`yaml-cpp` construct present in all 3 distros
#     (different per-distro rmf commits, same bug) -> shared fix. Not
#     compile-tested end-to-end (needs a full Qt5 GUI build of gui_lib/
#     traffic-editor); verified instead that ${YAML_CPP_LIBRARIES} is set
#     unconditionally in toolchain.cmake and is the exact pattern already
#     working elsewhere in-tree. ---
for _rf in \
  "$ROOT/fleet/rmf_traffic_editor/rmf_traffic_editor/CMakeLists.txt"; do
  if [ -f "$_rf" ] && grep -qE '^[[:space:]]*yaml-cpp[[:space:]]*$' "$_rf"; then
    sed "${SEDI[@]}" 's/^\([[:space:]]*\)yaml-cpp\([[:space:]]*\)$/\1${YAML_CPP_LIBRARIES}\2/' "$_rf"
    echo "  rmf_traffic_editor: bare yaml-cpp -> \${YAML_CPP_LIBRARIES} in ${_rf#$ROOT/}"
  fi
done

# --- rmf_traffic_ros2 has the identical bare-`yaml-cpp` bug as
#     rmf_traffic_editor above ("ld: library 'yaml-cpp' not found" at
#     librmf_traffic_ros2.dylib link). Its CMakeLists.txt has 3 occurrences of
#     the string "yaml-cpp": one in ament_export_dependencies() (a package
#     name, correct as-is, indented 2 spaces) and two in real
#     target_link_libraries() calls -- the main `rmf_traffic_ros2` target
#     (4-space indent) and the BUILD_TESTING-gated `test_rmf_traffic_ros2`
#     target (6-space indent). Only rewrite the 4/6-space-indented ones so the
#     ament_export_dependencies() package-name entry is left untouched (that
#     one must stay a plain package name for downstream find_package(), not an
#     absolute path). Same construct present in all 3 distros (different
#     per-distro rmf commits, same bug) -> shared fix, same proven idiom as
#     rmf_traffic_editor. Not end-to-end compile-tested (no local install/opt
#     /yaml_cpp_vendor build artifact in this checkout to link against, same
#     limitation noted for rmf_traffic_editor); verified structurally instead:
#     transform applied to a scratch copy of the real CMakeLists.txt leaves
#     the find_package(yaml-cpp REQUIRED) call and the 2-space
#     ament_export_dependencies() entry untouched and only rewrites the two
#     target_link_libraries() occurrences; ${YAML_CPP_LIBRARIES} is set
#     unconditionally in every distro's cmake/toolchain.cmake. ---
for _rf in \
  "$ROOT/fleet/rmf_ros2/rmf_traffic_ros2/CMakeLists.txt"; do
  if [ -f "$_rf" ] && grep -qE '^(    |      )yaml-cpp$' "$_rf"; then
    sed "${SEDI[@]}" -E 's/^(    |      )yaml-cpp$/\1${YAML_CPP_LIBRARIES}/' "$_rf"
    echo "  rmf_traffic_ros2: bare yaml-cpp -> \${YAML_CPP_LIBRARIES} in ${_rf#$ROOT/}"
  fi
done

# --- Lane 6: nebula_velodyne_common (humble+jazzy, identical nebula commit
#     ec2a724, absent kilted) -- same bare-`yaml-cpp` linker-name bug as
#     rmf_traffic_editor/rmf_traffic_ros2 above, but on a single-line
#     `target_link_libraries(nebula_velodyne_common PUBLIC yaml-cpp)` call
#     rather than the multi-line indented form, so it needs its own literal
#     match rather than reusing the indentation-anchored loop. "ld: library
#     'yaml-cpp' not found" at the final link of libnebula_velodyne_common.dylib
#     -- `yaml-cpp` resolves as a bare `-lyaml-cpp` linker flag, not a real
#     CMake target, in this scope. Swapped for the toolchain's own
#     ${YAML_CPP_LIBRARIES} absolute-path CACHE var (same proven idiom, used
#     natively upstream by 8+ other in-tree packages per the toolchain
#     comment). Left the separate `ament_export_dependencies(nebula_core_common
#     yaml-cpp)` line alone -- that's a package-name export, not a linker
#     flag, and unrelated to this bug. Not compile-tested end-to-end (same
#     limitation as rmf_traffic_editor/rmf_traffic_ros2: needs a populated
#     local yaml_cpp_vendor install/ tree to actually link-test, not available
#     in this checkout); verified structurally against the real CMakeLists.txt
#     in both humble and jazzy trees, idempotency-checked (2nd run: no match,
#     no double-replace). ---
d="$(_pkg_dir nebula_velodyne_common)"
if [ -n "$d" ] && [ -f "$d/CMakeLists.txt" ] && grep -q 'target_link_libraries(nebula_velodyne_common PUBLIC yaml-cpp)' "$d/CMakeLists.txt"; then
  sed "${SEDI[@]}" \
    -e 's/target_link_libraries(nebula_velodyne_common PUBLIC yaml-cpp)/target_link_libraries(nebula_velodyne_common PUBLIC ${YAML_CPP_LIBRARIES})/' \
    "$d/CMakeLists.txt"
  echo "  nebula_velodyne_common: bare yaml-cpp -> \${YAML_CPP_LIBRARIES}"
fi

# --- Lane 7-guard: rmw_stats_shim's `if(CMAKE_COMPILER_IS_GNUCXX OR
#     CMAKE_CXX_COMPILER_ID MATCHES "Clang")` unconditionally adds
#     `-Wl,--no-undefined` — a GNU-ld-only flag. MATCHES does a regex search,
#     so "AppleClang" matches the "Clang" pattern too -> Apple's ld64 rejects
#     the flag ("ld: unknown options: --no-undefined"). Apple's linker already
#     errors on undefined symbols by default for a shared lib (no dynamic_lookup
#     override here), so the flag is a no-op on Linux/GNU anyway if simply
#     dropped for APPLE -- just guard it. Compile-tested: reproduced the exact
#     guarded add_link_options() against a minimal SHARED-lib CMake probe;
#     built clean with AppleClang. ---
d="$(_pkg_dir rmw_stats_shim)"
if [ -n "$d" ] && [ -f "$d/CMakeLists.txt" ] && grep -q 'add_link_options("-Wl,--no-undefined")' "$d/CMakeLists.txt"; then
  perl -0pi -e 's/if\(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang"\)\n(\s*)add_compile_options\(-Wall -Wextra -Wpedantic\)\n(\s*)add_link_options\("-Wl,--no-undefined"\)\n(\s*)endif\(\)/if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")\n${1}add_compile_options(-Wall -Wextra -Wpedantic)\n${1}if(NOT APPLE)\n${2}add_link_options("-Wl,--no-undefined")\n${2}endif()\n${3}endif()/' "$d/CMakeLists.txt"
  echo "  rmw_stats_shim: guarded -Wl,--no-undefined with NOT APPLE"
fi

# --- Lane 6 (humble only, package absent in jazzy/kilted): as2_platform_crazyflie
#     FetchContent-pulls crazyflie_cpp, which statically links libusb-1.0.
#     libusb's macOS backend (darwin_usb.o) calls IOKit/CoreFoundation/Security
#     APIs directly (CFRunLoop*, IOService*, SecTask*, ...) but neither
#     crazyflie_cpp nor as2_platform_crazyflie's own CMakeLists link those
#     system frameworks -> "Undefined symbols for architecture arm64" at the
#     final link of both _node and _swarm_node executables. Standard macOS
#     libusb consumers must add these 3 frameworks explicitly (libusb's own
#     pkg-config .pc file lists them in Libs.private, which static linking
#     doesn't auto-propagate). Compile-tested: N/A (no local libusb-based repro
#     harness), but the fix is exactly the frameworks needed to resolve every
#     undefined symbol seen in the CI log (CFBooleanGetTypeID, IOServiceMatching,
#     SecTaskCreateFromSelf, etc — all covered by these 3 frameworks, a
#     well-documented libusb-on-macOS requirement). ---
d="$(_pkg_dir as2_platform_crazyflie)"
if [ -n "$d" ] && [ -f "$d/CMakeLists.txt" ] && ! grep -q 'framework CoreFoundation' "$d/CMakeLists.txt"; then
  perl -0pi -e 's/(if\(BUILD_TESTING\))/if(APPLE)\n  target_link_libraries(\${PROJECT_NAME}_node "-framework CoreFoundation" "-framework IOKit" "-framework Security")\n  target_link_libraries(\${PROJECT_NAME}_swarm_node "-framework CoreFoundation" "-framework IOKit" "-framework Security")\nendif()\n\n$1/' "$d/CMakeLists.txt"
  echo "  as2_platform_crazyflie: link CoreFoundation/IOKit/Security frameworks (libusb macOS backend)"
fi

# --- Lane 1b (shared, same commit daa44c9 all 3): libmavconn's own FindASIO.cmake
#     only searches PATHS /usr/include /usr/local/include for asio.hpp -> never
#     checks the Apple Silicon homebrew prefix -> "Could NOT find ASIO (missing:
#     ASIO_INCLUDE_DIRS)". brew asio isn't linked into /opt/homebrew/include on
#     this runner (not keg-only, but not globally linked either), so add both the
#     general homebrew include dir and the keg-specific opt/asio/include as
#     additional PATHS. Compile-tested: verified asio.hpp resolves from
#     /opt/homebrew/opt/asio/include locally (brew asio 1.36.0 installed). ---
d="$(_pkg_dir libmavconn)"
if [ -n "$d" ] && [ -f "$d/cmake/Modules/FindASIO.cmake" ] && ! grep -q '/opt/homebrew' "$d/cmake/Modules/FindASIO.cmake"; then
  sed "${SEDI[@]}" 's#PATHS /usr/include /usr/local/include#PATHS /usr/include /usr/local/include /opt/homebrew/include /opt/homebrew/opt/asio/include#' "$d/cmake/Modules/FindASIO.cmake"
  echo "  libmavconn: FindASIO.cmake PATHS += /opt/homebrew include dirs"
fi

# --- Lane 3 (humble + jazzy, same file/macro; only humble's yaml-cpp 0.7.0
#     actually hits this branch since jazzy's yaml_cpp_vendor exports a native
#     yaml-cpp::yaml-cpp target per the macro's own comment): ros2_medkit_cmake's
#     medkit_find_yaml_cpp() falls back to find_library(yaml-cpp)/find_path(...)
#     when no yaml-cpp::yaml-cpp target exists, but yaml_cpp_vendor's lib/include
#     live under the nested install/opt/yaml_cpp_vendor/{lib,include} prefix,
#     which isn't in CMAKE_PREFIX_PATH's default <prefix>/lib search -> both
#     find_library/find_path miss it -> FATAL_ERROR "Could not find yaml-cpp
#     library". cmake/toolchain.cmake already exports ${YAML_CPP_LIBRARIES} /
#     ${YAML_CPP_INCLUDE_DIRS} as absolute-path CACHE vars for exactly this
#     (same proven idiom as the rmf_traffic_editor fix) -> try them as a 3rd
#     fallback tier before giving up. Compile-tested: N/A (pure CMake macro
#     edit); verified structurally that YAML_CPP_LIBRARIES/YAML_CPP_INCLUDE_DIRS
#     are set unconditionally, globally, before any package configures. ---
d="$(_pkg_dir ros2_medkit_cmake)"
if [ -n "$d" ] && [ -f "$d/cmake/ROS2MedkitCompat.cmake" ] && ! grep -q 'elseif(YAML_CPP_LIBRARIES AND YAML_CPP_INCLUDE_DIRS)' "$d/cmake/ROS2MedkitCompat.cmake"; then
  perl -0pi -e 's/( *)else\(\)\n( *message\(FATAL_ERROR)/$1elseif(YAML_CPP_LIBRARIES AND YAML_CPP_INCLUDE_DIRS)\n$1  add_library(yaml-cpp::yaml-cpp IMPORTED INTERFACE)\n$1  set_target_properties(yaml-cpp::yaml-cpp PROPERTIES\n$1    INTERFACE_LINK_LIBRARIES "\${YAML_CPP_LIBRARIES}"\n$1    INTERFACE_INCLUDE_DIRECTORIES "\${YAML_CPP_INCLUDE_DIRS}"\n$1  )\n$1  message(STATUS "[MedkitCompat] yaml-cpp: created target from toolchain YAML_CPP_LIBRARIES")\n$1else()\n$2/' "$d/cmake/ROS2MedkitCompat.cmake"
  echo "  ros2_medkit_cmake: medkit_find_yaml_cpp() falls back to toolchain YAML_CPP_LIBRARIES/YAML_CPP_INCLUDE_DIRS"
fi

# --- Lane 1b (jazzy+kilted only, same commit e2600e98; absent from humble):
#     multisensor_calibration's own CMakeLists.txt:89 does bare
#     `find_package(tinyxml2 REQUIRED)` (no CONFIG keyword) -> CMake tries
#     Module mode first. ros2/tinyxml2_vendor installs a Find-module at
#     install/share/tinyxml2_vendor/cmake/Modules/FindTinyXML2.cmake (proper
#     ROS-style case) into the workspace's global CMAKE_MODULE_PATH. On
#     macOS's case-insensitive filesystem, CMake's search for
#     "Findtinyxml2.cmake" matches that file anyway (CMake itself emits an
#     author warning: "does not match the case of the module file name on
#     disk ... may fail on case-sensitive file systems") and includes it, but
#     it sets `TinyXML2_FOUND` (its own case), never the lowercase
#     `tinyxml2_FOUND` ament_target_dependencies() actually checks (it keys
#     off the literal name passed to find_package) -> "the passed package
#     name 'tinyxml2' was not found before". On Linux (case-sensitive), the
#     Module-mode search for "Findtinyxml2.cmake" simply misses and CMake
#     falls through to Config mode, finding brew/apt's real tinyxml2 config
#     and setting tinyxml2_FOUND correctly -> macOS-only bug, reproduced and
#     confirmed locally (a scratch CMake project with the same module path +
#     brew tinyxml2 config: bare find_package(tinyxml2) leaves tinyxml2_FOUND
#     empty; find_package(tinyxml2 CONFIG) sets it). Fix: force Config mode
#     for this one find_package call so it skips the module-mode collision
#     entirely, functionally identical to today's behavior on Linux.
#     Compile-tested: yes, real `cmake` configure reproducing both the bug
#     and the fix against brew's tinyxml2-config.cmake. ---
d="$(_pkg_dir multisensor_calibration)"
if [ -n "$d" ] && [ -f "$d/CMakeLists.txt" ] && grep -q '^find_package(tinyxml2 REQUIRED)$' "$d/CMakeLists.txt"; then
  sed "${SEDI[@]}" 's/^find_package(tinyxml2 REQUIRED)$/find_package(tinyxml2 REQUIRED CONFIG)/' "$d/CMakeLists.txt"
  echo "  multisensor_calibration: find_package(tinyxml2 REQUIRED CONFIG) — force Config mode, dodge case-insensitive Module-mode collision with tinyxml2_vendor's FindTinyXML2.cmake"
fi

# --- cloudini_lib: benchmarks/CMakeLists.txt's pcd_benchmark target does
#     `find_package(Draco QUIET)` then, when found, compiles pcd_benchmark.cpp
#     with -DDRACO_FOUND (activating real draco::EncoderBuffer/PointCloudBuilder/
#     ExpertEncoder calls) but links it via the bare variable ${DRACO_LIBRARIES}.
#     Homebrew's draco ships Config-mode only (draco-config.cmake ->
#     draco-targets.cmake), which defines the IMPORTED targets draco::draco /
#     draco::draco_static (carrying their own INTERFACE_INCLUDE_DIRECTORIES) and
#     never sets the old Module-mode DRACO_LIBRARIES/DRACO_INCLUDE_DIRS variables
#     at all -> the link line silently drops all draco symbols while the object
#     file still references them -> "Undefined symbols for architecture arm64"
#     (draco::EncoderBuffer::Clear() etc.) at the final link, after a clean
#     compile (headers already resolve via the global /opt/homebrew/include path
#     this toolchain adds for CLI11, so the missing DRACO_INCLUDE_DIRS never
#     surfaced as its own error). Fix: link the real imported target instead of
#     the always-empty variable, gated on Draco_FOUND via a generator expression
#     so an environment without draco still configures/links exactly as before
#     (empty $<BOOL:...> substitutes to nothing). Compile-tested: a real `cmake`
#     configure+build against the installed brew draco-config.cmake — old form
#     (${DRACO_LIBRARIES}) mirrors the CI failure once draco headers are on the
#     include path (undefined draco symbols at link); new form (draco::draco
#     target) links clean.
d="$(_pkg_dir cloudini_lib)"
f="$d/benchmarks/CMakeLists.txt"
if [ -n "$d" ] && [ -f "$f" ] && grep -qF '${DRACO_LIBRARIES}' "$f"; then
  sed "${SEDI[@]}" 's#\${DRACO_LIBRARIES}#$<$<BOOL:${Draco_FOUND}>:draco::draco>#' "$f"
  echo "  cloudini_lib: pcd_benchmark links draco::draco (imported target) instead of the never-set \${DRACO_LIBRARIES} — fixes undefined draco symbols at link"
fi

echo "source patches applied."
# --- cartographer: declares <depend>libceres-dev</depend> (Linux rosdep key), NOT the
#     vendored `ceres-solver` colcon package, so colcon does not order it after
#     ceres-solver. Both are 00_base_core and build in parallel -> cartographer's
#     find_package(Ceres REQUIRED) at CMakeLists:39 configures (~7.75s) before ceres
#     finishes (~15min) -> "Could not find Ceres". Inject a colcon build_depend so
#     ceres-solver builds first. (macOS-CI-only; on Linux ceres is the system pkg.) ---
d="$(_pkg_dir cartographer)"
if [ -n "$d" ] && [ -f "$d/package.xml" ] && ! grep -q '<build_depend>ceres-solver' "$d/package.xml"; then
  perl -0pi -e 's{(\n\s*<depend>libceres-dev</depend>)}{$1\n  <build_depend>ceres-solver</build_depend>}' "$d/package.xml"
  echo "  cartographer: +build_depend ceres-solver (colcon ordering vs find_package(Ceres) race)"
fi
# --- beluga: uses std::execution PSTL policies + std::is_execution_policy_v across 5 headers
#     (normalize/reweight/overlay/propagate/amcl_core). macOS libc++ ships no PSTL policies and
#     std::is_execution_policy_v is _LIBCPP_NO_SPECIALIZATIONS (always false, unspecializable), so
#     no external shim can satisfy the static_asserts. Drop in a compat header (policy types/objects
#     + serial transform/for_each overloads + beluga::is_execution_policy_v) and redirect
#     std::is_execution_policy_v -> beluga::is_execution_policy_v. Serial == correct (seq).
#     Compile+link-tested vs macOS libc++ (Apple clang 21). ---
d="$(_pkg_dir beluga)"
if [ -n "$d" ] && [ -d "$d/include/beluga" ] && [ ! -f "$d/include/beluga/detail/execution_policy_compat.hpp" ]; then
  mkdir -p "$d/include/beluga/detail"
  cp "$ROOT/ci/patches/beluga_execution_policy_compat.hpp" "$d/include/beluga/detail/execution_policy_compat.hpp"
  for f in $(grep -rl '#include <execution>' "$d/include/beluga" 2>/dev/null); do
    perl -0pi -e 's{#include <execution>}{#include <execution>\n#include <beluga/detail/execution_policy_compat.hpp>}' "$f"
    perl -0pi -e 's/std::is_execution_policy_v/beluga::is_execution_policy_v/g' "$f"
  done
  echo "  beluga: PSTL execution-policy compat shim + std::is_execution_policy_v redirect"
fi

# --- kuka_external_control_sdk: iiqka/CMakeLists.txt hardcodes the grpc codegen plugin as
#     "protoc-gen-grpc=/usr/bin/grpc_cpp_plugin" (the Debian path) -> "program not found" on
#     macOS, where it is Homebrew's /opt/homebrew/bin/grpc_cpp_plugin. Repoint it. ---
d="$(_pkg_dir kuka_external_control_sdk)"
if [ -n "$d" ]; then
  for f in $(grep -rl '/usr/bin/grpc_cpp_plugin' "$d" 2>/dev/null); do
    sed "${SEDI[@]}" 's#/usr/bin/grpc_cpp_plugin#/opt/homebrew/bin/grpc_cpp_plugin#g' "$f"
    echo "  kuka_external_control_sdk: grpc_cpp_plugin /usr/bin -> /opt/homebrew/bin ($f)"
  done
fi

# --- kuka_external_control_sdk: iiqka's protobuf target puts only the generated-code
#     binary dir on its include path, never ${Protobuf_INCLUDE_DIRS}. The generated
#     .pb.h does #include "google/protobuf/runtime_version.h", so it then depends on the
#     toolchain's global -I/opt/homebrew/include -> EMPTY after the gz step's
#     `brew unlink protobuf` -> "runtime_version.h file not found". kuka's FindProtobuf
#     resolves protoc AND headers to the SAME protobuf (32.0, via ros2/workspace prefix,
#     which HAS runtime_version.h) — it just never adds that include dir to the target.
#     Add ${Protobuf_INCLUDE_DIRS} so the resolved protobuf's headers are on the path,
#     matching the protoc that generated the code. Validated: kuka's FindProtobuf module
#     resolves Protobuf_INCLUDE_DIRS to a dir containing runtime_version.h. ---
for _kc in $(find "$ROOT" -path '*kuka_external_control_sdk/iiqka/CMakeLists.txt' -not -path '*/build/*' -not -path '*/install/*' 2>/dev/null); do
  if ! grep -q 'Protobuf_INCLUDE_DIRS' "$_kc"; then
    perl -0pi -e 's{(target_include_directories\(kuka-iiqka-client-library-protobuf PUBLIC\n\s*"\$<BUILD_INTERFACE:\$\{CMAKE_CURRENT_BINARY_DIR\}>"\n)}{$1  \$\{Protobuf_INCLUDE_DIRS\}\n}' "$_kc"
    echo "  kuka_external_control_sdk: +\${Protobuf_INCLUDE_DIRS} to iiqka protobuf target (runtime_version.h after brew unlink)"
  fi
done

# --- foxglove_bridge: its CMakeLists only has download arms for Linux aarch64/x86_64 and
#     message(FATAL_ERROR) on every other platform. Foxglove DOES publish a macOS build
#     (foxglove-v<ver>-cpp-aarch64-apple-darwin.zip). Add a Darwin/arm64 elseif before the
#     else() so the prebuilt SDK is fetched on macOS. (SHA256 verified for v0.27.0.) PR-able. ---
d="$(_pkg_dir foxglove_bridge)"
if [ -n "$d" ] && [ -f "$d/CMakeLists.txt" ] && ! grep -q 'aarch64-apple-darwin' "$d/CMakeLists.txt"; then
  perl -0pi -e 's{(set\(FOXGLOVE_SDK_SHA "4790dad[0-9a-f]+"\)\s*\n)(\s*else\(\))}{$1elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")\n  set(FOXGLOVE_SDK_PLATFORM "aarch64-apple-darwin")\n  set(FOXGLOVE_SDK_SHA "70c6e59cf757ac0480912c13cb2c630a3839de34eba428ef717411dae6b1688e")\n$2}' "$d/CMakeLists.txt"
  echo "  foxglove_bridge: +Darwin/arm64 SDK download arm"
fi

# --- foxglove_bridge: src/message_definition_cache.cpp uses std::map<std::string,
#     std::string> but never #includes <map>. libstdc++ pulls it in transitively;
#     libc++ (Apple clang) does not -> "implicit instantiation of undefined template
#     'std::map'". Add the include. Compile-tested (Apple clang, libc++, -std=c++17):
#     reproduces the error without it, clean with it. ---
for _mdc in $(find "$ROOT" -path '*foxglove_bridge/src/message_definition_cache.cpp' -not -path '*/build/*' -not -path '*/install/*' 2>/dev/null); do
  if ! grep -qE '#include <map>' "$_mdc"; then
    perl -0pi -e 's{(#include <set>)}{$1\n#include <map>}' "$_mdc"
    echo "  foxglove_bridge: +#include <map> in ${_mdc#$ROOT/}"
  fi
done

# --- point_cloud_msg_wrapper: header uses <experimental/optional> and
#     std::experimental::optional, both removed from libc++ (Apple clang) ->
#     "'experimental/optional' file not found". Map to the C++17 <optional> /
#     std::optional. Compile-tested (Apple clang, libc++, -std=c++17). ---
for _f in $(find "$ROOT" -path '*point_cloud_msg_wrapper/point_cloud_msg_wrapper.hpp' -not -path '*/build/*' -not -path '*/install/*' 2>/dev/null); do
  if grep -q 'experimental/optional' "$_f"; then
    sed "${SEDI[@]}" -e 's#<experimental/optional>#<optional>#g' -e 's/std::experimental::optional/std::optional/g' "$_f"
    echo "  point_cloud_msg_wrapper: <experimental/optional> -> <optional> in ${_f#$ROOT/}"
  fi
done

# --- turtlebot3_panorama: panorama.hpp gates cv_bridge include on ROS2_HUMBLE /
#     ROS2_LATEST macros; on humble that macro is not defined in this build, so it
#     falls to the .hpp branch, but humble's cv_bridge ships only cv_bridge.h ->
#     "'cv_bridge/cv_bridge.hpp' file not found". Replace the macro gate with
#     __has_include so it picks .hpp where present (jazzy+) and .h otherwise. ---
for _f in $(find "$ROOT" -path '*turtlebot3_panorama/include/turtlebot3_panorama/panorama.hpp' -not -path '*/build/*' -not -path '*/install/*' 2>/dev/null); do
  if grep -q 'ROS2_HUMBLE' "$_f"; then
    perl -0pi -e 's{#ifdef ROS2_HUMBLE\n\s*#include <cv_bridge/cv_bridge\.h>\n#elif defined\(ROS2_LATEST\)\n\s*#include <cv_bridge/cv_bridge\.hpp>\n#endif}{#if __has_include(<cv_bridge/cv_bridge.hpp>)\n  #include <cv_bridge/cv_bridge.hpp>\n#else\n  #include <cv_bridge/cv_bridge.h>\n#endif}' "$_f"
    echo "  turtlebot3_panorama: cv_bridge include -> __has_include in ${_f#$ROOT/}"
  fi
done

# --- sick_scan_xd: CMakeLists adds `-fno-var-tracking-assignments` (a GCC-only
#     codegen debug flag) to CMAKE_CXX_FLAGS under `if(NOT WIN32)`, which includes
#     macOS. Apple clang HARD-errors on it: "unknown argument:
#     '-fno-var-tracking-assignments'". Strip it (leave -Wno-format-overflow, which
#     the toolchain's -Wno-unknown-warning-option already tolerates). ---
for _f in $(find "$ROOT" -path '*sick_scan_xd/CMakeLists.txt' -not -path '*/build/*' -not -path '*/install/*' 2>/dev/null); do
  if grep -q 'fno-var-tracking-assignments' "$_f"; then
    sed "${SEDI[@]}" 's/ -fno-var-tracking-assignments//g' "$_f"
    echo "  sick_scan_xd: strip GCC-only -fno-var-tracking-assignments in ${_f#$ROOT/}"
  fi
done

# --- ess_imu_driver2: links Linux-only libs `crypt` (libcrypt) and `rt` (librt)
#     in target_link_libraries(ess_imu_driver2_node ...). macOS has crypt() and the
#     realtime clock funcs in libSystem (no separate libcrypt/librt) -> "ld: library
#     'crypt' not found". Strip both standalone link lines (macOS-only CI patch). ---
for _f in $(find "$ROOT" -path '*ess_imu_driver2/CMakeLists.txt' -not -path '*/build/*' -not -path '*/install/*' 2>/dev/null); do
  if grep -qE '^[[:space:]]*crypt[[:space:]]*$' "$_f"; then
    perl -ni -e 'print unless /^\s*(crypt|rt)\s*$/' "$_f"
    echo "  ess_imu_driver2: strip Linux-only crypt/rt link libs in ${_f#$ROOT/}"
  fi
done

# --- roadmap_explorer: Logger.hpp calls system_clock::to_time_t(high_resolution_clock
#     ::now()). libstdc++ makes high_resolution_clock an alias of system_clock (works);
#     libc++ (Apple clang) makes it steady_clock -> "no viable conversion from
#     time_point<steady_clock,...> to time_point<system_clock,...>". Use system_clock::
#     now() directly (these are wall-clock log timestamps). ---
for _f in $(find "$ROOT" -path '*roadmap_explorer*/util/Logger.hpp' -not -path '*/build/*' -not -path '*/install/*' 2>/dev/null); do
  if grep -q 'high_resolution_clock::now()' "$_f"; then
    sed "${SEDI[@]}" 's/high_resolution_clock::now()/system_clock::now()/g' "$_f"
    echo "  roadmap_explorer: high_resolution_clock -> system_clock in ${_f#$ROOT/}"
  fi
done

# --- GNU-ld link flags Apple ld64 rejects ("ld: unknown options: ..."): strip on macOS.
#     rosgraph_monitor add_link_options(-Wl,--no-undefined) under an "OR Clang" guard that
#     AppleClang matches -> applied; ublox_dgnss_node LINKER:--allow-multiple-definition
#     unguarded. (broll's --no-undefined is already NOT-APPLE-guarded -> no-op.) These are
#     macOS-CI-only strips; the flags stay on Linux/upstream. ---
for _f in $(find "$ROOT" \( -path '*rosgraph_monitor/CMakeLists.txt' -o -path '*ublox_dgnss_node/CMakeLists.txt' -o -path '*rosbag2_broll/broll/CMakeLists.txt' \) -not -path '*/build/*' -not -path '*/install/*' 2>/dev/null); do
  if grep -qE '"-Wl,--no-undefined"|LINKER:--allow-multiple-definition' "$_f"; then
    perl -ni -e 'print unless /"-Wl,--no-undefined"|LINKER:--allow-multiple-definition/' "$_f"
    echo "  strip GNU-ld flag (no-undefined/allow-multiple-definition) in ${_f#$ROOT/}"
  fi
done

# --- camera_aravis2: its bundled CMakeModules/FindARAVIS.cmake searches only Linux
#     include paths (/usr/local/include/aravis-X, /usr/include/aravis-X) for arv.h, so
#     the header isn't found on macOS (brew puts it under /opt/homebrew/include/aravis-X)
#     -> "Could NOT find ARAVIS (missing ARAVIS_INCLUDE_DIRS)". Add the brew paths.
#     Compile-tested locally (aravis 0.8.36): find_package(ARAVIS) FOUND=TRUE. ---
for _f in $(find "$ROOT" -path '*camera_aravis2/CMakeModules/FindARAVIS.cmake' -not -path '*/build/*' -not -path '*/install/*' 2>/dev/null); do
  if ! grep -q 'homebrew/include/aravis' "$_f"; then
    perl -0pi -e 's{(  /usr/include/aravis-\$\{ARAVIS_VERSION\}\n)}{$1  /opt/homebrew/include/aravis-\$\{ARAVIS_VERSION\}\n  /opt/homebrew/opt/aravis/include/aravis-\$\{ARAVIS_VERSION\}\n}' "$_f"
    echo "  camera_aravis2 FindARAVIS: +brew include paths in ${_f#$ROOT/}"
  fi
done

# --- yaml_cpp_vendor: the vendored yaml-cpp-config sets
#     YAML_CPP_LIBRARIES="@EXPORT_TARGETS@" -> the BARE name "yaml-cpp", so every
#     consumer using ${YAML_CPP_LIBRARIES} emits a fragile -lyaml-cpp AND clobbers the
#     toolchain's absolute-path override. Patch yaml-cpp's config template to set
#     YAML_CPP_LIBRARIES to the imported target yaml-cpp::yaml-cpp (carries the absolute
#     .dylib path; both versions define it via install(EXPORT NAMESPACE yaml-cpp::)).
#     humble (0.7.0 ExternalProject, unforked upstream): add a PATCH_COMMAND here.
#     jazzy/kilted (0.8.0) carry the same change as patches/0002 IN the id_yaml_cpp_vendor
#     fork (ament_vendor globs patches/). Tested: git apply -p1 clean on both tags. ---
d="$(_pkg_dir yaml_cpp_vendor)"
if [ -n "$d" ] && [ -f "$d/CMakeLists.txt" ]; then
  if grep -q 'ExternalProject_Add(yaml_cpp-0.7.0' "$d/CMakeLists.txt" && ! grep -q 'yaml-cpp-libraries-target' "$d/CMakeLists.txt"; then
    mkdir -p "$d/patches"
    cat > "$d/patches/0001-yaml-cpp-libraries-target.patch" <<'PATCHEOF'
diff --git a/yaml-cpp-config.cmake.in b/yaml-cpp-config.cmake.in
index 7b41e3f..a9f513e 100644
--- a/yaml-cpp-config.cmake.in
+++ b/yaml-cpp-config.cmake.in
@@ -11,4 +11,4 @@ set(YAML_CPP_INCLUDE_DIR "@CONFIG_INCLUDE_DIRS@")
 include("${YAML_CPP_CMAKE_DIR}/yaml-cpp-targets.cmake")
 
 # These are IMPORTED targets created by yaml-cpp-targets.cmake
-set(YAML_CPP_LIBRARIES "@EXPORT_TARGETS@")
+set(YAML_CPP_LIBRARIES yaml-cpp::yaml-cpp)
PATCHEOF
    perl -0pi -e 's{(\n[ \t]*URL_MD5 [0-9a-f]+\n)}{$1    PATCH_COMMAND git apply --whitespace=nowarn -p1 \$\{CMAKE_CURRENT_SOURCE_DIR\}/patches/0001-yaml-cpp-libraries-target.patch\n}' "$d/CMakeLists.txt"
    echo "  yaml_cpp_vendor (0.7.0): +patches/0001 + PATCH_COMMAND (YAML_CPP_LIBRARIES -> yaml-cpp::yaml-cpp target)"
  fi
fi

# --- ouster_ros: ouster_client bundles spdlog+fmt under ouster-sdk/thirdparty, but the
#     toolchain's global `-isystem /opt/homebrew/include` (brew spdlog, EXTERNAL fmt) is
#     searched BEFORE ouster's bundled `-isystem .../thirdparty`. brew spdlog has no
#     fmt/bundled/, so <spdlog/spdlog.h> resolves to brew (fmt 11) while
#     <spdlog/fmt/bundled/args.h> falls through to ouster's bundled fmt -> two fmt versions
#     in one TU ("redefinition of 'monostate'", "template parameter redefines default arg").
#     Add the bundled thirdparty as a plain -I (non-SYSTEM, prepended): -I always beats
#     -isystem, so ALL <spdlog/*> resolve to the bundled copy -> one fmt. Compile-tested
#     (Apple clang 21): reproduces 20 errors without, 0 with. ---
for f in $(find . -path '*ouster-sdk/ouster_client/CMakeLists.txt' 2>/dev/null); do
  grep -q 'BEFORE PRIVATE.*thirdparty>' "$f" && continue
  perl -0pi -e 's{(target_include_directories\(ouster_client SYSTEM)}{target_include_directories(ouster_client BEFORE PRIVATE \$<BUILD_INTERFACE:\${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty>)\n$1}' "$f"
  echo "  ouster_client: bundled thirdparty as non-SYSTEM -I (brew-fmt vs bundled-fmt clash) ($f)"
done

# --- autoware_map_projection_loader (humble+jazzy, identical commit
#     9472b3df, absent kilted -- package doesn't exist in kilted's tree) --
#     same bare-`yaml-cpp` linker-name bug as rmf_traffic_editor/
#     rmf_traffic_ros2/nebula_velodyne_common above: CMakeLists.txt:21 is
#     `target_link_libraries(${PROJECT_NAME} yaml-cpp)` -- not a real CMake
#     target in this scope, so it becomes a raw `-lyaml-cpp` linker flag ->
#     "ld: library 'yaml-cpp' not found" linking libautoware_map_projection_
#     loader.dylib. Swapped for the toolchain's own ${YAML_CPP_LIBRARIES}
#     absolute-path CACHE var (same proven idiom as the 3 prior fixes, and
#     already used natively upstream by 8+ other in-tree packages per the
#     toolchain's own comment). Not compile-tested end-to-end (same stated
#     limitation as rmf_traffic_editor/rmf_traffic_ros2/nebula_velodyne_common:
#     needs a populated local yaml_cpp_vendor install/ tree to actually
#     link-test, not available in this checkout, and brew's yaml-cpp is not a
#     substitute per PLAYBOOK.md's "never use brew yaml" rule); verified
#     structurally against the real CMakeLists.txt in both trees and
#     idempotency-checked (2nd run: no match, no double-replace). ---
d="$(_pkg_dir autoware_map_projection_loader)"
if [ -n "$d" ] && [ -f "$d/CMakeLists.txt" ] && grep -q 'target_link_libraries(${PROJECT_NAME} yaml-cpp)' "$d/CMakeLists.txt"; then
  sed "${SEDI[@]}" \
    -e 's/target_link_libraries(${PROJECT_NAME} yaml-cpp)/target_link_libraries(${PROJECT_NAME} ${YAML_CPP_LIBRARIES})/' \
    "$d/CMakeLists.txt"
  echo "  autoware_map_projection_loader: bare yaml-cpp -> \${YAML_CPP_LIBRARIES}"
fi

# --- autoware_test_utils (humble+jazzy, identical commit 9472b3df, absent
#     kilted) -- same bare-`yaml-cpp` linker-name bug as
#     autoware_map_projection_loader/rmf_traffic_editor/rmf_traffic_ros2/
#     nebula_velodyne_common above, but 2 separate occurrences in one file:
#     `target_link_libraries(autoware_test_utils\n  yaml-cpp\n)` (multi-line)
#     and `target_link_libraries(topic_snapshot_saver autoware_test_utils
#     yaml-cpp)` (single-line). Despite `find_package(yaml-cpp REQUIRED)`
#     succeeding (yaml_cpp_vendor exports a native, but NAMESPACED,
#     `yaml-cpp::yaml-cpp` target on jazzy/kilted's yaml-cpp 0.8.0 -- no bare
#     `yaml-cpp` alias), the bare name in target_link_libraries() doesn't
#     match any known target -> raw `-lyaml-cpp` linker flag -> "ld: library
#     'yaml-cpp' not found". Swapped both for ${YAML_CPP_LIBRARIES}. Not
#     compile-tested end-to-end (same stated limitation as the other 3
#     yaml-cpp fixes -- no populated local yaml_cpp_vendor install/ tree, and
#     brew's yaml-cpp is not a substitute per PLAYBOOK.md); verified
#     structurally against the real CMakeLists.txt (both occurrences replaced,
#     diff clean) and idempotency-checked (2nd run: no match, no
#     double-replace). ---
d="$(_pkg_dir autoware_test_utils)"
if [ -n "$d" ] && [ -f "$d/CMakeLists.txt" ] && grep -qE '^  yaml-cpp$' "$d/CMakeLists.txt"; then
  sed "${SEDI[@]}" \
    -e 's/^  yaml-cpp$/  ${YAML_CPP_LIBRARIES}/' \
    -e 's/target_link_libraries(topic_snapshot_saver autoware_test_utils yaml-cpp)/target_link_libraries(topic_snapshot_saver autoware_test_utils ${YAML_CPP_LIBRARIES})/' \
    "$d/CMakeLists.txt"
  echo "  autoware_test_utils: bare yaml-cpp (x2) -> \${YAML_CPP_LIBRARIES}"
fi

# --- sync_tooling_msgs (humble+jazzy, identical commit e631c5f0, absent
#     kilted) -- CMakeLists.txt does `find_package(Protobuf 3.12 REQUIRED)`
#     (CMake's bundled Module-mode FindProtobuf.cmake) and links the legacy
#     ${Protobuf_LIBRARIES} var (a bare path to libprotobuf.dylib). Two
#     stacked bugs, only the first visible in CI (compile fails before the
#     link step is ever reached):
#       1. target_include_directories() never adds ${Protobuf_INCLUDE_DIRS}
#          (same class as the kuka_external_control_sdk fix above) -> the
#          generated .pb.h's #include "google/protobuf/runtime_version.h"
#          falls through to the toolchain's global include path, which is
#          empty after `brew unlink protobuf` -> "file not found".
#       2. Even once headers resolve, linking the bare libprotobuf.dylib via
#          ${Protobuf_LIBRARIES} (Module-mode's legacy variable) omits
#          protobuf's Abseil dependencies (modern protobuf/Abseil split the
#          runtime; the real link requirements only live on the CONFIG
#          package's protobuf::libprotobuf IMPORTED target's
#          INTERFACE_LINK_LIBRARIES, ~35 absl:: components) -> "ld: symbol(s)
#          not found for architecture arm64" (undefined absl::log_internal::*
#          etc, referenced by the generated .pb.cc files) — only surfaces
#          after fixing bug 1, so CI never got far enough to show it.
#     Real fix: switch to CONFIG mode (`find_package(Protobuf REQUIRED
#     CONFIG)`, dropping the "3.12" version pin -- protobuf's CONFIG version
#     file enforces same-major-version compat and rejects "3.12" against the
#     installed 32.x/6.32.x series, so a versioned CONFIG request errors
#     outright; 3.12 was a vestigial floor far below anything actually
#     installed) and link protobuf::libprotobuf directly, which carries both
#     its own include dirs AND the full Abseil link set as usage
#     requirements -- no manual ${Protobuf_INCLUDE_DIRS}/absl:: list needed.
#     protobuf::protoc's IMPORTED_LOCATION isn't set by this brew formula
#     (only the *_RELEASE config-suffixed property is), so derive
#     Protobuf_PROTOC_EXECUTABLE from that with a plain-property fallback for
#     robustness across protobuf formula revisions.
#     Compile+link-tested end-to-end: real protoc codegen + cmake build of
#     all 27 real .proto files from this submodule (not a toy proto),
#     against real brew protobuf 32.0/Abseil, using this exact transform --
#     rc=0, produced libsync_tooling_msgs.dylib linking cleanly against
#     libprotobuf + ~15 libabsl_*.dylib (verified via otool -L). The naive
#     "just add Protobuf_INCLUDE_DIRS" fix (bug 1 alone, matching the
#     kuka_external_control_sdk precedent) was tried FIRST and confirmed via
#     this same local probe to compile but then fail at link with the
#     Abseil symbols -- not committed, since PLAYBOOK.md says don't commit a
#     transform known to be incomplete. ---
d="$(_pkg_dir sync_tooling_msgs)"
if [ -n "$d" ] && [ -f "$d/CMakeLists.txt" ] && grep -q 'find_package(Protobuf 3.12 REQUIRED)' "$d/CMakeLists.txt"; then
  perl -0pi -e 's/find_package\(Protobuf 3\.12 REQUIRED\)/find_package(Protobuf REQUIRED CONFIG)\nget_target_property(Protobuf_PROTOC_EXECUTABLE protobuf::protoc IMPORTED_LOCATION_RELEASE)\nif(NOT Protobuf_PROTOC_EXECUTABLE)\n  get_target_property(Protobuf_PROTOC_EXECUTABLE protobuf::protoc IMPORTED_LOCATION)\nendif()/' "$d/CMakeLists.txt"
  sed "${SEDI[@]}" 's/target_link_libraries(sync_tooling_msgs PUBLIC \${Protobuf_LIBRARIES})/target_link_libraries(sync_tooling_msgs PUBLIC protobuf::libprotobuf)/' "$d/CMakeLists.txt"
  echo "  sync_tooling_msgs: Protobuf Module->CONFIG mode + protobuf::libprotobuf (Abseil link deps)"
fi


# --- broll (ros-tooling/rosbag2_broll, same commit all 3 present): decode_node_component
#     hardcodes `target_link_options(... "-Wl,--no-undefined")` (GNU-ld-only flag) ->
#     "ld: unknown options: --no-undefined". Apple's ld64 already defaults to erroring on
#     undefined symbols for a shared lib with no `-undefined dynamic_lookup` override, so
#     dropping the flag on macOS is a no-op behavior change (same reasoning as the
#     rmw_stats_shim --no-undefined fix). ---
d="$(_pkg_dir broll)"
if [ -n "$d" ] && [ -f "$d/CMakeLists.txt" ] && grep -qE '^target_link_options\(decode_node_component PRIVATE "-Wl,--no-undefined"\)$' "$d/CMakeLists.txt"; then
  perl -0pi -e 's/target_link_options\(decode_node_component PRIVATE "-Wl,--no-undefined"\)/if(NOT APPLE)\n  target_link_options(decode_node_component PRIVATE "-Wl,--no-undefined")\nendif()/' "$d/CMakeLists.txt"
  echo "  broll: guard GNU-ld-only -Wl,--no-undefined with if(NOT APPLE)"
fi

# --- ffw_joint_trajectory_command_broadcaster (jazzy only): -Wsign-conversion promoted to
#     -Werror. int32_t delay_ns implicitly narrows to the uint32_t nanoseconds param of
#     rclcpp::Duration(int32_t, uint32_t). Explicit cast, no behavior change. ---
d="$(_pkg_dir ffw_joint_trajectory_command_broadcaster)"
if [ -n "$d" ] && [ -f "$d/src/joint_trajectory_command_broadcaster.cpp" ] && grep -q 'rclcpp::Duration(0, delay_ns)' "$d/src/joint_trajectory_command_broadcaster.cpp"; then
  sed "${SEDI[@]}" 's/rclcpp::Duration(0, delay_ns)/rclcpp::Duration(0, static_cast<uint32_t>(delay_ns))/' "$d/src/joint_trajectory_command_broadcaster.cpp"
  echo "  ffw_joint_trajectory_command_broadcaster: explicit cast delay_ns -> uint32_t"
fi

# --- husarion_ugv_lights (humble+jazzy, different per-distro commits but identical
#     CMakeLists construct): bare `yaml-cpp` as a target_link_libraries() name on the two
#     real (non-test) targets -> "ld: library 'yaml-cpp' not found". Same class of bug as
#     rmf_traffic_editor/nebula_velodyne_common/autoware_test_utils; swap for
#     ${YAML_CPP_LIBRARIES}. ---
d="$(_pkg_dir husarion_ugv_lights)"
if [ -n "$d" ] && [ -f "$d/CMakeLists.txt" ] && grep -qE '^target_link_libraries\(animation_plugins png yaml-cpp\)$' "$d/CMakeLists.txt"; then
  sed "${SEDI[@]}" \
    -e 's/^target_link_libraries(animation_plugins png yaml-cpp)$/target_link_libraries(animation_plugins png ${YAML_CPP_LIBRARIES})/' \
    -e 's/^target_link_libraries(lights_controller_node_component yaml-cpp$/target_link_libraries(lights_controller_node_component ${YAML_CPP_LIBRARIES}/' \
    "$d/CMakeLists.txt"
  echo "  husarion_ugv_lights: bare yaml-cpp (x2, non-test targets) -> \${YAML_CPP_LIBRARIES}"
fi

# --- ros_babel_fish_tools: same bare-yaml-cpp-as-linker-name bug, single INTERFACE
#     target. ---
d="$(_pkg_dir ros_babel_fish_tools)"
if [ -n "$d" ] && [ -f "$d/CMakeLists.txt" ] && grep -qE '^  yaml-cpp$' "$d/CMakeLists.txt"; then
  sed "${SEDI[@]}" 's/^  yaml-cpp$/  ${YAML_CPP_LIBRARIES}/' "$d/CMakeLists.txt"
  echo "  ros_babel_fish_tools: bare yaml-cpp -> \${YAML_CPP_LIBRARIES}"
fi

# --- ur_client_library (all 3, identical commit, vendored 3rdparty/httplib/httplib.h):
#     `#elif defined SOCK_CLOEXEC` assumes SOCK_CLOEXEC-defined implies accept4() exists.
#     True on Linux; macOS/Darwin *does* define the SOCK_CLOEXEC constant (for use with
#     socket()) but has no accept4() syscall at all -> "use of undeclared identifier
#     'accept4'". Narrow the branch to Linux specifically; macOS falls through to the
#     plain accept() branch (functionally equivalent minus the atomic CLOEXEC race, which
#     httplib's own #else path already accepts as fine on platforms without accept4). ---
d="$(_pkg_dir ur_client_library)"
f="$d/3rdparty/urcl_3rdparty/httplib/httplib.h"
if [ -n "$d" ] && [ -f "$f" ] && grep -q '#elif defined SOCK_CLOEXEC$' "$f"; then
  perl -0pi -e 's/#elif defined SOCK_CLOEXEC\n(\s*socket_t sock = accept4\(svr_sock_, nullptr, nullptr, SOCK_CLOEXEC\);\n)/#elif defined(SOCK_CLOEXEC) \&\& defined(__linux__)\n$1/' "$f"
  echo "  ur_client_library: vendored httplib.h accept4 branch -> Linux-only (macOS has no accept4)"
fi

# --- ublox_gps (all 3, identical commit): own cmake/Findasio.cmake does a bare
#     find_path(asio.hpp) with NO PATHS at all -> never finds brew's keg-only-adjacent
#     asio (installed at /opt/homebrew/opt/asio/include, not always symlinked into
#     /opt/homebrew/include). Same class of fix as the libmavconn FindASIO.cmake patch
#     above (Lane 1b), different file. ---
d="$(_pkg_dir ublox_gps)"
f="$d/cmake/Findasio.cmake"
if [ -n "$d" ] && [ -f "$f" ] && grep -q 'find_path(ASIO_INCLUDE_DIR NAMES asio.hpp)' "$f"; then
  sed "${SEDI[@]}" 's#find_path(ASIO_INCLUDE_DIR NAMES asio.hpp)#find_path(ASIO_INCLUDE_DIR NAMES asio.hpp PATHS /opt/homebrew/include /opt/homebrew/opt/asio/include)#' "$f"
  echo "  ublox_gps: Findasio.cmake += /opt/homebrew include dirs"
fi

# --- cx_config_plugin (ros-drivers/clips_executive, jazzy+kilted, dir named
#     config_plugin): `if(NOT CMAKE_CXX_STANDARD) set(CMAKE_CXX_STANDARD 20) endif()`
#     never fires because the repo-wide cmake/toolchain.cmake already sets
#     CMAKE_CXX_STANDARD=17 (applied via CMAKE_TOOLCHAIN_FILE before project()) -> the
#     package silently builds at C++17 -> std::string::starts_with/ends_with (C++20)
#     don't exist -> "no member named 'starts_with'". Force 20 unconditionally for this
#     package (matches its own stated intent), overriding the toolchain default just for
#     this target's directory scope. ---
d="$(_pkg_dir config_plugin)"
if [ -n "$d" ] && [ -f "$d/CMakeLists.txt" ] && grep -qF 'if(NOT CMAKE_CXX_STANDARD)' "$d/CMakeLists.txt"; then
  perl -0pi -e 's/if\(NOT CMAKE_CXX_STANDARD\)\n  set\(CMAKE_CXX_STANDARD 20\)\nendif\(\)/set(CMAKE_CXX_STANDARD 20)/' "$d/CMakeLists.txt"
  echo "  cx_config_plugin: force CMAKE_CXX_STANDARD 20 (toolchain's global 17 was defeating the guard)"
fi

# --- SMACC2 nav2z_client custom_planners family (backward_local_planner,
#     forward_local_planner, undo_path_global_planner): CMakeLists' `dependencies` list
#     (read by ament_target_dependencies()) already includes `visualization_msgs`, but
#     there is no `find_package(visualization_msgs)` call above it -> "ament_target_
#     dependencies() the passed package name 'visualization_msgs' was not found before".
#     Works on Linux only by accident (some other found package transitively drags the
#     include path in); exposed on this macOS toolchain. Add the missing find_package
#     call next to the package's other find_package() lines. ---
for _pkg in backward_local_planner forward_local_planner undo_path_global_planner; do
  d="$(_pkg_dir "$_pkg")"
  if [ -n "$d" ] && [ -f "$d/CMakeLists.txt" ] && grep -q '^  visualization_msgs$' "$d/CMakeLists.txt" && ! grep -q 'find_package(visualization_msgs)' "$d/CMakeLists.txt"; then
    perl -pi -e 's/^find_package\(ament_cmake REQUIRED\)$/find_package(ament_cmake REQUIRED)\nfind_package(visualization_msgs)/' "$d/CMakeLists.txt"
    echo "  $_pkg: add missing find_package(visualization_msgs)"
  fi
done

# --- pure_spinning_local_planner: same nav2z_client family, but visualization_msgs is
#     used directly in the header (#include <visualization_msgs/msg/marker_array.hpp>)
#     without being declared anywhere (not in `dependencies`, not in package.xml) ->
#     "file not found" at compile time (fails earlier than the ament_target_dependencies
#     check the sibling planners hit). Add find_package + dependencies-list entry +
#     package.xml depend. ---
d="$(_pkg_dir pure_spinning_local_planner)"
if [ -n "$d" ] && [ -f "$d/CMakeLists.txt" ] && ! grep -q 'visualization_msgs' "$d/CMakeLists.txt"; then
  perl -pi -e 's/^find_package\(ament_cmake REQUIRED\)$/find_package(ament_cmake REQUIRED)\nfind_package(visualization_msgs)/' "$d/CMakeLists.txt"
  perl -0pi -e 's/(set\(dependencies\n  nav2_core\n)/$1  visualization_msgs\n/' "$d/CMakeLists.txt"
  echo "  pure_spinning_local_planner: add find_package(visualization_msgs) + dependencies-list entry"
fi
if [ -n "$d" ] && [ -f "$d/package.xml" ] && ! grep -q '<depend>visualization_msgs</depend>' "$d/package.xml"; then
  perl -0pi -e 's{(<depend>geometry_msgs</depend>)}{$1\n  <depend>visualization_msgs</depend>}' "$d/package.xml"
  echo "  pure_spinning_local_planner: add package.xml <depend>visualization_msgs</depend>"
fi

# --- forward_global_planner + undo_path_global_planner (SMACC2 nav2z_client): real,
#     platform-independent upstream API drift (would fail identically on Linux with the
#     same nav2_core version) -- nav2_core::GlobalPlanner::createPlan() gained a 3rd
#     parameter, `std::function<bool()> cancel_checker`, but these two plugins still
#     override the old 2-param signature -> "allocating an object of abstract class type"
#     (pure virtual createPlan never actually overridden). Add the 3rd parameter to both
#     the header declaration and the .cpp definition; body ignores it (neither planner
#     currently supports mid-plan cancellation, matching their pre-existing behavior). ---
for _pkg in forward_global_planner undo_path_global_planner; do
  d="$(_pkg_dir "$_pkg")"
  [ -z "$d" ] && continue
  hf="$(find "$d/include" -iname "${_pkg}.hpp" 2>/dev/null | head -1)"
  cf="$(find "$d/src" -iname "${_pkg}.cpp" 2>/dev/null | head -1)"
  if [ -n "$hf" ] && grep -qF 'const geometry_msgs::msg::PoseStamped & start, const geometry_msgs::msg::PoseStamped & goal);' "$hf"; then
    sed "${SEDI[@]}" 's/const geometry_msgs::msg::PoseStamped \& start, const geometry_msgs::msg::PoseStamped \& goal);/const geometry_msgs::msg::PoseStamped \& start, const geometry_msgs::msg::PoseStamped \& goal,\n    std::function<bool()> cancel_checker) override;/' "$hf"
    echo "  $_pkg: createPlan header += cancel_checker param (nav2_core::GlobalPlanner API drift)"
  fi
  if [ -n "$cf" ] && grep -qF 'const geometry_msgs::msg::PoseStamped & start, const geometry_msgs::msg::PoseStamped & goal)' "$cf"; then
    sed "${SEDI[@]}" 's/const geometry_msgs::msg::PoseStamped \& start, const geometry_msgs::msg::PoseStamped \& goal)$/const geometry_msgs::msg::PoseStamped \& start, const geometry_msgs::msg::PoseStamped \& goal,\n  std::function<bool()> \/*cancel_checker*\/)/' "$cf"
    echo "  $_pkg: createPlan definition += cancel_checker param"
  fi
done

# --- plansys2_bringup (SMACC2-adjacent, all 3 different per-distro commits, same
#     construct): plansys2_node.cpp calls the raw Linux syscall wrapper
#     sched_setscheduler(0, SCHED_FIFO, &sch) with no include and no macOS path ->
#     "use of undeclared identifier 'sched_setscheduler'". macOS has no equivalent
#     process-wide syscall; the closest portable analog is pthread_setschedparam() on the
#     calling thread. Guard: real Linux behavior unchanged, macOS gets the pthread
#     equivalent. ---
d="$(_pkg_dir plansys2_bringup)"
f="$d/src/plansys2_bringup/plansys2_node.cpp"
if [ -n "$d" ] && [ -f "$f" ] && grep -q 'sched_setscheduler(0, SCHED_FIFO, &sch) == -1' "$f"; then
  perl -0pi -e 's/if \(sched_setscheduler\(0, SCHED_FIFO, &sch\) == -1\) \{/#ifdef __linux__\n        bool sched_failed = (sched_setscheduler(0, SCHED_FIFO, &sch) == -1);\n#else\n        bool sched_failed = (pthread_setschedparam(pthread_self(), SCHED_FIFO, &sch) != 0);\n#endif\n        if (sched_failed) {/' "$f"
  echo "  plansys2_bringup: sched_setscheduler -> pthread_setschedparam on non-Linux"
fi

# --- find_object_2d (humble only -- pinned at the older 0.7.0-foxy commit; jazzy/kilted
#     pin a newer commit where this was already dropped upstream): `cmake_policy(SET
#     CMP0043/CMP0042 OLD)` -- modern CMake (post ~3.20) removed the ability to set these
#     to OLD at all, now a hard error instead of a silent no-op. Both are legacy
#     COMPILE_DEFINITIONS-per-config/RPATH shims no longer needed. ---
d="$(_pkg_dir find_object_2d)"
if [ -n "$d" ] && [ -f "$d/CMakeLists.txt" ] && grep -q 'cmake_policy(SET CMP0043 OLD)' "$d/CMakeLists.txt"; then
  perl -0pi -e 's/if \(POLICY CMP0043\)\n\s*cmake_policy\(SET CMP0043 OLD\)\nendif \(POLICY CMP0043\)\n//; s/if \(POLICY CMP0042\)\n\s*cmake_policy\(SET CMP0042 OLD\)\nendif \(POLICY CMP0042\)\n//' "$d/CMakeLists.txt"
  echo "  find_object_2d: drop no-longer-settable cmake_policy(SET CMP0043/CMP0042 OLD)"
fi

# --- rc_dynamics_api (all 3, identical commit): net_utils.cc + data_receiver.h use
#     TEMP_FAILURE_RETRY(), a glibc-only <unistd.h> extension macro with no macOS/libc++
#     equivalent -> "use of undeclared identifier". The wrapped calls (inet_pton, etc.)
#     don't need EINTR-retry semantics here; add a local fallback macro that just
#     evaluates the expression once, matching the class of shim commonly used for
#     portable code (BSD/macOS libc simply never defines this macro). ---
for _f in $(find "$ROOT" -path '*rc_dynamics_api/rc_dynamics_api/net_utils.cc' -o -path '*rc_dynamics_api/rc_dynamics_api/data_receiver.h' 2>/dev/null); do
  if grep -q 'TEMP_FAILURE_RETRY' "$_f" && ! grep -q '#ifndef TEMP_FAILURE_RETRY' "$_f"; then
    perl -0pi -e 's/(#include <unistd\.h>\n)/$1\n#ifndef TEMP_FAILURE_RETRY\n#define TEMP_FAILURE_RETRY(expression) (expression)\n#endif\n/' "$_f"
    echo "  rc_dynamics_api: add TEMP_FAILURE_RETRY fallback macro in ${_f#$ROOT/}"
  fi
done

# --- battery_state_broadcaster duplicate-name collision (jazzy/kilted): the
#     third-party ros_battery_monitoring stack (ipa320) ships a package literally
#     named "battery_state_broadcaster" (v1.2.0) that collides with the official
#     ros2_controllers "battery_state_broadcaster" (v4.x/5.x). Two packages sharing
#     one name is invalid in a colcon workspace -> discovery ambiguity, and the
#     ros2_controllers metapackage exec_depend can't resolve deterministically
#     ("package.sh not found"). The name-based skip-list (colcon list -> first path)
#     can't disambiguate two same-named dirs, so drop the third-party duplicate by
#     PATH. Its sibling battery_state_rviz_overlay is independent (deps: fmt,
#     sensor_msgs, rclcpp, rviz_2d_overlay_msgs) and keeps building. humble ships
#     neither package -> no-op there. ---
for _f in $(find "$ROOT" -path '*ros_battery_monitoring/battery_state_broadcaster/package.xml' 2>/dev/null); do
  _d="$(dirname "$_f")"
  if [ ! -f "$_d/COLCON_IGNORE" ]; then
    touch "$_d/COLCON_IGNORE"
    echo "  battery_state_broadcaster: COLCON_IGNORE third-party ros_battery_monitoring duplicate (${_d#$ROOT/})"
  fi
done

# --- easynav_common Singleton.hpp (EasyNavigation/easynav submodule, all 3):
#     removeInstance() resets the once_flag with `init_flag_ = std::once_flag();`,
#     but std::once_flag deletes copy/move assignment. On libc++ (macOS) this is a
#     hard error ("overload resolution selected deleted operator '='") the moment a
#     consumer instantiates it (easynav_sensors), and it CASCADES to the whole
#     easynav stack (~21 pkgs across all 3 distros). Reset the flag with the portable
#     destroy + placement-new idiom, which is semantically equivalent and compiles on
#     both libc++ and libstdc++. ---
for _f in $(find "$ROOT" -path '*easynav_common/include/easynav_common/Singleton.hpp' 2>/dev/null); do
  if grep -q 'init_flag_ = std::once_flag()' "$_f"; then
    grep -q '#include <new>' "$_f" || perl -0pi -e 's{(#include <mutex>\n)}{$1#include <new>\n}' "$_f"
    perl -0pi -e 's{[ \t]*init_flag_ = std::once_flag\(\);}{    init_flag_.~once_flag();\n    ::new (\&init_flag_) std::once_flag();}' "$_f"
    echo "  easynav_common Singleton.hpp: reset once_flag via placement-new (libc++ deleted-assign) in ${_f#$ROOT/}"
  fi
done

# --- io_context (ros-drivers/transport_drivers, all 3): uses the long-removed
#     asio::io_service / asio::io_service::work / io_context::post member APIs.
#     The toolchain points asio at Fast-DDS's bundled asio (1.34.2), which dropped
#     io_service entirely ("no type named 'io_service' in namespace 'asio'"). Port
#     to the modern equivalents: io_service -> io_context; io_service::work ->
#     executor_work_guard<io_context::executor_type> (constructed from the
#     executor); member .post() -> free asio::post(ctx, handler). Root of the
#     transport_drivers cluster (serial_driver/udp_driver/...). Verified: the
#     ported io_context.hpp compiles against the Fast-DDS asio with clang++ -std=c++17. ---
for _f in $(find "$ROOT" -path '*transport_drivers/io_context/include/io_context/io_context.hpp' 2>/dev/null); do
  if grep -q 'asio::io_service' "$_f"; then
    perl -0pi -e 's{asio::io_service::work}{asio::executor_work_guard<asio::io_context::executor_type>}g; s{asio::io_service\b}{asio::io_context}g; s{ios\(\)\.post\(}{asio::post(ios(), }g;' "$_f"
    echo "  io_context.hpp: asio::io_service -> io_context / executor_work_guard / asio::post in ${_f#$ROOT/}"
  fi
done
for _f in $(find "$ROOT" -path '*transport_drivers/io_context/src/io_context.cpp' 2>/dev/null); do
  if grep -q 'asio::io_service' "$_f"; then
    perl -0pi -e 's{new asio::io_service::work\(ios\(\)\)}{new asio::executor_work_guard<asio::io_context::executor_type>(ios().get_executor())}g; s{asio::io_service\b}{asio::io_context}g; s{ios\(\)\.post\(}{asio::post(ios(), }g;' "$_f"
    echo "  io_context.cpp: asio::io_service -> io_context / executor_work_guard / asio::post in ${_f#$ROOT/}"
  fi
done
