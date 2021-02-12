
file(GLOB headers
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)

file(GLOB sources
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(timemory-interface OBJECT ${sources} ${headers})

target_link_libraries(timemory-interface PUBLIC
    slate-roofline::compile-flags
    timemory::timemory)

target_compile_definitions(timemory-interface PUBLIC
    USE_TIMEMORY)

add_library(slate-roofline::timemory ALIAS timemory-interface)
