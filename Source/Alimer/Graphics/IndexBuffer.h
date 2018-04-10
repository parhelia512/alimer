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

#include "../AlimerConfig.h"
#include "../Graphics/GPUObject.h"
#include "../Graphics/Graphics.h"
#include <memory>

#ifdef ALIMER_D3D11
struct ID3D11Buffer;
#endif

namespace Alimer
{
	/// GPU buffer for index data.
	class ALIMER_API IndexBuffer : public RefCounted, public GPUObject
	{
	public:
		/// Construct.
		IndexBuffer() = default;
		/// Destruct.
		~IndexBuffer();

		/// Release the index buffer and CPU shadow data.
		void Release() override;

		/// Define buffer. Immutable buffers must specify initial data here.  Return true on success.
		bool Define(ResourceUsage usage, uint32_t numIndices, uint32_t indexSize, bool useShadowData, const void* data = nullptr);
		/// Redefine buffer data either completely or partially. Not supported for immutable buffers. Return true on success.
		bool SetData(uint32_t firstIndex, uint32_t indexCount, const void* data);

		/// Return CPU-side shadow data if exists.
		uint8_t* ShadowData() const { return _shadowData.get(); }
		/// Return number of indices.
		uint32_t NumIndices() const { return _indexCount; }
		/// Return size of index in bytes.
		uint32_t IndexSize() const { return _indexSize; }
		/// Return resource usage type.
		ResourceUsage Usage() const { return _usage; }
		/// Return whether is dynamic.
		bool IsDynamic() const { return _usage == USAGE_DYNAMIC; }
		/// Return whether is immutable.
		bool IsImmutable() const { return _usage == USAGE_IMMUTABLE; }

#ifdef ALIMER_D3D11
		/// Return the D3D11 buffer. Used internally and should not be called by portable application code.
		ID3D11Buffer* GetD3DBuffer() const { return _buffer; }
#endif

	private:
		/// Create the GPU-side index buffer. Return true on success.
		bool Create(const void* data);

		/// CPU-side shadow data.
		std::unique_ptr<uint8_t[]> _shadowData;
		/// Number of indices.
		uint32_t _indexCount{ 0 };
		/// Size of index in bytes.
		uint32_t _indexSize{ 0 };
		/// Resource usage type.
		ResourceUsage _usage{ USAGE_DEFAULT };

#ifdef ALIMER_D3D11
		ID3D11Buffer* _buffer = nullptr;
#endif
	};
}