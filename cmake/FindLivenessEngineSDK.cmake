# Find the Vision Labs Liveness SDK & Face SDK.
# Sets the usual variables expected for find_package scripts:
# LSDK_INCLUDE_DIRS - headers location
# LSDK_LIBRARIES - libraries to link against
# LSDK_FOUND - true if Liveness SDK was found.

# This is the directory where the Liveness SDK is located.
# By default LSDKDIR environment variable value is taken.

find_package(FaceEngineSDK REQUIRED)

set(LSDK_ROOT "$ENV{LSDKDIR}" CACHE PATH "Vision Labs Liveness SDK root directory.")

# Look for headers.
find_path(LSDK_INCLUDE_DIRS
          NAMES lsdk/LivenessEngine.h lsdk/ILiveness.h
          HINTS $ENV{LSDKDIR}
          PATHS ${LSDK_ROOT}
          PATH_SUFFIXES include)

# iOS framework have different header structure: not flower/header.h, but Headers/header.h. But when you link against framework it's name is used as prefix for includes, so you still use its includes as flower/header.h in c++ code.
# Now the reason to set this variable is that its used for every other platform and its easier to fake it than rewrite most cmakelists 
if(IOS)
    set(LSDK_INCLUDE_DIRS ${LSDK_ROOT}/include)
endif()

#message(STATUS "LSDK [DEBUG]: LSDK_INCLUDE_DIRS = ${LSDK_INCLUDE_DIRS}.")

# Determine compiler version and architecture.
# ios has no architechture/compiler branching, because only can only use clang
# and if you need multiple architechtures there still compiled into single universal binary
if(NOT IOS)
	if(MSVC10)
		set(LSDK_COMPILER_NAME vs2010)
	elseif(MSVC11)
		set(LSDK_COMPILER_NAME vs2012)
	elseif(MSVC12)
		set(LSDK_COMPILER_NAME vs2013)
	elseif(MSVC14)
		set(LSDK_COMPILER_NAME vs2015)
	elseif(CMAKE_COMPILER_IS_GNUCXX)
		set(LSDK_COMPILER_NAME gcc4)
	elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		set(LSDK_COMPILER_NAME clang)
	else()
		message(SEND_ERROR "Unsupported compiler: ${LSDK_COMPILER_NAME}")
	endif()

	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(LSDK_TARGET_NAME x64)
	else()
		set(LSDK_TARGET_NAME x86)
	endif()

	# What version of Face SDK to use.
	set(LSDK_LIB_PREFIX ${LSDK_COMPILER_NAME}/${LSDK_TARGET_NAME})
endif()

if(IOS)
	# List of all SDK libraries.
	set(LSDK_LIB_NAMES
		LivenessEngine)
else()
	# List of all SDK libraries.
	set(LSDK_LIB_NAMES
		LivenessEngineSDK)
endif()

if(IOS)
    set(LSDK_LIB_PATH_SUFFIX Frameworks)
    set(LSDK_BIN_PATH_SUFFIX Frameworks)
else()
    set(LSDK_LIB_PATH_SUFFIX lib/${LSDK_LIB_PREFIX})
    set(LSDK_BIN_PATH_SUFFIX bin/${LSDK_LIB_PREFIX})
endif()
	
# Find optimized libraries.
set(LSDK_LIB)
foreach(LIB ${LSDK_LIB_NAMES})
	set(LIB_PATH ${LIB}-NOTFOUND)
	find_library(LIB_PATH
				 NAMES ${LIB}
				 HINTS $ENV{LSDKDIR}
				 PATHS ${LSDK_ROOT}
				 PATH_SUFFIXES	${LSDK_LIB_PATH_SUFFIX}
								${LSDK_BIN_PATH_SUFFIX})

	list(APPEND LSDK_LIB ${LIB_PATH})
endforeach()

# Find debug libraries (may be missing in your distribution).
set(LSDK_LIBD)
foreach(LIB ${LSDK_LIB_NAMES})
	set(LIB_PATH ${LIB}-NOTFOUND)
	find_library(LIB_PATH
				 NAMES ${LIB}d
				 HINTS $ENV{LSDKDIR}
				 PATHS ${LSDK_ROOT}
				 PATH_SUFFIXES	${LSDK_LIB_PATH_SUFFIX}
								${LSDK_BIN_PATH_SUFFIX})

	list(APPEND LSDK_LIBD ${LIB_PATH})
endforeach()

# Support the REQUIRED and QUIET arguments, and set LSDK_FOUND if found.
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LSDK DEFAULT_MSG 
                                  LSDK_LIB
                                  LSDK_INCLUDE_DIRS)

set(LSDK_LIBRARIES)

if(LSDK_FOUND)
	if(LSDK_LIB)
		foreach(LIB ${LSDK_LIB})
			list(APPEND LSDK_LIBRARIES optimized ${LIB})
		endforeach()
	endif()
	if(LSDK_LIBD)
		foreach(LIB ${LSDK_LIBD})
			list(APPEND LSDK_LIBRARIES debug ${LIB})
		endforeach()
		message(STATUS "LSDK [INFO]: Debug libraries are available.")
	elseif(LSDK_LIB)
		foreach(LIB ${LSDK_LIB})
			list(APPEND LSDK_LIBRARIES debug ${LIB})
		endforeach()
		message(STATUS "LSDK [WARN]: Debug libraries are NOT available.")
	endif()

	message(STATUS "LSDK [INFO]: Found SDK in ${LSDK_ROOT}.")
else()
	message(STATUS "LSDK [WARN]: SDK was NOT found.")
endif(LSDK_FOUND)

#message(STATUS "LSDK [DEBUG]: LSDK_LIBRARIES = ${LSDK_LIBRARIES}.")

# Don't show in GUI
mark_as_advanced(
	LSDK_INCLUDE_DIRS
	LSDK_LIBRARIES
	LSDK_COMPILER_NAME
	LSDK_TARGET_NAME
	LSDK_LIB_PREFIX
	LSDK_LIB_NAMES
	LSDK_LIB_NAMESD
	LSDK_LIB 
	LSDK_LIBD
	LIB_PATH
)
