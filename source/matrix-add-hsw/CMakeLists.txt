
add_executable(matrix-add-hsw matrix_add.c $<TARGET_OBJECTS:slate-roofline::timemory>)

target_link_libraries(matrix-add-hsw PRIVATE
    slate-roofline::compile-flags
    slate-roofline::tpls)

if(USE_COMPILER_INSTRUMENTATION)
    target_link_libraries(matrix-add-hsw PRIVATE
        timemory::timemory-compiler-instrument)
endif()

install(TARGETS matrix-add-hsw
    DESTINATION ${CMAKE_INSTALL_BINDIR})
