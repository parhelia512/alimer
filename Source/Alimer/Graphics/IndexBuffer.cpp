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
	IndexBuffer::~IndexBuffer()
	{
		Release();
	}

	bool IndexBuffer::Define(
		ResourceUsage usage,
		uint32_t numIndices, 
		uint32_t indexSize,
		bool useShadowData,
		const void* data)
	{
		ALIMER_PROFILE(DefineIndexBuffer);

		Release();

		if (!numIndices)
		{
			LOGERROR("Can not define index buffer with no indices");
			return false;
		}
		if (usage == USAGE_RENDERTARGET)
		{
			LOGERROR("Rendertarget usage is illegal for index buffers");
			return false;
		}
		if (usage == USAGE_IMMUTABLE && !data)
		{
			LOGERROR("Immutable index buffer must define initial data");
			return false;
		}
		if (indexSize != sizeof(uint32_t)
			&& indexSize != sizeof(uint16_t))
		{
			LOGERROR("Index buffer index size must be 2 or 4");
			return false;
		}

		_indexCount = numIndices;
		_indexSize = indexSize;
		_usage = usage;

		// If buffer is reinitialized with the same shadow data, no need to reallocate
		if (useShadowData && (!data || data != _shadowData.get()))
		{
			_shadowData.reset(new uint8_t[numIndices * indexSize]);
			if (data)
			{
				memcpy(_shadowData.get(), data, numIndices * indexSize);
			}
		}

		return Create(data);
	}

}
