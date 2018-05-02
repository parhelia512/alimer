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
#include "../Resource/ResourceCache.h"
#include "../IO/File.h"
#include "../IO/FileSystem.h"
#include "Shader.h"
#include "ShaderVariation.h"
using namespace std;

namespace Alimer
{
	Shader::~Shader()
	{
	}

	void Shader::RegisterObject()
	{
		RegisterFactory<Shader>();
	}

	bool Shader::BeginLoad(Stream& source)
	{
		string fileID = source.ReadFileID();
		if (fileID != "ASHD")
		{
			std::string extension = GetExtension(source.GetName());
			_stage = (extension == ".vs" || extension == ".vert") ? ShaderStage::Vertex : ShaderStage::Fragment;
			_sourceCode.clear();
			return ProcessIncludes(_sourceCode, source);

			//ALIMER_LOGERROR(source.GetName() + " is not a valid shader file.");
			//return false;
		}

		const uint32_t shaderCount = source.ReadUInt();
		for (uint32_t i = 0; i < shaderCount; ++i)
		{
			ShaderStage stage = static_cast<ShaderStage>(source.ReadUByte());
			auto bytecode = source.ReadBuffer();
		}

		return true;
	}

	bool Shader::EndLoad()
	{
		// Release existing variations (if any) to allow them to be recompiled with changed code
		for (auto it = _variations.begin(); it != _variations.end(); ++it)
		{
			it->second->Release();
		}

		return true;
	}

	void Shader::Define(ShaderStage stage, const std::string& code)
	{
		_stage = stage;
		_sourceCode = code;
		EndLoad();
	}

	ShaderVariation* Shader::CreateVariation(const std::string& definesIn)
	{
		StringHash definesHash(definesIn);
		auto it = _variations.find(definesHash);
		if (it != _variations.end())
			return it->second.Get();

		// If initially not found, normalize the defines and try again
		std::string defines = NormalizeDefines(definesIn);
		definesHash = StringHash(defines);
		it = _variations.find(definesHash);
		if (it != _variations.end())
			return it->second.Get();

		ShaderVariation* newVariation = new ShaderVariation(this, defines);
		_variations[definesHash] = newVariation;
		return newVariation;
	}

	bool Shader::ProcessIncludes(std::string& code, Stream& source)
	{
		ResourceCache* cache = GetSubsystem<ResourceCache>();

		while (!source.IsEof())
		{
			string line = source.ReadLine();

			if (str::StartsWith(line, "#include"))
			{
				string trimString = str::Trim(str::Replace(line.substr(9), "\"", ""));
				string includeFileName = GetPath(source.GetName()) + trimString;
				UniquePtr<Stream> includeStream = cache->OpenResource(includeFileName);
				if (!includeStream)
					return false;

				// Add the include file into the current code recursively
				if (!ProcessIncludes(code, *includeStream))
					return false;
			}
			else
			{
				code += line;
				code += "\n";
			}
		}

		// Finally insert an empty line to mark the space between files
		code += "\n";

		return true;
	}

	std::string Shader::NormalizeDefines(const std::string& defines)
	{
		std::string ret;
		std::vector<std::string> definesVec = str::Split(str::ToUpper(defines), " ");
		std::sort(definesVec.begin(), definesVec.end());

		for (auto it = definesVec.begin(); it != definesVec.end(); ++it)
		{
			if (it != definesVec.begin())
				ret += " ";
			ret += *it;
		}

		return ret;
	}

}
