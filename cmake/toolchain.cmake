message(WARN " Toolchain.cmake is being used.")

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
set(ROS_WORKSPACE_INSTALL "$ENV{HOME}/kilted-ros2/install")
list(APPEND CMAKE_PREFIX_PATH "${ROS_WORKSPACE_INSTALL}")

# --- Force System Python 3.11 ---
set(PYTHON_EXECUTABLE "/Library/Frameworks/Python.framework/Versions/3.11/bin/python3" CACHE FILEPATH "Python 3.11 interpreter" FORCE)
set(Python3_EXECUTABLE "/Library/Frameworks/Python.framework/Versions/3.11/bin/python3" CACHE FILEPATH "Python 3.11 executable" FORCE)
set(Python3_ROOT_DIR "/Library/Frameworks/Python.framework/Versions/3.11" CACHE PATH "Python3 root directory" FORCE)

# Optional: if you want CMake to locate headers & libs too
set(PYTHON_LIBRARY "/Library/Frameworks/Python.framework/Versions/3.11/lib/libpython3.11.dylib" CACHE FILEPATH "Python 3.11 library" FORCE)
set(PYTHON_INCLUDE_DIR "/Library/Frameworks/Python.framework/Versions/3.11/include/python3.11" CACHE PATH "Python 3.11 include dir" FORCE)

# Ceres
set(BUILD_BENCHMARKS OFF CACHE BOOL "Disable building benchmarks" FORCE) 
set(BUILD_EXAMPLES OFF CACHE BOOL "Disable building examples" FORCE)

# --- yaml-cpp from ROS 2 kilted install ---
set(yaml-cpp_DIR "$ENV{HOME}/kilted-ros2/install/yaml_cpp_vendor/opt/yaml_cpp_vendor/lib/cmake/yaml-cpp" CACHE PATH "yaml-cpp config dir")
set(YAML_CPP_INCLUDE_DIRS
    "$ENV{HOME}/kilted-ros2/install/yaml_cpp_vendor/opt/yaml_cpp_vendor/include"
    CACHE PATH "yaml-cpp include dir")
set(YAML_CPP_LIBRARIES
    "$ENV{HOME}/kilted-ros2/install/yaml_cpp_vendor/opt/yaml_cpp_vendor/lib/libyaml-cpp.dylib"
    CACHE FILEPATH "yaml-cpp library")

# Use the paths
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
 )

# macOS specific linker
set(CMAKE_MACOSX_RPATH ON)

# Thread config
set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)

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

# Boost
# Path to your custom Boost installation
set(BOOST_ROOT "$ENV{HOME}/kilted-ros2/src/ros-commondep/boost-1.89" CACHE PATH "Boost root")
set(BOOST_INCLUDEDIR "${BOOST_ROOT}/include" CACHE PATH "Boost include")
set(BOOST_LIBRARYDIR "${BOOST_ROOT}/lib" CACHE PATH "Boost lib")

# Asio from Boost
set(THIRDPARTY_Asio ON CACHE BOOL "Allow Thirdparty Asio" FORCE)

# Ensure Boost can be found without searching system paths first
set(CMAKE_PREFIX_PATH "${BOOST_ROOT};${CMAKE_PREFIX_PATH}")

# Force modern CMake Boost behavior
set(Boost_NO_SYSTEM_PATHS ON CACHE BOOL "Don't search system paths for Boost")
set(Boost_NO_BOOST_CMAKE OFF CACHE BOOL "Allow BoostConfig.cmake if present")

# Make sure CMake uses your Boost include & lib paths
set(Boost_INCLUDE_DIRS "${BOOST_INCLUDEDIR}" CACHE PATH "Boost include dirs" FORCE)
set(Boost_LIBRARY_DIRS "${BOOST_LIBRARYDIR}" CACHE PATH "Boost library dir" FORCE)

# Force include + lib dirs globally
include_directories(SYSTEM ${BOOST_INCLUDEDIR})
link_directories(${BOOST_LIBRARYDIR})

# Globally enable deprecated Boost Timer API
add_definitions(-DBOOST_TIMER_ENABLE_DEPRECATED)

# Python-specific paths
set(Boost_PYTHON_LIBRARY "${BOOST_LIBRARYDIR}/libboost_python311.dylib" CACHE FILEPATH "Boost Python library")
set(Boost_PYTHON_INCLUDE_DIR "${BOOST_INCLUDEDIR}" CACHE PATH "Boost Python include dir")

# Optional: force CMake to find Boost in these directories
set(CMAKE_PREFIX_PATH "${BOOST_ROOT}/lib/cmake/Boost-1.89.0;${CMAKE_PREFIX_PATH}" CACHE PATH "Boost CMake path")

# Google Benchmark paths (Homebrew)
set(CMAKE_PREFIX_PATH "/opt/homebrew/opt/google-benchmark;${CMAKE_PREFIX_PATH}")

#CLI
set(CLI11_INCLUDE_DIRS "/opt/homebrew/include" CACHE PATH "CLI11 include path" FORCE)
include_directories(SYSTEM ${CLI11_INCLUDE_DIRS})

# CSparse paths (Homebrew)
set(CSPARSE_INCLUDE_DIR "/opt/homebrew/include/suitesparse")
set(CSPARSE_LIBRARY "/opt/homebrew/lib/libsuitesparse.dylib")

# Ceres
set(Ceres_DIR "$ENV{HOME}/kilted-ros2/install/ceres-solver/lib/cmake/Ceres" CACHE PATH "")

# ProtoBuf
set(PROTOBUF_INSTALL_PATH 
    "$ENV{HOME}/kilted-ros2/install/protobuf" 
    CACHE PATH "Installation path for source-built Protobuf" FORCE)

# Set the CMAKE_PREFIX_PATH to prioritize the source install over Homebrew
set(CMAKE_PREFIX_PATH 
    "${PROTOBUF_INSTALL_PATH};${CMAKE_PREFIX_PATH}")

# Explicitly add the include directory to the system path
include_directories(SYSTEM "${PROTOBUF_INSTALL_PATH}/include")

# Explicitly set the variables that packages like gz-msgs will search for
set(PROTOBUF_INCLUDE_DIR "${PROTOBUF_INSTALL_PATH}/include" CACHE PATH "Protobuf Include Dir" FORCE)
set(PROTOBUF_LIBRARY "${PROTOBUF_INSTALL_PATH}/lib/libprotobuf.dylib" CACHE FILEPATH "Protobuf Library" FORCE)

# 1. Set the GDAL_CONFIG variable (often found in /opt/homebrew/bin)
set(GDAL_CONFIG_BIN "/opt/homebrew/bin/gdal-config" CACHE FILEPATH "Path to Homebrew gdal-config utility." FORCE)

# 2. Add it as an environment variable (often required by CMake's FindGDAL)
set(ENV{GDAL_CONFIG} ${GDAL_CONFIG_BIN})

# 3. Add the Homebrew prefix for general finding (if not already there)
set(CMAKE_PREFIX_PATH 
    "/opt/homebrew/opt/gdal;${CMAKE_PREFIX_PATH}" CACHE STRING "Prefix paths" FORCE)
    
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

# --- Eigen (Homebrew) ---
# Ensure CMake can find Eigen headers
set(EIGEN3_INCLUDE_DIR "/opt/homebrew/include/eigen3" CACHE PATH "Eigen3 include directory" FORCE)
include_directories(SYSTEM ${EIGEN3_INCLUDE_DIR})
set(CMAKE_PREFIX_PATH "/opt/homebrew/include/eigen3;${CMAKE_PREFIX_PATH}" CACHE STRING "CMake prefix path" FORCE)

# --- Google glog (Homebrew) ---
set(GLOG_INCLUDE_DIR "/opt/homebrew/include" CACHE PATH "glog include path" FORCE)
set(GLOG_LIBRARY "/opt/homebrew/lib/libglog.dylib" CACHE FILEPATH "glog library" FORCE)

# Add include directory globally
include_directories(SYSTEM ${GLOG_INCLUDE_DIR})

# Append glog library to all targets' linker flags
# This ensures Ceres symbols (used in nav2_constrained_smoother) are found
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GLOG_LIBRARY}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${GLOG_LIBRARY}")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${GLOG_LIBRARY}")

# --- PCL Conversions ---
set(PCL_CONVERSIONS_INCLUDE_DIR "$ENV{HOME}/kilted-ros2/src/ros-perception/perception_pcl/pcl_conversions/include")
# Export for downstream packages
set(pcl_conversions_INCLUDE_DIRS "${PCL_CONVERSIONS_INCLUDE_DIR}" CACHE PATH "PCL Conversions include dirs" FORCE)
