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
#include "GraphicsImpl.h"

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
		SafeDelete(_handle);
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

		_handle = graphics->CreateBuffer(_usage, _size, _stride, _resourceUsage, initialData);
		if (!_handle)
		{
			ALIMER_LOGERROR("Failed to create {} buffer", BufferUsageToString(_usage));
			return false;
		}

		ALIMER_LOGDEBUG(
			"Created {} buffer [size: {}, stride {}]",
			BufferUsageToString(_usage),
			_size,
			_stride);
		return true;
	}

	bool Buffer::SetData(uint32_t offset, uint32_t size, const void* data)
	{
		if (_shadowData)
		{
			memcpy(_shadowData.get() + offset, data, size);
		}

		return _handle->SetData(offset, size, data);
	}
}
