//
// Alimer is based on the Turso3D codebase.
// Copyright (c) 2018 Amer Koleci and contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "../Math/Matrix3x4.h"
#include "GraphicsDefs.h"

namespace Alimer
{
	extern ALIMER_API const char* resourceUsageNames[] =
	{
		"default",
		"immutable",
		"dynamic",
		nullptr
	};

	

	extern ALIMER_API const char* blendFactorNames[] =
	{
		"",
		"zero",
		"one",
		"srcColor",
		"invSrcColor",
		"srcAlpha",
		"invSrcAlpha",
		"destAlpha",
		"invDestAlpha",
		"destColor",
		"invDestColor",
		"srcAlphaSat",
		nullptr
	};

	extern ALIMER_API const char* blendOpNames[] =
	{
		"",
		"add",
		"subtract",
		"revSubtract",
		"min",
		"max",
		nullptr
	};

	extern ALIMER_API const char* blendModeNames[] =
	{
		"replace",
		"add",
		"multiply",
		"alpha",
		"addAlpha",
		"preMulAlpha",
		"invDestAlpha",
		"subtract",
		"subtractAlpha",
		nullptr
	};

	extern ALIMER_API const char* fillModeNames[] =
	{
		"",
		"",
		"wireframe",
		"solid",
		nullptr
	};

	extern ALIMER_API const char* cullModeNames[] =
	{
		"",
		"none",
		"front",
		"back",
		nullptr
	};

	extern ALIMER_API const char* compareFuncNames[] =
	{
		"",
		"never",
		"less",
		"equal",
		"lessEqual",
		"greater",
		"notEqual",
		"greaterEqual",
		"always",
		nullptr
	};

	extern ALIMER_API const char* stencilOpNames[] =
	{
		"",
		"keep",
		"zero",
		"replace",
		"incrSat",
		"descrSat",
		"invert",
		"incr",
		"decr",
		nullptr
	};

	extern ALIMER_API const BlendModeDesc blendModes[] =
	{
		BlendModeDesc(false, BLEND_ONE, BLEND_ONE, BLEND_OP_ADD, BLEND_ONE, BLEND_ONE, BLEND_OP_ADD),
		BlendModeDesc(true, BLEND_ONE, BLEND_ONE, BLEND_OP_ADD, BLEND_ONE, BLEND_ONE, BLEND_OP_ADD),
		BlendModeDesc(true, BLEND_DEST_COLOR, BLEND_ZERO, BLEND_OP_ADD, BLEND_DEST_COLOR, BLEND_ZERO, BLEND_OP_ADD),
		BlendModeDesc(true, BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA, BLEND_OP_ADD, BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA, BLEND_OP_ADD),
		BlendModeDesc(true, BLEND_SRC_ALPHA, BLEND_ONE, BLEND_OP_ADD, BLEND_SRC_ALPHA, BLEND_ONE, BLEND_OP_ADD),
		BlendModeDesc(true, BLEND_ONE, BLEND_INV_SRC_ALPHA, BLEND_OP_ADD, BLEND_ONE, BLEND_INV_SRC_ALPHA, BLEND_OP_ADD),
		BlendModeDesc(true, BLEND_INV_DEST_ALPHA, BLEND_DEST_ALPHA, BLEND_OP_ADD, BLEND_INV_DEST_ALPHA, BLEND_DEST_ALPHA, BLEND_OP_ADD),
		BlendModeDesc(true, BLEND_ONE, BLEND_ONE, BLEND_OP_REV_SUBTRACT, BLEND_ONE, BLEND_ONE, BLEND_OP_REV_SUBTRACT),
		BlendModeDesc(true, BLEND_SRC_ALPHA, BLEND_ONE, BLEND_OP_REV_SUBTRACT, BLEND_SRC_ALPHA, BLEND_ONE, BLEND_OP_REV_SUBTRACT)
	};

	uint32_t GetVertexFormatSize(VertexFormat format)
	{
		switch (format)
		{
		case VertexFormat::Float:
		case VertexFormat::Byte4:
		case VertexFormat::Byte4N:
		case VertexFormat::UByte4:
		case VertexFormat::UByte4N:
		case VertexFormat::Short2:
		case VertexFormat::Short2N:
			return 4;
		case VertexFormat::Float2:
		case VertexFormat::Short4:
		case VertexFormat::Short4N:
			return 8;
		case VertexFormat::Float3:
			return 12;
		case VertexFormat::Float4:
			return 16;

		default:
			return static_cast<uint32_t>(-1);
		}
	}

	uint32_t GetConstantElementSize(ConstantElementType type)
	{
		switch (type)
		{
		case ConstantElementType::Int:
		case ConstantElementType::Float:
			return 4;
		case ConstantElementType::Float2:
			return 8;
		case ConstantElementType::Float3:
			return 12;
		case ConstantElementType::Float4:
			return 16;

		case ConstantElementType::Matrix3x4:
			return sizeof(Matrix3x4);

		case ConstantElementType::Matrix4x4:
			return sizeof(Matrix4);

		default:
			return static_cast<uint32_t>(-1);
		}
	}
}
