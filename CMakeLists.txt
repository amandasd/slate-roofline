cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    set(MSG "")
    message(STATUS "Warning! Building from the source directory will overwrite Makefiles")
    message(STATUS "Remove 'CMakeCache.txt' and 'CMakeFiles' and build from a separate directory")
    message(FATAL_ERROR "In-source build")
endif()

project(slate-roofline LANGUAGES C CXX VERSION 0.0.1)

option(USE_CUDA "Enable CUDA build" OFF)
option(USE_PAPI "Enable PAPI support" OFF)
option(USE_TIMEMORY "Enable timemory support" ON)
option(USE_COMPILER_INSTRUMENTATION "Enable compiler instrumentation" OFF)
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

# if build type is not defined, default to release build
if("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

# build shared libraries
if(BUILD_SHARED_LIBS)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON) # enable -fPIC
    set(LIBRARY_TYPE SHARED)
else()
    set(LIBRARY_TYPE STATIC)
endif()

# generic library for compiler flags, include dirs, link libs, etc.
add_library(slate-roofline-compile-flags INTERFACE)
# third-party libraries
add_library(slate-roofline-tpls INTERFACE)

add_library(slate-roofline::compile-flags ALIAS slate-roofline-compile-flags)
add_library(slate-roofline::tpls ALIAS slate-roofline-tpls)

# enable warnings
target_compile_options(slate-roofline-compile-flags INTERFACE
    $<$<COMPILE_LANGUAGE:CXX>:-W -Wall>
    $<$<COMPILE_LANGUAGE:CUDA>:-Xcompiler=-W -Xcompiler=-Wall>)

find_package(MPI REQUIRED)
target_link_libraries(slate-roofline-tpls INTERFACE MPI::MPI_C)

if(USE_CUDA)
    enable_language(CUDA)
endif()

if(USE_TIMEMORY)
    find_package(timemory REQUIRED
        COMPONENTS headers
        OPTIONAL_COMPONENTS c cxx papi cuda cupti mpip-library)
    target_link_libraries(slate-roofline-tpls INTERFACE timemory::timemory)
endif()

if(USE_PAPI)
    if(NOT timemory_FOUND)
        find_package(timemory QUIET)
    endif()
    if(timemory_DIR)
        list(APPEND CMAKE_MODULE_PATH ${timemory_DIR}/Modules)
    endif()
    find_package(PAPI REQUIRED)
    target_compile_definitions(slate-roofline-tpls INTERFACE PAPI)
    target_link_libraries(slate-roofline-tpls INTERFACE PAPI::papi-static)
endif()
# add_subdirectory(lib)
# add_subdirectory(pt2pt)
# add_subdirectory(eigensolvers)

# if(USE_PAPI)
#     add_subdirectory(papi-ib-example)
# endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

if(USE_TIMEMORY)
    add_subdirectory(timemory-interface)
    target_link_libraries(slate-roofline-tpls INTERFACE slate-roofline::timemory)
    target_compile_definitions(slate-roofline-tpls INTERFACE USE_TIMEMORY)
endif()

target_include_directories(slate-roofline-compile-flags INTERFACE
    ${PROJECT_SOURCE_DIR}/timemory-interface)

if(USE_CUDA)
    add_subdirectory(matrix-add-gpu)
else()
    add_subdirectory(matrix-add-hsw)
endif()

# prefer clang-format 6.0
find_program(CLANG_FORMATTER
    NAMES
        clang-format-6
        clang-format-6.0
        clang-format-mp-6.0
        clang-format)

if(CLANG_FORMATTER)
    file(GLOB_RECURSE files
        ${PROJECT_SOURCE_DIR}/*.h
        ${PROJECT_SOURCE_DIR}/*.c
        ${PROJECT_SOURCE_DIR}/*.cu
        ${PROJECT_SOURCE_DIR}/*.cpp)

    add_custom_target(format
        COMMAND             ${CLANG_FORMATTER} -i ${files}
        COMMENT             "[${PROJECT_NAME}] Running '${CLANG_FORMATTER}'..."
        SOURCES             ${files}
        WORKING_DIRECTORY   ${PROJECT_SOURCE_DIR})
endif()
