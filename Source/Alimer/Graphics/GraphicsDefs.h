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

#pragma once

#include "../AlimerConfig.h"
#include "../Base/Utils.h"
#include "../Base/String.h"
#include "../Math/IntRect.h"

namespace Alimer
{
	/// Maximum vertex attributes.
	static constexpr uint32_t MaxVertexAttributes = 16u;
	/// Maximum simultaneous vertex buffers.
	static constexpr uint32_t MaxVertexBuffers = 4;
	/// Maximum simultaneous constant buffers.
	static constexpr uint32_t MAX_CONSTANT_BUFFERS = 15;
	/// Maximum number of textures in use at once.
	static constexpr uint32_t MAX_TEXTURE_UNITS = 16;
	/// Maximum number of textures reserved for materials, starting from 0.
	static constexpr uint32_t MAX_MATERIAL_TEXTURE_UNITS = 8;
	/// Maximum number of color rendertargets in use at once.
	static constexpr uint32_t MAX_RENDERTARGETS = 4;
	/// Number of cube map faces.
	static constexpr uint32_t MAX_CUBE_FACES = 6;

	/// Disable color write.
	static const unsigned char COLORMASK_NONE = 0x0;
	/// Write to red channel.
	static const unsigned char COLORMASK_R = 0x1;
	/// Write to green channel.
	static const unsigned char COLORMASK_G = 0x2;
	/// Write to blue channel.
	static const unsigned char COLORMASK_B = 0x4;
	/// Write to alpha channel.
	static const unsigned char COLORMASK_A = 0x8;
	/// Write to all color channels (default.)
	static const unsigned char COLORMASK_ALL = 0xf;

	enum class GraphicsDeviceType : uint8_t
	{
		Default,
		Empty,
		Direct3D11,
		OpenGL,
		Vulkan
	};

	enum class ClearFlagsBits : uint32_t
	{
		None = 0,
		/// Clear rendertarget color.
		Color = 0x1,
		/// Clear rendertarget depth.
		Depth = 0x2,
		/// Clear rendertarget stencil.
		Stencil = 0x4,
		/// Clear color+depth+stencil.
		All = Color | Depth | Stencil,
		Indirect = 0x10
	};
	using ClearFlags = Flags<ClearFlagsBits>;
	ALIMER_FORCE_INLINE ClearFlags operator|(ClearFlagsBits bit0, ClearFlagsBits bit1)
	{
		return ClearFlags(bit0) | bit1;
	}

	ALIMER_FORCE_INLINE ClearFlags operator~(ClearFlagsBits bits)
	{
		return ~(ClearFlags(bits));
	}

	/// Shader stages.
	enum class ShaderStage : uint32_t
	{
		Vertex = 0,
		Fragment,
		Count
	};

	/// Element types for constant buffers and vertex elements.
	enum class ConstantElementType : uint32_t
	{
		Int = 0,
		Float,
		Float2,
		Float3,
		Float4,
		Matrix3x4,
		Matrix4x4,
		Count
	};

	/// Format for VertexElement
	enum class VertexFormat : uint8_t
	{
		Float,
		Float2,
		Float3,
		Float4,
		Byte4,
		Byte4N,
		UByte4,
		UByte4N,
		Short2,
		Short2N,
		Short4,
		Short4N,
		Count
	};

	enum class VertexInputRate : uint8_t
	{
		Vertex,
		Instance
	};

	/// Element semantics for vertex elements.
	class ALIMER_API VertexElementSemantic
	{
	public:
		static constexpr const char* POSITION = "POSITION";
		static constexpr const char* NORMAL = "NORMAL";
		static constexpr const char* BINORMAL = "BINORMAL";
		static constexpr const char* TANGENT = "TANGENT";
		static constexpr const char* TEXCOORD = "TEXCOORD";
		static constexpr const char* COLOR = "COLOR";
		static constexpr const char* BLENDWEIGHT = "BLENDWEIGHT";
		static constexpr const char* BLENDINDICES = "BLENDINDICES";
	};

	/// Primitive types.
	enum PrimitiveType
	{
		POINT_LIST = 1,
		LINE_LIST,
		LINE_STRIP,
		TRIANGLE_LIST,
		TRIANGLE_STRIP,
		MAX_PRIMITIVE_TYPES
	};

	/// Blend factors.
	enum BlendFactor
	{
		BLEND_ZERO = 1,
		BLEND_ONE,
		BLEND_SRC_COLOR,
		BLEND_INV_SRC_COLOR,
		BLEND_SRC_ALPHA,
		BLEND_INV_SRC_ALPHA,
		BLEND_DEST_ALPHA,
		BLEND_INV_DEST_ALPHA,
		BLEND_DEST_COLOR,
		BLEND_INV_DEST_COLOR,
		BLEND_SRC_ALPHA_SAT,
		MAX_BLEND_FACTORS
	};

	/// Blend operations.
	enum BlendOp
	{
		BLEND_OP_ADD = 1,
		BLEND_OP_SUBTRACT,
		BLEND_OP_REV_SUBTRACT,
		BLEND_OP_MIN,
		BLEND_OP_MAX,
		MAX_BLEND_OPS
	};

	/// Predefined blend modes.
	enum BlendMode
	{
		BLEND_MODE_REPLACE = 0,
		BLEND_MODE_ADD,
		BLEND_MODE_MULTIPLY,
		BLEND_MODE_ALPHA,
		BLEND_MODE_ADDALPHA,
		BLEND_MODE_PREMULALPHA,
		BLEND_MODE_INVDESTALPHA,
		BLEND_MODE_SUBTRACT,
		BLEND_MODE_SUBTRACTALPHA,
		MAX_BLEND_MODES
	};

	/// Fill modes.
	enum FillMode
	{
		FILL_WIREFRAME = 2,
		FILL_SOLID = 3,
		MAX_FILL_MODES
	};

	/// Triangle culling modes.
	enum CullMode
	{
		CULL_NONE = 1,
		CULL_FRONT,
		CULL_BACK,
		MAX_CULL_MODES
	};

	/// Depth or stencil compare modes.
	enum CompareFunc
	{
		CMP_NEVER = 1,
		CMP_LESS,
		CMP_EQUAL,
		CMP_LESS_EQUAL,
		CMP_GREATER,
		CMP_NOT_EQUAL,
		CMP_GREATER_EQUAL,
		CMP_ALWAYS,
		MAX_COMPARE_MODES
	};

	/// Stencil operations.
	enum StencilOp
	{
		STENCIL_OP_KEEP = 1,
		STENCIL_OP_ZERO,
		STENCIL_OP_REPLACE,
		STENCIL_OP_INCR_SAT,
		STENCIL_OP_DECR_SAT,
		STENCIL_OP_INVERT,
		STENCIL_OP_INCR,
		STENCIL_OP_DECR,
		MAX_STENCIL_OPS
	};

	/// Resource usage modes. Rendertarget usage can only be used with textures.
	enum class ResourceUsage : uint8_t
	{
		Default = 0,
		Immutable,
		Dynamic
	};

	/// Texture filtering modes.
	enum TextureFilterMode
	{
		FILTER_POINT = 0,
		FILTER_BILINEAR,
		FILTER_TRILINEAR,
		FILTER_ANISOTROPIC,
		COMPARE_POINT,
		COMPARE_BILINEAR,
		COMPARE_TRILINEAR,
		COMPARE_ANISOTROPIC,
		Count
	};

	/// TextureSampler addressing modes.
	enum class SamplerAddressMode : uint32_t
	{
		Wrap = 0,
		Mirror,
		Clamp,
		Border,
		MirrorOnce,
		Count
	};

	enum class BufferUsageBits : uint32_t
	{
		None = 0,
		Vertex = 0x1,
		Index = 0x2,
		Uniform = 0x4,
		Storage = 0x8,
		Indirect = 0x10
	};
	using BufferUsage = Flags<BufferUsageBits>;
	ALIMER_FORCE_INLINE BufferUsage operator|(BufferUsageBits bit0, BufferUsageBits bit1)
	{
		return BufferUsage(bit0) | bit1;
	}

	ALIMER_FORCE_INLINE BufferUsage operator~(BufferUsageBits bits)
	{
		return ~(BufferUsage(bits));
	}

	/// Texture types.
	enum class IndexType : uint8_t
	{
		UInt16 = 0,
		UInt32
	};

	/// Description of an element in a vertex declaration.
	struct ALIMER_API VertexElement
	{
		/// Default-construct.
		VertexElement()
			: semanticName(VertexElementSemantic::POSITION)
			, semanticIndex(0)
			, format(VertexFormat::Float3)
			, offset(0)
		{
		}

		/// Construct with type, semantic, index and whether is per-instance data.
		VertexElement(
			VertexFormat format_,
			const char* semanticName_,
			uint32_t semanticIndex_ = 0,
			uint32_t offset_ = 0)
			: format(format_)
			, semanticName(semanticName_)
			, semanticIndex(semanticIndex_)
			, offset(offset_)
		{
		}

		/// Semantic of element.
		const char* semanticName;
		/// Semantic index of element, for example multi-texcoords.
		uint32_t semanticIndex;
		/// Format of element.
		VertexFormat format;
		/// Offset of element from vertex start.
		uint32_t offset;
	};

	/// Description of a shader constant.
	struct ALIMER_API Constant
	{
		/// Construct empty.
		Constant() :
			numElements(1)
		{
		}

		/// Construct with type, name and optional number of elements.
		Constant(ConstantElementType type_, const std::string& name_, uint32_t numElements_ = 1)
			: type(type_)
			, name(name_)
			, numElements(numElements_)
		{
		}

		/// Construct with type, name and optional number of elements.
		Constant(ConstantElementType type_, const char* name_, uint32_t numElements_ = 1)
			: type(type_)
			, name(name_)
			, numElements(numElements_)
		{
		}

		/// Data type of constant.
		ConstantElementType type;
		/// Name of constant.
		std::string name;
		/// Number of elements. Default 1.
		uint32_t numElements;
		/// Element size. Filled by ConstantBuffer.
		uint32_t elementSize;
		/// Offset from the beginning of the buffer. Filled by ConstantBuffer.
		uint32_t offset;
	};

	/// Description of a blend mode.
	struct ALIMER_API BlendModeDesc
	{
		/// Default-construct.
		BlendModeDesc()
		{
			Reset();
		}

		/// Construct with parameters.
		BlendModeDesc(bool blendEnable_, BlendFactor srcBlend_, BlendFactor destBlend_, BlendOp blendOp_, BlendFactor srcBlendAlpha_, BlendFactor destBlendAlpha_, BlendOp blendOpAlpha_) :
			blendEnable(blendEnable_),
			srcBlend(srcBlend_),
			destBlend(destBlend_),
			blendOp(blendOp_),
			srcBlendAlpha(srcBlendAlpha_),
			destBlendAlpha(destBlendAlpha_),
			blendOpAlpha(blendOpAlpha_)
		{
		}

		/// Reset to defaults.
		void Reset()
		{
			blendEnable = false;
			srcBlend = BLEND_ONE;
			destBlend = BLEND_ONE;
			blendOp = BLEND_OP_ADD;
			srcBlendAlpha = BLEND_ONE;
			destBlendAlpha = BLEND_ONE;
			blendOpAlpha = BLEND_OP_ADD;
		}

		/// Test for equality with another blend mode description.
		bool operator == (const BlendModeDesc& rhs) const { return blendEnable == rhs.blendEnable && srcBlend == rhs.srcBlend && destBlend == rhs.destBlend && blendOp == rhs.blendOp && srcBlendAlpha == rhs.srcBlendAlpha && destBlendAlpha == rhs.destBlendAlpha && blendOpAlpha == rhs.blendOpAlpha; }
		/// Test for inequality with another blend mode description.
		bool operator != (const BlendModeDesc& rhs) const { return !(*this == rhs); }

		/// Blend enable flag.
		bool blendEnable;
		/// Source color blend factor.
		BlendFactor srcBlend;
		/// Destination color blend factor.
		BlendFactor destBlend;
		/// Color blend operation.
		BlendOp blendOp;
		/// Source alpha blend factor.
		BlendFactor srcBlendAlpha;
		/// Destination alpha blend factor.
		BlendFactor destBlendAlpha;
		/// Alpha blend operation.
		BlendOp blendOpAlpha;
	};

	/// Description of a stencil test.
	struct ALIMER_API StencilTestDesc
	{
		/// Default-construct.
		StencilTestDesc()
		{
			Reset();
		}

		/// Reset to defaults.
		void Reset()
		{
			stencilReadMask = 0xff;
			stencilWriteMask = 0xff;
			frontFunc = CMP_ALWAYS;
			frontFail = STENCIL_OP_KEEP;
			frontDepthFail = STENCIL_OP_KEEP;
			frontPass = STENCIL_OP_KEEP;
			backFunc = CMP_ALWAYS;
			backFail = STENCIL_OP_KEEP;
			backDepthFail = STENCIL_OP_KEEP;
			backPass = STENCIL_OP_KEEP;
		}

		/// Stencil read bit mask.
		unsigned char stencilReadMask;
		/// Stencil write bit mask.
		unsigned char stencilWriteMask;
		/// Stencil front face compare function.
		CompareFunc frontFunc;
		/// Operation for front face stencil test fail.
		StencilOp frontFail;
		/// Operation for front face depth test fail.
		StencilOp frontDepthFail;
		/// Operation for front face pass.
		StencilOp frontPass;
		/// Stencil back face compare function.
		CompareFunc backFunc;
		/// Operation for back face stencil test fail.
		StencilOp backFail;
		/// Operation for back face depth test fail.
		StencilOp backDepthFail;
		/// Operation for back face pass.
		StencilOp backPass;
	};

	/// Collection of render state.
	struct RenderState
	{
		/// Default-construct.
		RenderState()
		{
			Reset();
		}

		/// Reset to defaults.
		void Reset()
		{
			depthFunc = CMP_LESS_EQUAL;
			depthWrite = true;
			depthClip = true;
			depthBias = 0;
			slopeScaledDepthBias = 0.0f;
			colorWriteMask = COLORMASK_ALL;
			alphaToCoverage = false;
			blendMode.Reset();
			cullMode = CULL_BACK;
			fillMode = FILL_SOLID;
			scissorEnable = false;
			scissorRect = IntRect::ZERO;
			stencilEnable = false;
			stencilRef = 0;
			stencilTest.Reset();
		}

		/// Depth test function.
		CompareFunc depthFunc;
		/// Depth write enable.
		bool depthWrite;
		/// Depth clipping enable.
		bool depthClip;
		/// Constant depth bias.
		int depthBias;
		/// Slope-scaled depth bias.
		float slopeScaledDepthBias;
		/// Rendertarget color channel write mask.
		unsigned char colorWriteMask;
		/// Alpha-to-coverage enable.
		bool alphaToCoverage;
		/// Blend mode parameters.
		BlendModeDesc blendMode;
		/// Polygon culling mode.
		CullMode cullMode;
		/// Polygon fill mode.
		FillMode fillMode;
		/// Scissor test enable.
		bool scissorEnable;
		/// Scissor rectangle as pixels from rendertarget top left corner.
		IntRect scissorRect;
		/// Stencil test enable.
		bool stencilEnable;
		/// Stencil reference value.
		unsigned char stencilRef;
		/// Stencil test parameters.
		StencilTestDesc stencilTest;
	};

	ALIMER_API uint32_t GetVertexFormatSize(VertexFormat format);
	ALIMER_API uint32_t GetConstantElementSize(ConstantElementType type);

	/// Resource usage names.
	extern ALIMER_API const char* resourceUsageNames[];
	/// Blend factor names.
	extern ALIMER_API const char* blendFactorNames[];
	/// Blend operation names.
	extern ALIMER_API const char* blendOpNames[];
	/// Predefined blend mode names.
	extern ALIMER_API const char* blendModeNames[];
	/// Fill mode names.
	extern ALIMER_API const char* fillModeNames[];
	/// Culling mode names.
	extern ALIMER_API const char* cullModeNames[];
	/// Compare function names.
	extern ALIMER_API const char* compareFuncNames[];
	/// Stencil operation names.
	extern ALIMER_API const char* stencilOpNames[];
	/// Predefined blend modes.
	extern ALIMER_API const BlendModeDesc blendModes[];

}
