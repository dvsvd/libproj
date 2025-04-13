set(LIBRARY_NAME libproj)

add_library(${LIBRARY_NAME} SHARED "")
set_target_properties(${LIBRARY_NAME} PROPERTIES
    DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
    OUTPUT_NAME ${LIBRARY_NAME}
    C_STANDARD 17
    C_STANDARD_REQUIRED ON
)

target_compile_options(${LIBRARY_NAME} PRIVATE "-pthread" "-m64")

target_sources(${LIBRARY_NAME} PRIVATE libproj.c iofcn.h iofcn.c memfcn.h memfcn.c
shared_buf.h shared_buf.c utility.h buf_queue.h buf_queue.c logger.h logger.c)
