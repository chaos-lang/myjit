get_filename_component(LIBRARY_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME)

set(Header_Files
        "amd64-codegen.h"
        "amd64-specific.h"
        "arm32-codegen.h"
        "arm32-specific.h"
        "common86-codegen.h"
        "common86-specific.h"
        "cpu-detect.h"
        "flow-analysis.h"
        "jitlib-core.h"
        "jitlib-debug.h"
        "jitlib.h"
        "llrb.h"
        "reg-allocator.h"
        "rmap.h"
        "set.h"
        "sparc-codegen.h"
        "sparc-specific.h"
        "sse2-specific.h"
        "util.h"
        "x86-codegen.h"
        "x86-specific.h"
)
source_group("Header Files" FILES "${Header_Files}")

set(Source_Files
        "code-check.c"
        "jitlib-core.c"
        "jitlib-debug.c"
        "llrb.c"
        "x86-common-stuff.c"
        )
source_group("Source Files" FILES "${Source_Files}")

add_library("${LIBRARY_NAME}" "${LIBRARY_TYPE_FLAG}" "${Header_Files}" "${Source_Files}")
target_include_directories(
        "${LIBRARY_NAME}"
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>"
        "$<INSTALL_INTERFACE:include>"
)
set_target_properties(
        "${LIBRARY_NAME}"
        PROPERTIES
        LINKER_LANGUAGE
        C
)

# install rules
include(GenerateExportHeader)
set(_export_file "${CMAKE_CURRENT_SOURCE_DIR}/${LIBRARY_NAME}_export.h")
generate_export_header("${LIBRARY_NAME}" EXPORT_FILE_NAME "${_export_file}")

# setup the version numbering
set_property(TARGET "${LIBRARY_NAME}" PROPERTY VERSION "1.0.0")
set_property(TARGET "${LIBRARY_NAME}" PROPERTY SOVERSION "1")

set(installable_libs "${LIBRARY_NAME}" "${PROJECT_NAME}_compiler_flags")
install(FILES "${Header_Files}" DESTINATION "include")

if (TARGET "${DEPENDANT_LIBRARY}")
    list(APPEND installable_libs "${DEPENDANT_LIBRARY}")
endif ()
install(TARGETS ${installable_libs}
        DESTINATION "lib"
        EXPORT "${LIBRARY_NAME}Targets")
