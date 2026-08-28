message(WARN " Toolchain.cmake is being used.")

# --- Detect if we are running in CI or local ---
if(CI_BUILD OR "$ENV{CI_BUILD}" STREQUAL "TRUE")
    set(IS_CI TRUE)
    message(STATUS "CI environment detected")
else()
    set(IS_CI FALSE)
    message(STATUS "Local environment detected")
endif()

# --- Set workspace root ---
if(IS_CI)
    set(WORKSPACE_ROOT "$ENV{GITHUB_WORKSPACE}")
else()
    set(WORKSPACE_ROOT "$ENV{HOME}/humble-ros2")
endif()

# --- Helper macro for src/ paths ---
macro(WORKSPACE_PATH result_path relative_path)
    if(IS_CI)
        # CI: skip src/
        set(${result_path} "${WORKSPACE_ROOT}/${relative_path}")
    else()
        # Local: include src/
        set(${result_path} "${WORKSPACE_ROOT}/src/${relative_path}")
    endif()
endmacro()

# --- CORE CONFIGURATION (Applicable to Local and CI) ---
# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)

# Setting all the Build Tests Off
set(BUILD_TESTING OFF CACHE BOOL "Disable building tests" FORCE)
set(BUILD_UNIT_TESTS OFF CACHE BOOL "Disable building tests" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "Disable building tests" FORCE)

# Make sure all workspace-installed packages are visible
set(ROS_WORKSPACE_INSTALL "${WORKSPACE_ROOT}/install")
list(APPEND CMAKE_PREFIX_PATH "${ROS_WORKSPACE_INSTALL}")

# Homebrew's tl-expected ships tl-expected-config.cmake (hyphen); several
# ros2_control-family packages (om_gravity_compensation_controller,
# om_spring_actuator_controller, ...) call find_package(tl_expected)
# (underscore, matching their package.xml <depend>) -> name mismatch, never
# found by CMake's default search. Bridge via cmake/Modules/Findtl_expected.cmake.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/Modules")

# --- Force System Python 3.11 ---
set(PYTHON_EXECUTABLE "/Library/Frameworks/Python.framework/Versions/3.11/bin/python3" CACHE FILEPATH "Python 3.11 interpreter" FORCE)
set(Python3_EXECUTABLE "/Library/Frameworks/Python.framework/Versions/3.11/bin/python3" CACHE FILEPATH "Python 3.11 executable" FORCE)
set(Python3_ROOT_DIR "/Library/Frameworks/Python.framework/Versions/3.11" CACHE PATH "Python3 root directory" FORCE)
# Make find_package(Python3) honor the forced 3.11 executable/root instead of
# picking the runner newest (3.14). mrt FindBoostPython uses find_package(Python3).
set(Python3_FIND_STRATEGY LOCATION CACHE STRING "" FORCE)
set(Python3_FIND_UNVERSIONED_NAMES FIRST CACHE STRING "" FORCE)
# --- versionless FindPython pin: find_package(Python)/mrt AutoDeps derive the Boost
#     python component from THIS; pin to 3.11 so consumers request boost_python311
#     (present in the vendored boost-1.89), not boost_python314 (runner default). ---
set(Python_EXECUTABLE "/Library/Frameworks/Python.framework/Versions/3.11/bin/python3" CACHE FILEPATH "Python 3.11 (versionless)" FORCE)
set(Python_ROOT_DIR "/Library/Frameworks/Python.framework/Versions/3.11" CACHE PATH "Python 3.11 root (versionless)" FORCE)
set(Python_FIND_STRATEGY "LOCATION" CACHE STRING "" FORCE)
set(PYTHON_LIBRARY "/Library/Frameworks/Python.framework/Versions/3.11/lib/libpython3.11.dylib" CACHE FILEPATH "Python 3.11 library" FORCE)
set(PYTHON_INCLUDE_DIR "/Library/Frameworks/Python.framework/Versions/3.11/include/python3.11" CACHE PATH "Python 3.11 include dir" FORCE)

# Boost (Custom ROS 2 build, not Homebrew)
WORKSPACE_PATH(BOOST_ROOT "ros-commondep/boost-1.89")
set(BOOST_INCLUDEDIR "${BOOST_ROOT}/include" CACHE PATH "Boost include")
set(BOOST_LIBRARYDIR "${BOOST_ROOT}/lib" CACHE PATH "Boost lib")
set(Boost_NO_SYSTEM_PATHS ON CACHE BOOL "Force Boost to use BOOST_ROOT" FORCE)
set(THIRDPARTY_Asio ON CACHE BOOL "Allow Thirdparty Asio" FORCE)
list(APPEND CMAKE_PREFIX_PATH "${BOOST_ROOT}")
set(Boost_INCLUDE_DIRS "${BOOST_INCLUDEDIR}" CACHE PATH "Boost include dirs" FORCE)
set(Boost_LIBRARY_DIRS "${BOOST_LIBRARYDIR}" CACHE PATH "Boost library dir" FORCE)
include_directories(SYSTEM ${BOOST_INCLUDEDIR})
link_directories(${BOOST_LIBRARYDIR})
add_definitions(-DBOOST_TIMER_ENABLE_DEPRECATED)
set(Boost_PYTHON_LIBRARY "${BOOST_LIBRARYDIR}/libboost_python311.dylib" CACHE FILEPATH "Boost Python library")
set(Boost_PYTHON_INCLUDE_DIR "${BOOST_INCLUDEDIR}" CACHE PATH "Boost Python include dir")
set(CMAKE_PREFIX_PATH "${BOOST_ROOT}/lib/cmake/Boost-1.89.0;${CMAKE_PREFIX_PATH}" CACHE PATH "Boost CMake path")

# RPATH settings for macOS
set(CMAKE_SKIP_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
if(IS_CI)
    # CI: boost will be copied into install/lib, so @loader_path/../lib covers it.
    # Don't add absolute runner paths to install rpath.
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
    set(CMAKE_BUILD_RPATH "@loader_path/../lib;${BOOST_LIBRARYDIR}")
    set(CMAKE_INSTALL_RPATH
        "@loader_path/../lib"       # lib/libfoo.dylib
        "@loader_path/.."           # lib/sub/libfoo.dylib
        "@loader_path/../.."        # lib/sub/plugins/libfoo.dylib
        "@loader_path/../../.."     # lib/python3.11/site-packages/pkg/libfoo.dylib
        "@loader_path/../../../.."  # lib/sub/plugins/gui/GzSim/libfoo.dylib
    )
else()
    # Local: boost lives outside install/, so embed its path explicitly.
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    set(CMAKE_BUILD_RPATH "@loader_path/../lib;${BOOST_LIBRARYDIR}")
    set(CMAKE_INSTALL_RPATH "@loader_path/../lib;${BOOST_LIBRARYDIR}")
endif()

# Ceres, yaml-cpp, PCL Conversions, pybind11 (Non-Homebrew/Vendor dependencies)
set(BUILD_BENCHMARKS OFF CACHE BOOL "Disable building benchmarks" FORCE) 
set(BUILD_EXAMPLES OFF CACHE BOOL "Disable building examples" FORCE)
set(yaml-cpp_DIR "${WORKSPACE_ROOT}/install/opt/yaml_cpp_vendor/share/cmake/yaml-cpp" CACHE PATH "yaml-cpp config directory")  # humble builds yaml-cpp 0.7.0 -> config in DATADIR/cmake=share/ (0.8.0 distros use lib/)
set(YAML_CPP_INCLUDE_DIRS "${WORKSPACE_ROOT}/install/opt/yaml_cpp_vendor/include" CACHE PATH "yaml-cpp include directory")
set(YAML_CPP_LIBRARIES "${WORKSPACE_ROOT}/install/opt/yaml_cpp_vendor/lib/libyaml-cpp.dylib" CACHE FILEPATH "yaml-cpp library")
include_directories(${YAML_CPP_INCLUDE_DIRS})
link_directories("${WORKSPACE_ROOT}/install/opt/yaml_cpp_vendor/lib")  # resolve bare -lyaml-cpp from consumers
set(CMAKE_EXE_LINKER_FLAGS "${backward_ros_full_path_LIBRARIES} ${CMAKE_EXE_LINKER_FLAGS}" CACHE STRING "Linker flags" FORCE)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
  add_compile_options(-O3 -march=native)
endif()
string(REPLACE "-Werror" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(REPLACE "-Werror" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" CACHE STRING "Modified CXX flags without -Werror" FORCE)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" CACHE STRING "Modified C flags without -Werror" FORCE)
set(BUILD_CLOUD_CLIENT OFF CACHE BOOL "Disable cloud client")

add_compile_options(
        -Wno-error=unused-command-line-argument
        -Wno-error=pessimizing-move
        -Wno-error=unused-variable
        -Wno-error=unused-parameter
        -Wno-error=unused-but-set-variable
        -Wno-error=shadow 
        -Wno-error=shadow-field
        -Wno-error=deprecated-dynamic-exception-spec
        -Wno-error=inconsistent-missing-override
        -Wno-error=format-security
        -Wno-error=overloaded-virtual
        -Wno-error=deprecated-builtins
        -Wno-error=deprecated-copy-with-dtor
        -Wno-error=deprecated-copy-with-user-provided-dtor
        -Wno-error=unused-lambda-capture
        -Wno-error=unused-private-field
        -Wno-error=sign-conversion
        -Wno-error=format
        -Wno-error=deprecated-declarations
        -Wno-error=sign-conversion
        -Wno-error=missing-template-arg-list-after-template-kw
        -Wno-error=thread-safety-analysis
 )

# --- macOS SDK sysroot (must use xcrun — xcodebuild -n was removed in Xcode 26) ---
if(NOT DEFINED CMAKE_OSX_SYSROOT OR CMAKE_OSX_SYSROOT STREQUAL "")
    execute_process(
        COMMAND xcrun --sdk macosx --show-sdk-path
        OUTPUT_VARIABLE _detected_sysroot
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if(_detected_sysroot)
        # Strip embedded newlines — xcrun may mix warnings into stdout on some Xcode versions
        string(REGEX REPLACE "[\r\n].*" "" _detected_sysroot "${_detected_sysroot}")
        string(STRIP "${_detected_sysroot}" _detected_sysroot)
        set(CMAKE_OSX_SYSROOT "${_detected_sysroot}" CACHE PATH "macOS SDK sysroot" FORCE)
    endif()
endif()

set(CMAKE_MACOSX_RPATH ON)
set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(BLAS_LIBRARIES "-framework Accelerate" CACHE STRING "BLAS libraries")
set(LAPACK_LIBRARIES "-framework Accelerate" CACHE STRING "LAPACK libraries")
set(PCL_ALL_IN_ONE_INSTALLER OFF CACHE BOOL "Disable bundled Boost" FORCE)
set(ROS_EDITION "ROS2" CACHE STRING "ROS edition")
set(ROS_VERSION "2" CACHE STRING "ROS Version")
set(KILTED_ROS "kilted" CACHE STRING "ROS 2 kilted")
set(ROS_DISTRO "humble" CACHE STRING "ROS 2 Distro")
WORKSPACE_PATH(PCL_CONVERSIONS_INCLUDE_DIR "ros-perception/perception_pcl/pcl_conversions/include")
set(pcl_conversions_INCLUDE_DIRS "${PCL_CONVERSIONS_INCLUDE_DIR}" CACHE PATH "PCL Conversions include dirs" FORCE)

# --- OpenMP (libomp) ---
set(OpenMP_INCLUDE_DIR "/opt/homebrew/opt/libomp/include")
set(OpenMP_LIB_DIR "/opt/homebrew/opt/libomp/lib")
include_directories(SYSTEM "${OpenMP_INCLUDE_DIR}")
set(OpenMP_CXX_LIB_NAMES "omp" CACHE STRING "")
set(OpenMP_C_LIB_NAMES "omp" CACHE STRING "")
set(OpenMP_CXX_FLAGS "-Xclang -fopenmp" CACHE STRING "")
set(OpenMP_C_FLAGS   "-Xclang -fopenmp" CACHE STRING "")
set(OpenMP_omp_LIBRARY "${OpenMP_LIB_DIR}/libomp.dylib" CACHE STRING "")
if (NOT TARGET OpenMP::OpenMP_CXX)
    add_library(OpenMP::OpenMP_CXX INTERFACE IMPORTED)
    set_target_properties(OpenMP::OpenMP_CXX PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${OpenMP_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "-L${OpenMP_LIB_DIR};-lomp"
        INTERFACE_COMPILE_OPTIONS "-Xclang;-fopenmp"
    )
endif()

# --- Google Benchmark, CLI11, CSparse, GDAL (Prefix/Includes) ---
set(CMAKE_PREFIX_PATH "/opt/homebrew/opt/google-benchmark;${CMAKE_PREFIX_PATH}")
set(CLI11_INCLUDE_DIRS "/opt/homebrew/include" CACHE PATH "CLI11 include path" FORCE)
include_directories(SYSTEM ${CLI11_INCLUDE_DIRS})
set(CSPARSE_INCLUDE_DIR "/opt/homebrew/include/suitesparse")
set(CSPARSE_LIBRARY "/opt/homebrew/lib/libsuitesparse.dylib")
set(GDAL_CONFIG_BIN "/opt/homebrew/bin/gdal-config" CACHE FILEPATH "Path to Homebrew gdal-config utility." FORCE)
set(ENV{GDAL_CONFIG} ${GDAL_CONFIG_BIN})
set(CMAKE_PREFIX_PATH 
    "/opt/homebrew/opt/gdal;${CMAKE_PREFIX_PATH}" CACHE STRING "Prefix paths" FORCE)

# --- Qt5/Qt6 ---
set(Qt5_DIR "/opt/homebrew/opt/qt@5/lib/cmake/Qt5" CACHE PATH "Qt5 CMake path")
set(QT5_PREFIX "/opt/homebrew/opt/qt@5" CACHE PATH "Qt5 prefix path")
set(Qt6_DIR "/opt/homebrew/opt/qt/lib/cmake/Qt6" CACHE PATH "Qt6 CMake path")
set(QT6_PREFIX "/opt/homebrew/opt/qt" CACHE PATH "Qt6 prefix path")
set(CMAKE_PREFIX_PATH "${QT6_PREFIX}/lib/cmake;${QT5_PREFIX}/lib/cmake;${CMAKE_PREFIX_PATH}" CACHE PATH "Qt CMake paths" FORCE)
set(ENV{PATH} "${QT6_PREFIX}/bin:${QT5_PREFIX}/bin:$ENV{PATH}")
set(CMAKE_FIND_FRAMEWORK LAST)

# --- OpenGL/GLEW/GLUT for Qt/Ogre ---
set(_GL_INCDIRS "/opt/homebrew/include" CACHE STRING "")
set(_qt5gui_OPENGL_INCLUDE_DIR "/opt/homebrew/include/GL" CACHE PATH "")
set(Qt5Gui_OPENGL_IMPLEMENTATION GL CACHE STRING "")
set(Qt5Gui_OPENGL_LIBRARIES "/opt/homebrew/lib/libGL.dylib" CACHE FILEPATH "")
set(GLEW_INCLUDE_DIR "/opt/homebrew/opt/glew/include" CACHE PATH "GLEW include directory")
set(GLEW_LIBRARY "/opt/homebrew/opt/glew/lib/libGLEW.dylib" CACHE FILEPATH "GLEW library")
set(GLUT_INCLUDE_DIR "/opt/homebrew/opt/freeglut/include" CACHE PATH "GLUT/FreeGLUT include directory")
set(GLUT_LIBRARY "/opt/homebrew/lib/libglut.dylib" CACHE FILEPATH "GLUT/FreeGLUT library")
set(GLEW_INCLUDE_DIRS ${GLEW_INCLUDE_DIR} CACHE INTERNAL "")
set(GLEW_LIBRARIES ${GLEW_LIBRARY} CACHE INTERNAL "")
set(GLUT_INCLUDE_DIRS ${GLUT_INCLUDE_DIR} CACHE INTERNAL "")
set(GLUT_LIBRARIES ${GLUT_LIBRARY} CACHE INTERNAL "")
set(SYSTEM_GL_INCLUDE_DIR ${GLEW_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS} CACHE INTERNAL "")
set(SYSTEM_GL_LIBRARIES ${GLEW_LIBRARIES} ${GLUT_LIBRARIES} CACHE INTERNAL "")
set(OGRE_FRAMEWORKS "-framework OpenGL")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OGRE_FRAMEWORKS}" CACHE STRING "Link OpenGL framework for Ogre" FORCE)
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${OGRE_FRAMEWORKS}" CACHE STRING "Link OpenGL framework for Ogre" FORCE)

# --- TinyXML2 (Homebrew fix for Gazebo vendors) ---
if (NOT TARGET tinyxml2::tinyxml2)
    add_library(tinyxml2::tinyxml2 INTERFACE IMPORTED)
    set_target_properties(tinyxml2::tinyxml2 PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "/opt/homebrew/include"
        INTERFACE_LINK_LIBRARIES "/opt/homebrew/lib/libtinyxml2.dylib"
    )
endif()
if (NOT TARGET TINYXML2::TINYXML2)
    add_library(TINYXML2::TINYXML2 INTERFACE IMPORTED)
    set_target_properties(TINYXML2::TINYXML2 PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "/opt/homebrew/include"
        INTERFACE_LINK_LIBRARIES "/opt/homebrew/lib/libtinyxml2.dylib"
    )
endif()

# --- Eigen (Homebrew) ---
# With the symlink 'sudo ln -s /opt/homebrew/opt/eigen@3/include/eigen3 /opt/homebrew/include/eigen3'
# standard discovery now works.
set(Eigen3_DIR "/opt/homebrew/opt/eigen@3/share/eigen3/cmake" CACHE PATH "" FORCE)
list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/eigen@3")

# --- ICU4C (Homebrew, keg-only) ---
# cx_ros_msgs_plugin (ros-drivers/clips_executive) calls
# find_package(ICU REQUIRED COMPONENTS uc i18n). icu4c is keg-only (not
# symlinked into /opt/homebrew/{include,lib}) so CMake's bundled FindICU sees
# the pkg-config version but can't locate the actual headers/libs without an
# explicit prefix hint.
list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/icu4c")

# --- Google glog (Homebrew) ---
set(GLOG_INCLUDE_DIR "/opt/homebrew/include" CACHE PATH "glog include path" FORCE)
set(GLOG_LIBRARY "/opt/homebrew/lib/libglog.dylib" CACHE FILEPATH "glog library" FORCE)
include_directories(SYSTEM ${GLOG_INCLUDE_DIR})
# Append glog library to all targets' linker flags
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GLOG_LIBRARY}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${GLOG_LIBRARY}")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${GLOG_LIBRARY}")

# --- OctoMap (Disable octovis subproject) ---
set(BUILD_OCTOVIS_SUBPROJECT OFF CACHE BOOL "Disable building octovis subproject" FORCE)

# --- MoveIt Task Constructor: Prioritize internal pybind11 headers to avoid Homebrew conflicts ---
if(PROJECT_NAME STREQUAL "moveit_task_constructor_core")
    set(MTC_PYBIND_INTERNAL "${CMAKE_CURRENT_SOURCE_DIR}/python/pybind11/include")
    
    # Ensure headers exist before injecting
    if(EXISTS "${MTC_PYBIND_INTERNAL}")
        message(STATUS "Toolchain: Prioritizing internal MTC pybind11 headers")
        include_directories(BEFORE SYSTEM "${MTC_PYBIND_INTERNAL}")
        
        # Prevent FindPackage from accidentally pulling in the Homebrew version
        set(pybind11_DIR "${MTC_PYBIND_INTERNAL}" CACHE PATH "" FORCE)
        set(pybind11_FOUND TRUE)
    endif()
endif()
# ---------------------------------------------------------------------------
# GTSAM (borglab/gtsam @ 4.2.2) build options — macOS arm64 / colcon
# GTSAM_* vars are namespaced, so setting them unconditionally is harmless to
# every other package. These control build time, bottle portability, Eigen ABI,
# and the Homebrew oneTBB incompatibility.
# ---------------------------------------------------------------------------
set(GTSAM_BUILD_TESTS              OFF CACHE BOOL "" FORCE)  # default ON  - skip tests
set(GTSAM_BUILD_EXAMPLES_ALWAYS    OFF CACHE BOOL "" FORCE)  # default ON  - skip examples
set(GTSAM_BUILD_TIMING_ALWAYS      OFF CACHE BOOL "" FORCE)
set(GTSAM_BUILD_UNSTABLE           OFF CACHE BOOL "" FORCE)  # consumers do not use gtsam_unstable
set(GTSAM_UNSTABLE_BUILD_PYTHON    OFF CACHE BOOL "" FORCE)
set(GTSAM_BUILD_PYTHON             OFF CACHE BOOL "" FORCE)
set(GTSAM_INSTALL_MATLAB_TOOLBOX   OFF CACHE BOOL "" FORCE)
set(GTSAM_USE_SYSTEM_EIGEN         ON  CACHE BOOL "" FORCE)  # share workspace Homebrew Eigen (avoid ABI mismatch)
set(GTSAM_WITH_TBB                 OFF CACHE BOOL "" FORCE)  # Homebrew = oneTBB removed tbb::task GTSAM 4.2 uses
set(GTSAM_BUILD_WITH_MARCH_NATIVE  OFF CACHE BOOL "" FORCE)  # keep bottle portable across Macs
# --- end GTSAM options ---

# ---------------------------------------------------------------------------
# Cartographer (ros2 branch) uses legacy unprefixed Abseil thread-safety macros
# (LOCKS_EXCLUDED / EXCLUSIVE_LOCKS_REQUIRED / GUARDED_BY) removed by newer
# Abseil. Force-include a shim mapping them to the ABSL_* equivalents.
# ---------------------------------------------------------------------------
if(CMAKE_PROJECT_NAME MATCHES "^cartographer")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -include ${CMAKE_CURRENT_LIST_DIR}/cartographer_absl_compat.h")
endif()

# ---------------------------------------------------------------------------
# Lane 1b: Homebrew as a general link/search prefix (Apple Silicon)
# /opt/homebrew is NOT a default search path here, so any package emitting a
# bare -lglog / -lproj / -lgeos_c / -logg / -ltheoraenc / -lbenchmark / -lzstd
# died with "ld: library 'X' not found". Adding -L/opt/homebrew/lib once fixes
# that whole class (the per-glog full-path hack above only helped glog, and not
# for packages that emit their own -lglog like cartographer).
# Placed LAST so it captures all prior linker-flag additions and is not wiped by
# earlier CACHE...FORCE sets. Appended (not prepended) so vendored -L paths
# (boost-1.89, yaml_cpp_vendor) still win; `brew unlink` keeps conflicting kegs
# (boost, yaml-cpp, qt, orocos-kdl) out of /opt/homebrew/lib, so no clash.
# ---------------------------------------------------------------------------
list(APPEND CMAKE_PREFIX_PATH  "/opt/homebrew")
list(APPEND CMAKE_LIBRARY_PATH "/opt/homebrew/lib")
list(APPEND CMAKE_INCLUDE_PATH "/opt/homebrew/include")

# --- asio: brew asio (1.36) is too new for several consumers and is deliberately
#     `brew unlink`ed (do NOT re-link). Fast-DDS vendors asio 1.34.2 under
#     thirdparty/asio (checked out via --recursive submodules). Point CMAKE_INCLUDE_PATH
#     at it so find_path(asio.hpp) in asio_cmake_module's FindASIO (io_context) and the
#     per-package Findasio (ublox_gps, ecal, ...) resolve the vendored copy. Header-only,
#     so include-path is all that's needed; scoped to find_path (not a global -I) so it
#     doesn't shadow Fast-DDS's own relative use. ---
WORKSPACE_PATH(_FASTDDS_ASIO_INC "eProsima/Fast-DDS/thirdparty/asio/asio/include")
if(EXISTS "${_FASTDDS_ASIO_INC}/asio.hpp")
  list(APPEND CMAKE_INCLUDE_PATH "${_FASTDDS_ASIO_INC}")
  message(STATUS "Toolchain: using Fast-DDS vendored asio 1.34.2 at ${_FASTDDS_ASIO_INC}")
endif()
link_directories(/opt/homebrew/lib)
include_directories(SYSTEM /opt/homebrew/include)
foreach(_lf CMAKE_EXE_LINKER_FLAGS CMAKE_SHARED_LINKER_FLAGS CMAKE_MODULE_LINKER_FLAGS)
  set(${_lf} "${${_lf}} -L/opt/homebrew/lib" CACHE STRING "" FORCE)
endforeach()

# ---------------------------------------------------------------------------
# OpenCV (Homebrew) — pin to opencv@4. Brew's default `opencv` is now 5.0,
# which dropped the legacy C-API headers (opencv2/imgproc/types_c.h); cv_bridge
# and other vision packages still include them. opencv@4 (keg-only) ships them.
# Mirrors kilted. Requires opencv@4 in the workflow's brew install list.
# ---------------------------------------------------------------------------
set(OpenCV_DIR "/opt/homebrew/opt/opencv@4/lib/cmake/opencv4" CACHE PATH "" FORCE)

# ---------------------------------------------------------------------------
# macOS / libc++ portability shims (Lane 4 clusters)
#  - clang errors on GCC-only warning flags (e.g. -Wmaybe-uninitialized) under
#    -Werror; ignore unknown warning options instead of failing.
#  - -Wignored-qualifiers (const on return type) is benign — don't make it fatal.
#  - glibc float/long-double math macros (M_PIf/M_PIl) are absent on macOS.
# ---------------------------------------------------------------------------
add_compile_options(-Wno-unknown-warning-option -Wno-error=ignored-qualifiers)
add_compile_definitions(M_PIf=3.14159265358979323846f
                        M_PI_2f=1.57079632679489661923f
                        M_PI_4f=0.78539816339744830962f
                        M_1_PIf=0.31830988618379067154f
                        M_2_PIf=0.63661977236758134308f
                        M_PIl=3.141592653589793238462643383279502884L)
# libc++ removed std::result_of etc. in C++20; re-enable for older third-party
# code (e.g. libcaer_driver device.cpp) instead of patching each.
add_compile_definitions(_LIBCPP_ENABLE_CXX20_REMOVED_TYPE_TRAITS)
# libc++ (Apple Clang) never ships the C++17 Parallel Algorithms / <execution> extension
# (std::execution::seq/par, std::is_execution_policy_v, the policy-taking overloads of
# std::transform/reduce/...) under the default config -- it's gated behind -fexperimental-
# library. beluga's normalize.hpp uses std::execution::seq with std::transform. Compile-
# tested (Apple clang 21.0.0): "no member 'seq'/no type 'sequenced_policy' in namespace
# 'std::execution'" without the flag; clean compile+link+run with it.
if(APPLE)
  add_compile_options(-fexperimental-library)
endif()
# std::codecvt_utf8_utf16 (rosidl_runtime_cpp/traits.hpp:132) is _LIBCPP_DEPRECATED_IN_CXX17;
# autoware & others compile with -Werror and re-add it AFTER the toolchain's
# -Wno-error=deprecated-declarations (order loses), so the deprecation escalates to an error.
# Disable libc++ deprecation warnings at the source so -Werror has nothing to escalate.
# Compile-tested: reproduces the 6 deprecated-errors without it; clean with it, under -Werror.
add_compile_definitions(_LIBCPP_DISABLE_DEPRECATION_WARNINGS)
# Linux termios high-baud constants absent on macOS (it caps standard names at B230400 and
# uses IOSSIOSPEED for the rest). Define the numeric values so serial drivers compile
# (ess_imu_driver2, rosbot_mavlink_bridge, …); they aren't run on macOS CI. macOS defines
# none of these, so no redefinition clash.
add_compile_definitions(B460800=460800 B500000=500000 B921600=921600
                        B1000000=1000000 B1152000=1152000 B1500000=1500000
                        B2000000=2000000 B2500000=2500000 B3000000=3000000)

# Force-include a macOS-compat shim (glibc endian.h funcs htole*/le*toh/be*, SOCK_CLOEXEC) so
# ports needing them don't each require an #ifdef. APPLE-only; no-op on Linux. Compile-tested.
if(APPLE)
  add_compile_options(-include "${CMAKE_CURRENT_LIST_DIR}/macos_compat.h")
endif()
