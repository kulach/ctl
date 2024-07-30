cmake_minimum_required(VERSION 3.22.1)

project(
    ctl
)

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_FLAGS "-Wall -Wpedantic -Wuninitialized -Wextra") # -fno-omit-frame-pointer")
SET(CMAKE_CXX_FLAGS_DEBUG "-g -DBUILD_DEBUG")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3")
SET(CMAKE_LINKER_FLAGS_DEBUG)

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")


if (NOT CMAKE_BUILD_TYPE)
    message("CMAKE_BUILD_TYPE not set -- Defaulting to Release")
    SET(CMAKE_BUILD_TYPE Release)
endif()

if (NOT ADDRESS_SANITIZER)
    SET(ADDRESS_SANITIZER FALSE)
endif()

if (${ADDRESS_SANITIZER} STREQUAL "TRUE")
    message("Adding address sanitizer ${CMAKE_BUILD_TYPE}")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
    SET(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fno-omit-frame-pointer -fsanitize=address")
endif()

if (NOT PERF)
    SET(PERF FALSE)
endif()

if (${PERF} STREQUAL "TRUE") 
    message("Adding fields to allow for perf support")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fno-omit-frame-pointer")
endif()

if (NOT AVX2)
    SET(AVX2 FALSE)
endif()

if (${AVX2} STREQUAL "TRUE")
    message("Enabling AVX2 SIMD Intrinsics")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2 -mfma")
endif()

file(GLOB TEST_SRC tests/*.cpp)

add_executable(ctl src/ctl.cpp)
add_executable(ctltests ${TEST_SRC})
