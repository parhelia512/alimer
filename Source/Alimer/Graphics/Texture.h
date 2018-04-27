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

#include "../Base/Utils.h"
#include "../Math/Color.h"
#include "../Math/Size.h"
#include "../Resource/Image.h"
#include "../Graphics/GPUObject.h"
#include "../Graphics/GraphicsDefs.h"

#ifdef ALIMER_D3D11
struct ID3D11Resource;
struct ID3D11SamplerState;
struct ID3D11ShaderResourceView;
#endif

namespace Alimer
{
	/// Texture types.
	enum class TextureType : uint8_t
	{
		Type1D = 0,
		Type2D,
		Type3D,
		TypeCube,
	};

	enum class TextureUsageBits : uint32_t
	{
		/// Unknown invalid usage.
		Unknown = 0,
		/// An option that enables reading or sampling from the texture.
		ShaderRead = 1 << 0,
		/// An option that enables writing to the texture.
		ShaderWrite = 1 << 1,
		///An option that enables using the texture as a color, depth, or stencil render target in a RenderPass.
		RenderTarget = 1 << 2,
	};
	using TextureUsage = Flags<TextureUsageBits>;
	ALIMER_FORCE_INLINE TextureUsage operator|(TextureUsageBits bit0, TextureUsageBits bit1)
	{
		return TextureUsage(bit0) | bit1;
	}

	ALIMER_FORCE_INLINE TextureUsage operator~(TextureUsageBits bits)
	{
		return ~(TextureUsage(bits));
	}

	class TextureHandle;

	/// GPU Texture.
	class ALIMER_API Texture : public Resource, public GPUObject
	{
		ALIMER_OBJECT(Texture, Resource);

	public:
		/// Construct.
		Texture() = default;
		/// Destruct.
		~Texture();

		/// Register object factory.
		static void RegisterObject();

		/// Load the texture image data from a stream. Return true on success.
		bool BeginLoad(Stream& source) override;
		/// Finish texture loading by uploading to the GPU. Return true on success.
		bool EndLoad() override;
		/// Release the texture and sampler objects.
		void Release() override;

		/// Define texture type and dimensions and set initial data. %ImageLevel structures only need the data pointer and row byte size filled. Return true on success.
		bool Define(
			TextureType type,
			const Size& size,
			PixelFormat format,
			uint32_t mipLevels,
			TextureUsage usage = TextureUsageBits::ShaderRead,
			const ImageLevel* initialData = 0);

		/// Define sampling parameters. Return true on success.
		bool DefineSampler(
			TextureFilterMode filter = FILTER_TRILINEAR,
			SamplerAddressMode u = SamplerAddressMode::Wrap,
			SamplerAddressMode v = SamplerAddressMode::Wrap,
			SamplerAddressMode w = SamplerAddressMode::Wrap,
			uint32_t maxAnisotropy = 16,
			float minLod = -M_MAX_FLOAT,
			float maxLod = M_MAX_FLOAT,
			const Color& borderColor = Color::BLACK);

		/// Set data for a mipmap level. Not supported for immutable textures. Return true on success.
		bool SetData(uint32_t face, uint32_t level, IntRect rect, const ImageLevel& data);

		/// Return texture type.
		TextureType GetTextureType() const { return _type; }
		/// Return dimensions.
		Size GetSize() const { return _size; }
		/// Return width.
		uint32_t GetWidth() const { return _size.width; }
		/// Return height.
		uint32_t GetHeight() const { return _size.height; }
		/// Return image format.
		PixelFormat GetFormat() const { return _format; }
		/// Return whether uses a compressed format.
		bool IsCompressed() const { return Alimer::IsCompressed(_format); }
		/// Return number of mipmap levels.
		uint32_t GetMipLevels() const { return _mipLevels; }
		uint32_t NumFaces() const {
			return _type == TextureType::TypeCube ? MAX_CUBE_FACES : 1;
		}
		/// Return resource usage type.
		TextureUsage GetUsage() const { return _usage; }
		/// Return whether is a color rendertarget texture.
		bool IsRenderTarget() const { return (_usage & TextureUsageBits::RenderTarget) && !Alimer::IsDepthStencilFormat(_format); }
		/// Return whether is a depth-stencil texture.
		bool IsDepthStencil() const { return (_usage & TextureUsageBits::RenderTarget) && Alimer::IsDepthStencilFormat(_format); }

		TextureHandle* GetHandle() { return _handle; }

#ifdef ALIMER_D3D11
		/// Return the D3D11 texture object. Used internally and should not be called by portable application code.
		ID3D11Resource* D3DTexture() const { return _texture; }
		/// Return the D3D11 shader resource view object. Used internally and should not be called by portable application code.
		ID3D11ShaderResourceView* D3DResourceView() const { return _resourceView; }
		/// Return the D3D11 rendertarget or depth-stencil view object. Used internally and should not be called by portable application code.
		void* D3DRenderTargetView(uint32_t index = 0) const;
		/// Return the D3D11 texture sampler object. Used internally and should not be called by portable application code.
		ID3D11SamplerState* D3DSampler() const { return _sampler; }
#elif ALIMER_OPENGL
#endif

	private:
		/// Texture type.
		TextureType _type{ TextureType::Type2D };
		/// Texture usage mode.
		TextureUsage _usage{ TextureUsageBits::ShaderRead };
		/// Texture dimensions in pixels.
		Size _size = Size::One;

		/// Image format.
		PixelFormat _format{ PixelFormat::Undefined };
		/// Number of mipmap levels.
		uint32_t _mipLevels{ 1 };
		/// Images used for loading.
		std::vector<std::unique_ptr<Image>> _loadImages;

		/// Texture filtering mode.
		TextureFilterMode _filter;
		/// Texture addressing modes for each coordinate axis.
		SamplerAddressMode _addressModes[3];
		/// Maximum anisotropy.
		uint32_t _maxAnisotropy;
		/// Minimum LOD.
		float _minLod;
		/// Maximum LOD.
		float _maxLod;
		/// Border color. Only effective in border addressing mode.
		Color _borderColor;

		/// Backend handle.
		TextureHandle* _handle = nullptr;

#ifdef ALIMER_D3D11
		/// D3D11 texture object.
		ID3D11Resource* _texture = nullptr;
		/// D3D11 resource view object.
		ID3D11ShaderResourceView* _resourceView = nullptr;
		/// D3D11 rendertarget or depth-stencil view object.
		void* _renderTargetView = nullptr;
		/// D3D11 texture sampler object.
		ID3D11SamplerState* _sampler = nullptr;
#elif ALIMER_OPENGL
#endif
	};
}