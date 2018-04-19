#
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

set(SPIRV_CROSS_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/GLSL.std.450.h
	${CMAKE_CURRENT_SOURCE_DIR}/spirv_common.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/spirv_cfg.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/spirv_cfg.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/spirv_cross.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/spirv_cross.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/spirv.hpp
)

if (ALIMER_D3D11 OR ALIMER_D3D12)
	list(APPEND SPIRV_CROSS_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/spirv_hlsl.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/spirv_hlsl.hpp
	)
endif()

if (ALIMER_VULKAN OR ALIMER_OPENGL)
	list(APPEND SPIRV_CROSS_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/spirv_glsl.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/spirv_glsl.hpp
	)
endif()

add_library(spirv_cross OBJECT ${SPIRV_CROSS_SOURCES})
target_include_directories(spirv_cross
	PUBLIC    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
set_target_properties(spirv_cross PROPERTIES FOLDER ThirdParty)