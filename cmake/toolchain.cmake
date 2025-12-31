message(WARN " Toolchain.cmake is being used.")

# --- DYNAMIC PATH DETECTION (Local vs CI) ---
if(CI_BUILD OR "$ENV{CI_BUILD}" STREQUAL "TRUE")
    set(WORKSPACE_ROOT "$ENV{GITHUB_WORKSPACE}")
else()
    set(WORKSPACE_ROOT "$ENV{HOME}/humble-ros2")
endif()

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

# --- Force System Python 3.11 ---
set(PYTHON_EXECUTABLE "/Library/Frameworks/Python.framework/Versions/3.11/bin/python3" CACHE FILEPATH "Python 3.11 interpreter" FORCE)
set(Python3_EXECUTABLE "/Library/Frameworks/Python.framework/Versions/3.11/bin/python3" CACHE FILEPATH "Python 3.11 executable" FORCE)
set(Python3_ROOT_DIR "/Library/Frameworks/Python.framework/Versions/3.11" CACHE PATH "Python3 root directory" FORCE)
set(PYTHON_LIBRARY "/Library/Frameworks/Python.framework/Versions/3.11/lib/libpython3.11.dylib" CACHE FILEPATH "Python 3.11 library" FORCE)
set(PYTHON_INCLUDE_DIR "/Library/Frameworks/Python.framework/Versions/3.11/include/python3.11" CACHE PATH "Python 3.11 include dir" FORCE)

# Boost (Custom ROS 2 build, not Homebrew)
set(BOOST_ROOT "${WORKSPACE_ROOT}/src/ros-commondep/boost-1.89" CACHE PATH "Boost root")
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
set(CMAKE_MACOSX_RPATH ON)
set(CMAKE_SKIP_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(BASE_RPATH "@loader_path/../lib")
list(APPEND BASE_RPATH "${BOOST_LIBRARYDIR}")
set(CMAKE_BUILD_RPATH "${BASE_RPATH}")
set(CMAKE_INSTALL_RPATH "${BASE_RPATH}")

# Ceres, yaml-cpp, PCL Conversions, pybind11 (Non-Homebrew/Vendor dependencies)
set(BUILD_BENCHMARKS OFF CACHE BOOL "Disable building benchmarks" FORCE) 
set(BUILD_EXAMPLES OFF CACHE BOOL "Disable building examples" FORCE)
set(yaml-cpp_DIR "${WORKSPACE_ROOT}/install/opt/yaml_cpp_vendor/lib/cmake/yaml-cpp" CACHE PATH "yaml-cpp config directory")
set(YAML_CPP_INCLUDE_DIRS "${WORKSPACE_ROOT}/install/opt/yaml_cpp_vendor/include" CACHE PATH "yaml-cpp include directory")
set(YAML_CPP_LIBRARIES "${WORKSPACE_ROOT}/install/opt/yaml_cpp_vendor/lib/libyaml-cpp.dylib" CACHE FILEPATH "yaml-cpp library")
include_directories(${YAML_CPP_INCLUDE_DIRS})
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
 )

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
set(PCL_CONVERSIONS_INCLUDE_DIR "${WORKSPACE_ROOT}/src/ros-perception/perception_pcl/pcl_conversions/include")
set(pcl_conversions_INCLUDE_DIRS "${PCL_CONVERSIONS_INCLUDE_DIR}" CACHE PATH "PCL Conversions include dirs" FORCE)
set(pybind11_DIR "${WORKSPACE_ROOT}/install/opt/pybind11_vendor/share/cmake/pybind11" CACHE PATH "Path to pybind11_vendor CMake config" FORCE)

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

# --- Eigen (Homebrew) - Forced for VTK/PCL Compatibility ---
set(EIGEN_ROOT_PATH "/opt/homebrew/opt/eigen@3")
# We include both the versioned folder and the parent folder to satisfy 
# both #include <Eigen/Core> and #include <eigen3/Eigen/Core>
set(EIGEN_INCLUDE_PATH "${EIGEN_ROOT_PATH}/include/eigen3")
set(EIGEN_PARENT_INCLUDE_PATH "${EIGEN_ROOT_PATH}/include")

set(Eigen3_DIR "${EIGEN_ROOT_PATH}/share/eigen3/cmake" CACHE PATH "Force Eigen3 dir" FORCE)
set(EIGEN3_ROOT "${EIGEN_ROOT_PATH}" CACHE PATH "Root hint for Eigen3" FORCE)
set(EIGEN3_INCLUDE_DIR "${EIGEN_INCLUDE_PATH}" CACHE PATH "" FORCE)
set(Eigen3_INCLUDE_DIRS "${EIGEN_INCLUDE_PATH}" CACHE PATH "" FORCE)
set(EIGEN3_INCLUDE_DIRS "${EIGEN_INCLUDE_PATH}" CACHE PATH "" FORCE)

set(Eigen3_FOUND TRUE CACHE BOOL "" FORCE)
set(EIGEN3_FOUND TRUE CACHE BOOL "" FORCE)

if(NOT TARGET Eigen3::Eigen)
    add_library(Eigen3::Eigen INTERFACE IMPORTED)
    set_target_properties(Eigen3::Eigen PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${EIGEN_INCLUDE_PATH};${EIGEN_PARENT_INCLUDE_PATH}"
    )
endif()

# This is the key fix for the fatal error:
include_directories(SYSTEM "${EIGEN_INCLUDE_PATH}")
include_directories(SYSTEM "${EIGEN_PARENT_INCLUDE_PATH}")

list(PREPEND CMAKE_PREFIX_PATH "${EIGEN_ROOT_PATH}")

# --- Force FCL and libccd to use correct paths ---
# Define paths for libccd (usually installed via Homebrew)
set(CCD_INCLUDE_DIR "/opt/homebrew/include" CACHE PATH "" FORCE)
set(CCD_LIBRARY "/opt/homebrew/lib/libccd.dylib" CACHE FILEPATH "" FORCE)

# Force FCL variables
set(fcl_FOUND TRUE CACHE BOOL "Satisfy find_package(fcl)" FORCE)
set(FCL_FOUND TRUE CACHE BOOL "Satisfy find_package(FCL)" FORCE)
set(fcl_INCLUDE_DIRS "/opt/homebrew/include;/opt/homebrew/opt/eigen@3/include/eigen3" CACHE PATH "" FORCE)
set(fcl_LIBRARIES "/opt/homebrew/lib/libfcl.dylib;${CCD_LIBRARY}" CACHE FILEPATH "" FORCE)

# Define the fcl target with the FULL PATH to libccd
if(NOT TARGET fcl)
    add_library(fcl INTERFACE IMPORTED)
    set_target_properties(fcl PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "/opt/homebrew/include;/opt/homebrew/opt/eigen@3/include/eigen3"
        # We use the absolute path to libccd here so the linker doesn't have to "guess"
        INTERFACE_LINK_LIBRARIES "/opt/homebrew/lib/libfcl.dylib;${CCD_LIBRARY}"
    )
endif()

# Add Homebrew lib to link directories globally as a safety net
link_directories(/opt/homebrew/lib)

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