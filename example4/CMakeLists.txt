cmake_minimum_required(VERSION 2.8)

project(Example4)

set(SOURCES main.cpp)

source_group("Source Files" FILES ${SOURCES})

find_package(FaceEngineSDK REQUIRED)
include_directories(${FSDK_INCLUDE_DIRS})

include_directories(${FREEIMAGE_INCLUDE_DIRS})

add_executable(Example4 ${SOURCES})

set(FREEIMAGE_USE_STATIC_LIB FALSE)

if(FREEIMAGE_USE_STATIC_LIB)
	add_definitions(-DFREEIMAGE_STATIC_LIB)
endif()

target_link_libraries(Example4 ${FSDK_LIBRARIES} ${FREEIMAGE_LIBRARIES})

install(TARGETS Example4 RUNTIME DESTINATION bin)
