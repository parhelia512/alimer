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
#include "../Window/Window.h"
#include "Graphics.h"
#include "GraphicsImpl.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Texture.h"

#ifdef ALIMER_D3D11
#	include "D3D11/D3D11Graphics.h"
#endif

#ifdef ALIMER_VULKAN
#	include "Vulkan/VulkanGraphics.h"
#endif

#include <algorithm>
using namespace std;

namespace Alimer
{
	Graphics::Graphics(GraphicsDeviceType deviceType, bool validation)
		: _deviceType(deviceType)
		, _validation(validation)
	{
		RegisterSubsystem(this);
	}

	Graphics::~Graphics()
	{
		Finalize();
		RemoveSubsystem(this);
	}

	/// Sort resources by type.
	//static bool SortResources(GPUObject* x, GPUObject* y)
	//{
	//	return x->GetType() < y->GetType();
	//}

	void Graphics::Finalize()
	{
		if (!_initialized)
			return;

		if (_gpuObjects.size())
		{
			lock_guard<mutex> lock(_gpuResourceMutex);

			// Release all GPU objects that still exist
			//std::sort(gpuObjects.begin(), gpuObjects.end(), SortResources);
			for (size_t i = 0; i < _gpuObjects.size(); ++i)
			{
				GPUObject* resource = _gpuObjects.at(i);
				assert(resource);
				resource->Release();
			}

			_gpuObjects.clear();
		}

		_initialized = false;
	}

	bool Graphics::IsInitialized() const
	{
		return _initialized;
	}

	void Graphics::SetIndexBuffer(IndexBuffer* buffer)
	{
		if (buffer)
		{
			SetIndexBufferCore(buffer->GetHandle(), buffer->GetIndexType());
		}
		else
		{
			SetIndexBufferCore(nullptr, IndexType::UInt16);
		}
	}

	void Graphics::SetRenderTarget(Texture* renderTarget, Texture* depthStencil)
	{
		static std::vector<Texture*> renderTargetVector(1);
		renderTargetVector[0] = renderTarget;
		SetRenderTargets(renderTargetVector, depthStencil);
	}

	void Graphics::SetColorState(const BlendModeDesc& blendMode, bool alphaToCoverage, unsigned char colorWriteMask)
	{
		renderState.blendMode = blendMode;
		renderState.colorWriteMask = colorWriteMask;
		renderState.alphaToCoverage = alphaToCoverage;

		blendStateDirty = true;
	}

	void Graphics::SetColorState(BlendMode blendMode, bool alphaToCoverage, unsigned char colorWriteMask)
	{
		renderState.blendMode = blendModes[blendMode];
		renderState.colorWriteMask = colorWriteMask;
		renderState.alphaToCoverage = alphaToCoverage;

		blendStateDirty = true;
	}

	void Graphics::SetDepthState(CompareFunc depthFunc, bool depthWrite, bool depthClip, int depthBias, float slopeScaledDepthBias)
	{
		renderState.depthFunc = depthFunc;
		renderState.depthWrite = depthWrite;
		renderState.depthClip = depthClip;
		renderState.depthBias = depthBias;
		renderState.slopeScaledDepthBias = slopeScaledDepthBias;

		depthStateDirty = true;
		rasterizerStateDirty = true;
	}

	void Graphics::SetRasterizerState(CullMode cullMode, FillMode fillMode)
	{
		renderState.cullMode = cullMode;
		renderState.fillMode = fillMode;

		rasterizerStateDirty = true;
	}

	void Graphics::AddGPUObject(GPUObject* object)
	{
		if (object)
			_gpuObjects.push_back(object);
	}

	void Graphics::RemoveGPUObject(GPUObject* object)
	{
		auto it = std::find(_gpuObjects.begin(), _gpuObjects.end(), object);
		if (it != end(_gpuObjects))
		{
			_gpuObjects.erase(it);
		}
	}


	std::vector<GraphicsDeviceType> Graphics::GetAvailableDrivers()
	{
		static std::vector<GraphicsDeviceType> availableDrivers;

		if (availableDrivers.empty())
		{
			availableDrivers.push_back(GraphicsDeviceType::Empty);

#ifdef ALIMER_D3D11
			if (D3D11Graphics::IsSupported())
			{
				availableDrivers.push_back(GraphicsDeviceType::Direct3D11);
			}
#endif

#ifdef ALIMER_VULKAN
			if (VulkanGraphics::IsSupported())
			{
				availableDrivers.push_back(GraphicsDeviceType::Vulkan);
			}
#endif

			// TODO: Add more
		}

		return availableDrivers;
	}

	bool Graphics::IsBackendSupported(GraphicsDeviceType deviceType)
	{
		switch (deviceType)
		{
		case GraphicsDeviceType::Empty:
			return true;

		case GraphicsDeviceType::Direct3D11:
#ifdef ALIMER_D3D11
			if (D3D11Graphics::IsSupported())
			{
				return true;
			}
#endif
			return false;

		case GraphicsDeviceType::Vulkan:
#ifdef ALIMER_VULKAN
			if (VulkanGraphics::IsSupported())
			{
				return true;
			}
#endif
			return false;

		case GraphicsDeviceType::OpenGL:
#ifdef ALIMER_OPENGL
			if (GraphicsGL::IsSupported())
			{
				return true;
			}
#endif
			return false;

		default:
			return false;
		}
	}

	Graphics* Graphics::Create(GraphicsDeviceType deviceType, bool validation, const string& applicationName)
	{
		if (deviceType == GraphicsDeviceType::Default)
		{
			auto availableDrivers = Graphics::GetAvailableDrivers();

			if (std::find(availableDrivers.begin(), availableDrivers.end(), GraphicsDeviceType::Vulkan) != availableDrivers.end())
			{
				deviceType = GraphicsDeviceType::Vulkan;
			}
			else if (std::find(availableDrivers.begin(), availableDrivers.end(), GraphicsDeviceType::Direct3D11) != availableDrivers.end())
			{
				deviceType = GraphicsDeviceType::Direct3D11;
			}
			else if (std::find(availableDrivers.begin(), availableDrivers.end(), GraphicsDeviceType::OpenGL) != availableDrivers.end())
			{
				deviceType = GraphicsDeviceType::OpenGL;
			}
			else
			{
				deviceType = GraphicsDeviceType::Empty;
			}
		}

		Graphics* graphics = nullptr;
		switch (deviceType)
		{
		case GraphicsDeviceType::Empty: {
			ALIMER_LOGINFO("Using empty graphics backend.");
			break;
		}

		case GraphicsDeviceType::Direct3D11: {
#ifdef ALIMER_D3D11
			ALIMER_LOGINFO("Using Direct3D11 graphics backend.");
			graphics = new D3D11Graphics(validation, applicationName);
#else
			ALIMER_LOGWARN("Direct3D11 backend not supported on given platform.");
#endif
			break;
		}

		case GraphicsDeviceType::Vulkan: {
#ifdef ALIMER_VULKAN
			ALIMER_LOGINFO("Using Vulkan graphics backend.");
			graphics = new VulkanGraphics(validation, applicationName);
#else
			ALIMER_LOGWARN("Vulkan backend not supported on given platform.");
#endif
			break;
		}
		}

		return graphics;
	}

	void RegisterGraphicsLibrary()
	{
		static bool registered = false;
		if (registered)
			return;
		registered = true;

		Shader::RegisterObject();
		Texture::RegisterObject();
	}
}
