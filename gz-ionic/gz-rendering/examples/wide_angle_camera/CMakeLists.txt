cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)
project(gz-rendering-wide-angle-camera)
find_package(gz-rendering9 REQUIRED)

include_directories(SYSTEM
  ${PROJECT_BINARY_DIR}
)

if (APPLE OR UNIX)
  find_package(GLUT REQUIRED)
  include_directories(SYSTEM ${GLUT_INCLUDE_DIRS})
  link_directories(${GLUT_LIBRARY_DIRS})

  find_package(OpenGL REQUIRED)
  include_directories(SYSTEM ${OpenGL_INCLUDE_DIRS})
  link_directories(${OpenGL_LIBRARY_DIRS})

  set(TARGET_THIRD_PARTY_DEPENDS
    ${TARGET_THIRD_PARTY_DEPENDS}
    ${OPENGL_LIBRARIES}
    ${GLUT_LIBRARIES}
  )
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
endif()

if (NOT APPLE)
  find_package(GLEW REQUIRED)
  include_directories(SYSTEM ${GLEW_INCLUDE_DIRS})
  link_directories(${GLEW_LIBRARY_DIRS})

  set(TARGET_THIRD_PARTY_DEPENDS
    ${TARGET_THIRD_PARTY_DEPENDS}
    ${GLEW_LIBRARIES}
  )
endif()

if (WIN32)
  find_package(FreeGLUT REQUIRED)
  set(TARGET_THIRD_PARTY_DEPENDS ${TARGET_THIRD_PARTY_DEPENDS} FreeGLUT::freeglut)
endif()

add_executable(wide_angle_camera Main.cc GlutWindow.cc)

target_link_libraries(wide_angle_camera
  ${GZ-RENDERING_LIBRARIES}
  ${TARGET_THIRD_PARTY_DEPENDS}
)

if (WIN32)
  set_target_properties(wide_angle_camera
    PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
      RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_BINARY_DIR}
      RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_BINARY_DIR}
      RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${PROJECT_BINARY_DIR}
      RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${PROJECT_BINARY_DIR}
  )
endif()
