cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0015 NEW)

project(Example8)

set(SOURCES main.cpp)

find_package(LivenessEngineSDK REQUIRED)
include_directories(${LSDK_INCLUDE_DIRS})
include_directories(${FSDK_INCLUDE_DIRS})

find_package(OpenCV REQUIRED highgui imgproc videoio)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(Example8 ${HEADERS} ${SOURCES})

target_link_libraries(Example8 ${OpenCV_LIBS} ${FSDK_LIBRARIES} ${LSDK_LIBRARIES})
	
install(TARGETS Example8 RUNTIME DESTINATION bin/${OUTPUT_PREFIX})