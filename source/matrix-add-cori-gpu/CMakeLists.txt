if(NOT USE_CUDA)
    return()
endif()

#
#      cpu 
#
add_executable(matrix-add-cpu matrix_add_cpu.c $<TARGET_OBJECTS:slate-roofline::timemory>)

target_link_libraries(matrix-add-cpu PRIVATE
    slate-roofline::compile-flags
    slate-roofline::tpls)

if(USE_COMPILER_INSTRUMENTATION)
    target_link_libraries(matrix-add-cpu PRIVATE
        timemory::timemory-compiler-instrument)
endif()

#
#       Non-pinned memory
#
add_executable(matrix-add-gpu matrix_add_gpu.cu $<TARGET_OBJECTS:slate-roofline::timemory>)

target_link_libraries(matrix-add-gpu PRIVATE
    slate-roofline::compile-flags
    slate-roofline::tpls)

if(USE_COMPILER_INSTRUMENTATION)
    target_link_libraries(matrix-add-gpu PRIVATE
        timemory::timemory-compiler-instrument)
endif()

#
#       Pinned memory
#
add_executable(matrix-add-gpu-pinned matrix_add_gpu.cu $<TARGET_OBJECTS:slate-roofline::timemory>)

target_compile_definitions(matrix-add-gpu-pinned PRIVATE PINNED)

target_link_libraries(matrix-add-gpu-pinned PRIVATE
    slate-roofline::compile-flags
    slate-roofline::tpls)

if(USE_COMPILER_INSTRUMENTATION)
    target_link_libraries(matrix-add-gpu-pinned PRIVATE
        timemory::timemory-compiler-instrument)
endif()

install(TARGETS matrix-add-gpu matrix-add-gpu-pinned matrix-add-cpu
    DESTINATION ${CMAKE_INSTALL_BINDIR})
