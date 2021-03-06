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
#include "../Debug/Profiler.h"
#include "../Graphics/IndexBuffer.h"
#include "../Graphics/VertexBuffer.h"
#include "../Scene/Node.h"
#include "GeometryNode.h"
#include "Material.h"
#include "Model.h"

namespace Alimer
{

	Bone::Bone() :
		initialPosition(Vector3::ZERO),
		initialRotation(Quaternion::IDENTITY),
		initialScale(Vector3::ONE),
		offsetMatrix(Matrix3x4::IDENTITY),
		radius(0.0f),
		boundingBox(0.0f, 0.0f),
		parentIndex(0),
		animated(true)
	{
	}

	Bone::~Bone()
	{
	}

	Model::Model()
	{
	}

	Model::~Model()
	{
	}

	void Model::RegisterObject()
	{
		RegisterFactory<Model>();
	}

	static Quaternion ReadQuaternionUrho(Stream& source)
	{
		float data[4];
		source.Read(data, sizeof data);
		return Quaternion(data[1], data[2], data[3], data[0]);
	}

	bool Model::BeginLoad(Stream& source)
	{
		/// \todo Develop own format for Alimer
		std::string fileID = source.ReadFileID();

		if (fileID != "UMDL"
			&& fileID != "UMD2")
		{
			ALIMER_LOGERROR(source.GetName() + " is not a valid model file");
			return false;
		}

		const bool hasVertexDeclarations = (fileID == "UMD2");

		vbDescs.clear();
		ibDescs.clear();
		geomDescs.clear();

		uint32_t numVertexBuffers = source.ReadUInt();
		vbDescs.resize(numVertexBuffers);
		for (size_t i = 0; i < numVertexBuffers; ++i)
		{
			VertexBufferDesc& vbDesc = vbDescs[i];

			vbDesc.vertexCount = source.ReadUInt();
			uint32_t elementMask = source.ReadUInt();
			source.ReadUInt(); // morphRangeStart
			source.ReadUInt(); // morphRangeCount

			size_t vertexSize = 0;
			if (elementMask & 1)
			{
				vbDesc.vertexElements.push_back(VertexElement(VertexFormat::Float3, VertexElementSemantic::POSITION));
				vertexSize += sizeof(Vector3);
			}
			if (elementMask & 2)
			{
				vbDesc.vertexElements.push_back(VertexElement(VertexFormat::Float3, VertexElementSemantic::NORMAL));
				vertexSize += sizeof(Vector3);
			}
			if (elementMask & 4)
			{
				vbDesc.vertexElements.push_back(VertexElement(VertexFormat::UByte4, VertexElementSemantic::COLOR));
				vertexSize += 4;
			}
			if (elementMask & 8)
			{
				vbDesc.vertexElements.push_back(VertexElement(VertexFormat::Float2, VertexElementSemantic::TEXCOORD));
				vertexSize += sizeof(Vector2);
			}
			if (elementMask & 16)
			{
				vbDesc.vertexElements.push_back(VertexElement(VertexFormat::Float2, VertexElementSemantic::TEXCOORD, 1));
				vertexSize += sizeof(Vector2);
			}
			if (elementMask & 32)
			{
				vbDesc.vertexElements.push_back(VertexElement(VertexFormat::Float3, VertexElementSemantic::TEXCOORD));
				vertexSize += sizeof(Vector3);
			}
			if (elementMask & 64)
			{
				vbDesc.vertexElements.push_back(VertexElement(VertexFormat::Float3, VertexElementSemantic::TEXCOORD, 1));
				vertexSize += sizeof(Vector3);
			}
			if (elementMask & 128)
			{
				vbDesc.vertexElements.push_back(VertexElement(VertexFormat::Float4, VertexElementSemantic::TANGENT));
				vertexSize += sizeof(Vector4);
			}
			if (elementMask & 256)
			{
				vbDesc.vertexElements.push_back(VertexElement(VertexFormat::Float4, VertexElementSemantic::BLENDWEIGHT));
				vertexSize += sizeof(Vector4);
			}
			if (elementMask & 512)
			{
				vbDesc.vertexElements.push_back(VertexElement(VertexFormat::UByte4, VertexElementSemantic::BLENDINDICES));
				vertexSize += 4;
			}

			vbDesc.vertexData.reset(new uint8_t[vbDesc.vertexCount * vertexSize]);
			source.Read(
				vbDesc.vertexData.get(), 
				vbDesc.vertexCount * vertexSize);
		}

		uint32_t numIndexBuffers = source.ReadUInt();
		ibDescs.resize(numIndexBuffers);
		for (size_t i = 0; i < numIndexBuffers; ++i)
		{
			IndexBufferDesc& ibDesc = ibDescs[i];

			ibDesc.indexCount = source.ReadUInt();
			uint32_t indexSize = source.ReadUInt();
			ibDesc.indexType = indexSize == 2 ? IndexType::UInt16 : IndexType::UInt32;
			ibDesc.indexData.reset(new uint8_t[ibDesc.indexCount * indexSize]);
			source.Read(ibDesc.indexData.get(), ibDesc.indexCount * indexSize);
		}

		size_t numGeometries = source.ReadUInt();

		geomDescs.resize(numGeometries);
		boneMappings.resize(numGeometries);
		for (size_t i = 0; i < numGeometries; ++i)
		{
			// Read bone mappings
			size_t boneMappingCount = source.ReadUInt();
			boneMappings[i].resize(boneMappingCount);
			/// \todo Should read as a batch
			for (size_t j = 0; j < boneMappingCount; ++j)
				boneMappings[i][j] = source.ReadUInt();

			size_t numLodLevels = source.ReadUInt();
			geomDescs[i].resize(numLodLevels);

			for (size_t j = 0; j < numLodLevels; ++j)
			{
				GeometryDesc& geomDesc = geomDescs[i][j];

				geomDesc.lodDistance = source.Read<float>();
				source.ReadUInt(); // Primitive type
				geomDesc.primitiveType = TRIANGLE_LIST; // Always assume triangle list for now
				geomDesc.vbRef = source.ReadUInt();
				geomDesc.ibRef = source.ReadUInt();
				geomDesc.drawStart = source.ReadUInt();
				geomDesc.drawCount = source.ReadUInt();
			}
		}

		// Read (skip) morphs
		size_t numMorphs = source.ReadUInt();
		if (numMorphs)
		{
			ALIMER_LOGERROR("Models with vertex morphs are not supported for now");
			return false;
		}

		// Read skeleton
		uint32_t numBones = source.ReadUInt();
		bones.resize(numBones);
		for (uint32_t i = 0; i < numBones; ++i)
		{
			Bone& bone = bones[i];
			bone.name = source.ReadString();
			bone.parentIndex = source.ReadUInt();
			bone.initialPosition = source.Read<Vector3>();

			// Read quaternion in wxyz.

			bone.initialRotation = ReadQuaternionUrho(source);
			bone.initialScale = source.Read<Vector3>();
			bone.offsetMatrix = source.Read<Matrix3x4>();

			uint8_t boneCollisionType = source.ReadUByte();
			if (boneCollisionType & 1)
				bone.radius = source.ReadFloat();
			if (boneCollisionType & 2)
				bone.boundingBox = source.Read<BoundingBox>();

			if (bone.parentIndex == i)
				rootBoneIndex = i;
		}

		// Read bounding box
		boundingBox = source.Read<BoundingBox>();

		return true;
	}

	bool Model::EndLoad()
	{
		std::vector<SharedPtr<VertexBuffer> > vbs;
		for (size_t i = 0; i < vbDescs.size(); ++i)
		{
			const VertexBufferDesc& vbDesc = vbDescs[i];
			SharedPtr<VertexBuffer> vb(new VertexBuffer());

			vb->Define(ResourceUsage::Immutable, vbDesc.vertexCount, vbDesc.vertexElements, true, vbDesc.vertexData.get());
			vbs.push_back(vb);
		}

		std::vector<SharedPtr<IndexBuffer> > ibs;
		for (size_t i = 0; i < ibDescs.size(); ++i)
		{
			const IndexBufferDesc& ibDesc = ibDescs[i];
			SharedPtr<IndexBuffer> ib(new IndexBuffer());

			ib->Define(
				ResourceUsage::Immutable,
				ibDesc.indexCount,
				ibDesc.indexType,
				true,
				ibDesc.indexData.get());
			ibs.push_back(ib);
		}

		// Set the buffers to each geometry
		geometries.resize(geomDescs.size());
		for (size_t i = 0; i < geomDescs.size(); ++i)
		{
			geometries[i].resize(geomDescs[i].size());
			for (size_t j = 0; j < geomDescs[i].size(); ++j)
			{
				const GeometryDesc& geomDesc = geomDescs[i][j];
				SharedPtr<Geometry> geom(new Geometry());

				geom->lodDistance = geomDesc.lodDistance;
				geom->primitiveType = geomDesc.primitiveType;
				geom->drawStart = geomDesc.drawStart;
				geom->drawCount = geomDesc.drawCount;

				if (geomDesc.vbRef < vbs.size())
					geom->vertexBuffer = vbs[geomDesc.vbRef];
				else
					ALIMER_LOGERROR("Out of range vertex buffer reference in " + GetName());

				if (geomDesc.ibRef < ibs.size())
					geom->indexBuffer = ibs[geomDesc.ibRef];
				else
					ALIMER_LOGERROR("Out of range index buffer reference in " + GetName());

				geometries[i][j] = geom;
			}
		}

		vbDescs.clear();
		ibDescs.clear();
		geomDescs.clear();

		return true;
	}

	void Model::SetNumGeometries(size_t num)
	{
		geometries.resize(num);
		// Ensure that each geometry has at least 1 LOD level
		for (size_t i = 0; i < geometries.size(); ++i)
		{
			if (!geometries[i].size())
				SetNumLodLevels(i, 1);
		}
	}

	void Model::SetNumLodLevels(size_t index, size_t num)
	{
		if (index >= geometries.size())
		{
			ALIMER_LOGERROR("Out of bounds geometry index for setting number of LOD levels");
			return;
		}

		geometries[index].resize(num);
		// Ensure that a valid geometry object exists at each index
		for (auto it = geometries[index].begin(); it != geometries[index].end(); ++it)
		{
			if (it->IsNull())
				*it = new Geometry();
		}
	}

	void Model::SetLocalBoundingBox(const BoundingBox& box)
	{
		boundingBox = box;
	}

	void Model::SetBones(const std::vector<Bone>& bones_, size_t rootBoneIndex_)
	{
		bones = bones_;
		rootBoneIndex = rootBoneIndex_;
	}

	void Model::SetBoneMappings(const std::vector<std::vector<size_t> >& boneMappings_)
	{
		boneMappings = boneMappings_;
	}

	size_t Model::NumLodLevels(size_t index) const
	{
		return index < geometries.size() ? geometries[index].size() : 0;
	}

	Geometry* Model::GetGeometry(size_t index, size_t lodLevel) const
	{
		return (index < geometries.size() && lodLevel < geometries[index].size()) ? geometries[index][lodLevel].Get() : nullptr;
	}
}
