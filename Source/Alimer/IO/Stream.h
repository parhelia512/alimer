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

#include "../Base/String.h"
#include "../Math/Quaternion.h"

namespace Alimer
{
	class JSONValue;
	class StringHash;
	struct ObjectRef;
	struct ResourceRef;
	struct ResourceRefList;

	/// Abstract stream for reading and writing.
	class ALIMER_API Stream
	{
	public:
		/// Default-construct with zero size.
		Stream();
		/// Construct with defined byte size.
		Stream(size_t numBytes);
		/// Destruct.
		virtual ~Stream();

		/// Read bytes from the stream. Return number of bytes actually read.
		virtual size_t Read(void* dest, size_t numBytes) = 0;
		/// Set position in bytes from the beginning of the stream. Return the position after the seek.
		virtual size_t Seek(size_t position) = 0;
		/// Write bytes to the stream. Return number of bytes actually written.
		virtual size_t Write(const void* data, size_t size) = 0;
		/// Return whether read operations are allowed.
		virtual bool IsReadable() const = 0;
		/// Return whether write operations are allowed.
		virtual bool IsWritable() const = 0;

		/// Change the stream name.
		void SetName(const String& newName);
		/// Change the stream name.
		void SetName(const char* newName);

		/// Read a 8-bit signed integer.
		int8_t ReadByte();
		/// Read a 16-bit integer.
		int16_t ReadShort();
		/// Read a 32-bit integer.
		int32_t ReadInt();
		/// Read a 64-bit integer.
		int64_t ReadInt64();
		/// Read a 8-bit unsigned integer.
		uint8_t ReadUByte();
		/// Read a 16-bit unsigned integer.
		uint16_t ReadUShort();
		/// Read a 32-bit unsigned integer.
		uint32_t ReadUInt();
		/// Read a 64-bit unsigned integer.
		uint64_t ReadUInt64();
		/// Read a boolean.
		bool ReadBool();
		/// Read a float.
		float ReadFloat();
		/// Read a double.
		double ReadDouble();

		/// Read a variable-length encoded unsigned integer, which can use 29 bits maximum.
		uint32_t ReadVLE();

		/// Read null terminated string.
		String ReadString();

		/// Read a text line.
		String ReadLine();
		/// Read a 4-character file ID.
		std::string ReadFileID();
		/// Read a byte buffer, with size prepended as a VLE value.
		std::vector<uint8_t> ReadBuffer();

		/// Read a quaternion.
		Quaternion ReadQuaternion();

		/// Write a four-letter file ID. If the string is not long enough, spaces will be appended.
		void WriteFileID(const String& value);
		/// Write a byte buffer, with size encoded as VLE.
		void WriteBuffer(const std::vector<uint8_t>& buffer);
		/// Write a variable-length encoded unsigned integer, which can use 29 bits maximum.
		void WriteVLE(uint32_t value);
		/// Write a text line. Char codes 13 & 10 will be automatically appended.
		void WriteLine(const String& value);

		/// Write a value, template version.
		template <class T> void Write(const T& value) { Write(&value, sizeof value); }

		/// Read a value, template version.
		template <class T> T Read()
		{
			T ret;
			Read(&ret, sizeof ret);
			return ret;
		}

		/// Return the stream name.
		const String& Name() const { return name; }
		/// Return current position in bytes.
		size_t Position() const { return position; }
		/// Return size in bytes.
		size_t Size() const { return size; }
		/// Return whether the end of stream has been reached.
		bool IsEof() const { return position >= size; }

	protected:
		/// Stream position.
		size_t position;
		/// Stream size.
		size_t size;
		/// Stream name.
		String name;
	};

	template<> ALIMER_API bool Stream::Read();
	template<> ALIMER_API String Stream::Read();
	template<> ALIMER_API StringHash Stream::Read();
	template<> ALIMER_API ResourceRef Stream::Read();
	template<> ALIMER_API ResourceRefList Stream::Read();
	template<> ALIMER_API ObjectRef Stream::Read();
	template<> ALIMER_API JSONValue Stream::Read();
	template<> ALIMER_API void Stream::Write(const bool& value);
	template<> ALIMER_API void Stream::Write(const String& value);
	template<> ALIMER_API void Stream::Write(const StringHash& value);
	template<> ALIMER_API void Stream::Write(const ResourceRef& value);
	template<> ALIMER_API void Stream::Write(const ResourceRefList& value);
	template<> ALIMER_API void Stream::Write(const ObjectRef& value);
	template<> ALIMER_API void Stream::Write(const JSONValue& value);

}
