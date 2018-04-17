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
#include "BoundingBox.h"
#include "Frustum.h"
#include "Polyhedron.h"
#include "Sphere.h"

namespace Alimer
{
	void Sphere::Define(const Vector3* vertices, size_t count)
	{
		Undefine();
		Merge(vertices, count);
	}

	void Sphere::Define(const BoundingBox& box)
	{
		const Vector3& min = box.min;
		const Vector3& max = box.max;

		Undefine();
		Merge(min);
		Merge(Vector3(max.x, min.y, min.z));
		Merge(Vector3(min.x, max.y, min.z));
		Merge(Vector3(max.x, max.y, min.z));
		Merge(Vector3(min.x, min.y, max.z));
		Merge(Vector3(max.x, min.y, max.z));
		Merge(Vector3(min.x, max.y, max.z));
		Merge(max);
	}

	void Sphere::Define(const Frustum& frustum)
	{
		Define(frustum.vertices, NUM_FRUSTUM_VERTICES);
	}

	void Sphere::Define(const Polyhedron& poly)
	{
		Undefine();
		Merge(poly);
	}

	void Sphere::Merge(const Vector3* vertices, size_t count)
	{
		while (count--)
			Merge(*vertices++);
	}

	void Sphere::Merge(const BoundingBox& box)
	{
		const Vector3& min = box.min;
		const Vector3& max = box.max;

		Merge(min);
		Merge(Vector3(max.x, min.y, min.z));
		Merge(Vector3(min.x, max.y, min.z));
		Merge(Vector3(max.x, max.y, min.z));
		Merge(Vector3(min.x, min.y, max.z));
		Merge(Vector3(max.x, min.y, max.z));
		Merge(Vector3(min.x, max.y, max.z));
		Merge(max);
	}

	void Sphere::Merge(const Frustum& frustum)
	{
		const Vector3* vertices = frustum.vertices;
		Merge(vertices, NUM_FRUSTUM_VERTICES);
	}

	void Sphere::Merge(const Polyhedron& poly)
	{
		for (size_t i = 0; i < poly.faces.size(); ++i)
		{
			const std::vector<Vector3>& face = poly.faces[i];
			if (!face.empty())
				Merge(&face[0], face.size());
		}
	}

	void Sphere::Merge(const Sphere& sphere)
	{
		// If undefined, set initial dimensions
		if (!IsDefined())
		{
			center = sphere.center;
			radius = sphere.radius;
			return;
		}

		Vector3 offset = sphere.center - center;
		float dist = offset.Length();

		// If sphere fits inside, do nothing
		if (dist + sphere.radius < radius)
			return;

		// If we fit inside the other sphere, become it
		if (dist + radius < sphere.radius)
		{
			center = sphere.center;
			radius = sphere.radius;
		}
		else
		{
			Vector3 normalizedOffset = offset / dist;

			Vector3 min = center - radius * normalizedOffset;
			Vector3 max = sphere.center + sphere.radius * normalizedOffset;
			center = (min + max) * 0.5f;
			radius = (max - center).Length();
		}
	}

	Intersection Sphere::IsInside(const BoundingBox& box) const
	{
		float radiusSquared = radius * radius;
		float distSquared = 0;
		float temp;
		Vector3 min = box.min;
		Vector3 max = box.max;

		if (center.x < min.x)
		{
			temp = center.x - min.x;
			distSquared += temp * temp;
		}
		else if (center.x > max.x)
		{
			temp = center.x - max.x;
			distSquared += temp * temp;
		}
		if (center.y < min.y)
		{
			temp = center.y - min.y;
			distSquared += temp * temp;
		}
		else if (center.y > max.y)
		{
			temp = center.y - max.y;
			distSquared += temp * temp;
		}
		if (center.z < min.z)
		{
			temp = center.z - min.z;
			distSquared += temp * temp;
		}
		else if (center.z > max.z)
		{
			temp = center.z - max.z;
			distSquared += temp * temp;
		}

		if (distSquared >= radiusSquared)
			return OUTSIDE;

		min -= center;
		max -= center;

		Vector3 tempVec = min; // - - -
		if (tempVec.LengthSquared() >= radiusSquared)
			return INTERSECTS;
		tempVec.x = max.x; // + - -
		if (tempVec.LengthSquared() >= radiusSquared)
			return INTERSECTS;
		tempVec.y = max.y; // + + -
		if (tempVec.LengthSquared() >= radiusSquared)
			return INTERSECTS;
		tempVec.x = min.x; // - + -
		if (tempVec.LengthSquared() >= radiusSquared)
			return INTERSECTS;
		tempVec.z = max.z; // - + +
		if (tempVec.LengthSquared() >= radiusSquared)
			return INTERSECTS;
		tempVec.y = min.y; // - - +
		if (tempVec.LengthSquared() >= radiusSquared)
			return INTERSECTS;
		tempVec.x = max.x; // + - +
		if (tempVec.LengthSquared() >= radiusSquared)
			return INTERSECTS;
		tempVec.y = max.y; // + + +
		if (tempVec.LengthSquared() >= radiusSquared)
			return INTERSECTS;

		return INSIDE;
	}

	Intersection Sphere::IsInsideFast(const BoundingBox& box) const
	{
		float radiusSquared = radius * radius;
		float distSquared = 0;
		float temp;
		Vector3 min = box.min;
		Vector3 max = box.max;

		if (center.x < min.x)
		{
			temp = center.x - min.x;
			distSquared += temp * temp;
		}
		else if (center.x > max.x)
		{
			temp = center.x - max.x;
			distSquared += temp * temp;
		}
		if (center.y < min.y)
		{
			temp = center.y - min.y;
			distSquared += temp * temp;
		}
		else if (center.y > max.y)
		{
			temp = center.y - max.y;
			distSquared += temp * temp;
		}
		if (center.z < min.z)
		{
			temp = center.z - min.z;
			distSquared += temp * temp;
		}
		else if (center.z > max.z)
		{
			temp = center.z - max.z;
			distSquared += temp * temp;
		}

		if (distSquared >= radiusSquared)
			return OUTSIDE;
		else
			return INSIDE;
	}
}
