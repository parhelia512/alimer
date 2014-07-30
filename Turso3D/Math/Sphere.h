//
// Copyright (c) 2008-2014 the Turso3D project.
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

namespace Turso3D
{

class BoundingBox;
class Polyhedron;
class Frustum;

/// %Sphere in three-dimensional space.
class TURSO3D_API Sphere
{
public:
    /// Sphere center.
    Vector3 center;
    /// Sphere radius.
    float radius;
    
    /// Construct with illegal size. This allows the first merge to set the initial size.
    Sphere() :
        center(Vector3::ZERO),
        radius(-M_INFINITY)
    {
    }
    
    /// Copy-construct.
    Sphere(const Sphere& sphere) :
        center(sphere.center),
        radius(sphere.radius)
    {
    }
    
    /// Construct from center and radius.
    Sphere(const Vector3& center_, float radius_) :
        center(center_),
        radius(radius_)
    {
    }
    
    /// Construct from an array of vertices.
    Sphere(const Vector3* vertices, size_t count)
    {
        Define(vertices, count);
    }
    
    /// Construct from a bounding box.
    Sphere(const BoundingBox& box)
    {
        Define(box);
    }
    
    /// Construct from a frustum.
    Sphere(const Frustum& frustum)
    {
        Define(frustum);
    }
    
    /// Construct from a polyhedron.
    Sphere(const Polyhedron& poly)
    {
        Define(poly);
    }
    
    /// Assign from another sphere.
    Sphere& operator = (const Sphere& rhs)
    {
        center = rhs.center;
        radius = rhs.radius;
        return *this;
    }
    
    /// Test for equality with another sphere without epsilon.
    bool operator == (const Sphere& rhs) const { return center == rhs.center && radius == rhs.radius; }
    /// Test for inequality with another sphere without epsilon.
    bool operator != (const Sphere& rhs) const { return !(*this == rhs); }
    
    /// Define from another sphere.
    void Define(const Sphere& sphere)
    {
        center = sphere.center;
        radius = sphere.radius;
    }
    
    /// Define from center and radius.
    void Define(const Vector3& center_, float radius_)
    {
        center = center_;
        radius = radius_;
    }
    
    /// Define from an array of vertices.
    void Define(const Vector3* vertices, size_t count);
    /// Define from a bounding box.
    void Define(const BoundingBox& box);
    /// Define from a frustum.
    void Define(const Frustum& frustum);
    /// Define from a polyhedron.
    void Define(const Polyhedron& poly);
    
    /// Merge a point.
    void Merge(const Vector3& point)
    {
        // If is illegal, set initial dimensions
        if (radius < 0.0f)
        {
            center = point;
            radius = 0.0f;
            return;
        }
        
        Vector3 offset = point - center;
        float dist = offset.Length();
        
        if (dist > radius)
        {
            float half = (dist - radius) * 0.5f;
            radius += half;
            center += (half / dist) * offset;
        }
    }
    
    /// Set illegal to allow the next merge to set initial size.
    void SetIllegal()
    {
        radius = -M_INFINITY;
    }
    
    /// Merge an array of vertices.
    void Merge(const Vector3* vertices, size_t count);
    /// Merge a bounding box.
    void Merge(const BoundingBox& box);
    /// Merge a frustum.
    void Merge(const Frustum& frustum);
    /// Merge a polyhedron.
    void Merge(const Polyhedron& poly);
    /// Merge a sphere.
    void Merge(const Sphere& sphere);
    
    /// Test if a point is inside.
    Intersection IsInside(const Vector3& point) const
    {
        float distSquared = (point - center).LengthSquared();
        if (distSquared < radius * radius)
            return INSIDE;
        else
            return OUTSIDE;
    }
    
    /// Test if another sphere is inside, outside or intersects.
    Intersection IsInside(const Sphere& sphere) const
    {
        float dist = (sphere.center - center).Length();
        if (dist >= sphere.radius + radius)
            return OUTSIDE;
        else if (dist + sphere.radius < radius)
            return INSIDE;
        else
            return INTERSECTS;
    }
    
    /// Test if another sphere is (partially) inside or outside.
    Intersection IsInsideFast(const Sphere& sphere) const
    {
        float distSquared = (sphere.center - center).LengthSquared();
        float combined = sphere.radius + radius;
        
        if (distSquared >= combined * combined)
            return OUTSIDE;
        else
            return INSIDE;
    }
    
    /// Test if a bounding box is inside, outside or intersects.
    Intersection IsInside(const BoundingBox& box) const;
    /// Test if a bounding box is (partially) inside or outside.
    Intersection IsInsideFast(const BoundingBox& box) const;
    
    /// Return distance of a point to the surface, or 0 if inside.
    float Distance(const Vector3& point) const { return Max((point - center).Length() - radius, 0.0f); }
};

}