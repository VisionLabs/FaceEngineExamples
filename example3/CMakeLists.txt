cmake_minimum_required(VERSION 2.8)

project(Example3_Attributes)

set(SOURCES
  main.cc
)

set(HEADERS
)

source_group("Header Files" FILES ${HEADERS})
source_group("Source Files" FILES ${SOURCES})

add_executable(Example3_Attributes ${SOURCES} ${HEADERS})

find_package(FaceEngineSDK REQUIRED)
include_directories(${FSDK_INCLUDE_DIRS})
target_link_libraries(Example3_Attributes ${FSDK_LIBRARIES})

install(TARGETS Example3_Attributes
        RUNTIME DESTINATION bin)
