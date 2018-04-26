//
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

#include "../GraphicsImpl.h"
#include "D3D11Prerequisites.h"

namespace Alimer
{
	class D3D11Graphics;

	class ALIMER_API D3D11Buffer final : public BufferHandle
	{
	public:
		/// Construct.
		D3D11Buffer(D3D11Graphics* graphics, BufferUsage usage, uint32_t size, uint32_t stride, ResourceUsage resourceUsage, const void* initialData);
		/// Destruct.
		~D3D11Buffer();

		bool SetData(uint32_t offset, uint32_t size, const void* data) override;

		/// Return the D3D11 buffer.
		ID3D11Buffer* GetD3DBuffer() const { return _buffer.Get(); }

	private:
		D3D11Graphics* _graphics;
		bool _isDynamic;

		/// D3D11 buffer.
		Microsoft::WRL::ComPtr<ID3D11Buffer> _buffer;
	};

}