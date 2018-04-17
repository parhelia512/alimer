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

#include "../AlimerConfig.h"
#include "../Base/StdHeaders.h"
#include "Math.h"

namespace Alimer
{
	/// Two-dimensional vector with integer values.
	class ALIMER_API IntVector2
	{
	public:
		/// X coordinate.
		int x;
		/// Y coordinate.
		int y;

		/// Construct undefined.
		IntVector2()
		{
		}

		/// Copy-construct.
		IntVector2(const IntVector2& vector) :
			x(vector.x),
			y(vector.y)
		{
		}

		/// Construct from coordinates.
		IntVector2(int x_, int y_) :
			x(x_),
			y(y_)
		{
		}

		/// Construct from an int array.
		IntVector2(const int* data) :
			x(data[0]),
			y(data[1])
		{
		}

		/// Construct by parsing a string.
		IntVector2(const std::string& str)
		{
			FromString(str);
		}

		/// Construct by parsing a C string.
		IntVector2(const char* str)
		{
			FromString(str);
		}

		/// Add-assign a vector.
		IntVector2& operator += (const IntVector2& rhs)
		{
			x += rhs.x;
			y += rhs.y;
			return *this;
		}

		/// Subtract-assign a vector.
		IntVector2& operator -= (const IntVector2& rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
			return *this;
		}

		/// Multiply-assign a scalar.
		IntVector2& operator *= (int rhs)
		{
			x *= rhs;
			y *= rhs;
			return *this;
		}

		/// Divide-assign a scalar.
		IntVector2& operator /= (int rhs)
		{
			x /= rhs;
			y /= rhs;
			return *this;
		}

		/// Test for equality with another vector.
		bool operator == (const IntVector2& rhs) const { return x == rhs.x && y == rhs.y; }
		/// Test for inequality with another vector.
		bool operator != (const IntVector2& rhs) const { return !(*this == rhs); }
		/// Add a vector.
		IntVector2 operator + (const IntVector2& rhs) const { return IntVector2(x + rhs.x, y + rhs.y); }
		/// Return negation.
		IntVector2 operator - () const { return IntVector2(-x, -y); }
		/// Subtract a vector.
		IntVector2 operator - (const IntVector2& rhs) const { return IntVector2(x - rhs.x, y - rhs.y); }
		/// Multiply with a scalar.
		IntVector2 operator * (int rhs) const { return IntVector2(x * rhs, y * rhs); }
		/// Divide by a scalar.
		IntVector2 operator / (int rhs) const { return IntVector2(x / rhs, y / rhs); }

		/// Parse from a string. Return true on success.
		bool FromString(const std::string& str);
		/// Parse from a C string. Return true on success.
		bool FromString(const char* str);

		/// Return integer data.
		const int* Data() const { return &x; }
		/// Return as string.
		std::string ToString() const;

		/// Zero vector.
		static const IntVector2 ZERO;
	};

	/// Multiply IntVector2 with a scalar.
	inline IntVector2 operator * (int lhs, const IntVector2& rhs) { return rhs * lhs; }

}

