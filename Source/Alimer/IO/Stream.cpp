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

#include "../Math/Math.h"
#include "Stream.h"
#include "ObjectRef.h"
#include "ResourceRef.h"

namespace Alimer
{
	Stream::Stream(size_t numBytes)
		: _position(0)
		, _size(numBytes)
	{
	}

	Stream::~Stream()
	{
	}

	void Stream::SetName(const std::string& newName)
	{
		_name = newName;
	}

	int8_t Stream::ReadByte()
	{
		int8_t ret;
		Read(&ret, sizeof ret);
		return ret;
	}

	int16_t Stream::ReadShort()
	{
		int16_t ret;
		Read(&ret, sizeof ret);
		return ret;
	}

	int32_t Stream::ReadInt()
	{
		int32_t ret;
		Read(&ret, sizeof ret);
		return ret;
	}

	int64_t Stream::ReadInt64()
	{
		int64_t ret;
		Read(&ret, sizeof ret);
		return ret;
	}

	uint8_t Stream::ReadUByte()
	{
		uint8_t ret;
		Read(&ret, sizeof ret);
		return ret;
	}

	uint16_t Stream::ReadUShort()
	{
		uint16_t ret;
		Read(&ret, sizeof ret);
		return ret;
	}

	uint32_t Stream::ReadUInt()
	{
		uint32_t ret;
		Read(&ret, sizeof ret);
		return ret;
	}

	uint64_t Stream::ReadUInt64()
	{
		uint64_t ret;
		Read(&ret, sizeof ret);
		return ret;
	}

	bool Stream::ReadBool()
	{
		return ReadUByte() != 0;
	}

	float Stream::ReadFloat()
	{
		float ret;
		Read(&ret, sizeof ret);
		return ret;
	}

	double Stream::ReadDouble()
	{
		double ret;
		Read(&ret, sizeof ret);
		return ret;
	}

	uint32_t Stream::ReadVLE()
	{
		uint32_t ret;
		uint8_t byte;

		byte = ReadUByte();
		ret = byte & 0x7f;
		if (byte < 0x80)
			return ret;

		byte = ReadUByte();
		ret |= ((uint32_t)(byte & 0x7f)) << 7;
		if (byte < 0x80)
			return ret;

		byte = ReadUByte();
		ret |= ((uint32_t)(byte & 0x7f)) << 14;
		if (byte < 0x80)
			return ret;

		byte = ReadUByte();
		ret |= ((uint32_t)byte) << 21;
		return ret;
	}

	std::string Stream::ReadString()
	{
		std::string ret;

		while (!IsEof())
		{
			char c = ReadByte();
			if (!c)
				break;
			else
				ret += c;
		}

		return ret;
	}

	std::string Stream::ReadLine()
	{
		std::string ret;

		while (!IsEof())
		{
			char c = ReadByte();
			if (c == 10)
				break;
			if (c == 13)
			{
				// Peek next char to see if it's 10, and skip it too
				if (!IsEof())
				{
					char next = ReadByte();
					if (next != 10)
						Seek(_position - 1);
				}
				break;
			}

			ret += c;
		}

		return ret;
	}

	std::string Stream::ReadFileID()
	{
		std::string ret;
		ret.resize(4);
		Read(&ret.front(), 4);
		return ret;
	}

	StringHash Stream::ReadStringHash()
	{
		return StringHash(ReadUInt());
	}

	std::vector<uint8_t> Stream::ReadBuffer()
	{
		std::vector<uint8_t> ret(ReadVLE());
		if (ret.size())
		{
			Read(ret.data(), ret.size());
		}

		return ret;
	}

	Quaternion Stream::ReadQuaternion()
	{
		float data[4];
		Read(data, sizeof data);
		return Quaternion(data);
	}

	nlohmann::json Stream::ReadJSON()
	{
		auto buffer = ReadBuffer();
		return nlohmann::json::from_ubjson(buffer);
	}

	template<> bool Stream::Read<bool>()
	{
		return ReadUByte() != 0;
	}

	template<> std::string Stream::Read<std::string>()
	{
		return ReadString();
	}

	template<> StringHash Stream::Read<StringHash>()
	{
		return StringHash(ReadUInt());
	}

	template<> ResourceRef Stream::Read<ResourceRef>()
	{
		ResourceRef ret;
		ret.FromBinary(*this);
		return ret;
	}

	template<> ResourceRefList Stream::Read<ResourceRefList>()
	{
		ResourceRefList ret;
		ret.FromBinary(*this);
		return ret;
	}

	template<> ObjectRef Stream::Read<ObjectRef>()
	{
		ObjectRef ret;
		ret.id = ReadUInt();
		return ret;
	}

	template<> nlohmann::json Stream::Read<nlohmann::json>()
	{
		return ReadJSON();
	}

	void Stream::WriteByte(int8_t value)
	{
		Write(&value, sizeof value);
	}

	void Stream::WriteShort(int16_t value)
	{
		Write(&value, sizeof value);
	}

	void Stream::WriteInt(int32_t value)
	{
		Write(&value, sizeof value);
	}

	void Stream::WriteInt64(int64_t value)
	{
		Write(&value, sizeof value);
	}

	void Stream::WriteUByte(uint8_t value)
	{
		Write(&value, sizeof value);
	}

	void Stream::WriteUShort(uint16_t value)
	{
		Write(&value, sizeof value);
	}

	void Stream::WriteUInt(uint32_t value)
	{
		Write(&value, sizeof value);
	}

	void Stream::WriteUInt64(uint64_t value)
	{
		Write(&value, sizeof value);
	}

	void Stream::WriteBool(bool value)
	{
		WriteUByte((uint8_t)(value ? 1 : 0));
	}

	void Stream::WriteFloat(float value)
	{
		Write(&value, sizeof value) ;
	}

	void Stream::WriteDouble(double value)
	{
		Write(&value, sizeof value);
	}

	void Stream::WriteString(const String& value)
	{
		const char* chars = value.c_str();
		// Count length to the first zero, because ReadString() does the same
		size_t length = strlen(chars);
		Write(chars, length + 1);
	}

	void Stream::WriteStringHash(const StringHash& value)
	{
		return WriteUInt(value.Value());
	}

	void Stream::WriteFileID(const std::string& value)
	{
		Write(value.c_str(), Min((int)value.length(), 4));
		for (size_t i = value.length(); i < 4; ++i)
		{
			WriteByte(' ');
		}
	}

	void Stream::WriteBuffer(const std::vector<uint8_t>& value)
	{
		uint32_t numBytes = static_cast<uint32_t>(value.size());

		WriteVLE(numBytes);
		if (numBytes)
			Write(&value[0], numBytes);
	}

	void Stream::WriteVLE(uint32_t value)
	{
		uint8_t data[4];

		if (value < 0x80)
		{
			WriteUByte((uint8_t)value);
		}
		else if (value < 0x4000)
		{
			data[0] = (uint8_t)value | 0x80;
			data[1] = (uint8_t)(value >> 7);
			Write(data, 2);
		}
		else if (value < 0x200000)
		{
			data[0] = (uint8_t)value | 0x80;
			data[1] = (uint8_t)((value >> 7) | 0x80);
			data[2] = (uint8_t)(value >> 14);
			Write(data, 3);
		}
		else
		{
			data[0] = (uint8_t)value | 0x80;
			data[1] = (uint8_t)((value >> 7) | 0x80);
			data[2] = (uint8_t)((value >> 14) | 0x80);
			data[3] = (uint8_t)(value >> 21);
			Write(data, 4);
		}
	}

	void Stream::WriteLine(const std::string& value)
	{
		Write(value.c_str(), value.length());
		WriteUByte('\r');
		WriteUByte('\n');
	}

	void Stream::Write(const ResourceRef& value)
	{
		value.ToBinary(*this);
	}

	void Stream::Write(const ResourceRefList& value)
	{
		value.ToBinary(*this);
	}

	void Stream::Write(const ObjectRef& value)
	{
		WriteUInt(value.id);
	}

	void Stream::Write(const nlohmann::json& value)
	{
		std::vector<std::uint8_t> v_ubjson = nlohmann::json::to_ubjson(value);
		WriteBuffer(v_ubjson);
	}

	template<> void Stream::Write<bool>(const bool& value)
	{
		WriteBool(value);
	}

	template<> void Stream::Write<std::string>(const std::string& value)
	{
		WriteString(value);
	}

	template<> void Stream::Write<StringHash>(const StringHash& value)
	{
		WriteStringHash(value);
	}

	template<> void Stream::Write<ResourceRef>(const ResourceRef& value)
	{
		Write(value);
	}

	template<> void Stream::Write<ResourceRefList>(const ResourceRefList& value)
	{
		Write(value);
	}

	template<> void Stream::Write<ObjectRef>(const ObjectRef& value)
	{
		Write(value);
	}

	template<> void Stream::Write<nlohmann::json>(const nlohmann::json& value)
	{
		Write(value);
	}
}
