cmake_minimum_required(VERSION 2.8)

project(Example6)

set(SOURCES main.cpp)
set(HEADERS io_util.h)

source_group("Source Files" FILES ${SOURCES})
source_group("Header Files" FILES ${HEADERS})

find_package(FaceEngineSDK REQUIRED)
include_directories(${FSDK_INCLUDE_DIRS})

add_executable(Example6 ${SOURCES} ${HEADERS})

target_link_libraries(Example6 ${FSDK_LIBRARIES})

install(TARGETS Example6 RUNTIME DESTINATION bin)
