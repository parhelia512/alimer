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

#include "../AlimerConfig.h"
#include "../Math/Size.h"
#include "Resource.h"
#include <memory>

namespace Alimer
{
	/// Pixel formats.
	enum class PixelFormat : uint32_t
	{
		Undefined = 0,
		A8UNorm,
		R8UNorm,
		RG8UNorm,
		RGBA8UNorm,
		R16UNorm,
		RG16UNorm,
		RGBA16UNorm,
		R16Float,
		RG16Float,
		RGBA16Float,
		R32Float,
		RG32Float,
		RGBA32Float,
		Depth16UNorm,
		Depth32Float,
		Depth24UNormStencil8,
		Stencil8,
		BC1,
		BC2,
		BC3,
		ETC1,
		PVRTC_RGB_2BPP,
		PVRTC_RGBA_2BPP,
		PVRTC_RGB_4BPP,
		PVRTC_RGBA_4BPP,
		Count
	};

	ALIMER_API bool IsDepthFormat(PixelFormat format);
	ALIMER_API bool IsStencilFormat(PixelFormat format);
	ALIMER_API bool IsDepthStencilFormat(PixelFormat format);
	ALIMER_API bool IsCompressed(PixelFormat format);
	ALIMER_API const char* EnumToString(PixelFormat format);
	ALIMER_API uint32_t GetPixelFormatSize(PixelFormat format);

	/// Description of image mip level data.
	struct ALIMER_API ImageLevel
	{
		/// Default construct.
		ImageLevel() = default;

		/// Pointer to pixel data.
		uint8_t* data = nullptr;
		/// Level size in pixels.
		Size size = Size::Empty;
		/// Row size in bytes.
		uint32_t rowSize = 0;
		/// Number of rows.
		uint32_t rows = 0;
	};

	/// %Image resource.
	class ALIMER_API Image : public Resource
	{
		ALIMER_OBJECT(Image, Resource);

	public:
		/// Construct.
		Image() = default;
		/// Destruct.
		~Image() = default;

		/// Register object factory.
		static void RegisterObject();

		/// Load image from a stream. Return true on success.
		bool BeginLoad(Stream& source) override;
		/// Save the image to a stream. Regardless of original format, the image is saved as png. Compressed image data is not supported. Return true on success.
		bool Save(Stream& dest) override;

		/// Set new image pixel dimensions and format. Setting a compressed format is not supported.
		void SetSize(const Size& newSize, PixelFormat newFormat);
		/// Set new pixel data.
		void SetData(const uint8_t* pixelData);

		/// Return image dimensions in pixels.
		const Size& GetSize() const { return _size; }
		/// Return image width in pixels.
		uint32_t GetWidth() const { return _size.width; }
		/// Return image height in pixels.
		uint32_t GetHeight() const { return _size.height; }
		/// Return pixel data.
		uint8_t* Data() const { return _data.get(); }
		/// Return the image format.
		PixelFormat GetFormat() const { return _format; }
		/// Return whether is a compressed image.
		bool IsCompressed() const { return Alimer::IsCompressed(_format); }
		/// Return number of mip levels contained in the image data.
		uint32_t GetMipLevels() const { return _mipLevels; }
		/// Calculate the next mip image with halved width and height. Supports uncompressed 8 bits per pixel images only. Return true on success.
		bool GenerateMipImage(Image& dest) const;
		/// Return the data for a mip level. Images loaded from eg. PNG or JPG formats will only have one (index 0) level.
		ImageLevel GetLevel(uint32_t index) const;
		/// Decompress a mip level as 8-bit RGBA. Supports compressed images only. Return true on success.
		bool DecompressLevel(uint8_t* dest, uint32_t levelIndex) const;

		/// Calculate the data size of an image level.
		static uint32_t CalculateDataSize(const Size& size, PixelFormat format, uint32_t* numRows = 0, uint32_t* rowSize = 0);

	private:
		/// Decode image pixel data using the stb_image library.
		static uint8_t* DecodePixelData(Stream& source, int& width, int& height, unsigned& components);
		/// Free the decoded pixel data.
		static void FreePixelData(uint8_t* pixelData);

		/// Image dimensions.
		Size _size{ Size::Empty };
		/// Image format.
		PixelFormat _format{ PixelFormat::Undefined };
		/// Number of mip levels. 1 for uncompressed images.
		uint32_t _mipLevels{ 1 };

		/// Image pixel data.
		std::unique_ptr<uint8_t[]> _data;
	};
}