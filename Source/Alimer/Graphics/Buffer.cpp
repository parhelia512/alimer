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
#include "Buffer.h"
#include "Graphics.h"

namespace Alimer
{
	Buffer::Buffer(BufferUsage usage)
		: _usage(usage)
	{

	}

	Buffer::~Buffer()
	{
		Release();
	}

	void Buffer::Release()
	{
#ifdef ALIMER_D3D11
		graphics->DestroyBuffer(_handle.ptr);
		_handle.ptr = nullptr;
#endif
	}

	static const char* BufferUsageToString(BufferUsage usage)
	{
		switch (usage)
		{
		case BufferUsage::Vertex:	return "vertex";
		case BufferUsage::Index:	return "index";
		case BufferUsage::Uniform:	return "uniform";
		default: return "unknown";
		}
	}

	bool Buffer::Create(bool useShadowData, const void* initialData)
	{
		if (!graphics || !graphics->IsInitialized())
			return false;

		// If buffer is reinitialized with the same shadow data, no need to reallocate
		if (useShadowData && (!initialData || initialData != _shadowData.get()))
		{
			_shadowData.reset(new uint8_t[_size]);
			if (initialData)
				memcpy(_shadowData.get(), initialData, _size);
		}

		_handle.ptr = graphics->CreateBuffer(_usage, _size, _stride, _resourceUsage, initialData);
		if (_handle.ptr == nullptr)
		{
			LOGERRORF("Failed to create %s buffer", BufferUsageToString(_usage));
			return false;
		}

		LOGDEBUGF("Created %s buffer [size: %u, stride %u]",
			BufferUsageToString(_usage),
			_size,
			_stride);
		return true;
	}
}
