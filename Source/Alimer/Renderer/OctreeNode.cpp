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

#include "../Math/Ray.h"
#include "../Scene/Scene.h"
#include "Camera.h"
#include "Octree.h"

namespace Alimer
{
	OctreeNode::OctreeNode()
	{
		SetFlag(NF_BOUNDING_BOX_DIRTY, true);
	}

	OctreeNode::~OctreeNode()
	{
		RemoveFromOctree();
	}

	void OctreeNode::RegisterObject()
	{
		CopyBaseAttributes<OctreeNode, SpatialNode>();
		RegisterAttribute("castShadows", &OctreeNode::CastShadows, &OctreeNode::SetCastShadows, false);
	}

	void OctreeNode::SetCastShadows(bool enable)
	{
		SetFlag(NF_CASTSHADOWS, enable);
	}

	void OctreeNode::OnPrepareRender(uint32_t frameNumber, Camera* camera)
	{
		_lastFrameNumber = frameNumber;
		_distance = camera->Distance(WorldPosition());
	}

	void OctreeNode::OnRaycast(std::vector<RaycastResult>& dest, const Ray& ray, float maxDistance)
	{
		float distance = ray.HitDistance(WorldBoundingBox());
		if (distance < maxDistance)
		{
			RaycastResult res;
			res.position = ray.origin + distance * ray.direction;
			res.normal = -ray.direction;
			res.distance = distance;
			res.node = this;
			res.subObject = 0;
			dest.push_back(res);
		}
	}

	void OctreeNode::OnSceneSet(Scene* newScene, Scene*)
	{
		/// Remove from current octree if any
		RemoveFromOctree();

		if (newScene)
		{
			// Octree must be attached to the scene root as a child
			_octree = newScene->FindChild<Octree>();
			// Transform may not be final yet. Schedule update but do not insert into octree yet
			if (_octree)
				_octree->QueueUpdate(this);
		}
	}

	void OctreeNode::OnTransformChanged()
	{
		SpatialNode::OnTransformChanged();
		SetFlag(NF_BOUNDING_BOX_DIRTY, true);

		if (!TestFlag(NF_OCTREE_UPDATE_QUEUED) && _octree)
		{
			_octree->QueueUpdate(this);
		}
	}

	void OctreeNode::OnWorldBoundingBoxUpdate() const
	{
		// The OctreeNode base class does not have a defined size, so represent as a point
		_worldBoundingBox.Define(WorldPosition());
		SetFlag(NF_BOUNDING_BOX_DIRTY, false);
	}

	void OctreeNode::RemoveFromOctree()
	{
		if (_octree)
		{
			_octree->RemoveNode(this);
			_octree = nullptr;
		}
	}

}