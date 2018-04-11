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

#include "../Debug/Log.h"
#include "../Debug/Profiler.h"
#include "VertexBuffer.h"

namespace Alimer
{
	bool VertexBuffer::Define(ResourceUsage usage_, size_t numVertices_, const std::vector<VertexElement>& elements_, bool useShadowData, const void* data)
	{
		if (!numVertices_ || !elements_.size())
		{
			LOGERROR("Can not define vertex buffer with no vertices or no elements");
			return false;
		}

		return Define(usage_, numVertices_, elements_.size(), elements_.data(), useShadowData, data);
	}

	bool VertexBuffer::Define(ResourceUsage usage_, size_t numVertices_, size_t numElements_, const VertexElement* elements_, bool useShadowData, const void* data)
	{
		ALIMER_PROFILE(DefineVertexBuffer);

		if (!numVertices_ || !numElements_ || !elements_)
		{
			LOGERROR("Can not define vertex buffer with no vertices or no elements");
			return false;
		}
		if (usage_ == USAGE_RENDERTARGET)
		{
			LOGERROR("Rendertarget usage is illegal for vertex buffers");
			return false;
		}
		if (usage_ == USAGE_IMMUTABLE && !data)
		{
			LOGERROR("Immutable vertex buffer must define initial data");
			return false;
		}

		for (size_t i = 0; i < numElements_; ++i)
		{
			if (elements_[i].type >= ELEM_MATRIX3X4)
			{
				LOGERROR("Matrix elements are not supported in vertex buffers");
				return false;
			}
		}

		Release();

		numVertices = numVertices_;
		usage = usage_;

		// Determine offset of elements and the vertex size & element hash
		vertexSize = 0;
		elementHash = 0;
		elements.resize(numElements_);
		for (size_t i = 0; i < numElements_; ++i)
		{
			elements[i] = elements_[i];
			elements[i].offset = vertexSize;
			vertexSize += elementSizes[elements[i].type];
			elementHash |= ElementHash(i, elements[i].semantic);
		}

		// If buffer is reinitialized with the same shadow data, no need to reallocate
		if (useShadowData && (!data || data != _shadowData.get()))
		{
			_shadowData.reset(new uint8_t[numVertices * vertexSize]);
			if (data)
			{
				memcpy(_shadowData.get(), data, numVertices * vertexSize);
			}
		}

		return Create(data);
	}

}
