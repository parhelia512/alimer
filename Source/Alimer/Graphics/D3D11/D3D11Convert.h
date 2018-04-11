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

#include "D3D11Prerequisites.h"
#include "../GraphicsDefs.h"

namespace Alimer
{
	namespace d3d11
	{
		static inline D3D11_USAGE Convert(ResourceUsage usage)
		{
			switch (usage)
			{
			case USAGE_DEFAULT: return D3D11_USAGE_DEFAULT;
			case USAGE_IMMUTABLE: return D3D11_USAGE_IMMUTABLE;
			case USAGE_DYNAMIC: return D3D11_USAGE_DYNAMIC;
			default:
				return D3D11_USAGE_DEFAULT;
			}
		}
		
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
	}
}