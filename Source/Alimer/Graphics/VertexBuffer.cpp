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
	VertexBuffer::VertexBuffer() 
		: Buffer(BufferUsage::Vertex)
	{
	}

	VertexBuffer::~VertexBuffer()
	{
		Release();
	}

	bool VertexBuffer::Define(
		ResourceUsage usage, 
		uint32_t vertexCount,
		const std::vector<VertexElement>& elements,
		bool useShadowData, 
		const void* data)
	{
		if (!vertexCount || !elements.size())
		{
			LOGERROR("Can not define vertex buffer with no vertices or no elements");
			return false;
		}

		return Define(
			usage, 
			vertexCount, 
			static_cast<uint32_t>(elements.size()),
			elements.data(), 
			useShadowData, data);
	}

	bool VertexBuffer::Define(
		ResourceUsage usage,
		uint32_t vertexCount,
		uint32_t numElements,
		const VertexElement* elements,
		bool useShadowData, 
		const void* data)
	{
		ALIMER_PROFILE(DefineVertexBuffer);

		if (!vertexCount || !numElements || !elements)
		{
			LOGERROR("Can not define vertex buffer with no vertices or no elements");
			return false;
		}

		if (usage == ResourceUsage::Immutable && !data)
		{
			LOGERROR("Immutable vertex buffer must define initial data");
			return false;
		}

		for (size_t i = 0; i < numElements; ++i)
		{
			if (elements[i].type >= ELEM_MATRIX3X4)
			{
				LOGERROR("Matrix elements are not supported in vertex buffers");
				return false;
			}
		}

		Release();
		_vertexCount = vertexCount;
		_resourceUsage = usage;

		// Determine offset of elements and the vertex size & element hash
		_stride = 0;
		_elementHash = 0;
		_elements.resize(numElements);
		for (uint32_t i = 0; i < numElements; ++i)
		{
			_elements[i] = elements[i];
			_elements[i].offset = _stride;
			_stride += elementSizes[elements[i].type];
			_elementHash |= ElementHash(i, elements[i].semantic);
		}

		_size = _stride * _vertexCount;
		return Buffer::Create(useShadowData, data);
	}

	bool VertexBuffer::SetData(
		uint32_t firstVertex, 
		uint32_t vertexCount,
		const void* data)
	{
		ALIMER_PROFILE(UpdateVertexBuffer);

		if (!data)
		{
			LOGERROR("Null source data for updating vertex buffer");
			return false;
		}

		if (firstVertex + vertexCount > GetVertexCount())
		{
			LOGERROR("Out of bounds range for updating vertex buffer");
			return false;
		}

		if (_handle && _resourceUsage == ResourceUsage::Immutable)
		{
			LOGERROR("Can not update immutable vertex buffer");
			return false;
		}

		return Buffer::SetData(
			firstVertex * _stride, 
			vertexCount * _stride, 
			data);
	}
}
