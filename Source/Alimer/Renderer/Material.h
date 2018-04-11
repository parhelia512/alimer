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

#pragma once

#include "../Graphics/GraphicsDefs.h"
#include "../Resource/Resource.h"

namespace Alimer
{
	class ConstantBuffer;
	class JSONFile;
	class JSONValue;
	class Material;
	class Shader;
	class ShaderVariation;
	class Texture;

	/// Render pass, which defines render state and shaders. A material may define several of these.
	class Pass : public RefCounted
	{
	public:
		/// Construct.
		Pass(Material* parent, const String& name);
		/// Destruct.
		~Pass();

		/// Load from JSON data. Return true on success.
		bool LoadJSON(const JSONValue& source);
		/// Save to JSON data. Return true on success.
		bool SaveJSON(JSONValue& dest);
		/// Set a predefined blend mode.
		void SetBlendMode(BlendMode mode);
		/// Set shader names and defines.
		void SetShaders(const String& vsName, const String& psName, const String& vsDefines = String::EMPTY, const String& psDefines = String::EMPTY);
		/// Reset render state to defaults.
		void Reset();

		/// Return parent material resource.
		Material* Parent() const;
		/// Return pass name.
		const String& Name() const { return name; }
		/// Return shader name by stage.
		const String& ShaderName(ShaderStage stage) const { return shaderNames[stage]; }
		/// Return shader defines by stage.
		const String& ShaderDefines(ShaderStage stage) const { return shaderDefines[stage]; }
		/// Return combined shader defines from the material and pass by stage.
		const String& CombinedShaderDefines(ShaderStage stage) const { return combinedShaderDefines[stage]; }
		/// Return shader hash value for state sorting.
		unsigned ShaderHash() const { return shaderHash; }

		/// Refresh the combined shader defines and shader hash and clear any cached shader variations. Called internally.
		void OnShadersChanged();

		/// Depth compare function.
		CompareFunc depthFunc;
		/// Depth write enable.
		bool depthWrite;
		/// Depth clipping enable.
		bool depthClip;
		/// Alpha to coverage enable.
		bool alphaToCoverage;
		/// Color write mask.
		unsigned char colorWriteMask;
		/// Blend mode parameters.
		BlendModeDesc blendMode;
		/// Polygon culling mode.
		CullMode cullMode;
		/// Polygon fill mode.
		FillMode fillMode;
		/// Shader resources. Filled by Renderer.
		SharedPtr<Shader> shaders[MAX_SHADER_STAGES];
		/// Cached shader variations. Filled by Renderer.
		std::map<uint16_t, WeakPtr<ShaderVariation> > shaderVariations[MAX_SHADER_STAGES];
		/// Shader load attempted flag. Filled by Renderer.
		bool shadersLoaded;

	private:
		/// Parent material resource.
		WeakPtr<Material> parent;
		/// Pass name.
		String name;
		/// Shader names.
		String shaderNames[MAX_SHADER_STAGES];
		/// Shader defines.
		String shaderDefines[MAX_SHADER_STAGES];
		/// Combined shader defines from both the pass and material. Filled by Renderer.
		String combinedShaderDefines[MAX_SHADER_STAGES];
		/// Shader hash calculated from names and defines.
		unsigned shaderHash;
	};

	/// %Material resource, which describes how to render 3D geometry and refers to textures. A material can contain several passes (for example normal rendering, and depth only.)
	class Material : public Resource
	{
		OBJECT(Material);

	public:
		/// Construct.
		Material();
		/// Destruct.
		~Material();

		/// Register object factory.
		static void RegisterObject();

		/// Load material from a stream. Return true on success.
		bool BeginLoad(Stream& source) override;
		/// Finalize material loading in the main thread. Return true on success.
		bool EndLoad() override;
		/// Save the material to a stream. Return true on success.
		bool Save(Stream& dest) override;

		/// Create and return a new pass. If pass with same name exists, it will be returned.
		Pass* CreatePass(const String& name);
		/// Remove a pass.
		void RemovePass(const String& name);
		/// Set a texture.
		void SetTexture(size_t index, Texture* texture);
		/// Reset all texture assignments.
		void ResetTextures();
		/// Set a constant buffer.
		void SetConstantBuffer(ShaderStage stage, ConstantBuffer* buffer);
		/// Set global shader defines. Clears existing shader cached variations from all passes.
		void SetShaderDefines(const String& vsDefines = String::EMPTY, const String& psDefines = String::EMPTY);

		/// Return pass by name or null if not found. Should not be called in performance-sensitive rendering loops.
		Pass* FindPass(const String& name) const;
		/// Return pass by index or null if not found.
		Pass* GetPass(unsigned char index) const;
		/// Return texture by texture unit.
		Texture* GetTexture(size_t index) const;
		/// Return constant buffer by stage.
		ConstantBuffer* GetConstantBuffer(ShaderStage stage) const;
		/// Return shader defines by stage.
		const String& ShaderDefines(ShaderStage stage) const;

		/// Return pass index from name. By default reserve a new index if the name was not known.
		static uint8_t PassIndex(const String& name, bool createNew = true);
		/// Return pass name by index.
		static const String& PassName(uint8_t index);
		/// Return a default opaque untextured material.
		static Material* DefaultMaterial();

		/// Material textures.
		SharedPtr<Texture> textures[MAX_MATERIAL_TEXTURE_UNITS];
		/// Constant buffers.
		SharedPtr<ConstantBuffer> constantBuffers[MAX_SHADER_STAGES];

	private:
		/// Passes by index.
		Vector<SharedPtr<Pass> > passes;
		/// Global shader defines.
		String shaderDefines[MAX_SHADER_STAGES];
		/// JSON data used for loading.
		std::unique_ptr<JSONFile> loadJSON;

		/// Default material.
		static SharedPtr<Material> defaultMaterial;
		/// Pass name to index mapping.
		static std::unordered_map<String, uint8_t> passIndices;
		/// Pass names by index.
		static Vector<String> passNames;
		/// Next free pass index.
		static unsigned char nextPassIndex;
	};

}
