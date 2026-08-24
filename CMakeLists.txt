cmake_minimum_required(VERSION 3.25)

project(Lumora
    VERSION 0.1.0
    LANGUAGES C CXX
)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Testbed)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_SOURCE_DIR}/bin/Debug")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin/Release")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_SOURCE_DIR}/bin/Debug")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin/Release")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_SOURCE_DIR}/lib/Debug")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/lib/Release")

# ============================================================
# Engine
# ============================================================

file(GLOB_RECURSE LUMORA_ENGINE_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Engine/Source/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/Engine/Source/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Engine/Source/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Engine/Source/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Engine/Source/*.inl"
)

add_library(Engine SHARED
    ${LUMORA_ENGINE_SOURCES}
)

source_group(
    TREE "${CMAKE_CURRENT_SOURCE_DIR}/Engine/Source"
    PREFIX "Source"
    FILES ${LUMORA_ENGINE_SOURCES}
)

target_include_directories(Engine
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/Engine/Source"

    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/Engine/Source/Core"
        "${CMAKE_CURRENT_SOURCE_DIR}/Engine/Source/Renderer"
)

target_compile_definitions(Engine
    PRIVATE
        LUMORA_EXPORT
        _CRT_SECURE_NO_WARNINGS
)

target_compile_features(Engine
    PUBLIC
        c_std_17
        cxx_std_17
)

# ============================================================
# Compiler Options
# ============================================================

if(MSVC)

    message(STATUS "Lumora compiler: MSVC")

    target_compile_options(Engine PRIVATE /W4)
    target_compile_options(Engine PRIVATE
        $<$<CONFIG:Release>:/Zi>
        $<$<CONFIG:Release>:/Zo>
    )
    target_link_options(Engine PRIVATE
        $<$<CONFIG:Release>:/DEBUG>
        $<$<CONFIG:Release>:/INCREMENTAL:NO>
    )

elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")

    message(STATUS "Lumora compiler: Clang")

    target_compile_options(Engine PRIVATE
        -Wall
        -Wextra
        -Wvarargs
        $<$<CONFIG:Release>:-g>
        $<$<CONFIG:Release>:-fno-omit-frame-pointer>
    )

elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")

    message(STATUS "Lumora compiler: GCC")

    target_compile_options(Engine
        PRIVATE
            -Wall
            -Wextra
            -Werror
    )

else()

    message(WARNING
        "Unsupported compiler: ${CMAKE_C_COMPILER_ID}"
    )

endif()


# ============================================================
# Vulkan
# ============================================================

find_package(Vulkan REQUIRED)

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
    target_link_libraries(Engine PRIVATE
            dl
            pthread
    )
    target_link_libraries(Engine PRIVATE
            X11::X11
            X11::xcb
            X11::X11_xcb
    )

endif()


# ============================================================
# Testbed
# ============================================================

file(GLOB_RECURSE LUMORA_TESTBED_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Testbed/Source/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/Testbed/Source/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Testbed/Source/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Testbed/Source/*.hpp"
)

add_executable(Testbed
    ${LUMORA_TESTBED_SOURCES}
)

source_group(
    TREE "${CMAKE_CURRENT_SOURCE_DIR}/Testbed/Source"
    PREFIX "Source"
    FILES ${LUMORA_TESTBED_SOURCES}
)

target_link_libraries(Testbed
    PRIVATE
        Engine
)