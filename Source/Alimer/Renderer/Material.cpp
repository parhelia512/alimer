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
#include "../IO/Stream.h"

using namespace std;

namespace Alimer
{
	SharedPtr<Material> Material::defaultMaterial;
	unordered_map<string, uint8_t> Material::_passIndices;
	vector<string> Material::_passNames;
	uint8_t Material::_nextPassIndex = 0;

	Pass::Pass(Material* parent, const std::string& name)
		: _parent(parent)
		, _name(name)
		, _shaderHash(0)
		, shadersLoaded(false)
	{
		Reset();
	}

	Pass::~Pass()
	{
	}

	bool Pass::LoadJSON(const json& source)
	{
		if (source.count("vs"))
			_shaderNames[static_cast<unsigned>(ShaderStage::Vertex)] = source["vs"].get<string>();
		if (source.count("ps"))
			_shaderNames[static_cast<unsigned>(ShaderStage::Fragment)] = source["ps"].get<string>();
		if (source.count("vsDefines"))
			_shaderDefines[static_cast<unsigned>(ShaderStage::Vertex)] = source["vsDefines"].get<string>();
		if (source.count("psDefines"))
			_shaderDefines[static_cast<unsigned>(ShaderStage::Fragment)] = source["psDefines"].get<string>();

		if (source.count("depthFunc"))
			depthFunc = (CompareFunc)str::ListIndex(source["depthFunc"].get<string>(), compareFuncNames, CMP_LESS_EQUAL);
		if (source.count("depthWrite"))
			depthWrite = source["depthWrite"].get<bool>();
		if (source.count("depthClip"))
			depthClip = source["depthClip"].get<bool>();
		if (source.count("alphaToCoverage"))
			alphaToCoverage = source["alphaToCoverage"].get<bool>();
		if (source.count("colorWriteMask"))
			colorWriteMask = source["colorWriteMask"].get<uint8_t>();

		if (source.count("blendMode"))
		{
			blendMode = blendModes[str::ListIndex(source["blendMode"].get<string>(), blendModeNames, BLEND_MODE_REPLACE)];
		}
		else
		{
			if (source.count("blendEnable"))
				blendMode.blendEnable = source["blendEnable"].get<bool>();
			if (source.count("srcBlend"))
				blendMode.srcBlend = (BlendFactor)str::ListIndex(source["srcBlend"].get<string>(), blendFactorNames, BLEND_ONE);
			if (source.count("destBlend"))
				blendMode.destBlend = (BlendFactor)str::ListIndex(source["destBlend"].get<string>(), blendFactorNames, BLEND_ONE);
			if (source.count("blendOp"))
				blendMode.blendOp = (BlendOp)str::ListIndex(source["blendOp"].get<string>(), blendOpNames, BLEND_OP_ADD);
			if (source.count("srcBlendAlpha"))
				blendMode.srcBlendAlpha = (BlendFactor)str::ListIndex(source["srcBlendAlpha"].get<string>(), blendFactorNames, BLEND_ONE);
			if (source.count("destBlendAlpha"))
				blendMode.destBlendAlpha = (BlendFactor)str::ListIndex(source["destBlendAlpha"].get<string>(), blendFactorNames, BLEND_ONE);
			if (source.count("blendOpAlpha"))
				blendMode.blendOpAlpha = (BlendOp)str::ListIndex(source["blendOpAlpha"].get<string>(), blendOpNames, BLEND_OP_ADD);
		}

		if (source.count("fillMode"))
			fillMode = (FillMode)str::ListIndex(source["fillMode"].get<string>(), fillModeNames, FILL_SOLID);
		if (source.count("cullMode"))
			cullMode = (CullMode)str::ListIndex(source["cullMode"].get<string>(), cullModeNames, CULL_BACK);

		OnShadersChanged();
		return true;
	}

	bool Pass::SaveJSON(json& dest)
	{
		dest.object();

		if (_shaderNames[static_cast<unsigned>(ShaderStage::Vertex)].length())
			dest["vs"] = _shaderNames[static_cast<unsigned>(ShaderStage::Vertex)];
		if (_shaderNames[static_cast<unsigned>(ShaderStage::Fragment)].length())
			dest["ps"] = _shaderNames[static_cast<unsigned>(ShaderStage::Fragment)];
		if (_shaderDefines[static_cast<unsigned>(ShaderStage::Vertex)].length())
			dest["vsDefines"] = _shaderDefines[static_cast<unsigned>(ShaderStage::Vertex)];
		if (_shaderDefines[static_cast<unsigned>(ShaderStage::Fragment)].length())
			dest["psDefines"] = _shaderDefines[static_cast<unsigned>(ShaderStage::Fragment)];

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
		_shaderNames[static_cast<unsigned>(ShaderStage::Vertex)] = vsName;
		_shaderNames[static_cast<unsigned>(ShaderStage::Fragment)] = psName;
		_shaderDefines[static_cast<unsigned>(ShaderStage::Vertex)] = vsDefines;
		_shaderDefines[static_cast<unsigned>(ShaderStage::Fragment)] = psDefines;
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

	Material* Pass::GetParent() const
	{
		return _parent.Get();
	}

	void Pass::OnShadersChanged()
	{
		// Reset existing variations
		for (uint32_t i = 0; i < static_cast<uint32_t>(ShaderStage::Count); ++i)
		{
			shaders[i].Reset();
			shaderVariations[i].clear();
		}

		shadersLoaded = false;

		// Combine and trim the shader defines
		for (uint32_t i = 0; i < static_cast<uint32_t>(ShaderStage::Count); ++i)
		{
			const std::string& materialDefines = _parent->ShaderDefines((ShaderStage)i);
			if (materialDefines.length())
				_combinedShaderDefines[i] = (str::Trim(materialDefines) + " " + str::Trim(_shaderDefines[i]));
			else
				_combinedShaderDefines[i] = str::Trim(_shaderDefines[i]);
		}

		_shaderHash = StringHash(
			_shaderNames[static_cast<uint32_t>(ShaderStage::Vertex)]
			+ _shaderNames[static_cast<uint32_t>(ShaderStage::Fragment)]
			+ _combinedShaderDefines[static_cast<uint32_t>(ShaderStage::Vertex)]
			+ _combinedShaderDefines[static_cast<uint32_t>(ShaderStage::Fragment)]
		).Value();
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

		_loadJSON = std::make_unique<JSONFile>();
		if (!_loadJSON->Load(source))
			return false;

		const json& root = _loadJSON->GetRoot();

		_shaderDefines[static_cast<uint32_t>(ShaderStage::Vertex)].clear();
		_shaderDefines[static_cast<uint32_t>(ShaderStage::Fragment)].clear();
		if (root.count("vsDefines"))
			_shaderDefines[static_cast<uint32_t>(ShaderStage::Vertex)] = root["vsDefines"].get<string>();
		if (root.count("psDefines"))
			_shaderDefines[static_cast<uint32_t>(ShaderStage::Fragment)] = root["psDefines"].get<string>();

		return true;
	}

	bool Material::EndLoad()
	{
		ALIMER_PROFILE(EndLoadMaterial);

		const json& root = _loadJSON->GetRoot();

		_passes.clear();
		if (root.count("passes") && root["passes"].is_object())
		{
			const json& jsonPasses = root["passes"];
			for (auto it = jsonPasses.begin(); it != jsonPasses.end(); ++it)
			{
				Pass* newPass = CreatePass(it.key());
				newPass->LoadJSON(it.value());
			}
		}

		constantBuffers[static_cast<uint32_t>(ShaderStage::Vertex)].Reset();
		if (root.count("vsConstantBuffer"))
		{
			constantBuffers[static_cast<uint32_t>(ShaderStage::Vertex)] = new ConstantBuffer();
			constantBuffers[static_cast<uint32_t>(ShaderStage::Vertex)]->LoadJSON(root["vsConstantBuffer"]);
		}

		constantBuffers[static_cast<uint32_t>(ShaderStage::Fragment)].Reset();
		if (root.count("psConstantBuffer"))
		{
			constantBuffers[static_cast<uint32_t>(ShaderStage::Fragment)] = new ConstantBuffer();
			constantBuffers[static_cast<uint32_t>(ShaderStage::Fragment)]->LoadJSON(root["psConstantBuffer"]);
		}

		/// \todo Queue texture loads during BeginLoad()
		ResetTextures();
		if (root.count("textures")
			&& root["textures"].is_object())
		{
			ResourceCache* cache = GetSubsystem<ResourceCache>();
			const json jsonTextures = root["textures"];
			for (auto it = jsonTextures.begin(); it != jsonTextures.end(); ++it)
			{
				SetTexture(str::ToInt(it.key().c_str()), cache->LoadResource<Texture>(it.value().get<string>()));
			}
		}

		_loadJSON.reset();
		return true;
	}

	bool Material::Save(Stream& dest)
	{
		ALIMER_PROFILE(SaveMaterial);

		json root = json::object();

		if (_shaderDefines[static_cast<uint32_t>(ShaderStage::Vertex)].length())
			root["vsDefines"] = _shaderDefines[static_cast<uint32_t>(ShaderStage::Vertex)];
		if (_shaderDefines[static_cast<uint32_t>(ShaderStage::Fragment)].length())
			root["psDefines"] = _shaderDefines[static_cast<uint32_t>(ShaderStage::Fragment)];

		if (_passes.size())
		{
			root["passes"].object();
			for (auto pass : _passes)
			{
				if (pass)
					pass->SaveJSON(root["passes"][pass->GetName()]);
			}
		}

		if (constantBuffers[static_cast<uint32_t>(ShaderStage::Vertex)])
			constantBuffers[static_cast<uint32_t>(ShaderStage::Vertex)]->SaveJSON(root["vsConstantBuffer"]);
		if (constantBuffers[static_cast<uint32_t>(ShaderStage::Fragment)])
			constantBuffers[static_cast<uint32_t>(ShaderStage::Fragment)]->SaveJSON(root["psConstantBuffer"]);

		root["textures"].object();
		for (size_t i = 0; i < MAX_MATERIAL_TEXTURE_UNITS; ++i)
		{
			if (textures[i])
			{
				root["textures"][std::to_string((int)i)] = textures[i]->GetName();
			}
		}

		dest.WriteString(root.dump(4));
		return true;
	}

	Pass* Material::CreatePass(const std::string& name)
	{
		const uint8_t index = GetPassIndex(name);
		if (_passes.size() <= index)
			_passes.resize(index + 1);

		if (!_passes[index])
			_passes[index] = std::make_shared<Pass>(this, name);

		return _passes[index].get();
	}

	void Material::RemovePass(const std::string& name)
	{
		const uint8_t index = GetPassIndex(name, false);
		if (index < _passes.size())
			_passes[index].reset();
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
		constantBuffers[static_cast<uint32_t>(stage)] = buffer;
	}

	void Material::SetShaderDefines(
		const std::string& vsDefines,
		const std::string& psDefines)
	{
		_shaderDefines[static_cast<uint32_t>(ShaderStage::Vertex)] = vsDefines;
		_shaderDefines[static_cast<uint32_t>(ShaderStage::Fragment)] = psDefines;

		for (auto pass : _passes)
		{
			if (pass)
				pass->OnShadersChanged();
		}
	}

	Pass* Material::FindPass(const string& name) const
	{
		return GetPass(GetPassIndex(name, false));
	}

	Pass* Material::GetPass(uint8_t index) const
	{
		return index < _passes.size() ? _passes[index].get() : nullptr;
	}

	Texture* Material::GetTexture(size_t index) const
	{
		return index < MAX_MATERIAL_TEXTURE_UNITS ? textures[index].Get() : nullptr;
	}

	ConstantBuffer* Material::GetConstantBuffer(ShaderStage stage) const
	{
		if (stage < ShaderStage::Count)
			return constantBuffers[static_cast<unsigned>(stage)].Get();

		return nullptr;
	}

	const std::string& Material::ShaderDefines(ShaderStage stage) const
	{
		if (stage < ShaderStage::Count)
			return _shaderDefines[static_cast<unsigned>(stage)];

		return str::EMPTY;
	}

	uint8_t Material::GetPassIndex(const std::string& name, bool createNew)
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

	const string& Material::GetPassName(uint8_t index)
	{
		return index < _passNames.size() ? _passNames[index] : str::EMPTY;
	}

	Material* Material::GetDefaultMaterial()
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
