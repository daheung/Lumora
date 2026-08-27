function(lumora_apply_compiler_options TARGET_NAME)

    if(MSVC)

        message(STATUS "Lumora compiler: MSVC")

        target_compile_definitions(${TARGET_NAME}
            PRIVATE
                _CRT_SECURE_NO_WARNINGS
        )

        target_compile_options(${TARGET_NAME}
            PRIVATE
                /W4
                $<$<CONFIG:Release>:/Zi>
                $<$<CONFIG:Release>:/Zo>
        )

        target_link_options(${TARGET_NAME}
            PRIVATE
                $<$<CONFIG:Release>:/DEBUG>
                $<$<CONFIG:Release>:/INCREMENTAL:NO>
        )

    elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")

        message(STATUS "Lumora compiler: Clang")

        target_compile_options(${TARGET_NAME}
            PRIVATE
                -Wall
                -Wextra
                -Wvarargs
                $<$<CONFIG:Release>:-g>
                $<$<CONFIG:Release>:-fno-omit-frame-pointer>
        )

    elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")

        message(STATUS "Lumora compiler: GCC")

        target_compile_options(${TARGET_NAME}
            PRIVATE
                -Wall
                -Wextra
        )

    else()

        message(WARNING
            "Unsupported compiler: ${CMAKE_C_COMPILER_ID}"
        )

    endif()

endfunction()
