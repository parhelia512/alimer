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

#include "VectorBuffer.h"

namespace Alimer
{

	VectorBuffer::VectorBuffer()
	{
		SetName("Vector");
	}

	VectorBuffer::VectorBuffer(const std::vector<uint8_t>& data)
	{
		SetData(data);
		SetName("Vector");
	}

	VectorBuffer::VectorBuffer(const void* data, size_t numBytes)
	{
		SetData(data, numBytes);
		SetName("Vector");
	}

	VectorBuffer::VectorBuffer(Stream& source, size_t numBytes)
	{
		SetData(source, numBytes);
		SetName("Vector");
	}

	VectorBuffer::~VectorBuffer()
	{
		Clear();
	}

	size_t VectorBuffer::Read(void* dest, size_t numBytes)
	{
		if (numBytes + _position > _size)
			numBytes = _size - _position;
		if (!numBytes)
			return 0;

		uint8_t* srcPtr = &_buffer[_position];
		uint8_t* destPtr = (uint8_t*)dest;
		_position += numBytes;

		size_t copySize = numBytes;
		while (copySize >= sizeof(unsigned))
		{
			*((unsigned*)destPtr) = *((unsigned*)srcPtr);
			srcPtr += sizeof(unsigned);
			destPtr += sizeof(unsigned);
			copySize -= sizeof(unsigned);
		}
		if (copySize & sizeof(uint16_t))
		{
			*((uint16_t*)destPtr) = *((uint16_t*)srcPtr);
			srcPtr += sizeof(uint16_t);
			destPtr += sizeof(uint16_t);
		}
		if (copySize & 1)
			*destPtr = *srcPtr;

		return _size;
	}

	size_t VectorBuffer::Seek(size_t newPosition)
	{
		if (newPosition > _size)
			newPosition = _size;

		_position = newPosition;
		return _position;
	}

	size_t VectorBuffer::Write(const void* data, size_t numBytes)
	{
		if (!numBytes)
			return 0;

		// Expand the buffer if necessary
		if (numBytes + _position > _size)
		{
			_size = _position + numBytes;
			_buffer.resize(_size);
		}

		uint8_t* srcPtr = (uint8_t*)data;
		uint8_t* destPtr = &_buffer[_position];
		_position += numBytes;

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

	bool VectorBuffer::IsReadable() const
	{
		return true;
	}

	bool VectorBuffer::IsWritable() const
	{
		return true;
	}

	void VectorBuffer::SetData(const std::vector<uint8_t>& data)
	{
		_buffer = data;
		_position = 0;
		_size = data.size();
	}

	void VectorBuffer::SetData(const void* data, size_t numBytes)
	{
		if (!data)
			numBytes = 0;

		_buffer.resize(numBytes);
		if (numBytes)
			memcpy(_buffer.data(), data, numBytes);

		_position = 0;
		_size = numBytes;
	}

	void VectorBuffer::SetData(Stream& source, size_t numBytes)
	{
		_buffer.resize(numBytes);
		size_t actualSize = source.Read(_buffer.data(), numBytes);
		if (actualSize != numBytes)
			_buffer.resize(actualSize);

		_position = 0;
		_size = actualSize;
	}

	void VectorBuffer::Clear()
	{
		_buffer.clear();
		_position = 0;
		_size = 0;
	}

	void VectorBuffer::Resize(size_t newSize)
	{
		_buffer.resize(newSize);
		_size = newSize;
		if (_position > _size)
			_position = _size;
	}
}
