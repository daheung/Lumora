function(lumora_compile_shader
    OUTPUT_VARIABLE
    SHADER_SOURCE_DIR
    SHADER_OUTPUT_DIR
    SHADER_NAME
    SHADER_STAGE
)

    set(SHADER_SOURCE
        "${SHADER_SOURCE_DIR}/${SHADER_NAME}"
    )

    if(NOT EXISTS "${SHADER_SOURCE}")
        message(STATUS "Skipping missing shader: ${SHADER_NAME}")

        set(
            ${OUTPUT_VARIABLE}
            ""
            PARENT_SCOPE
        )

        return()
    endif()

    string(
        REGEX REPLACE "\\.glsl$" ".spv"
        SHADER_OUTPUT_NAME
        "${SHADER_NAME}"
    )

    set(SHADER_OUTPUT
        "${SHADER_OUTPUT_DIR}/${SHADER_OUTPUT_NAME}"
    )

    add_custom_command(
        OUTPUT
            "${SHADER_OUTPUT}"

        COMMAND
            ${CMAKE_COMMAND} -E make_directory
            "${SHADER_OUTPUT_DIR}"

        COMMAND
            Vulkan::glslc
            -fshader-stage=${SHADER_STAGE}
            "${SHADER_SOURCE}"
            -o "${SHADER_OUTPUT}"

        DEPENDS
            "${SHADER_SOURCE}"

        COMMENT
            "Compiling shader: ${SHADER_NAME}"

        VERBATIM
    )

    set(
        ${OUTPUT_VARIABLE}
        "${SHADER_OUTPUT}"
        PARENT_SCOPE
    )

endfunction()
