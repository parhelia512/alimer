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

enable_language (C)
set (SDL2_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set (SDL2_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

file (GLOB HEADER_FILES 
	include/*.h 
	src/*.h
	src/audio/*.h
	src/dynapi/*.h
	src/events/*.h
	src/haptic/*.h
	src/libm/*.h
	src/joystick/*.h
	src/power/*.h
	src/thread/*.h
	src/timer/*.h
	src/video/*.h
	src/video/yuv2rgb/*.h
	src/render/*.h
	)
file (GLOB  SOURCE_FILES 
	src/*.c
	src/atomic/*.c
	src/audio/*.c
	src/cpuinfo/*.c
	src/dynapi/*.c
	src/events/*.c
	src/file/*.c
	src/haptic/*.c
	src/libm/*.c
	src/joystick/*.c
	src/power/*.c
	src/stdlib/*.c
	src/thread/*.c
	src/timer/*.c
	src/video/*.c
	src/video/yuv2rgb/*.c
	src/render/*.c
	)

if (PLATFORM_WINDOWS)
	file (GLOB PLATFORM_HEADER_FILES
		src/audio/disk/*.h
		src/audio/wasapi/*.h
		src/core/windows/*.h
		src/haptic/windows/*.h
		src/joystick/windows/*.h
		src/thread/windows/*.h
		src/video/windows/*.h
		src/render/direct3d/*.h
		)
	file (GLOB PLATFORM_SOURCE_FILES
		src/audio/disk/*.c
		src/audio/wasapi/*.c
		src/core/windows/*.c
		src/filesystem/windows/*.c
		src/haptic/windows/*.c
		src/loadso/windows/*.c
		src/joystick/windows/*.c
		src/power/windows/*.c
		src/thread/generic/SDL_syscond.c
		src/thread/windows/*.c
		src/timer/windows/*.c
		src/video/windows/*.c
		src/render/direct3d/*.c
		)
endif()

list(APPEND HEADER_FILES ${PLATFORM_HEADER_FILES})
list(APPEND SOURCE_FILES ${PLATFORM_SOURCE_FILES})

add_library(SDL2 STATIC ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(SDL2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_definitions(SDL2 PRIVATE
	-DSDL_RENDER_DISABLED)

if (BUILD_SHARED_LIBS)
	target_compile_definitions(SDL2 PUBLIC -DSDL_EXPORTS)
endif ()

if (PLATFORM_WINDOWS)
	target_link_libraries (SDL2 user32 gdi32 winmm imm32 ole32 oleaut32 version uuid advapi32 shell32)
endif ()

set_property(TARGET SDL2 PROPERTY FOLDER "ThirdParty")

# Install headers for building and using the Urho3D library (install dependency for InputEvents.h)
install (DIRECTORY include/ DESTINATION ${DEST_THIRDPARTY_HEADERS}/ FILES_MATCHING PATTERN *.h)    # Note: the trailing slash is significant
install (FILES ${SDL2_BINARY_DIR}/include/SDL/generated/SDL_config.h DESTINATION ${DEST_THIRDPARTY_HEADERS}/SDL)