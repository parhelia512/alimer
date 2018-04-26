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
define_engine_source_files(Base Debug IO Math Object Renderer Resource Scene)
define_engine_source_files(NORECURSE . Window Application Graphics)
define_engine_source_files(nlohmann)

if (ALIMER_SDL)
	define_engine_source_files(Application/SDL2)
	define_engine_source_files(Window/SDL2)
endif ()

if (ALIMER_D3D11)
	define_engine_source_files(Graphics/D3D11)
endif ()

if (ALIMER_OPENGL)
	define_engine_source_files(Graphics/GL)
endif ()

if (PLATFORM_WINDOWS)
	define_engine_source_files(Application/Windows)
	define_engine_source_files(IO/Windows)
	define_engine_source_files(Window/Win32)
endif ()

define_engine_source_files(ThirdParty/STB)

# Group source code in VS solution
group_sources()

set (THIRD_PARTY_DEPENDENCIES)
if (ALIMER_SDL)
	list(APPEND THIRD_PARTY_DEPENDENCIES SDL)
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
add_library (Alimer ${ALIMER_LIBRARY_TYPE} ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(Alimer PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	$<INSTALL_INTERFACE:${DEST_BASE_INCLUDE_DIR}>
	$<INSTALL_INTERFACE:${DEST_THIRDPARTY_HEADERS_DIR}>
	$<BUILD_INTERFACE:${ALIMER_THIRD_PARTY_DIR}/spdlog/include>
)

target_include_directories(Alimer PRIVATE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty>
)

if (BUILD_SHARED_LIBS OR EMSCRIPTEN)
	target_compile_definitions (Alimer PUBLIC -DALIMER_SHARED)
	target_compile_definitions (Alimer PRIVATE -DALIMER_EXPORTS)
endif ()

if (ALIMER_SDL)
	target_compile_definitions (Alimer PRIVATE -DALIMER_SDL)
endif ()

# Add rendering API specific includes and library dependencies
if (ALIMER_D3D11)
	target_compile_definitions (Alimer PRIVATE -DNOMINMAX)
endif ()

if (ALIMER_OPENGL)
	include_directories (ThirdParty/FlextGL)
	find_package (OpenGL REQUIRED)
	target_link_libraries (Alimer ${OPENGL_gl_LIBRARY})
endif ()

# Link dependencies
foreach (dep ${THIRD_PARTY_DEPENDENCIES})
	if (TARGET "${dep}")
		get_target_property(_TARGET_TYPE ${dep} TYPE)
		if(_TARGET_TYPE STREQUAL "OBJECT_LIBRARY")
			target_link_objects(Alimer "${dep}")
		else ()
			target_link_libraries(Alimer "${dep}")
		endif ()
	endif ()
endforeach()

# Add OS-specific library dependencies
if (WIN32)
	target_link_libraries (Alimer winmm user32 gdi32 winmm imm32 ole32 oleaut32 version uuid)
else ()
	target_link_libraries (Alimer pthread)
endif ()

install(TARGETS Alimer
	EXPORT Alimer
	LIBRARY DESTINATION ${DEST_LIBRARY_DIR}
	RUNTIME DESTINATION ${DEST_BIN_DIR}
	ARCHIVE DESTINATION ${DEST_ARCHIVE_DIR}
)

# Generate platform specific pkg-config file for the benefit of Alimer library users via SDK without CMake
get_property(ALIMER_LIBS TARGET Alimer PROPERTY INTERFACE_LINK_LIBRARIES)
foreach (lib ${ALIMER_LIBS})
	string(REPLACE ".lib" "" cleaned_lib "${lib}")
	if (TARGET ${cleaned_lib} OR "${cleaned_lib}" STREQUAL AlimerBase)
		list (REMOVE_ITEM ALIMER_LIBS ${lib})
	endif ()
endforeach ()
set (LIB_NAME Alimer)
if (MSVC)
	set (DASH /)
	set (LIB_DIR "/LIBPATH:\"\${libdir}\"")
	string (REPLACE ";" ".lib\" \"" ALIMER_LIBS "\"${LIB_NAME}.lib\" \"${ALIMER_LIBS}.lib\"")
	set (PC_SUFFIX .msvc)
else ()
	set (DASH -)
	set (LIB_DIR "-L\"\${libdir}\"")
	string (REPLACE ";" " -l" ALIMER_LIBS "-l${LIB_NAME};${ALIMER_LIBS}")
endif ()