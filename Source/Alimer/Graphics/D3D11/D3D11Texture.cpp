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
#include "D3D11Graphics.h"
#include "../Texture.h"

namespace Alimer
{
	static const D3D11_FILTER s_filterMode[] =
	{
		D3D11_FILTER_MIN_MAG_MIP_POINT,
		D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT,
		D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		D3D11_FILTER_ANISOTROPIC,
		D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT,
		D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
		D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR,
		D3D11_FILTER_COMPARISON_ANISOTROPIC
	};
	static_assert(
		ecast(TextureFilterMode::Count) == _countof(s_filterMode), "TextureFilterMode miss match");

	static const D3D11_TEXTURE_ADDRESS_MODE s_addressMode[] =
	{
		D3D11_TEXTURE_ADDRESS_WRAP,			// Wrap
		D3D11_TEXTURE_ADDRESS_MIRROR,		// Mirror,
		D3D11_TEXTURE_ADDRESS_CLAMP,		// Clamp,
		D3D11_TEXTURE_ADDRESS_BORDER,		// Border,
		D3D11_TEXTURE_ADDRESS_MIRROR_ONCE	// MirrorOnce
	};
	static_assert(
		ecast(SamplerAddressMode::Count) == _countof(s_addressMode), "SamplerAddressMode miss match");

	static const DXGI_FORMAT s_textureFormat[] =
	{
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_A8_UNORM,
		DXGI_FORMAT_R8_UNORM,
		DXGI_FORMAT_R8G8_UNORM,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_R16_UNORM,
		DXGI_FORMAT_R16G16_UNORM,
		DXGI_FORMAT_R16G16B16A16_UNORM,
		DXGI_FORMAT_R16_FLOAT,
		DXGI_FORMAT_R16G16_FLOAT,
		DXGI_FORMAT_R16G16B16A16_FLOAT,
		DXGI_FORMAT_R32_FLOAT,
		DXGI_FORMAT_R32G32_FLOAT,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		DXGI_FORMAT_R16_TYPELESS, // Depth16UNorm
		DXGI_FORMAT_R32_TYPELESS, // Depth32Float
		DXGI_FORMAT_R24_UNORM_X8_TYPELESS, // Depth24UNormStencil8
		DXGI_FORMAT_R24G8_TYPELESS,	// Stencil8
		DXGI_FORMAT_BC1_UNORM,
		DXGI_FORMAT_BC2_UNORM,
		DXGI_FORMAT_BC3_UNORM,
		DXGI_FORMAT_UNKNOWN, //ETC1,
		DXGI_FORMAT_UNKNOWN, //PVRTC_RGB_2BPP,
		DXGI_FORMAT_UNKNOWN, //PVRTC_RGBA_2BPP,
		DXGI_FORMAT_UNKNOWN, //PVRTC_RGB_4BPP,
		DXGI_FORMAT_UNKNOWN, //,
	};
	static_assert(
		ecast(PixelFormat::Count) == _countof(s_textureFormat), "PixelFormat missmatch");

	static const DXGI_FORMAT depthStencilViewFormat[] =
	{
		DXGI_FORMAT_D16_UNORM,
		DXGI_FORMAT_D32_FLOAT,
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		DXGI_FORMAT_D24_UNORM_S8_UINT // Stencil8
	};

	static const DXGI_FORMAT depthStencilResourceViewFormat[] =
	{
		DXGI_FORMAT_R16_UNORM,
		DXGI_FORMAT_R32_FLOAT,
		DXGI_FORMAT_R24_UNORM_X8_TYPELESS
	};

	static const D3D11_SRV_DIMENSION srvDimension[] =
	{
		D3D11_SRV_DIMENSION_TEXTURE1D,
		D3D11_SRV_DIMENSION_TEXTURE2D,
		D3D11_SRV_DIMENSION_TEXTURE3D,
		D3D11_SRV_DIMENSION_TEXTURECUBE
	};

	static const D3D11_RTV_DIMENSION rtvDimension[] =
	{
		D3D11_RTV_DIMENSION_TEXTURE1D,
		D3D11_RTV_DIMENSION_TEXTURE2D,
		D3D11_RTV_DIMENSION_TEXTURE3D,
		D3D11_RTV_DIMENSION_TEXTURE2D, /// \todo Implement views per face
	};

	static const D3D11_DSV_DIMENSION dsvDimension[] =
	{
		D3D11_DSV_DIMENSION_TEXTURE1D,
		D3D11_DSV_DIMENSION_TEXTURE2D,
		D3D11_DSV_DIMENSION_TEXTURE2D,
		D3D11_DSV_DIMENSION_TEXTURE2D,
	};

	void Texture::Release()
	{
		if (graphics)
		{
			for (uint32_t i = 0; i < MAX_TEXTURE_UNITS; ++i)
			{
				//if (graphics->GetTexture(i) == this)
				//	graphics->SetTexture(i, 0);
			}

			if (_usage & TextureUsageBits::RenderTarget)
			{
				bool clear = false;

				for (uint32_t i = 0; i < MAX_RENDERTARGETS; ++i)
				{
					//if (graphics->GetRenderTarget(i) == this)
					//{
					//	clear = true;
					//	break;
					//}
				}

				if (!clear && graphics->GetDepthStencil() == this)
				{
					clear = true;
				}

				if (clear)
					graphics->ResetRenderTargets();
			}
		}

		if (_resourceView)
		{
			_resourceView->Release();
			_resourceView = nullptr;
		}
		if (_renderTargetView)
		{
			if (IsRenderTarget())
			{
				ID3D11RenderTargetView* d3dRenderTargetView = (ID3D11RenderTargetView*)_renderTargetView;
				d3dRenderTargetView->Release();
			}
			else if (IsDepthStencil())
			{
				ID3D11DepthStencilView* d3dDepthStencilView = (ID3D11DepthStencilView*)_renderTargetView;
				d3dDepthStencilView->Release();
			}
			_renderTargetView = nullptr;
		}

		SafeRelease(_sampler);
		SafeRelease(_texture);
	}

	bool Texture::Define(
		TextureType type, 
		const Size& size, 
		PixelFormat format,
		uint32_t mipLevels,
		TextureUsage usage,
		const ImageLevel* initialData)
	{
		ALIMER_PROFILE(DefineTexture);
		Release();

		if (type != TextureType::Type2D
			&& type != TextureType::TypeCube)
		{
			ALIMER_LOGERROR("Only 2D textures and cube maps supported for now");
			return false;
		}

		if (format > PixelFormat::BC3)
		{
			ALIMER_LOGERROR("ETC1 and PVRTC formats are unsupported");
			return false;
		}
		if (type == TextureType::TypeCube 
			&& size.width != size.height)
		{
			ALIMER_LOGERROR("Cube map must have square dimensions");
			return false;
		}

		if (mipLevels < 1)
			mipLevels = 1;

		_type = type;
		_usage = usage;

		D3D11_USAGE textureUsage = D3D11_USAGE_DEFAULT;
		UINT d3dUsage = 0;
		if (usage & TextureUsageBits::ShaderRead)
			d3dUsage |= D3D11_BIND_SHADER_RESOURCE;

		if (usage & TextureUsageBits::ShaderWrite)
			d3dUsage |= D3D11_BIND_UNORDERED_ACCESS;

		if (graphics && graphics->IsInitialized())
		{
			ID3D11Device1* d3dDevice = static_cast<D3D11Graphics*>(graphics.Get())->GetD3DDevice();

			D3D11_TEXTURE2D_DESC textureDesc = {};
			textureDesc.Width = size.width;
			textureDesc.Height = size.height;
			textureDesc.MipLevels = mipLevels;
			textureDesc.ArraySize = NumFaces();
			textureDesc.Format = s_textureFormat[ecast(format)];
			/// \todo Support defining multisampled textures
			textureDesc.SampleDesc.Count = 1;
			textureDesc.SampleDesc.Quality = 0;
			textureDesc.Usage = textureUsage;
			textureDesc.BindFlags = d3dUsage;

			if (usage & TextureUsageBits::RenderTarget)
			{
				if (!IsDepthStencilFormat(format))
				{
					textureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
				}
				else
				{
					textureDesc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
				}
			}

			const bool dynamic = false;
			textureDesc.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
			if (_type == TextureType::TypeCube)
			{
				textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
			}

			std::vector<D3D11_SUBRESOURCE_DATA> subResourceData;
			if (initialData)
			{
				subResourceData.resize(NumFaces() * mipLevels);
				for (size_t i = 0; i < NumFaces() * mipLevels; ++i)
				{
					subResourceData[i].pSysMem = initialData[i].data;
					subResourceData[i].SysMemPitch = (UINT)initialData[i].rowSize;
					subResourceData[i].SysMemSlicePitch = 0;
				}
			}

			HRESULT hr = d3dDevice->CreateTexture2D(
				&textureDesc, 
				subResourceData.size() ? subResourceData.data() : (D3D11_SUBRESOURCE_DATA*)nullptr, 
				(ID3D11Texture2D**)&_texture);

			if (FAILED(hr))
			{
				_size = Size::Empty;
				_format = PixelFormat::Undefined;
				_mipLevels = 0;

				ALIMER_LOGERROR("Failed to create texture");
				return false;
			}
			else
			{
				_size = size;
				_format = format;
				_mipLevels = mipLevels;

				ALIMER_LOGDEBUG(
					"Created texture width {} height {} format {} numLevels {}",
					_size.width,
					_size.height,
					EnumToString(_format),
					_mipLevels);
			}

			D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDesc = {};
			resourceViewDesc.ViewDimension = srvDimension[static_cast<uint8_t>(_type)];
			switch (_type)
			{
			case TextureType::Type2D:
				resourceViewDesc.Texture2D.MipLevels = _mipLevels;
				resourceViewDesc.Texture2D.MostDetailedMip = 0;
				break;

			case TextureType::TypeCube:
				resourceViewDesc.TextureCube.MipLevels = _mipLevels;
				resourceViewDesc.TextureCube.MostDetailedMip = 0;
				break;
			}
			resourceViewDesc.Format = textureDesc.Format;

			if (IsRenderTarget())
			{
				D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
				renderTargetViewDesc.Format = textureDesc.Format;
				renderTargetViewDesc.ViewDimension = rtvDimension[(uint8_t)_type];
				renderTargetViewDesc.Texture2D.MipSlice = 0;

				d3dDevice->CreateRenderTargetView(_texture, &renderTargetViewDesc, (ID3D11RenderTargetView**)&_renderTargetView);
				if (!_renderTargetView)
				{
					ALIMER_LOGERROR("Failed to create rendertarget view for texture");
				}
			}
			else if (IsDepthStencil())
			{
				D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
				// Readable depth textures are created typeless, while the actual format is specified for the depth stencil
				// and shader resource views
				resourceViewDesc.Format = depthStencilResourceViewFormat[ecast(_format) - ecast(PixelFormat::Depth16UNorm)];
				depthStencilViewDesc.Format = depthStencilViewFormat[ecast(_format) - ecast(PixelFormat::Depth16UNorm)];
				depthStencilViewDesc.ViewDimension = dsvDimension[(uint8_t)_type];
				depthStencilViewDesc.Flags = 0;

				d3dDevice->CreateDepthStencilView(_texture, &depthStencilViewDesc, (ID3D11DepthStencilView**)&_renderTargetView);
				if (!_renderTargetView)
				{
					ALIMER_LOGERROR("Failed to create depth-stencil view for texture");
				}
			}

			d3dDevice->CreateShaderResourceView(_texture, &resourceViewDesc, (ID3D11ShaderResourceView**)&_resourceView);
			if (!_resourceView)
			{
				ALIMER_LOGERROR("Failed to create shader resource view for texture");
			}
		}

		return true;
	}

	bool Texture::DefineSampler(
		TextureFilterMode filter,
		SamplerAddressMode u, 
		SamplerAddressMode v,
		SamplerAddressMode w,
		uint32_t maxAnisotropy,
		float minLod, 
		float maxLod, 
		const Color& borderColor)
	{
		ALIMER_PROFILE(DefineTextureSampler);

		_filter = filter;
		_addressModes[0] = u;
		_addressModes[1] = v;
		_addressModes[2] = w;
		_maxAnisotropy = maxAnisotropy;
		_minLod = minLod;
		_maxLod = maxLod;
		_borderColor = borderColor;

		// Release the previous sampler first.
		SafeRelease(_sampler);

		if (graphics
			&& graphics->IsInitialized())
		{
			D3D11_SAMPLER_DESC samplerDesc = {};

			samplerDesc.Filter = s_filterMode[ecast(filter)];
			samplerDesc.AddressU = s_addressMode[ecast(u)];
			samplerDesc.AddressV = s_addressMode[ecast(v)];
			samplerDesc.AddressW = s_addressMode[ecast(w)];
			samplerDesc.MaxAnisotropy = maxAnisotropy;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
			samplerDesc.MinLOD = minLod;
			samplerDesc.MaxLOD = maxLod;
			memcpy(&samplerDesc.BorderColor, borderColor.Data(), 4 * sizeof(float));

			ID3D11Device1* d3dDevice = static_cast<D3D11Graphics*>(graphics.Get())->GetD3DDevice();
			d3dDevice->CreateSamplerState(&samplerDesc, &_sampler);

			if (!_sampler)
			{
				ALIMER_LOGERROR("Failed to create sampler state");
				return false;
			}
			
			ALIMER_LOGDEBUG("Created sampler state");
		}

		return true;
	}

	bool Texture::SetData(uint32_t face, uint32_t level, IntRect rect, const ImageLevel& data)
	{
		ALIMER_PROFILE(UpdateTextureLevel);

		if (_texture)
		{
			if (face >= NumFaces())
			{
				ALIMER_LOGERROR("Face to update out of bounds");
				return false;
			}

			if (level >= _mipLevels)
			{
				ALIMER_LOGERROR("Mipmap level to update out of bounds");
				return false;
			}

			IntRect levelRect(0, 0, 
				Max(_size.width >> level, 1), 
				Max(_size.height >> level, 1));
			if (levelRect.IsInside(rect) != INSIDE)
			{
				ALIMER_LOGERROR("Texture update region is outside level");
				return false;
			}

			// If compressed, align the update region on a block
			if (IsCompressed())
			{
				rect.left &= 0xfffffffc;
				rect.top &= 0xfffffffc;
				rect.right += 3;
				rect.bottom += 3;
				rect.right &= 0xfffffffc;
				rect.bottom &= 0xfffffffc;
			}

			ID3D11DeviceContext* d3dDeviceContext = static_cast<D3D11Graphics*>(graphics.Get())->GetD3DDeviceContext();
			uint32_t subResource = D3D11CalcSubresource(
				level, 
				face, 
				_mipLevels);
			
			const bool dynamic = false;
			if (dynamic)
			{
				const uint32_t pixelByteSize = GetPixelFormatSize(_format);
				if (!pixelByteSize)
				{
					ALIMER_LOGERROR("Updating dynamic compressed texture is not supported");
					return false;
				}

				D3D11_MAPPED_SUBRESOURCE mappedData;
				mappedData.pData = nullptr;

				d3dDeviceContext->Map(_texture, subResource, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
				if (mappedData.pData)
				{
					for (int y = rect.top; y < rect.bottom; ++y)
					{
						memcpy(
							(uint8_t*)mappedData.pData + y * mappedData.RowPitch + rect.left + pixelByteSize, 
							data.data + (y - rect.top) * data.rowSize, 
							(rect.right - rect.left) * pixelByteSize);
					}

					d3dDeviceContext->Unmap(_texture, subResource);
				}
				else
				{
					ALIMER_LOGERROR("Failed to map texture for update");
					return false;
				}
			}
			else
			{
				D3D11_BOX destBox;
				destBox.left = rect.left;
				destBox.right = rect.right;
				destBox.top = rect.top;
				destBox.bottom = rect.bottom;
				destBox.front = 0;
				destBox.back = 1;

				d3dDeviceContext->UpdateSubresource(
					_texture, 
					subResource, 
					&destBox, 
					data.data,
					(unsigned)data.rowSize, 
					0);
			}
		}

		return true;
	}

	void* Texture::D3DRenderTargetView(uint32_t /*index*/) const
	{
		/// \todo Handle different indices for eg. cube maps
		return _renderTargetView;
	}
}
