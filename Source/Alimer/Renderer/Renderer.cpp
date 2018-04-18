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
#include "../Graphics/ConstantBuffer.h"
#include "../Graphics/Graphics.h"
#include "../Graphics/Shader.h"
#include "../Graphics/ShaderVariation.h"
#include "../Graphics/Texture.h"
#include "../Graphics/VertexBuffer.h"
#include "../Resource/ResourceCache.h"
#include "../Scene/Scene.h"
#include "Light.h"
#include "Material.h"
#include "Model.h"
#include "Octree.h"
#include "Renderer.h"
#include "StaticModel.h"
#include <algorithm>

using namespace std;

namespace Alimer
{
	static const uint32_t LVS_GEOMETRY = (0x1 | 0x2);
	static const uint32_t LVS_NUMSHADOWCOORDS = (0x4 | 0x8 | 0x10);

	static const uint32_t LPS_AMBIENT = 0x1;
	static const uint32_t LPS_NUMSHADOWCOORDS = (0x2 | 0x4 | 0x8);
	static const uint32_t LPS_LIGHT0 = (0x10 | 0x20 | 0x40);
	static const uint32_t LPS_LIGHT1 = (0x80 | 0x100 | 0x200);
	static const uint32_t LPS_LIGHT2 = (0x400 | 0x800 | 0x1000);
	static const uint32_t LPS_LIGHT3 = (0x2000 | 0x4000 | 0x8000);

	static const CullMode cullModeFlip[] =
	{
		CULL_NONE,
		CULL_NONE,
		CULL_BACK,
		CULL_FRONT
	};

	const std::string geometryDefines[] =
	{
		"",
		"INSTANCED"
	};

	const std::string lightDefines[] =
	{
		"AMBIENT",
		"NUMSHADOWCOORDS",
		"DIRLIGHT",
		"POINTLIGHT",
		"SPOTLIGHT",
		"SHADOW"
	};

	inline bool CompareLights(Light* lhs, Light* rhs)
	{
		return lhs->GetDistance() < rhs->GetDistance();
	}

	Renderer::Renderer()
	{
	}

	Renderer::~Renderer()
	{
	}

	void Renderer::SetupShadowMaps(uint32_t num, uint32_t size, ImageFormat format)
	{
		if (size < 1)
			size = 1;
		size = NextPowerOfTwo(size);

		shadowMaps.resize(num);
		for (auto it = shadowMaps.begin(); it != shadowMaps.end(); ++it)
		{
			if (it->texture->Define(
				TextureType::Type2D,
				Size(size, size),
				format, 
				1,
				TextureUsage::ShaderRead | TextureUsage::RenderTarget))
			{
				// Setup shadow map sampling with hardware depth compare
				it->texture->DefineSampler(COMPARE_BILINEAR, ADDRESS_CLAMP, ADDRESS_CLAMP, ADDRESS_CLAMP, 1);
			}
		}
	}

	bool Renderer::PrepareView(
		Scene* scene_,
		Camera* camera_, 
		const std::vector<PassDesc>& passes)
	{
		if (!CollectObjects(scene_, camera_))
			return false;

		CollectLightInteractions();
		CollectBatches(passes);
		return true;
	}

	bool Renderer::CollectObjects(Scene* scene, Camera* camera_)
	{
		ALIMER_PROFILE(CollectObjects);

		// Acquire Graphics subsystem now, which needs to be initialized with a screen mode
		if (!graphics)
			Initialize();

		geometries.clear();
		_lights.clear();
		_instanceTransforms.clear();
		lightLists.clear();
		lightPasses.clear();
		for (auto it = batchQueues.begin(); it != batchQueues.end(); ++it)
			it->second.Clear();
		for (auto it = shadowMaps.begin(); it != shadowMaps.end(); ++it)
			it->Clear();
		usedShadowViews = 0;

		_scene = scene;
		_camera = camera_;
		_octree = scene ? _scene->FindChild<Octree>() : nullptr;
		if (!_scene || !_camera || !_octree)
			return false;

		// Increment frame number. Never use 0, as that is the default for objects that have never been rendered
		++_frameNumber;
		if (!_frameNumber)
			++_frameNumber;

		// Reinsert moved objects to the octree
		_octree->Update();

		_frustum = _camera->WorldFrustum();
		_viewMask = _camera->ViewMask();
		_octree->FindNodes(_frustum, this, &Renderer::CollectGeometriesAndLights);

		return true;
	}

	void Renderer::CollectLightInteractions()
	{
		ALIMER_PROFILE(CollectLightInteractions);

		{
			// Sort lights by increasing distance. Directional lights will have distance 0 which ensure they have the first
			// opportunity to allocate shadow maps
			ALIMER_PROFILE(SortLights);
			std::sort(_lights.begin(), _lights.end(), CompareLights);
		}

		for (auto it = _lights.begin(), end = _lights.end(); it != end; ++it)
		{
			Light* light = *it;
			unsigned lightMask = light->LightMask();

			litGeometries.clear();
			bool hasReceivers = false;

			// Create a light list that contains only this light. It will be used for nodes that have no light interactions so far
			uint64_t key = (uint64_t)light;
			LightList* lightList = &lightLists[key];
			lightList->lights.push_back(light);
			lightList->key = key;
			lightList->useCount = 0;

			switch (light->GetLightType())
			{
			case LIGHT_DIRECTIONAL:
				for (auto gIt = geometries.begin(), gEnd = geometries.end(); gIt != gEnd; ++gIt)
				{
					GeometryNode* node = *gIt;
					if (node->GetLayerMask() & lightMask)
					{
						AddLightToNode(node, light, lightList);
						hasReceivers = true;
					}
				}
				break;

			case LIGHT_POINT:
				_octree->FindNodes(reinterpret_cast<std::vector<OctreeNode*>&>(litGeometries), light->WorldSphere(), NF_ENABLED |
					NF_GEOMETRY, lightMask);
				for (auto gIt = litGeometries.begin(), gEnd = litGeometries.end(); gIt != gEnd; ++gIt)
				{
					GeometryNode* node = *gIt;
					// Add light only to nodes which are actually inside the frustum this frame
					if (node->GetLastFrameNumber() == _frameNumber)
					{
						AddLightToNode(node, light, lightList);
						hasReceivers = true;
					}
				}
				break;

			case LIGHT_SPOT:
				_octree->FindNodes(reinterpret_cast<std::vector<OctreeNode*>&>(litGeometries), light->WorldFrustum(), NF_ENABLED |
					NF_GEOMETRY, lightMask);
				for (auto gIt = litGeometries.begin(), gEnd = litGeometries.end(); gIt != gEnd; ++gIt)
				{
					GeometryNode* node = *gIt;
					if (node->GetLastFrameNumber() == _frameNumber)
					{
						AddLightToNode(node, light, lightList);
						hasReceivers = true;
					}
				}
				break;
			}

			if (!light->CastShadows() || !hasReceivers)
			{
				light->SetShadowMap(nullptr);
				continue;
			}

			// Try to allocate shadow map rectangle. Retry with smaller size two times if fails
			IntVector2 request = light->TotalShadowMapSize();
			IntRect shadowRect;
			size_t retries = 3;
			size_t index = 0;

			while (retries--)
			{
				for (index = 0; index < shadowMaps.size(); ++index)
				{
					ShadowMap& shadowMap = shadowMaps[index];
					int x, y;
					if (shadowMap.allocator.Allocate(request.x, request.y, x, y))
					{
						light->SetShadowMap(shadowMaps[index].texture, IntRect(x, y, x + request.x, y + request.y));
						break;
					}
				}

				if (index < shadowMaps.size())
					break;
				else
				{
					request.x /= 2;
					request.y /= 2;
				}
			}

			// If no room in any shadow map, render unshadowed
			if (index >= shadowMaps.size())
			{
				light->SetShadowMap(nullptr);
				continue;
			}

			// Setup shadow cameras & find shadow casters
			size_t startIndex = usedShadowViews;
			light->SetupShadowViews(_camera, shadowViews, usedShadowViews);
			bool hasShadowBatches = false;

			for (size_t i = startIndex; i < usedShadowViews; ++i)
			{
				ShadowView* view = shadowViews[i].get();
				Frustum shadowFrustum = view->shadowCamera.WorldFrustum();
				BatchQueue& shadowQueue = view->shadowQueue;
				shadowQueue.sort = SORT_STATE;
				shadowQueue.lit = false;
				shadowQueue.baseIndex = Material::GetPassIndex("shadow");
				shadowQueue.additiveIndex = 0;

				switch (light->GetLightType())
				{
				case LIGHT_DIRECTIONAL:
					// Directional light needs a new frustum query for each split, as the shadow cameras are typically far outside
					// the main view
					litGeometries.clear();
					_octree->FindNodes(reinterpret_cast<std::vector<OctreeNode*>&>(litGeometries),
						shadowFrustum, NF_ENABLED | NF_GEOMETRY | NF_CASTSHADOWS, light->LightMask());
					CollectShadowBatches(litGeometries, shadowQueue, shadowFrustum, false, false);
					break;

				case LIGHT_POINT:
					// Check which lit geometries are shadow casters and inside each shadow frustum. First check whether the
					// shadow frustum is inside the view at all
					/// \todo Could use a frustum-frustum test for more accuracy
					if (_frustum.IsInsideFast(BoundingBox(shadowFrustum)))
					{
						CollectShadowBatches(litGeometries, shadowQueue, shadowFrustum, true, true);
					}
					break;

				case LIGHT_SPOT:
					// For spot light only need to check which lit geometries are shadow casters
					CollectShadowBatches(litGeometries, shadowQueue, shadowFrustum, true, false);
					break;
				}

				shadowQueue.Sort(_instanceTransforms);

				// Mark shadow map for rendering only if it has a view with some batches
				if (shadowQueue.batches.size())
				{
					shadowMaps[index].shadowViews.push_back(view);
					shadowMaps[index].used = true;
					hasShadowBatches = true;
				}
			}

			if (!hasShadowBatches)
			{
				// Light did not have any shadow batches: convert to unshadowed and reuse the views
				light->SetShadowMap(nullptr);
				usedShadowViews = startIndex;
			}
		}

		{
			ALIMER_PROFILE(BuildLightPasses);

			for (auto it = lightLists.begin(), end = lightLists.end(); it != end; ++it)
			{
				LightList& list = it->second;
				if (!list.useCount)
					continue;

				// Sort lights according to the light pointer to prevent camera angle from changing the light list order and
				// causing extra shader variations to be compiled
				std::sort(list.lights.begin(), list.lights.end());

				size_t lightsLeft = list.lights.size();
				static std::vector<bool> lightDone;
				static std::vector<Light*> currentPass;
				lightDone.resize(lightsLeft);
				for (size_t i = 0; i < lightsLeft; ++i)
					lightDone[i] = false;

				size_t index = 0;
				while (lightsLeft)
				{
					// Find lights to the current pass, while obeying rules for shadow coord allocations (shadowed directional & spot
					// lights can not share a pass)
					currentPass.clear();
					size_t startIndex = index;
					size_t shadowCoordsLeft = MAX_LIGHTS_PER_PASS;
					while (lightsLeft && currentPass.size() < MAX_LIGHTS_PER_PASS)
					{
						if (!lightDone[index])
						{
							Light* light = list.lights[index];
							size_t shadowCoords = light->NumShadowCoords();
							if (shadowCoords <= shadowCoordsLeft)
							{
								lightDone[index] = true;
								currentPass.push_back(light);
								shadowCoordsLeft -= shadowCoords;
								--lightsLeft;
							}
						}

						index = (index + 1) % list.lights.size();
						if (index == startIndex)
							break;
					}

					uint64_t passKey = 0;
					for (size_t i = 0; i < currentPass.size(); ++i)
						passKey += (uint64_t)currentPass[i] << (i * 16);
					if (list.lightPasses.empty())
						++passKey; // First pass includes ambient light

					auto lpIt = lightPasses.find(passKey);
					if (lpIt != lightPasses.end())
					{
						list.lightPasses.push_back(&lpIt->second);
					}
					else
					{
						LightPass* newLightPass = &lightPasses[passKey];
						newLightPass->vsBits = 0;
						newLightPass->psBits = list.lightPasses.empty() ? LPS_AMBIENT : 0;
						for (size_t i = 0; i < MAX_LIGHTS_PER_PASS; ++i)
							newLightPass->shadowMaps[i] = nullptr;

						size_t numShadowCoords = 0;
						for (size_t i = 0; i < currentPass.size(); ++i)
						{
							Light* light = currentPass[i];
							newLightPass->psBits |= (light->GetLightType() + 1) << (i * 3 + 4);

							float cutoff = cosf(light->Fov() * 0.5f * M_DEGTORAD);
							newLightPass->lightPositions[i] = Vector4(light->WorldPosition(), 1.0f);
							newLightPass->lightDirections[i] = Vector4(-light->WorldDirection(), 0.0f);
							newLightPass->lightAttenuations[i] = Vector4(1.0f / Max(light->Range(), M_EPSILON), cutoff, 1.0f /
								(1.0f - cutoff), 0.0f);
							newLightPass->lightColors[i] = light->GetColor();

							if (light->ShadowMap())
							{
								// Enable shadowed shader variation, setup shadow parameters
								newLightPass->psBits |= 4 << (i * 3 + 4);
								newLightPass->shadowMaps[i] = light->ShadowMap();

								const std::vector<Matrix4>& shadowMatrices = light->ShadowMatrices();
								for (size_t j = 0; j < shadowMatrices.size() && numShadowCoords < MAX_LIGHTS_PER_PASS; ++j)
								{
									newLightPass->shadowMatrices[numShadowCoords++] = shadowMatrices[j];
								}

								newLightPass->shadowParameters[i] = light->ShadowParameters();

								if (light->GetLightType() == LIGHT_DIRECTIONAL)
								{
									float fadeStart = light->ShadowFadeStart() * light->MaxShadowDistance() / _camera->FarClip();
									float fadeRange = light->MaxShadowDistance() / _camera->FarClip() - fadeStart;
									newLightPass->dirShadowSplits = light->ShadowSplits() / _camera->FarClip();
									newLightPass->dirShadowFade = Vector4(fadeStart / fadeRange, 1.0f / fadeRange, 0.0f, 0.0f);
								}
								else if (light->GetLightType() == LIGHT_POINT)
									newLightPass->pointShadowParameters[i] = light->PointShadowParameters();
							}

							newLightPass->vsBits |= numShadowCoords << 2;
							newLightPass->psBits |= numShadowCoords << 1;
						}

						list.lightPasses.push_back(newLightPass);
					}
				}
			}
		}
	}

	void Renderer::CollectBatches(const std::vector<PassDesc>& passes)
	{
		ALIMER_PROFILE(CollectBatches);

		// Setup batch queues for each requested pass
		static std::vector<BatchQueue*> currentQueues;
		currentQueues.resize(passes.size());
		for (size_t i = 0; i < passes.size(); ++i)
		{
			const PassDesc& srcPass = passes[i];
			uint8_t baseIndex = Material::GetPassIndex(srcPass.name);
			BatchQueue* batchQueue = &batchQueues[baseIndex];
			currentQueues[i] = batchQueue;
			batchQueue->sort = srcPass.sort;
			batchQueue->lit = srcPass.lit;
			batchQueue->baseIndex = baseIndex;
			batchQueue->additiveIndex = srcPass.lit ? Material::GetPassIndex(srcPass.name + "add") : 0;
		}

		// Loop through geometry nodes
		for (auto gIt = geometries.begin(), gEnd = geometries.end(); gIt != gEnd; ++gIt)
		{
			GeometryNode* node = *gIt;
			LightList* lightList = node->GetLightList();

			Batch newBatch;
			newBatch.type = node->GetGeometryType();
			newBatch.worldMatrix = &node->WorldTransform();

			// Loop through node's geometries
			for (auto bIt = node->GetBatches().begin(), bEnd = node->GetBatches().end(); bIt != bEnd; ++bIt)
			{
				newBatch.geometry = bIt->geometry.Get();
				Material* material = bIt->material.Get();
				assert(material);

				// Loop through requested queues
				for (auto qIt = currentQueues.begin(); qIt != currentQueues.end(); ++qIt)
				{
					BatchQueue& batchQueue = **qIt;
					newBatch.pass = material->GetPass(batchQueue.baseIndex);
					// Material may not have the requested pass at all, skip further processing as fast as possible in that case
					if (!newBatch.pass)
						continue;

					newBatch.lights = batchQueue.lit ? lightList ? lightList->lightPasses[0] : &ambientLightPass : nullptr;
					if (batchQueue.sort < SORT_BACK_TO_FRONT)
						newBatch.CalculateSortKey();
					else
						newBatch.distance = node->GetDistance();

					batchQueue.batches.push_back(newBatch);

					// Create additive light batches if necessary
					if (batchQueue.lit
						&& lightList
						&& lightList->lightPasses.size() > 1)
					{
						newBatch.pass = material->GetPass(batchQueue.additiveIndex);
						if (!newBatch.pass)
							continue;

						for (size_t i = 1; i < lightList->lightPasses.size(); ++i)
						{
							newBatch.lights = lightList->lightPasses[i];
							if (batchQueue.sort != SORT_BACK_TO_FRONT)
							{
								newBatch.CalculateSortKey();
								batchQueue.additiveBatches.push_back(newBatch);
							}
							else
							{
								// In back-to-front mode base and additive batches must be mixed. Manipulate distance to make
								// the additive batches render later
								newBatch.distance = node->GetDistance() * 0.99999f;
								batchQueue.batches.push_back(newBatch);
							}
						}
					}
				}
			}
		}

		size_t oldSize = _instanceTransforms.size();

		for (auto qIt = currentQueues.begin(); qIt != currentQueues.end(); ++qIt)
		{
			BatchQueue& batchQueue = **qIt;
			batchQueue.Sort(_instanceTransforms);
		}

		// Check if more instances where added
		if (_instanceTransforms.size() != oldSize)
		{
			_instanceTransformsDirty = true;
		}
	}

	void Renderer::CollectBatches(const PassDesc& pass)
	{
		static vector<PassDesc> passDescs(1);
		passDescs[0] = pass;
		CollectBatches(passDescs);
	}

	void Renderer::RenderShadowMaps()
	{
		ALIMER_PROFILE(RenderShadowMaps);

		// Make sure the shadow maps are not bound on any unit
		graphics->ResetTextures();

		for (auto it = shadowMaps.begin(); it != shadowMaps.end(); ++it)
		{
			if (!it->used)
				continue;

			graphics->SetRenderTarget(nullptr, it->texture);
			graphics->Clear(ClearFlags::Depth, Color::BLACK, 1.0f);

			for (auto vIt = it->shadowViews.begin(); vIt < it->shadowViews.end(); ++vIt)
			{
				ShadowView* view = *vIt;
				Light* light = view->light;
				graphics->SetViewport(view->viewport);
				RenderBatches(view->shadowQueue.batches, &view->shadowCamera, true, true, light->DepthBias(), light->SlopeScaledDepthBias());
			}
		}
	}

	void Renderer::RenderBatches(const std::vector<PassDesc>& passes)
	{
		ALIMER_PROFILE(RenderBatches);

		for (size_t i = 0, count = passes.size(); i < count; ++i)
		{
			uint8_t passIndex = Material::GetPassIndex(passes[i].name);
			BatchQueue& batchQueue = batchQueues[passIndex];
			RenderBatches(batchQueue.batches, _camera, i == 0);
			RenderBatches(batchQueue.additiveBatches, _camera, false);
		}
	}

	void Renderer::RenderBatches(const std::string& pass)
	{
		ALIMER_PROFILE(RenderBatches);

		uint8_t passIndex = Material::GetPassIndex(pass);
		BatchQueue& batchQueue = batchQueues[passIndex];
		RenderBatches(batchQueue.batches, _camera);
		RenderBatches(batchQueue.additiveBatches, _camera, false);
	}

	void Renderer::Initialize()
	{
		graphics = GetSubsystem<Graphics>();
		assert(graphics && graphics->IsInitialized());

		std::vector<Constant> constants;

		vsFrameConstantBuffer = new ConstantBuffer();
		constants.push_back(Constant(ELEM_MATRIX3X4, "viewMatrix"));
		constants.push_back(Constant(ELEM_MATRIX4, "projectionMatrix"));
		constants.push_back(Constant(ELEM_MATRIX4, "viewProjMatrix"));
		constants.push_back(Constant(ELEM_VECTOR4, "depthParameters"));
		vsFrameConstantBuffer->Define(constants, true);

		psFrameConstantBuffer = new ConstantBuffer();
		constants.clear();
		constants.push_back(Constant(ELEM_VECTOR4, "ambientColor"));
		psFrameConstantBuffer->Define(constants, true);

		vsObjectConstantBuffer = new ConstantBuffer();
		constants.clear();
		constants.push_back(Constant(ELEM_MATRIX3X4, "worldMatrix"));
		vsObjectConstantBuffer->Define(constants, true);

		vsLightConstantBuffer = new ConstantBuffer();
		constants.clear();
		constants.push_back(Constant(ELEM_MATRIX4, "shadowMatrices", MAX_LIGHTS_PER_PASS));
		vsLightConstantBuffer->Define(constants, true);

		psLightConstantBuffer = new ConstantBuffer();
		constants.clear();
		constants.push_back(Constant(ELEM_VECTOR4, "lightPositions", MAX_LIGHTS_PER_PASS));
		constants.push_back(Constant(ELEM_VECTOR4, "lightDirections", MAX_LIGHTS_PER_PASS));
		constants.push_back(Constant(ELEM_VECTOR4, "lightColors", MAX_LIGHTS_PER_PASS));
		constants.push_back(Constant(ELEM_VECTOR4, "lightAttenuations", MAX_LIGHTS_PER_PASS));
		constants.push_back(Constant(ELEM_VECTOR4, "shadowParameters", MAX_LIGHTS_PER_PASS));
		constants.push_back(Constant(ELEM_VECTOR4, "pointShadowParameters", MAX_LIGHTS_PER_PASS));
		constants.push_back(Constant(ELEM_VECTOR4, "dirShadowSplits"));
		constants.push_back(Constant(ELEM_VECTOR4, "dirShadowFade"));
		psLightConstantBuffer->Define(constants, true);

		// Instance vertex buffer contains texcoords 4-6 which define the instances' world matrices
		_instanceVertexBuffer = std::make_unique<VertexBuffer>();
		_instanceVertexElements.emplace_back(ELEM_VECTOR4, SEM_TEXCOORD, INSTANCE_TEXCOORD, true);
		_instanceVertexElements.emplace_back(ELEM_VECTOR4, SEM_TEXCOORD, INSTANCE_TEXCOORD + 1, true);
		_instanceVertexElements.emplace_back(ELEM_VECTOR4, SEM_TEXCOORD, INSTANCE_TEXCOORD + 2, true);

		// Setup ambient light only -pass
		ambientLightPass.vsBits = 0;
		ambientLightPass.psBits = LPS_AMBIENT;

		// Setup point light face selection textures
		_faceSelectionTexture1 = make_unique<Texture>();
		_faceSelectionTexture2 = make_unique<Texture>();
		DefineFaceSelectionTextures();
	}

	void Renderer::DefineFaceSelectionTextures()
	{
		ALIMER_PROFILE(DefineFaceSelectionTextures);

		const float faceSelectionData1[] = {
			1.0f, 0.0f, 0.0f, 0.0f,
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f
		};

		const float faceSelectionData2[] = {
			-0.5f, 0.5f, 0.5f, 1.5f,
			0.5f, 0.5f, 0.5f, 0.5f,
			-0.5f, 0.5f, 1.5f, 1.5f,
			-0.5f, -0.5f, 1.5f, 0.5f,
			0.5f, 0.5f, 2.5f, 1.5f,
			-0.5f, 0.5f, 2.5f, 0.5f
		};

		std::vector<ImageLevel> faces1;
		std::vector<ImageLevel> faces2;
		for (size_t i = 0; i < MAX_CUBE_FACES; ++i)
		{
			ImageLevel level;
			level.rowSize = 4 * sizeof(float);
			level.data = (uint8_t*)&faceSelectionData1[4 * i];
			faces1.push_back(level);
			level.data = (uint8_t*)&faceSelectionData2[4 * i];
			faces2.push_back(level);
		}

		_faceSelectionTexture1->Define(TextureType::TypeCube, Size(1, 1), FMT_RGBA32F, 1, TextureUsage::ShaderRead, &faces1[0]);
		_faceSelectionTexture1->DefineSampler(FILTER_POINT, ADDRESS_CLAMP, ADDRESS_CLAMP, ADDRESS_CLAMP);
		_faceSelectionTexture1->SetDataLost(false);

		_faceSelectionTexture2->Define(TextureType::TypeCube, Size(1, 1), FMT_RGBA32F, 1, TextureUsage::ShaderRead, &faces2[0]);
		_faceSelectionTexture2->DefineSampler(FILTER_POINT, ADDRESS_CLAMP, ADDRESS_CLAMP, ADDRESS_CLAMP);
		_faceSelectionTexture2->SetDataLost(false);
	}

	void Renderer::CollectGeometriesAndLights(
		std::vector<OctreeNode*>::const_iterator begin,
		std::vector<OctreeNode*>::const_iterator end,
		bool inside)
	{
		if (inside)
		{
			for (auto it = begin; it != end; ++it)
			{
				OctreeNode* node = *it;
				uint16_t flags = node->GetFlags();
				if ((flags & NF_ENABLED) && (flags & (NF_GEOMETRY | NF_LIGHT)) && (node->GetLayerMask() & _viewMask))
				{
					if (flags & NF_GEOMETRY)
					{
						GeometryNode* geometry = static_cast<GeometryNode*>(node);
						geometry->OnPrepareRender(_frameNumber, _camera);
						geometries.push_back(geometry);
					}
					else
					{
						Light* light = static_cast<Light*>(node);
						light->OnPrepareRender(_frameNumber, _camera);
						_lights.push_back(light);
					}
				}
			}
		}
		else
		{
			for (auto it = begin; it != end; ++it)
			{
				OctreeNode* node = *it;
				uint16_t flags = node->GetFlags();
				if ((flags & NF_ENABLED)
					&& (flags & (NF_GEOMETRY | NF_LIGHT))
					&& (node->GetLayerMask() & _viewMask)
					&& _frustum.IsInsideFast(node->WorldBoundingBox()))
				{
					if (flags & NF_GEOMETRY)
					{
						GeometryNode* geometry = static_cast<GeometryNode*>(node);
						geometry->OnPrepareRender(_frameNumber, _camera);
						geometries.push_back(geometry);
					}
					else
					{
						Light* light = static_cast<Light*>(node);
						light->OnPrepareRender(_frameNumber, _camera);
						_lights.push_back(light);
					}
				}
			}
		}
	}

	void Renderer::AddLightToNode(GeometryNode* node, Light* light, LightList* lightList)
	{
		LightList* oldList = node->GetLightList();

		if (!oldList)
		{
			// First light assigned on this frame
			node->SetLightList(lightList);
			++lightList->useCount;
		}
		else
		{
			// Create new light list based on the node's existing one
			--oldList->useCount;
			uint64_t newListKey = oldList->key;
			newListKey += (uint64_t)light << ((oldList->lights.size() & 3) * 16);
			auto it = lightLists.find(newListKey);
			if (it != lightLists.end())
			{
				LightList* newList = &it->second;
				node->SetLightList(newList);
				++newList->useCount;
			}
			else
			{
				LightList* newList = &lightLists[newListKey];
				newList->key = newListKey;
				newList->lights = oldList->lights;
				newList->lights.push_back(light);
				newList->useCount = 1;
				node->SetLightList(newList);
			}
		}
	}

	void Renderer::CollectShadowBatches(
		const std::vector<GeometryNode*>& nodes,
		BatchQueue& batchQueue,
		const Frustum& frustum,
		bool checkShadowCaster,
		bool checkFrustum)
	{
		Batch newBatch;
		newBatch.lights = nullptr;

		for (auto gIt = nodes.begin(), gEnd = nodes.end(); gIt != gEnd; ++gIt)
		{
			GeometryNode* node = *gIt;
			if (checkShadowCaster && !(node->GetFlags() & NF_CASTSHADOWS))
				continue;
			if (checkFrustum && !frustum.IsInsideFast(node->WorldBoundingBox()))
				continue;

			// Node was possibly not in the main view. Update geometry first in that case
			if (node->GetLastFrameNumber() != _frameNumber)
			{
				node->OnPrepareRender(_frameNumber, _camera);
			}

			newBatch.type = node->GetGeometryType();
			newBatch.worldMatrix = &node->WorldTransform();

			// Loop through node's geometries
			for (auto bIt = node->GetBatches().begin(), bEnd = node->GetBatches().end(); bIt != bEnd; ++bIt)
			{
				newBatch.geometry = bIt->geometry.Get();
				Material* material = bIt->material.Get();
				assert(material);

				newBatch.pass = material->GetPass(batchQueue.baseIndex);
				// Material may not have the requested pass at all, skip further processing as fast as possible in that case
				if (!newBatch.pass)
					continue;

				newBatch.CalculateSortKey();
				batchQueue.batches.push_back(newBatch);
			}
		}
	}

	void Renderer::RenderBatches(
		const std::vector<Batch>& batches, 
		Camera* camera_, 
		bool setPerFrameConstants,
		bool overrideDepthBias,
		int depthBias, 
		float slopeScaledDepthBias)
	{
		if (_faceSelectionTexture1->IsDataLost() || _faceSelectionTexture2->IsDataLost())
			DefineFaceSelectionTextures();

		// Bind point light shadow face selection textures
		graphics->SetTexture(MAX_MATERIAL_TEXTURE_UNITS + MAX_LIGHTS_PER_PASS, _faceSelectionTexture1.get());
		graphics->SetTexture(MAX_MATERIAL_TEXTURE_UNITS + MAX_LIGHTS_PER_PASS + 1, _faceSelectionTexture2.get());

		// If rendering to a texture on OpenGL, flip the camera vertically to ensure similar texture coordinate addressing
#ifdef ALIMER_OPENGL
		bool flipVertical = camera_->FlipVertical();
		if (graphics->RenderTarget(0) || graphics->DepthStencil())
			camera_->SetFlipVertical(!flipVertical);
#endif

		if (setPerFrameConstants)
		{
			// Set per-frame values to the frame constant buffers
			Matrix3x4 viewMatrix = camera_->ViewMatrix();
			Matrix4 projectionMatrix = camera_->ProjectionMatrix();
			Matrix4 viewProjMatrix = projectionMatrix * viewMatrix;
			Vector4 depthParameters(Vector4::ZERO);
			depthParameters.x = camera_->NearClip();
			depthParameters.y = camera_->FarClip();
			if (camera_->IsOrthographic())
			{
#ifdef USE_OPENGL
				depthParameters.z = 0.5f;
				depthParameters.w = 0.5f;
#else
				depthParameters.z = 1.0f;
#endif
			}
			else
				depthParameters.w = 1.0f / _camera->FarClip();

			vsFrameConstantBuffer->SetConstant(VS_FRAME_VIEW_MATRIX, viewMatrix);
			vsFrameConstantBuffer->SetConstant(VS_FRAME_PROJECTION_MATRIX, projectionMatrix);
			vsFrameConstantBuffer->SetConstant(VS_FRAME_VIEWPROJ_MATRIX, viewProjMatrix);
			vsFrameConstantBuffer->SetConstant(VS_FRAME_DEPTH_PARAMETERS, depthParameters);
			vsFrameConstantBuffer->Apply();

			/// \todo Add also fog settings
			psFrameConstantBuffer->SetConstant(PS_FRAME_AMBIENT_COLOR, camera_->AmbientColor());
			psFrameConstantBuffer->Apply();

			graphics->SetConstantBuffer(SHADER_VS, CB_FRAME, vsFrameConstantBuffer);
			graphics->SetConstantBuffer(SHADER_PS, CB_FRAME, psFrameConstantBuffer);
		}

		if (_instanceTransformsDirty 
			&& _instanceTransforms.size())
		{
			if (_instanceVertexBuffer->GetVertexCount() < _instanceTransforms.size())
			{
				_instanceVertexBuffer->Define(
					ResourceUsage::Dynamic, 
					NextPowerOfTwo(static_cast<uint32_t>(_instanceTransforms.size())), 
					_instanceVertexElements, false);
			}

			_instanceVertexBuffer->SetData(0, static_cast<uint32_t>(_instanceTransforms.size()), _instanceTransforms.data());
			graphics->SetVertexBuffer(1, _instanceVertexBuffer.get());
			_instanceTransformsDirty = false;
		}

		{
			Pass* lastPass = nullptr;
			Material* lastMaterial = nullptr;
			LightPass* lastLights = nullptr;

			for (auto it = batches.begin(); it != batches.end();)
			{
				const Batch& batch = *it;
				bool instanced = batch.type == GEOM_INSTANCED;

				Pass* pass = batch.pass;
				if (!pass->shadersLoaded)
				{
					LoadPassShaders(pass);
				}

				// Check that pass is legal
				if (pass->shaders[SHADER_VS].Get() && pass->shaders[SHADER_PS].Get())
				{
					// Get the shader variations
					LightPass* lights = batch.lights;
					ShaderVariation* vs = FindShaderVariation(SHADER_VS, pass, (unsigned short)batch.type | (lights ? lights->vsBits : 0));
					ShaderVariation* ps = FindShaderVariation(SHADER_PS, pass, lights ? lights->psBits : 0);
					graphics->SetShaders(vs, ps);

					Geometry* geometry = batch.geometry;
					assert(geometry);

					// Apply pass render state
					if (pass != lastPass)
					{
						graphics->SetColorState(pass->blendMode, pass->alphaToCoverage, pass->colorWriteMask);

						if (!overrideDepthBias)
							graphics->SetDepthState(pass->depthFunc, pass->depthWrite, pass->depthClip);
						else
							graphics->SetDepthState(pass->depthFunc, pass->depthWrite, pass->depthClip, depthBias, slopeScaledDepthBias);

						if (!camera_->UseReverseCulling())
							graphics->SetRasterizerState(pass->cullMode, pass->fillMode);
						else
							graphics->SetRasterizerState(cullModeFlip[pass->cullMode], pass->fillMode);

						lastPass = pass;
					}

					// Apply material render state
					Material* material = pass->GetParent();
					if (material != lastMaterial)
					{
						for (size_t i = 0; i < MAX_MATERIAL_TEXTURE_UNITS; ++i)
						{
							if (material->textures[i])
								graphics->SetTexture(i, material->textures[i]);
						}
						graphics->SetConstantBuffer(SHADER_VS, CB_MATERIAL, material->constantBuffers[SHADER_VS].Get());
						graphics->SetConstantBuffer(SHADER_PS, CB_MATERIAL, material->constantBuffers[SHADER_PS].Get());

						lastMaterial = material;
					}

					// Apply object render state
					if (geometry->constantBuffers[SHADER_VS])
						graphics->SetConstantBuffer(SHADER_VS, CB_OBJECT, geometry->constantBuffers[SHADER_VS].Get());
					else if (!instanced)
					{
						vsObjectConstantBuffer->SetConstant(VS_OBJECT_WORLD_MATRIX, *batch.worldMatrix);
						vsObjectConstantBuffer->Apply();
						graphics->SetConstantBuffer(SHADER_VS, CB_OBJECT, vsObjectConstantBuffer.Get());
					}
					graphics->SetConstantBuffer(SHADER_PS, CB_OBJECT, geometry->constantBuffers[SHADER_PS].Get());

					// Apply light constant buffers and shadow maps
					if (lights && lights != lastLights)
					{
						// If light queue is ambient only, no need to update the constants
						if (lights->psBits > LPS_AMBIENT)
						{
							if (lights->vsBits & LVS_NUMSHADOWCOORDS)
							{
								vsLightConstantBuffer->SetData(lights->shadowMatrices);
								graphics->SetConstantBuffer(SHADER_VS, CB_LIGHTS, vsLightConstantBuffer.Get());
							}

							psLightConstantBuffer->SetData(lights->lightPositions);
							graphics->SetConstantBuffer(SHADER_PS, CB_LIGHTS, psLightConstantBuffer.Get());

							for (size_t i = 0; i < MAX_LIGHTS_PER_PASS; ++i)
								graphics->SetTexture(MAX_MATERIAL_TEXTURE_UNITS + i, lights->shadowMaps[i]);
						}

						lastLights = lights;
					}

					// Set vertex / index buffers and draw
					if (instanced)
						geometry->DrawInstanced(graphics, batch.instanceStart, batch.instanceCount);
					else
						geometry->Draw(graphics);
				}

				// Advance. If instanced, skip over the batches that were converted
				it += instanced ? batch.instanceCount : 1;
			}
		}

		// Restore original camera vertical flipping state now
#ifdef ALIMER_OPENGL
		camera_->SetFlipVertical(flipVertical);
#endif
	}

	void Renderer::LoadPassShaders(Pass* pass)
	{
		ALIMER_PROFILE(LoadPassShaders);

		ResourceCache* cache = GetSubsystem<ResourceCache>();
		// Use different extensions for GLSL & HLSL shaders
#ifdef ALIMER_OPENGL
		pass->shaders[SHADER_VS] = cache->LoadResource<Shader>(pass->GetShaderName(SHADER_VS) + ".vert");
		pass->shaders[SHADER_PS] = cache->LoadResource<Shader>(pass->GetShaderName(SHADER_PS) + ".frag");
#else
		pass->shaders[SHADER_VS] = cache->LoadResource<Shader>(pass->GetShaderName(SHADER_VS) + ".vs");
		pass->shaders[SHADER_PS] = cache->LoadResource<Shader>(pass->GetShaderName(SHADER_PS) + ".ps");
#endif

		pass->shadersLoaded = true;
	}

	ShaderVariation* Renderer::FindShaderVariation(ShaderStage stage, Pass* pass, unsigned short bits)
	{
		/// \todo Evaluate whether the hash lookup is worth the memory save vs using just straightforward vectors
		auto& variations = pass->shaderVariations[stage];
		auto it = variations.find(bits);

		if (it != variations.end())
			return it->second.Get();

		if (stage == SHADER_VS)
		{
			std::string vsString = pass->CombinedShaderDefines(stage) + " " + geometryDefines[bits & LVS_GEOMETRY];
			if (bits & LVS_NUMSHADOWCOORDS)
				vsString += " " + lightDefines[1] + "=" + std::to_string((bits & LVS_NUMSHADOWCOORDS) >> 2);

			auto vsVariation = pass->shaders[stage]->CreateVariation(str::Trim(vsString));
			variations.insert(std::make_pair(bits, WeakPtr<ShaderVariation>(vsVariation)));
			return vsVariation;
		}
		else
		{
			std::string psString = pass->CombinedShaderDefines(stage);
			if (bits & LPS_AMBIENT)
				psString += " " + lightDefines[0];
			if (bits & LPS_NUMSHADOWCOORDS)
				psString += " " + lightDefines[1] + "=" + std::to_string((bits & LPS_NUMSHADOWCOORDS) >> 1);
			for (size_t i = 0; i < MAX_LIGHTS_PER_PASS; ++i)
			{
				uint16_t lightBits = (bits >> (i * 3 + 4)) & 7;
				if (lightBits)
					psString += " " + lightDefines[(lightBits & 3) + 1] + std::to_string((int)i);
				if (lightBits & 4)
					psString += " " + lightDefines[5] + std::to_string((int)i);
			}

			auto fsVariation = pass->shaders[stage]->CreateVariation(str::Trim(psString));
			variations.insert(std::make_pair(bits, WeakPtr<ShaderVariation>(fsVariation)));
			return fsVariation;
		}
	}

	void RegisterRendererLibrary()
	{
		static bool registered = false;
		if (registered)
			return;
		registered = true;

		// Scene node base attributes are needed
		RegisterSceneLibrary();
		Octree::RegisterObject();
		Camera::RegisterObject();
		OctreeNode::RegisterObject();
		GeometryNode::RegisterObject();
		StaticModel::RegisterObject();
		Light::RegisterObject();
		Material::RegisterObject();
		Model::RegisterObject();
	}

}
