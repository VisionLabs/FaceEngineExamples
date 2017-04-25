cmake_minimum_required(VERSION 2.8)

project(Examples)

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
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif ()

option(WITH_FREEIMAGE_EXAMPLE "Built example with freeimage library." ON)
option(WITH_QT_EXAMPLE "Built example with qt library." OFF)

if (WIN32)
    set(FSDK_FIND_VLF ON CACHE BOOL "Find FSDK vlf library.")
endif ()

add_subdirectory(example1)
add_subdirectory(example2)
add_subdirectory(example3)
if (WITH_FREEIMAGE_EXAMPLE)
    add_subdirectory(example4)
endif ()
if (WITH_QT_EXAMPLE)
    add_subdirectory(example5)
endif ()
