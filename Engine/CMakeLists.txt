# ============================================================
# Engine
# ============================================================

set(LUMORA_ENGINE_SOURCE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/Source"
)

set(LUMORA_ENGINE_LAUNCH_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/Launch"
)

set(LUMORA_ENGINE_SHADER_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/../Assets/Shaders"
)

file(GLOB_RECURSE LUMORA_ENGINE_SOURCES
    CONFIGURE_DEPENDS
    "${LUMORA_ENGINE_SOURCE_DIR}/*.c"
    "${LUMORA_ENGINE_SOURCE_DIR}/*.cpp"
    "${LUMORA_ENGINE_SOURCE_DIR}/*.h"
    "${LUMORA_ENGINE_SOURCE_DIR}/*.hpp"
    "${LUMORA_ENGINE_SOURCE_DIR}/*.inl"
)

file(GLOB_RECURSE LUMORA_ENGINE_LAUNCH_SOURCES
    CONFIGURE_DEPENDS
    "${LUMORA_ENGINE_LAUNCH_DIR}/*.c"
    "${LUMORA_ENGINE_LAUNCH_DIR}/*.cpp"
    "${LUMORA_ENGINE_LAUNCH_DIR}/*.h"
    "${LUMORA_ENGINE_LAUNCH_DIR}/*.hpp"
    "${LUMORA_ENGINE_LAUNCH_DIR}/*.inl"
)

file(GLOB_RECURSE LUMORA_ENGINE_SHADERS
    CONFIGURE_DEPENDS
    "${LUMORA_ENGINE_SHADER_DIR}/*.glsl"
)

add_library(Engine SHARED
    ${LUMORA_ENGINE_SOURCES}
    ${LUMORA_ENGINE_LAUNCH_SOURCES}
    ${LUMORA_ENGINE_SHADERS}
)

set_source_files_properties(
    ${LUMORA_ENGINE_LAUNCH_SOURCES}
    ${LUMORA_ENGINE_SHADERS}
    PROPERTIES
        HEADER_FILE_ONLY TRUE
)

source_group(
    TREE "${LUMORA_ENGINE_SOURCE_DIR}"
    PREFIX "Source"
    FILES ${LUMORA_ENGINE_SOURCES}
)

source_group(
    TREE "${LUMORA_ENGINE_LAUNCH_DIR}"
    PREFIX "Launch"
    FILES ${LUMORA_ENGINE_LAUNCH_SOURCES}
)

source_group(
    TREE "${LUMORA_ENGINE_SHADER_DIR}"
    PREFIX "Shaders"
    FILES ${LUMORA_ENGINE_SHADERS}
)

target_include_directories(Engine
    PUBLIC
        "${LUMORA_ENGINE_SOURCE_DIR}"
        "${LUMORA_ENGINE_LAUNCH_DIR}"

    PRIVATE
        "${LUMORA_ENGINE_SOURCE_DIR}/Core"
        "${LUMORA_ENGINE_SOURCE_DIR}/Renderer"
        "${LUMORA_ENGINE_SOURCE_DIR}/Runtime"
)

target_compile_features(Engine
    PUBLIC
        c_std_17
        cxx_std_17
)

target_compile_definitions(Engine
    PRIVATE
        LUMORA_EXPORT

    INTERFACE
        LUMORA_IMPORT
)

lumora_apply_compiler_options(Engine)


target_link_libraries(Engine
    PRIVATE
        Vulkan::Vulkan
)

if(WIN32)

    target_link_libraries(Engine
        PRIVATE
            ws2_32
            user32
    )

elseif(UNIX)

    find_package(X11 REQUIRED)

    target_link_libraries(Engine
        PRIVATE
            dl
            pthread
            X11::X11
            X11::xcb
            X11::X11_xcb
    )

endif()
