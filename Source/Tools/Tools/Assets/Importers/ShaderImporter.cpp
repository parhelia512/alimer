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

#include "ShaderImporter.hpp"
#include "IO/VectorBuffer.h"
#include "Graphics/GraphicsDefs.h"
#include <sstream>
#include <fstream>

#ifdef _MSC_VER
#	pragma warning(push)
#	pragma warning(disable : 4467)
#	include <wrl.h>
#	pragma warning(pop)

#	include <d3dcompiler.h>
#endif

#ifdef _MSC_VER
namespace Alimer
{
	struct D3DCompiler
	{
		const wchar_t* fileName;
		const GUID  IID_ID3D11ShaderReflection;
	};

	static const D3DCompiler s_d3dcompiler[] =
	{
		// BK - the only different method in interface is GetRequiresFlags at the end
		//      of IID_ID3D11ShaderReflection47 (which is not used anyway).
		{ L"D3DCompiler_47.dll",{ 0x8d536ca1, 0x0cca, 0x4956,{ 0xa8, 0x37, 0x78, 0x69, 0x63, 0x75, 0x55, 0x84 } } },
		{ L"D3DCompiler_46.dll",{ 0x0a233719, 0x3960, 0x4578,{ 0x9d, 0x7c, 0x20, 0x3b, 0x8b, 0x1d, 0x9c, 0xc1 } } },
		{ L"D3DCompiler_45.dll",{ 0x0a233719, 0x3960, 0x4578,{ 0x9d, 0x7c, 0x20, 0x3b, 0x8b, 0x1d, 0x9c, 0xc1 } } },
		{ L"D3DCompiler_44.dll",{ 0x0a233719, 0x3960, 0x4578,{ 0x9d, 0x7c, 0x20, 0x3b, 0x8b, 0x1d, 0x9c, 0xc1 } } },
		{ L"D3DCompiler_43.dll",{ 0x0a233719, 0x3960, 0x4578,{ 0x9d, 0x7c, 0x20, 0x3b, 0x8b, 0x1d, 0x9c, 0xc1 } } },
	};

	typedef HRESULT(WINAPI* PFN_D3D_COMPILE)(_In_reads_bytes_(SrcDataSize) LPCVOID pSrcData
		, _In_ SIZE_T SrcDataSize
		, _In_opt_ LPCSTR pSourceName
		, _In_reads_opt_(_Inexpressible_(pDefines->Name != NULL)) CONST D3D_SHADER_MACRO* pDefines
		, _In_opt_ ID3DInclude* pInclude
		, _In_opt_ LPCSTR pEntrypoint
		, _In_ LPCSTR pTarget
		, _In_ UINT Flags1
		, _In_ UINT Flags2
		, _Out_ ID3DBlob** ppCode
		, _Always_(_Outptr_opt_result_maybenull_) ID3DBlob** ppErrorMsgs
		);

	typedef HRESULT(WINAPI* PFN_D3D_DISASSEMBLE)(_In_reads_bytes_(SrcDataSize) LPCVOID pSrcData
		, _In_ SIZE_T SrcDataSize
		, _In_ UINT Flags
		, _In_opt_ LPCSTR szComments
		, _Out_ ID3DBlob** ppDisassembly
		);

	typedef HRESULT(WINAPI* PFN_D3D_REFLECT)(_In_reads_bytes_(SrcDataSize) LPCVOID pSrcData
		, _In_ SIZE_T SrcDataSize
		, _In_ REFIID pInterface
		, _Out_ void** ppReflector
		);

	typedef HRESULT(WINAPI* PFN_D3D_STRIP_SHADER)(_In_reads_bytes_(BytecodeLength) LPCVOID pShaderBytecode
		, _In_ SIZE_T BytecodeLength
		, _In_ UINT uStripFlags
		, _Out_ ID3DBlob** ppStrippedBlob
		);

	PFN_D3D_COMPILE      D3DCompile;
	PFN_D3D_DISASSEMBLE  D3DDisassemble;
	PFN_D3D_REFLECT      D3DReflect;
	PFN_D3D_STRIP_SHADER D3DStripShader;

	static const D3DCompiler* s_compiler;
	static HMODULE s_d3dCompilerModule = nullptr;

	struct CompilerInitializer
	{
		CompilerInitializer()
		{
			for (uint32_t ii = 0; ii < ARRAYSIZE(s_d3dcompiler); ++ii)
			{
				const D3DCompiler* compiler = &s_d3dcompiler[ii];
				s_d3dCompilerModule = ::LoadLibraryW(compiler->fileName);
				if (s_d3dCompilerModule == nullptr)
				{
					continue;
				}

				D3DCompile = (PFN_D3D_COMPILE)::GetProcAddress(s_d3dCompilerModule, "D3DCompile");
				D3DDisassemble = (PFN_D3D_DISASSEMBLE)::GetProcAddress(s_d3dCompilerModule, "D3DDisassemble");
				D3DReflect = (PFN_D3D_REFLECT)::GetProcAddress(s_d3dCompilerModule, "D3DReflect");
				D3DStripShader = (PFN_D3D_STRIP_SHADER)::GetProcAddress(s_d3dCompilerModule, "D3DStripShader");

				if (D3DCompile == nullptr
					|| D3DDisassemble == nullptr
					|| D3DReflect == nullptr
					|| D3DStripShader == nullptr)
				{
					::FreeLibrary(s_d3dCompilerModule);
					continue;
				}

				s_compiler = compiler;
				return;
			}

			OutputDebugStringW(L"Unable to find D3DCompiler module.");
		}

		~CompilerInitializer()
		{
			if (s_d3dCompilerModule) {
				::FreeLibrary(s_d3dCompilerModule);
				s_d3dCompilerModule = nullptr;
			}
		}
	};

	CompilerInitializer __compilerInitializer__;

	static std::vector<uint8_t> CompileHLSL(
		const std::vector<uint8_t>& data,
		ShaderStage stage)
	{
		std::string entryPoint;
		std::string target;

		switch (stage)
		{
		case ShaderStage::Vertex:
			entryPoint = "VSMain";
			target = "vs_4_0";
			break;
		case ShaderStage::Fragment:
			entryPoint = "PSMain";
			target = "ps_4_0";
			break;
		default:
			break;
		}

		std::vector<D3D_SHADER_MACRO> macros;
		D3D_SHADER_MACRO endMacro = { nullptr, nullptr };
		macros.push_back(endMacro);

		UINT flags = 0;

		Microsoft::WRL::ComPtr<ID3DBlob> codeBlob;
		Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
		if (FAILED(D3DCompile(
			data.data(),
			static_cast<SIZE_T>(data.size()),
			"",
			macros.data(),
			0,
			entryPoint.c_str(),
			target.c_str(),
			flags,
			0,
			codeBlob.ReleaseAndGetAddressOf(),
			errorBlob.ReleaseAndGetAddressOf())))
		{
			if (errorBlob)
			{
				//ALIMER_LOGERROR("Could not compile shader {}: {}", GetFullName(), errorBlob->GetBufferPointer());
			}

		}

		auto result = std::vector<uint8_t>(codeBlob->GetBufferSize());
		memcpy_s(result.data(), result.size(), codeBlob->GetBufferPointer(), codeBlob->GetBufferSize());
		return result;
	}
}
#endif

namespace Alimer
{
	ShaderImporter::ShaderImporter()
	{
		//glslang::InitializeProcess();
	}

	ShaderImporter::~ShaderImporter()
	{
		//glslang::FinalizeProcess();
	}

	bool ShaderImporter::CanImport(const std::string& fileExt) const
	{
		return fileExt.compare(".hlsl") == 0;
	}

	void ShaderImporter::Import(const Asset& asset, IAssetImporterContext& context)
	{
		std::map<ShaderStage, std::vector<uint8_t>> shaders;
		for (auto& input : asset.inputFiles)
		{
			std::string shaderSource(input.data.begin(), input.data.end());

#ifdef _MSC_VER
			if (shaderSource.find("VSMain") != std::string::npos)
			{
				shaders[ShaderStage::Vertex] = CompileHLSL(input.data, ShaderStage::Vertex);
			}

			if (shaderSource.find("PSMain") != std::string::npos)
			{
				shaders[ShaderStage::Fragment] = CompileHLSL(input.data, ShaderStage::Fragment);
			}
#endif
		}

		VectorBuffer output;
		output.WriteFileID("ASHD");
		output.WriteUInt(static_cast<uint32_t>(shaders.size()));
		for (auto it : shaders)
		{
			output.WriteUByte(static_cast<uint8_t>(it.first));
			output.WriteBuffer(it.second);
		}
		context.Output(asset.assetId, output.Buffer());
	}
}