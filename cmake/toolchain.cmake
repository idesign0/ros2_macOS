message(WARN " Toolchain.cmake is being used.")

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "Disable building tests" FORCE)
set(BUILD_UNIT_TESTS OFF CACHE BOOL "Disable building tests" FORCE)

# Ceres
set(BUILD_BENCHMARKS OFF CACHE BOOL "Disable building benchmarks" FORCE) 
set(BUILD_EXAMPLES OFF CACHE BOOL "Disable building examples" FORCE)

# Optimization flags
if(CMAKE_BUILD_TYPE STREQUAL "Release")
  add_compile_options(-O3 -march=native)
endif()


# Suppress treating warnings as errors
string(REPLACE "-Werror" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(REPLACE "-Werror" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

# Suppress warning as errors
add_compile_options(-Wno-unused-command-line-argument
        -Wno-pessimizing-move
        -Wno-error=pessimizing-move
        -Wno-deprecated-dynamic-exception-spec
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

# Ensure Boost can be found without searching system paths first
set(CMAKE_PREFIX_PATH "${BOOST_ROOT};${CMAKE_PREFIX_PATH}")

# Force modern CMake Boost behavior
set(Boost_NO_SYSTEM_PATHS ON CACHE BOOL "Don't search system paths for Boost")
set(Boost_NO_BOOST_CMAKE OFF CACHE BOOL "Allow BoostConfig.cmake if present")

# Actually find Boost now, so Boost::boost is defined globally
find_package(Boost REQUIRED)   # Add COMPONENTS here if you want (e.g., REQUIRED headers, system, thread)

# Make the target visible to everything using this toolchain
set(Boost_INCLUDE_DIRS ${Boost_INCLUDE_DIRS} CACHE PATH "Boost include dirs" FORCE)
set(Boost_LIBRARIES ${Boost_LIBRARIES} CACHE STRING "Boost libraries" FORCE)

# Globally enable deprecated Boost Timer API
add_definitions(-DBOOST_TIMER_ENABLE_DEPRECATED)

# Google Benchmark paths (Homebrew)
set(CMAKE_PREFIX_PATH "/opt/homebrew/Cellar/google-benchmark/1.9.4;${CMAKE_PREFIX_PATH}")

# CSparse paths (Homebrew)
set(CSPARSE_INCLUDE_DIR "/opt/homebrew/include/suitesparse")
set(CSPARSE_LIBRARY "/opt/homebrew/lib/libsuitesparse.dylib")

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