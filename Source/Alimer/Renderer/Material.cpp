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

#include "../Debug/Profiler.h"
#include "../Graphics/ConstantBuffer.h"
#include "../Graphics/ShaderVariation.h"
#include "../Graphics/Shader.h"
#include "../Graphics/Texture.h"
#include "../Resource/JSONFile.h"
#include "../Resource/ResourceCache.h"
#include "Material.h"

using namespace std;

namespace Alimer
{
	SharedPtr<Material> Material::defaultMaterial;
	unordered_map<string, uint8_t> Material::_passIndices;
	vector<string> Material::_passNames;
	uint8_t Material::_nextPassIndex = 0;

	Pass::Pass(Material* parent_, const std::string& name)
		: parent(parent_)
		, _name(name)
		, _shaderHash(0)
		, shadersLoaded(false)
	{
		Reset();
	}

	Pass::~Pass()
	{
	}

	bool Pass::LoadJSON(const JSONValue& source)
	{
		if (source.Contains("vs"))
			shaderNames[SHADER_VS] = source["vs"].GetStdString();
		if (source.Contains("ps"))
			shaderNames[SHADER_PS] = source["ps"].GetStdString();
		if (source.Contains("vsDefines"))
			shaderDefines[SHADER_VS] = source["vsDefines"].GetStdString();
		if (source.Contains("psDefines"))
			shaderDefines[SHADER_PS] = source["psDefines"].GetStdString();

		if (source.Contains("depthFunc"))
			depthFunc = (CompareFunc)String::ListIndex(source["depthFunc"].GetString(), compareFuncNames, CMP_LESS_EQUAL);
		if (source.Contains("depthWrite"))
			depthWrite = source["depthWrite"].GetBool();
		if (source.Contains("depthClip"))
			depthClip = source["depthClip"].GetBool();
		if (source.Contains("alphaToCoverage"))
			alphaToCoverage = source["alphaToCoverage"].GetBool();
		if (source.Contains("colorWriteMask"))
			colorWriteMask = (unsigned char)source["colorWriteMask"].GetNumber();
		if (source.Contains("blendMode"))
			blendMode = blendModes[String::ListIndex(source["blendMode"].GetString(), blendModeNames, BLEND_MODE_REPLACE)];
		else
		{
			if (source.Contains("blendEnable"))
				blendMode.blendEnable = source["blendEnable"].GetBool();
			if (source.Contains("srcBlend"))
				blendMode.srcBlend = (BlendFactor)String::ListIndex(source["srcBlend"].GetString(), blendFactorNames, BLEND_ONE);
			if (source.Contains("destBlend"))
				blendMode.destBlend = (BlendFactor)String::ListIndex(source["destBlend"].GetString(), blendFactorNames, BLEND_ONE);
			if (source.Contains("blendOp"))
				blendMode.blendOp = (BlendOp)String::ListIndex(source["blendOp"].GetString(), blendOpNames, BLEND_OP_ADD);
			if (source.Contains("srcBlendAlpha"))
				blendMode.srcBlendAlpha = (BlendFactor)String::ListIndex(source["srcBlendAlpha"].GetString(), blendFactorNames, BLEND_ONE);
			if (source.Contains("destBlendAlpha"))
				blendMode.destBlendAlpha = (BlendFactor)String::ListIndex(source["destBlendAlpha"].GetString(), blendFactorNames, BLEND_ONE);
			if (source.Contains("blendOpAlpha"))
				blendMode.blendOpAlpha = (BlendOp)String::ListIndex(source["blendOpAlpha"].GetString(), blendOpNames, BLEND_OP_ADD);
		}

		if (source.Contains("fillMode"))
			fillMode = (FillMode)String::ListIndex(source["fillMode"].GetString(), fillModeNames, FILL_SOLID);
		if (source.Contains("cullMode"))
			cullMode = (CullMode)String::ListIndex(source["cullMode"].GetString(), cullModeNames, CULL_BACK);

		OnShadersChanged();
		return true;
	}

	bool Pass::SaveJSON(JSONValue& dest)
	{
		dest.SetEmptyObject();

		if (shaderNames[SHADER_VS].length())
			dest["vs"] = shaderNames[SHADER_VS];
		if (shaderNames[SHADER_PS].length())
			dest["ps"] = shaderNames[SHADER_PS];
		if (shaderDefines[SHADER_VS].length())
			dest["vsDefines"] = shaderDefines[SHADER_VS];
		if (shaderDefines[SHADER_PS].length())
			dest["psDefines"] = shaderDefines[SHADER_PS];

		dest["depthFunc"] = compareFuncNames[depthFunc];
		dest["depthWrite"] = depthWrite;
		dest["depthClip"] = depthClip;
		dest["alphaToCoverage"] = alphaToCoverage;
		dest["colorWriteMask"] = colorWriteMask;

		// Prefer saving a predefined blend mode if possible for better readability
		bool blendModeFound = false;
		for (size_t i = 0; i < MAX_BLEND_MODES; ++i)
		{
			if (blendMode == blendModes[i])
			{
				dest["blendMode"] = blendModeNames[i];
				blendModeFound = true;
				break;
			}
		}

		if (!blendModeFound)
		{
			dest["blendEnable"] = blendMode.blendEnable;
			dest["srcBlend"] = blendFactorNames[blendMode.srcBlend];
			dest["destBlend"] = blendFactorNames[blendMode.destBlend];
			dest["blendOp"] = blendOpNames[blendMode.blendOp];
			dest["srcBlendAlpha"] = blendFactorNames[blendMode.srcBlendAlpha];
			dest["destBlendAlpha"] = blendFactorNames[blendMode.destBlendAlpha];
			dest["blendOpAlpha"] = blendOpNames[blendMode.blendOpAlpha];
		}

		dest["fillMode"] = fillModeNames[fillMode];
		dest["cullMode"] = cullModeNames[cullMode];

		return true;
	}

	void Pass::SetBlendMode(BlendMode mode)
	{
		blendMode = blendModes[mode];
	}

	void Pass::SetShaders(
		const std::string& vsName,
		const std::string& psName,
		const std::string& vsDefines,
		const std::string& psDefines)
	{
		shaderNames[SHADER_VS] = vsName;
		shaderNames[SHADER_PS] = psName;
		shaderDefines[SHADER_VS] = vsDefines;
		shaderDefines[SHADER_PS] = psDefines;
		OnShadersChanged();
	}

	void Pass::Reset()
	{
		depthFunc = CMP_LESS_EQUAL;
		depthWrite = true;
		depthClip = true;
		alphaToCoverage = false;
		colorWriteMask = COLORMASK_ALL;
		blendMode.Reset();
		cullMode = CULL_BACK;
		fillMode = FILL_SOLID;
	}

	Material* Pass::Parent() const
	{
		return parent;
	}

	void Pass::OnShadersChanged()
	{
		// Reset existing variations
		for (size_t i = 0; i < MAX_SHADER_STAGES; ++i)
		{
			shaders[i].Reset();
			shaderVariations[i].clear();
		}

		shadersLoaded = false;

		// Combine and trim the shader defines
		for (size_t i = 0; i < MAX_SHADER_STAGES; ++i)
		{
			const std::string& materialDefines = parent->ShaderDefines((ShaderStage)i);
			if (materialDefines.length())
				_combinedShaderDefines[i] = (str::Trim(materialDefines) + " " + str::Trim(shaderDefines[i]));
			else
				_combinedShaderDefines[i] = str::Trim(shaderDefines[i]);
		}

		_shaderHash = StringHash(shaderNames[SHADER_VS] + shaderNames[SHADER_PS] + _combinedShaderDefines[SHADER_VS] +
			_combinedShaderDefines[SHADER_PS]).Value();
	}

	Material::Material()
	{
	}

	Material::~Material()
	{
	}

	void Material::RegisterObject()
	{
		RegisterFactory<Material>();
	}

	bool Material::BeginLoad(Stream& source)
	{
		ALIMER_PROFILE(BeginLoadMaterial);

		loadJSON = std::make_unique<JSONFile>();
		if (!loadJSON->Load(source))
			return false;

		const JSONValue& root = loadJSON->Root();

		shaderDefines[SHADER_VS].clear();
		shaderDefines[SHADER_PS].clear();
		if (root.Contains("vsDefines"))
			shaderDefines[SHADER_VS] = root["vsDefines"].GetStdString();
		if (root.Contains("psDefines"))
			shaderDefines[SHADER_PS] = root["psDefines"].GetStdString();

		return true;
	}

	bool Material::EndLoad()
	{
		ALIMER_PROFILE(EndLoadMaterial);

		const JSONValue& root = loadJSON->Root();

		passes.clear();
		if (root.Contains("passes"))
		{
			const JSONObject& jsonPasses = root["passes"].GetObject();
			for (auto it = jsonPasses.begin(); it != jsonPasses.end(); ++it)
			{
				Pass* newPass = CreatePass(it->first);
				newPass->LoadJSON(it->second);
			}
		}

		constantBuffers[SHADER_VS].Reset();
		if (root.Contains("vsConstantBuffer"))
		{
			constantBuffers[SHADER_VS] = new ConstantBuffer();
			constantBuffers[SHADER_VS]->LoadJSON(root["vsConstantBuffer"].GetObject());
		}

		constantBuffers[SHADER_PS].Reset();
		if (root.Contains("psConstantBuffer"))
		{
			constantBuffers[SHADER_PS] = new ConstantBuffer();
			constantBuffers[SHADER_PS]->LoadJSON(root["psConstantBuffer"].GetObject());
		}

		/// \todo Queue texture loads during BeginLoad()
		ResetTextures();
		if (root.Contains("textures"))
		{
			ResourceCache* cache = Subsystem<ResourceCache>();
			const JSONObject& jsonTextures = root["textures"].GetObject();
			for (auto it = jsonTextures.begin(); it != jsonTextures.end(); ++it)
				SetTexture(str::ToInt(it->first.c_str()), cache->LoadResource<Texture>(it->second.GetStdString()));
		}

		loadJSON.reset();
		return true;
	}

	bool Material::Save(Stream& dest)
	{
		ALIMER_PROFILE(SaveMaterial);

		JSONFile saveJSON;
		JSONValue& root = saveJSON.Root();
		root.SetEmptyObject();

		if (shaderDefines[SHADER_VS].length())
			root["vsDefines"] = shaderDefines[SHADER_VS];
		if (shaderDefines[SHADER_PS].length())
			root["psDefines"] = shaderDefines[SHADER_PS];

		if (passes.size())
		{
			root["passes"].SetEmptyObject();
			for (auto it = passes.begin(); it != passes.end(); ++it)
			{
				Pass* pass = *it;
				if (pass)
					pass->SaveJSON(root["passes"][pass->GetName()]);
			}
		}

		if (constantBuffers[SHADER_VS])
			constantBuffers[SHADER_VS]->SaveJSON(root["vsConstantBuffer"]);
		if (constantBuffers[SHADER_PS])
			constantBuffers[SHADER_PS]->SaveJSON(root["psConstantBuffer"]);

		root["textures"].SetEmptyObject();
		for (size_t i = 0; i < MAX_MATERIAL_TEXTURE_UNITS; ++i)
		{
			if (textures[i])
			{
				root["textures"][std::to_string((int)i)] = textures[i]->Name();
			}
		}

		return saveJSON.Save(dest);
	}

	Pass* Material::CreatePass(const std::string& name)
	{
		size_t index = PassIndex(name);
		if (passes.size() <= index)
			passes.resize(index + 1);

		if (!passes[index])
			passes[index] = new Pass(this, name);

		return passes[index];
	}

	void Material::RemovePass(const std::string& name)
	{
		size_t index = PassIndex(name, false);
		if (index < passes.size())
			passes[index].Reset();
	}

	void Material::SetTexture(size_t index, Texture* texture)
	{
		if (index < MAX_MATERIAL_TEXTURE_UNITS)
			textures[index] = texture;
	}

	void Material::ResetTextures()
	{
		for (size_t i = 0; i < MAX_MATERIAL_TEXTURE_UNITS; ++i)
			textures[i].Reset();
	}

	void Material::SetConstantBuffer(ShaderStage stage, ConstantBuffer* buffer)
	{
		if (stage < MAX_SHADER_STAGES)
			constantBuffers[stage] = buffer;
	}

	void Material::SetShaderDefines(
		const std::string& vsDefines,
		const std::string& psDefines)
	{
		shaderDefines[SHADER_VS] = vsDefines;
		shaderDefines[SHADER_PS] = psDefines;

		for (auto it = passes.begin(); it != passes.end(); ++it)
		{
			Pass* pass = *it;
			if (pass)
				pass->OnShadersChanged();
		}
	}

	Pass* Material::FindPass(const std::string& name) const
	{
		return GetPass(PassIndex(name, false));
	}

	Pass* Material::GetPass(uint8_t index) const
	{
		return index < passes.size() ? passes[index].Get() : nullptr;
	}

	Texture* Material::GetTexture(size_t index) const
	{
		return index < MAX_MATERIAL_TEXTURE_UNITS ? textures[index].Get() : nullptr;
	}

	ConstantBuffer* Material::GetConstantBuffer(ShaderStage stage) const
	{
		return stage < MAX_SHADER_STAGES ? constantBuffers[stage].Get() : nullptr;
	}

	const std::string& Material::ShaderDefines(ShaderStage stage) const
	{
		return stage < MAX_SHADER_STAGES ? shaderDefines[stage] : "";
	}

	uint8_t Material::PassIndex(const std::string& name, bool createNew)
	{
		std::string nameLower = str::ToLower(name);
		auto it = _passIndices.find(nameLower);
		if (it != end(_passIndices))
			return it->second;

		if (createNew)
		{
			_passIndices[nameLower] = _nextPassIndex;
			_passNames.push_back(nameLower);
			return _nextPassIndex++;
		}
		
		return 0xff;
	}

	const String& Material::PassName(uint8_t index)
	{
		return index < _passNames.size() ? _passNames[index] : String::EMPTY;
	}

	Material* Material::DefaultMaterial()
	{
		// Create on demand
		if (!defaultMaterial)
		{
			defaultMaterial = new Material();
			Pass* pass = defaultMaterial->CreatePass("opaque");
			pass->SetShaders("NoTexture", "NoTexture");

			pass = defaultMaterial->CreatePass("opaqueadd");
			pass->SetShaders("NoTexture", "NoTexture");
			pass->SetBlendMode(BLEND_MODE_ADD);
			pass->depthWrite = false;

			pass = defaultMaterial->CreatePass("shadow");
			pass->SetShaders("Shadow", "Shadow");
			pass->colorWriteMask = COLORMASK_NONE;
		}

		return defaultMaterial.Get();
	}
}
