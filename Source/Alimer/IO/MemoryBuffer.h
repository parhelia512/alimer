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

#include "Stream.h"
#include <vector>

namespace Alimer
{
	/// Memory area that can be read and written to as a stream.
	class ALIMER_API MemoryBuffer : public Stream
	{
	public:
		/// Construct with a pointer and size.
		MemoryBuffer(void* data, size_t numBytes);
		/// Construct as read-only with a pointer and size.
		MemoryBuffer(const void* data, size_t numBytes);
		/// Construct from a vector, which must not go out of scope before MemoryBuffer.
		MemoryBuffer(std::vector<uint8_t>& data);
		/// Construct from a read-only vector, which must not go out of scope before MemoryBuffer.
		MemoryBuffer(const std::vector<uint8_t>& data);

		/// Read bytes from the memory area. Return number of bytes actually read.
		size_t Read(void* dest, size_t numBytes) override;
		/// Set position in bytes from the beginning of the memory area.
		size_t Seek(size_t newPosition) override;
		/// Write bytes to the memory area.
		size_t Write(const void* data, size_t numBytes) override;
		/// Return whether read operations are allowed.
		bool IsReadable() const override;
		/// Return whether write operations are allowed.
		bool IsWritable() const override;

		/// Return memory area.
		uint8_t* Data() { return _buffer; }

		using Stream::Read;
		using Stream::Write;

	private:
		/// Pointer to the memory area.
		uint8_t* _buffer;
		/// Read-only flag.
		bool _readOnly;
	};

}
