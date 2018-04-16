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

#include "IntVector2.h"
#include "Size.h"

namespace Alimer
{
	/// Two-dimensional bounding rectangle with integer values.
	class ALIMER_API IntRect
	{
	public:
		/// Left coordinate.
		int left;
		/// Top coordinate.
		int top;
		/// Right coordinate.
		int right;
		/// Bottom coordinate.
		int bottom;

		/// Construct with all values to zero.
		IntRect() : left(0), top(0), right(0), bottom(0)
		{
		}

		/// Copy-construct.
		IntRect(const IntRect& rect)
			: left(rect.left)
			, top(rect.top)
			, right(rect.right)
			, bottom(rect.bottom)
		{
		}

		/// Construct from coordinates.
		IntRect(int left_, int top_, int right_, int bottom_)
			: left(left_)
			, top(top_)
			, right(right_)
			, bottom(bottom_)
		{
		}

		/// Construct from an int array.
		IntRect(const int* data) :
			left(data[0]),
			top(data[1]),
			right(data[2]),
			bottom(data[3])
		{
		}

		/// Construct from Size.
		IntRect(const Size& size)
			: left(0)
			, top(0)
			, right(size.width)
			, bottom(size.height)
		{
		}

		/// Construct by parsing a string.
		IntRect(const std::string& str)
		{
			FromString(str);
		}

		/// Construct by parsing a C string.
		IntRect(const char* str)
		{
			FromString(str);
		}

		/// Test for equality with another rect.
		bool operator == (const IntRect& rhs) const { return left == rhs.left && top == rhs.top && right == rhs.right && bottom == rhs.bottom; }
		/// Test for inequality with another rect.
		bool operator != (const IntRect& rhs) const { return !(*this == rhs); }

		/// Parse from a string. Return true on success.
		bool FromString(const std::string& str);
		/// Parse from a C string. Return true on success.
		bool FromString(const char* str);

		/// Return size.
		IntVector2 Size() const { return IntVector2(Width(), Height()); }
		/// Return width.
		int Width() const { return right - left; }
		/// Return height.
		int Height() const { return bottom - top; }

		/// Test whether a point is inside.
		Intersection IsInside(const IntVector2& point) const
		{
			if (point.x < left || point.y < top || point.x >= right || point.y >= bottom)
				return OUTSIDE;
			
			return INSIDE;
		}

		/// Test whether another rect is inside or intersects.
		Intersection IsInside(const IntRect& rect) const
		{
			if (rect.right <= left || rect.left >= right || rect.bottom <= top || rect.top >= bottom)
				return OUTSIDE;
			else if (rect.left >= left && rect.right <= right && rect.top >= top && rect.bottom <= bottom)
				return INSIDE;
			else
				return INTERSECTS;
		}

		/// Return integer data.
		const int* Data() const { return &left; }
		/// Return as string.
		std::string ToString() const;

		/// Zero-sized rect.
		static const IntRect ZERO;
	};
}
