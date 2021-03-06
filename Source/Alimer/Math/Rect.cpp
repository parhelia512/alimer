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
#include "Rect.h"

#include <cstdlib>

namespace Alimer
{
	const Rect Rect::FULL(-1.0f, -1.0f, 1.0f, 1.0f);
	const Rect Rect::POSITIVE(0.0f, 0.0f, 1.0f, 1.0f);
	const Rect Rect::ZERO(0.0f, 0.0f, 0.0f, 0.0f);

	void Rect::Clip(const Rect& rect)
	{
		if (rect.min.x > min.x)
			min.x = rect.min.x;
		if (rect.max.x < max.x)
			max.x = rect.max.x;
		if (rect.min.y > min.y)
			min.y = rect.min.y;
		if (rect.max.y < max.y)
			max.y = rect.max.y;

		if (min.x > max.x)
			std::swap(min.x, max.x);
		if (min.y > max.y)
			std::swap(min.y, max.y);
	}

	bool Rect::FromString(const std::string& str)
	{
		return FromString(str.c_str());
	}

	bool Rect::FromString(const char* str)
	{
		size_t elements = str::CountElements(str, ' ');
		if (elements < 4)
			return false;

		char* ptr = (char*)str;
		min.x = (float)strtod(ptr, &ptr);
		min.y = (float)strtod(ptr, &ptr);
		max.x = (float)strtod(ptr, &ptr);
		max.y = (float)strtod(ptr, &ptr);

		return true;
	}

	std::string Rect::ToString() const
	{
		return min.ToString() + " " + max.ToString();
	}
}
