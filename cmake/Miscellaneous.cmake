
function(add_interface_library)
    foreach(_LIB ${ARGN})
        add_library(${_LIB} INTERFACE)
        string(REPLACE "slate-roofline-" "" _TMP "${_LIB}")
        add_library(${PROJECT_NAME}::${_TMP} ALIAS ${_LIB})
    endforeach()
endfunction()

# generic library for compiler flags, include dirs, link libs, etc.
add_interface_library(slate-roofline-compile-flags)

# third-party libraries
add_interface_library(slate-roofline-tpls)

# enable warnings
target_compile_options(slate-roofline-compile-flags INTERFACE
    $<$<COMPILE_LANGUAGE:CXX>:-W -Wall>
    $<$<COMPILE_LANGUAGE:CUDA>:-Xcompiler=-W -Xcompiler=-Wall>)

find_package(MPI REQUIRED)
target_link_libraries(slate-roofline-tpls INTERFACE MPI::MPI_C)

if(USE_CUDA)
    enable_language(CUDA)
endif()

if(USE_PROFILING)
    target_compile_definitions(slate-roofline-compile-flags INTERFACE
        PROFILING)
endif()

if(USE_TIMEMORY)
    find_package(timemory REQUIRED
        COMPONENTS headers
        OPTIONAL_COMPONENTS cxx papi cuda cupti mpip-library)
    target_link_libraries(slate-roofline-tpls INTERFACE timemory::timemory)
    list(APPEND CMAKE_INSTALL_RPATH ${timemory_ROOT_DIR}/lib ${timemory_ROOT_DIR}/lib64)
    message(STATUS "${CMAKE_INSTALL_RPATH}")
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

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)
