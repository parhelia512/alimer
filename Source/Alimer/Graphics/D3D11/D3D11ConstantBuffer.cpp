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
#include "../../Math/Color.h"
#include "../../Math/Matrix3.h"
#include "../../Math/Matrix3x4.h"
#include "D3D11ConstantBuffer.h"
#include "../Graphics.h"

#include <d3d11.h>

namespace Alimer
{
	ConstantBuffer::ConstantBuffer() :
		buffer(nullptr),
		usage(USAGE_DEFAULT),
		dirty(false)
	{
	}

	ConstantBuffer::~ConstantBuffer()
	{
		Release();
	}

	void ConstantBuffer::Release()
	{
		if (graphics)
		{
			for (size_t i = 0; i < MAX_SHADER_STAGES; ++i)
			{
				for (size_t j = 0; j < MAX_CONSTANT_BUFFERS; ++j)
				{
					if (graphics->GetConstantBuffer((ShaderStage)i, j) == this)
						graphics->SetConstantBuffer((ShaderStage)i, j, 0);
				}
			}
		}

		if (buffer)
		{
			ID3D11Buffer* d3dBuffer = (ID3D11Buffer*)buffer;
			d3dBuffer->Release();
			buffer = nullptr;
		}
	}

	bool ConstantBuffer::SetData(const void* data, bool copyToShadow)
	{
		if (copyToShadow)
		{
			memcpy(_shadowData.get(), data, _byteSize);
		}

		if (usage == USAGE_IMMUTABLE)
		{
			if (!buffer)
				return Create(data);
			else
			{
				LOGERROR("Apply can only be called once on an immutable constant buffer");
				return false;
			}
		}

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
					memcpy((unsigned char*)mappedData.pData, data, _byteSize);
					d3dDeviceContext->Unmap((ID3D11Buffer*)buffer, 0);
				}
				else
				{
					LOGERROR("Failed to map constant buffer for update");
					return false;
				}
			}
			else
				d3dDeviceContext->UpdateSubresource((ID3D11Buffer*)buffer, 0, nullptr, data, 0, 0);
		}

		dirty = false;
		return true;
	}

	bool ConstantBuffer::Create(const void* data)
	{
		dirty = false;

		if (graphics && graphics->IsInitialized())
		{
			D3D11_SUBRESOURCE_DATA initialData = { data, 0, 0 };

			D3D11_BUFFER_DESC bufferDesc = {};
			bufferDesc.ByteWidth = _byteSize;
			bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bufferDesc.CPUAccessFlags = (usage == USAGE_DYNAMIC) ? D3D11_CPU_ACCESS_WRITE : 0;
			bufferDesc.Usage = (D3D11_USAGE)usage;

			ID3D11Device* d3dDevice = (ID3D11Device*)graphics->D3DDevice();
			d3dDevice->CreateBuffer(&bufferDesc, data ? &initialData : nullptr, (ID3D11Buffer**)&buffer);

			if (!buffer)
			{
				LOGERROR("Failed to create constant buffer");
				return false;
			}
			
			LOGDEBUGF("Created constant buffer size %u", _byteSize);
		}

		return true;
	}

}
