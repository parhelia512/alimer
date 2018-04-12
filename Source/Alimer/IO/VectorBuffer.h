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

	/// Dynamically sized buffer that can be read and written to as a stream.
	class ALIMER_API VectorBuffer final : public Stream
	{
	public:
		/// Construct an empty buffer.
		VectorBuffer();
		/// Construct from another buffer.
		VectorBuffer(const std::vector<uint8_t>& data);
		/// Construct from a memory area.
		VectorBuffer(const void* data, size_t numBytes);
		/// Construct from a stream.
		VectorBuffer(Stream& source, size_t numBytes);
		/// Destructor, calls Clear.
		~VectorBuffer() override;

		/// Read bytes from the buffer. Return number of bytes actually read.
		size_t Read(void* dest, size_t size) override;
		/// Set position in bytes from the beginning of the buffer.
		size_t Seek(size_t newPosition) override;
		/// Write bytes to the buffer. Return number of bytes actually written.
		size_t Write(const void* data, size_t size) override;
		/// Return whether read operations are allowed.
		bool IsReadable() const override;
		/// Return whether write operations are allowed.
		bool IsWritable() const override;

		/// Set data from another buffer.
		void SetData(const std::vector<uint8_t>& data);
		/// Set data from a memory area.
		void SetData(const void* data, size_t numBytes);
		/// Set data from a stream.
		void SetData(Stream& source, size_t numBytes);
		/// Reset to zero size.
		void Clear();
		/// Set size.
		void Resize(size_t newSize);

		/// Return data.
		const uint8_t* Data() const { return _buffer.data(); }
		/// Return non-const data.
		uint8_t* ModifiableData() { return _buffer.data(); }
		/// Return the buffer.
		const std::vector<uint8_t >& Buffer() const { return _buffer; }

		using Stream::Read;
		using Stream::Write;

	private:
		/// Dynamic data buffer.
		std::vector<uint8_t> _buffer;
	};

}
