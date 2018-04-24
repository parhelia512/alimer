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

#include "../Graphics/Buffer.h"

namespace Alimer
{
	/// GPU buffer for vertex data.
	class ALIMER_API VertexBuffer final : public Buffer
	{
	public:
		/// Construct.
		VertexBuffer();
		/// Destruct.
		~VertexBuffer() override;

		/// Define buffer. Immutable buffers must specify initial data here. Return true on success.
		bool Define(ResourceUsage usage, uint32_t vertexCount, const std::vector<VertexElement>& elements, bool useShadowData, const void* data = nullptr);
		/// Define buffer. Immutable buffers must specify initial data here. Return true on success.
		bool Define(ResourceUsage usage, uint32_t vertexCount, uint32_t numElements, const VertexElement* elements, bool useShadowData, const void* data = nullptr);
		/// Redefine buffer data either completely or partially. Not supported for immutable buffers. Return true on success.
		bool SetData(uint32_t firstVertex, uint32_t vertexCount, const void* data);

		/// Return number of vertices.
		uint32_t GetVertexCount() const { return _vertexCount; }
		/// Return number of vertex elements.
		uint32_t GetElementsCout() const { return static_cast<uint32_t>(_elements.size()); }
		/// Return vertex elements.
		const std::vector<VertexElement>& GetElements() const { return _elements; }
		/// Return vertex declaration hash code.
		uint64_t GetElementHash() const { return _elementHash; }

		/// Compute the hash code of one vertex element by index and semantic.
		static uint64_t ElementHash(uint32_t index, const char* semanticName);

	private:
		uint32_t _vertexCount{};
		/// Vertex elements.
		std::vector<VertexElement> _elements;
		/// Vertex element hash code.
		uint64_t _elementHash;
	};
}