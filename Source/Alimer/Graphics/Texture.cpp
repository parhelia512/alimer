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

#include "../Debug/Log.h"
#include "../Debug/Profiler.h"
#include "Texture.h"

namespace Alimer
{
	Texture::~Texture()
	{
		Release();
	}

	void Texture::RegisterObject()
	{
		RegisterFactory<Texture>();
	}

	bool Texture::BeginLoad(Stream& source)
	{
		_loadImages.clear();
		_loadImages.push_back(std::make_unique<Image>());
		if (!_loadImages[0]->Load(source))
		{
			_loadImages.clear();
			return false;
		}

		// If image uses unsupported format, decompress to RGBA now
		if (_loadImages[0]->Format() >= FMT_ETC1)
		{
			Image* rgbaImage = new Image();
			rgbaImage->SetSize(_loadImages[0]->GetSize(), FMT_RGBA8);
			_loadImages[0]->DecompressLevel(rgbaImage->Data(), 0);
			_loadImages[0].reset(rgbaImage); // This destroys the original compressed image
		}

		// Construct mip levels now if image is uncompressed
		if (!_loadImages[0]->IsCompressed())
		{
			auto* mipImage = _loadImages[0].get();

			while (mipImage->GetWidth() > 1
				|| mipImage->GetHeight() > 1)
			{
				_loadImages.push_back(std::make_unique<Image>());
				mipImage->GenerateMipImage(*_loadImages.back());
				mipImage = _loadImages.back().get();
			}
		}

		return true;
	}

	bool Texture::EndLoad()
	{
		if (_loadImages.empty())
			return false;

		std::vector<ImageLevel> initialData;

		for (size_t i = 0; i < _loadImages.size(); ++i)
		{
			for (uint32_t j = 0; j < _loadImages[i]->GetMipLevels(); ++j)
			{
				initialData.push_back(_loadImages[i]->GetLevel(j));
			}
		}

		Image* image = _loadImages[0].get();
		bool success = Define(
			TextureType::Type2D,
			image->GetSize(),
			image->Format(),
			static_cast<uint32_t>(initialData.size()),
			TextureUsage::ShaderRead,
			initialData.data());

		/// \todo Read a parameter file for the sampling parameters
		success &= DefineSampler(FILTER_TRILINEAR, ADDRESS_WRAP, ADDRESS_WRAP, ADDRESS_WRAP);

		_loadImages.clear();
		return success;
	}
}
