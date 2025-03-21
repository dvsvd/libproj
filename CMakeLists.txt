cmake_minimum_required (VERSION 3.8)

project ("libproj" LANGUAGES C)

set(CMAKE_DEBUG_POSTFIX d)
set(CMAKE_C_STANDARD_REQUIRED 17)
set(CMAKE_C_STANDARD 17)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -shared -fPIC -MMD -MP -o ${CMAKE_PROJECT_NAME}.so")
message(${CMAKE_C_FLAGS})

add_subdirectory ("libproj")
