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

#include "../Math/BoundingBox.h"
#include "../Scene/SpatialNode.h"

namespace Alimer
{

	class Camera;
	class Octree;
	class Ray;
	struct Octant;
	struct RaycastResult;

	/// Base class for scene nodes that insert themselves to the octree for rendering.
	class ALIMER_API OctreeNode : public SpatialNode
	{
		friend class Octree;

		ALIMER_OBJECT(OctreeNode, SpatialNode);

	public:
		/// Construct.
		OctreeNode();
		/// Destruct. Remove self from the octree.
		~OctreeNode();

		/// Register attributes.
		static void RegisterObject();

		/// Prepare object for rendering. Reset framenumber and calculate distance from camera. Called by Renderer.
		virtual void OnPrepareRender(uint32_t frameNumber, Camera* camera);
		/// Perform ray test on self and add possible hit to the result vector.
		virtual void OnRaycast(std::vector<RaycastResult>& dest, const Ray& ray, float maxDistance);

		/// Set whether to cast shadows. Default false on both lights and geometries.
		void SetCastShadows(bool enable);

		/// Return world space bounding box. Update if necessary.
		const BoundingBox& WorldBoundingBox() const { if (TestFlag(NF_BOUNDING_BOX_DIRTY)) OnWorldBoundingBoxUpdate(); return _worldBoundingBox; }
		/// Return whether casts shadows.
		bool CastShadows() const { return TestFlag(NF_CASTSHADOWS); }
		/// Return current octree this node resides in.
		Octree* GetOctree() const { return _octree; }
		/// Return current octree octant this node resides in.
		Octant* GetOctant() const { return _octant; }
		/// Return distance from camera in the current view.
		float GetDistance() const { return _distance; }
		/// Return last frame number when was visible. The frames are counted by Renderer internally and have no significance outside it.
		uint32_t GetLastFrameNumber() const { return _lastFrameNumber; }

	protected:
		/// Search for an octree from the scene root and add self to it.
		void OnSceneSet(Scene* newScene, Scene* oldScene) override;
		/// Handle the transform matrix changing.
		void OnTransformChanged() override;
		/// Recalculate the world space bounding box.
		virtual void OnWorldBoundingBoxUpdate() const;

		/// World space bounding box.
		mutable BoundingBox _worldBoundingBox;
		/// Distance from camera in the current view.
		float _distance;
		/// Last frame number when was visible.
		uint32_t _lastFrameNumber;

	private:
		/// Remove from the current octree.
		void RemoveFromOctree();

		/// Current octree.
		Octree* _octree;
		/// Current octree octant.
		Octant* _octant;
	};
}
