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

	/// Image formats.
	enum ImageFormat
	{
		FMT_NONE = 0,
		FMT_R8,
		FMT_RG8,
		FMT_RGBA8,
		FMT_A8,
		FMT_R16,
		FMT_RG16,
		FMT_RGBA16,
		FMT_R16F,
		FMT_RG16F,
		FMT_RGBA16F,
		FMT_R32F,
		FMT_RG32F,
		FMT_RGB32F,
		FMT_RGBA32F,
		FMT_D16,
		FMT_D32,
		FMT_D24S8,
		FMT_DXT1,
		FMT_DXT3,
		FMT_DXT5,
		FMT_ETC1,
		FMT_PVRTC_RGB_2BPP,
		FMT_PVRTC_RGBA_2BPP,
		FMT_PVRTC_RGB_4BPP,
		FMT_PVRTC_RGBA_4BPP
	};

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
		void SetSize(const Size& newSize, ImageFormat newFormat);
		/// Set new pixel data.
		void SetData(const uint8_t* pixelData);

		/// Return image dimensions in pixels.
		const Size& GetSize() const { return _size; }
		/// Return image width in pixels.
		uint32_t GetWidth() const { return _size.width; }
		/// Return image height in pixels.
		uint32_t GetHeight() const { return _size.height; }
		/// Return number of components in a pixel. Will return 0 for formats which are not 8 bits per pixel.
		int Components() const { return _components[format]; }
		/// Return byte size of a pixel. Will return 0 for block compressed formats.
		uint32_t GetPixelByteSize() const { return _pixelByteSizes[format]; }
		/// Return pixel data.
		uint8_t* Data() const { return _data.get(); }
		/// Return the image format.
		ImageFormat Format() const { return format; }
		/// Return whether is a compressed image.
		bool IsCompressed() const { return format >= FMT_DXT1; }
		/// Return number of mip levels contained in the image data.
		uint32_t GetMipLevels() const { return _mipLevels; }
		/// Calculate the next mip image with halved width and height. Supports uncompressed 8 bits per pixel images only. Return true on success.
		bool GenerateMipImage(Image& dest) const;
		/// Return the data for a mip level. Images loaded from eg. PNG or JPG formats will only have one (index 0) level.
		ImageLevel GetLevel(uint32_t index) const;
		/// Decompress a mip level as 8-bit RGBA. Supports compressed images only. Return true on success.
		bool DecompressLevel(uint8_t* dest, uint32_t levelIndex) const;

		/// Calculate the data size of an image level.
		static uint32_t CalculateDataSize(const Size& size, ImageFormat format, uint32_t* numRows = 0, uint32_t* rowSize = 0);

		/// Pixel components per format.
		static const int _components[];
		/// Pixel byte sizes per format.
		static const uint32_t _pixelByteSizes[];

	private:
		/// Decode image pixel data using the stb_image library.
		static uint8_t* DecodePixelData(Stream& source, int& width, int& height, unsigned& components);
		/// Free the decoded pixel data.
		static void FreePixelData(uint8_t* pixelData);

		/// Image dimensions.
		Size _size{ Size::Empty };
		/// Image format.
		ImageFormat format{ FMT_NONE };
		/// Number of mip levels. 1 for uncompressed images.
		uint32_t _mipLevels{ 1 };
		/// Image pixel data.
		std::unique_ptr<uint8_t[]> _data;
	};
}
