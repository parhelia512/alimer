// For conditions of distribution and use, see copyright notice in License.txt

#pragma once

#include "Image.h"

namespace Alimer
{

/// Decompress DXT1/3/5 image data.
ALIMER_API void DecompressImageDXT(unsigned char* dest, const void* blocks, int width, int height, ImageFormat format);
/// Decompress ETC image data.
ALIMER_API void DecompressImageETC(unsigned char* dest, const void* blocks, int width, int height);
/// Decompress PVRTC image data.
ALIMER_API void DecompressImagePVRTC(unsigned char* dest, const void* blocks, int width, int height, ImageFormat format);

}
