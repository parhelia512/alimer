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

#include "Matrix3.h"

namespace Alimer
{
	/// Rotation represented as a four-dimensional normalized vector.
	class ALIMER_API Quaternion
	{
	public:
		/// Construct undefined.
		Quaternion()
		{
		}

		/// Copy-construct.
		Quaternion(const Quaternion& quat)
			:  x(quat.x)
			, y(quat.y)
			, z(quat.z)
			, w(quat.w)
		{
		}

		/// Construct from values.
		Quaternion(float x_, float y_, float z_, float w_) 
			: x(x_), y(y_), z(z_), w(w_)
		{
		}

		/// Construct from a float array.
		Quaternion(const float* data) 
			: x(data[0])
			, y(data[1])
			, z(data[2])
			, w(data[3])
		{
		}

		/// Construct from an angle (in degrees) and axis.
		Quaternion(float angle, const Vector3& axis)
		{
			FromAngleAxis(angle, axis);
		}

		/// Construct from a rotation angle (in degrees) about the Z axis.
		Quaternion(float angle)
		{
			FromAngleAxis(angle, Vector3::FORWARD);
		}

		/// Construct from Euler angles (in degrees.)
		Quaternion(float x, float y, float z)
		{
			FromEulerAngles(x, y, z);
		}

		/// Construct from the rotation difference between two direction vectors.
		Quaternion(const Vector3& start, const Vector3& end)
		{
			FromRotationTo(start, end);
		}

		/// Construct from orthonormal axes.
		Quaternion(const Vector3& xAxis, const Vector3& yAxis, const Vector3& zAxis)
		{
			FromAxes(xAxis, yAxis, zAxis);
		}

		/// Construct from a rotation matrix.
		Quaternion(const Matrix3& matrix)
		{
			FromRotationMatrix(matrix);
		}

		/// Construct by parsing a string.
		Quaternion(const String& str)
		{
			FromString(str);
		}

		/// Construct by parsing a C string.
		Quaternion(const char* str)
		{
			FromString(str);
		}

		/// Assign from another quaternion.
		Quaternion& operator = (const Quaternion& rhs)
		{
			x = rhs.x;
			y = rhs.y;
			z = rhs.z;
			w = rhs.w;
			return *this;
		}

		/// Add-assign a quaternion.
		Quaternion& operator += (const Quaternion& rhs)
		{
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;
			w += rhs.w;
			return *this;
		}

		/// Multiply-assign a scalar.
		Quaternion& operator *= (float rhs)
		{
			x *= rhs;
			y *= rhs;
			z *= rhs;
			w *= rhs;
			return *this;
		}

		/// Test for equality with another quaternion without epsilon.
		bool operator == (const Quaternion& rhs) const { return w == rhs.w && x == rhs.x && y == rhs.y && z == rhs.z; }
		/// Test for inequality with another quaternion without epsilon.
		bool operator != (const Quaternion& rhs) const { return !(*this == rhs); }
		/// Multiply with a scalar.
		Quaternion operator * (float rhs) const { return Quaternion(x * rhs, y * rhs, z * rhs, w * rhs); }
		/// Return negation.
		Quaternion operator - () const { return Quaternion(-x, -y, -z, -w); }
		/// Add a quaternion.
		Quaternion operator + (const Quaternion& rhs) const { return Quaternion(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w); }
		/// Subtract a quaternion.
		Quaternion operator - (const Quaternion& rhs) const { return Quaternion(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w); }

		/// Multiply a quaternion.
		Quaternion operator * (const Quaternion& rhs) const
		{
			return Quaternion(
				w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y,
				w * rhs.y + y * rhs.w + z * rhs.x - x * rhs.z,
				w * rhs.z + z * rhs.w + x * rhs.y - y * rhs.x,
				w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z
			);
		}

		/// Multiply a Vector3.
		Vector3 operator * (const Vector3& rhs) const
		{
			Vector3 qVec(x, y, z);
			Vector3 cross1(qVec.CrossProduct(rhs));
			Vector3 cross2(qVec.CrossProduct(cross1));

			return rhs + 2.0f * (cross1 * w + cross2);
		}

		/// Define from an angle (in degrees) and axis.
		void FromAngleAxis(float angle, const Vector3& axis);
		/// Define from Euler angles (in degrees.)
		void FromEulerAngles(float x, float y, float z);
		/// Define from the rotation difference between two direction vectors.
		void FromRotationTo(const Vector3& start, const Vector3& end);
		/// Define from orthonormal axes.
		void FromAxes(const Vector3& xAxis, const Vector3& yAxis, const Vector3& zAxis);
		/// Define from a rotation matrix.
		void FromRotationMatrix(const Matrix3& matrix);
		/// Define from a direction to look in and an up direction. Return true on success, or false if would result in a NaN, in which case the current value remains.
		bool FromLookRotation(const Vector3& direction, const Vector3& up = Vector3::UP);
		/// Parse from a string. Return true on success.
		bool FromString(const String& str);
		/// Parse from a C string. Return true on success.
		bool FromString(const char* str);

		/// Normalize to unit length.
		void Normalize()
		{
			float lenSquared = LengthSquared();
			if (!Alimer::Equals(lenSquared, 1.0f) && lenSquared > 0.0f)
			{
				float invLen = 1.0f / sqrtf(lenSquared);
				w *= invLen;
				x *= invLen;
				y *= invLen;
				z *= invLen;
			}
		}

		/// Return normalized to unit length.
		Quaternion Normalized() const
		{
			float lenSquared = LengthSquared();
			if (!Alimer::Equals(lenSquared, 1.0f) && lenSquared > 0.0f)
			{
				float invLen = 1.0f / sqrtf(lenSquared);
				return *this * invLen;
			}
			else
				return *this;
		}

		/// Return inverse.
		Quaternion Inverse() const
		{
			float lenSquared = LengthSquared();
			if (lenSquared == 1.0f)
				return Conjugate();
			else if (lenSquared >= M_EPSILON)
				return Conjugate() * (1.0f / lenSquared);
			else
				return IDENTITY;
		}

		/// Return squared length.
		float LengthSquared() const { return w * w + x * x + y * y + z * z; }
		/// Calculate dot product.
		float DotProduct(const Quaternion& rhs) const { return w * rhs.w + x * rhs.x + y * rhs.y + z * rhs.z; }
		/// Test for equality with another quaternion with epsilon.
		bool Equals(const Quaternion& rhs) const { return Alimer::Equals(w, rhs.w) && Alimer::Equals(x, rhs.x) && Alimer::Equals(y, rhs.y) && Alimer::Equals(z, rhs.z); }
		/// Return whether is NaN.
		bool IsNaN() const { return Alimer::IsNaN(w) || Alimer::IsNaN(x) || Alimer::IsNaN(y) || Alimer::IsNaN(z); }
		/// Return conjugate.
		Quaternion Conjugate() const { return Quaternion(-x, -y, -z, w); }

		/// Return Euler angles in degrees.
		Vector3 EulerAngles() const;
		/// Return yaw angle in degrees.
		float YawAngle() const;
		/// Return pitch angle in degrees.
		float PitchAngle() const;
		/// Return roll angle in degrees.
		float RollAngle() const;
		/// Return the rotation matrix that corresponds to this quaternion.
		Matrix3 RotationMatrix() const;
		/// Spherical interpolation with another quaternion.
		Quaternion Slerp(Quaternion rhs, float t) const;
		/// Normalized linear interpolation with another quaternion.
		Quaternion Nlerp(Quaternion rhs, float t, bool shortestPath = false) const;
		/// Return float data.
		const float* Data() const { return &w; }
		/// Return as string.
		String ToString() const;

		
		/// X coordinate.
		float x;
		/// Y coordinate.
		float y;
		/// Z coordinate.
		float z;
		/// W coordinate.
		float w;

		/// Identity quaternion.
		static const Quaternion IDENTITY;
	};

}
