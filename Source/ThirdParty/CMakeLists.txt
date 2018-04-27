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

if (ALIMER_SDL)
	# Setup SDL2.
	set (SDL_SHARED OFF)
	set (SDL_STATIC ON)
	set (VIDEO_OPENGL OFF)
	set (VIDEO_OPENGLES OFF)
	set (VIDEO_VULKAN OFF)
	set (SDL_RENDER_ENABLED_BY_DEFAULT OFF)

	if( PLATFORM_WINDOWS )
		set (EXTRA_CFLAGS "-MT -Z7 -DSDL_MAIN_HANDLED -DHAVE_LIBC -D_USE_MATH_DEFINES -D_UNICODE -DALIMER_DISABLE_DYNAPI")
		set (LIBC ON)
	endif ()

	add_subdirectory (SDL2 EXCLUDE_FROM_ALL)

	if (SDL_SHARED)
		set_target_properties(SDL2 PROPERTIES FOLDER ThirdParty)
	else ()
		set_target_properties(SDL2-static PROPERTIES FOLDER ThirdParty)
	endif()
endif()