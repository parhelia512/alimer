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

# Set alimer sdk path.
set (ALIMER_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set (ALIMER_SDK_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Source/Alimer)
set (ALIMER_THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Source/ThirdParty)

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

# Packaging
include(AlimerPackaging)

# Install CMake modules and toolchains provided by and for Urho3D
install (DIRECTORY ${CMAKE_SOURCE_DIR}/CMake/ DESTINATION ${DEST_SHARE_DIR}/CMake)    # Note: the trailing slash is significant

# Install CMake build scripts
file (GLOB CMAKE_SCRIPTS ${CMAKE_SOURCE_DIR}/*${SCRIPT_EXT})
install (PROGRAMS ${CMAKE_SCRIPTS} DESTINATION ${DEST_SHARE_DIR}/Scripts)
# Install data files
file (MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
if (NOT ANDROID)
    foreach (RES_DIR ${RESOURCE_DIRS})
        get_filename_component (NAME ${RES_DIR} NAME)
        install (DIRECTORY ${RES_DIR} DESTINATION ${DEST_RESOURCE_DIR})
        if (NOT EXISTS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NAME})
            create_symlink(${RES_DIR} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NAME})
        endif ()
    endforeach ()
    foreach (RES_DIR ${AUTOLOAD_DIRS})
        get_filename_component (NAME ${RES_DIR} NAME)
        install (DIRECTORY ${RES_DIR} DESTINATION ${DEST_RESOURCE_DIR}/Autoload)
        if (NOT EXISTS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Autoload/${NAME})
            create_symlink(${RES_DIR} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Autoload/${NAME})
        endif ()
    endforeach ()
endif ()
# install (DIRECTORY ${CMAKE_BINARY_DIR}/bin/Cache DESTINATION ${DEST_RESOURCE_DIR})

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
message(STATUS "  D3D11           ${ALIMER_D3D11}")
message(STATUS "  D3D12           ${ALIMER_D3D12}")

message(STATUS "Developer options:")
message(STATUS "  Packaging       ${ALIMER_PACKAGING}")
message(STATUS "  Logging         ${ALIMER_LOGGING}")
message(STATUS "  Profiling       ${ALIMER_PROFILING}")
message(STATUS "  Tools           ${ALIMER_TOOLS}")


# Set VS Startup project.
if(CMAKE_VERSION VERSION_GREATER  "3.6")
	if (ALIMER_TOOLS)
		set_property( DIRECTORY PROPERTY VS_STARTUP_PROJECT "AlimerStudio" )
	else ()
		# set_property( DIRECTORY PROPERTY VS_STARTUP_PROJECT "AlimerPlayer" )
	endif ()
endif()