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

#include "StringHash.h"

#include <cstdio>

namespace Alimer
{
	const StringHash StringHash::ZERO;

	StringHash::StringHash(const char* str) noexcept
		: _value(Calculate(str))
	{
	}

	StringHash::StringHash(const std::string& str) noexcept
		: _value(Calculate(str.c_str()))
	{
	}

	StringHash::StringHash(const String& str) noexcept
		: _value(Calculate(str.CString()))
	{
	}

	uint32_t StringHash::Calculate(const char* str, uint32_t hash)
	{
		if (!str)
			return hash;

		while (*str)
		{
			hash = (Alimer::ToLower(*str)) + (hash << 6) + (hash << 16) - hash;
			++str;
		}

		return hash;
	}

	std::string StringHash::ToString() const
	{
		char tempBuffer[CONVERSION_BUFFER_LENGTH];
		sprintf(tempBuffer, "%08X", _value);
		return std::string(tempBuffer);
	}
}
