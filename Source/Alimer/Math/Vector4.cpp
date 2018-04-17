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

#include "../Base/String.h"
#include "Vector4.h"

#include <cstdio>
#include <cstdlib>

namespace Alimer
{
	const Vector4 Vector4::ZERO(0.0f, 0.0f, 0.0f, 0.0f);
	const Vector4 Vector4::ONE(1.0f, 1.0f, 1.0f, 1.0f);

	bool Vector4::FromString(const std::string& str)
	{
		return FromString(str.c_str());
	}

	bool Vector4::FromString(const char* str)
	{
		size_t elements = str::CountElements(str, ' ');
		if (elements < 4)
			return false;

		char* ptr = (char*)str;
		x = (float)strtod(ptr, &ptr);
		y = (float)strtod(ptr, &ptr);
		z = (float)strtod(ptr, &ptr);
		w = (float)strtod(ptr, &ptr);

		return true;
	}

	std::string Vector4::ToString() const
	{
		char tempBuffer[CONVERSION_BUFFER_LENGTH];
		sprintf(tempBuffer, "%g %g %g %g", x, y, z, w);
		return std::string(tempBuffer);
	}
}
