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

# Set CMake minimum version
cmake_minimum_required (VERSION 3.5)

# Set project name
project (Alimer)

# Set CMake modules search path
set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMake/Modules)

# Include file that sets up all configurable properties
include(AlimerOptions)

# Include common utilitles
include(AlimerCommon)

# Enable common build options
set (CMAKE_CXX_STANDARD 11)
# Workarounds
if (EMSCRIPTEN OR APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
endif ()
if (APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif ()

set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_POSITION_INDEPENDENT_CODE ON)

# Enable the creation of folders for Visual Studio projects
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (ALIMER_SIMD AND NOT MSVC)
	# Build engine and it's dependencies with SSE/SSE2 enabled.
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse -msse2")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse -msse2")
endif ()

# Set default configuration to Release for single-configuration generators
if (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
	set (CMAKE_BUILD_TYPE Release)
endif ()

# Find DirectX SDK
# Based on realXtend Tundra CMake build system (https://github.com/realXtend/tundra)
if (WIN32 AND NOT TURSO3D_OPENGL)
	# Do not search for the SDK from Visual Studio 2012 onward to avoid incompatibility
	# between DirectX SDK and Windows SDK defines and the resulting warning spam
	if (MSVC AND MSVC_VERSION LESS 1700)
		set (DIRECTX_INC_SEARCH_PATH
			"C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Include"
			"C:/Program Files/Microsoft DirectX SDK (June 2010)/Include"
			"$ENV{DIRECTX_ROOT}/Include"
			"$ENV{DXSDK_DIR}/Include")
		find_path (DIRECT3D_INCLUDE_DIRS d3d11.h ${DIRECTX_INC_SEARCH_PATH})

		if (CMAKE_CL_64)
			set (DIRECTX_LIB_SEARCH_PATH
				"C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Lib/x64"
				"C:/Program Files/Microsoft DirectX SDK (June 2010)/Lib/x64"
				"$ENV{DIRECTX_ROOT}/Lib/x64"
				"$ENV{DXSDK_DIR}/Lib/x64")
		else ()
			set (DIRECTX_LIB_SEARCH_PATH
				"C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Lib"
				"C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Lib/x86"
				"C:/Program Files/Microsoft DirectX SDK (June 2010)/Lib"
				"C:/Program Files/Microsoft DirectX SDK (June 2010)/Lib/x86"
				"$ENV{DIRECTX_ROOT}/Lib"
				"$ENV{DIRECTX_ROOT}/Lib/x86"
				"$ENV{DXSDK_DIR}/Lib"
				"$ENV{DXSDK_DIR}/Lib/x86")
		endif ()

		find_library (D3D11_LIBRARY d3d11 ${DIRECTX_LIB_SEARCH_PATH})
		find_library (DXGI_LIBRARY dxgi ${DIRECTX_LIB_SEARCH_PATH})
		find_library (DXGUID_LIBRARY dxguid ${DIRECTX_LIB_SEARCH_PATH})
		find_library (D3DCOMPILER_LIBRARY d3dcompiler ${DIRECTX_LIB_SEARCH_PATH})
	else ()
		message (STATUS "Skipping DirectX SDK search as the used compiler does not require it")
	endif ()

	if (DIRECT3D_INCLUDE_DIRS AND D3D11_LIBRARY AND DXGI_LIBRARY AND DXGUID_LIBRARY AND D3DCOMPILER_LIBRARY)
		set (DIRECT3D_FOUND 1)
		set (DIRECT3D_LIBRARIES ${D3D11_LIBRARY} ${DXGI_LIBRARY} ${DXGUID_LIBRARY} ${D3DCOMPILER_LIBRARY})
		include (FindPackageMessage)
		find_package_message (Direct3D "Found DirectX SDK: ${DIRECT3D_LIBRARIES} ${DIRECT3D_INCLUDE_DIRS}" "[${DIRECT3D_LIBRARIES}][${DIRECT3D_INCLUDE_DIRS}]")
	else ()
		# If DirectX SDK not found, assume the includes & libs are found in the compiler default directories
		set (DIRECT3D_LIBRARIES d3d11 dxgi dxguid d3dcompiler)
	endif ()
endif ()

# Compiler-specific setup
if (MSVC)
	if (ALIMER_STATIC_RUNTIME)
		set (RELEASE_RUNTIME /MT)
		set (DEBUG_RUNTIME /MTd)
	else ()
		set (RELEASE_RUNTIME "")
		set (DEBUG_RUNTIME "")
	endif ()
	add_definitions (-D_CRT_SECURE_NO_WARNINGS)
	# Enable parallel builds
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
	set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${DEBUG_RUNTIME}")
	set (CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELEASE} ${RELEASE_RUNTIME} /fp:fast /Zi /GS-")
	set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELWITHDEBINFO}")

	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
	set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${DEBUG_RUNTIME} /W4")
	set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELEASE} ${RELEASE_RUNTIME} /W4 /fp:fast /Zi /GS- ")
	set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")

	set (CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /OPT:REF /OPT:ICF /DEBUG")
	set (CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /OPT:REF /OPT:ICF")
elseif (NOT XCODE)
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffast-math")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wno-invalid-offsetof -ffast-math")
	if (WIN32)
		set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc -static")
		set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc -static")
		set (CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG")
		set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG")
		set (CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
		set (CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
	endif ()
endif ()

# Source
add_subdirectory (Source)

# Samples
add_subdirectory (Samples)

# Install CMake modules and toolchains provided by and for Urho3D
install (DIRECTORY ${CMAKE_SOURCE_DIR}/CMake/ DESTINATION ${DEST_SHARE_DIR}/CMake)    # Note: the trailing slash is significant

# Install CMake build scripts
file (GLOB CMAKE_SCRIPTS ${CMAKE_SOURCE_DIR}/*${SCRIPT_EXT})
install (PROGRAMS ${CMAKE_SCRIPTS} DESTINATION ${DEST_SHARE_DIR}/Scripts)

# Print current build configuration
message(STATUS "Alimer Configuration:")
message(STATUS "  Platform        ${PLATFORM_NAME}")

if (ALIMER_STATIC_RUNTIME)
	message(STATUS "  Runtime         STATIC")
else ()
	message(STATUS "  Runtime         SHARED")
endif ()

message(STATUS "  Library         ${ALIMER_LIBRARY_TYPE}")
message(STATUS "  SIMD            ${ALIMER_SIMD}")

message(STATUS "Developer options:")
message(STATUS "  Logging         ${ALIMER_LOGGING}")
message(STATUS "  Profiling       ${ALIMER_PROFILING}")