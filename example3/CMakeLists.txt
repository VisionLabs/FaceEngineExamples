cmake_minimum_required(VERSION 2.8)

project(Example3)

set(SOURCES main.cpp)

source_group("Source Files" FILES ${SOURCES})

find_package(FaceEngineSDK REQUIRED)
include_directories(${FSDK_INCLUDE_DIRS})

add_executable(Example3 ${SOURCES})

target_link_libraries(Example3 ${FSDK_LIBRARIES})

install(TARGETS Example3 RUNTIME DESTINATION bin)
