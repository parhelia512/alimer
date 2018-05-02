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
option(ALIMER_ENABLE_ALL "Enables all optional subsystems by default" OFF)

set (ALIMER_SDL_DEFAULT ON)
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

if (NOT ALIMER_DISABLE_OPENGL)
	# TODO: Restore once supported
	#if (NOT PLATFORM_UWP)
	#	set (ALIMER_OPENGL_DEFAULT ON)
	#else ()
		set (ALIMER_OPENGL_DEFAULT OFF)
	#endif ()
endif ()

if (NOT ALIMER_DISABLE_VULKAN)
	if (PLATFORM_WINDOWS OR PLATFORM_LINUX OR PLATFORM_ANDROID)
		set (ALIMER_VULKAN_DEFAULT ON)
	else ()
		set (ALIMER_VULKAN_DEFAULT OFF)
	endif ()
endif ()

# Configuration presets
set(ALIMER_PRESET None CACHE STRING "Select configuration preset: None | Developer | Release")
string(TOUPPER "${ALIMER_PRESET}" ALIMER_PRESET)

if ("${ALIMER_PRESET}" STREQUAL "DEVELOPER")
    set (ALIMER_DEVELOPER ON)
    set (ALIMER_RELEASE OFF)
elseif ("${ALIMER_PRESET}" STREQUAL "RELEASE")
    set (ALIMER_DEVELOPER OFF)
    set (ALIMER_RELEASE ON)
else ()
    set (ALIMER_DEVELOPER ${ALIMER_ENABLE_ALL})
    set (ALIMER_RELEASE ${ALIMER_ENABLE_ALL})
endif ()

if (NOT ALIMER_DISABLE_TOOLS)
	if (PLATFORM_WINDOWS OR PLATFORM_LINUX OR PLATFORM_OSX)
		set (ALIMER_TOOLS_DEFAULT ON)
	else ()
		set(ALIMER_TOOLS_DEFAULT OFF)
	endif ()
endif ()

set (ALIMER_AVX OFF CACHE BOOL "Enable AVX instructions set support.")

option (ALIMER_SDL "USE SDL2" ${ALIMER_SDL_DEFAULT})
option (ALIMER_LOGGING "Enable logging" ${ALIMER_LOGGING_DEFAULT})
option (ALIMER_PROFILING "Enable performance profiling" ${ALIMER_PROFILING_DEFAULT})
option (ALIMER_THREADING "Enable multithreading" ${ALIMER_THREADS_DEFAULT})
option (ALIMER_SIMD "Enable SIMD (SSE, NEON) instructions" ${ALIMER_SIMD_DEFAULT})
option (ALIMER_D3D11 "Enable D3D11 backend" ${ALIMER_D3D11_DEFAULT})
option (ALIMER_D3D12 "Enable D3D12 backend" ${ALIMER_D3D12_DEFAULT})
option (ALIMER_OPENGL "Enable OpenGL backend" ${ALIMER_OPENGL_DEFAULT})
option (ALIMER_VULKAN "Enable Vulkan backend" ${ALIMER_VULKAN_DEFAULT})

option(ALIMER_PACKAGING "Package resources" ${ALIMER_RELEASE})
option(ALIMER_CSHARP "Enable C# support" ${ALIMER_ENABLE_ALL})

# Tools
option(ALIMER_TOOLS "Tools enabled" ${ALIMER_TOOLS_DEFAULT})

# Unset any default config variables so they do not pollute namespace
get_cmake_property(__cmake_variables VARIABLES)
foreach (var ${__cmake_variables})
    if ("${var}" MATCHES "^ALIMER_.*_DEFAULT")
        if (${${var}})
            set(${var})
        endif ()
    endif ()
endforeach()