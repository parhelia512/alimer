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
#include "IndexBuffer.h"

namespace Alimer
{
	IndexBuffer::IndexBuffer()
		: Buffer(BufferUsageBits::Index)
	{

	}

	IndexBuffer::~IndexBuffer()
	{
		Release();
	}

	bool IndexBuffer::Define(
		ResourceUsage usage,
		uint32_t indexCount, 
		IndexType indexType,
		bool useShadowData,
		const void* data)
	{
		ALIMER_PROFILE(DefineIndexBuffer);

		Release();

		if (!indexCount)
		{
			ALIMER_LOGERROR("Can not define index buffer with no indices");
			return false;
		}

		if (usage == ResourceUsage::Immutable && !data)
		{
			ALIMER_LOGERROR("Immutable index buffer must define initial data");
			return false;
		}

		if (indexType != IndexType::UInt16
			&& indexType != IndexType::UInt32)
		{
			ALIMER_LOGERROR("Index type must be UInt16 or UInt32");
			return false;
		}

		_indexCount = indexCount;
		_stride = indexType == IndexType::UInt16 ? 2 : 4;
		_size = _indexCount * _stride;
		_resourceUsage = usage;

		return Buffer::Create(useShadowData, data);
	}

	bool IndexBuffer::SetData(
		uint32_t firstIndex, 
		uint32_t indexCount, 
		const void* data)
	{
		ALIMER_PROFILE(UpdateIndexBuffer);

		if (!data)
		{
			ALIMER_LOGERROR("Null source data for updating index buffer");
			return false;
		}
		if (firstIndex + indexCount > GetIndexCount())
		{
			ALIMER_LOGERROR("Out of bounds range for updating index buffer");
			return false;
		}

		if (_handle && _resourceUsage == ResourceUsage::Immutable)
		{
			ALIMER_LOGERROR("Can not update immutable index buffer");
			return false;
		}

		return Buffer::SetData(
			firstIndex * _stride,
			indexCount * _stride,
			data);
	}
}
