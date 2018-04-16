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

#include "../../Debug/Log.h"
#include "../../Debug/Profiler.h"
#include "../../Window/Window.h"
#include "D3D11Graphics.h"
#include "D3D11Buffer.h"
#include "../GPUObject.h"
#include "../Shader.h"
#include "../Graphics.h"
#include "../ConstantBuffer.h"
#include "../IndexBuffer.h"
#include "D3D11ShaderVariation.h"
#include "../Texture.h"
#include "../VertexBuffer.h"
#include "D3D11Convert.h"

#include <cstdlib>

using namespace std;

namespace Alimer
{
	static_assert(static_cast<uint32_t>(ResourceUsage::Default) == D3D11_USAGE_DEFAULT, "D3D11: Wrong ResourceUsage map");
	static_assert(static_cast<uint32_t>(ResourceUsage::Immutable) == D3D11_USAGE_IMMUTABLE, "D3D11: Wrong ResourceUsage map");
	static_assert(static_cast<uint32_t>(ResourceUsage::Dynamic) == D3D11_USAGE_DYNAMIC, "D3D11: Wrong ResourceUsage map");

	static const DXGI_FORMAT d3dElementFormats[] = {
		DXGI_FORMAT_R32_SINT,
		DXGI_FORMAT_R32_FLOAT,
		DXGI_FORMAT_R32G32_FLOAT,
		DXGI_FORMAT_R32G32B32_FLOAT,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		DXGI_FORMAT_R8G8B8A8_UINT,
		DXGI_FORMAT_R32G32B32A32_FLOAT, // Incorrect, but included to not cause out-of-range indexing
		DXGI_FORMAT_R32G32B32A32_FLOAT  //                          --||--
	};

	/// \cond PRIVATE
	struct GraphicsImpl
	{
		/// Construct.
		GraphicsImpl() :
			device(nullptr),
			deviceContext(nullptr),
			swapChain(nullptr),
			defaultRenderTargetView(nullptr),
			defaultDepthTexture(nullptr),
			defaultDepthStencilView(nullptr),
			blendState(nullptr),
			depthState(nullptr),
			rasterizerState(nullptr),
			depthStencilView(nullptr),
			blendStateHash(0xffffffffffffffff),
			depthStateHash(0xffffffffffffffff),
			rasterizerStateHash(0xffffffffffffffff),
			stencilRef(0)
		{
			for (size_t i = 0; i < MAX_RENDERTARGETS; ++i)
				renderTargetViews[i] = nullptr;
		}

		/// Graphics device.
		ID3D11Device* device;
		/// Immediate device context.
		ID3D11DeviceContext* deviceContext;
		/// Swap chain.
		IDXGISwapChain* swapChain;
		/// Default (backbuffer) rendertarget view.
		ID3D11RenderTargetView* defaultRenderTargetView;
		/// Default depth-stencil texture.
		ID3D11Texture2D* defaultDepthTexture;
		/// Default depth-stencil view.
		ID3D11DepthStencilView* defaultDepthStencilView;
		/// Current blend state object.
		ID3D11BlendState* blendState;
		/// Current depth state object.
		ID3D11DepthStencilState* depthState;
		/// Current rasterizer state object.
		ID3D11RasterizerState* rasterizerState;
		/// Current shader resource views.
		ID3D11ShaderResourceView* resourceViews[MAX_TEXTURE_UNITS];
		/// Current sampler states.
		ID3D11SamplerState* samplers[MAX_TEXTURE_UNITS];
		/// Current color rendertarget views.
		ID3D11RenderTargetView* renderTargetViews[MAX_RENDERTARGETS];
		/// Current depth-stencil view.
		ID3D11DepthStencilView* depthStencilView;
		/// Stencil ref value set to the device.
		unsigned char stencilRef;
		/// Current blend state hash code.
		unsigned long long blendStateHash;
		/// Current depth state hash code.
		unsigned long long depthStateHash;
		/// Current rasterizer state hash code.
		unsigned long long rasterizerStateHash;
	};
	/// \endcond

	D3D11Graphics::D3D11Graphics(bool validation, const string& applicationName)
		: Graphics(GraphicsDeviceType::Direct3D11, validation)
	{
		Unused(applicationName);
		impl = new GraphicsImpl();
		ResetState();
	}

	D3D11Graphics::~D3D11Graphics()
	{
		WaitIdle();
		Finalize();
	}

	void D3D11Graphics::WaitIdle()
	{

	}

	bool D3D11Graphics::IsSupported()
	{
		return true;
	}

	ID3D11Device* D3D11Graphics::GetD3DDevice() const
	{
		return impl->device;
	}

	ID3D11DeviceContext* D3D11Graphics::GetD3DDeviceContext() const
	{
		return impl->deviceContext;
	}

	bool Graphics::SetMode(const Size& size, bool fullscreen, bool resizable, int multisample_)
	{
		multisample_ = Clamp(multisample_, 1, 16);

		if (!window->SetSize(size, fullscreen, resizable))
			return false;

		// Create D3D11 device and swap chain when setting mode for the first time, or swap chain again when changing multisample
		if (!impl->device || multisample_ != multisample)
		{
			if (!CreateD3DDevice(multisample_))
				return false;
			// Swap chain needs to be updated manually for the first time, otherwise window resize event takes care of it
			UpdateSwapChain(window->GetWidth(), window->GetHeight());
		}

		screenModeEvent.size = _backbufferSize;
		screenModeEvent.fullscreen = IsFullscreen();
		screenModeEvent.resizable = IsResizable();
		screenModeEvent.multisample = multisample;
		SendEvent(screenModeEvent);

		LOGDEBUGF("Set screen mode %dx%d fullscreen %d resizable %d multisample %d",
			_backbufferSize.width,
			_backbufferSize.height,
			IsFullscreen(), IsResizable(), multisample);

		_initialized = true;
		return true;
	}

	bool Graphics::SetFullscreen(bool enable)
	{
		if (!IsInitialized())
			return false;

		return SetMode(_backbufferSize, enable, window->IsResizable(), multisample);
	}

	bool Graphics::SetMultisample(int multisample_)
	{
		if (!IsInitialized())
			return false;

		return SetMode(_backbufferSize, window->IsFullscreen(), window->IsResizable(), multisample_);
	}

	void Graphics::SetVSync(bool enable)
	{
		vsync = enable;
	}

	void D3D11Graphics::Finalize()
	{
		// Release all GPU objects
		Graphics::Finalize();

		for (auto it = _inputLayouts.begin(); it != _inputLayouts.end(); ++it)
		{
			ID3D11InputLayout* d3dLayout = it->second;
			d3dLayout->Release();
		}
		_inputLayouts.clear();

		for (auto it = blendStates.begin(); it != blendStates.end(); ++it)
		{
			ID3D11BlendState* d3dState = (ID3D11BlendState*)it->second;
			d3dState->Release();
		}
		blendStates.clear();

		for (auto it = depthStates.begin(); it != depthStates.end(); ++it)
		{
			ID3D11DepthStencilState* d3dState = (ID3D11DepthStencilState*)it->second;
			d3dState->Release();
		}
		depthStates.clear();

		for (auto it = rasterizerStates.begin(); it != rasterizerStates.end(); ++it)
		{
			ID3D11RasterizerState* d3dState = (ID3D11RasterizerState*)it->second;
			d3dState->Release();
		}
		rasterizerStates.clear();

		if (impl->deviceContext)
		{
			ID3D11RenderTargetView* nullView = nullptr;
			impl->deviceContext->OMSetRenderTargets(1, &nullView, nullptr);
		}
		if (impl->defaultRenderTargetView)
		{
			impl->defaultRenderTargetView->Release();
			impl->defaultRenderTargetView = nullptr;
		}
		if (impl->defaultDepthStencilView)
		{
			impl->defaultDepthStencilView->Release();
			impl->defaultDepthStencilView = nullptr;
		}
		if (impl->defaultDepthTexture)
		{
			impl->defaultDepthTexture->Release();
			impl->defaultDepthTexture = nullptr;
		}
		if (impl->swapChain)
		{
			impl->swapChain->Release();
			impl->swapChain = nullptr;
		}
		if (impl->deviceContext)
		{
			impl->deviceContext->Release();
			impl->deviceContext = nullptr;
		}
		if (impl->device)
		{
			impl->device->Release();
			impl->device = nullptr;
		}

		window->Close();
		ResetState();
	}

	void Graphics::Present()
	{
		{
			ALIMER_PROFILE(Present);

			HRESULT hr = impl->swapChain->Present(vsync ? 1 : 0, 0);
			if (FAILED(hr))
			{
				LOGERROR("[D3D11] - Swapchain Present failed");
			}
		}
	}

	void Graphics::SetRenderTarget(Texture* renderTarget, Texture* depthStencil)
	{
		static std::vector<Texture*> renderTargetVector(1);
		renderTargetVector[0] = renderTarget;
		SetRenderTargets(renderTargetVector, depthStencil);
	}

	void Graphics::SetRenderTargets(
		const std::vector<Texture*>& renderTargets,
		Texture* depthStencil)
	{
		PrepareTextures();

		// If depth stencil is specified but no rendertarget, use null instead of backbuffer, unless the depth stencil has same
		// size as the backbuffer
		bool depthOnlyRendering = depthStencil && depthStencil->GetSize() != _backbufferSize;
		for (size_t i = 0; i < MAX_RENDERTARGETS && i < renderTargets.size(); ++i)
		{
			_renderTargets[i] = (renderTargets[i] && renderTargets[i]->IsRenderTarget()) ? renderTargets[i] : nullptr;
			impl->renderTargetViews[i] = renderTargets[i] ? (ID3D11RenderTargetView*)renderTargets[i]->D3DRenderTargetView() :
				(i > 0 || depthOnlyRendering) ? nullptr : impl->defaultRenderTargetView;
		}

		for (size_t i = renderTargets.size(); i < MAX_RENDERTARGETS; ++i)
		{
			_renderTargets[i] = nullptr;
			impl->renderTargetViews[i] = nullptr;
		}

		_depthStencil = (depthStencil && depthStencil->IsDepthStencil()) ? depthStencil : nullptr;
		impl->depthStencilView = _depthStencil ? (ID3D11DepthStencilView*)_depthStencil->D3DRenderTargetView() :
			impl->defaultDepthStencilView;

		if (_renderTargets[0])
		{
			_renderTargetSize = renderTargets[0]->GetSize();
		}
		else if (depthStencil)
		{
			_renderTargetSize = depthStencil->GetSize();
		}
		else
		{
			_renderTargetSize = _backbufferSize;
		}

		impl->deviceContext->OMSetRenderTargets(MAX_RENDERTARGETS, impl->renderTargetViews, impl->depthStencilView);
	}

	void Graphics::SetViewport(const IntRect& viewport_)
	{
		/// \todo Implement a member function in IntRect for clipping
		viewport.left = Clamp(viewport_.left, 0, _renderTargetSize.width - 1);
		viewport.top = Clamp(viewport_.top, 0, _renderTargetSize.height - 1);
		viewport.right = Clamp(viewport_.right, viewport.left + 1, _renderTargetSize.width);
		viewport.bottom = Clamp(viewport_.bottom, viewport.top + 1, _renderTargetSize.height);

		static D3D11_VIEWPORT d3dViewport;
		d3dViewport.TopLeftX = static_cast<float>(viewport.left);
		d3dViewport.TopLeftY = static_cast<float>(viewport.top);
		d3dViewport.Width = static_cast<float>(viewport.right - viewport.left);
		d3dViewport.Height = static_cast<float>(viewport.bottom - viewport.top);
		d3dViewport.MinDepth = 0.0f;
		d3dViewport.MaxDepth = 1.0f;

		impl->deviceContext->RSSetViewports(1, &d3dViewport);
	}

	void Graphics::SetVertexBuffer(uint32_t index, VertexBuffer* buffer)
	{
		if (index < MAX_VERTEX_STREAMS
			&& buffer != vertexBuffers[index])
		{
			vertexBuffers[index] = buffer;
			ID3D11Buffer* d3dBuffer = buffer ? static_cast<D3D11Buffer*>(buffer->GetHandle())->GetD3DBuffer() : nullptr;
			UINT stride = buffer ? buffer->GetStride() : 0;
			UINT offset = 0;
			impl->deviceContext->IASetVertexBuffers(
				index, 
				1,
				&d3dBuffer, 
				&stride,
				&offset);
			inputLayoutDirty = true;
		}
	}

	void Graphics::SetConstantBuffer(ShaderStage stage, uint32_t index, ConstantBuffer* buffer)
	{
		if (stage < MAX_SHADER_STAGES
			&& index < MAX_CONSTANT_BUFFERS
			&& buffer != constantBuffers[stage][index])
		{
			constantBuffers[stage][index] = buffer;
			ID3D11Buffer* d3dBuffer = buffer ? static_cast<D3D11Buffer*>(buffer->GetHandle())->GetD3DBuffer() : nullptr;

			switch (stage)
			{
			case SHADER_VS:
				impl->deviceContext->VSSetConstantBuffers(index, 1, &d3dBuffer);
				break;

			case SHADER_PS:
				impl->deviceContext->PSSetConstantBuffers(index, 1, &d3dBuffer);
				break;

			default:
				break;
			}
		}
	}

	void Graphics::SetTexture(size_t index, Texture* texture)
	{
		if (index < MAX_TEXTURE_UNITS)
		{
			textures[index] = texture;

			ID3D11ShaderResourceView* d3dResourceView = texture ? (ID3D11ShaderResourceView*)texture->D3DResourceView() :
				nullptr;
			ID3D11SamplerState* d3dSampler = texture ? (ID3D11SamplerState*)texture->D3DSampler() : nullptr;

			if (d3dResourceView != impl->resourceViews[index])
			{
				impl->resourceViews[index] = d3dResourceView;
				texturesDirty = true;
			}
			if (d3dSampler != impl->samplers[index])
			{
				impl->samplers[index] = d3dSampler;
				texturesDirty = true;
			}
		}
	}

	void D3D11Graphics::SetIndexBufferCore(BufferHandle* handle, IndexType type)
	{
		if (handle == _indexBuffer)
			return;

		// Unset?
		if (!handle)
		{
			impl->deviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
			return;
		}

		_indexBuffer = static_cast<D3D11Buffer*>(handle);
		impl->deviceContext->IASetIndexBuffer(
			_indexBuffer->GetD3DBuffer(),
			d3d11::Convert(type),
			0);
	}

	void Graphics::SetShaders(ShaderVariation* vs, ShaderVariation* ps)
	{
		if (vs != vertexShader)
		{
			if (vs && vs->GetStage() == SHADER_VS)
			{
				if (!vs->IsCompiled())
					vs->Compile();
				impl->deviceContext->VSSetShader((ID3D11VertexShader*)vs->ShaderObject(), nullptr, 0);
			}
			else
				impl->deviceContext->VSSetShader(nullptr, nullptr, 0);

			vertexShader = vs;
			inputLayoutDirty = true;
		}

		if (ps != pixelShader)
		{
			if (ps && ps->GetStage() == SHADER_PS)
			{
				if (!ps->IsCompiled())
					ps->Compile();
				impl->deviceContext->PSSetShader((ID3D11PixelShader*)ps->ShaderObject(), nullptr, 0);
			}
			else
				impl->deviceContext->PSSetShader(nullptr, nullptr, 0);

			pixelShader = ps;
		}
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

	void Graphics::SetScissorTest(bool scissorEnable, const IntRect& scissorRect)
	{
		renderState.scissorEnable = scissorEnable;

		if (scissorRect != renderState.scissorRect)
		{
			/// \todo Implement a member function in IntRect for clipping
			renderState.scissorRect.left = Clamp(scissorRect.left, 0, _renderTargetSize.width - 1);
			renderState.scissorRect.top = Clamp(scissorRect.top, 0, _renderTargetSize.height - 1);
			renderState.scissorRect.right = Clamp(scissorRect.right, renderState.scissorRect.left + 1, _renderTargetSize.width);
			renderState.scissorRect.bottom = Clamp(scissorRect.bottom, renderState.scissorRect.top + 1, _renderTargetSize.height);

			D3D11_RECT d3dRect;
			d3dRect.left = renderState.scissorRect.left;
			d3dRect.top = renderState.scissorRect.top;
			d3dRect.right = renderState.scissorRect.right;
			d3dRect.bottom = renderState.scissorRect.bottom;
			impl->deviceContext->RSSetScissorRects(1, &d3dRect);
		}

		rasterizerStateDirty = true;
	}

	void Graphics::SetStencilTest(bool stencilEnable, const StencilTestDesc& stencilTest, unsigned char stencilRef)
	{
		renderState.stencilEnable = stencilEnable;
		// When stencil test is disabled, always set default stencil test parameters to prevent creating unnecessary states
		renderState.stencilTest = stencilEnable ? stencilTest : StencilTestDesc();
		renderState.stencilRef = stencilRef;

		depthStateDirty = true;
	}

	void Graphics::ResetRenderTargets()
	{
		SetRenderTarget(nullptr, nullptr);
	}

	void Graphics::ResetViewport()
	{
		SetViewport(IntRect(_renderTargetSize));
	}

	void Graphics::ResetVertexBuffers()
	{
		for (uint32_t i = 0; i < MAX_VERTEX_STREAMS; ++i)
			SetVertexBuffer(i, nullptr);
	}

	void Graphics::ResetConstantBuffers()
	{
		for (uint32_t i = 0; i < MAX_SHADER_STAGES; ++i)
		{
			for (uint32_t j = 0; i < MAX_CONSTANT_BUFFERS; ++j)
			{
				SetConstantBuffer((ShaderStage)i, j, nullptr);
			}
		}
	}

	void Graphics::ResetTextures()
	{
		for (size_t i = 0; i < MAX_TEXTURE_UNITS; ++i)
			SetTexture(i, nullptr);
	}

	void Graphics::Clear(ClearFlags clearFlags, const Color& clearColor, float clearDepth, unsigned char clearStencil)
	{
		PrepareTextures();

		if (any(clearFlags & ClearFlags::Color)
			&& impl->renderTargetViews[0])
		{
			impl->deviceContext->ClearRenderTargetView(impl->renderTargetViews[0], clearColor.Data());
		}

		if (any(clearFlags & (ClearFlags::Depth | ClearFlags::Stencil))
			&& impl->depthStencilView)
		{
			UINT depthClearFlags = 0;
			if (any(clearFlags & ClearFlags::Depth))
				depthClearFlags |= D3D11_CLEAR_DEPTH;
			if (any(clearFlags & ClearFlags::Stencil))
				depthClearFlags |= D3D11_CLEAR_STENCIL;
			impl->deviceContext->ClearDepthStencilView(impl->depthStencilView, depthClearFlags, clearDepth, clearStencil);
		}
	}

	void Graphics::Draw(PrimitiveType type, size_t vertexStart, size_t vertexCount)
	{
		if (!PrepareDraw(type))
			return;

		impl->deviceContext->Draw((unsigned)vertexCount, (unsigned)vertexStart);
	}

	void Graphics::DrawIndexed(PrimitiveType type, size_t indexStart, size_t indexCount, size_t vertexStart)
	{
		if (!PrepareDraw(type))
			return;

		impl->deviceContext->DrawIndexed((unsigned)indexCount, (unsigned)indexStart, (unsigned)vertexStart);
	}

	void Graphics::DrawInstanced(PrimitiveType type, size_t vertexStart, size_t vertexCount, size_t instanceStart,
		size_t instanceCount)
	{
		if (!PrepareDraw(type))
			return;

		impl->deviceContext->DrawInstanced((unsigned)vertexCount, (unsigned)instanceCount, (unsigned)vertexStart,
			(unsigned)instanceStart);
	}

	void Graphics::DrawIndexedInstanced(PrimitiveType type, size_t indexStart, size_t indexCount, size_t vertexStart, size_t
		instanceStart, size_t instanceCount)
	{
		if (!PrepareDraw(type))
			return;

		impl->deviceContext->DrawIndexedInstanced((unsigned)indexCount, (unsigned)instanceCount, (unsigned)indexStart, (unsigned)
			vertexStart, (unsigned)instanceStart);
	}



	bool Graphics::IsFullscreen() const
	{
		return window->IsFullscreen();
	}

	bool Graphics::IsResizable() const
	{
		return window->IsResizable();
	}

	Texture* Graphics::GetRenderTarget(size_t index) const
	{
		return index < MAX_RENDERTARGETS ? _renderTargets[index] : nullptr;
	}

	VertexBuffer* Graphics::GetVertexBuffer(size_t index) const
	{
		return index < MAX_VERTEX_STREAMS ? vertexBuffers[index] : nullptr;
	}

	ConstantBuffer* Graphics::GetConstantBuffer(ShaderStage stage, size_t index) const
	{
		return (stage < MAX_SHADER_STAGES && index < MAX_CONSTANT_BUFFERS) ? constantBuffers[stage][index] : nullptr;
	}

	Texture* Graphics::GetTexture(size_t index) const
	{
		return (index < MAX_TEXTURE_UNITS) ? textures[index] : nullptr;
	}

	void Graphics::AddGPUObject(GPUObject* object)
	{
		if (object)
			gpuObjects.push_back(object);
	}

	void Graphics::RemoveGPUObject(GPUObject* object)
	{
		auto it = std::find(gpuObjects.begin(), gpuObjects.end(), object);
		if (it != gpuObjects.end())
			gpuObjects.erase(it);
	}

	void* Graphics::D3DDevice() const
	{
		return impl->device;
	}

	void* Graphics::D3DDeviceContext() const
	{
		return impl->deviceContext;
	}

	bool Graphics::CreateD3DDevice(int multisample_)
	{
		// Device needs only to be created once
		if (!impl->device)
		{
			D3D11CreateDevice(
				nullptr,
				D3D_DRIVER_TYPE_HARDWARE,
				0,
				0,
				nullptr,
				0,
				D3D11_SDK_VERSION,
				&impl->device,
				nullptr,
				&impl->deviceContext
			);

			if (!impl->device || !impl->deviceContext)
			{
				LOGERROR("Failed to create D3D11 device");
				return false;
			}
		}

		// Create swap chain. Release old if necessary
		if (impl->swapChain)
		{
			impl->swapChain->Release();
			impl->swapChain = nullptr;
		}

		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		memset(&swapChainDesc, 0, sizeof swapChainDesc);
		swapChainDesc.BufferCount = 1;
		swapChainDesc.BufferDesc.Width = window->GetWidth();
		swapChainDesc.BufferDesc.Height = window->GetHeight();
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.OutputWindow = (HWND)window->GetHandle();
		swapChainDesc.SampleDesc.Count = multisample_;
		swapChainDesc.SampleDesc.Quality = multisample_ > 1 ? 0xffffffff : 0;
		swapChainDesc.Windowed = TRUE;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		IDXGIDevice* dxgiDevice = nullptr;
		impl->device->QueryInterface(IID_IDXGIDevice, (void **)&dxgiDevice);
		IDXGIAdapter* dxgiAdapter = nullptr;
		dxgiDevice->GetParent(IID_IDXGIAdapter, (void **)&dxgiAdapter);
		IDXGIFactory* dxgiFactory = nullptr;
		dxgiAdapter->GetParent(IID_IDXGIFactory, (void **)&dxgiFactory);
		dxgiFactory->CreateSwapChain(impl->device, &swapChainDesc, &impl->swapChain);
		// After creating the swap chain, disable automatic Alt-Enter fullscreen/windowed switching
		// (the application will switch manually if it wants to)
		dxgiFactory->MakeWindowAssociation(
			(HWND)window->GetHandle(),
			DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);

		dxgiFactory->Release();
		dxgiAdapter->Release();
		dxgiDevice->Release();

		if (impl->swapChain)
		{
			multisample = multisample_;
			return true;
		}
		else
		{
			LOGERROR("Failed to create D3D11 swap chain");
			return false;
		}
	}

	bool Graphics::UpdateSwapChain(uint32_t width, uint32_t height)
	{
		bool success = true;

		ID3D11RenderTargetView* nullView = nullptr;
		impl->deviceContext->OMSetRenderTargets(1, &nullView, nullptr);
		if (impl->defaultRenderTargetView)
		{
			impl->defaultRenderTargetView->Release();
			impl->defaultRenderTargetView = nullptr;
		}
		if (impl->defaultDepthStencilView)
		{
			impl->defaultDepthStencilView->Release();
			impl->defaultDepthStencilView = nullptr;
		}
		if (impl->defaultDepthTexture)
		{
			impl->defaultDepthTexture->Release();
			impl->defaultDepthTexture = nullptr;
		}

		impl->swapChain->ResizeBuffers(1, width, height, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

		// Create default rendertarget view representing the backbuffer
		ID3D11Texture2D* backbufferTexture;
		impl->swapChain->GetBuffer(0, IID_ID3D11Texture2D, (void**)&backbufferTexture);
		if (backbufferTexture)
		{
			impl->device->CreateRenderTargetView(backbufferTexture, 0, &impl->defaultRenderTargetView);
			backbufferTexture->Release();
		}
		else
		{
			LOGERROR("Failed to get backbuffer texture");
			success = false;
		}

		// Create default depth-stencil texture and view
		D3D11_TEXTURE2D_DESC depthDesc;
		ZeroMemory(&depthDesc, sizeof(D3D11_TEXTURE2D_DESC));
		depthDesc.Width = width;
		depthDesc.Height = height;
		depthDesc.MipLevels = 1;
		depthDesc.ArraySize = 1;
		depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthDesc.SampleDesc.Count = multisample;
		depthDesc.SampleDesc.Quality = multisample > 1 ? 0xffffffff : 0;
		depthDesc.Usage = D3D11_USAGE_DEFAULT;
		depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthDesc.CPUAccessFlags = 0;
		depthDesc.MiscFlags = 0;
		impl->device->CreateTexture2D(&depthDesc, 0, &impl->defaultDepthTexture);
		if (impl->defaultDepthTexture)
			impl->device->CreateDepthStencilView(impl->defaultDepthTexture, 0, &impl->defaultDepthStencilView);
		else
		{
			LOGERROR("Failed to create backbuffer depth-stencil texture");
			success = false;
		}

		// Update internally held backbuffer size and fullscreen state
		_backbufferSize.width = width;
		_backbufferSize.height = height;

		ResetRenderTargets();
		ResetViewport();
		return success;
	}

	void Graphics::HandleResize(WindowResizeEvent& /*event*/)
	{
		// Handle window resize
		if (impl->swapChain
			&& (window->GetWidth() != _backbufferSize.width || window->GetHeight() != _backbufferSize.height))
		{
			UpdateSwapChain(window->GetWidth(), window->GetHeight());
		}
	}

	void Graphics::PrepareTextures()
	{
		if (texturesDirty)
		{
			// Set both VS & PS textures to mimic OpenGL behavior
			impl->deviceContext->VSSetShaderResources(0, MAX_TEXTURE_UNITS, impl->resourceViews);
			impl->deviceContext->VSSetSamplers(0, MAX_TEXTURE_UNITS, impl->samplers);
			impl->deviceContext->PSSetShaderResources(0, MAX_TEXTURE_UNITS, impl->resourceViews);
			impl->deviceContext->PSSetSamplers(0, MAX_TEXTURE_UNITS, impl->samplers);
			texturesDirty = false;
		}
	}

	BufferHandle* D3D11Graphics::CreateBuffer(BufferUsage usage, uint32_t size, uint32_t stride, ResourceUsage resourceUsage, const void* initialData)
	{
		return new D3D11Buffer(this, usage, size, stride, resourceUsage, initialData);
	}

	bool Graphics::PrepareDraw(PrimitiveType type)
	{
		PrepareTextures();

		if (!vertexShader || !pixelShader || !vertexShader->ShaderObject() || !pixelShader->ShaderObject())
			return false;

		if (primitiveType != type)
		{
			impl->deviceContext->IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY)type);
			primitiveType = type;
		}

		if (inputLayoutDirty)
		{
			inputLayoutDirty = false;

			InputLayoutDesc newInputLayout;
			newInputLayout.first = 0;
			newInputLayout.second = vertexShader->ElementHash();
			for (size_t i = 0; i < MAX_VERTEX_STREAMS; ++i)
			{
				if (vertexBuffers[i])
					newInputLayout.first |= (uint64_t)vertexBuffers[i]->GetElementHash() << (i * 16);
			}

			if (newInputLayout != inputLayout)
			{
				// Check if layout already exists
				auto it = _inputLayouts.find(newInputLayout);
				if (it != _inputLayouts.end())
				{
					impl->deviceContext->IASetInputLayout(it->second);
					inputLayout = newInputLayout;
				}
				else
				{
					// Not found, create new
					std::vector<D3D11_INPUT_ELEMENT_DESC> elementDescs;

					for (uint32_t i = 0; i < MAX_VERTEX_STREAMS; ++i)
					{
						if (vertexBuffers[i])
						{
							const std::vector<VertexElement>& elements = vertexBuffers[i]->GetElements();

							for (const VertexElement& element : elements)
							{
								D3D11_INPUT_ELEMENT_DESC newDesc;
								newDesc.SemanticName = elementSemanticNames[element.semantic];
								newDesc.SemanticIndex = element.index;
								newDesc.Format = (element.semantic == SEM_COLOR && element.type == ELEM_UBYTE4) ?
									DXGI_FORMAT_R8G8B8A8_UNORM : d3dElementFormats[element.type];
								newDesc.InputSlot = (unsigned)i;
								newDesc.AlignedByteOffset = element.offset;
								newDesc.InputSlotClass = element.perInstance ? D3D11_INPUT_PER_INSTANCE_DATA :
									D3D11_INPUT_PER_VERTEX_DATA;
								newDesc.InstanceDataStepRate = element.perInstance ? 1 : 0;
								elementDescs.push_back(newDesc);
							}
						}
					}

					ID3D11InputLayout* d3dInputLayout = nullptr;
					ID3DBlob* d3dBlob = (ID3DBlob*)vertexShader->BlobObject();
					impl->device->CreateInputLayout(
						elementDescs.data(),
						(UINT)elementDescs.size(),
						d3dBlob->GetBufferPointer(),
						d3dBlob->GetBufferSize(), &d3dInputLayout);
					if (d3dInputLayout)
					{
						_inputLayouts[newInputLayout] = d3dInputLayout;
						impl->deviceContext->IASetInputLayout(d3dInputLayout);
						inputLayout = newInputLayout;
					}
					else
						LOGERROR("Failed to create input layout");
				}
			}
		}

		if (blendStateDirty)
		{
			unsigned long long blendStateHash =
				renderState.colorWriteMask |
				(renderState.alphaToCoverage ? 0x10 : 0x0) |
				(renderState.blendMode.blendEnable ? 0x20 : 0x0) |
				(renderState.blendMode.srcBlend << 6) |
				(renderState.blendMode.destBlend << 10) |
				(renderState.blendMode.blendOp << 14) |
				(renderState.blendMode.srcBlendAlpha << 17) |
				(renderState.blendMode.destBlendAlpha << 21) |
				(renderState.blendMode.blendOpAlpha << 25);

			if (blendStateHash != impl->blendStateHash)
			{
				auto it = blendStates.find(blendStateHash);
				if (it != blendStates.end())
				{
					ID3D11BlendState* newBlendState = (ID3D11BlendState*)it->second;
					impl->deviceContext->OMSetBlendState(newBlendState, nullptr, 0xffffffff);
					impl->blendState = newBlendState;
					impl->blendStateHash = blendStateHash;
				}
				else
				{
					ALIMER_PROFILE(CreateBlendState);

					D3D11_BLEND_DESC stateDesc = {};

					stateDesc.AlphaToCoverageEnable = renderState.alphaToCoverage;
					stateDesc.IndependentBlendEnable = false;
					stateDesc.RenderTarget[0].BlendEnable = renderState.blendMode.blendEnable;
					stateDesc.RenderTarget[0].SrcBlend = (D3D11_BLEND)renderState.blendMode.srcBlend;
					stateDesc.RenderTarget[0].DestBlend = (D3D11_BLEND)renderState.blendMode.destBlend;
					stateDesc.RenderTarget[0].BlendOp = (D3D11_BLEND_OP)renderState.blendMode.blendOp;
					stateDesc.RenderTarget[0].SrcBlendAlpha = (D3D11_BLEND)renderState.blendMode.srcBlendAlpha;
					stateDesc.RenderTarget[0].DestBlendAlpha = (D3D11_BLEND)renderState.blendMode.destBlendAlpha;
					stateDesc.RenderTarget[0].BlendOpAlpha = (D3D11_BLEND_OP)renderState.blendMode.blendOpAlpha;
					stateDesc.RenderTarget[0].RenderTargetWriteMask = renderState.colorWriteMask & COLORMASK_ALL;

					ID3D11BlendState* newBlendState = nullptr;
					impl->device->CreateBlendState(&stateDesc, &newBlendState);
					if (newBlendState)
					{
						impl->deviceContext->OMSetBlendState(newBlendState, nullptr, 0xffffffff);
						impl->blendState = newBlendState;
						impl->blendStateHash = blendStateHash;
						blendStates[blendStateHash] = newBlendState;

						LOGDEBUGF("Created new blend state with hash %x", blendStateHash & 0xffffffff);
					}
				}
			}

			blendStateDirty = false;
		}

		if (depthStateDirty)
		{
			unsigned long long depthStateHash =
				(renderState.depthWrite ? 0x1 : 0x0) |
				(renderState.stencilEnable ? 0x2 : 0x0) |
				(renderState.depthFunc << 2) |
				(renderState.stencilTest.stencilReadMask << 6) |
				(renderState.stencilTest.stencilWriteMask << 14) |
				(renderState.stencilTest.frontFail << 22) |
				(renderState.stencilTest.frontDepthFail << 26) |
				((unsigned long long)renderState.stencilTest.frontPass << 30) |
				((unsigned long long)renderState.stencilTest.frontFunc << 34) |
				((unsigned long long)renderState.stencilTest.frontFail << 38) |
				((unsigned long long)renderState.stencilTest.frontDepthFail << 42) |
				((unsigned long long)renderState.stencilTest.frontPass << 46) |
				((unsigned long long)renderState.stencilTest.frontFunc << 50);

			if (depthStateHash != impl->depthStateHash || renderState.stencilRef != impl->stencilRef)
			{
				auto it = depthStates.find(depthStateHash);
				if (it != depthStates.end())
				{
					ID3D11DepthStencilState* newDepthState = (ID3D11DepthStencilState*)it->second;
					impl->deviceContext->OMSetDepthStencilState(newDepthState, renderState.stencilRef);
					impl->depthState = newDepthState;
					impl->depthStateHash = depthStateHash;
					impl->stencilRef = renderState.stencilRef;
				}
				else
				{
					ALIMER_PROFILE(CreateDepthStencilState);

					D3D11_DEPTH_STENCIL_DESC stateDesc = {};

					stateDesc.DepthEnable = TRUE;
					stateDesc.DepthWriteMask = renderState.depthWrite ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
					stateDesc.DepthFunc = (D3D11_COMPARISON_FUNC)renderState.depthFunc;
					stateDesc.StencilEnable = renderState.stencilEnable;
					stateDesc.StencilReadMask = renderState.stencilTest.stencilReadMask;
					stateDesc.StencilWriteMask = renderState.stencilTest.stencilWriteMask;
					stateDesc.FrontFace.StencilFailOp = (D3D11_STENCIL_OP)renderState.stencilTest.frontFail;
					stateDesc.FrontFace.StencilDepthFailOp = (D3D11_STENCIL_OP)renderState.stencilTest.frontDepthFail;
					stateDesc.FrontFace.StencilPassOp = (D3D11_STENCIL_OP)renderState.stencilTest.frontPass;
					stateDesc.FrontFace.StencilFunc = (D3D11_COMPARISON_FUNC)renderState.stencilTest.frontFunc;
					stateDesc.BackFace.StencilFailOp = (D3D11_STENCIL_OP)renderState.stencilTest.backFail;
					stateDesc.BackFace.StencilDepthFailOp = (D3D11_STENCIL_OP)renderState.stencilTest.backDepthFail;
					stateDesc.BackFace.StencilPassOp = (D3D11_STENCIL_OP)renderState.stencilTest.backPass;
					stateDesc.BackFace.StencilFunc = (D3D11_COMPARISON_FUNC)renderState.stencilTest.backFunc;

					ID3D11DepthStencilState* newDepthState = nullptr;
					impl->device->CreateDepthStencilState(&stateDesc, &newDepthState);
					if (newDepthState)
					{
						impl->deviceContext->OMSetDepthStencilState(newDepthState, renderState.stencilRef);
						impl->depthState = newDepthState;
						impl->depthStateHash = depthStateHash;
						impl->stencilRef = renderState.stencilRef;
						depthStates[depthStateHash] = newDepthState;

						LOGDEBUGF("Created new depth state with hash %x", depthStateHash & 0xffffffff);
					}
				}
			}

			depthStateDirty = false;
		}

		if (rasterizerStateDirty)
		{
			unsigned long long rasterizerStateHash =
				(renderState.depthClip ? 0x1 : 0x0) |
				(renderState.scissorEnable ? 0x2 : 0x0) |
				(renderState.fillMode << 2) |
				(renderState.cullMode << 4) |
				((renderState.depthBias & 0xff) << 6) |
				((unsigned long long)*((unsigned*)&renderState.slopeScaledDepthBias) << 14);

			if (rasterizerStateHash != impl->rasterizerStateHash)
			{
				auto it = rasterizerStates.find(rasterizerStateHash);
				if (it != rasterizerStates.end())
				{
					ID3D11RasterizerState* newRasterizerState = (ID3D11RasterizerState*)it->second;
					impl->deviceContext->RSSetState(newRasterizerState);
					impl->rasterizerState = newRasterizerState;
					impl->rasterizerStateHash = rasterizerStateHash;
				}
				else
				{
					ALIMER_PROFILE(CreateRasterizerState);

					D3D11_RASTERIZER_DESC stateDesc;
					memset(&stateDesc, 0, sizeof stateDesc);

					stateDesc.FillMode = (D3D11_FILL_MODE)renderState.fillMode;
					stateDesc.CullMode = (D3D11_CULL_MODE)renderState.cullMode;
					stateDesc.FrontCounterClockwise = FALSE;
					stateDesc.DepthBias = renderState.depthBias;
					stateDesc.DepthBiasClamp = M_INFINITY;
					stateDesc.SlopeScaledDepthBias = renderState.slopeScaledDepthBias;
					stateDesc.DepthClipEnable = renderState.depthClip;
					stateDesc.ScissorEnable = renderState.scissorEnable;
					stateDesc.MultisampleEnable = TRUE;
					stateDesc.AntialiasedLineEnable = FALSE;

					ID3D11RasterizerState* newRasterizerState = nullptr;
					impl->device->CreateRasterizerState(&stateDesc, &newRasterizerState);
					if (newRasterizerState)
					{
						impl->deviceContext->RSSetState(newRasterizerState);
						impl->rasterizerState = newRasterizerState;
						impl->rasterizerStateHash = rasterizerStateHash;
						rasterizerStates[rasterizerStateHash] = newRasterizerState;

						LOGDEBUGF("Created new rasterizer state with hash %x", rasterizerStateHash & 0xffffffff);
					}
				}
			}

			rasterizerStateDirty = false;
		}

		return true;
	}

	void D3D11Graphics::ResetState()
	{
		for (size_t i = 0; i < MAX_VERTEX_STREAMS; ++i)
			vertexBuffers[i] = nullptr;

		for (size_t i = 0; i < MAX_SHADER_STAGES; ++i)
		{
			for (size_t j = 0; j < MAX_CONSTANT_BUFFERS; ++j)
				constantBuffers[i][j] = nullptr;
		}

		for (size_t i = 0; i < MAX_TEXTURE_UNITS; ++i)
		{
			textures[i] = nullptr;
			impl->resourceViews[i] = nullptr;
			impl->samplers[i] = nullptr;
		}

		for (size_t i = 0; i < MAX_RENDERTARGETS; ++i)
			impl->renderTargetViews[i] = nullptr;

		renderState.Reset();

		_indexBuffer = nullptr;
		vertexShader = nullptr;
		pixelShader = nullptr;
		impl->blendState = nullptr;
		impl->depthState = nullptr;
		impl->rasterizerState = nullptr;
		impl->depthStencilView = nullptr;
		impl->blendStateHash = 0xffffffffffffffff;
		impl->depthStateHash = 0xffffffffffffffff;
		impl->rasterizerStateHash = 0xffffffffffffffff;
		impl->stencilRef = 0;
		inputLayout.first = 0;
		inputLayout.second = 0;
		texturesDirty = false;
		inputLayoutDirty = false;
		blendStateDirty = false;
		depthStateDirty = false;
		rasterizerStateDirty = false;
		scissorRectDirty = false;
		primitiveType = MAX_PRIMITIVE_TYPES;
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
