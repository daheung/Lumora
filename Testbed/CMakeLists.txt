# ===========================================================
# TestBed
# ============================================================

set(LUMORA_TESTBED_SOURCE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/Source"
)

file(GLOB_RECURSE LUMORA_TESTBED_SOURCES
    CONFIGURE_DEPENDS
    "${LUMORA_TESTBED_SOURCE_DIR}/*.c"
    "${LUMORA_TESTBED_SOURCE_DIR}/*.cpp"
    "${LUMORA_TESTBED_SOURCE_DIR}/*.h"
    "${LUMORA_TESTBED_SOURCE_DIR}/*.hpp"
)

set(LUMORA_ENGINE_ENTRY_POINT
    "${CMAKE_SOURCE_DIR}/Engine/Launch/EntryPoint.c"
)

add_executable(TestBed
    ${LUMORA_TESTBED_SOURCES}
    "${LUMORA_ENGINE_ENTRY_POINT}"
)

source_group(
    TREE "${LUMORA_TESTBED_SOURCE_DIR}"
    PREFIX "Source"
    FILES ${LUMORA_TESTBED_SOURCES}
)

target_link_libraries(TestBed
    PRIVATE
        Engine
)