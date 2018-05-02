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

#pragma once

#include "../Graphics.h"
#include "../../Base/HashMap.h"
#include "VulkanPrerequisites.h"

namespace Alimer
{
	/// Vulkan graphics backend.
	class VulkanGraphics final : public Graphics
	{
	public:
		/// Construct. Set parent shader and defines but do not compile yet.
		VulkanGraphics(bool validation, const std::string& applicationName);
		/// Destruct.
		~VulkanGraphics() override;

		/// Is backend supported?
		static bool IsSupported();

		bool Initialize(const GraphicsSettings& settings) override;

		// 
		void SetRenderTargets(const std::vector<Texture*>& renderTargets, Texture* stencilBuffer) override;
		void SetViewport(const IntRect& viewport) override;
		void SetVertexBuffer(
			uint32_t index,
			VertexBuffer* buffer,
			uint32_t vertexOffset,
			VertexInputRate stepRate) override;
		void SetConstantBuffer(ShaderStage stage, uint32_t index, ConstantBuffer* buffer) override;
		void SetTexture(size_t index, Texture* texture) override;
		void SetShaders(ShaderVariation* vs, ShaderVariation* ps) override;
		void SetScissorTest(bool scissorEnable, const IntRect& scissorRect) override;

		void Clear(ClearFlags clearFlags, const Color& clearColor, float clearDepth, uint8_t clearStencil) override;
		void Draw(PrimitiveType type, uint32_t vertexStart, uint32_t vertexCount) override;
		void DrawIndexed(PrimitiveType type, uint32_t indexStart, uint32_t indexCount, uint32_t vertexStart) override;
		void DrawInstanced(PrimitiveType type, uint32_t vertexStart, uint32_t vertexCount, uint32_t instanceStart, uint32_t instanceCount) override;
		void DrawIndexedInstanced(PrimitiveType type, uint32_t indexStart, uint32_t indexCount, uint32_t vertexStart, uint32_t instanceStart, uint32_t instanceCount) override;

		VkDevice GetVkDevice() const { return _device; }

	private:
		void Finalize() override;

		bool BeginFrame() override;
		void Present() override;
		bool PrepareDraw(PrimitiveType type);

		BufferHandle* CreateBuffer(BufferUsage usage, uint32_t size, uint32_t stride, ResourceUsage resourceUsage, const void* initialData) override;

		void SetIndexBufferCore(BufferHandle* handle, IndexType type) override;
		void WaitIdle();

		VkDevice _device = VK_NULL_HANDLE;
	};
}