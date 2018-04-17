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

#include "Vector3.h"

namespace Alimer
{
	/// Four-dimensional vector.
	class ALIMER_API Vector4
	{
	public:
		/// X coordinate.
		float x;
		/// Y coordinate.
		float y;
		/// Z coordinate.
		float z;
		/// W coordinate.
		float w;

		/// Construct undefined.
		Vector4()
		{
		}

		/// Copy-construct.
		Vector4(const Vector4& vector) :
			x(vector.x),
			y(vector.y),
			z(vector.z),
			w(vector.w)
		{
		}

		/// Construct from a 3-dimensional vector and the W coordinate.
		Vector4(const Vector3& vector, float w_) :
			x(vector.x),
			y(vector.y),
			z(vector.z),
			w(w_)
		{
		}

		/// Construct from coordinates.
		Vector4(float x_, float y_, float z_, float w_) :
			x(x_),
			y(y_),
			z(z_),
			w(w_)
		{
		}

		/// Construct from a float array.
		Vector4(const float* data) :
			x(data[0]),
			y(data[1]),
			z(data[2]),
			w(data[3])
		{
		}

		/// Assign from another vector.
		Vector4& operator = (const Vector4& rhs)
		{
			x = rhs.x;
			y = rhs.y;
			z = rhs.z;
			w = rhs.w;
			return *this;
		}

		/// Construct by parsing a string.
		Vector4(const std::string& str)
		{
			FromString(str);
		}

		/// Construct by parsing a C string.
		Vector4(const char* str)
		{
			FromString(str);
		}

		/// Test for equality with another vector without epsilon.
		bool operator == (const Vector4& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w; }
		/// Test for inequality with another vector without epsilon.
		bool operator != (const Vector4& rhs) const { return !(*this == rhs); }
		/// Add a vector.
		Vector4 operator + (const Vector4& rhs) const { return Vector4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w); }
		/// Return negation.
		Vector4 operator - () const { return Vector4(-x, -y, -z, -w); }
		/// Subtract a vector.
		Vector4 operator - (const Vector4& rhs) const { return Vector4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w); }
		/// Multiply with a scalar.
		Vector4 operator * (float rhs) const { return Vector4(x * rhs, y * rhs, z * rhs, w * rhs); }
		/// Multiply with a vector.
		Vector4 operator * (const Vector4& rhs) const { return Vector4(x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w); }
		/// Divide by a scalar.
		Vector4 operator / (float rhs) const { return Vector4(x / rhs, y / rhs, z / rhs, w / rhs); }
		/// Divide by a vector.
		Vector4 operator / (const Vector4& rhs) const { return Vector4(x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w); }

		/// Add-assign a vector.
		Vector4& operator += (const Vector4& rhs)
		{
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;
			w += rhs.w;
			return *this;
		}

		/// Subtract-assign a vector.
		Vector4& operator -= (const Vector4& rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
			z -= rhs.z;
			w -= rhs.w;
			return *this;
		}

		/// Multiply-assign a scalar.
		Vector4& operator *= (float rhs)
		{
			x *= rhs;
			y *= rhs;
			z *= rhs;
			w *= rhs;
			return *this;
		}

		/// Multiply-assign a vector.
		Vector4& operator *= (const Vector4& rhs)
		{
			x *= rhs.x;
			y *= rhs.y;
			z *= rhs.z;
			w *= rhs.w;
			return *this;
		}

		/// Divide-assign a scalar.
		Vector4& operator /= (float rhs)
		{
			float invRhs = 1.0f / rhs;
			x *= invRhs;
			y *= invRhs;
			z *= invRhs;
			w *= invRhs;
			return *this;
		}

		/// Divide-assign a vector.
		Vector4& operator /= (const Vector4& rhs)
		{
			x /= rhs.x;
			y /= rhs.y;
			z /= rhs.z;
			w /= rhs.w;
			return *this;
		}

		/// Parse from a string. Return true on success.
		bool FromString(const std::string& str);
		/// Parse from a C string. Return true on success.
		bool FromString(const char* str);

		/// Calculate dot product.
		float DotProduct(const Vector4& rhs) const { return x * rhs.x + y * rhs.y + z * rhs.z + w * rhs.w; }
		/// Calculate absolute dot product.
		float AbsDotProduct(const Vector4& rhs) const { return Alimer::Abs(x * rhs.x) + Alimer::Abs(y * rhs.y) + Alimer::Abs(z * rhs.z) + Alimer::Abs(w * rhs.w); }
		/// Return absolute vector.
		Vector4 Abs() const { return Vector4(Alimer::Abs(x), Alimer::Abs(y), Alimer::Abs(z), Alimer::Abs(w)); }
		/// Linear interpolation with another vector.
		Vector4 Lerp(const Vector4& rhs, float t) const { return *this * (1.0f - t) + rhs * t; }
		/// Test for equality with another vector with epsilon.
		bool Equals(const Vector4& rhs) const { return Alimer::Equals(x, rhs.x) && Alimer::Equals(y, rhs.y) && Alimer::Equals(z, rhs.z) && Alimer::Equals(w, rhs.w); }
		/// Return whether is NaN.
		bool IsNaN() const { return Alimer::IsNaN(x) || Alimer::IsNaN(y) || Alimer::IsNaN(z) || Alimer::IsNaN(w); }

		/// Return float data.
		const float* Data() const { return &x; }
		/// Return as string.
		std::string ToString() const;

		/// Zero vector.
		static const Vector4 ZERO;
		/// (1,1,1) vector.
		static const Vector4 ONE;
	};

	/// Multiply Vector4 with a scalar.
	inline Vector4 operator * (float lhs, const Vector4& rhs) { return rhs * lhs; }

}
