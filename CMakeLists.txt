cmake_minimum_required(VERSION 3.25)

project(Lumora
    VERSION 0.1.0
    LANGUAGES C CXX
)


# ============================================================
# Project Settings
# ============================================================

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT TestBed)

set(LUMORA_OUTPUT_DIR
    "${CMAKE_SOURCE_DIR}/Outputs/$<CONFIG>"
)

set(LUMORA_BINARY_OUTPUT_DIR
    "${LUMORA_OUTPUT_DIR}/bin"
)

# .exe, Windows .dll
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
    "${LUMORA_OUTPUT_DIR}"
)

# Linux .so, macOS .dylib
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
    "${LUMORA_OUTPUT_DIR}"
)

# .lib, .a, Windows DLL import library
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY
    "${LUMORA_BINARY_OUTPUT_DIR}"
)

# Linker-generated PDB (.exe/.dll PDB)
set(CMAKE_PDB_OUTPUT_DIRECTORY
    "${LUMORA_BINARY_OUTPUT_DIR}"
)

# Compiler-generated PDB
set(CMAKE_COMPILE_PDB_OUTPUT_DIRECTORY
    "${LUMORA_BINARY_OUTPUT_DIR}"
)


# ============================================================
# CMake Modules
# ============================================================

list(APPEND CMAKE_MODULE_PATH
    "${CMAKE_SOURCE_DIR}/CMake"
)

include(CompilerOptions)
include(Assets)
include(Shaders)


# ============================================================
# Dependencies
# ============================================================

find_package(Vulkan REQUIRED COMPONENTS glslc)


# ============================================================
# Assets
# ============================================================

set(LUMORA_ASSET_SOURCE_DIR
    "${CMAKE_SOURCE_DIR}/Assets"
)

set(LUMORA_ASSET_OUTPUT_DIR
    "${CMAKE_SOURCE_DIR}/Outputs/$<CONFIG>/Assets"
)

set(LUMORA_SHADER_SOURCE_DIR
    "${LUMORA_ASSET_SOURCE_DIR}/Shaders"
)

set(LUMORA_SHADER_OUTPUT_DIR
    "${LUMORA_ASSET_OUTPUT_DIR}/Shaders"
)

lumora_create_asset_copy_target(
    CopyAssets
    "${LUMORA_ASSET_SOURCE_DIR}"
    "${LUMORA_ASSET_OUTPUT_DIR}"
)


# ============================================================
# Shader Compilation
# ============================================================

set(LUMORA_SHADER_OUTPUTS)

lumora_compile_shader(
    LUMORA_OBJECT_VERT_SPV
    "${LUMORA_SHADER_SOURCE_DIR}"
    "${LUMORA_SHADER_OUTPUT_DIR}"
    "Builtin.ObjectShader.vert.glsl"
    vert
)

if(LUMORA_OBJECT_VERT_SPV)
    list(APPEND LUMORA_SHADER_OUTPUTS
        "${LUMORA_OBJECT_VERT_SPV}"
    )
endif()

lumora_compile_shader(
    LUMORA_OBJECT_FRAG_SPV
    "${LUMORA_SHADER_SOURCE_DIR}"
    "${LUMORA_SHADER_OUTPUT_DIR}"
    "Builtin.ObjectShader.frag.glsl"
    frag
)

if(LUMORA_OBJECT_FRAG_SPV)
    list(APPEND LUMORA_SHADER_OUTPUTS
        "${LUMORA_OBJECT_FRAG_SPV}"
    )
endif()

add_custom_target(CompileShaders
    ALL
    DEPENDS
        ${LUMORA_SHADER_OUTPUTS}
)

add_dependencies(CompileShaders CopyAssets)

set_target_properties(
    CopyAssets
    CompileShaders
    PROPERTIES
        FOLDER "CMake"
)


# ============================================================
# Targets
# ============================================================

add_subdirectory(Engine)
add_subdirectory(TestBed)
add_subdirectory(UnitTest)

add_dependencies(TestBed CompileShaders)
