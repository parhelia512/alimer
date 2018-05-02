//
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

#include <string>
#include <vector>

namespace Alimer
{
	class AssetFile
	{
	public:
		std::string name;
		std::vector<uint8_t> data;

		AssetFile() {}
		AssetFile(const std::string& path, std::vector<uint8_t>&& data)
			: name(path)
			, data(data)
		{}
	};

	class Asset
	{
	public:
		std::string assetId;
		std::vector<AssetFile> inputFiles;
	};

	class IAssetImporterContext
	{
	public:
		virtual ~IAssetImporterContext() {}

		virtual void Output(const std::string& name, const std::vector<uint8_t>& data) = 0;
		virtual const std::string& GetOutputDirectory() = 0;
	};

	class IAssetImporter
	{
	protected:
		/// Constructor.
		IAssetImporter() = default;

	public:
		/// Destructor.
		virtual ~IAssetImporter() = default;

		virtual bool CanImport(const std::string& fileExt) const = 0;
		virtual void Import(const Asset& asset, IAssetImporterContext& context) = 0;
	};
}