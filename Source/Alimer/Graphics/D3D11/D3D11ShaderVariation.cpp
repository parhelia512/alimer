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
#include "../VertexBuffer.h"

#include <d3d11.h>
#include <d3dcompiler.h>

namespace Alimer
{
	unsigned InspectInputSignature(ID3DBlob* d3dBlob)
	{
		ID3D11ShaderReflection* reflection = nullptr;
		D3D11_SHADER_DESC shaderDesc;
		unsigned elementHash = 0;

		D3DReflect(d3dBlob->GetBufferPointer(), d3dBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflection);
		if (!reflection)
		{
			LOGERROR("Failed to reflect vertex shader's input signature");
			return elementHash;
		}

		reflection->GetDesc(&shaderDesc);
		for (size_t i = 0; i < shaderDesc.InputParameters; ++i)
		{
			D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
			reflection->GetInputParameterDesc((unsigned)i, &paramDesc);

			for (size_t j = 0; elementSemanticNames[j]; ++j)
			{
				if (!str::Compare(paramDesc.SemanticName, elementSemanticNames[j]))
				{
					elementHash |= VertexBuffer::ElementHash(i, (ElementSemantic)j);
					break;
				}
			}
		}

		reflection->Release();
		return elementHash;
	}

	ShaderVariation::ShaderVariation(Shader* parent_, const std::string& defines)
		: parent(parent_)
		, stage(parent->Stage())
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
			if (stage == SHADER_VS)
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

		elementHash = 0;
		compiled = false;
	}

	bool ShaderVariation::Compile()
	{
		if (compiled)
			return shader != nullptr;

		ALIMER_PROFILE(CompileShaderVariation);

		// Do not retry without a Release() inbetween
		compiled = true;

		if (!graphics || !graphics->IsInitialized())
		{
			LOGERROR("Can not compile shader without initialized Graphics subsystem");
			return false;
		}
		if (!parent)
		{
			LOGERROR("Can not compile shader without parent shader resource");
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

		/// \todo Level 3 could be used, but can lead to longer shader compile times, considering there is no binary caching yet
		DWORD flags = D3DCOMPILE_OPTIMIZATION_LEVEL2 | D3DCOMPILE_PREFER_FLOW_CONTROL;
		ID3DBlob* errorBlob = nullptr;
		if (FAILED(D3DCompile(
			parent->GetSourceCode().c_str(),
			(SIZE_T)parent->GetSourceCode().length(),
			"",
			macros.data(),
			0,
			"main",
			stage == SHADER_VS ? "vs_4_0" : "ps_4_0", flags, 0, (ID3DBlob**)&blob, &errorBlob)))
		{
			if (errorBlob)
			{
				LOGERRORF("Could not compile shader %s: %s", GetFullName().c_str(), errorBlob->GetBufferPointer());
				errorBlob->Release();
			}
			return false;
		}

		if (errorBlob)
		{
			errorBlob->Release();
			errorBlob = nullptr;
		}

		ID3D11Device* d3dDevice = (ID3D11Device*)graphics->D3DDevice();
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

		if (stage == SHADER_VS)
		{
			elementHash = InspectInputSignature(d3dBlob);
			d3dDevice->CreateVertexShader(d3dBlob->GetBufferPointer(), d3dBlob->GetBufferSize(), 0, (ID3D11VertexShader**)&shader);
		}
		else
			d3dDevice->CreatePixelShader(d3dBlob->GetBufferPointer(), d3dBlob->GetBufferSize(), 0, (ID3D11PixelShader**)&shader);

		if (!shader)
		{
			LOGERROR("Failed to create shader " + GetFullName());
			return false;
		}
		else
			LOGDEBUGF("Compiled shader %s bytecode size %u", GetFullName().c_str(), (unsigned)d3dBlob->GetBufferSize());

		return true;
	}

	Shader* ShaderVariation::GetParent() const
	{
		return parent;
	}

	std::string ShaderVariation::GetFullName() const
	{
		if (parent)
			return _defines.empty() ? parent->Name() : parent->Name() + " (" + _defines + ")";

		return "";
	}

}