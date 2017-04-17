cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH
	 ${CMAKE_MODULE_PATH}
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

project(Examples)

SET(CMAKE_INSTALL_RPATH "$ORIGIN/../lib:$ORIGIN/")

# Use solution folders in MSVS.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS -DNOMINMAX)
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

add_subdirectory(example1)
add_subdirectory(example2)
