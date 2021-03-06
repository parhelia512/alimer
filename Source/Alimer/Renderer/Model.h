// For conditions of distribution and use, see copyright notice in License.txt

#pragma once

#include "../Graphics/GraphicsDefs.h"
#include "../Math/BoundingBox.h"
#include "../Resource/Resource.h"

namespace Alimer
{

	class VertexBuffer;
	class IndexBuffer;
	struct Geometry;

	/// Load-time description of a vertex buffer, to be uploaded on the GPU later.
	struct ALIMER_API VertexBufferDesc
	{
		/// Vertex declaration.
		std::vector<VertexElement> vertexElements;
		/// Number of vertices.
		uint32_t vertexCount;
		/// Vertex data.
		std::unique_ptr<uint8_t[]> vertexData;
	};

	/// Load-time description of an index buffer, to be uploaded on the GPU later.
	struct ALIMER_API IndexBufferDesc
	{
		/// Index size.
		IndexType indexType;
		/// Number of indices.
		uint32_t indexCount;
		/// Index data.
		std::unique_ptr<uint8_t[]> indexData;
	};

	/// Load-time description of a geometry.
	struct ALIMER_API GeometryDesc
	{
		/// LOD distance.
		float lodDistance;
		/// Primitive type.
		PrimitiveType primitiveType;
		/// Vertex buffer ref.
		unsigned vbRef;
		/// Index buffer ref.
		unsigned ibRef;
		/// Draw range start.
		unsigned drawStart;
		/// Draw range element count.
		unsigned drawCount;
	};


	/// Model's bone description.
	struct ALIMER_API Bone
	{
		/// Default-construct.
		Bone();
		/// Destruct.
		~Bone();

		/// Name.
		std::string name;
		/// Reset position.
		Vector3 initialPosition;
		/// Reset rotation.
		Quaternion initialRotation;
		/// Reset scale.
		Vector3 initialScale;
		/// Offset matrix for skinning.
		Matrix3x4 offsetMatrix;
		/// Collision radius.
		float radius;
		/// Collision bounding box.
		BoundingBox boundingBox;
		/// Parent bone index.
		size_t parentIndex;
		/// Associated scene node.
		WeakPtr<Node> node;
		/// Animated flag.
		bool animated;
	};

	/// 3D model resource.
	class ALIMER_API Model : public Resource
	{
		ALIMER_OBJECT(Model, Resource);

	public:
		/// Construct.
		Model();
		/// Destruct.
		~Model();

		/// Register object factory.
		static void RegisterObject();

		/// Load model from a stream. Return true on success.
		bool BeginLoad(Stream& source) override;
		/// Finalize model loading in the main thread. Return true on success.
		bool EndLoad() override;

		/// Set number of geometries.
		void SetNumGeometries(size_t num);
		/// Set number of LOD levels in a geometry.
		void SetNumLodLevels(size_t index, size_t num);
		/// Set local space bounding box.
		void SetLocalBoundingBox(const BoundingBox& box);
		/// Set bones.
		void SetBones(const std::vector<Bone>& bones, size_t rootBoneIndex);
		/// Set per-geometry bone mappings.
		void SetBoneMappings(const std::vector<std::vector<size_t> >& boneMappings);

		/// Return number of geometries.
		size_t NumGeometries() const { return geometries.size(); }
		/// Return number of LOD levels in a geometry.
		size_t NumLodLevels(size_t index) const;
		/// Return the geometry at batch index and LOD level.
		Geometry* GetGeometry(size_t index, size_t lodLevel) const;
		/// Return the LOD geometries at batch index.
		const std::vector<SharedPtr<Geometry> >& GetLodGeometries(size_t index) const { return geometries[index]; }
		/// Return the local space bounding box.
		const BoundingBox& LocalBoundingBox() const { return boundingBox; }
		/// Return the model's bones.
		const std::vector<Bone>& GetBones() const { return bones; }
		/// Return the root bone index.
		size_t RootBoneIndex() const { return rootBoneIndex; }
		/// Return per-geometry bone mapping.
		const std::vector<std::vector<size_t>> GetBoneMappings() const { return boneMappings; }

	private:
		/// Geometry LOD levels.
		std::vector<std::vector<SharedPtr<Geometry> > > geometries;
		/// Local space bounding box.
		BoundingBox boundingBox;
		/// %Model's bones.
		std::vector<Bone> bones;
		/// Root bone index.
		size_t rootBoneIndex;
		/// Per-geometry bone mappings.
		std::vector<std::vector<size_t>> boneMappings;
		/// Vertex buffer data for loading.
		std::vector<VertexBufferDesc> vbDescs;
		/// Index buffer data for loading.
		std::vector<IndexBufferDesc> ibDescs;
		/// Geometry descriptions for loading.
		std::vector<std::vector<GeometryDesc>> geomDescs;
	};

}
