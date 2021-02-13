cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    set(MSG "")
    message(STATUS "Warning! Building from the source directory will overwrite Makefiles")
    message(STATUS "Remove 'CMakeCache.txt' and 'CMakeFiles' and build from a separate directory")
    message(FATAL_ERROR "In-source build")
endif()

project(slate-roofline LANGUAGES C CXX VERSION 0.0.1)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(USE_TIMEMORY "Enable timemory support" ON)
option(USE_PROFILING "Enable MPI_Wtime profiling" ON)

option(USE_CUDA "Enable CUDA build" OFF)
option(USE_PAPI "Enable PAPI support" OFF)
option(USE_COMPILER_INSTRUMENTATION "Enable compiler instrumentation" OFF)

# default installation dirs
include(GNUInstallDirs)

# miscellaneous stuff: interface libraries, etc.
include(Miscellaneous)

add_subdirectory(source)
