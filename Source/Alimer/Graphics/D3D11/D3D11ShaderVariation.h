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

#include "../../Base/String.h"
#include "../GPUObject.h"
#include "../GraphicsDefs.h"

namespace Alimer
{
	class Shader;

	/// Compiled shader with specific defines.
	class ALIMER_API ShaderVariation : public RefCounted, public GPUObject
	{
	public:
		/// Construct. Set parent shader and defines but do not compile yet.
		ShaderVariation(Shader* parent, const std::string& defines);
		/// Destruct.
		~ShaderVariation();

		/// Release the compiled shader.
		void Release() override;

		/// Compile. Return true on success. No-op that returns previous result if compile already attempted.
		bool Compile();

		/// Return the parent shader resource.
		Shader* GetParent() const;
		/// Return full name combined from parent resource name and compilation defines.
		std::string GetFullName() const;
		/// Return shader stage.
		ShaderStage GetStage() const { return stage; }
		/// Return vertex element hash code for vertex shaders.
		unsigned ElementHash() const { return elementHash; }
		/// Return whether compile attempted.
		bool IsCompiled() const { return compiled; }

		/// Return the D3D11 shader byte blob. Null if not compiled yet or compile failed. Used internally and should not be called by portable application code.
		void* BlobObject() const { return blob; }
		/// Return the D3D11 shader. Null if not compiled yet or compile failed. Used internally and should not be called by portable application code.
		void* ShaderObject() { return shader; }

	private:
		/// Parent shader resource.
		WeakPtr<Shader> parent;
		/// Shader stage.
		ShaderStage stage;
		/// Compilation defines.
		std::string _defines;
		/// D3D11 shader byte blob.
		void* blob = nullptr;
		/// D3D11 shader.
		void* shader = nullptr;
		/// Vertex shader element hash code.
		uint32_t elementHash{};
		/// Compile attempted flag.
		bool compiled{};
	};

}