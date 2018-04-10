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

#include "../../Debug/Log.h"
#include "../../Debug/Profiler.h"
#include "D3D11Graphics.h"
#include "D3D11IndexBuffer.h"

#include <d3d11.h>

namespace Alimer
{

	IndexBuffer::IndexBuffer() :
		buffer(nullptr),
		numIndices(0),
		indexSize(0),
		usage(USAGE_DEFAULT)
	{
	}

	IndexBuffer::~IndexBuffer()
	{
		Release();
	}

	void IndexBuffer::Release()
	{
		if (graphics && graphics->GetIndexBuffer() == this)
			graphics->SetIndexBuffer(nullptr);

		if (buffer)
		{
			ID3D11Buffer* d3dBuffer = (ID3D11Buffer*)buffer;
			d3dBuffer->Release();
			buffer = nullptr;
		}
	}

	bool IndexBuffer::SetData(size_t firstIndex, size_t numIndices_, const void* data)
	{
		ALIMER_PROFILE(UpdateIndexBuffer);

		if (!data)
		{
			LOGERROR("Null source data for updating index buffer");
			return false;
		}
		if (firstIndex + numIndices_ > numIndices)
		{
			LOGERROR("Out of bounds range for updating index buffer");
			return false;
		}
		if (buffer && usage == USAGE_IMMUTABLE)
		{
			LOGERROR("Can not update immutable index buffer");
			return false;
		}

		if (_shadowData)
			memcpy(_shadowData.get() + firstIndex * indexSize, data, numIndices_ * indexSize);

		if (buffer)
		{
			ID3D11DeviceContext* d3dDeviceContext = (ID3D11DeviceContext*)graphics->D3DDeviceContext();

			if (usage == USAGE_DYNAMIC)
			{
				D3D11_MAPPED_SUBRESOURCE mappedData;
				mappedData.pData = nullptr;

				d3dDeviceContext->Map((ID3D11Buffer*)buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
				if (mappedData.pData)
				{
					memcpy((unsigned char*)mappedData.pData + firstIndex * indexSize, data, numIndices_ * indexSize);
					d3dDeviceContext->Unmap((ID3D11Buffer*)buffer, 0);
				}
				else
				{
					LOGERROR("Failed to map index buffer for update");
					return false;
				}
			}
			else
			{
				D3D11_BOX destBox;
				destBox.left = (unsigned)(firstIndex * indexSize);
				destBox.right = destBox.left + (unsigned)(numIndices_ * indexSize);
				destBox.top = 0;
				destBox.bottom = 1;
				destBox.front = 0;
				destBox.back = 1;

				d3dDeviceContext->UpdateSubresource((ID3D11Buffer*)buffer, 0, &destBox, data, 0, 0);
			}
		}

		return true;
	}

	bool IndexBuffer::Create(const void* data)
	{
		if (graphics && graphics->IsInitialized())
		{
			D3D11_BUFFER_DESC bufferDesc;
			D3D11_SUBRESOURCE_DATA initialData;
			memset(&bufferDesc, 0, sizeof bufferDesc);
			memset(&initialData, 0, sizeof initialData);
			initialData.pSysMem = data;

			bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bufferDesc.CPUAccessFlags = (usage == USAGE_DYNAMIC) ? D3D11_CPU_ACCESS_WRITE : 0;
			bufferDesc.Usage = (D3D11_USAGE)usage;
			bufferDesc.ByteWidth = (unsigned)(numIndices * indexSize);

			ID3D11Device* d3dDevice = (ID3D11Device*)graphics->D3DDevice();
			d3dDevice->CreateBuffer(&bufferDesc, data ? &initialData : nullptr, (ID3D11Buffer**)&buffer);

			if (!buffer)
			{
				LOGERROR("Failed to create index buffer");
				return false;
			}
			else
				LOGDEBUGF("Created index buffer numIndices %u indexSize %u", (unsigned)numIndices, (unsigned)indexSize);
		}

		return true;
	}

}
