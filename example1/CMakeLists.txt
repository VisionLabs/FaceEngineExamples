cmake_minimum_required(VERSION 2.8)

project(Example1)

set(SOURCES main.cpp)

source_group("Source Files" FILES ${SOURCES})

find_package(FaceEngineSDK REQUIRED)
include_directories(${FSDK_INCLUDE_DIRS})
if (WIN32)
    set(FSDK_FIND_VLF ON CACHE BOOL "Find FSDK vlf library.")
endif ()

add_executable(Example1 ${SOURCES})

target_link_libraries(Example1 ${FSDK_LIBRARIES})

install(TARGETS Example1 RUNTIME DESTINATION bin)
