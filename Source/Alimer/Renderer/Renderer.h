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

#include "../Graphics/Texture.h"
#include "../Math/Color.h"
#include "../Math/Frustum.h"
#include "../Resource/Image.h"
#include "Batch.h"

namespace Alimer
{
	class ConstantBuffer;
	class GeometryNode;
	class Octree;
	class Scene;
	class VertexBuffer;

	/// Shader constant buffers used by high-level rendering.
	enum RendererConstantBuffer
	{
		CB_FRAME = 0,
		CB_OBJECT,
		CB_MATERIAL,
		CB_LIGHTS
	};

	/// Parameter indices in constant buffers used by high-level rendering.
	static const uint32_t VS_FRAME_VIEW_MATRIX = 0;
	static const uint32_t VS_FRAME_PROJECTION_MATRIX = 1;
	static const uint32_t VS_FRAME_VIEWPROJ_MATRIX = 2;
	static const uint32_t VS_FRAME_DEPTH_PARAMETERS = 3;
	static const uint32_t VS_OBJECT_WORLD_MATRIX = 0;
	static const uint32_t VS_LIGHT_SHADOW_MATRICES = 0;
	static const uint32_t PS_FRAME_AMBIENT_COLOR = 0;
	static const uint32_t PS_LIGHT_POSITIONS = 0;
	static const uint32_t PS_LIGHT_DIRECTIONS = 1;
	static const uint32_t PS_LIGHT_ATTENUATIONS = 2;
	static const uint32_t PS_LIGHT_COLORS = 3;
	static const uint32_t PS_LIGHT_SHADOW_PARAMETERS = 4;
	static const uint32_t PS_LIGHT_DIR_SHADOW_SPLITS = 5;
	static const uint32_t PS_LIGHT_DIR_SHADOW_FADE = 6;
	static const uint32_t PS_LIGHT_POINT_SHADOW_PARAMETERS = 7;

	/// Texture coordinate index for the instance world matrix.
	static const uint32_t INSTANCE_TEXCOORD = 4;

	/// Description of a pass from the client to the renderer.
	struct ALIMER_API PassDesc
	{
		/// Construct undefined.
		PassDesc() = default;

		/// Construct with parameters.
		PassDesc(const std::string& name_, BatchSortMode sort_ = SORT_STATE, bool lit_ = true) :
			name(name_),
			sort(sort_),
			lit(lit_)
		{
		}

		/// %Pass name.
		std::string name{};
		/// Sorting mode.
		BatchSortMode sort{ SORT_NONE };
		/// Lighting flag.
		bool lit{};
	};

	/// High-level rendering subsystem. Performs rendering of 3D scenes.
	class ALIMER_API Renderer : public Object
	{
		ALIMER_OBJECT(Renderer, Object);

	public:
		/// Construct and register subsystem.
		Renderer();
		/// Destruct.
		~Renderer();

		/// Set number, size and format of shadow maps. These will be divided among the lights that need to render shadow maps.
		void SetupShadowMaps(uint32_t num, uint32_t size, PixelFormat format);
		/// Prepare a view for rendering. Convenience function that calls CollectObjects(), CollectLightInteractions() and CollectBatches() in one go. Return true on success.
		bool PrepareView(Scene* scene, Camera* camera, const std::vector<PassDesc>& passes);
		/// Initialize rendering of a new view and collect visible objects from the camera's point of view. Return true on success (scene, camera and octree are non-null.)
		bool CollectObjects(Scene* scene, Camera* camera);
		/// Collect light interactions with geometries from the current view. If lights are shadowed, collects batches for shadow casters.
		void CollectLightInteractions();
		/// Collect and sort batches from the visible objects. To not go through the objects several times, all the passes should be specified at once instead of multiple calls to CollectBatches().
		void CollectBatches(const std::vector<PassDesc>& passes);
		/// Collect and sort batches from the visible objects. Convenience function for one pass only.
		void CollectBatches(const PassDesc& pass);
		/// Render shadow maps. Should be called after all CollectBatches() calls but before RenderBatches(). Note that you must reassign your rendertarget and viewport after calling this.
		void RenderShadowMaps();
		/// Render several passes to the currently set rendertarget and viewport. Avoids setting the per-frame constants multiple times.
		void RenderBatches(const std::vector<PassDesc>& passes);
		/// Render a pass to the currently set rendertarget and viewport. Convenience function for one pass only.
		void RenderBatches(const std::string& pass);

		/// Per-frame vertex shader constant buffer.
		SharedPtr<ConstantBuffer> vsFrameConstantBuffer;
		/// Per-frame pixel shader constant buffer.
		SharedPtr<ConstantBuffer> psFrameConstantBuffer;
		/// Per-object vertex shader constant buffer.
		SharedPtr<ConstantBuffer> vsObjectConstantBuffer;
		/// Lights vertex shader constant buffer.
		SharedPtr<ConstantBuffer> vsLightConstantBuffer;
		/// Lights pixel shader constant buffer.
		SharedPtr<ConstantBuffer> psLightConstantBuffer;

	private:
		/// Initialize. Needs the Graphics subsystem and rendering context to exist.
		void Initialize();
		/// (Re)define face selection textures.
		void DefineFaceSelectionTextures();
		/// Octree callback for collecting lights and geometries.
		void CollectGeometriesAndLights(std::vector<OctreeNode*>::const_iterator begin, std::vector<OctreeNode*>::const_iterator end, bool inside);
		/// Assign a light list to a node. Creates new light lists as necessary to handle multiple lights.
		void AddLightToNode(GeometryNode* node, Light* light, LightList* lightList);
		/// Collect shadow caster batches.
		void CollectShadowBatches(const std::vector<GeometryNode*>& nodes, BatchQueue& batchQueue, const Frustum& frustum, bool checkShadowCaster, bool checkFrustum);
		/// Render batches from a specific queue and camera.
		void RenderBatches(const std::vector<Batch>& batches, Camera* camera, bool setPerFrameContants = true, bool overrideDepthBias = false, int depthBias = 0, float slopeScaledDepthBias = 0.0f);
		/// Load shaders for a pass.
		void LoadPassShaders(Pass* pass);
		/// Return or create a shader variation for a pass. Vertex shader variations handle different geometry types and pixel shader variations handle different light combinations.
		ShaderVariation* FindShaderVariation(ShaderStage stage, Pass* pass, unsigned short bits);

		/// Graphics subsystem pointer.
		WeakPtr<Graphics> graphics;
		/// Current scene.
		Scene* _scene;
		/// Current scene camera.
		Camera* _camera;
		/// Current octree.
		Octree* _octree;
		/// Camera's view frustum.
		Frustum _frustum;
		/// Camera's view mask.
		uint32_t _viewMask;
		/// Geometries in frustum.
		std::vector<GeometryNode*> geometries;
		/// Lights in frustum.
		std::vector<Light*> _lights;
		/// Batch queues per pass.
		std::map<uint8_t, BatchQueue> batchQueues;
		/// Instance transforms for uploading to the instance vertex buffer.
		std::vector<Matrix3x4> _instanceTransforms;
		/// Lit geometries query result.
		std::vector<GeometryNode*> litGeometries;
		/// %Light lists.
		std::map<uint64_t, LightList> lightLists;
		/// %Light passes.
		std::map<uint64_t, LightPass> lightPasses;
		/// Ambient only light pass.
		LightPass ambientLightPass;
		/// Current frame number.
		uint32_t _frameNumber{ 0 };
		/// Instance vertex buffer dirty flag.
		bool _instanceTransformsDirty{ false };
		/// Shadow maps.
		std::vector<ShadowMap> shadowMaps;
		/// Shadow views.
		std::vector<std::unique_ptr<ShadowView>> shadowViews;
		/// Used shadow views so far.
		size_t usedShadowViews;
		/// Instance transform vertex buffer.
		std::unique_ptr<VertexBuffer> _instanceVertexBuffer;
		/// Vertex elements for the instance vertex buffer.
		std::vector<VertexElement> _instanceVertexElements;
		/// First point light face selection cube map.
		std::unique_ptr<Texture> _faceSelectionTexture1;
		/// Second point light face selection cube map.
		std::unique_ptr<Texture> _faceSelectionTexture2;
	};

	/// Register Renderer related object factories and attributes.
	ALIMER_API void RegisterRendererLibrary();
}

