# set echo on

# mkdir -p ../bin

# cFilenames=$(find . -type f -name "*.c")

# assembly="testbed"
# compilerFlags="-g -fdeclspec -fPIC"
# # -fms-extensions
# # -Wall -Werror
# includeFlags="-Isrc -I../Engine/Source/"
# linkerFlags="-L../bin/ -lengine -Wl,-rpath,."
# defines="-D_DEBUG -DLUMORA_IMPORT"

# echo "Building $assembly..."
# echo clang $cFilenames $compilerFlags -o ../bin/$assembly $defines $includeFlags $linkerFlags
# clang $cFilenames $compilerFlags -o ../bin/$assembly $defines $includeFlags $linkerFlags

#!/usr/bin/env bash
# Build script for Lumora Testbed on Linux

set -e
set -x

assembly="testbed"

sourceDir="Source"
buildDir="../bin/obj/${assembly}"
outputDir="../bin"
outputFile="${outputDir}/${assembly}"

mkdir -p "${buildDir}"
mkdir -p "${outputDir}"

commonCompilerFlags=(
    -g
    -fdeclspec
    -fPIC
)

cCompilerFlags=(
    -std=c17
)

cppCompilerFlags=(
    -std=c++17
)

includeFlags=(
    "-I${sourceDir}"
    "-I../Engine/Source"
)

defines=(
    -D_DEBUG
    -DLUMORA_IMPORT
)

linkerFlags=(
    -L../bin
    -lengine

    # 실행 파일이 위치한 디렉터리에서 libengine.so를 검색한다.
    '-Wl,-rpath,$ORIGIN'
)

objectFiles=()

echo "Compiling C source files..."

while IFS= read -r -d '' sourceFile; do
    relativePath="${sourceFile#${sourceDir}/}"
    objectFile="${buildDir}/${relativePath}.o"

    mkdir -p "$(dirname "${objectFile}")"

    clang \
        "${commonCompilerFlags[@]}" \
        "${cCompilerFlags[@]}" \
        "${defines[@]}" \
        "${includeFlags[@]}" \
        -c "${sourceFile}" \
        -o "${objectFile}"

    objectFiles+=("${objectFile}")
done < <(
    find "${sourceDir}" -type f -name "*.c" -print0
)

echo "Compiling C++ source files..."

while IFS= read -r -d '' sourceFile; do
    relativePath="${sourceFile#${sourceDir}/}"
    objectFile="${buildDir}/${relativePath}.o"

    mkdir -p "$(dirname "${objectFile}")"

    clang++ \
        "${commonCompilerFlags[@]}" \
        "${cppCompilerFlags[@]}" \
        "${defines[@]}" \
        "${includeFlags[@]}" \
        -c "${sourceFile}" \
        -o "${objectFile}"

    objectFiles+=("${objectFile}")
done < <(
    find "${sourceDir}" -type f \
        \( -name "*.cpp" -o -name "*.cc" -o -name "*.cxx" \) \
        -print0
)

if [[ ${#objectFiles[@]} -eq 0 ]]; then
    echo "Error: No C or C++ source files were found in ${sourceDir}."
    exit 1
fi

echo "Linking ${outputFile}..."

clang++ \
    "${objectFiles[@]}" \
    "${linkerFlags[@]}" \
    -o "${outputFile}"

echo "Build succeeded: ${outputFile}"