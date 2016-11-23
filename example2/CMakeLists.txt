cmake_minimum_required(VERSION 2.8)

project(Example2_Attributes)

set(SOURCES
  main.cc
)

set(HEADERS
)

source_group("Header Files" FILES ${HEADERS})
source_group("Source Files" FILES ${SOURCES})

add_executable(Example2_Attributes ${SOURCES} ${HEADERS})

find_package(FaceEngineSDK REQUIRED)
include_directories(${FSDK_INCLUDE_DIRS})
target_link_libraries(Example2_Attributes ${FSDK_LIBRARIES})

install(TARGETS Example2_Attributes
        RUNTIME DESTINATION bin)
