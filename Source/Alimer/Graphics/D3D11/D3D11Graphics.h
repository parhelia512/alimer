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

#include "../Graphics.h"
#include "../../Base/HashMap.h"
#include "D3D11Prerequisites.h"

namespace Alimer
{
	struct GraphicsImpl;
	class D3D11Buffer;

	/// Compiled shader with specific defines.
	class D3D11Graphics final : public Graphics
	{
	public:
		/// Construct. Set parent shader and defines but do not compile yet.
		D3D11Graphics(bool validation, const std::string& applicationName);
		/// Destruct.
		~D3D11Graphics() override;

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

		ID3D11Device1* GetD3DDevice() const { return _d3dDevice.Get(); }
		ID3D11DeviceContext1* GetD3DDeviceContext() const { return _d3dContext.Get(); }

		VertexBuffer* GetVertexBuffer(uint32_t index) const
		{
			return _vbo.buffers[index];
		}

		/// Return currently bound index buffer.
		D3D11Buffer* GetIndexBuffer() const { return _currentIndexBuffer; }

	private:
		void Finalize() override;

		/// Create the D3D11 device and swap chain. Requires an open window. Can also be called again to recrease swap chain. Return true on success.
		bool CreateD3DDevice(
			HWND windowHandle,
			uint32_t backbufferWidth,
			uint32_t backbufferHeight,
			uint32_t multisample);

		/// Update swap chain state for a new mode and create views for the backbuffer & default depth buffer.
		bool UpdateSwapChain(uint32_t width, uint32_t height);

		/// Set texture state for the next draw call. PrepareDraw() calls this.
		void PrepareTextures();

		/// Set state for the next draw call. Return false if the draw call should not be attempted.
		bool PrepareDraw(PrimitiveType type);

		bool BeginFrame() override;
		void Present() override;

		BufferHandle* CreateBuffer(BufferUsage usage, uint32_t size, uint32_t stride, ResourceUsage resourceUsage, const void* initialData) override;

		void SetIndexBufferCore(BufferHandle* handle, IndexType type) override;

		void WaitIdle();

		/// Reset internally tracked state.
		void ResetState();

		Microsoft::WRL::ComPtr<IDXGIFactory1> _dxgiFactory{};
		Microsoft::WRL::ComPtr<ID3D11Device1> _d3dDevice{};
		Microsoft::WRL::ComPtr<ID3D11DeviceContext1> _d3dContext{};
		/// Swap chain.
		Microsoft::WRL::ComPtr<IDXGISwapChain> _swapChain{};

		D3D_FEATURE_LEVEL _d3dFeatureLevel{ D3D_FEATURE_LEVEL_9_1 };
		bool _debugMode{};

		using InputLayoutDesc = std::pair<uint64_t, uint64_t>;
		using InputLayoutMap = std::map<InputLayoutDesc, ID3D11InputLayout*>;

		/// Current input layout: vertex buffers' element mask and vertex shader's element mask combined.
		InputLayoutDesc _currentInputLayout;

		/// Current blend state object.
		ID3D11BlendState1* _currentBlendState = nullptr;
		/// Current depth state object.
		ID3D11DepthStencilState* _currentDepthState = nullptr;
		/// Current rasterizer state object.
		ID3D11RasterizerState1* _currentRasterizerState = nullptr;

		/// Current index buffer.
		D3D11Buffer* _currentIndexBuffer = nullptr;

		/// Input layout dirty flag.
		bool _inputLayoutDirty;

		struct VertexBindingState {
			VertexBuffer* buffers[MaxVertexBuffers];
			uint32_t vertexOffsets[MaxVertexBuffers];
			uint32_t strides[MaxVertexBuffers];
			VertexInputRate rates[MaxVertexBuffers];
		};

		VertexBindingState _vbo = {};

		/// Bound constant buffers by shader stage.
		ConstantBuffer* _constantBuffers[ecast(ShaderStage::Count)][MAX_CONSTANT_BUFFERS];

		/// Blend state objects.
		HashMap<ID3D11BlendState1*> _blendStates;
		/// Depth state objects.
		HashMap<ID3D11DepthStencilState*> _depthStates;
		/// Rasterizer state objects.
		HashMap<ID3D11RasterizerState1*> _rasterizerStates;

		/// Input layouts.
		InputLayoutMap _inputLayouts;
	};
}