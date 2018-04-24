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

#include "Debug/Log.h"
#include "Debug/Profiler.h"
#include "D3D11Buffer.h"
#include "D3D11Graphics.h"
#include "D3D11Convert.h"
#include "../VertexBuffer.h"

namespace Alimer
{
	D3D11Buffer::D3D11Buffer(D3D11Graphics* graphics, BufferUsage usage, uint32_t size, uint32_t stride, ResourceUsage resourceUsage, const void* initialData)
		: _graphics(graphics)
		, _isDynamic(resourceUsage == ResourceUsage::Dynamic)
	{
		D3D11_SUBRESOURCE_DATA resourceData = { initialData, 0, 0 };

		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
		desc.BindFlags = d3d11::Convert(usage);
		desc.CPUAccessFlags = _isDynamic ? D3D11_CPU_ACCESS_WRITE : 0;
		desc.Usage = static_cast<D3D11_USAGE>(resourceUsage);
		desc.ByteWidth = size;

		if (any(usage & BufferUsage::Indirect))
		{
			desc.StructureByteStride = stride;
			desc.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
		}

		D3D11_SUBRESOURCE_DATA initData;
		D3D11_SUBRESOURCE_DATA* pInitData = nullptr;
		if (initialData)
		{
			initData.pSysMem = initialData;
			initData.SysMemPitch = 0;
			initData.SysMemSlicePitch = 0;
			pInitData = &initData;
		}

		HRESULT hr = graphics->GetD3DDevice()->CreateBuffer(
			&desc,
			pInitData,
			_buffer.ReleaseAndGetAddressOf());
		if (FAILED(hr))
		{
			ALIMER_LOGERROR("[D3D11] - Failed to create Buffer");
		}
	}

	D3D11Buffer::~D3D11Buffer()
	{
		if (_graphics && _graphics->GetIndexBuffer() == this)
			_graphics->SetIndexBuffer(nullptr);

		for (uint32_t i = 0; i < MaxVertexBuffers; ++i)
		{
			auto boundVbo = _graphics->GetVertexBuffer(i);
			if (boundVbo && boundVbo->GetHandle() == this)
			{
				_graphics->SetVertexBuffer(i, nullptr, 0, VertexInputRate::Vertex);
			}
		}
	}

	bool D3D11Buffer::SetData(uint32_t offset, uint32_t size, const void* data)
	{
		ID3D11DeviceContext* d3dDeviceContext = _graphics->GetD3DDeviceContext();

		if (_isDynamic)
		{
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			HRESULT hr = d3dDeviceContext->Map(
				_buffer.Get(),
				0,
				D3D11_MAP_WRITE_DISCARD,
				0,
				&mappedResource);

			if (FAILED(hr))
			{
				ALIMER_LOGERROR("Failed to map index buffer for update");
				return false;
			}

			memcpy(
				static_cast<uint8_t*>(mappedResource.pData) + offset,
				data,
				size);

			d3dDeviceContext->Unmap(_buffer.Get(), 0);
		}
		else
		{
			D3D11_BOX destBox;
			destBox.left = offset;
			destBox.right = size;
			destBox.top = destBox.front = 0;
			destBox.bottom = destBox.back = 1;

			d3dDeviceContext->UpdateSubresource(
				_buffer.Get(),
				0,
				&destBox,
				data,
				0,
				0);
		}

		return true;
	}
}
