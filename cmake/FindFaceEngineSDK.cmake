# Find the Vision Labs Face SDK.
# Sets the usual variables expected for find_package scripts:
# FSDK_INCLUDE_DIRS - headers location
# FSDK_LIBRARIES - libraries to link against
# FSDK_FOUND - true if Face SDK was found.

# This is the directory where the Face SDK is located.
# By default FSDKDIR environment variable value is taken.
set(FSDK_ROOT "$ENV{FSDKDIR}" CACHE PATH "Vision Labs Face SDK root directory.")

if (WIN32)
    option(FSDK_FIND_VLF ON)
else ()
    option(FSDK_FIND_VLF OFF)
endif ()

# Look for headers.
find_path(FSDK_INCLUDE_DIRS
          NAMES FaceEngine.h Types.h Def.h
          HINTS $ENV{FSDKDIR}
          PATHS ${FSDK_ROOT}
          PATH_SUFFIXES include)

#message(STATUS "FSDK [DEBUG]: FSDK_INCLUDE_DIRS = ${FSDK_INCLUDE_DIRS}.")

# Determine compiler version and architecture.
if(MSVC10)
	set(FSDK_COMPILER_NAME vs2010)
elseif(MSVC11)
	set(FSDK_COMPILER_NAME vs2012)
elseif(MSVC12)
	set(FSDK_COMPILER_NAME vs2013)
elseif(MSVC14)
	set(FSDK_COMPILER_NAME vs2015)
elseif(CMAKE_COMPILER_IS_GNUCXX)
	set(FSDK_COMPILER_NAME gcc4)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	set(FSDK_COMPILER_NAME clang)
else()
	message(SEND_ERROR "Unsupported compiler")
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(FSDK_TARGET_NAME x64)
else()
	set(FSDK_TARGET_NAME x86)
endif()

# What version of Face SDK to use.
set(FSDK_LIB_PREFIX ${FSDK_COMPILER_NAME}/${FSDK_TARGET_NAME})

# List of all SDK libraries.
set(FSDK_LIB_NAMES
	FaceEngineCore
	FaceEngineSDK)

# Find optimized libraries.
set(FSDK_LIB)
foreach(LIB ${FSDK_LIB_NAMES})
	set(LIB_PATH ${LIB}-NOTFOUND)
	find_library(LIB_PATH
				 NAMES ${LIB}
				 HINTS $ENV{FSDKDIR}
				 PATHS ${FSDK_ROOT}
				 PATH_SUFFIXES	lib/${FSDK_LIB_PREFIX}
								bin/${FSDK_LIB_PREFIX})

	list(APPEND FSDK_LIB ${LIB_PATH})
endforeach()

# Find debug libraries (may be missing in your distribution).
set(FSDK_LIBD)
foreach(LIB ${FSDK_LIB_NAMES})
	set(LIB_PATH ${LIB}-NOTFOUND)
	find_library(LIB_PATH
				 NAMES ${LIB}d
				 HINTS $ENV{FSDKDIR}
				 PATHS ${FSDK_ROOT}
				 PATH_SUFFIXES	lib/${FSDK_LIB_PREFIX}
								bin/${FSDK_LIB_PREFIX})

	list(APPEND FSDK_LIBD ${LIB_PATH})
endforeach()

if(${FSDK_FIND_VLF})
	# Find vlf release
	set(VLF_LIB "VLF-NOTFOUND")
	find_library(VLF_LIB
				 NAMES vlf
				 HINTS $ENV{FSDKDIR}
				 PATHS ${FSDK_ROOT}
				 PATH_SUFFIXES	lib/${FSDK_LIB_PREFIX}
								bin/${FSDK_LIB_PREFIX})
elseif(${FSDK_FIND_VLFD})
	# Find vlf debug
	set(VLF_LIBD "VLFD-NOTFOUND")
	find_library(VLF_LIBD
				 NAMES vlfd
				 HINTS $ENV{FSDKDIR}
				 PATHS ${FSDK_ROOT}
				 PATH_SUFFIXES	lib/${FSDK_LIB_PREFIX}
								bin/${FSDK_LIB_PREFIX})
endif()


#message(STATUS "FSDK [DEBUG]: FSDK_LIB = ${FSDK_LIB}.")
#message(STATUS "FSDK [DEBUG]: FSDK_LIBD = ${FSDK_LIBD}.")

# Support the REQUIRED and QUIET arguments, and set FSDK_FOUND if found.
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(FSDK DEFAULT_MSG 
                                  FSDK_LIB
                                  FSDK_INCLUDE_DIRS)

set(FSDK_LIBRARIES)

if(FSDK_FOUND)
	if(FSDK_LIB)
		foreach(LIB ${FSDK_LIB})
			list(APPEND FSDK_LIBRARIES optimized ${LIB})
		endforeach()
	endif()
	if(FSDK_LIBD)
		foreach(LIB ${FSDK_LIBD})
			list(APPEND FSDK_LIBRARIES debug ${LIB})
		endforeach()
		message(STATUS "FSDK [INFO]: Debug libraries are available.")
	elseif(FSDK_LIB)
		foreach(LIB ${FSDK_LIB})
			list(APPEND FSDK_LIBRARIES debug ${LIB})
		endforeach()
		message(STATUS "FSDK [WARN]: Debug libraries are NOT available.")
	endif()
	# add vlf
	if(${FSDK_FIND_VLF})
		list(APPEND FSDK_LIBRARIES optimized ${VLF_LIB})
	elseif(${FSDK_FIND_VLFD})
		list(APPEND FSDK_LIBRARIES debug ${VLF_LIBD})
		message("Find vlf")
	endif()

	message(STATUS "FSDK [INFO]: Found SDK in ${FSDK_ROOT}.")
else()
	message(STATUS "FSDK [WARN]: SDK was NOT found.")
endif(FSDK_FOUND)

#message(STATUS "FSDK [DEBUG]: FSDK_LIBRARIES = ${FSDK_LIBRARIES}.")

# Don't show in GUI
mark_as_advanced(
	FSDK_INCLUDE_DIRS
	FSDK_LIBRARIES
	FSDK_COMPILER_NAME
	FSDK_TARGET_NAME
	FSDK_LIB_PREFIX
	FSDK_LIB_NAMES
	FSDK_LIB_NAMESD
	FSDK_LIB 
	FSDK_LIBD
	LIB_PATH
)
