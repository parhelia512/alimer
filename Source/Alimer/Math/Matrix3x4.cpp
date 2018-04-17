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
#include "Matrix3x4.h"

#include <cstdio>
#include <cstdlib>

namespace Alimer
{
	const Matrix3x4 Matrix3x4::ZERO(
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f);

	const Matrix3x4 Matrix3x4::IDENTITY(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f);

	Matrix3x4::Matrix3x4(const Vector3& translation, const Quaternion& rotation, float scale)
	{
		SetRotation(rotation.RotationMatrix() * scale);
		SetTranslation(translation);
	}

	Matrix3x4::Matrix3x4(const Vector3& translation, const Quaternion& rotation, const Vector3& scale)
	{
		SetRotation(rotation.RotationMatrix().Scaled(scale));
		SetTranslation(translation);
	}

	bool Matrix3x4::FromString(const std::string& str)
	{
		return FromString(str.c_str());
	}

	bool Matrix3x4::FromString(const char* str)
	{
		size_t elements = str::CountElements(str, ' ');
		if (elements < 12)
			return false;

		char* ptr = (char*)str;
		m00 = (float)strtod(ptr, &ptr);
		m01 = (float)strtod(ptr, &ptr);
		m02 = (float)strtod(ptr, &ptr);
		m03 = (float)strtod(ptr, &ptr);
		m10 = (float)strtod(ptr, &ptr);
		m11 = (float)strtod(ptr, &ptr);
		m12 = (float)strtod(ptr, &ptr);
		m13 = (float)strtod(ptr, &ptr);
		m20 = (float)strtod(ptr, &ptr);
		m21 = (float)strtod(ptr, &ptr);
		m22 = (float)strtod(ptr, &ptr);
		m23 = (float)strtod(ptr, &ptr);

		return true;
	}

	void Matrix3x4::Decompose(Vector3& translation, Quaternion& rotation, Vector3& scale) const
	{
		translation.x = m03;
		translation.y = m13;
		translation.z = m23;

		scale.x = sqrtf(m00 * m00 + m10 * m10 + m20 * m20);
		scale.y = sqrtf(m01 * m01 + m11 * m11 + m21 * m21);
		scale.z = sqrtf(m02 * m02 + m12 * m12 + m22 * m22);

		Vector3 invScale(1.0f / scale.x, 1.0f / scale.y, 1.0f / scale.z);
		rotation = Quaternion(ToMatrix3().Scaled(invScale));
	}

	Matrix3x4 Matrix3x4::Inverse() const
	{
		float det = m00 * m11 * m22 +
			m10 * m21 * m02 +
			m20 * m01 * m12 -
			m20 * m11 * m02 -
			m10 * m01 * m22 -
			m00 * m21 * m12;

		float invDet = 1.0f / det;
		Matrix3x4 ret;

		ret.m00 = (m11 * m22 - m21 * m12) * invDet;
		ret.m01 = -(m01 * m22 - m21 * m02) * invDet;
		ret.m02 = (m01 * m12 - m11 * m02) * invDet;
		ret.m03 = -(m03 * ret.m00 + m13 * ret.m01 + m23 * ret.m02);
		ret.m10 = -(m10 * m22 - m20 * m12) * invDet;
		ret.m11 = (m00 * m22 - m20 * m02) * invDet;
		ret.m12 = -(m00 * m12 - m10 * m02) * invDet;
		ret.m13 = -(m03 * ret.m10 + m13 * ret.m11 + m23 * ret.m12);
		ret.m20 = (m10 * m21 - m20 * m11) * invDet;
		ret.m21 = -(m00 * m21 - m20 * m01) * invDet;
		ret.m22 = (m00 * m11 - m10 * m01) * invDet;
		ret.m23 = -(m03 * ret.m20 + m13 * ret.m21 + m23 * ret.m22);

		return ret;
	}

	std::string Matrix3x4::ToString() const
	{
		char tempBuffer[CONVERSION_BUFFER_LENGTH];
		sprintf(tempBuffer, "%g %g %g %g %g %g %g %g %g %g %g %g", m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22,
			m23);
		return std::string(tempBuffer);
	}
}
