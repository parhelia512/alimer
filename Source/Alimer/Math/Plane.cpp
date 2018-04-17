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

#include "Plane.h"

namespace Alimer
{
	// Static initialization order can not be relied on, so do not use Vector3 constants
	const Plane Plane::UP(Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f));

	void Plane::Transform(const Matrix3& transform)
	{
		Define(Matrix4(transform).Inverse().Transpose() * ToVector4());
	}

	void Plane::Transform(const Matrix3x4& transform)
	{
		Define(transform.ToMatrix4().Inverse().Transpose() * ToVector4());
	}

	void Plane::Transform(const Matrix4& transform)
	{
		Define(transform.Inverse().Transpose() * ToVector4());
	}

	Matrix3x4 Plane::ReflectionMatrix() const
	{
		return Matrix3x4(
			-2.0f * normal.x * normal.x + 1.0f,
			-2.0f * normal.x * normal.y,
			-2.0f * normal.x * normal.z,
			-2.0f * normal.x * d,
			-2.0f * normal.y * normal.x,
			-2.0f * normal.y * normal.y + 1.0f,
			-2.0f * normal.y * normal.z,
			-2.0f * normal.y * d,
			-2.0f * normal.z * normal.x,
			-2.0f * normal.z * normal.y,
			-2.0f * normal.z * normal.z + 1.0f,
			-2.0f * normal.z * d
		);
	}

	Plane Plane::Transformed(const Matrix3& transform) const
	{
		return Plane(Matrix4(transform).Inverse().Transpose() * ToVector4());
	}

	Plane Plane::Transformed(const Matrix3x4& transform) const
	{
		return Plane(transform.ToMatrix4().Inverse().Transpose() * ToVector4());
	}

	Plane Plane::Transformed(const Matrix4& transform) const
	{
		return Plane(transform.Inverse().Transpose() * ToVector4());
	}
}
