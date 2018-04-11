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
#include "../Graphics/GPUObject.h"

#ifdef ALIMER_D3D11
struct ID3D11Buffer;
#endif

namespace Alimer
{
	class Graphics;

	/// API-specific GPU buffer representation.
	union BufferHandle
	{
#ifdef ALIMER_D3D11
		/// Object pointer (Direct3D.)
		ID3D11Buffer* ptr;
#endif
		/// Object name (OpenGL.)
		unsigned id;
	};

	/// GPU buffer for index data.
	class ALIMER_API Buffer : public RefCounted, public GPUObject
	{
	protected:
		/// Construct.
		Buffer(BufferUsage usage);

	public:
		/// Destruct.
		~Buffer();

		/// Release the index buffer and CPU shadow data.
		void Release() override final;

		/// Return resource usage type.
		ResourceUsage GetResourceUsage() const { return _resourceUsage; }
		/// Return whether is dynamic.
		bool IsDynamic() const { return _resourceUsage == USAGE_DYNAMIC; }
		/// Return whether is immutable.
		bool IsImmutable() const { return _resourceUsage == USAGE_IMMUTABLE; }

		BufferUsage GetUsage() const { return _usage; }
#ifdef ALIMER_D3D11
		ID3D11Buffer* GetHandle() { return _handle.ptr; }
#endif

		uint32_t GetId() { return _handle.id; }

		/// Return CPU-side shadow data if exists.
		uint8_t* GetShadowData() const { return _shadowData.get(); }

	protected:
		/// Create the GPU-side buffer. Return true on success.
		bool Create(bool useShadowData, const void* initialData);

	protected:
		uint64_t _size{};
		uint32_t _stride{};
		BufferUsage _usage;
		/// Resource usage type.
		ResourceUsage _resourceUsage{ USAGE_DEFAULT };

		/// CPU-side shadow data.
		std::unique_ptr<uint8_t[]> _shadowData;

		BufferHandle _handle{};
	};
}