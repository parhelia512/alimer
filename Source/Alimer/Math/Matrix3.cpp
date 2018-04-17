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
#include "Matrix3.h"

#include <cstdio>
#include <cstdlib>

namespace Alimer
{
	const Matrix3 Matrix3::ZERO(
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f);

	const Matrix3 Matrix3::IDENTITY(
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f);

	bool Matrix3::FromString(const std::string& str)
	{
		return FromString(str.c_str());
	}

	bool Matrix3::FromString(const char* str)
	{
		size_t elements = str::CountElements(str, ' ');
		if (elements < 9)
			return false;

		char* ptr = (char*)str;
		m00 = (float)strtod(ptr, &ptr);
		m01 = (float)strtod(ptr, &ptr);
		m02 = (float)strtod(ptr, &ptr);
		m10 = (float)strtod(ptr, &ptr);
		m11 = (float)strtod(ptr, &ptr);
		m12 = (float)strtod(ptr, &ptr);
		m20 = (float)strtod(ptr, &ptr);
		m21 = (float)strtod(ptr, &ptr);
		m22 = (float)strtod(ptr, &ptr);

		return true;
	}

	Matrix3 Matrix3::Inverse() const
	{
		float det = m00 * m11 * m22 +
			m10 * m21 * m02 +
			m20 * m01 * m12 -
			m20 * m11 * m02 -
			m10 * m01 * m22 -
			m00 * m21 * m12;

		float invDet = 1.0f / det;

		return Matrix3(
			(m11 * m22 - m21 * m12) * invDet,
			-(m01 * m22 - m21 * m02) * invDet,
			(m01 * m12 - m11 * m02) * invDet,
			-(m10 * m22 - m20 * m12) * invDet,
			(m00 * m22 - m20 * m02) * invDet,
			-(m00 * m12 - m10 * m02) * invDet,
			(m10 * m21 - m20 * m11) * invDet,
			-(m00 * m21 - m20 * m01) * invDet,
			(m00 * m11 - m10 * m01) * invDet
		);
	}

	std::string Matrix3::ToString() const
	{
		char tempBuffer[CONVERSION_BUFFER_LENGTH];
		sprintf(tempBuffer, "%g %g %g %g %g %g %g %g %g", m00, m01, m02, m10, m11, m12, m20, m21, m22);
		return std::string(tempBuffer);
	}
}
