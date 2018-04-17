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

#include "String.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
using namespace std;

namespace Alimer
{
	string str::Replace(
		const string& str,
		const string& find,
		const string& replace,
		uint32_t maxReplacements)
	{
		string dest = str;
		size_t pos = 0;
		while ((pos = dest.find(find, pos)) != string::npos)
		{
			dest.replace(dest.find(find), find.size(), replace);
			maxReplacements--;
			pos += replace.size();
			if (maxReplacements == 0)
				break;
		}

		return dest;
	}

	vector<string> str::Split(const string& value, const string& separator, bool keepEmpty)
	{
		assert(!separator.empty());
		vector<string> result;
		string::const_iterator itStart = value.begin(), itEnd;

		for (;;)
		{
			itEnd = search(itStart, value.end(), separator.begin(), separator.end());
			string token(itStart, itEnd);
			if (keepEmpty || !token.empty())
				result.push_back(token);
			if (itEnd == value.end())
				break;
			itStart = itEnd + separator.size();
		}
		return result;
	}

	string str::Join(const vector<string>& collection, const string& glue)
	{
		string result;
		if (!collection.empty())
		{
			result = collection.front();

			for (auto it = collection.begin() + 1; it != collection.end(); it++)
				result += glue + *it;
		}
		return result;
	}

	string str::Trim(const string& source)
	{
		size_t trimStart = 0;
		size_t trimEnd = source.length();

		while (trimStart < trimEnd)
		{
			char c = source[trimStart];
			if (c != ' ' && c != 9)
				break;
			++trimStart;
		}
		while (trimEnd > trimStart)
		{
			char c = source[trimEnd - 1];
			if (c != ' ' && c != 9)
				break;
			--trimEnd;
		}

		return source.substr(trimStart, trimEnd - trimStart);
	}

	std::string str::ToUpper(const std::string& source)
	{
		std::string result(source);

		for (auto it = result.begin(); it != result.end(); ++it)
			*it = Alimer::ToUpper(*it);

		return result;
	}

	string str::ToLower(const string& source)
	{
		string result(source);

		for (auto it = result.begin(); it != result.end(); ++it)
			*it = Alimer::ToLower(*it);

		return result;
	}

	bool str::StartsWith(const string& str, const string& value, bool caseSensitive)
	{
		if (caseSensitive)
			return str.find(value) != string::npos;

		return str::ToLower(str).find(str::ToLower(value)) != string::npos;
	}

	bool str::EndsWith(const std::string& str, const std::string& value, bool caseSensitive)
	{
		size_t pos;
		if (caseSensitive)
		{
			pos = str.find_last_of(value);
		}
		else
		{
			pos = str::ToLower(str).find_last_of(str::ToLower(value));
		}

		return pos != string::npos && pos == str.length() - str.length();
	}

	bool str::ToBool(const char* str)
	{
		const size_t length = strlen(str);

		for (size_t i = 0; i < length; ++i)
		{
			char c = Alimer::ToLower(str[i]);
			if (c == 't' || c == 'y' || c == '1')
				return true;
			else if (c != ' ' && c != '\t')
				break;
		}

		return false;
	}

	int str::ToInt(const char* str)
	{
		if (!str)
			return 0;

		// Explicitly ask for base 10 to prevent source starts with '0' or '0x' from being converted to base 8 or base 16, respectively
		return strtol(str, 0, 10);
	}

	unsigned str::ToUInt(const char* str)
	{
		if (!str)
			return 0;

		return strtoul(str, 0, 10);
	}

	float str::ToFloat(const char* str)
	{
		if (!str)
			return 0;

		return (float)strtod(str, 0);
	}

	int str::Compare(const std::string& str1, const std::string& str2, bool caseSensitive)
	{
		return str::Compare(str1.c_str(), str2.c_str(), caseSensitive);
	}

	int str::Compare(const char* str1, const char* str2, bool caseSensitive)
	{
		if (caseSensitive)
			return strcmp(str1, str2);

#ifdef _MSC_VER
		return _strcmpi(str1, str2);
#else
		return strcasecmp(str1, str2);
#endif
	}

	string str::Format(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		char buf[256];
		size_t n = std::vsnprintf(buf, sizeof(buf), format, args);
		va_end(args);

		// Static buffer large enough?
		if (n < sizeof(buf)) {
			return { buf, n };
		}

		// Static buffer too small
		string s(n + 1, 0);
		va_start(args, format);
		std::vsnprintf(const_cast<char*>(s.data()), s.size(), format, args);
		va_end(args);
		return s;
	}

	size_t str::ListIndex(const string& value, const string* strings, size_t defaultIndex, bool caseSensitive)
	{
		return str::ListIndex(value.c_str(), strings, defaultIndex, caseSensitive);
	}

	size_t str::ListIndex(const char* value, const string* strings, size_t defaultIndex, bool caseSensitive)
	{
		size_t i = 0;

		while (!strings[i].empty())
		{
			if (!str::Compare(strings[i], value, caseSensitive))
				return i;
			++i;
		}

		return defaultIndex;
	}

	size_t str::ListIndex(const string& value, const char** strings, size_t defaultIndex, bool caseSensitive)
	{
		return ListIndex(value.c_str(), strings, defaultIndex, caseSensitive);
	}

	size_t str::ListIndex(const char* value, const char** strings, size_t defaultIndex, bool caseSensitive)
	{
		size_t i = 0;

		while (strings[i])
		{
			if (!str::Compare(value, strings[i], caseSensitive))
				return i;
			++i;
		}

		return defaultIndex;
	}

	void str::EncodeUTF8(char*& dest, unsigned unicodeChar)
	{
		if (unicodeChar < 0x80)
			*dest++ = (char)unicodeChar;
		else if (unicodeChar < 0x800)
		{
			dest[0] = 0xc0 | ((unicodeChar >> 6) & 0x1f);
			dest[1] = 0x80 | (unicodeChar & 0x3f);
			dest += 2;
		}
		else if (unicodeChar < 0x10000)
		{
			dest[0] = 0xe0 | ((unicodeChar >> 12) & 0xf);
			dest[1] = 0x80 | ((unicodeChar >> 6) & 0x3f);
			dest[2] = 0x80 | (unicodeChar & 0x3f);
			dest += 3;
		}
		else if (unicodeChar < 0x200000)
		{
			dest[0] = 0xf0 | ((unicodeChar >> 18) & 0x7);
			dest[1] = 0x80 | ((unicodeChar >> 12) & 0x3f);
			dest[2] = 0x80 | ((unicodeChar >> 6) & 0x3f);
			dest[3] = 0x80 | (unicodeChar & 0x3f);
			dest += 4;
		}
		else if (unicodeChar < 0x4000000)
		{
			dest[0] = 0xf8 | ((unicodeChar >> 24) & 0x3);
			dest[1] = 0x80 | ((unicodeChar >> 18) & 0x3f);
			dest[2] = 0x80 | ((unicodeChar >> 12) & 0x3f);
			dest[3] = 0x80 | ((unicodeChar >> 6) & 0x3f);
			dest[4] = 0x80 | (unicodeChar & 0x3f);
			dest += 5;
		}
		else
		{
			dest[0] = 0xfc | ((unicodeChar >> 30) & 0x1);
			dest[1] = 0x80 | ((unicodeChar >> 24) & 0x3f);
			dest[2] = 0x80 | ((unicodeChar >> 18) & 0x3f);
			dest[3] = 0x80 | ((unicodeChar >> 12) & 0x3f);
			dest[4] = 0x80 | ((unicodeChar >> 6) & 0x3f);
			dest[5] = 0x80 | (unicodeChar & 0x3f);
			dest += 6;
		}
	}

	string& str::AppendUTF8(string& dest, unsigned unicodeChar)
	{
		char temp[7];
		char* destTemp = temp;
		str::EncodeUTF8(destTemp, unicodeChar);
		*destTemp = 0;
		return dest.append(temp);
	}

	size_t str::CountElements(const char* buffer, char separator)
	{
		if (!buffer)
			return 0;

		const char* endPos = buffer + strlen(buffer);
		const char* pos = buffer;
		size_t ret = 0;

		while (pos < endPos)
		{
			if (*pos != separator)
				break;
			++pos;
		}

		while (pos < endPos)
		{
			const char* start = pos;

			while (start < endPos)
			{
				if (*start == separator)
					break;

				++start;
			}

			if (start == endPos)
			{
				++ret;
				break;
			}

			const char* end = start;

			while (end < endPos)
			{
				if (*end != separator)
					break;

				++end;
			}

			++ret;
			pos = end;
		}

		return ret;
	}
}
