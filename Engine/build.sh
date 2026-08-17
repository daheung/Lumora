#!/bin/bash
# Build script for engine
set echo on

mkdir -p ../bin

cFilenames=$(find . -type f -name "*.c")

# echo "Files:" $cFilenames

assembly="engine"
compilerFlags="-g -shared -fdeclspec -fPIC"
# -fms-extrensions
# -Wall -Werror
includeFlags="-ISource -ISource/Core -ISource/Renderer -I$VULKAN_SDK/include"
linkerFlags="-lvulkan -lxcb -lX11 -lX11-xcb -lxkbcommon -L%VULKAN_SDK/lib -L/usr/X11R6/lib"
defines="-D_DEBUG -DLUMORA_EXPORT"

echo "Building $assembly..."
clang $cFilenames $compilerFlags -o ../bin/lib$assembly.so $defines $includeFlags $linkerFlags

# !/usr/bin/env bash
# Build script for Lumora Engine on Linux

# set -e
# set -x

# assembly="engine"

# sourceDir="Source"
# buildDir="../bin/obj"
# outputDir="../bin"
# outputFile="${outputDir}/lib${assembly}.so"

# mkdir -p "${buildDir}"
# mkdir -p "${outputDir}"

# if [[ -z "${VULKAN_SDK:-}" ]]; then
#     echo "Error: VULKAN_SDK environment variable is not set."
#     exit 1
# fi

# commonCompilerFlags=(
#     -g
#     -fPIC
#     -fdeclspec
#     -Wall
#     -Wextra
# )

# cCompilerFlags=(
#     -std=c17
# )

# cppCompilerFlags=(
#     -std=c++17
# )

# includeFlags=(
#     "-I${sourceDir}"
#     "-I${sourceDir}/Core"
#     "-I${VULKAN_SDK}/include"
# )

# defines=(
#     -D_DEBUG
#     -DLUMORA_EXPORT
# )

# linkerFlags=(
#     -shared
#     "-L${VULKAN_SDK}/lib"
#     -L/usr/X11R6/lib
#     -lvulkan
#     -lxcb
#     -lX11
#     -lX11-xcb
#     -lxkbcommon
# )

# objectFiles=()

# echo "Compiling C source files..."

# while IFS= read -r -d '' sourceFile; do
#     relativePath="${sourceFile#${sourceDir}/}"
#     objectFile="${buildDir}/${relativePath}.o"

#     mkdir -p "$(dirname "${objectFile}")"

#     clang \
#         "${commonCompilerFlags[@]}" \
#         "${cCompilerFlags[@]}" \
#         "${defines[@]}" \
#         "${includeFlags[@]}" \
#         -c "${sourceFile}" \
#         -o "${objectFile}"

#     objectFiles+=("${objectFile}")
# done < <(
#     find "${sourceDir}" -type f -name "*.c" -print0
# )

# echo "Compiling C++ source files..."

# while IFS= read -r -d '' sourceFile; do
#     relativePath="${sourceFile#${sourceDir}/}"
#     objectFile="${buildDir}/${relativePath}.o"

#     mkdir -p "$(dirname "${objectFile}")"

#     clang++ \
#         "${commonCompilerFlags[@]}" \
#         "${cppCompilerFlags[@]}" \
#         "${defines[@]}" \
#         "${includeFlags[@]}" \
#         -c "${sourceFile}" \
#         -o "${objectFile}"

#     objectFiles+=("${objectFile}")
# done < <(
#     find "${sourceDir}" -type f \
#         \( -name "*.cpp" -o -name "*.cc" -o -name "*.cxx" \) \
#         -print0
# )

# if [[ ${#objectFiles[@]} -eq 0 ]]; then
#     echo "Error: No C or C++ source files were found in ${sourceDir}."
#     exit 1
# fi

# echo "Linking ${outputFile}..."

# clang++ \
#     "${objectFiles[@]}" \
#     "${linkerFlags[@]}" \
#     -o "${outputFile}"

# echo "Build succeeded: ${outputFile}"