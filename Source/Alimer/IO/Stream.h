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
#include "../json/json.hpp"

namespace Alimer
{
	class StringHash;
	struct ObjectRef;
	struct ResourceRef;
	struct ResourceRefList;

	/// Abstract stream for reading and writing.
	class ALIMER_API Stream
	{
	public:
		/// Default-construct with zero size.
		Stream() = default;
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
		void SetName(const std::string& newName);

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
		std::string ReadString();

		/// Read a text line.
		std::string ReadLine();
		/// Read a 4-character file ID.
		std::string ReadFileID();
		/// Read a 32-bit StringHash.
		StringHash ReadStringHash();
		/// Read a byte buffer, with size prepended as a VLE value.
		std::vector<uint8_t> ReadBuffer();

		/// Read a quaternion.
		Quaternion ReadQuaternion();

		/// Read a binary json value.
		nlohmann::json ReadJSON();

		/// Write an 8-bit integer.
		void WriteByte(int8_t value);
		/// Write a 16-bit integer.
		void WriteShort(int16_t value);
		/// Write a 32-bit integer.
		void WriteInt(int32_t value);
		/// Write a 64-bit integer.
		void WriteInt64(int64_t value);
		/// Write an 8-bit unsigned integer.
		void WriteUByte(uint8_t value);
		/// Write a 16-bit unsigned integer.
		void WriteUShort(uint16_t value);
		/// Write a 32-bit unsigned integer.
		void WriteUInt(uint32_t value);
		/// Write a 64-bit unsigned integer.
		void WriteUInt64(uint64_t value);
		/// Write a bool.
		void WriteBool(bool value);
		/// Write a float.
		void WriteFloat(float value);
		/// Write a double.
		void WriteDouble(double value);
		/// Write a null-terminated string.
		void WriteString(const String& value);
		/// Write a 32-bit StringHash.
		void WriteStringHash(const StringHash& value);
		/// Write a four-letter file ID. If the string is not long enough, spaces will be appended.
		void WriteFileID(const std::string& value);
		/// Write a byte buffer, with size encoded as VLE.
		void WriteBuffer(const std::vector<uint8_t>& buffer);
		/// Write a variable-length encoded unsigned integer, which can use 29 bits maximum.
		void WriteVLE(uint32_t value);
		/// Write a text line. Char codes 13 & 10 will be automatically appended.
		void WriteLine(const std::string& value);
		/// Write a resource reference.
		void Write(const ResourceRef& value);
		/// Write a resource reference list.
		void Write(const ResourceRefList& value);
		/// Write a object reference.
		void Write(const ObjectRef& value);
		/// Write a json value in binary format.
		void Write(const nlohmann::json& value);

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
		const std::string& GetName() const { return _name; }
		/// Return current position in bytes.
		size_t Position() const { return _position; }
		/// Return size in bytes.
		size_t Size() const { return _size; }
		/// Return whether the end of stream has been reached.
		bool IsEof() const { return _position >= _size; }

	protected:
		/// Stream position.
		size_t _position{};
		/// Stream size.
		size_t _size{};
		/// Stream name.
		std::string _name;
	};

	template<> ALIMER_API bool Stream::Read();
	template<> ALIMER_API std::string Stream::Read();
	template<> ALIMER_API StringHash Stream::Read();
	template<> ALIMER_API ResourceRef Stream::Read();
	template<> ALIMER_API ResourceRefList Stream::Read();
	template<> ALIMER_API ObjectRef Stream::Read();
	template<> ALIMER_API nlohmann::json Stream::Read();

	template<> ALIMER_API void Stream::Write(const bool& value);
	template<> ALIMER_API void Stream::Write(const std::string& value);
	template<> ALIMER_API void Stream::Write(const StringHash& value);
	template<> ALIMER_API void Stream::Write(const ResourceRef& value);
	template<> ALIMER_API void Stream::Write(const ResourceRefList& value);
	template<> ALIMER_API void Stream::Write(const ObjectRef& value);
	template<> ALIMER_API void Stream::Write(const nlohmann::json& value);
}
