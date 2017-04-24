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

add_subdirectory(example1)
add_subdirectory(example2)
add_subdirectory(example3)
if (WITH_FREEIMAGE_EXAMPLE)
    set(FREEIMAGE_DOWNLOAD_PATH "${CMAKE_BINARY_DIR}/freeimage/")
    if (NOT EXISTS "${FREEIMAGE_DOWNLOAD_PATH}")
        if (WIN32)
            set(FREEIMAGE_INCLUDE_URL "http://download.visionlabs.ru/freeimage/windows/include/FreeImage.h")
            set(FREEIMAGE_LIBRARY_URL "http://download.visionlabs.ru/freeimage/windows/lib/FreeImage.lib")
            set(FREEIMAGE_BINARY_URL "http://download.visionlabs.ru/freeimage/windows/bin/FreeImage.dll")
            
            file(DOWNLOAD "${FREEIMAGE_INCLUDE_URL}" "${FREEIMAGE_DOWNLOAD_PATH}/include/FreeImage.lib")
            file(DOWNLOAD "${FREEIMAGE_LIBRARY_URL}" "${FREEIMAGE_DOWNLOAD_PATH}/lib/FreeImage.lib")
            file(DOWNLOAD "${FREEIMAGE_BINARY_URL}" "${FREEIMAGE_DOWNLOAD_PATH}/bin/FreeImage.dll")
        else ()
            set(FREEIMAGE_INCLUDE_URL "http://download.visionlabs.ru/freeimage/centos/include/FreeImage.h")
            set(FREEIMAGE_LIBRARY_URL "http://download.visionlabs.ru/freeimage/centos/lib/libfreeimage.so.3")
            
            file(DOWNLOAD "${FREEIMAGE_INCLUDE_URL}" "${FREEIMAGE_DOWNLOAD_PATH}/include/FreeImage.h")
            file(DOWNLOAD "${FREEIMAGE_LIBRARY_URL}" "${FREEIMAGE_DOWNLOAD_PATH}/lib/libfreeimage.so.3")
        endif ()

        set(FREEIMAGE_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/freeimage/include/" CACHE PATH "FreeImage include dir.")
        if (WIN32)
            set(FREEIMAGE_LIBRARIES "${CMAKE_BINARY_DIR}/freeimage/lib/FreeImage.lib" CACHE FILEPATH "FreeImage lib file path.")
        else ()
            set(FREEIMAGE_LIBRARIES "${CMAKE_BINARY_DIR}/freeimage/lib/libfreeimage.so.3" CACHE FILEPATH "FreeImage lib file path.")
        endif ()
    endif ()
    add_subdirectory(example4)
endif ()
if (WITH_QT_EXAMPLE)
    add_subdirectory(example5)
endif ()

if (WITH_FREEIMAGE_EXAMPLE)
    install(DIRECTORY "${CMAKE_BINARY_DIR}/freeimage/include/" DESTINATION include)
    install(DIRECTORY "${CMAKE_BINARY_DIR}/freeimage/lib/" DESTINATION lib)
    if (WIN32)
        install(DIRECTORY "${CMAKE_BINARY_DIR}/freeimage/bin/" DESTINATION bin)
    endif ()
endif ()
