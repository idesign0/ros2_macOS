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
#     on macOS) -> -Wmacro-redefined error. Pass CFLAGS to suppress that warning. ---
d="$(_pkg_dir lely_core_libraries)"
if [ -n "$d" ] && [ -f "$d/CMakeLists.txt" ] && ! grep -q 'Wno-macro-redefined' "$d/CMakeLists.txt"; then
  sed "${SEDI[@]}" 's#<SOURCE_DIR>/configure --prefix#<SOURCE_DIR>/configure "CFLAGS=-O2 -Wno-macro-redefined -DLELY_HAVE_THREADS_H=0 -DLELY_HAVE_PTHREAD_H=1" --prefix#' "$d/CMakeLists.txt"
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
#     merged archive with both input symbols present. ---
d="$(_pkg_dir fmilibrary_vendor)"
if [ -n "$d" ] && [ -f "$d/CMakeLists.txt" ] && ! grep -q 'ci-fmilib-cmp0026' "$d/CMakeLists.txt"; then
  _FMI_SNIPPET="$(mktemp)"
  cat > "$_FMI_SNIPPET" <<'PATCHEOF'
  PATCH_COMMAND perl -0pi -e [[s/get_target_property\(libfile (\$\{lib\}) LOCATION\)/'set(libfile "'.chr(36).'<TARGET_FILE:'.$1.'>")'/ge]] <SOURCE_DIR>/Config.cmake/mergestaticlibs.cmake
  COMMAND perl -0pi -e [[s/get_target_property\(outfile (\$\{outlib\}) LOCATION\)/'set(outfile "'.chr(36).'<TARGET_FILE:'.$1.'>")'/ge]] <SOURCE_DIR>/Config.cmake/mergestaticlibs.cmake # ci-fmilib-cmp0026
PATCHEOF
  sed -e "/^[[:space:]]*TIMEOUT 60[[:space:]]*\$/r $_FMI_SNIPPET" "$d/CMakeLists.txt" > "$d/CMakeLists.txt.__p" && mv "$d/CMakeLists.txt.__p" "$d/CMakeLists.txt"
  rm -f "$_FMI_SNIPPET"
  echo "  fmilibrary_vendor: PATCH_COMMAND fixes CMP0026 LOCATION reads in mergestaticlibs.cmake"
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

echo "source patches applied."
