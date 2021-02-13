
if(NOT USE_CUDA)
    return()
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

install(TARGETS matrix-add-gpu matrix-add-gpu-pinned
    DESTINATION ${CMAKE_INSTALL_BINDIR})
