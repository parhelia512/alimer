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

set (TARGET_NAME Alimer)

# Macro to add files from a directory grouped to the solution file
macro (add_engine_directory DIRECTORY)
	file (GLOB DIR_FILES ${DIRECTORY}/*.h ${DIRECTORY}/*.c ${DIRECTORY}/*.cpp)
	get_filename_component (GROUP ${DIRECTORY} NAME)
	source_group ("${GROUP}" FILES ${DIR_FILES})
	list (APPEND SOURCE_FILES ${DIR_FILES})
endmacro ()

macro (add_engine_directory_group DIRECTORY GROUP)
	file (GLOB DIR_FILES ${DIRECTORY}/*.h ${DIRECTORY}/*.c ${DIRECTORY}/*.cpp)
	source_group ("${GROUP}" FILES ${DIR_FILES})
	list (APPEND SOURCE_FILES ${DIR_FILES})
endmacro ()

# Set definitions
if (WIN32 AND NOT ALIMER_OPENGL)
	set (ALIMER_D3D11 TRUE)
endif ()

# Generate the AlimerConfig.h file
configure_file (AlimerConfig.h.in ${CMAKE_CURRENT_SOURCE_DIR}/AlimerConfig.h)

# Add source files from subdirectories
add_engine_directory (Base)
add_engine_directory (Debug)
add_engine_directory (Graphics)
add_engine_directory (IO)
add_engine_directory (Math)
add_engine_directory (Object)
add_engine_directory (Renderer)
add_engine_directory (Resource)
add_engine_directory (Scene)
add_engine_directory (Thread)
add_engine_directory (ThirdParty/STB)
add_engine_directory (Window)
if (ALIMER_D3D11)
	add_engine_directory_group (Graphics/D3D11 Graphics)
endif ()
if (ALIMER_OPENGL)
	add_engine_directory_group (Graphics/GL Graphics)
	add_engine_directory (ThirdParty/FlextGL)
endif ()
if (WIN32)
	add_engine_directory_group (Window/Win32 Window)
endif ()

# Add root-level files
file (GLOB ROOT_FILES *.h *.c *.cpp)
list (APPEND SOURCE_FILES ${ROOT_FILES})

# Define the library target.
add_library (${TARGET_NAME} ${ALIMER_LIBRARY_TYPE} ${SOURCE_FILES})

target_include_directories(${TARGET_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>        # For parent projects to find engine headers
	PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

if (BUILD_SHARED_LIBS OR EMSCRIPTEN)
	target_compile_definitions (${TARGET_NAME} PRIVATE -DALIMER_EXPORTS)
else ()
	target_compile_definitions (${TARGET_NAME} PUBLIC -DALIMER_STATIC)
endif ()

# Add third party include directories. Only used internally
include_directories (ThirdParty/STB)

# Add rendering API specific includes and library dependencies
if (ALIMER_D3D11)
	if (DIRECT3D_FOUND)
		include_directories (${DIRECT3D_INCLUDE_DIRS})
	endif ()
	target_link_libraries (${TARGET_NAME} ${DIRECT3D_LIBRARIES})
endif ()

if (ALIMER_OPENGL)
	include_directories (ThirdParty/FlextGL)
	find_package (OpenGL REQUIRED)
	target_link_libraries (${TARGET_NAME} ${OPENGL_gl_LIBRARY})
endif ()

# Add OS-specific library dependencies
if (WIN32)
	target_link_libraries (${TARGET_NAME} winmm)
else ()
	target_link_libraries (${TARGET_NAME} pthread)
endif ()
