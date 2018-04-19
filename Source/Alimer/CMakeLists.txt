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

macro (define_engine_source_files)
	cmake_parse_arguments(DEFINE_SRC_FILES "NORECURSE" "" "" ${ARGN} )
	if (DEFINE_SRC_FILES_NORECURSE)
		set (_action GLOB)
	else ()
		set (_action GLOB_RECURSE)
	endif ()
	foreach (path ${DEFINE_SRC_FILES_UNPARSED_ARGUMENTS})
		# Get header files
		file (${_action} _files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${path}/*.h)
		list (REMOVE_ITEM _files
			Alimer.h
			Graphics/GraphicsImpl.h
		)
		list (APPEND HEADER_FILES ${_files})
		# Install them
		install (FILES ${_files} DESTINATION ${DEST_INCLUDE_DIR}/${path})
		# Get source files
		file (${_action} _files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${path}/*.cpp)
		list (APPEND SOURCE_FILES ${_files})
	endforeach ()
endmacro()

# Generate the AlimerConfig.h file
configure_file (AlimerConfig.h.in ${CMAKE_CURRENT_SOURCE_DIR}/AlimerConfig.h)

# Add source files from subdirectories
define_engine_source_files(NORECURSE . Graphics)
define_engine_source_files (Base Debug IO JSON Math Object Renderer Resource Scene Window Application)

if (ALIMER_SDL)
	define_engine_source_files (Application/SDL2)
	define_engine_source_files  (Window/SDL2)
endif ()

if (ALIMER_D3D11)
	define_engine_source_files  (Graphics/D3D11)
endif ()

if (ALIMER_OPENGL)
	define_engine_source_files  (Graphics/GL)
endif ()

if (PLATFORM_WINDOWS)
	define_engine_source_files  (Application/Windows)
	define_engine_source_files  (IO/Windows)
	define_engine_source_files  (Window/Win32)
endif ()

# Group source code in VS solution
group_sources()

set (THIRD_PARTY_DEPENDENCIES STB spirv_cross SDL2)
if (ALIMER_SDL)
	list(APPEND THIRD_PARTY_DEPENDENCIES SDL2)
endif()

foreach (dep ${THIRD_PARTY_DEPENDENCIES})
	if (TARGET "${dep}")
		get_target_property(_TARGET_TYPE ${dep} TYPE)
		if(_TARGET_TYPE STREQUAL "OBJECT_LIBRARY")
			list(APPEND SOURCE_FILES "$<TARGET_OBJECTS:${dep}>")
		endif ()
	endif ()
endforeach()

# Define the library target.
add_library (${TARGET_NAME} ${ALIMER_LIBRARY_TYPE} ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(${TARGET_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/..>        # For parent projects to find auto-generated engine headers
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>        # For parent projects to find engine headers
	$<INSTALL_INTERFACE:${DEST_BASE_INCLUDE_DIR}>
	$<INSTALL_INTERFACE:${DEST_THIRDPARTY_HEADERS}>
	$<BUILD_INTERFACE:${ALIMER_THIRD_PARTY_DIR}/spdlog/include>
	PRIVATE
		$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

if (BUILD_SHARED_LIBS OR EMSCRIPTEN)
	target_compile_definitions (${TARGET_NAME} PRIVATE -DALIMER_EXPORTS)
else ()
	target_compile_definitions (${TARGET_NAME} PUBLIC -DALIMER_STATIC)
endif ()

if (ALIMER_SDL)
	target_compile_definitions (${TARGET_NAME} PRIVATE -DALIMER_SDL)
endif ()

# Add rendering API specific includes and library dependencies
if (ALIMER_D3D11)
	target_compile_definitions (${TARGET_NAME} PRIVATE -DNOMINMAX)
endif ()

if (ALIMER_OPENGL)
	include_directories (ThirdParty/FlextGL)
	find_package (OpenGL REQUIRED)
	target_link_libraries (${TARGET_NAME} ${OPENGL_gl_LIBRARY})
endif ()

# Link dependencies
foreach (dep ${THIRD_PARTY_DEPENDENCIES})
	if (TARGET "${dep}")
		get_target_property(_TARGET_TYPE ${dep} TYPE)
		if(_TARGET_TYPE STREQUAL "OBJECT_LIBRARY")
			target_link_objects(${TARGET_NAME} "${dep}")
		else ()
			target_link_libraries(${TARGET_NAME} "${dep}")
		endif ()
	endif ()
endforeach()

# Add OS-specific library dependencies
if (WIN32)
	target_link_libraries (${TARGET_NAME} user32 gdi32 winmm imm32 ole32 oleaut32 version uuid advapi32 shell32)
else ()
	target_link_libraries (${TARGET_NAME} pthread)
endif ()

install(TARGETS Alimer
	EXPORT Alimer
	LIBRARY DESTINATION ${SHARED_LIB_INSTALL_DIR}
	RUNTIME DESTINATION ${SHARED_LIB_INSTALL_DIR}
	ARCHIVE DESTINATION lib${LIB_SUFFIX}
)