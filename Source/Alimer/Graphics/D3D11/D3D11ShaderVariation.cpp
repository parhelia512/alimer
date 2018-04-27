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

#include "../../Debug/Log.h"
#include "../../Debug/Profiler.h"
#include "../Shader.h"
#include "../Graphics.h"
#include "D3D11ShaderVariation.h"
#include "D3D11Graphics.h"
#include "../VertexBuffer.h"

using namespace Microsoft::WRL;

namespace Alimer
{
#if ALIMER_PLATFORM_WINDOWS && !ALIMER_PLATFORM_UWP
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
#endif

	unsigned InspectInputSignature(ID3DBlob* d3dBlob)
	{
		D3D11_SHADER_DESC shaderDesc;
		unsigned elementHash = 0;

#if ALIMER_PLATFORM_WINDOWS && !ALIMER_PLATFORM_UWP
		const GUID reflectGuid = s_compiler->IID_ID3D11ShaderReflection;
#else
		const GUID reflectGuid = IID_ID3D11ShaderReflection;
#endif

		ComPtr<ID3D11ShaderReflection> reflection;
		D3DReflect(
			d3dBlob->GetBufferPointer(),
			d3dBlob->GetBufferSize(),
			reflectGuid,
			IID_PPV_ARGS_Helper(&reflection));
		if (!reflection)
		{
			ALIMER_LOGERROR("Failed to reflect vertex shader's input signature");
			return elementHash;
		}

		reflection->GetDesc(&shaderDesc);
		for (UINT i = 0; i < shaderDesc.InputParameters; ++i)
		{
			D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
			reflection->GetInputParameterDesc(i, &paramDesc);

			elementHash |= VertexBuffer::ElementHash(i, paramDesc.SemanticName);
		}

		return elementHash;
	}

	ShaderVariation::ShaderVariation(Shader* parent_, const std::string& defines)
		: parent(parent_)
		, _stage(parent->GetStage())
		, _defines(defines)
	{
	}

	ShaderVariation::~ShaderVariation()
	{
		Release();
	}

	void ShaderVariation::Release()
	{
		if (graphics && (graphics->GetVertexShader() == this || graphics->GetPixelShader() == this))
			graphics->SetShaders(nullptr, nullptr);

		if (blob)
		{
			ID3DBlob* d3dBlob = (ID3DBlob*)blob;
			d3dBlob->Release();
			blob = nullptr;
		}

		if (shader)
		{
			if (_stage == ShaderStage::Vertex)
			{
				ID3D11VertexShader* d3dShader = (ID3D11VertexShader*)shader;
				d3dShader->Release();
			}
			else
			{
				ID3D11PixelShader* d3dShader = (ID3D11PixelShader*)shader;
				d3dShader->Release();
			}
			shader = nullptr;
		}

		_elementHash = 0;
		_compiled = false;
	}

	static inline std::string GetShaderTarget(
		ShaderStage stage, 
		uint32_t major, uint32_t minor)
	{
		switch (stage)
		{
		case ShaderStage::Vertex:
			return fmt::format("vs_{}_{}", major, minor);

		case ShaderStage::Fragment:
			return fmt::format("ps_{}_{}", major, minor);
		default:
			return nullptr;
		}
	}

	bool ShaderVariation::Compile()
	{
		if (_compiled)
			return shader != nullptr;

		ALIMER_PROFILE(CompileShaderVariation);

		// Do not retry without a Release() inbetween
		_compiled = true;

		if (!graphics || !graphics->IsInitialized())
		{
			ALIMER_LOGERROR("Can not compile shader without initialized Graphics subsystem");
			return false;
		}
		if (!parent)
		{
			ALIMER_LOGERROR("Can not compile shader without parent shader resource");
			return false;
		}

		// Collect defines into macros
		auto defineNames = str::Split(_defines, " ");
		std::vector<std::string> defineValues;
		std::vector<D3D_SHADER_MACRO> macros;

		for (size_t i = 0; i < defineNames.size(); ++i)
		{
			size_t equalsPos = defineNames[i].find('=');
			if (equalsPos != std::string::npos)
			{
				defineValues.push_back(defineNames[i].substr(equalsPos + 1));
				defineNames[i].resize(equalsPos);
			}
			else
				defineValues.push_back("1");
		}
		for (size_t i = 0; i < defineNames.size(); ++i)
		{
			D3D_SHADER_MACRO macro;
			macro.Name = defineNames[i].c_str();
			macro.Definition = defineValues[i].c_str();
			macros.push_back(macro);
		}

		D3D_SHADER_MACRO endMacro = { nullptr, nullptr };
		macros.push_back(endMacro);

		const auto& target = GetShaderTarget(_stage, 4, 0);

		/// \todo Level 3 could be used, but can lead to longer shader compile times, considering there is no binary caching yet
		DWORD flags = D3DCOMPILE_OPTIMIZATION_LEVEL2 | D3DCOMPILE_PREFER_FLOW_CONTROL;
		ComPtr<ID3DBlob> errorBlob;
		if (FAILED(D3DCompile(
			parent->GetSourceCode().c_str(),
			static_cast<SIZE_T>(parent->GetSourceCode().length()),
			"",
			macros.data(),
			0,
			"main",
			target.c_str(), 
			flags, 
			0, 
			(ID3DBlob**)&blob, 
			errorBlob.ReleaseAndGetAddressOf())))
		{
			if (errorBlob)
			{
				ALIMER_LOGERROR("Could not compile shader {}: {}", GetFullName(), errorBlob->GetBufferPointer());
			}

			return false;
		}

		ID3D11Device1* d3dDevice = static_cast<D3D11Graphics*>(graphics.Get())->GetD3DDevice();
		ID3DBlob* d3dBlob = (ID3DBlob*)blob;

#ifdef SHOW_DISASSEMBLY
		ID3DBlob* asmBlob = nullptr;
		D3DDisassemble(d3dBlob->GetBufferPointer(), d3dBlob->GetBufferSize(), 0, nullptr, &asmBlob);
		if (asmBlob)
		{
			std::string text((const char*)asmBlob->GetBufferPointer(), asmBlob->GetBufferSize());
			LOGINFOF("Shader %s disassembly: %s", FullName().CString(), text.CString());
			asmBlob->Release();
		}
#endif

		if (_stage == ShaderStage::Vertex)
		{
			_elementHash = InspectInputSignature(d3dBlob);
			d3dDevice->CreateVertexShader(d3dBlob->GetBufferPointer(), d3dBlob->GetBufferSize(), 0, (ID3D11VertexShader**)&shader);
		}
		else
		{
			d3dDevice->CreatePixelShader(d3dBlob->GetBufferPointer(), d3dBlob->GetBufferSize(), 0, (ID3D11PixelShader**)&shader);
		}

		if (!shader)
		{
			ALIMER_LOGERROR("Failed to create shader {}", GetFullName());
			return false;
		}

		ALIMER_LOGDEBUG(
			"[D3D11] - Compiled shader {} bytecode size {}",
			GetFullName().c_str(),
			d3dBlob->GetBufferSize());
		return true;
	}

	Shader* ShaderVariation::GetParent() const
	{
		return parent;
	}

	std::string ShaderVariation::GetFullName() const
	{
		if (parent)
			return _defines.empty() ? parent->GetName() : parent->GetName() + " (" + _defines + ")";

		return str::EMPTY;
	}

}