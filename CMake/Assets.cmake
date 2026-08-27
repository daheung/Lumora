function(lumora_create_asset_copy_target TARGET_NAME SOURCE_DIR OUTPUT_DIR)

    add_custom_target(${TARGET_NAME}
        COMMAND
            ${CMAKE_COMMAND} -E make_directory
            "${OUTPUT_DIR}"

        COMMAND
            ${CMAKE_COMMAND} -E copy_directory
            "${SOURCE_DIR}"
            "${OUTPUT_DIR}"

        COMMENT
            "Copying assets..."
    )

endfunction()
