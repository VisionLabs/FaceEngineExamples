cmake_minimum_required(VERSION 2.8)

project(Example1)

set(SOURCES
  main.cc
)

set(HEADERS
)

source_group("Header Files" FILES ${HEADERS})
source_group("Source Files" FILES ${SOURCES})

add_executable(Example1 ${SOURCES} ${HEADERS})

find_package(FaceEngineSDK REQUIRED)
include_directories(${FSDK_INCLUDE_DIRS})
target_link_libraries(Example1 ${FSDK_LIBRARIES})

install(TARGETS Example1
	RUNTIME DESTINATION bin)
