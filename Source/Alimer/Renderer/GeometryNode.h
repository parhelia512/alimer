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

#include "../Graphics/GraphicsDefs.h"
#include "../IO/ResourceRef.h"
#include "OctreeNode.h"

namespace Alimer
{
	class ConstantBuffer;
	class Graphics;
	class IndexBuffer;
	class Material;
	class VertexBuffer;
	struct LightList;

	/// Geometry types.
	enum GeometryType
	{
		GEOM_STATIC = 0,
		GEOM_INSTANCED
	};

	/// Description of geometry to be rendered. %Scene nodes that render the same object can share these to reduce memory load and allow instancing.
	struct ALIMER_API Geometry : public RefCounted
	{
		/// Default-construct.
		Geometry();
		/// Destruct.
		~Geometry();

		/// Draw using the Graphics subsystem. The constant buffers are not applied automatically, rather they must have been applied beforehand.
		void Draw(Graphics* graphics);
		/// Draw an instance range. A separate instance data vertex buffer must be bound.
		void DrawInstanced(Graphics* graphics, uint32_t start, uint32_t count);

		/// %Geometry vertex buffer.
		SharedPtr<VertexBuffer> vertexBuffer;
		/// %Geometry index buffer.
		SharedPtr<IndexBuffer> indexBuffer;
		/// Constant buffers.
		SharedPtr<ConstantBuffer> constantBuffers[static_cast<unsigned>(ShaderStage::Count)];
		/// %Geometry's primitive type.
		PrimitiveType primitiveType;
		/// Draw range start. Specifies index start if index buffer defined, vertex start otherwise.
		uint32_t drawStart;
		/// Draw range count. Specifies number of indices if index buffer defined, number of vertices otherwise.
		uint32_t drawCount;
		/// LOD transition distance.
		float lodDistance;
	};

	/// Draw call source data.
	struct ALIMER_API SourceBatch
	{
		/// Construct empty.
		SourceBatch();
		/// Destruct.
		~SourceBatch();

		/// The geometry to render. Must be non-null.
		SharedPtr<Geometry> geometry;
		/// The material to use for rendering. Must be non-null.
		SharedPtr<Material> material;
	};

	/// Base class for scene nodes that contain geometry to be rendered.
	class ALIMER_API GeometryNode : public OctreeNode
	{
		ALIMER_OBJECT(GeometryNode, OctreeNode);

	public:
		/// Construct.
		GeometryNode();
		/// Destruct.
		~GeometryNode();

		/// Register factory and attributes.
		static void RegisterObject();

		/// Prepare object for rendering. Reset framenumber and light list and calculate distance from camera. Called by Renderer.
		void OnPrepareRender(unsigned frameNumber, Camera* camera) override;

		/// Set geometry type, which is shared by all geometries.
		void SetGeometryType(GeometryType type);
		/// Set number of geometries.
		void SetNumGeometries(size_t num);
		/// Set geometry at index.
		void SetGeometry(uint32_t index, Geometry* geometry);
		/// Set material at every geometry index. Specifying null will use the default material (opaque white.)
		void SetMaterial(Material* material);
		/// Set material at geometry index.
		void SetMaterial(uint32_t index, Material* material);
		/// Set local space bounding box.
		void SetLocalBoundingBox(const BoundingBox& box);

		/// Return geometry type.
		GeometryType GetGeometryType() const { return geometryType; }
		/// Return number of geometries.
		size_t NumGeometries() const { return _batches.size(); }
		/// Return geometry by index.
		Geometry* GetGeometry(size_t index) const;
		/// Return material by geometry index.
		Material* GetMaterial(size_t index) const;
		/// Return source information for all draw calls.
		const std::vector<SourceBatch>& GetBatches() const { return _batches; }
		/// Return local space bounding box.
		const BoundingBox& LocalBoundingBox() const { return boundingBox; }

		/// Set new light list. Called by Renderer.
		void SetLightList(LightList* list) { lightList = list; }
		/// Return current light list.
		LightList* GetLightList() const { return lightList; }

	protected:
		/// Recalculate the world space bounding box.
		void OnWorldBoundingBoxUpdate() const override;
		/// Set materials list. Used in serialization.
		void SetMaterialsAttr(const ResourceRefList& materials);
		/// Return materials list. Used in serialization.
		ResourceRefList MaterialsAttr() const;

		/// %Light list for rendering.
		LightList* lightList;
		/// Geometry type.
		GeometryType geometryType;
		/// Draw call source datas.
		std::vector<SourceBatch> _batches;
		/// Local space bounding box.
		BoundingBox boundingBox;
	};

}
