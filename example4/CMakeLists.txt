cmake_minimum_required(VERSION 2.8)

project(Example4)

set(SOURCES main.cpp)

source_group("Source Files" FILES ${SOURCES})

find_package(FaceEngineSDK REQUIRED)
include_directories(${FSDK_INCLUDE_DIRS})

message("-- CMAKE_SYSTEM_INFO_FILE: ${CMAKE_SYSTEM_INFO_FILE}")
message("-- CMAKE_SYSTEM_NAME:      ${CMAKE_SYSTEM_NAME}")
message("-- CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")
message("-- CMAKE_SYSTEM:           ${CMAKE_SYSTEM}")

set(FREEIMAGE_DOWNLOAD_PATH "${CMAKE_BINARY_DIR}/freeimage/")
if (NOT EXISTS "${FREEIMAGE_DOWNLOAD_PATH}")
    if (WIN32)
        set(FREEIMAGE_INCLUDE_URL "http://download.visionlabs.ru/freeimage/windows/include/FreeImage.h")
        set(FREEIMAGE_LIBRARY_URL "http://download.visionlabs.ru/freeimage/windows/lib/FreeImage.lib")
        set(FREEIMAGE_BINARY_URL "http://download.visionlabs.ru/freeimage/windows/bin/FreeImage.dll")
        
        file(DOWNLOAD "${FREEIMAGE_INCLUDE_URL}" "${FREEIMAGE_DOWNLOAD_PATH}/include/FreeImage.h")
        file(DOWNLOAD "${FREEIMAGE_LIBRARY_URL}" "${FREEIMAGE_DOWNLOAD_PATH}/lib/FreeImage.lib")
        file(DOWNLOAD "${FREEIMAGE_BINARY_URL}" "${FREEIMAGE_DOWNLOAD_PATH}/bin/FreeImage.dll")
    else ()
        execute_process(COMMAND bash "-c" "lsb_release -i | sed 's/^.*:\t//'" OUTPUT_VARIABLE DISTRIBUTOR)
        if (${DISTRIBUTOR} STREQUAL "CentOS\n")
            set(FREEIMAGE_INCLUDE_URL "http://download.visionlabs.ru/freeimage/centos/include/FreeImage.h")
            set(FREEIMAGE_LIBRARY_URL "http://download.visionlabs.ru/freeimage/centos/lib/libfreeimage.so.3")
            
            file(DOWNLOAD "${FREEIMAGE_INCLUDE_URL}" "${FREEIMAGE_DOWNLOAD_PATH}/include/FreeImage.h")
            file(DOWNLOAD "${FREEIMAGE_LIBRARY_URL}" "${FREEIMAGE_DOWNLOAD_PATH}/lib/libfreeimage.so.3")
        elseif (${DISTRIBUTOR} STREQUAL "Ubuntu\n")
            set(FREEIMAGE_INCLUDE_URL "http://download.visionlabs.ru/freeimage/ubuntu/include/FreeImage.h")
            set(FREEIMAGE_LIBRARY_URL "http://download.visionlabs.ru/freeimage/ubuntu/lib/libfreeimage.so.3")
            
            file(DOWNLOAD "${FREEIMAGE_INCLUDE_URL}" "${FREEIMAGE_DOWNLOAD_PATH}/include/FreeImage.h")
            file(DOWNLOAD "${FREEIMAGE_LIBRARY_URL}" "${FREEIMAGE_DOWNLOAD_PATH}/lib/libfreeimage.so.3")
        endif ()
    endif ()

    set(FREEIMAGE_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/freeimage/include/" CACHE PATH "FreeImage include dir.")
    if (WIN32)
        set(FREEIMAGE_LIBRARIES "${CMAKE_BINARY_DIR}/freeimage/lib/FreeImage.lib" CACHE FILEPATH "FreeImage lib file path.")
    else ()
        set(FREEIMAGE_LIBRARIES "${CMAKE_BINARY_DIR}/freeimage/lib/libfreeimage.so.3" CACHE FILEPATH "FreeImage lib file path.")
    endif ()
endif ()

include_directories(${FREEIMAGE_INCLUDE_DIRS})

set(FREEIMAGE_USE_STATIC_LIB FALSE)
if (FREEIMAGE_USE_STATIC_LIB)
	add_definitions(-DFREEIMAGE_STATIC_LIB)
endif ()

add_executable(Example4 ${SOURCES})

target_link_libraries(Example4 ${FSDK_LIBRARIES} ${FREEIMAGE_LIBRARIES})

install(TARGETS Example4 RUNTIME DESTINATION bin)

install(DIRECTORY "${CMAKE_BINARY_DIR}/freeimage/include/" DESTINATION include)
install(DIRECTORY "${CMAKE_BINARY_DIR}/freeimage/lib/" DESTINATION lib)
if (WIN32)
    install(DIRECTORY "${CMAKE_BINARY_DIR}/freeimage/bin/" DESTINATION bin)
endif ()
