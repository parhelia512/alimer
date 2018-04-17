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

#include "../Debug/Log.h"
#include "../Graphics/ConstantBuffer.h"
#include "../Graphics/Graphics.h"
#include "../Graphics/IndexBuffer.h"
#include "../Graphics/VertexBuffer.h"
#include "../Resource/ResourceCache.h"
#include "Camera.h"
#include "GeometryNode.h"
#include "Material.h"

namespace Alimer
{
	Geometry::Geometry() 
		: primitiveType(TRIANGLE_LIST)
		, drawStart(0)
		, drawCount(0)
		, lodDistance(0.0f)
	{
	}

	Geometry::~Geometry()
	{
	}

	void Geometry::Draw(Graphics* graphics)
	{
		graphics->SetVertexBuffer(0, vertexBuffer.Get());
		if (indexBuffer.Get())
		{
			graphics->SetIndexBuffer(indexBuffer.Get());
			graphics->DrawIndexed(primitiveType, drawStart, drawCount, 0);
		}
		else
			graphics->Draw(primitiveType, drawStart, drawCount);
	}

	void Geometry::DrawInstanced(Graphics* graphics, uint32_t start, uint32_t count)
	{
		graphics->SetVertexBuffer(0, vertexBuffer.Get());
		if (indexBuffer.Get())
		{
			graphics->SetIndexBuffer(indexBuffer.Get());
			graphics->DrawIndexedInstanced(primitiveType, drawStart, drawCount, 0, start, count);
		}
		else
			graphics->DrawInstanced(primitiveType, drawStart, drawCount, start, count);
	}

	SourceBatch::SourceBatch()
	{
	}

	SourceBatch::~SourceBatch()
	{
	}

	GeometryNode::GeometryNode() :
		lightList(nullptr),
		geometryType(GEOM_STATIC)
	{
		SetFlag(NF_GEOMETRY, true);
	}

	GeometryNode::~GeometryNode()
	{
	}

	void GeometryNode::RegisterObject()
	{
		RegisterFactory<GeometryNode>();
		CopyBaseAttributes<GeometryNode, OctreeNode>();
		RegisterMixedRefAttribute("materials", &GeometryNode::MaterialsAttr, &GeometryNode::SetMaterialsAttr,
			ResourceRefList(Material::GetTypeStatic()));
	}

	void GeometryNode::OnPrepareRender(unsigned frameNumber, Camera* camera)
	{
		_lastFrameNumber = frameNumber;
		lightList = nullptr;
		_distance = camera->Distance(WorldPosition());
	}

	void GeometryNode::SetGeometryType(GeometryType type)
	{
		geometryType = type;
	}

	void GeometryNode::SetNumGeometries(size_t num)
	{
		_batches.resize(num);

		// Ensure non-null materials
		for (auto it = _batches.begin(); it != _batches.end(); ++it)
		{
			if (!it->material.Get())
				it->material = Material::GetDefaultMaterial();
		}
	}

	void GeometryNode::SetGeometry(size_t index, Geometry* geometry)
	{
		if (!geometry)
		{
			LOGERROR("Can not assign null geometry");
			return;
		}

		if (index < _batches.size())
			_batches[index].geometry = geometry;
		else
			LOGERRORF("Out of bounds batch index %d for setting geometry", (int)index);
	}

	void GeometryNode::SetMaterial(Material* material)
	{
		if (!material)
			material = Material::GetDefaultMaterial();

		for (size_t i = 0; i < _batches.size(); ++i)
			_batches[i].material = material;
	}

	void GeometryNode::SetMaterial(size_t index, Material* material)
	{
		if (index < _batches.size())
		{
			if (!material)
				material = Material::GetDefaultMaterial();
			_batches[index].material = material;
		}
		else
			LOGERRORF("Out of bounds batch index %d for setting material", (int)index);
	}

	void GeometryNode::SetLocalBoundingBox(const BoundingBox& box)
	{
		boundingBox = box;
		// Changing the bounding box may require octree reinsertion
		OctreeNode::OnTransformChanged();
	}

	Geometry* GeometryNode::GetGeometry(size_t index) const
	{
		return index < _batches.size() ? _batches[index].geometry.Get() : nullptr;
	}

	Material* GeometryNode::GetMaterial(size_t index) const
	{
		return index < _batches.size() ? _batches[index].material.Get() : nullptr;
	}

	void GeometryNode::OnWorldBoundingBoxUpdate() const
	{
		_worldBoundingBox = boundingBox.Transformed(WorldTransform());
		SetFlag(NF_BOUNDING_BOX_DIRTY, false);
	}

	void GeometryNode::SetMaterialsAttr(const ResourceRefList& materials)
	{
		ResourceCache* cache = Subsystem<ResourceCache>();
		for (size_t i = 0; i < materials.names.size(); ++i)
			SetMaterial(i, cache->LoadResource<Material>(materials.names[i]));
	}

	ResourceRefList GeometryNode::MaterialsAttr() const
	{
		ResourceRefList ret(Material::GetTypeStatic());

		ret.names.resize(_batches.size());
		for (size_t i = 0; i < _batches.size(); ++i)
		{
			ret.names[i] = ResourceName(_batches[i].material.Get());
		}

		return ret;
	}
}
