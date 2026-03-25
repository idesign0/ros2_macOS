cmake_minimum_required(VERSION 3.15)
project(cx_up_nextflap)

# Require C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ament_cmake REQUIRED)
find_package(pybind11_vendor REQUIRED)
find_package(pybind11 REQUIRED)
find_package(cx_z3_vendor REQUIRED)
find_package(Z3 REQUIRED)

# Get Python information
find_package(Python3 COMPONENTS Interpreter Development)

# Define source files
set(SOURCES
    nextflap/nextflap.cpp
    nextflap/parser/parser.cpp
    nextflap/parser/syntaxAnalyzer.cpp
    nextflap/parser/parsedTask.cpp
    nextflap/preprocess/preprocess.cpp
    nextflap/preprocess/preprocessedTask.cpp
    nextflap/grounder/grounder.cpp
    nextflap/grounder/groundedTask.cpp
    nextflap/heuristics/evaluator.cpp
    nextflap/heuristics/hFF.cpp
    nextflap/heuristics/hLand.cpp
    nextflap/heuristics/landmarks.cpp
    nextflap/heuristics/numericRPG.cpp
    nextflap/heuristics/rpg.cpp
    nextflap/heuristics/temporalRPG.cpp
    nextflap/planner/intervalCalculations.cpp
    nextflap/planner/linearizer.cpp
    nextflap/planner/plan.cpp
    nextflap/planner/planBuilder.cpp
    nextflap/planner/planComponents.cpp
    nextflap/planner/planEffects.cpp
    nextflap/planner/planner.cpp
    nextflap/planner/plannerSetting.cpp
    nextflap/planner/printPlan.cpp
    nextflap/planner/selector.cpp
    nextflap/planner/state.cpp
    nextflap/planner/successors.cpp
    nextflap/planner/z3Checker.cpp
    nextflap/sas/mutexGraph.cpp
    nextflap/sas/sasTask.cpp
    nextflap/sas/sasTranslator.cpp
    nextflap/utils/utils.cpp)

# Add library add_library(nextflap SHARED ${SOURCES})

pybind11_add_module(nextflap ${SOURCES})
# Include directories
target_include_directories(nextflap PUBLIC nextflap/ ${Python3_INCLUDE_DIRS}
                                           ${Z3_INCLUDE_DIRS})
# set_target_properties(nextflap PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON
# CXX_VISIBILITY_PRESET default)
#
# Link libraries
target_link_libraries(nextflap PUBLIC z3::libz3)
# Locate ament_package template files.
set(PYTHON_INSTALL_DIR "lib/python${PYTHON_MAJOR_MINOR}/site-packages")

# Install library for actual use
install(TARGETS nextflap DESTINATION "${PYTHON_INSTALL_DIR}")

target_link_libraries(nextflap PRIVATE pybind11::module)

set_property(TARGET nextflap PROPERTY CXX_STANDARD ${CMAKE_CXX_STANDARD})

# Install Python modules
ament_python_install_package(up_nextflap)

# Installation rules
install(
  TARGETS ${OUTPUT_FILE}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
ament_package()
