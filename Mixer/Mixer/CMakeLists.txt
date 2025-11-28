project(Mixer)

file(GLOB_RECURSE MIXER_SRC
    "src/*.cpp"
    "src/*.h"
)

add_library(Mixer SHARED ${MIXER_SRC})

target_include_directories(Mixer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Mixer
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog/include
)

target_compile_definitions(Mixer
    PRIVATE MX_BUILD_DLL
    PUBLIC MX_PLATFORM_WINDOWS
)

target_compile_options(Mixer PRIVATE "/utf-8")

# Post-build: copy Mixer.dll to Sandbox folder
add_custom_command(TARGET Mixer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        ${CMAKE_SOURCE_DIR}/bin/${OUTPUT_DIR}/Sandbox
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:Mixer>
        ${CMAKE_SOURCE_DIR}/bin/${OUTPUT_DIR}/Sandbox
)
