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

# Source environment
if ("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Windows")
	execute_process(COMMAND cmd /c set OUTPUT_VARIABLE ENVIRONMENT)
else ()
	execute_process(COMMAND env OUTPUT_VARIABLE ENVIRONMENT)
endif ()
string(REGEX REPLACE "=[^\n]*\n?" ";" ENVIRONMENT "${ENVIRONMENT}")
set (IMPORT_ALIMER_VARIABLES_FROM_ENV BUILD_SHARED_LIBS)
foreach(key ${ENVIRONMENT})
	list (FIND IMPORT_ALIMER_VARIABLES_FROM_ENV ${key} _index)
	if ("${key}" MATCHES "^(ALIMER_|CMAKE_|ANDROID_).+" OR ${_index} GREATER -1)
		if (NOT DEFINED ${key})
			set (${key} $ENV{${key}} CACHE STRING "" FORCE)
		endif ()
	endif ()
endforeach()

# Set platform variables
if( "${CMAKE_SYSTEM_NAME}" STREQUAL "Windows" )
	set (PLATFORM_WINDOWS ON)
	set (PLATFORM_NAME "Windows")
elseif( "${CMAKE_SYSTEM_NAME}" STREQUAL "WindowsStore" )
	set(PLATFORM_UWP ON)
	set(PLATFORM_NAME "UWP")
elseif( "${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin" )
	if( IOS )
		set (PLATFORM_IOS ON)
		set (PLATFORM_NAME "iOS")
	else()
		set (PLATFORM_OSX ON)
		set (PLATFORM_NAME "macOS")
	endif()
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
	set (PLATFORM_LINUX ON)
	set (PLATFORM_NAME "Linux")
elseif( "${CMAKE_SYSTEM_NAME}" STREQUAL "Android" )
	set (PLATFORM_ANDROID ON)
	set (PLATFORM_NAME "Android")
elseif( "${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten" )
	set (PLATFORM_WEB ON)
	set (PLATFORM_NAME "Web")
else()
	message(FATAL_ERROR "Unknown platform ${CMAKE_SYSTEM_NAME}!")
endif ()

if (PLATFORM_WINDOWS OR PLATFORM_LINUX OR PLATFORM_OSX)
	set (ALIMER_DESKTOP ON)
endif ()

if (PLATFORM_ANDROID OR PLATFORM_IOS)
	set (ALIMER_MOBILE ON)
endif ()

# Detect target architecture
if( ((PLATFORM_WINDOWS OR PLATFORM_OSX OR PLATFORM_UWP) AND CMAKE_CL_64) OR (PLATFORM_IOS AND CMAKE_OSX_ARCHITECTURES MATCHES "arm64") OR PLATFORM_LINUX )
	set(ALIMER_64BIT 1)
endif()

if (NOT DEFINED ALIMER_64BIT)
	if (CMAKE_SIZEOF_VOID_P MATCHES 8)
		set(ALIMER_64BIT ON)
	else ()
		set(ALIMER_64BIT OFF)
	endif ()
endif ()

# Determine library type
string(TOUPPER "${BUILD_SHARED_LIBS}" BUILD_SHARED_LIBS)
if ("${BUILD_SHARED_LIBS}" STREQUAL "MODULE")
	set (BUILD_SHARED_LIBS OFF)
	set (ALIMER_LIBRARY_TYPE MODULE)
elseif (BUILD_SHARED_LIBS)
	set (ALIMER_LIBRARY_TYPE SHARED)
else ()
	set (ALIMER_LIBRARY_TYPE STATIC)
endif ()

# Define CMake options
include (CMakeDependentOption)
option(ALIMER_ENABLE_ALL "Enables all optional subsystems by default" OFF)

set (ALIMER_LOGGING_DEFAULT ON)
set (ALIMER_PROFILING_DEFAULT ON)

# Threads are still experimental on emscripten.
if (NOT PLATFORM_WEB OR ALIMER_ENABLE_ALL)
	set (ALIMER_THREADS_DEFAULT ON)
else ()
	set (ALIMER_THREADS_DEFAULT OFF)
endif ()

# Disable SIMD under emscripten
if (NOT PLATFORM_WEB OR ALIMER_ENABLE_ALL)
	set (ALIMER_SIMD_DEFAULT ON)
else ()
	set (ALIMER_SIMD_DEFAULT OFF)
endif ()

# Graphics backends
if (NOT ALIMER_DISABLE_D3D11)
	if (PLATFORM_WINDOWS OR PLATFORM_UWP)
		set (ALIMER_D3D11_DEFAULT ON)
	else ()
		set (ALIMER_D3D11_DEFAULT OFF)
	endif ()
endif ()

if (NOT ALIMER_DISABLE_D3D12)
	if (PLATFORM_WINDOWS OR PLATFORM_UWP)
		set (ALIMER_D3D12_DEFAULT ON)
	else ()
		set (ALIMER_D3D12_DEFAULT OFF)
	endif ()
endif ()

cmake_dependent_option (ALIMER_STATIC_RUNTIME "Use static C/C++ runtime library with MSVC" FALSE "MSVC" FALSE)
option (ALIMER_LOGGING "Enable logging" ${ALIMER_LOGGING_DEFAULT})
option (ALIMER_PROFILING "Enable performance profiling" ${ALIMER_PROFILING_DEFAULT})
option (ALIMER_THREADING "Enable multithreading" ${ALIMER_THREADS_DEFAULT})
option (ALIMER_SIMD "Enable SIMD (SSE, NEON) instructions" ${ALIMER_SIMD_DEFAULT})
option (ALIMER_D3D11 "Enable D3D11 backend" ${ALIMER_D3D11_DEFAULT})
option (ALIMER_D3D12 "Enable D3D12 backend" ${ALIMER_D3D12_DEFAULT})

cmake_dependent_option (ALIMER_OPENGL "Use OpenGL instead of Direct3D11 on Windows" FALSE "WIN32" TRUE)