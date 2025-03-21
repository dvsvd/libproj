cmake_minimum_required (VERSION 3.8)

add_executable (libproj "libproj.c")
set_target_properties(libproj PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
