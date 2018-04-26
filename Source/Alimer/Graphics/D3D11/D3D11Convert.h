//
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

#include "D3D11Prerequisites.h"
#include "../GraphicsDefs.h"
#include "../Texture.h"

namespace Alimer
{
	namespace d3d11
	{
		static inline UINT Convert(BufferUsage usage)
		{
			if (any(usage & BufferUsage::Uniform))
				return D3D11_BIND_CONSTANT_BUFFER;

			UINT d3dUsage = 0;
			if (any(usage & BufferUsage::Vertex))
				d3dUsage |= D3D11_BIND_VERTEX_BUFFER;

			if (any(usage & BufferUsage::Index))
				d3dUsage |= D3D11_BIND_INDEX_BUFFER;

			if (any(usage & BufferUsage::Storage))
				d3dUsage |= D3D11_BIND_UNORDERED_ACCESS;

			return d3dUsage;
		}

		static inline DXGI_FORMAT Convert(IndexType type)
		{
			switch (type)
			{
			case IndexType::UInt16: return DXGI_FORMAT_R16_UINT;
			case IndexType::UInt32: return DXGI_FORMAT_R32_UINT;
			default: return DXGI_FORMAT_UNKNOWN;
			}
		}

		static inline DXGI_FORMAT Convert(VertexFormat format)
		{
			switch (format)
			{
			case VertexFormat::Float:
				return DXGI_FORMAT_R32_FLOAT;
			case VertexFormat::Float2:
				return DXGI_FORMAT_R32G32_FLOAT;
			case VertexFormat::Float3:
				return DXGI_FORMAT_R32G32B32_FLOAT;
			case VertexFormat::Float4:
				return DXGI_FORMAT_R32G32B32A32_FLOAT;
			case VertexFormat::Byte4:
				return DXGI_FORMAT_R8G8B8A8_SINT;
			case VertexFormat::Byte4N:
				return DXGI_FORMAT_R8G8B8A8_SNORM;
			case VertexFormat::UByte4:
				return DXGI_FORMAT_R8G8B8A8_UINT;
			case VertexFormat::UByte4N:
				return DXGI_FORMAT_R8G8B8A8_UNORM;
			case VertexFormat::Short2:
				return DXGI_FORMAT_R16G16_SINT;
			case VertexFormat::Short2N:
				return DXGI_FORMAT_R16G16_SNORM;
			case VertexFormat::Short4:
				return DXGI_FORMAT_R16G16B16A16_SINT;
			case VertexFormat::Short4N:
				return DXGI_FORMAT_R16G16B16A16_SNORM;
			default:
				return DXGI_FORMAT_UNKNOWN;
			}
		}
	}
}