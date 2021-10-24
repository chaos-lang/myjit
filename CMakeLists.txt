get_filename_component(_EXEC_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
add_subdirectory("myjit")

foreach(i RANGE 1 3)
    set(EXEC_NAME "${_EXEC_NAME}_demo_${i}")

    set(Source_Files "demo${i}.c")
    source_group("${EXEC_NAME} Source Files" FILES "${Source_Files}")

    add_executable("${EXEC_NAME}" "${Source_Files}")
    set_target_properties(
            "${EXEC_NAME}"
            PROPERTIES
            LINKER_LANGUAGE
            C
    )
    target_link_libraries("${EXEC_NAME}" PRIVATE "myjit")

    # install rules

    set(installable_libs "${EXEC_NAME}" "${PROJECT_NAME}_compiler_flags")

    if (TARGET "${DEPENDANT_LIBRARY}")
        list(APPEND installable_libs "${DEPENDANT_LIBRARY}")
    endif ()
    install(TARGETS ${installable_libs}
            DESTINATION "bin"
            EXPORT "${EXEC_NAME}Targets")
endforeach ()
