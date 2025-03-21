cmake_minimum_required (VERSION 3.8)

add_library(libproj SHARED "libproj.c")
set_target_properties(libproj PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
