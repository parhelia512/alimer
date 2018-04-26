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

#include "Base/Utils.h"
#include "../Debug/Log.h"
#include "../Debug/Profiler.h"
#include "../IO/Stream.h"
#include "../Math/Math.h"
#include "Decompress.h"

#include <cstdlib>
#include <cstring>

#define STB_IMAGE_IMPLEMENTATION
#include <STB/stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <STB/stb_image_write.h>

#ifndef MAKEFOURCC
#	define MAKEFOURCC(ch0, ch1, ch2, ch3) ((unsigned)(ch0) | ((unsigned)(ch1) << 8) | ((unsigned)(ch2) << 16) | ((unsigned)(ch3) << 24))
#endif

#define FOURCC_DXT1 (MAKEFOURCC('D','X','T','1'))
#define FOURCC_DXT2 (MAKEFOURCC('D','X','T','2'))
#define FOURCC_DXT3 (MAKEFOURCC('D','X','T','3'))
#define FOURCC_DXT4 (MAKEFOURCC('D','X','T','4'))
#define FOURCC_DXT5 (MAKEFOURCC('D','X','T','5'))

namespace Alimer
{

	const int Image::_components[] =
	{
		0,      // Undefined
		1,      // FMT_R8
		2,      // FMT_RG8
		4,      // FMT_RGBA8
		1,      // FMT_A8
		0,      // FMT_R16
		0,      // FMT_RG16
		0,      // FMT_RGBA16
		0,      // FMT_R16F
		0,      // FMT_RG16F
		0,      // FMT_RGBA16F
		0,      // FMT_R32F
		0,      // FMT_RG32F
		0,      // FMT_RGB32F
		0,      // FMT_RGBA32F
		0,      // FMT_D16
		0,      // FMT_D32
		0,      // FMT_D24S8
		0,      // FMT_DXT1
		0,      // FMT_DXT3
		0,      // FMT_DXT5
		0,      // FMT_ETC1
		0,      // FMT_PVRTC_RGB_2BPP
		0,      // FMT_PVRTC_RGBA_2BPP
		0,      // FMT_PVRTC_RGB_4BPP
		0       // FMT_PVRTC_RGBA_4BPP
	};

	const uint32_t Image::_pixelByteSizes[] =
	{
		0,      // Undefined
		1,      // FMT_R8
		2,      // FMT_RG8
		4,      // FMT_RGBA8
		1,      // FMT_A8
		2,      // FMT_R16
		4,      // FMT_RG16
		8,      // FMT_RGBA16
		2,      // FMT_R16F
		4,      // FMT_RG16F
		8,      // FMT_RGBA16F
		4,      // FMT_R32F
		8,      // FMT_RG32F
		12,     // FMT_RGB32F
		16,     // FMT_RGBA32F
		2,      // FMT_D16
		4,      // FMT_D32
		4,      // FMT_D24S8
		0,      // FMT_DXT1
		0,      // FMT_DXT3
		0,      // FMT_DXT5
		0,      // FMT_ETC1
		0,      // FMT_PVRTC_RGB_2BPP
		0,      // FMT_PVRTC_RGBA_2BPP
		0,      // FMT_PVRTC_RGB_4BPP
		0       // FMT_PVRTC_RGBA_4BPP
	};

	static const PixelFormat componentsToFormat[] =
	{
		PixelFormat::Undefined,
		PixelFormat::R8UNorm,
		PixelFormat::RG8UNorm,
		PixelFormat::RGBA8UNorm,
		PixelFormat::RGBA8UNorm
	};

	/// \cond PRIVATE
	struct DDColorKey
	{
		unsigned dwColorSpaceLowValue;
		unsigned dwColorSpaceHighValue;
	};
	/// \endcond

	/// \cond PRIVATE
	struct DDPixelFormat
	{
		unsigned dwSize;
		unsigned dwFlags;
		unsigned dwFourCC;
		union
		{
			unsigned dwRGBBitCount;
			unsigned dwYUVBitCount;
			unsigned dwZBufferBitDepth;
			unsigned dwAlphaBitDepth;
			unsigned dwLuminanceBitCount;
			unsigned dwBumpBitCount;
			unsigned dwPrivateFormatBitCount;
		};
		union
		{
			unsigned dwRBitMask;
			unsigned dwYBitMask;
			unsigned dwStencilBitDepth;
			unsigned dwLuminanceBitMask;
			unsigned dwBumpDuBitMask;
			unsigned dwOperations;
		};
		union
		{
			unsigned dwGBitMask;
			unsigned dwUBitMask;
			unsigned dwZBitMask;
			unsigned dwBumpDvBitMask;
			struct
			{
				unsigned short wFlipMSTypes;
				unsigned short wBltMSTypes;
			} multiSampleCaps;
		};
		union
		{
			unsigned dwBBitMask;
			unsigned dwVBitMask;
			unsigned dwStencilBitMask;
			unsigned dwBumpLuminanceBitMask;
		};
		union
		{
			unsigned dwRGBAlphaBitMask;
			unsigned dwYUVAlphaBitMask;
			unsigned dwLuminanceAlphaBitMask;
			unsigned dwRGBZBitMask;
			unsigned dwYUVZBitMask;
		};
	};
	/// \endcond

	/// \cond PRIVATE
	struct DDSCaps2
	{
		unsigned dwCaps;
		unsigned dwCaps2;
		unsigned dwCaps3;
		union
		{
			unsigned dwCaps4;
			unsigned dwVolumeDepth;
		};
	};
	/// \endcond

	/// \cond PRIVATE
	struct DDSurfaceDesc2
	{
		unsigned dwSize;
		unsigned dwFlags;
		unsigned dwHeight;
		unsigned dwWidth;
		union
		{
			unsigned lPitch;
			unsigned dwLinearSize;
		};
		union
		{
			unsigned dwBackBufferCount;
			unsigned dwDepth;
		};
		union
		{
			unsigned dwMipMapCount;
			unsigned dwRefreshRate;
			unsigned dwSrcVBHandle;
		};
		unsigned dwAlphaBitDepth;
		unsigned dwReserved;
		unsigned lpSurface; // Do not define as a void pointer, as it is 8 bytes in a 64bit build
		union
		{
			DDColorKey ddckCKDestOverlay;
			unsigned dwEmptyFaceColor;
		};
		DDColorKey ddckCKDestBlt;
		DDColorKey ddckCKSrcOverlay;
		DDColorKey ddckCKSrcBlt;
		union
		{
			DDPixelFormat ddpfPixelFormat;
			unsigned dwFVF;
		};
		DDSCaps2 ddsCaps;
		unsigned dwTextureStage;
	};
	/// \endcond

	void Image::RegisterObject()
	{
		RegisterFactory<Image>();
	}

	bool Image::BeginLoad(Stream& source)
	{
		ALIMER_PROFILE(LoadImage);

		// Check for DDS, KTX or PVR compressed format
		std::string fileID = source.ReadFileID();

		if (fileID == "DDS ")
		{
			// DDS compressed format
			DDSurfaceDesc2 ddsd;
			source.Read(&ddsd, sizeof(ddsd));

			switch (ddsd.ddpfPixelFormat.dwFourCC)
			{
			case FOURCC_DXT1:
				_format = PixelFormat::BC1;
				break;

			case FOURCC_DXT3:
				_format = PixelFormat::BC2;
				break;

			case FOURCC_DXT5:
				_format = PixelFormat::BC3;
				break;

			default:
				ALIMER_LOGERROR("Unsupported DDS format");
				return false;
			}

			size_t dataSize = source.Size() - source.Position();
			_data.reset(new uint8_t[dataSize]);
			_size = Size(ddsd.dwWidth, ddsd.dwHeight);
			_mipLevels = ddsd.dwMipMapCount ? ddsd.dwMipMapCount : 1;
			source.Read(_data.get(), dataSize);
		}
		else if (fileID == "\253KTX")
		{
			source.Seek(12);

			uint32_t endianness = source.ReadUInt();
			uint32_t type = source.ReadUInt();
			/* unsigned typeSize = */ source.ReadUInt();
			uint32_t imageFormat = source.ReadUInt();
			uint32_t internalFormat = source.ReadUInt();
			/* uint32_t baseInternalFormat = */ source.ReadUInt();
			uint32_t imageWidth = source.ReadUInt();
			uint32_t imageHeight = source.ReadUInt();
			uint32_t depth = source.ReadUInt();
			/* uint32_t arrayElements = */ source.ReadUInt();
			uint32_t faces = source.ReadUInt();
			uint32_t mipmaps = source.ReadUInt();
			uint32_t keyValueBytes = source.ReadUInt();

			if (endianness != 0x04030201)
			{
				ALIMER_LOGERROR("Big-endian KTX files not supported");
				return false;
			}

			if (type != 0 || imageFormat != 0)
			{
				ALIMER_LOGERROR("Uncompressed KTX files not supported");
				return false;
			}

			if (faces > 1 || depth > 1)
			{
				ALIMER_LOGERROR("3D or cube KTX files not supported");
				return false;
			}

			if (mipmaps == 0)
			{
				ALIMER_LOGERROR("KTX files without explicitly specified mipmap count not supported");
				return false;
			}

			_format = PixelFormat::Undefined;
			switch (internalFormat)
			{
			case 0x83f1:
				_format = PixelFormat::BC1;
				break;

			case 0x83f2:
				_format = PixelFormat::BC2;
				break;

			case 0x83f3:
				_format = PixelFormat::BC3;
				break;

			case 0x8d64:
				_format = PixelFormat::ETC1;
				break;

			case 0x8c00:
				_format = PixelFormat::PVRTC_RGB_4BPP;
				break;

			case 0x8c01:
				_format = PixelFormat::PVRTC_RGB_2BPP;
				break;

			case 0x8c02:
				_format = PixelFormat::PVRTC_RGBA_4BPP;
				break;

			case 0x8c03:
				_format = PixelFormat::PVRTC_RGBA_2BPP;
				break;
			}

			if (_format == PixelFormat::Undefined)
			{
				ALIMER_LOGERROR("Unsupported texture format in KTX file");
				return false;
			}

			source.Seek(source.Position() + keyValueBytes);
			size_t dataSize = source.Size() - source.Position() - mipmaps * sizeof(unsigned);

			_data.reset(new uint8_t[dataSize]);
			_size = Size(imageWidth, imageHeight);
			_mipLevels = mipmaps;

			size_t dataOffset = 0;
			for (size_t i = 0; i < mipmaps; ++i)
			{
				size_t levelSize = source.Read<unsigned>();
				if (levelSize + dataOffset > dataSize)
				{
					ALIMER_LOGERROR("KTX mipmap level data size exceeds file size");
					return false;
				}

				source.Read(&_data[dataOffset], levelSize);
				dataOffset += levelSize;
				if (source.Position() & 3)
					source.Seek((source.Position() + 3) & 0xfffffffc);
			}
		}
		else if (fileID == "PVR\3")
		{
			/* uint32_t flags = */ source.ReadUInt();
			uint32_t pixelFormatLo = source.ReadUInt();
			/* uint32_t pixelFormatHi = */ source.ReadUInt();
			/* uint32_t colourSpace = */ source.ReadUInt();
			/* uint32_t channelType = */ source.ReadUInt();
			uint32_t imageHeight = source.ReadUInt();
			uint32_t imageWidth = source.ReadUInt();
			uint32_t depth = source.ReadUInt();
			/* uint32_t numSurfaces = */ source.ReadUInt();
			uint32_t numFaces = source.ReadUInt();
			uint32_t mipmapCount = source.ReadUInt();
			uint32_t metaDataSize = source.ReadUInt();

			if (depth > 1 || numFaces > 1)
			{
				ALIMER_LOGERROR("3D or cube PVR files not supported");
				return false;
			}

			if (mipmapCount == 0)
			{
				ALIMER_LOGERROR("PVR files without explicitly specified mipmap count not supported");
				return false;
			}

			_format = PixelFormat::Undefined;
			switch (pixelFormatLo)
			{
			case 0:
				_format = PixelFormat::PVRTC_RGB_2BPP;
				break;

			case 1:
				_format = PixelFormat::PVRTC_RGBA_2BPP;
				break;

			case 2:
				_format = PixelFormat::PVRTC_RGB_4BPP;
				break;

			case 3:
				_format = PixelFormat::PVRTC_RGBA_4BPP;
				break;

			case 6:
				_format = PixelFormat::ETC1;
				break;

			case 7:
				_format = PixelFormat::BC1;
				break;

			case 9:
				_format = PixelFormat::BC2;
				break;

			case 11:
				_format = PixelFormat::BC3;
				break;
			}

			if (_format == PixelFormat::Undefined)
			{
				ALIMER_LOGERROR("Unsupported texture format in PVR file");
				return false;
			}

			source.Seek(source.Position() + metaDataSize);
			size_t dataSize = source.Size() - source.Position();

			_data.reset(new uint8_t[dataSize]);
			_size = Size(imageWidth, imageHeight);
			_mipLevels = mipmapCount;

			source.Read(_data.get(), dataSize);
		}
		else
		{
			// Not DDS, KTX or PVR, use STBImage to load other image formats as uncompressed
			source.Seek(0);
			int imageWidth, imageHeight;
			uint32_t imageComponents;
			uint8_t* pixelData = DecodePixelData(source, imageWidth, imageHeight, imageComponents);
			if (!pixelData)
			{
				ALIMER_LOGERROR("Could not load image " + source.GetName() + ": " + std::string(stbi_failure_reason()));
				return false;
			}

			SetSize(Size(imageWidth, imageHeight), componentsToFormat[imageComponents]);

			if (imageComponents != 3)
			{
				SetData(pixelData);
			}
			else
			{
				// Convert RGB to RGBA as for example Direct3D 11 does not support 24-bit formats
				std::unique_ptr<uint8_t[]> rgbaData(new uint8_t[4 * imageWidth * imageHeight]);
				unsigned char* src = pixelData;
				unsigned char* dest = rgbaData.get();
				for (int i = 0; i < imageWidth * imageHeight; ++i)
				{
					*dest++ = *src++;
					*dest++ = *src++;
					*dest++ = *src++;
					*dest++ = 0xff;
				}

				SetData(rgbaData.get());
			}

			FreePixelData(pixelData);
		}

		return true;
	}

	void StbiWriteCallback(void *context, void *data, int len)
	{
		auto stream = reinterpret_cast<Stream*>(context);
		stream->Write(data, len);
	}

	bool Image::Save(Stream& dest)
	{
		ALIMER_PROFILE(SaveImage);

		if (IsCompressed())
		{
			ALIMER_LOGERROR("Can not save compressed image " + GetName());
			return false;
		}

		if (!_data)
		{
			ALIMER_LOGERROR("Can not save zero-sized image " + GetName());
			return false;
		}

		int components = (int)GetPixelByteSize();
		if (components < 1 || components > 4)
		{
			ALIMER_LOGERROR("Unsupported pixel format for PNG save on image " + GetName());
			return false;
		}

		bool success = stbi_write_png_to_func(
			StbiWriteCallback,
			&dest,
			static_cast<int>(_size.width),
			static_cast<int>(_size.height),
			components,
			_data.get(),
			0) != 0;
		return success;
	}

	void Image::SetSize(const Size& newSize, PixelFormat newFormat)
	{
		if (newSize == _size && newFormat == _format)
			return;

		if (newSize.width <= 0 || newSize.height <= 0)
		{
			ALIMER_LOGERROR("Can not set zero or negative image size");
			return;
		}

		if (_pixelByteSizes[static_cast<uint32_t>(newFormat)] == 0)
		{
			ALIMER_LOGERROR("Can not set image size with unspecified pixel byte size (including compressed formats)");
			return;
		}

		_data.reset(new uint8_t[newSize.width * newSize.height * _pixelByteSizes[static_cast<uint32_t>(newFormat)]]);
		_size = newSize;
		_format = newFormat;
		_mipLevels = 1;
	}

	void Image::SetData(const uint8_t* pixelData)
	{
		if (!IsCompressed())
		{
			memcpy(_data.get(), pixelData, _size.width * _size.height * GetPixelByteSize());
		}
		else
			ALIMER_LOGERROR("Can not set pixel data of a compressed image");
	}

	uint8_t* Image::DecodePixelData(Stream& source, int& width, int& height, unsigned& components)
	{
		size_t dataSize = source.Size();

		std::unique_ptr<uint8_t[]> buffer(new uint8_t[dataSize]);
		source.Read(buffer.get(), dataSize);
		return stbi_load_from_memory(buffer.get(), (int)dataSize, &width, &height, (int *)&components, 0);
	}

	void Image::FreePixelData(uint8_t* pixelData)
	{
		if (!pixelData)
			return;

		stbi_image_free(pixelData);
	}

	bool Image::GenerateMipImage(Image& dest) const
	{
		ALIMER_PROFILE(GenerateMipImage);

		int components = Components();
		if (components < 1 || components > 4)
		{
			ALIMER_LOGERROR("Unsupported format for calculating the next mip level");
			return false;
		}

		Size sizeOut(Max(_size.width / 2, 1), Max(_size.height / 2, 1));
		dest.SetSize(sizeOut, _format);

		const uint8_t* pixelDataIn = _data.get();
		uint8_t* pixelDataOut = dest._data.get();

		switch (components)
		{
		case 1:
			for (uint32_t y = 0; y < sizeOut.height; ++y)
			{
				const uint8_t* inUpper = &pixelDataIn[(y * 2) * _size.width];
				const uint8_t* inLower = &pixelDataIn[(y * 2 + 1) * _size.width];
				uint8_t* out = &pixelDataOut[y * sizeOut.width];

				for (uint32_t x = 0; x < sizeOut.width; ++x)
				{
					out[x] = ((uint32_t)inUpper[x * 2] + inUpper[x * 2 + 1] + inLower[x * 2] + inLower[x * 2 + 1]) >> 2;
				}
			}
			break;

		case 2:
			for (uint32_t y = 0; y < sizeOut.height; ++y)
			{
				const uint8_t* inUpper = &pixelDataIn[(y * 2) * _size.width * 2];
				const uint8_t* inLower = &pixelDataIn[(y * 2 + 1) * _size.width * 2];
				uint8_t* out = &pixelDataOut[y * sizeOut.width * 2];

				for (uint32_t x = 0; x < sizeOut.width * 2; x += 2)
				{
					out[x] = ((uint32_t)inUpper[x * 2] + inUpper[x * 2 + 2] + inLower[x * 2] + inLower[x * 2 + 2]) >> 2;
					out[x + 1] = ((uint32_t)inUpper[x * 2 + 1] + inUpper[x * 2 + 3] + inLower[x * 2 + 1] + inLower[x * 2 + 3]) >> 2;
				}
			}
			break;

		case 4:
			for (uint32_t y = 0; y < sizeOut.height; ++y)
			{
				const uint8_t* inUpper = &pixelDataIn[(y * 2) * _size.width * 4];
				const uint8_t* inLower = &pixelDataIn[(y * 2 + 1) * _size.width * 4];
				uint8_t* out = &pixelDataOut[y * sizeOut.width * 4];

				for (uint32_t x = 0; x < sizeOut.width * 4; x += 4)
				{
					out[x] = ((uint32_t)inUpper[x * 2] + inUpper[x * 2 + 4] + inLower[x * 2] + inLower[x * 2 + 4]) >> 2;
					out[x + 1] = ((uint32_t)inUpper[x * 2 + 1] + inUpper[x * 2 + 5] + inLower[x * 2 + 1] + inLower[x * 2 + 5]) >> 2;
					out[x + 2] = ((uint32_t)inUpper[x * 2 + 2] + inUpper[x * 2 + 6] + inLower[x * 2 + 2] + inLower[x * 2 + 6]) >> 2;
					out[x + 3] = ((uint32_t)inUpper[x * 2 + 3] + inUpper[x * 2 + 7] + inLower[x * 2 + 3] + inLower[x * 2 + 7]) >> 2;
				}
			}
			break;
		}

		return true;
	}

	ImageLevel Image::GetLevel(uint32_t index) const
	{
		ImageLevel level;

		if (index >= _mipLevels)
			return level;

		size_t i = 0;
		size_t offset = 0;

		for (;;)
		{
			level.size = Size(Max(_size.width >> i, 1), Max(_size.height >> i, 1));
			level.data = _data.get() + offset;

			size_t dataSize = CalculateDataSize(level.size, _format, &level.rows, &level.rowSize);
			if (i == index)
				return level;

			offset += dataSize;
			++i;
		}
	}

	bool Image::DecompressLevel(uint8_t* dest, uint32_t index) const
	{
		ALIMER_PROFILE(DecompressImageLevel);

		if (!dest)
		{
			ALIMER_LOGERROR("Null destination data for DecompressLevel");
			return false;
		}

		if (index >= _mipLevels)
		{
			ALIMER_LOGERROR("Mip level index out of bounds for DecompressLevel");
			return false;
		}

		ImageLevel level = GetLevel(index);

		switch (_format)
		{
		case PixelFormat::BC1:
		case PixelFormat::BC2:
		case PixelFormat::BC3:
			DecompressImageDXT(dest, level.data, level.size.width, level.size.height, _format);
			break;

		case PixelFormat::ETC1:
			DecompressImageETC(dest, level.data, level.size.width, level.size.height);
			break;

		case PixelFormat::PVRTC_RGB_2BPP:
		case PixelFormat::PVRTC_RGBA_2BPP:
		case PixelFormat::PVRTC_RGB_4BPP:
		case PixelFormat::PVRTC_RGBA_4BPP:
			DecompressImagePVRTC(dest, level.data, level.size.width, level.size.height, _format);
			break;

		default:
			ALIMER_LOGERROR("Unsupported format for DecompressLevel");
			return false;
		}

		return true;
	}

	uint32_t Image::CalculateDataSize(const Size& size, PixelFormat format, uint32_t* dstRows, uint32_t* dstRowSize)
	{
		uint32_t rows, rowSize, dataSize;

		if (format < PixelFormat::BC1)
		{
			rows = size.height;
			rowSize = size.width * _pixelByteSizes[ecast(format)];
			dataSize = rows * rowSize;
		}
		else if (format < PixelFormat::PVRTC_RGB_2BPP)
		{
			uint32_t blockSize = (format == PixelFormat::BC1 || format == PixelFormat::ETC1) ? 8 : 16;
			rows = (size.height + 3) / 4;
			rowSize = ((size.width + 3) / 4) * blockSize;
			dataSize = rows * rowSize;
		}
		else
		{
			uint32_t blockSize = format < PixelFormat::PVRTC_RGB_4BPP ? 2 : 4;
			uint32_t dataWidth = Max(size.width, blockSize == 2 ? 16 : 8);
			rows = Max(size.height, 8);
			dataSize = (dataWidth * rows * blockSize + 7) >> 3;
			rowSize = dataSize / rows;
		}

		if (dstRows)
			*dstRows = rows;
		if (dstRowSize)
			*dstRowSize = rowSize;
		return dataSize;
	}

	bool IsDepthFormat(PixelFormat format)
	{
		switch (format)
		{
		case PixelFormat::Depth16UNorm:
			//case PixelFormat::Depth16UNormStencil8:
		case PixelFormat::Depth24UNormStencil8:
		case PixelFormat::Depth32Float:
			//case PixelFormat::Depth32FloatStencil8:
			return true;
		default:
			return false;
		}
	}

	bool IsStencilFormat(PixelFormat format)
	{
		switch (format)
		{
		case PixelFormat::Stencil8:
			return true;
		default:
			return false;
		}
	}

	bool IsDepthStencilFormat(PixelFormat format)
	{
		return IsDepthFormat(format) || IsStencilFormat(format);
	}

	bool IsCompressed(PixelFormat format)
	{
		switch (format)
		{
		case PixelFormat::BC1:
			//case PixelFormat::BC1_SRGB:
		case PixelFormat::BC2:
			//case PixelFormat::BC2_SRGB:
		case PixelFormat::BC3:
			//case PixelFormat::BC3_SRGB:
			return true;
		default:
			return false;
		}
	}

	const char* EnumToString(PixelFormat format)
	{
#define CASE_STRING(ENUM_VALUE) case Alimer::PixelFormat::##ENUM_VALUE : return #ENUM_VALUE;
		switch (format)
		{
			CASE_STRING(Undefined);
			CASE_STRING(A8UNorm);
			CASE_STRING(R8UNorm);
			CASE_STRING(RG8UNorm);
			CASE_STRING(RGBA8UNorm);
			CASE_STRING(R16UNorm);
			CASE_STRING(RG16UNorm);
			CASE_STRING(RGBA16UNorm);
			CASE_STRING(R16Float);
			CASE_STRING(RG16Float);
			CASE_STRING(RGBA16Float);
			CASE_STRING(R32Float);
			CASE_STRING(RG32Float);
			CASE_STRING(RGBA32Float);
			CASE_STRING(Depth16UNorm);
			CASE_STRING(Depth32Float);
			CASE_STRING(Depth24UNormStencil8);
			CASE_STRING(Stencil8);
			CASE_STRING(BC1);
			CASE_STRING(BC2);
			CASE_STRING(BC3);
			CASE_STRING(ETC1);
			CASE_STRING(PVRTC_RGB_2BPP);
			CASE_STRING(PVRTC_RGBA_2BPP);
			CASE_STRING(PVRTC_RGB_4BPP);
			CASE_STRING(PVRTC_RGBA_4BPP);
		default:
			break;
		}

#undef CASE_STRING
		return "";
	}
}
