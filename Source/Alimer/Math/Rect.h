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

#include "Vector4.h"

namespace Alimer
{
	/// Two-dimensional bounding rectangle.
	class ALIMER_API Rect
	{
	public:
		/// Minimum vector.
		Vector2 min;
		/// Maximum vector.
		Vector2 max;

		/// Construct as undefined (negative size.)
		Rect() :
			min(Vector2(M_INFINITY, M_INFINITY)),
			max(Vector2(-M_INFINITY, -M_INFINITY))
		{
		}

		/// Copy-construct.
		Rect(const Rect& rect) :
			min(rect.min),
			max(rect.max)
		{
		}

		/// Construct from minimum and maximum vectors.
		Rect(const Vector2& min_, const Vector2& max_) :
			min(min_),
			max(max_)
		{
		}

		/// Construct from coordinates.
		Rect(float left, float top, float right, float bottom) :
			min(left, top),
			max(right, bottom)
		{
		}

		/// Construct from a Vector4.
		Rect(const Vector4& vector) :
			min(vector.x, vector.y),
			max(vector.z, vector.w)
		{
		}

		/// Construct from a float array.
		Rect(const float* data) :
			min(data[0], data[1]),
			max(data[2], data[3])
		{
		}

		/// Construct by parsing a string.
		Rect(const std::string& str)
		{
			FromString(str);
		}

		/// Construct by parsing a C string.
		Rect(const char* str)
		{
			FromString(str);
		}

		/// Assign from another rect.
		Rect& operator = (const Rect& rhs)
		{
			min = rhs.min;
			max = rhs.max;
			return *this;
		}

		/// Test for equality with another rect without epsilon.
		bool operator == (const Rect& rhs) const { return min == rhs.min && max == rhs.max; }
		/// Test for inequality with another rect without epsilon.
		bool operator != (const Rect& rhs) const { return !(*this == rhs); }

		/// Define from another rect.
		void Define(const Rect& rect)
		{
			min = rect.min;
			max = rect.max;
		}

		/// Define from minimum and maximum vectors.
		void Define(const Vector2& min_, const Vector2& max_)
		{
			min = min_;
			max = max_;
		}

		/// Define from a point.
		void Define(const Vector2& point)
		{
			min = max = point;
		}

		/// Merge a point.
		void Merge(const Vector2& point)
		{
			// If undefined, set initial dimensions
			if (!IsDefined())
			{
				min = max = point;
				return;
			}

			if (point.x < min.x)
				min.x = point.x;
			if (point.x > max.x)
				max.x = point.x;
			if (point.y < min.y)
				min.y = point.y;
			if (point.y > max.y)
				max.y = point.y;
		}

		/// Merge a rect.
		void Merge(const Rect& rect)
		{
			if (min.x > max.x)
			{
				min = rect.min;
				max = rect.max;
				return;
			}

			if (rect.min.x < min.x)
				min.x = rect.min.x;
			if (rect.min.y < min.y)
				min.y = rect.min.y;
			if (rect.max.x > max.x)
				max.x = rect.max.x;
			if (rect.max.y > max.y)
				max.y = rect.max.y;
		}

		/// Set as undefined to allow the next merge to set initial size.
		void Undefine()
		{
			min = Vector2(M_INFINITY, M_INFINITY);
			max = -min;
		}

		/// Clip with another rect.
		void Clip(const Rect& rect);
		/// Parse from a string. Return true on success.
		bool FromString(const std::string& str);
		/// Parse from a C string. Return true on success.
		bool FromString(const char* str);

		/// Return whether has non-negative size.
		bool IsDefined() const { return (min.x <= max.x); }
		/// Return center.
		Vector2 Center() const { return (max + min) * 0.5f; }
		/// Return size.
		Vector2 Size() const { return max - min; }
		/// Return half-size.
		Vector2 HalfSize() const { return (max - min) * 0.5f; }
		/// Test for equality with another rect with epsilon.
		bool Equals(const Rect& rhs) const { return min.Equals(rhs.min) && max.Equals(rhs.max); }

		/// Test whether a point is inside.
		Intersection IsInside(const Vector2& point) const
		{
			if (point.x < min.x || point.y < min.y || point.x > max.x || point.y > max.y)
				return OUTSIDE;
			else
				return INSIDE;
		}

		/// Return float data.
		const void* Data() const { return &min.x; }
		/// Return as a vector.
		Vector4 ToVector4() const { return Vector4(min.x, min.y, max.x, max.y); }
		/// Return as string.
		std::string ToString() const;

		/// Rect in the range (-1, -1) - (1, 1)
		static const Rect FULL;
		/// Rect in the range (0, 0) - (1, 1)
		static const Rect POSITIVE;
		/// Zero-sized rect.
		static const Rect ZERO;
	};

}
