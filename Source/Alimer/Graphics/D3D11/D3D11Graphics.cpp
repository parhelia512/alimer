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
using namespace Microsoft::WRL;

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

#if ALIMER_PLATFORM_WINDOWS && !ALIMER_PLATFORM_UWP
	typedef HRESULT(WINAPI* PFN_CREATE_DXGI_FACTORY)(REFIID riid, _COM_Outptr_ void **ppFactory);
	typedef HRESULT(WINAPI* PFN_CREATE_DXGI_FACTORY2)(UINT flags, REFIID _riid, _Out_ void** ppFactory);
	typedef HRESULT(WINAPI* PFN_GET_DXGI_DEBUG_INTERFACE)(UINT flags, REFIID _riid, void** _debug);

	bool libInitialized = false;
	HMODULE DXGIDebugHandle = nullptr;
	HMODULE DXGIHandle = nullptr;
	HMODULE D3D11Handle = nullptr;

	PFN_GET_DXGI_DEBUG_INTERFACE DXGIGetDebugInterface1 = nullptr;
	PFN_CREATE_DXGI_FACTORY CreateDXGIFactory1;
	PFN_CREATE_DXGI_FACTORY2 CreateDXGIFactory2;

	PFN_D3D11_CREATE_DEVICE D3D11CreateDevice;

	HRESULT LoadLibraries()
	{
		if (libInitialized)
			return S_OK;

		DXGIDebugHandle = ::LoadLibraryW(L"dxgidebug.dll");
		DXGIHandle = ::LoadLibraryW(L"dxgi.dll");
		D3D11Handle = ::LoadLibraryW(L"d3d11.dll");

		if (!DXGIHandle)
			return E_FAIL;
		if (!D3D11Handle)
			return E_FAIL;

		// Load symbols.
		DXGIGetDebugInterface1 = (PFN_GET_DXGI_DEBUG_INTERFACE)::GetProcAddress(DXGIHandle, "DXGIGetDebugInterface1");
		CreateDXGIFactory1 = (PFN_CREATE_DXGI_FACTORY)::GetProcAddress(DXGIHandle, "CreateDXGIFactory1");
		CreateDXGIFactory2 = (PFN_CREATE_DXGI_FACTORY2)::GetProcAddress(DXGIHandle, "CreateDXGIFactory2");

		// We need at least D3D11.1
		if (!CreateDXGIFactory1)
			return E_FAIL;

		D3D11CreateDevice = (PFN_D3D11_CREATE_DEVICE)::GetProcAddress(D3D11Handle, "D3D11CreateDevice");

		if (!D3D11CreateDevice)
			return E_FAIL;

		return S_OK;
	}
#endif

#if defined(_DEBUG)
	// Check for SDK Layer support.
	inline bool SdkLayersAvailable()
	{
		HRESULT hr = D3D11CreateDevice(
			nullptr,
			D3D_DRIVER_TYPE_NULL,       // There is no need to create a real hardware device.
			0,
			D3D11_CREATE_DEVICE_DEBUG,  // Check for the SDK layers.
			nullptr,                    // Any feature level will do.
			0,
			D3D11_SDK_VERSION,          // Always set this to D3D11_SDK_VERSION for Windows Runtime apps.
			nullptr,                    // No need to keep the D3D device reference.
			nullptr,                    // No need to know the feature level.
			nullptr                     // No need to keep the D3D device context reference.
		);

		return SUCCEEDED(hr);
	}
#endif

	/// \cond PRIVATE
	struct GraphicsImpl
	{
		/// Construct.
		GraphicsImpl() :
			defaultRenderTargetView(nullptr),
			defaultDepthTexture(nullptr),
			defaultDepthStencilView(nullptr),
			depthStencilView(nullptr),
			blendStateHash(0xffffffffffffffff),
			depthStateHash(0xffffffffffffffff),
			rasterizerStateHash(0xffffffffffffffff),
			stencilRef(0)
		{
			for (size_t i = 0; i < MAX_RENDERTARGETS; ++i)
				renderTargetViews[i] = nullptr;
		}

		/// Default (backbuffer) rendertarget view.
		ID3D11RenderTargetView* defaultRenderTargetView;
		/// Default depth-stencil texture.
		ID3D11Texture2D* defaultDepthTexture;
		/// Default depth-stencil view.
		ID3D11DepthStencilView* defaultDepthStencilView;
		
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
#if ALIMER_PLATFORM_WINDOWS && !ALIMER_PLATFORM_UWP
		if (FAILED(LoadLibraries()))
		{
			LOGERRORF("Failed to load D3D11 libraries.");
		}
#endif

		if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory))))
		{
			LOGERRORF("Failed to create DXGIFactory.");
		}

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

	bool D3D11Graphics::Initialize(const GraphicsSettings& settings)
	{
		if (_initialized)
			return true;

		_window = settings.window;
		uint32_t multisample = Clamp(settings.multisample, 1, 16);

		// Create D3D11 device and swap chain when setting mode for the first time, or swap chain again when changing multisample
		if (!_d3dDevice
			|| _multisample != multisample)
		{
			if (!CreateD3DDevice(multisample))
				return false;

			// Swap chain needs to be updated manually for the first time, otherwise window resize event takes care of it
			UpdateSwapChain(_window->GetWidth(), _window->GetHeight());
		}

		screenModeEvent.size = _backbufferSize;
		screenModeEvent.fullscreen = _window->IsFullscreen();
		screenModeEvent.resizable = _window->IsResizable();
		screenModeEvent.multisample = multisample;
		SendEvent(screenModeEvent);

		LOGDEBUGF("Set screen mode %dx%d fullscreen %d resizable %d multisample %d",
			_backbufferSize.width,
			_backbufferSize.height,
			_window->IsFullscreen(), 
			_window->IsResizable(), multisample);

		_initialized = true;
		return true;
	}

	bool Graphics::SetMultisample(uint32_t multisample)
	{
		if (!IsInitialized())
			return false;

		// TODO: Handle
		_multisample = multisample;
		return false;
		//return SetMode(_backbufferSize, _window->IsFullscreen(), _window->IsResizable(), multisample_);
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

		for (auto it = _blendStates.begin(); it != _blendStates.end(); ++it)
		{
			it->second->Release();
		}
		_blendStates.clear();

		for (auto it = _depthStates.begin(); it != _depthStates.end(); ++it)
		{
			it->second->Release();
		}
		_depthStates.clear();

		for (auto it = _rasterizerStates.begin(); it != _rasterizerStates.end(); ++it)
		{
			it->second->Release();
		}
		_rasterizerStates.clear();

		if (_d3dContext)
		{
			ID3D11RenderTargetView* nullViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = { nullptr };
			_d3dContext->OMSetRenderTargets(ARRAYSIZE(nullViews), nullViews, nullptr);
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


		_swapChain.Reset();
		_d3dContext.Reset();
		_d3dDevice.Reset();
		_window->Close();
		ResetState();
	}

	bool D3D11Graphics::BeginFrame()
	{
		if (!_initialized)
			return false;

		return true;
	}

	void D3D11Graphics::Present()
	{
		if (!_initialized)
			return;

		{
			ALIMER_PROFILE(Present);

			HRESULT hr = _swapChain->Present(vsync ? 1 : 0, 0);
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

	void D3D11Graphics::SetRenderTargets(
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

		_d3dContext->OMSetRenderTargets(MAX_RENDERTARGETS, impl->renderTargetViews, impl->depthStencilView);
	}

	void D3D11Graphics::SetViewport(const IntRect& viewport_)
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

		_d3dContext->RSSetViewports(1, &d3dViewport);
	}

	void D3D11Graphics::SetVertexBuffer(uint32_t index, VertexBuffer* buffer)
	{
		if (index < MAX_VERTEX_STREAMS
			&& buffer != vertexBuffers[index])
		{
			vertexBuffers[index] = buffer;
			ID3D11Buffer* d3dBuffer = buffer ? static_cast<D3D11Buffer*>(buffer->GetHandle())->GetD3DBuffer() : nullptr;
			UINT stride = buffer ? buffer->GetStride() : 0;
			UINT offset = 0;
			_d3dContext->IASetVertexBuffers(
				index,
				1,
				&d3dBuffer,
				&stride,
				&offset);

			_inputLayoutDirty = true;
		}
	}

	void D3D11Graphics::SetConstantBuffer(ShaderStage stage, uint32_t index, ConstantBuffer* buffer)
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
				_d3dContext->VSSetConstantBuffers(index, 1, &d3dBuffer);
				break;

			case SHADER_PS:
				_d3dContext->PSSetConstantBuffers(index, 1, &d3dBuffer);
				break;

			default:
				break;
			}
		}
	}

	void D3D11Graphics::SetTexture(size_t index, Texture* texture)
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
		if (handle == _currentIndexBuffer)
			return;

		// Unset?
		if (!handle)
		{
			_currentIndexBuffer = nullptr;
			_d3dContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
			return;
		}

		_currentIndexBuffer = static_cast<D3D11Buffer*>(handle);
		_d3dContext->IASetIndexBuffer(
			_currentIndexBuffer->GetD3DBuffer(),
			d3d11::Convert(type),
			0);
	}

	void D3D11Graphics::SetShaders(ShaderVariation* vs, ShaderVariation* ps)
	{
		if (vs != vertexShader)
		{
			if (vs && vs->GetStage() == SHADER_VS)
			{
				if (!vs->IsCompiled())
					vs->Compile();
				_d3dContext->VSSetShader((ID3D11VertexShader*)vs->ShaderObject(), nullptr, 0);
			}
			else
				_d3dContext->VSSetShader(nullptr, nullptr, 0);

			vertexShader = vs;
			_inputLayoutDirty = true;
		}

		if (ps != pixelShader)
		{
			if (ps && ps->GetStage() == SHADER_PS)
			{
				if (!ps->IsCompiled())
					ps->Compile();
				_d3dContext->PSSetShader((ID3D11PixelShader*)ps->ShaderObject(), nullptr, 0);
			}
			else
			{
				_d3dContext->PSSetShader(nullptr, nullptr, 0);
			}

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

	void D3D11Graphics::SetScissorTest(bool scissorEnable, const IntRect& scissorRect)
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
			_d3dContext->RSSetScissorRects(1, &d3dRect);
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

	void D3D11Graphics::Clear(ClearFlags clearFlags, const Color& clearColor, float clearDepth, uint8_t clearStencil)
	{
		PrepareTextures();

		if (any(clearFlags & ClearFlags::Color)
			&& impl->renderTargetViews[0])
		{
			_d3dContext->ClearRenderTargetView(impl->renderTargetViews[0], clearColor.Data());
		}

		if (any(clearFlags & (ClearFlags::Depth | ClearFlags::Stencil))
			&& impl->depthStencilView)
		{
			UINT depthClearFlags = 0;
			if (any(clearFlags & ClearFlags::Depth))
				depthClearFlags |= D3D11_CLEAR_DEPTH;
			if (any(clearFlags & ClearFlags::Stencil))
				depthClearFlags |= D3D11_CLEAR_STENCIL;
			_d3dContext->ClearDepthStencilView(impl->depthStencilView, depthClearFlags, clearDepth, clearStencil);
		}
	}

	void D3D11Graphics::Draw(PrimitiveType type, uint32_t vertexStart, uint32_t vertexCount)
	{
		if (!PrepareDraw(type))
			return;

		_d3dContext->Draw(vertexCount, vertexStart);
	}

	void D3D11Graphics::DrawIndexed(PrimitiveType type, uint32_t indexStart, uint32_t indexCount, uint32_t vertexStart)
	{
		if (!PrepareDraw(type))
			return;

		_d3dContext->DrawIndexed(indexCount, indexStart, vertexStart);
	}

	void D3D11Graphics::DrawInstanced(
		PrimitiveType type, 
		uint32_t vertexStart, 
		uint32_t vertexCount, 
		uint32_t instanceStart,
		uint32_t instanceCount)
	{
		if (!PrepareDraw(type))
			return;

		_d3dContext->DrawInstanced(
			vertexCount, 
			instanceCount, 
			vertexStart,
			instanceStart);
	}

	void D3D11Graphics::DrawIndexedInstanced(
		PrimitiveType type, 
		uint32_t indexStart,
		uint32_t indexCount,
		uint32_t vertexStart,
		uint32_t instanceStart,
		uint32_t instanceCount)
	{
		if (!PrepareDraw(type))
			return;

		_d3dContext->DrawIndexedInstanced(
			indexCount, 
			instanceCount, 
			indexStart, 
			vertexStart, 
			instanceStart);
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

	bool D3D11Graphics::CreateD3DDevice(uint32_t multisample)
	{
		HRESULT hr = S_OK;
		// Device needs only to be created once.
		if (!_d3dDevice)
		{
			// This flag adds support for surfaces with a different color channel ordering
			// than the API default. It is required for compatibility with Direct2D.
			UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
			if (SdkLayersAvailable())
			{
				// If the project is in a debug build, enable debugging via SDK Layers with this flag.
				creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
			}
#endif

			D3D_FEATURE_LEVEL featureLevels[] =
			{
				D3D_FEATURE_LEVEL_11_1,
				D3D_FEATURE_LEVEL_11_0,
				D3D_FEATURE_LEVEL_10_1,
				D3D_FEATURE_LEVEL_10_0
			};

			ComPtr<ID3D11Device> device;
			ComPtr<ID3D11DeviceContext> context;
			hr = D3D11CreateDevice(
				nullptr,
				D3D_DRIVER_TYPE_HARDWARE,
				0,
				creationFlags,
				featureLevels,
				ARRAYSIZE(featureLevels),
				D3D11_SDK_VERSION,
				&device,
				&_d3dFeatureLevel,
				&context
			);

			if (FAILED(hr))
			{
				// If the initialization fails, fall back to the WARP device.
				// For more information on WARP, see:
				// http://go.microsoft.com/fwlink/?LinkId=286690
				hr = D3D11CreateDevice(
					nullptr,              // Use the default DXGI adapter for WARP.
					D3D_DRIVER_TYPE_WARP, // Create a WARP device instead of a hardware device.
					0,
					creationFlags,
					featureLevels,
					ARRAYSIZE(featureLevels),
					D3D11_SDK_VERSION,
					&device,
					&_d3dFeatureLevel,
					&context
				);

				if (FAILED(hr))
				{
					LOGERROR("Failed to create D3D11 device");
					return false;
				}
			}

			// Store pointers to the Direct3D device and immediate context.
			ThrowIfFailed(
				device.As(&_d3dDevice)
			);

			ThrowIfFailed(
				context.As(&_d3dContext)
			);

			_debugMode = _d3dDevice->GetCreationFlags() & D3D11_CREATE_DEVICE_DEBUG;
		}

		// Create swap chain. Release old if necessary
		_swapChain.Reset();

		WindowPlatformData platformData = _window->GetPlatformData();

		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		swapChainDesc.BufferCount = 1;
		swapChainDesc.BufferDesc.Width = _window->GetWidth();
		swapChainDesc.BufferDesc.Height = _window->GetHeight();
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.OutputWindow = platformData.hwnd;
		swapChainDesc.SampleDesc.Count = multisample;
		swapChainDesc.SampleDesc.Quality = multisample > 1 ? 0xffffffff : 0;
		swapChainDesc.Windowed = TRUE;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		hr = _dxgiFactory->CreateSwapChain(
			_d3dDevice.Get(),
			&swapChainDesc,
			_swapChain.ReleaseAndGetAddressOf());

#if !ALIMER_PLATFORM_UWP
		// After creating the swap chain, disable automatic Alt-Enter fullscreen/windowed switching
		// (the application will switch manually if it wants to)
		_dxgiFactory->MakeWindowAssociation(
			platformData.hwnd,
			DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);
#endif

		if (SUCCEEDED(hr))
		{
			_multisample = multisample;
			return true;
		}

		LOGERROR("Failed to create D3D11 swap chain");
		return false;
	}

	bool D3D11Graphics::UpdateSwapChain(uint32_t width, uint32_t height)
	{
		bool success = true;

		ID3D11RenderTargetView* nullViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = { nullptr };
		_d3dContext->OMSetRenderTargets(ARRAYSIZE(nullViews), nullViews, nullptr);

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

		_swapChain->ResizeBuffers(1, width, height, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

		// Create default rendertarget view representing the backbuffer
		ComPtr<ID3D11Texture2D> backbufferTexture;
		_swapChain->GetBuffer(0, IID_PPV_ARGS(&backbufferTexture));
		if (backbufferTexture)
		{
			_d3dDevice->CreateRenderTargetView(backbufferTexture.Get(), 0, &impl->defaultRenderTargetView);
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
		depthDesc.SampleDesc.Count = _multisample;
		depthDesc.SampleDesc.Quality = _multisample > 1 ? 0xffffffff : 0;
		depthDesc.Usage = D3D11_USAGE_DEFAULT;
		depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthDesc.CPUAccessFlags = 0;
		depthDesc.MiscFlags = 0;
		_d3dDevice->CreateTexture2D(&depthDesc, 0, &impl->defaultDepthTexture);
		if (impl->defaultDepthTexture)
		{
			_d3dDevice->CreateDepthStencilView(impl->defaultDepthTexture, 0, &impl->defaultDepthStencilView);
		}
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

#if TOOD
	void D3D11Graphics::HandleResize(WindowResizeEvent& /*event*/)
	{
		// Handle window resize
		if (_swapChain
			&& (_window->GetWidth() != _backbufferSize.width || _window->GetHeight() != _backbufferSize.height))
		{
			UpdateSwapChain(_window->GetWidth(), _window->GetHeight());
		}
	}
#endif // TOOD


	void D3D11Graphics::PrepareTextures()
	{
		if (texturesDirty)
		{
			// Set both VS & PS textures to mimic OpenGL behavior
			_d3dContext->VSSetShaderResources(0, MAX_TEXTURE_UNITS, impl->resourceViews);
			_d3dContext->VSSetSamplers(0, MAX_TEXTURE_UNITS, impl->samplers);
			_d3dContext->PSSetShaderResources(0, MAX_TEXTURE_UNITS, impl->resourceViews);
			_d3dContext->PSSetSamplers(0, MAX_TEXTURE_UNITS, impl->samplers);
			texturesDirty = false;
		}
	}

	BufferHandle* D3D11Graphics::CreateBuffer(BufferUsage usage, uint32_t size, uint32_t stride, ResourceUsage resourceUsage, const void* initialData)
	{
		return new D3D11Buffer(this, usage, size, stride, resourceUsage, initialData);
	}

	bool D3D11Graphics::PrepareDraw(PrimitiveType type)
	{
		PrepareTextures();

		if (!vertexShader || !pixelShader || !vertexShader->ShaderObject() || !pixelShader->ShaderObject())
			return false;

		if (primitiveType != type)
		{
			_d3dContext->IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY)type);
			primitiveType = type;
		}

		HRESULT hr = S_OK;
		if (_inputLayoutDirty)
		{
			_inputLayoutDirty = false;

			InputLayoutDesc newInputLayout;
			newInputLayout.first = 0;
			newInputLayout.second = vertexShader->ElementHash();
			for (size_t i = 0; i < MAX_VERTEX_STREAMS; ++i)
			{
				if (vertexBuffers[i])
				{
					newInputLayout.first |= (uint64_t)vertexBuffers[i]->GetElementHash() << (i * 16);
				}
			}

			if (_currentInputLayout != newInputLayout)
			{
				// Check if layout already exists
				auto it = _inputLayouts.find(newInputLayout);
				if (it != _inputLayouts.end())
				{
					_d3dContext->IASetInputLayout(it->second);
					_currentInputLayout = newInputLayout;
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

					ID3DBlob* d3dBlob = (ID3DBlob*)vertexShader->BlobObject();
					ID3D11InputLayout* d3dInputLayout = nullptr;
					hr = _d3dDevice->CreateInputLayout(
						elementDescs.data(),
						static_cast<UINT>(elementDescs.size()),
						d3dBlob->GetBufferPointer(),
						d3dBlob->GetBufferSize(), &d3dInputLayout);

					if (FAILED(hr))
					{
						LOGERROR("Failed to create input layout");
					}
					else
					{
						_inputLayouts[newInputLayout] = d3dInputLayout;
						_d3dContext->IASetInputLayout(d3dInputLayout);
						_currentInputLayout = newInputLayout;
					}
				}
			}
		}

		if (blendStateDirty)
		{
			Hasher h;
			h.UInt32(renderState.colorWriteMask);
			h.UInt32(renderState.alphaToCoverage);
			h.Data(reinterpret_cast<uint32_t *>(&renderState.blendMode), sizeof(renderState.blendMode));

			auto blendStateHash = h.GetValue();

			if (blendStateHash != impl->blendStateHash)
			{
				auto it = _blendStates.find(blendStateHash);
				if (it != end(_blendStates))
				{
					ID3D11BlendState1* newBlendState = it->second;
					_d3dContext->OMSetBlendState(newBlendState, nullptr, 0xffffffff);
					_currentBlendState = newBlendState;
					impl->blendStateHash = blendStateHash;
				}
				else
				{
					ALIMER_PROFILE(CreateBlendState);

					D3D11_BLEND_DESC1 stateDesc = {};

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

					ID3D11BlendState1* newBlendState = nullptr;
					_d3dDevice->CreateBlendState1(&stateDesc, &newBlendState);
					if (newBlendState)
					{
						_d3dContext->OMSetBlendState(newBlendState, nullptr, 0xffffffff);
						_currentBlendState = newBlendState;
						impl->blendStateHash = blendStateHash;
						_blendStates[blendStateHash] = newBlendState;

						LOGDEBUGF("Created new blend state with hash %x", blendStateHash & 0xffffffff);
					}
				}
			}

			blendStateDirty = false;
		}

		if (depthStateDirty)
		{
			uint64_t depthStateHash =
				(renderState.depthWrite ? 0x1 : 0x0) |
				(renderState.stencilEnable ? 0x2 : 0x0) |
				(renderState.depthFunc << 2) |
				(renderState.stencilTest.stencilReadMask << 6) |
				(renderState.stencilTest.stencilWriteMask << 14) |
				(renderState.stencilTest.frontFail << 22) |
				(renderState.stencilTest.frontDepthFail << 26) |
				((uint64_t)renderState.stencilTest.frontPass << 30) |
				((uint64_t)renderState.stencilTest.frontFunc << 34) |
				((uint64_t)renderState.stencilTest.frontFail << 38) |
				((uint64_t)renderState.stencilTest.frontDepthFail << 42) |
				((uint64_t)renderState.stencilTest.frontPass << 46) |
				((uint64_t)renderState.stencilTest.frontFunc << 50);

			if (depthStateHash != impl->depthStateHash || renderState.stencilRef != impl->stencilRef)
			{
				auto it = _depthStates.find(depthStateHash);
				if (it != end(_depthStates))
				{
					ID3D11DepthStencilState* newDepthState = it->second;
					_d3dContext->OMSetDepthStencilState(newDepthState, renderState.stencilRef);
					_currentDepthState = newDepthState;
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
					_d3dDevice->CreateDepthStencilState(&stateDesc, &newDepthState);
					if (newDepthState)
					{
						_d3dContext->OMSetDepthStencilState(newDepthState, renderState.stencilRef);
						_currentDepthState = newDepthState;
						impl->depthStateHash = depthStateHash;
						impl->stencilRef = renderState.stencilRef;
						_depthStates[depthStateHash] = newDepthState;

						LOGDEBUGF("Created new depth state with hash %x", depthStateHash & 0xffffffff);
					}
				}
			}

			depthStateDirty = false;
		}

		if (rasterizerStateDirty)
		{
			uint64_t rasterizerStateHash =
				(renderState.depthClip ? 0x1 : 0x0) |
				(renderState.scissorEnable ? 0x2 : 0x0) |
				(renderState.fillMode << 2) |
				(renderState.cullMode << 4) |
				((renderState.depthBias & 0xff) << 6) |
				((uint64_t)*((uint32_t*)&renderState.slopeScaledDepthBias) << 14);

			if (rasterizerStateHash != impl->rasterizerStateHash)
			{
				auto it = _rasterizerStates.find(rasterizerStateHash);
				if (it != end(_rasterizerStates))
				{
					ID3D11RasterizerState1* newRasterizerState = it->second;
					_d3dContext->RSSetState(newRasterizerState);
					_currentRasterizerState = newRasterizerState;
					impl->rasterizerStateHash = rasterizerStateHash;
				}
				else
				{
					ALIMER_PROFILE(CreateRasterizerState);

					D3D11_RASTERIZER_DESC1 stateDesc = {};

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
					stateDesc.ForcedSampleCount = 0;
					ID3D11RasterizerState1* newRasterizerState = nullptr;
					_d3dDevice->CreateRasterizerState1(&stateDesc, &newRasterizerState);
					if (newRasterizerState)
					{
						_d3dContext->RSSetState(newRasterizerState);
						_currentRasterizerState = newRasterizerState;
						impl->rasterizerStateHash = rasterizerStateHash;
						_rasterizerStates[rasterizerStateHash] = newRasterizerState;

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

		_currentIndexBuffer = nullptr;
		vertexShader = nullptr;
		pixelShader = nullptr;
		_currentBlendState = nullptr;
		_currentDepthState = nullptr;
		_currentRasterizerState = nullptr;
		impl->depthStencilView = nullptr;
		impl->blendStateHash = 0xffffffffffffffff;
		impl->depthStateHash = 0xffffffffffffffff;
		impl->rasterizerStateHash = 0xffffffffffffffff;
		impl->stencilRef = 0;
		_currentInputLayout.first = 0;
		_currentInputLayout.second = 0;
		texturesDirty = false;
		_inputLayoutDirty = false;
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
