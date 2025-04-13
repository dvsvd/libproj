set(APP_NAME testproj)

add_executable(${APP_NAME} "")
set_target_properties(${APP_NAME} PROPERTIES
    DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
    OUTPUT_NAME ${APP_NAME}
    C_STANDARD 17
    C_STANDARD_REQUIRED ON
)

target_compile_options(${APP_NAME} PRIVATE "-pthread" "-m64")

target_sources(${APP_NAME} PRIVATE testproj.c)
