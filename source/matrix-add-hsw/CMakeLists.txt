
add_executable(matrix-add-cpu matrix_add.c $<TARGET_OBJECTS:slate-roofline::timemory>)

target_link_libraries(matrix-add-cpu PRIVATE
    slate-roofline::compile-flags
    slate-roofline::tpls)

if(USE_COMPILER_INSTRUMENTATION)
    target_link_libraries(matrix-add-cpu PRIVATE
        timemory::timemory-compiler-instrument)
endif()

install(TARGETS matrix-add-cpu
    DESTINATION ${CMAKE_INSTALL_BINDIR})
