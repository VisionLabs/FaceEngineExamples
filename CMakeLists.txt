cmake_minimum_required(VERSION 2.8)

project(LUNASDKExamples)

set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    "${CMAKE_SOURCE_DIR}/cmake/"
)

SET(CMAKE_INSTALL_RPATH "$ORIGIN/../lib:$ORIGIN/")

# Use solution folders in MSVS.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -DNOMINMAX)
else ()
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif ()

option(WITH_FREEIMAGE_EXAMPLE "Built example with freeimage library." OFF)
option(WITH_QT_EXAMPLE "Built example with qt library." OFF)

add_subdirectory(example1)
add_subdirectory(example2)
if (WITH_FREEIMAGE_EXAMPLE)
    add_subdirectory(example3)
endif ()
if (WITH_QT_EXAMPLE)
    add_subdirectory(example4)
endif ()
add_subdirectory(example5)
add_subdirectory(example6)
add_subdirectory(example7)
