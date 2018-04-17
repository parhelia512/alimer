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

#include "../Resource/Resource.h"
#include "GraphicsDefs.h"

namespace Alimer
{
	class ShaderVariation;

	/// %Shader resource. Defines either vertex or pixel shader source code, from which variations can be compiled by specifying defines.
	class ALIMER_API Shader : public Resource
	{
		OBJECT(Shader);

	public:
		/// Construct.
		Shader() = default;
		/// Destruct.
		~Shader();

		/// Register object factory.
		static void RegisterObject();

		/// Load shader code from a stream. Return true on success.
		bool BeginLoad(Stream& source) override;
		/// Finish shader loading in the main thread. Return true on success.
		bool EndLoad() override;

		/// Define shader stage and source code. All existing variations are destroyed.
		void Define(ShaderStage stage, const std::string& code);
		/// Create and return a variation with defines, eg. "PERPIXEL NORMALMAP NUMLIGHTS=4". Existing variation is returned if possible. Variations should be cached to avoid repeated query.
		ShaderVariation* CreateVariation(const std::string& defines = "");

		/// Return shader stage.
		ShaderStage Stage() const { return _stage; }
		/// Return shader source code.
		const std::string& GetSourceCode() const { return _sourceCode; }

		/// Sort the defines and strip extra spaces to prevent creation of unnecessary duplicate shader variations. When requesting variations, the defines should preferably be normalized already to save time.
		static std::string NormalizeDefines(const std::string& defines);

	private:
		/// Process include statements in the shader source code recursively. Return true if successful.
		bool ProcessIncludes(std::string& code, Stream& source);

		/// %Shader variations.
		UnorderedMap<StringHash, SharedPtr<ShaderVariation> > _variations;
		/// %Shader stage.
		ShaderStage _stage{ SHADER_VS };
		/// %Shader source code.
		std::string _sourceCode;
	};

}
