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

#include "JSONValue.h"
#include "Stream.h"

#include <cstdio>
#include <cstdlib>
using namespace std;

namespace Alimer
{
	const JSONValue JSONValue::EMPTY;
	const JSONArray JSONValue::emptyJSONArray;
	const JSONObject JSONValue::emptyJSONObject;

	JSONValue::JSONValue() :
		type(JSON_NULL)
	{
	}

	JSONValue::JSONValue(const JSONValue& value) :
		type(JSON_NULL)
	{
		*this = value;
	}

	JSONValue::JSONValue(bool value) :
		type(JSON_NULL)
	{
		*this = value;
	}

	JSONValue::JSONValue(int value) :
		type(JSON_NULL)
	{
		*this = value;
	}

	JSONValue::JSONValue(unsigned value) :
		type(JSON_NULL)
	{
		*this = value;
	}

	JSONValue::JSONValue(float value) :
		type(JSON_NULL)
	{
		*this = value;
	}

	JSONValue::JSONValue(double value) :
		type(JSON_NULL)
	{
		*this = value;
	}

	JSONValue::JSONValue(const std::string& value)
		: type(JSON_NULL)
	{
		*this = value;
	}

	JSONValue::JSONValue(const char* value) :
		type(JSON_NULL)
	{
		*this = value;
	}

	JSONValue::JSONValue(const JSONArray& value) :
		type(JSON_NULL)
	{
		*this = value;
	}

	JSONValue::JSONValue(const JSONObject& value) :
		type(JSON_NULL)
	{
		*this = value;
	}

	JSONValue::~JSONValue()
	{
		SetType(JSON_NULL);
	}

	JSONValue& JSONValue::operator = (const JSONValue& rhs)
	{
		SetType(rhs.type);

		switch (type)
		{
		case JSON_BOOL:
			data.boolValue = rhs.data.boolValue;
			break;

		case JSON_NUMBER:
			data.numberValue = rhs.data.numberValue;
			break;

		case JSON_STRING:
			*(reinterpret_cast<std::string*>(&data)) = *(reinterpret_cast<const std::string*>(&rhs.data));
			break;

		case JSON_ARRAY:
			*(reinterpret_cast<JSONArray*>(&data)) = *(reinterpret_cast<const JSONArray*>(&rhs.data));
			break;

		case JSON_OBJECT:
			*(reinterpret_cast<JSONObject*>(&data)) = *(reinterpret_cast<const JSONObject*>(&rhs.data));
			break;

		default:
			break;
		}

		return *this;
	}

	JSONValue& JSONValue::operator = (bool rhs)
	{
		SetType(JSON_BOOL);
		data.boolValue = rhs;
		return *this;
	}

	JSONValue& JSONValue::operator = (int rhs)
	{
		SetType(JSON_NUMBER);
		data.numberValue = (double)rhs;
		return *this;
	}

	JSONValue& JSONValue::operator = (unsigned rhs)
	{
		SetType(JSON_NUMBER);
		data.numberValue = (double)rhs;
		return *this;
	}

	JSONValue& JSONValue::operator = (float rhs)
	{
		SetType(JSON_NUMBER);
		data.numberValue = (double)rhs;
		return *this;
	}

	JSONValue& JSONValue::operator = (double rhs)
	{
		SetType(JSON_NUMBER);
		data.numberValue = rhs;
		return *this;
	}

	JSONValue& JSONValue::operator = (const String& value)
	{
		SetType(JSON_STRING);
		*(reinterpret_cast<std::string*>(&data)) = value.CString();
		return *this;
	}

	JSONValue& JSONValue::operator = (const std::string& value)
	{
		SetType(JSON_STRING);
		*(reinterpret_cast<std::string*>(&data)) = value;
		return *this;
	}

	JSONValue& JSONValue::operator = (const char* value)
	{
		SetType(JSON_STRING);
		*(reinterpret_cast<std::string*>(&data)) = value;
		return *this;
	}

	JSONValue& JSONValue::operator = (const JSONArray& value)
	{
		SetType(JSON_ARRAY);
		*(reinterpret_cast<JSONArray*>(&data)) = value;
		return *this;
	}

	JSONValue& JSONValue::operator = (const JSONObject& value)
	{
		SetType(JSON_OBJECT);
		*(reinterpret_cast<JSONObject*>(&data)) = value;
		return *this;
	}

	JSONValue& JSONValue::operator [] (size_t index)
	{
		if (type != JSON_ARRAY)
			SetType(JSON_ARRAY);

		return (*(reinterpret_cast<JSONArray*>(&data)))[index];
	}

	const JSONValue& JSONValue::operator [] (size_t index) const
	{
		if (type == JSON_ARRAY)
			return (*(reinterpret_cast<const JSONArray*>(&data)))[index];
		else
			return EMPTY;
	}

	JSONValue& JSONValue::operator [] (const std::string& key)
	{
		if (type != JSON_OBJECT)
			SetType(JSON_OBJECT);

		return (*(reinterpret_cast<JSONObject*>(&data)))[key];
	}

	const JSONValue& JSONValue::operator [] (const std::string& key) const
	{
		if (type == JSON_OBJECT)
		{
			const JSONObject& object = *(reinterpret_cast<const JSONObject*>(&data));
			auto it = object.find(key);
			return it != object.end() ? it->second : EMPTY;
		}
		else
			return EMPTY;
	}

	bool JSONValue::operator == (const JSONValue& rhs) const
	{
		if (type != rhs.type)
			return false;

		switch (type)
		{
		case JSON_BOOL:
			return data.boolValue == rhs.data.boolValue;

		case JSON_NUMBER:
			return data.numberValue == rhs.data.numberValue;

		case JSON_STRING:
			return *(reinterpret_cast<const std::string*>(&data)) == *(reinterpret_cast<const std::string*>(&rhs.data));

		case JSON_ARRAY:
			return *(reinterpret_cast<const JSONArray*>(&data)) == *(reinterpret_cast<const JSONArray*>(&rhs.data));

		case JSON_OBJECT:
			return *(reinterpret_cast<const JSONObject*>(&data)) == *(reinterpret_cast<const JSONObject*>(&rhs.data));

		default:
			return true;
		}
	}

	bool JSONValue::FromString(const std::string& str)
	{
		const char* pos = str.c_str();
		const char* end = pos + str.length();
		return Parse(pos, end);
	}

	bool JSONValue::FromString(const char* str)
	{
		const char* pos = str;
		const char* end = pos + strlen(str);
		return Parse(pos, end);
	}

	void JSONValue::FromBinary(Stream& source)
	{
		JSONType newType = (JSONType)source.Read<unsigned char>();

		switch (newType)
		{
		case JSON_NULL:
			Clear();
			break;

		case JSON_BOOL:
			*this = source.Read<bool>();
			break;

		case JSON_NUMBER:
			*this = source.Read<double>();
			break;

		case JSON_STRING:
			*this = source.ReadString();
			break;

		case JSON_ARRAY:
		{
			SetEmptyArray();
			size_t num = source.ReadVLE();
			for (size_t i = 0; i < num && !source.IsEof(); ++i)
				Push(source.Read<JSONValue>());
		}
		break;

		case JSON_OBJECT:
		{
			SetEmptyObject();
			size_t num = source.ReadVLE();
			for (size_t i = 0; i < num && !source.IsEof(); ++i)
			{
				std::string key = source.ReadString();
				(*this)[key] = source.Read<JSONValue>();
			}
		}
		break;

		default:
			break;
		}
	}

	void JSONValue::ToString(std::string& dest, int spacing, int indent) const
	{
		switch (type)
		{
		case JSON_BOOL:
			dest += std::to_string(data.boolValue);
			return;

		case JSON_NUMBER:
			dest += std::to_string(data.numberValue);
			return;

		case JSON_STRING:
			WriteJSONString(dest, *(reinterpret_cast<const std::string*>(&data)));
			return;

		case JSON_ARRAY:
		{
			const JSONArray& array = GetArray();
			dest += '[';

			if (array.size())
			{
				indent += spacing;
				for (auto it = array.begin(); it < array.end(); ++it)
				{
					if (it != array.begin())
						dest += ',';
					dest += '\n';
					WriteIndent(dest, indent);
					it->ToString(dest, spacing, indent);
				}
				indent -= spacing;
				dest += '\n';
				WriteIndent(dest, indent);
			}

			dest += ']';
		}
		break;

		case JSON_OBJECT:
		{
			const JSONObject& object = GetObject();
			dest += '{';

			if (object.size())
			{
				indent += spacing;
				for (auto it = object.begin(); it != object.end(); ++it)
				{
					if (it != object.begin())
						dest += ',';
					dest += '\n';
					WriteIndent(dest, indent);
					WriteJSONString(dest, it->first);
					dest += ": ";
					it->second.ToString(dest, spacing, indent);
				}
				indent -= spacing;
				dest += '\n';
				WriteIndent(dest, indent);
			}

			dest += '}';
		}
		break;

		default:
			dest += "null";
		}
	}

	std::string JSONValue::ToString(int spacing) const
	{
		std::string ret;
		ToString(ret, spacing);
		return ret;
	}

	void JSONValue::ToBinary(Stream& dest) const
	{
		dest.Write((unsigned char)type);

		switch (type)
		{
		case JSON_BOOL:
			dest.Write(data.boolValue);
			break;

		case JSON_NUMBER:
			dest.Write(data.numberValue);
			break;

		case JSON_STRING:
			dest.Write(GetString());
			break;

		case JSON_ARRAY:
		{
			const JSONArray& array = GetArray();
			dest.WriteVLE(static_cast<uint32_t>(array.size()));
			for (auto it = array.begin(); it != array.end(); ++it)
				it->ToBinary(dest);
		}
		break;

		case JSON_OBJECT:
		{
			const JSONObject& object = GetObject();
			dest.WriteVLE(static_cast<uint32_t>(object.size()));
			for (auto it = object.begin(); it != object.end(); ++it)
			{
				dest.Write(it->first);
				it->second.ToBinary(dest);
			}
		}
		break;

		default:
			break;
		}
	}

	void JSONValue::Push(const JSONValue& value)
	{
		SetType(JSON_ARRAY);
		(*(reinterpret_cast<JSONArray*>(&data))).push_back(value);
	}

	void JSONValue::Insert(size_t index, const JSONValue& value)
	{
		SetType(JSON_ARRAY);
		(*(reinterpret_cast<JSONArray*>(&data))).insert((*(reinterpret_cast<JSONArray*>(&data))).begin() + index, value);
	}

	void JSONValue::Pop()
	{
		if (type == JSON_ARRAY)
			(*(reinterpret_cast<JSONArray*>(&data))).pop_back();
	}

	void JSONValue::Erase(size_t pos)
	{
		if (type == JSON_ARRAY)
			(*(reinterpret_cast<JSONArray*>(&data))).erase((*(reinterpret_cast<JSONArray*>(&data))).begin() + pos);
	}

	void JSONValue::Resize(size_t newSize)
	{
		SetType(JSON_ARRAY);
		(*(reinterpret_cast<JSONArray*>(&data))).resize(newSize);
	}

	void JSONValue::Insert(const std::pair<std::string, JSONValue>& pair)
	{
		SetType(JSON_OBJECT);
		(*(reinterpret_cast<JSONObject*>(&data))).insert(pair);
	}

	void JSONValue::Erase(const std::string& key)
	{
		if (type == JSON_OBJECT)
			(*(reinterpret_cast<JSONObject*>(&data))).erase(key);
	}

	void JSONValue::Clear()
	{
		if (type == JSON_ARRAY)
			(*(reinterpret_cast<JSONArray*>(&data))).clear();
		else if (type == JSON_OBJECT)
			(*(reinterpret_cast<JSONObject*>(&data))).clear();
	}

	void JSONValue::SetEmptyArray()
	{
		SetType(JSON_ARRAY);
		Clear();
	}

	void JSONValue::SetEmptyObject()
	{
		SetType(JSON_OBJECT);
		Clear();
	}

	void JSONValue::SetNull()
	{
		SetType(JSON_NULL);
	}

	size_t JSONValue::Size() const
	{
		if (type == JSON_ARRAY)
			return (*(reinterpret_cast<const JSONArray*>(&data))).size();
		else if (type == JSON_OBJECT)
			return (*(reinterpret_cast<const JSONObject*>(&data))).size();
		else
			return 0;
	}

	bool JSONValue::IsEmpty() const
	{
		if (type == JSON_ARRAY)
			return (*(reinterpret_cast<const JSONArray*>(&data))).empty();
		else if (type == JSON_OBJECT)
			return (*(reinterpret_cast<const JSONObject*>(&data))).empty();
		else
			return false;
	}

	bool JSONValue::Contains(const std::string& key) const
	{
		if (type == JSON_OBJECT)
			return (*(reinterpret_cast<const JSONObject*>(&data))).find(key) != end((*(reinterpret_cast<const JSONObject*>(&data))));
		
		return false;
	}

	bool JSONValue::Parse(const char*& pos, const char*& end)
	{
		char c;

		// Handle comments
		for (;;)
		{
			if (!NextChar(c, pos, end, true))
				return false;

			if (c == '/')
			{
				if (!NextChar(c, pos, end, false))
					return false;
				if (c == '/')
				{
					// Skip until end of line
					if (!MatchChar('\n', pos, end))
						return false;
				}
				else if (c == '*')
				{
					// Skip until end of comment
					if (!MatchChar('*', pos, end))
						return false;
					if (!MatchChar('/', pos, end))
						return false;
				}
				else
					return false;
			}
			else
				break;
		}

		if (c == '}' || c == ']')
			return false;
		else if (c == 'n')
		{
			SetNull();
			return MatchString("ull", pos, end);
		}
		else if (c == 'f')
		{
			*this = false;
			return MatchString("alse", pos, end);
		}
		else if (c == 't')
		{
			*this = true;
			return MatchString("rue", pos, end);
		}
		else if (IsDigit(c) || c == '-')
		{
			--pos;
			*this = strtod(pos, const_cast<char**>(&pos));
			return true;
		}
		else if (c == '\"')
		{
			SetType(JSON_STRING);
			return ReadJSONString(*(reinterpret_cast<std::string*>(&data)), pos, end, true);
		}
		else if (c == '[')
		{
			SetEmptyArray();
			// Check for empty first
			if (!NextChar(c, pos, end, true))
				return false;
			if (c == ']')
				return true;
			else
				--pos;

			for (;;)
			{
				JSONValue arrayValue;
				if (!arrayValue.Parse(pos, end))
					return false;
				Push(arrayValue);
				if (!NextChar(c, pos, end, true))
					return false;
				if (c == ']')
					break;
				else if (c != ',')
					return false;
			}
			return true;
		}
		else if (c == '{')
		{
			SetEmptyObject();
			if (!NextChar(c, pos, end, true))
				return false;
			if (c == '}')
				return true;
			else
				--pos;

			for (;;)
			{
				std::string key;
				if (!ReadJSONString(key, pos, end, false))
					return false;
				if (!NextChar(c, pos, end, true))
					return false;
				if (c != ':')
					return false;

				JSONValue objectValue;
				if (!objectValue.Parse(pos, end))
					return false;
				(*this)[key] = objectValue;
				if (!NextChar(c, pos, end, true))
					return false;
				if (c == '}')
					break;
				else if (c != ',')
					return false;
			}
			return true;
		}

		return false;
	}

	void JSONValue::SetType(JSONType newType)
	{
		if (type == newType)
			return;

		switch (type)
		{
		case JSON_STRING:
			(reinterpret_cast<string*>(&data))->~string();
			break;

		case JSON_ARRAY:
			(reinterpret_cast<JSONArray*>(&data))->~JSONArray();
			break;

		case JSON_OBJECT:
			(reinterpret_cast<JSONObject*>(&data))->~JSONObject();
			break;

		default:
			break;
		}

		type = newType;

		switch (type)
		{
		case JSON_STRING:
			new(reinterpret_cast<std::string*>(&data)) std::string();
			break;

		case JSON_ARRAY:
			new(reinterpret_cast<JSONArray*>(&data)) JSONArray();
			break;

		case JSON_OBJECT:
			new(reinterpret_cast<JSONObject*>(&data)) JSONObject();
			break;

		default:
			break;
		}
	}

	void JSONValue::WriteJSONString(std::string& dest, const std::string& str)
	{
		dest += '\"';

		for (auto it = str.begin(); it != str.end(); ++it)
		{
			char c = *it;

			if (c >= 0x20 && c != '\"' && c != '\\')
				dest += c;
			else
			{
				dest += '\\';

				switch (c)
				{
				case '\"':
				case '\\':
					dest += c;
					break;

				case '\b':
					dest += 'b';
					break;

				case '\f':
					dest += 'f';
					break;

				case '\n':
					dest += 'n';
					break;

				case '\r':
					dest += 'r';
					break;

				case '\t':
					dest += 't';
					break;

				default:
				{
					char buffer[6];
					sprintf(buffer, "u%04x", c);
					dest += buffer;
				}
				break;
				}
			}
		}

		dest += '\"';
	}

	void JSONValue::WriteIndent(std::string& dest, int indent)
	{
		size_t oldLength = dest.length();
		dest.resize(oldLength + indent);
		for (int i = 0; i < indent; ++i)
			dest[oldLength + i] = ' ';
	}

	bool JSONValue::ReadJSONString(std::string& dest, const char*& pos, const char*& end, bool inQuote)
	{
		char c;

		if (!inQuote)
		{
			if (!NextChar(c, pos, end, true) || c != '\"')
				return false;
		}

		dest.clear();
		for (;;)
		{
			if (!NextChar(c, pos, end, false))
				return false;
			if (c == '\"')
				break;
			else if (c != '\\')
				dest += c;
			else
			{
				if (!NextChar(c, pos, end, false))
					return false;
				switch (c)
				{
				case '\\':
					dest += '\\';
					break;

				case '\"':
					dest += '\"';
					break;

				case 'b':
					dest += '\b';
					break;

				case 'f':
					dest += '\f';
					break;

				case 'n':
					dest += '\n';
					break;

				case 'r':
					dest += '\r';
					break;

				case 't':
					dest += '\t';
					break;

				case 'u':
				{
					/// \todo Doesn't handle unicode surrogate pairs
					uint32_t code;
					sscanf(pos, "%x", &code);
					pos += 4;

					char temp[7];
					char* destTmp = temp;
					String::EncodeUTF8(destTmp, code);
					*destTmp = 0;
					dest.append(destTmp);
				}
				break;
				}
			}
		}

		return true;
	}

	bool JSONValue::MatchString(const char* str, const char*& pos, const char*& end)
	{
		while (*str)
		{
			if (pos >= end || *pos != *str)
				return false;
			else
			{
				++pos;
				++str;
			}
		}

		return true;
	}

	bool JSONValue::MatchChar(char c, const char*& pos, const char*& end)
	{
		char next;

		while (NextChar(next, pos, end, false))
		{
			if (next == c)
				return true;
		}
		return false;
	}

}
