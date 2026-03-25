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
    # Derive workspace root from toolchain location: src/cmake/toolchain.cmake -> ../..
    get_filename_component(WORKSPACE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
endif()

# --- CMake policy compatibility for older packages ---
set(CMAKE_POLICY_VERSION_MINIMUM "3.5" CACHE STRING "Minimum CMake policy version" FORCE)

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

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)

# --- block /usr/local/ completely ---
set(CMAKE_IGNORE_PATH
  /usr/local
)

# --- ensure correct stdlib ---
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")

# Add Homebrew library search paths to linker flags (ensures -L paths propagate to link phase)
set(_HB_LINK_DIRS "-L/opt/homebrew/lib -L/opt/homebrew/opt/ffmpeg/lib -L/opt/homebrew/opt/theora/lib -L/opt/homebrew/opt/libogg/lib -L/opt/homebrew/opt/geos/lib -L/opt/homebrew/opt/proj/lib -L/opt/homebrew/opt/libspnav/lib -L/opt/homebrew/opt/zstd/lib -L/opt/homebrew/opt/zbar/lib -L/opt/homebrew/opt/google-benchmark/lib -L/opt/homebrew/opt/gstreamer/lib -L/opt/homebrew/opt/aravis/lib -L/opt/homebrew/opt/yaml-cpp/lib")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${_HB_LINK_DIRS}" CACHE STRING "" FORCE)
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${_HB_LINK_DIRS}" CACHE STRING "" FORCE)
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${_HB_LINK_DIRS}" CACHE STRING "" FORCE)

# Setting all the Build Tests Off
set(BUILD_TESTING OFF CACHE BOOL "Disable building tests" FORCE)
set(BUILD_UNIT_TESTS OFF CACHE BOOL "Disable building tests" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "Disable building tests" FORCE)

# Make sure all workspace-installed packages are visible
set(ROS_WORKSPACE_INSTALL "${WORKSPACE_ROOT}/install")
list(APPEND CMAKE_PREFIX_PATH "${ROS_WORKSPACE_INSTALL}")

# --- Gazebo workspace install (set via env GZ_INSTALL_DIR from build.zsh) ---
if(DEFINED ENV{GZ_INSTALL_DIR} AND NOT "$ENV{GZ_INSTALL_DIR}" STREQUAL "")
    set(GZ_INSTALL_PREFIX "$ENV{GZ_INSTALL_DIR}")
else()
    # Fallback: derive from workspace root (sibling gz-ionic directory)
    get_filename_component(GZ_INSTALL_PREFIX "${WORKSPACE_ROOT}/../gz-ionic/install" ABSOLUTE)
endif()
if(EXISTS "${GZ_INSTALL_PREFIX}")
    list(APPEND CMAKE_PREFIX_PATH "${GZ_INSTALL_PREFIX}")
    message(STATUS "Gazebo install dir: ${GZ_INSTALL_PREFIX}")
endif()

# --- Force System Python 3.11 ---
# The /Library/Frameworks stub only has bin/; real headers+libs live under Homebrew
set(PYTHON_EXECUTABLE "/Library/Frameworks/Python.framework/Versions/3.11/bin/python3" CACHE FILEPATH "Python 3.11 interpreter" FORCE)
set(Python3_EXECUTABLE "/Library/Frameworks/Python.framework/Versions/3.11/bin/python3" CACHE FILEPATH "Python 3.11 executable" FORCE)
set(Python3_ROOT_DIR "/opt/homebrew/opt/python@3.11/Frameworks/Python.framework/Versions/3.11" CACHE PATH "Python3 root directory" FORCE)

# Headers & libs from the real Homebrew framework location
set(PYTHON_INCLUDE_DIR "/opt/homebrew/opt/python@3.11/Frameworks/Python.framework/Versions/3.11/include/python3.11" CACHE PATH "Python 3.11 include dir" FORCE)
set(PYTHON_LIBRARY "/opt/homebrew/opt/python@3.11/Frameworks/Python.framework/Versions/3.11/lib/libpython3.11.dylib" CACHE FILEPATH "Python 3.11 library" FORCE)
set(Python3_INCLUDE_DIR "${PYTHON_INCLUDE_DIR}" CACHE PATH "Python3 include dir" FORCE)
set(Python3_LIBRARY "${PYTHON_LIBRARY}" CACHE FILEPATH "Python3 library" FORCE)
set(Python3_NumPy_INCLUDE_DIR "/opt/homebrew/lib/python3.11/site-packages/numpy/_core/include" CACHE PATH "NumPy include dir" FORCE)

# Check the package/project name
if("${CMAKE_PROJECT_NAME}" STREQUAL "kinematics_interface_pinocchio")
    message(STATUS "Stitching Boost and Boost-Python for ${CMAKE_PROJECT_NAME}")
    set(Boost_INCLUDE_DIR "/opt/homebrew/opt/boost/include" CACHE PATH "" FORCE)
    # Explicitly map the Python 3.14 library path you just found
    set(Boost_PYTHON314_LIBRARY_RELEASE "/opt/homebrew/opt/boost-python3/lib/libboost_python314.dylib" CACHE FILEPATH "" FORCE)
    set(Boost_PYTHON314_LIBRARY_DEBUG "/opt/homebrew/opt/boost-python3/lib/libboost_python314.dylib" CACHE FILEPATH "" FORCE)
    set(Boost_NO_SYSTEM_PATHS OFF CACHE BOOL "" FORCE)
else()
    message(STATUS "Using custom CMake Boost for ${CMAKE_PROJECT_NAME}")

    # Path to your custom Boost installation
    WORKSPACE_PATH(BOOST_ROOT "ros-commondep/boost-1.89")
    set(BOOST_INCLUDEDIR "${BOOST_ROOT}/include" CACHE PATH "Boost include")
    set(BOOST_LIBRARYDIR "${BOOST_ROOT}/lib" CACHE PATH "Boost lib")

    # Ensure Boost can be found without searching system paths first
    set(CMAKE_PREFIX_PATH "${BOOST_ROOT};${CMAKE_PREFIX_PATH}")

    # Force modern CMake Boost behavior
    set(Boost_NO_SYSTEM_PATHS ON CACHE BOOL "Don't search system paths for Boost")
    set(Boost_NO_BOOST_CMAKE OFF CACHE BOOL "Allow BoostConfig.cmake if present")

    # Make sure CMake uses your Boost include & lib paths
    set(Boost_INCLUDE_DIRS "${BOOST_INCLUDEDIR}" CACHE PATH "Boost include dirs" FORCE)
    set(Boost_LIBRARY_DIRS "${BOOST_LIBRARYDIR}" CACHE PATH "Boost library dir" FORCE)

    include_directories(SYSTEM ${BOOST_INCLUDEDIR})
    link_directories(${BOOST_LIBRARYDIR})

    # Python-specific paths
    set(Boost_PYTHON_LIBRARY "${BOOST_LIBRARYDIR}/libboost_python311.dylib" CACHE FILEPATH "Boost Python library")
    set(Boost_PYTHON_INCLUDE_DIR "${BOOST_INCLUDEDIR}" CACHE PATH "Boost Python include dir")
endif()



# --- RPATH settings for macOS ---
set(CMAKE_MACOSX_RPATH ON)
set(CMAKE_SKIP_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Base rpath: relative to executable for merged install
set(BASE_RPATH "@loader_path/../lib")

# Append your custom Boost library path
list(APPEND BASE_RPATH "${BOOST_LIBRARYDIR}")

# Apply to build and install
set(CMAKE_BUILD_RPATH "${BASE_RPATH}")
set(CMAKE_INSTALL_RPATH "${BASE_RPATH}")

# Ceres
set(BUILD_BENCHMARKS OFF CACHE BOOL "Disable building benchmarks" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "Disable building examples" FORCE)
set(Ceres_DIR "/opt/homebrew/Cellar/ceres-solver/2.2.0_6/lib/cmake/Ceres" CACHE PATH "Ceres Solver CMake path" FORCE)

# METIS (required by Ceres via SuiteSparse, keg-only in Homebrew)
set(METIS_INCLUDE_DIR "/opt/homebrew/opt/metis/include" CACHE PATH "METIS include dir" FORCE)
set(METIS_LIBRARY "/opt/homebrew/opt/metis/lib/libmetis.dylib" CACHE FILEPATH "METIS library" FORCE)
set(METIS_LIBRARY_RELEASE "/opt/homebrew/opt/metis/lib/libmetis.dylib" CACHE FILEPATH "METIS release library" FORCE)
list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/metis")

# --- Standalone ASIO (unlinked from Homebrew) ---
set(ASIO_INCLUDE_DIRS "/opt/homebrew/opt/asio/include" CACHE PATH "Standalone ASIO include dir" FORCE)
set(ASIO_INCLUDE_DIR "/opt/homebrew/opt/asio/include" CACHE PATH "Standalone ASIO include dir" FORCE)
set(asio_INCLUDE_DIR "/opt/homebrew/opt/asio/include" CACHE PATH "Standalone ASIO include dir" FORCE)
list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/asio")
list(APPEND CMAKE_INCLUDE_PATH "/opt/homebrew/opt/asio/include")

# --- yaml-cpp (use Homebrew since vendor doesn't install on macOS) ---
set(YAML_CPP_PREFIX "/opt/homebrew/opt/yaml-cpp" CACHE PATH "yaml-cpp prefix")
list(APPEND CMAKE_PREFIX_PATH "${YAML_CPP_PREFIX}")
set(yaml-cpp_DIR "${YAML_CPP_PREFIX}/lib/cmake/yaml-cpp" CACHE PATH "yaml-cpp config directory")
set(YAML_CPP_INCLUDE_DIRS "${YAML_CPP_PREFIX}/include" CACHE PATH "yaml-cpp include directory")
set(YAML_CPP_LIBRARIES "${YAML_CPP_PREFIX}/lib/libyaml-cpp.dylib" CACHE FILEPATH "yaml-cpp library")
include_directories(${YAML_CPP_INCLUDE_DIRS})

# Backward ROS
# On macOS, do NOT add Linux-only flags
set(CMAKE_EXE_LINKER_FLAGS "${backward_ros_full_path_LIBRARIES} ${CMAKE_EXE_LINKER_FLAGS}" CACHE STRING "Linker flags" FORCE)

# Optimization flags
if(CMAKE_BUILD_TYPE STREQUAL "Release")
  add_compile_options(-O3 -march=native)
endif()


# Suppress treating warnings as errors
string(REPLACE "-Werror" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(REPLACE "-Werror" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

# 2. Force the new, modified flags into the CMake cache
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" CACHE STRING "Modified CXX flags without -Werror" FORCE)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" CACHE STRING "Modified C flags without -Werror" FORCE)

# glog 0.7.1+ requires GLOG_USE_GLOG_EXPORT to be defined before including glog headers
add_definitions(-DGLOG_USE_GLOG_EXPORT)

# Ruckig/MoveIt fix: Undefine the macro that causes linking errors with Ruckig Community Edition
set(BUILD_CLOUD_CLIENT OFF CACHE BOOL "Disable cloud client")

# Suppress warning as errors
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
        -Wno-error=missing-template-arg-list-after-template-kw
        -Wno-unknown-warning-option
        -Wno-error=unknown-warning-option
        -Wno-error=#warnings
        -Wno-error=deprecated-declarations
 )

# macOS specific linker
set(CMAKE_MACOSX_RPATH ON)

# Thread config
set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)

# Asio FastDDS fix
set(THIRDPARTY_Asio ON CACHE BOOL "Allow Thirdparty Asio" FORCE)

# OpenMP for Apple Clang
set(OpenMP_INCLUDE_DIR "/opt/homebrew/opt/libomp/include")
set(OpenMP_LIB_DIR "/opt/homebrew/opt/libomp/lib")

# Tell FindOpenMP where libomp is
include_directories(SYSTEM "${OpenMP_INCLUDE_DIR}")
set(OpenMP_CXX_LIB_NAMES "omp" CACHE STRING "")
set(OpenMP_C_LIB_NAMES "omp" CACHE STRING "")

# Provide flags ONLY to FindOpenMP (not global)
set(OpenMP_CXX_FLAGS "-Xclang -fopenmp" CACHE STRING "")
set(OpenMP_C_FLAGS   "-Xclang -fopenmp" CACHE STRING "")

# Provide the actual library file for FindOpenMP
set(OpenMP_omp_LIBRARY "${OpenMP_LIB_DIR}/libomp.dylib" CACHE STRING "")

# Create imported target so packages that call find_package(OpenMP)
# receive correct includes + libs
if (NOT TARGET OpenMP::OpenMP_CXX)
    add_library(OpenMP::OpenMP_CXX INTERFACE IMPORTED)
    set_target_properties(OpenMP::OpenMP_CXX PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${OpenMP_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "-L${OpenMP_LIB_DIR};-lomp"
        INTERFACE_COMPILE_OPTIONS "-Xclang;-fopenmp"
    )
endif()

# macOS provides LAPACK and BLAS inside Accelerate framework
set(BLAS_LIBRARIES "-framework Accelerate" CACHE STRING "BLAS libraries")
set(LAPACK_LIBRARIES "-framework Accelerate" CACHE STRING "LAPACK libraries")

# Paths related to dependencies used at build time (absolute or relative)



# Google Benchmark paths (Homebrew)
set(CMAKE_PREFIX_PATH "/opt/homebrew/opt/google-benchmark;${CMAKE_PREFIX_PATH}")

#CLI
set(CLI11_INCLUDE_DIRS "/opt/homebrew/include" CACHE PATH "CLI11 include path" FORCE)
include_directories(SYSTEM ${CLI11_INCLUDE_DIRS})

# CSparse paths (Homebrew)
set(CSPARSE_INCLUDE_DIR "/opt/homebrew/include/suitesparse")
set(CSPARSE_LIBRARY "/opt/homebrew/lib/libsuitesparse.dylib")

# 1. Set the GDAL_CONFIG variable (often found in /opt/homebrew/bin)
set(GDAL_CONFIG_BIN "/opt/homebrew/bin/gdal-config" CACHE FILEPATH "Path to Homebrew gdal-config utility." FORCE)

# 2. Add it as an environment variable (often required by CMake's FindGDAL)
set(ENV{GDAL_CONFIG} ${GDAL_CONFIG_BIN})

# 3. Add the Homebrew prefix for general finding (if not already there)
# /opt/homebrew is needed so find_package(absl CONFIG) works for protobuf's abseil dependency
# GZ_INSTALL_PREFIX must come BEFORE /opt/homebrew so we pick up gz-ionic's protobuf/sdformat first
set(CMAKE_PREFIX_PATH
    "${GZ_INSTALL_PREFIX};/opt/homebrew;/opt/homebrew/opt/gdal;/opt/homebrew/opt/opencv;${CMAKE_PREFIX_PATH}" CACHE STRING "Prefix paths" FORCE)
    
#livox_ros_driver2
set(PCL_ALL_IN_ONE_INSTALLER OFF CACHE BOOL "Disable bundled Boost" FORCE)
set(ROS_EDITION "ROS2" CACHE STRING "ROS edition")
set(ROS_VERSION "2" CACHE STRING "ROS Version")
set(KILTED_ROS "kilted" CACHE STRING "ROS 2 kilted")
set(ROS_DISTRO "kilted" CACHE STRING "ROS 2 Distro")

# Qt5 path (for ROS packages)
set(Qt5_DIR "/opt/homebrew/opt/qt@5/lib/cmake/Qt5" CACHE PATH "Qt5 CMake path")
set(QT5_PREFIX "/opt/homebrew/opt/qt@5" CACHE PATH "Qt5 prefix path")

# Qt6 path (for Ignition Gazebo)
set(Qt6_DIR "/opt/homebrew/opt/qt/lib/cmake/Qt6" CACHE PATH "Qt6 CMake path")
set(QT6_PREFIX "/opt/homebrew/opt/qt" CACHE PATH "Qt6 prefix path")

# Add both Qt5 and Qt6 CMake modules to CMAKE_PREFIX_PATH
set(CMAKE_PREFIX_PATH "${QT6_PREFIX}/lib/cmake;${QT5_PREFIX}/lib/cmake;${CMAKE_PREFIX_PATH}" CACHE PATH "Qt CMake paths" FORCE)

# Add Qt5 and Qt6 binaries to PATH (for moc, uic, rcc)
set(ENV{PATH} "${QT6_PREFIX}/bin:${QT5_PREFIX}/bin:$ENV{PATH}")

# Tell CMake to look for frameworks last (helps prevent conflicts)
set(CMAKE_FIND_FRAMEWORK LAST)

# OpenGL (for Qt5Gui)
set(_GL_INCDIRS "/opt/homebrew/include" CACHE STRING "")
set(_qt5gui_OPENGL_INCLUDE_DIR "/opt/homebrew/include/GL" CACHE PATH "")
set(Qt5Gui_OPENGL_IMPLEMENTATION GL CACHE STRING "")
set(Qt5Gui_OPENGL_LIBRARIES "/opt/homebrew/lib/libGL.dylib" CACHE FILEPATH "")

# --- OpenCV (Homebrew) ---
set(OpenCV_DIR "/opt/homebrew/opt/opencv/lib/cmake/opencv4" CACHE PATH "OpenCV cmake config" FORCE)

# moveit_ros_perception
# Toolchain for macOS Homebrew OpenGL dependencies
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

# macOS: link system OpenGL framework for Ogre
set(OGRE_FRAMEWORKS "-framework OpenGL")
# Pass to global linker flags for executables & shared libs
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OGRE_FRAMEWORKS}" CACHE STRING "Link OpenGL framework for Ogre" FORCE)
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${OGRE_FRAMEWORKS}" CACHE STRING "Link OpenGL framework for Ogre" FORCE)

# --- Force TinyXML2 targets for Gazebo vendors (Mac/Homebrew fix) ---
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

# --- Additional Homebrew library search paths ---
# Many ROS packages use pkg-config or FindXXX to locate libraries.
# On macOS with Homebrew, keg-only and regular libs need explicit paths.
# CMAKE_LIBRARY_PATH works at find_library() time (more reliable than link_directories in toolchains)
list(APPEND CMAKE_LIBRARY_PATH
    /opt/homebrew/lib
    /opt/homebrew/opt/ffmpeg/lib
    /opt/homebrew/opt/theora/lib
    /opt/homebrew/opt/libogg/lib
    /opt/homebrew/opt/geos/lib
    /opt/homebrew/opt/proj/lib
    /opt/homebrew/opt/libspnav/lib
    /opt/homebrew/opt/zstd/lib
    /opt/homebrew/opt/zbar/lib
    /opt/homebrew/opt/google-benchmark/lib
    /opt/homebrew/opt/gstreamer/lib
    /opt/homebrew/opt/aravis/lib
)
list(APPEND CMAKE_INCLUDE_PATH
    /opt/homebrew/include
    /opt/homebrew/opt/ffmpeg/include
    /opt/homebrew/opt/zbar/include
    /opt/homebrew/opt/gstreamer/include/gstreamer-1.0
    /opt/homebrew/opt/aravis/include/aravis-0.8
    /opt/homebrew/opt/libspnav/include
    /opt/homebrew/opt/theora/include
    /opt/homebrew/opt/proj/include
    /opt/homebrew/opt/geos/include
)
# Also set PKG_CONFIG_PATH so pkg-config can find .pc files
set(ENV{PKG_CONFIG_PATH} "/opt/homebrew/opt/ffmpeg/lib/pkgconfig:/opt/homebrew/opt/theora/lib/pkgconfig:/opt/homebrew/opt/libogg/lib/pkgconfig:/opt/homebrew/opt/geos/lib/pkgconfig:/opt/homebrew/opt/proj/lib/pkgconfig:/opt/homebrew/opt/zstd/lib/pkgconfig:/opt/homebrew/opt/zbar/lib/pkgconfig:/opt/homebrew/opt/gstreamer/lib/pkgconfig:/opt/homebrew/opt/aravis/lib/pkgconfig:/opt/homebrew/opt/google-benchmark/lib/pkgconfig:/opt/homebrew/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
# link_directories for linking stage
link_directories(
    /opt/homebrew/lib
    /opt/homebrew/opt/ffmpeg/lib
    /opt/homebrew/opt/theora/lib
    /opt/homebrew/opt/libogg/lib
    /opt/homebrew/opt/geos/lib
    /opt/homebrew/opt/proj/lib
    /opt/homebrew/opt/libspnav/lib
    /opt/homebrew/opt/zstd/lib
    /opt/homebrew/opt/zbar/lib
    /opt/homebrew/opt/google-benchmark/lib
    /opt/homebrew/opt/gstreamer/lib
    /opt/homebrew/opt/aravis/lib
)

# --- Eigen (Homebrew) ---
# Use Eigen 5.0.1 (Homebrew's current eigen) — Ceres was compiled against it.
# Eigen 5 is the successor to Eigen 3.4 with compatible API.
set(Eigen3_DIR "/opt/homebrew/opt/eigen/share/eigen3/cmake" CACHE PATH "" FORCE)
list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/eigen")
# Eigen 5 Config only provides the Eigen3::Eigen target. Set legacy variables
# for packages that use ${EIGEN3_INCLUDE_DIRS} or ${EIGEN3_INCLUDE_DIR}.
set(EIGEN3_INCLUDE_DIRS "/opt/homebrew/opt/eigen/include/eigen3" CACHE PATH "Eigen3 include dir" FORCE)
set(EIGEN3_INCLUDE_DIR "/opt/homebrew/opt/eigen/include/eigen3" CACHE PATH "Eigen3 include dir" FORCE)
include_directories(SYSTEM "/opt/homebrew/opt/eigen/include/eigen3")

# --- Google glog ---
# glog is needed by nav2_constrained_smoother, slam_toolbox (via Ceres public API).
# Even though local Ceres uses miniglog, downstream packages that include
# Ceres headers may pull in glog. Provide the cmake config path so that
# find_package(glog) succeeds and sets correct compile definitions.
set(glog_DIR "/opt/homebrew/opt/glog/lib/cmake/glog" CACHE PATH "glog cmake config" FORCE)
list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/glog")

# --- urdfdom (explicit path for downstream MoveIt packages) ---
set(urdfdom_DIR "${WORKSPACE_ROOT}/install/lib/urdfdom/cmake" CACHE PATH "urdfdom cmake config" FORCE)
set(urdfdom_headers_DIR "${WORKSPACE_ROOT}/install/lib/urdfdom_headers/cmake" CACHE PATH "urdfdom_headers cmake config" FORCE)

# --- PCL Conversions ---
WORKSPACE_PATH(PCL_CONVERSIONS_INCLUDE_DIR "ros-perception/perception_pcl/pcl_conversions/include")
# Export for downstream packages
set(pcl_conversions_INCLUDE_DIRS "${PCL_CONVERSIONS_INCLUDE_DIR}" CACHE PATH "PCL Conversions include dirs" FORCE)

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

# Only force 'FOUND' to OFF for the vendor project itself
if(CMAKE_PROJECT_NAME STREQUAL "orocos_kdl_vendor")
    set(orocos_kdl_FOUND OFF CACHE BOOL "" FORCE)
    set(FORCE_BUILD_VENDOR_PKG ON CACHE BOOL "" FORCE)
endif()