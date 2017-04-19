cmake_minimum_required(VERSION 2.8)

project(Example3)

set(SOURCES
  main.cc
)

set(HEADERS
)

source_group("Header Files" FILES ${HEADERS})
source_group("Source Files" FILES ${SOURCES})

add_executable(Example3 ${SOURCES} ${HEADERS})

find_package(Qt5 COMPONENTS Gui REQUIRED)

find_package(FaceEngineSDK REQUIRED)
include_directories(${FSDK_INCLUDE_DIRS})

target_link_libraries(Example3 ${FSDK_LIBRARIES})

qt5_use_modules(Example3 Gui)

install(TARGETS Example3
        RUNTIME DESTINATION bin)
