#
# Alimer is based on the Turso3D codebase.
# Copyright (c) 2018 Amer Koleci and contributors.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

include (AlimerMacros)

# Define standard configurations
if ( CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_CONFIGURATION_TYPES MATCHES "Debug;Dev;Release" )
    set (CMAKE_CONFIGURATION_TYPES "Debug;Dev;Release" CACHE STRING "List of supported configurations." FORCE)
endif()

# Set compiler variable
set ("${CMAKE_CXX_COMPILER_ID}" ON)

# Configure variables
set (ALIMER_URL "https://github.com/amerkoleci/alimer")
set (ALIMER_DESCRIPTION "Alimer is a free lightweight, cross-platform 2D and 3D game engine implemented in C++11 and released under the MIT license. Forked from Turso3D (https://github.com/cadaver/turso3d).")
execute_process (COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/CMake/Modules/GetAlimerRevision.cmake WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_VARIABLE ALIMER_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
string (REGEX MATCH "([^.]+)\\.([^.]+)\\.(.+)" MATCHED "${ALIMER_VERSION}")

# Setup SDK install destinations
if (WIN32)
    set (SCRIPT_EXT .bat)
else ()
    set (SCRIPT_EXT .sh)
endif ()

if (ANDROID)
    # For Android platform, install to a path based on the chosen Android ABI, e.g. libs/armeabi-v7a
    set (LIB_SUFFIX s/${ANDROID_NDK_ABI_NAME})
endif ()

set (DEST_BASE_INCLUDE_DIR include)
set (DEST_INCLUDE_DIR ${DEST_BASE_INCLUDE_DIR}/Alimer)
set (DEST_BIN_DIR bin)
set (DEST_TOOLS_DIR ${DEST_BIN_DIR})
set (DEST_SAMPLES_DIR ${DEST_BIN_DIR})
set (DEST_SHARE_DIR share)
set (DEST_RESOURCE_DIR ${DEST_BIN_DIR})
set (DEST_BUNDLE_DIR ${DEST_SHARE_DIR}/Applications)
set (DEST_ARCHIVE_DIR lib${LIB_SUFFIX})
set (DEST_THIRDPARTY_HEADERS_DIR ${DEST_INCLUDE_DIR}/ThirdParty)
if (ANDROID)
    set (DEST_LIBRARY_DIR ${DEST_ARCHIVE_DIR})
else ()
    set (DEST_LIBRARY_DIR bin)
endif ()

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${DEST_BIN_DIR})
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${DEST_LIBRARY_DIR})
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${DEST_ARCHIVE_DIR})

# Setup global per-platform compiler/linker options
if( PLATFORM_WINDOWS OR PLATFORM_UWP )
	add_compile_options(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE)
	add_compile_options($<$<CONFIG:DEBUG>:-D_SECURE_SCL_THROWS=0> $<$<CONFIG:DEBUG>:-D_SILENCE_DEPRECATION_OF_SECURE_SCL_THROWS>)
	add_compile_options(-D_HAS_ITERATOR_DEBUGGING=$<CONFIG:DEBUG> -D_SECURE_SCL=$<CONFIG:DEBUG>)
	add_compile_options(-D_HAS_EXCEPTIONS=0)

	# Enable full optimization in dev/release
	add_compile_options($<$<CONFIG:DEBUG>:/Od> $<$<NOT:$<CONFIG:DEBUG>>:/Ox>)

	# Inline function expansion
	add_compile_options(/Ob2)

	# Enable intrinsic functions in dev/release
	add_compile_options($<$<NOT:$<CONFIG:DEBUG>>:/Oi>)

	# Favor fast code
	add_compile_options(/Ot)

	# Enable fiber-safe optimizations in dev/release
	add_compile_options($<$<NOT:$<CONFIG:DEBUG>>:/GT>)

	# Enable string pooling
	add_compile_options(/GF)

	# Select static/dynamic runtime library
	if( PLATFORM_WINDOWS )
		add_compile_options($<$<CONFIG:DEBUG>:/MTd> $<$<NOT:$<CONFIG:DEBUG>>:/MT>)
	elseif( PLATFORM_UWP )
		add_compile_options($<$<CONFIG:DEBUG>:/MDd> $<$<NOT:$<CONFIG:DEBUG>>:/MD>)
	endif()

	# Disable specific link libraries
	if( PLATFORM_WINDOWS )
		add_linker_flags(/NODEFAULTLIB:"MSVCRT.lib")
	endif()

	# Use security checks only in debug
	if( PLATFORM_UWP )
		add_compile_options($<$<CONFIG:DEBUG>:/sdl> $<$<NOT:$<CONFIG:DEBUG>>:/sdl->)
	else()
		add_compile_options($<$<CONFIG:DEBUG>:/GS> $<$<NOT:$<CONFIG:DEBUG>>:/GS->)
	endif()

	# Enable function-level linking
	add_compile_options(/Gy)

	# Enable SIMD
	if( PLATFORM_WINDOWS )
		if( ALIMER_64BIT )
			if( ALIMER_AVX )
				add_compile_options(/arch:AVX -DAVX)
			endif()
		else()
			add_compile_options(/arch:SSE2)
		endif()
	endif()

	# Use fast floating point model
	add_compile_options(/fp:fast)

	# Enable multi-processor compilation for Visual Studio 2012 and above
	add_compile_options(/MP)

	# Set warning level 3
	add_compile_options(/W3)

	# Disable specific warnings
	add_compile_options(/wd4351 /wd4005)

	# Disable specific warnings for MSVC14 and above
	if( (PLATFORM_WINDOWS OR PLATFORM_UWP) AND (NOT MSVC_VERSION LESS 1900) )
		add_compile_options(/wd4838 /wd4312 /wd4477 /wd4244 /wd4091 /wd4311 /wd4302 /wd4476 /wd4474)
		add_compile_options(/wd4309)	# truncation of constant value
	endif()

	# Force specific warnings as errors
	add_compile_options(/we4101)
endif ()

# Initialize the development configuration using release configuration
copy_release_build_flags(DEV)

set(DEBUG_COMPILE_OPTIONS "-DALIMER_DEV=1" "-DRMT_ENABLED=1")
add_compile_options("$<$<CONFIG:Debug>:${DEBUG_COMPILE_OPTIONS}>")

set(DEV_COMPILE_OPTIONS "-DALIMER_DEV=1" "-DRMT_ENABLED=1")
add_compile_options("$<$<CONFIG:Dev>:${DEV_COMPILE_OPTIONS}>")

set(RELEASE_COMPILE_OPTIONS "-DALIMER_DEV=0" "-DRMT_ENABLED=0")
add_compile_options("$<$<CONFIG:Release>:${RELEASE_COMPILE_OPTIONS}>")