cmake_minimum_required(VERSION 2.8)

project(Example8)

set(SOURCES main.cpp)
set(HEADERS io_util.h)

source_group("Source Files" FILES ${SOURCES})
source_group("Header Files" FILES ${HEADERS})

find_package(FaceEngineSDK REQUIRED)
include_directories(${FSDK_INCLUDE_DIRS})

add_executable(Example8 ${SOURCES} ${HEADERS})

target_link_libraries(Example8 ${FSDK_LIBRARIES})

install(TARGETS Example8 RUNTIME DESTINATION bin)
