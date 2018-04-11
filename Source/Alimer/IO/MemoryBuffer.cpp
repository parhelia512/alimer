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

#include "../Base/Vector.h"
#include "MemoryBuffer.h"

#include <cstring>

namespace Alimer
{
	MemoryBuffer::MemoryBuffer(void* data, size_t numBytes)
		: Stream(data ? numBytes : 0)
		, _buffer((uint8_t*)data)
		, _readOnly(false)
	{
		SetName("Memory");
	}

	MemoryBuffer::MemoryBuffer(const void* data, size_t numBytes)
		: Stream(data ? numBytes : 0)
		, _buffer((uint8_t*)data)
		, _readOnly(true)
	{
		SetName("Memory");
	}

	MemoryBuffer::MemoryBuffer(Vector<uint8_t>& data)
		: Stream(data.Size())
		, _buffer(data.Begin().ptr)
		, _readOnly(false)
	{
	}

	MemoryBuffer::MemoryBuffer(const Vector<unsigned char>& data)
		: Stream(data.Size())
		, _buffer(data.Begin().ptr)
		, _readOnly(true)
	{
	}

	size_t MemoryBuffer::Read(void* dest, size_t numBytes)
	{
		if (numBytes + position > size)
			numBytes = size - position;
		if (!numBytes)
			return 0;

		uint8_t* srcPtr = &_buffer[position];
		uint8_t* destPtr = (uint8_t*)dest;
		position += numBytes;

		size_t copySize = numBytes;
		while (copySize >= sizeof(unsigned))
		{
			*((unsigned*)destPtr) = *((unsigned*)srcPtr);
			srcPtr += sizeof(unsigned);
			destPtr += sizeof(unsigned);
			copySize -= sizeof(unsigned);
		}
		if (copySize & sizeof(unsigned short))
		{
			*((unsigned short*)destPtr) = *((unsigned short*)srcPtr);
			srcPtr += sizeof(unsigned short);
			destPtr += sizeof(unsigned short);
		}
		if (copySize & 1)
			*destPtr = *srcPtr;

		return numBytes;
	}

	size_t MemoryBuffer::Seek(size_t newPosition)
	{
		if (newPosition > size)
			newPosition = size;

		position = newPosition;
		return position;
	}

	size_t MemoryBuffer::Write(const void* data, size_t numBytes)
	{
		if (numBytes + position > size)
			numBytes = size - position;
		if (!numBytes || _readOnly)
			return 0;

		uint8_t* srcPtr = (uint8_t*)data;
		uint8_t* destPtr = &_buffer[position];
		position += numBytes;

		size_t copySize = numBytes;
		while (copySize >= sizeof(unsigned))
		{
			*((unsigned*)destPtr) = *((unsigned*)srcPtr);
			srcPtr += sizeof(unsigned);
			destPtr += sizeof(unsigned);
			copySize -= sizeof(unsigned);
		}
		if (copySize & sizeof(unsigned short))
		{
			*((unsigned short*)destPtr) = *((unsigned short*)srcPtr);
			srcPtr += sizeof(unsigned short);
			destPtr += sizeof(unsigned short);
		}
		if (copySize & 1)
			*destPtr = *srcPtr;

		return numBytes;
	}

	bool MemoryBuffer::IsReadable() const
	{
		return _buffer != nullptr;
	}

	bool MemoryBuffer::IsWritable() const
	{
		return _buffer && !_readOnly;
	}
}
