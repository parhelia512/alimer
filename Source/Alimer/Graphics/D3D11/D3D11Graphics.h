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

#include "../Graphics.h"
#include "D3D11Prerequisites.h"

namespace Alimer
{
	struct GraphicsImpl;
	class D3D11Buffer;

	/// Compiled shader with specific defines.
	class D3D11Graphics final : public Graphics
	{
	public:
		/// Construct. Set parent shader and defines but do not compile yet.
		D3D11Graphics(bool validation, const std::string& applicationName);
		/// Destruct.
		~D3D11Graphics() override;

		/// Is backend supported?
		static bool IsSupported();

		ID3D11Device* GetD3DDevice() const;
		ID3D11DeviceContext* GetD3DDeviceContext() const;

		/// Return currently bound index buffer.
		D3D11Buffer* GetIndexBuffer() const { return _indexBuffer; }

	private:
		void Finalize() override;
		BufferHandle* CreateBuffer(BufferUsage usage, uint32_t size, uint32_t stride, ResourceUsage resourceUsage, const void* initialData) override;

		void SetIndexBufferCore(BufferHandle* handle, IndexType type) override;

		void WaitIdle();

		/// Reset internally tracked state.
		void ResetState();

		/// Bound index buffer.
		D3D11Buffer* _indexBuffer = nullptr;
	};

}