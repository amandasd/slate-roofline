
add_executable(matrix-add matrix_add.cu $<TARGET_OBJECTS:slate-roofline::timemory>)

target_link_libraries(matrix-add PRIVATE
    slate-roofline::compile-flags
    slate-roofline::tpls)

if(USE_COMPILER_INSTRUMENTATION)
    target_link_libraries(matrix-add PRIVATE
        timemory::timemory-compiler-instrument)
endif()
