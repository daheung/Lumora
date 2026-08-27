# ============================================================
# UnitTest
# ============================================================

set(LUMORA_UNITTEST_SOURCE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/Source"
)

file(GLOB_RECURSE LUMORA_UNITTEST_SOURCES
    CONFIGURE_DEPENDS
    "${LUMORA_UNITTEST_SOURCE_DIR}/*.c"
    "${LUMORA_UNITTEST_SOURCE_DIR}/*.cpp"
    "${LUMORA_UNITTEST_SOURCE_DIR}/*.h"
    "${LUMORA_UNITTEST_SOURCE_DIR}/*.hpp"
)

add_executable(UnitTest
    ${LUMORA_UNITTEST_SOURCES}
)

source_group(
    TREE "${LUMORA_UNITTEST_SOURCE_DIR}"
    PREFIX "Source"
    FILES ${LUMORA_UNITTEST_SOURCES}
)

target_include_directories(UnitTest
    PRIVATE
        "${LUMORA_UNITTEST_SOURCE_DIR}"
)

target_compile_options(UnitTest
    PRIVATE
        $<$<COMPILE_LANG_AND_ID:C,Clang>:-Wno-missing-braces>
        $<$<COMPILE_LANG_AND_ID:CXX,Clang>:-Wno-missing-braces>
)

target_link_libraries(UnitTest
    PRIVATE
        Engine
)

set_target_properties(UnitTest
    PROPERTIES
        FOLDER "Tests"
)
