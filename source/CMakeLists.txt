
target_include_directories(slate-roofline-compile-flags INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if(USE_TIMEMORY)
    add_subdirectory(timemory-interface)
    target_link_libraries(slate-roofline-tpls INTERFACE slate-roofline::timemory)
    target_compile_definitions(slate-roofline-tpls INTERFACE USE_TIMEMORY)
endif()

add_subdirectory(matrix-add-hsw)
add_subdirectory(matrix-add-cori-gpu)

# add_subdirectory(lib)
# add_subdirectory(pt2pt)
# add_subdirectory(eigensolvers)

# if(USE_PAPI)
#     add_subdirectory(papi-ib-example)
# endif()

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
