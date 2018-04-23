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

#include "../Debug/Log.h"
#include "../Resource/ResourceCache.h"
#include "Camera.h"
#include "Material.h"
#include "Model.h"
#include "StaticModel.h"

namespace Alimer
{
	static Vector3 DOT_SCALE(1 / 3.0f, 1 / 3.0f, 1 / 3.0f);

	StaticModel::StaticModel() :
		lodBias(1.0f),
		hasLodLevels(false)
	{
	}

	StaticModel::~StaticModel()
	{
	}

	void StaticModel::RegisterObject()
	{
		RegisterFactory<StaticModel>();
		// Copy base attributes from OctreeNode instead of GeometryNode, as the model attribute needs to be set first so that
		// there is the correct amount of materials to assign
		CopyBaseAttributes<StaticModel, OctreeNode>();
		RegisterMixedRefAttribute("model", &StaticModel::ModelAttr, &StaticModel::SetModelAttr, ResourceRef(Model::GetTypeStatic()));
		CopyBaseAttribute<StaticModel, GeometryNode>("materials");
		RegisterAttribute("lodBias", &StaticModel::LodBias, &StaticModel::SetLodBias, 1.0f);
	}

	void StaticModel::OnPrepareRender(unsigned frameNumber, Camera* camera)
	{
		_lastFrameNumber = frameNumber;
		lightList = nullptr;
		_distance = camera->Distance(WorldPosition());

		// Find out the new LOD level if model has LODs
		if (hasLodLevels)
		{
			float lodDistance = camera->LodDistance(_distance, WorldScale().DotProduct(DOT_SCALE), lodBias);

			for (size_t i = 0; i < _batches.size(); ++i)
			{
				const auto& lodGeometries = model->GetLodGeometries(i);
				if (lodGeometries.size() > 1)
				{
					size_t j;
					for (j = 1; j < lodGeometries.size(); ++j)
					{
						if (lodDistance <= lodGeometries[j]->lodDistance)
							break;
					}
					_batches[i].geometry = lodGeometries[j - 1];
				}
			}
		}
	}

	void StaticModel::SetModel(Model* model_)
	{
		model = model_;
		hasLodLevels = false;

		if (!model)
		{
			SetNumGeometries(0);
			SetLocalBoundingBox(BoundingBox(0.0f, 0.0f));
			return;
		}

		SetNumGeometries(model->NumGeometries());
		// Start at LOD level 0
		for (uint32_t i = 0, count = static_cast<uint32_t>(_batches.size()); i < count; ++i)
		{
			SetGeometry(i, model->GetGeometry(i, 0));
			if (model->NumLodLevels(i) > 1)
				hasLodLevels = true;
		}

		SetLocalBoundingBox(model->LocalBoundingBox());
	}

	void StaticModel::SetLodBias(float bias)
	{
		lodBias = Max(bias, M_EPSILON);
	}

	Model* StaticModel::GetModel() const
	{
		return model.Get();
	}

	void StaticModel::SetModelAttr(const ResourceRef& model_)
	{
		ResourceCache* cache = GetSubsystem<ResourceCache>();
		SetModel(cache->LoadResource<Model>(model_.name));
	}

	ResourceRef StaticModel::ModelAttr() const
	{
		return ResourceRef(
			Model::GetTypeStatic(), 
			GetResourceName(model.Get()));
	}
}
