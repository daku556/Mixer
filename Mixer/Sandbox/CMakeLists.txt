project(Sandbox)

file(GLOB_RECURSE SANDBOX_SRC
    "src/*.cpp"
    "src/*.h"
)

add_executable(Sandbox ${SANDBOX_SRC})

target_include_directories(Sandbox PUBLIC
    ${CMAKE_SOURCE_DIR}/Mixer/vendor/spdlog/include
    ${CMAKE_SOURCE_DIR}/Mixer/src
)

target_link_libraries(Sandbox PRIVATE Mixer)

target_compile_definitions(Sandbox PRIVATE MX_PLATFORM_WINDOWS)
target_compile_options(Sandbox PRIVATE "/utf-8")
