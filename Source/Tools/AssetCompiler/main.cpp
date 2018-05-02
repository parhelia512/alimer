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

#include <iostream>
#include <fstream>
#include "IO/FileSystem.h"
#include "Assets/Importers/ShaderImporter.hpp"
using namespace Alimer;

class TestAssetImporterContext final : public Alimer::IAssetImporterContext
{
public:
	TestAssetImporterContext(const std::string& outputDirectory);

	void Output(const std::string& name, const std::vector<uint8_t>& data) override;
	const std::string& GetOutputDirectory() override;

private:
	std::string _outputDirectory;
};

TestAssetImporterContext::TestAssetImporterContext(const std::string& outputDirectory)
	: _outputDirectory(outputDirectory)
{

}

void TestAssetImporterContext::Output(const std::string& name, const std::vector<uint8_t>& data)
{
	auto currentPath = Alimer::GetCurrentDir();
	Alimer::SetCurrentDir(_outputDirectory);
	std::ofstream fp(name, std::ios::binary | std::ios::out);
	fp.write(reinterpret_cast<const char*>(data.data()), data.size());
	fp.close();
	Alimer::SetCurrentDir(currentPath);
}

const std::string& TestAssetImporterContext::GetOutputDirectory()
{
	return _outputDirectory;
}

std::vector<uint8_t> ReadFile(const std::string& path)
{
	std::vector<uint8_t> result;

	std::ifstream fp(path, std::ios::binary | std::ios::in);
	if (!fp.is_open()) {
		return result;
	}

	fp.seekg(0, std::ios::end);
	size_t size = fp.tellg();
	fp.seekg(0, std::ios::beg);
	result.resize(size);

	fp.read(reinterpret_cast<char*>(result.data()), size);
	fp.close();

	return result;
}

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cout << "Usage: AlimerAssetCompiler [file/path] [outPath]" << std::endl;
		return 1;
	}

	std::string inputFile = argv[1];
	std::string outputPath = argv[2];
	TestAssetImporterContext context(outputPath);
	Asset asset;
	asset.assetId = ReplaceExtension(GetFileName(inputFile), ".bin");
	asset.inputFiles.emplace_back(AssetFile(inputFile, ReadFile(inputFile)));
	ShaderImporter importer;
	importer.Import(asset, context);

	std::cout << "Usage: AlimerAssetCompiler" << " " << argv[1];
	return EXIT_SUCCESS;
}
