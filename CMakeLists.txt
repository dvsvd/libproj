cmake_minimum_required(VERSION 3.8)

project ("libproj" LANGUAGES C)

set(CMAKE_DEBUG_POSTFIX d)

include_directories(".")

add_subdirectory ("libproj")
add_subdirectory ("testproj")
add_subdirectory("daemon")
