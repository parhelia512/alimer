//
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

#include "../../Debug/Log.h"
#include "../../Debug/Profiler.h"
#include "../../Window/Window.h"
#include "VulkanGraphics.h"
using namespace std;

namespace Alimer
{
	VulkanGraphics::VulkanGraphics(bool validation, const string& applicationName)
		: Graphics(GraphicsDeviceType::Vulkan, validation)
	{

	}

	VulkanGraphics::~VulkanGraphics()
	{
		WaitIdle();
		Finalize();
	}

	void VulkanGraphics::WaitIdle()
	{

	}

	bool VulkanGraphics::IsSupported()
	{
		static bool availableCheck = false;
		static bool isAvailable = false;

		if (availableCheck)
			return isAvailable;

		availableCheck = true;
		VkResult vkRes = volkInitialize();
		if (vkRes != VK_SUCCESS)
		{
			isAvailable = false;
			ALIMER_LOGERROR("Failed to initialize Vulkan");
			return false;
		}

		isAvailable = true;
		return true;
	}

	bool VulkanGraphics::Initialize(const GraphicsSettings& settings)
	{
		if (_initialized)
			return true;

		uint32_t multisample = Clamp(settings.multisample, 1, 16);

		// Create D3D11 device and swap chain when setting mode for the first time, or swap chain again when changing multisample
		if (!_device
			|| _multisample != multisample)
		{
		}

		screenModeEvent.size = _backbufferSize;
		screenModeEvent.fullscreen = settings.window->IsFullscreen();
		screenModeEvent.resizable = settings.window->IsResizable();
		screenModeEvent.multisample = multisample;
		SendEvent(screenModeEvent);

		ALIMER_LOGDEBUG("Set screen mode {}x{} fullscreen {} resizable {} multisample {}",
			_backbufferSize.width,
			_backbufferSize.height,
			settings.window->IsFullscreen(),
			settings.window->IsResizable(), 
			multisample);

		_initialized = true;
		return true;
	}

	void VulkanGraphics::Finalize()
	{
		// Release all GPU objects
		Graphics::Finalize();


		// Destroy logical device.
		if (_device != VK_NULL_HANDLE)
		{
			vkDestroyDevice(_device, nullptr);
			_device = VK_NULL_HANDLE;
		}
	}

	bool VulkanGraphics::BeginFrame()
	{
		if (!_initialized)
			return false;

		return true;
	}

	void VulkanGraphics::Present()
	{
		if (!_initialized)
			return;

	}

	void VulkanGraphics::SetRenderTargets(
		const std::vector<Texture*>& renderTargets,
		Texture* depthStencil)
	{
	}

	void VulkanGraphics::SetViewport(const IntRect& viewport_)
	{
		/// \todo Implement a member function in IntRect for clipping
		viewport.left = Clamp(viewport_.left, 0, _renderTargetSize.width - 1);
		viewport.top = Clamp(viewport_.top, 0, _renderTargetSize.height - 1);
		viewport.right = Clamp(viewport_.right, viewport.left + 1, _renderTargetSize.width);
		viewport.bottom = Clamp(viewport_.bottom, viewport.top + 1, _renderTargetSize.height);
	}

	void VulkanGraphics::SetVertexBuffer(
		uint32_t index,
		VertexBuffer* buffer,
		uint32_t vertexOffset,
		VertexInputRate stepRate)
	{
		if (index >= MaxVertexBuffers)
		{
			ALIMER_LOGERROR("SetVertexBuffer index out of bound");
			return;
		}
	}

	void VulkanGraphics::SetConstantBuffer(ShaderStage stage, uint32_t index, ConstantBuffer* buffer)
	{
		
	}

	void VulkanGraphics::SetTexture(size_t index, Texture* texture)
	{
		
	}

	void VulkanGraphics::SetIndexBufferCore(BufferHandle* handle, IndexType type)
	{
		
	}

	void VulkanGraphics::SetShaders(ShaderVariation* vs, ShaderVariation* ps)
	{
		
	}

	void VulkanGraphics::SetScissorTest(bool scissorEnable, const IntRect& scissorRect)
	{
		renderState.scissorEnable = scissorEnable;

		if (scissorRect != renderState.scissorRect)
		{
			/// \todo Implement a member function in IntRect for clipping
			renderState.scissorRect.left = Clamp(scissorRect.left, 0, _renderTargetSize.width - 1);
			renderState.scissorRect.top = Clamp(scissorRect.top, 0, _renderTargetSize.height - 1);
			renderState.scissorRect.right = Clamp(scissorRect.right, renderState.scissorRect.left + 1, _renderTargetSize.width);
			renderState.scissorRect.bottom = Clamp(scissorRect.bottom, renderState.scissorRect.top + 1, _renderTargetSize.height);
			
		}

		rasterizerStateDirty = true;
	}

	void VulkanGraphics::Clear(ClearFlags clearFlags, const Color& clearColor, float clearDepth, uint8_t clearStencil)
	{
		
	}

	void VulkanGraphics::Draw(PrimitiveType type, uint32_t vertexStart, uint32_t vertexCount)
	{
		if (!PrepareDraw(type))
			return;
	}

	void VulkanGraphics::DrawIndexed(PrimitiveType type, uint32_t indexStart, uint32_t indexCount, uint32_t vertexStart)
	{
		if (!PrepareDraw(type))
			return;
	}

	void VulkanGraphics::DrawInstanced(
		PrimitiveType type, 
		uint32_t vertexStart, 
		uint32_t vertexCount, 
		uint32_t instanceStart,
		uint32_t instanceCount)
	{
		if (!PrepareDraw(type))
			return;
	}

	void VulkanGraphics::DrawIndexedInstanced(
		PrimitiveType type, 
		uint32_t indexStart,
		uint32_t indexCount,
		uint32_t vertexStart,
		uint32_t instanceStart,
		uint32_t instanceCount)
	{
		if (!PrepareDraw(type))
			return;
	}

	BufferHandle* VulkanGraphics::CreateBuffer(BufferUsage usage, uint32_t size, uint32_t stride, ResourceUsage resourceUsage, const void* initialData)
	{
		return nullptr;
	}

	bool VulkanGraphics::PrepareDraw(PrimitiveType type)
	{
		return true;
	}
}

// Include and compile volk.c
#include "../../../ThirdParty/volk/volk.c"