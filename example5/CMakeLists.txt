cmake_minimum_required(VERSION 2.8)

project(Example5)

set(SOURCES main.cpp)

source_group("Source Files" FILES ${SOURCES})

find_package(FaceEngineSDK REQUIRED)
include_directories(${FSDK_INCLUDE_DIRS})
if (WIN32)
    set(FSDK_FIND_VLF ON CACHE BOOL "Find FSDK vlf library.")
endif ()

###### QT
# Set CMake paths for Qt modules
set(QT5_CMAKES CACHE PATH "QT5 Cmake files (qtbase/lib/cmake)")

set(CMAKE_PREFIX_PATH ${QT5_CMAKES})

if(CMAKE_PREFIX_PATH STREQUAL "")
    message(FATAL_ERROR "Must provide qt5 cmake files directory. It's qtbase/lib/cmake in wherever you have qt5")
endif()

# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Gui REQUIRED)

add_executable(Example5 ${SOURCES})

target_link_libraries(Example5 ${FSDK_LIBRARIES})

qt5_use_modules(Example5 Gui)

install(TARGETS Example5 RUNTIME DESTINATION bin)
