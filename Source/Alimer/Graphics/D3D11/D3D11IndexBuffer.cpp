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
#include "../Graphics.h"
#include "../IndexBuffer.h"

#include <d3d11.h>

namespace Alimer
{
	/*void IndexBuffer::Release()
	{
		if (graphics && graphics->GetIndexBuffer() == this)
			graphics->SetIndexBuffer(nullptr);

		if (_buffer)
		{
			_buffer->Release();
			_buffer = nullptr;
		}
	}*/

	bool IndexBuffer::SetData(uint32_t firstIndex, uint32_t indexCount, const void* data)
	{
#if TODO
		ALIMER_PROFILE(UpdateIndexBuffer);

		if (!data)
		{
			LOGERROR("Null source data for updating index buffer");
			return false;
		}
		if (firstIndex + indexCount > _indexCount)
		{
			LOGERROR("Out of bounds range for updating index buffer");
			return false;
		}
		if (_buffer && _resourceUsage == USAGE_IMMUTABLE)
		{
			LOGERROR("Can not update immutable index buffer");
			return false;
		}

		if (_shadowData)
		{
			memcpy(
				_shadowData.get() + firstIndex * _indexSize,
				data,
				indexCount * _indexSize);
		}

		if (_buffer)
		{
			ID3D11DeviceContext* d3dDeviceContext = (ID3D11DeviceContext*)graphics->D3DDeviceContext();

			if (_resourceUsage == USAGE_DYNAMIC)
			{
				D3D11_MAPPED_SUBRESOURCE mappedData;
				mappedData.pData = nullptr;

				d3dDeviceContext->Map(_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
				if (mappedData.pData)
				{
					memcpy((uint8_t*)mappedData.pData + firstIndex * _indexSize, data, indexCount * _indexSize);
					d3dDeviceContext->Unmap(_buffer, 0);
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
				destBox.left = firstIndex * _indexSize;
				destBox.right = destBox.left + indexCount * _indexSize;
				destBox.top = 0;
				destBox.bottom = 1;
				destBox.front = 0;
				destBox.back = 1;

				d3dDeviceContext->UpdateSubresource(_buffer, 0, &destBox, data, 0, 0);
			}
		}
#endif // TODO


		return true;
	}
}
