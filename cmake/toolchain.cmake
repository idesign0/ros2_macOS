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

# Ceres
set(BUILD_BENCHMARKS OFF CACHE BOOL "Disable building benchmarks" FORCE) 
set(BUILD_EXAMPLES OFF CACHE BOOL "Disable building examples" FORCE)

# --- yaml-cpp from ROS 2 Humble install ---
set(YAML_CPP_INCLUDE_DIRS
    "$ENV{HOME}/humble-ros2/install/yaml_cpp_vendor/opt/yaml_cpp_vendor/include"
    CACHE PATH "yaml-cpp include dir")

set(YAML_CPP_LIBRARIES
    "$ENV{HOME}/humble-ros2/install/yaml_cpp_vendor/opt/yaml_cpp_vendor/lib/libyaml-cpp.dylib"
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

# Suppress warning as errors
add_compile_options(-Wno-unused-command-line-argument
        -Wno-pessimizing-move
        -Wno-error=pessimizing-move
        -Wno-deprecated-dynamic-exception-spec
        -Wno-unused-variable
        -Wno-unused-parameter
        -Wno-unused-but-set-variable
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

include_directories(SYSTEM "${OpenMP_INCLUDE_DIR}")

set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -Xclang -fopenmp")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xclang -fopenmp")

set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS} -L${OpenMP_LIB_DIR} -lomp")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -L${OpenMP_LIB_DIR} -lomp")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -L${OpenMP_LIB_DIR} -lomp")

# Create the imported target so downstream packages can use it
if (NOT TARGET OpenMP::OpenMP_CXX)
    add_library(OpenMP::OpenMP_CXX INTERFACE IMPORTED)
    set_target_properties(OpenMP::OpenMP_CXX PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${OpenMP_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "-L${OpenMP_LIB_DIR} -lomp"
    )
endif()

# macOS provides LAPACK and BLAS inside Accelerate framework
set(BLAS_LIBRARIES "-framework Accelerate" CACHE STRING "BLAS libraries")
set(LAPACK_LIBRARIES "-framework Accelerate" CACHE STRING "LAPACK libraries")

# Paths related to dependencies used at build time (absolute or relative)

# Boost
# Path to your custom Boost installation
set(BOOST_ROOT "$ENV{HOME}/humble-ros2/src/ros-commondep/boost-1.89" CACHE PATH "Boost root")
set(BOOST_INCLUDEDIR "${BOOST_ROOT}/include" CACHE PATH "Boost include")
set(BOOST_LIBRARYDIR "${BOOST_ROOT}/lib" CACHE PATH "Boost lib")

# Asio from Boost
set(THIRDPARTY_Asio ON CACHE BOOL "Allow Thirdparty Asio" FORCE)
set(Asio_INCLUDE_DIR "$ENV{HOME}/humble-ros2/src/ros-commondep/asio-1.10.8/asio/include" CACHE PATH "Asio include directory" FORCE)

# Ensure Boost can be found without searching system paths first
set(CMAKE_PREFIX_PATH "${BOOST_ROOT};${CMAKE_PREFIX_PATH}")

# Force modern CMake Boost behavior
set(Boost_NO_SYSTEM_PATHS ON CACHE BOOL "Don't search system paths for Boost")
set(Boost_NO_BOOST_CMAKE OFF CACHE BOOL "Allow BoostConfig.cmake if present")
find_package(Boost REQUIRED)

# Make sure CMake uses your Boost include & lib paths
set(Boost_INCLUDE_DIRS "${BOOST_INCLUDEDIR}" CACHE PATH "Boost include dirs" FORCE)
set(Boost_LIBRARY_DIRS "${BOOST_LIBRARYDIR}" CACHE PATH "Boost library dir" FORCE)

# Force include + lib dirs globally
include_directories(SYSTEM ${BOOST_INCLUDEDIR})
link_directories(${BOOST_LIBRARYDIR})

# Globally enable deprecated Boost Timer API
add_definitions(-DBOOST_TIMER_ENABLE_DEPRECATED)

# Google Benchmark paths (Homebrew)
set(CMAKE_PREFIX_PATH "/opt/homebrew/opt/google-benchmark;${CMAKE_PREFIX_PATH}")

# CSparse paths (Homebrew)
set(CSPARSE_INCLUDE_DIR "/opt/homebrew/include/suitesparse")
set(CSPARSE_LIBRARY "/opt/homebrew/lib/libsuitesparse.dylib")

# Ceres
set(Ceres_DIR "$ENV{HOME}/humble-ros2/install/ceres-solver/lib/cmake/Ceres" CACHE PATH "")

#livox_ros_driver2
set(PCL_ALL_IN_ONE_INSTALLER OFF CACHE BOOL "Disable bundled Boost" FORCE)
set(ROS_EDITION "ROS2" CACHE STRING "ROS edition")
set(ROS_VERSION "2" CACHE STRING "ROS Version")
set(HUMBLE_ROS "humble" CACHE STRING "ROS 2 Humble")
set(ROS_DISTRO "humble" CACHE STRING "ROS 2 Distro")

# Qt5 installation prefix (Homebrew)
set(QT5_PREFIX "/opt/homebrew/opt/qt@5" CACHE PATH "Qt5 prefix path")

# Add Qt5 CMake modules to CMAKE_PREFIX_PATH (Homebrew first)
set(CMAKE_PREFIX_PATH "${QT5_PREFIX}/lib/cmake;${CMAKE_PREFIX_PATH}" CACHE PATH "Prefix path for Qt5" FORCE)

# Add Qt5 binaries to PATH (for moc, uic, rcc)
set(ENV{PATH} "${QT5_PREFIX}/bin:$ENV{PATH}")

# Tell CMake to look for frameworks last (helps prevent conflicts with system Qt/OpenGL)
set(CMAKE_FIND_FRAMEWORK LAST)

# --- OpenGL from Homebrew Mesa ---
# In toolchain.cmake, before find_package(Qt5 COMPONENTS Gui REQUIRED)
set(_GL_INCDIRS "/opt/homebrew/include" CACHE STRING "")
set(_qt5gui_OPENGL_INCLUDE_DIR "/opt/homebrew/include/GL" CACHE PATH "")
set(Qt5Gui_OPENGL_IMPLEMENTATION GL CACHE STRING "")

# Libraries
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