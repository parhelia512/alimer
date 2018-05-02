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

#include "../Debug/Log.h"
#include "../Debug/Profiler.h"
#include "../Object/Attribute.h"
#include "../Math/Color.h"
#include "../Math/Matrix3.h"
#include "../Math/Matrix3x4.h"
#include "ConstantBuffer.h"
#include "GraphicsDefs.h"
using namespace std;

namespace Alimer
{
	static const AttributeType elementToAttribute[] =
	{
		ATTR_INT,
		ATTR_FLOAT,
		ATTR_VECTOR2,
		ATTR_VECTOR3,
		ATTR_VECTOR4,
		MAX_ATTR_TYPES,
		ATTR_MATRIX3X4,
		ATTR_MATRIX4,
		MAX_ATTR_TYPES
	};

	static const char* elementTypeNames[] =
	{
		"int",
		"float",
		"Vector2",
		"Vector3",
		"Vector4",
		"UByte4",
		"Matrix3x4",
		"Matrix4",
		nullptr
	};

	ConstantBuffer::ConstantBuffer()
		: Buffer(BufferUsageBits::Uniform)
	{
	}

	ConstantBuffer::~ConstantBuffer()
	{
		Release();
	}

	bool ConstantBuffer::LoadJSON(const json& source)
	{
		bool hostVisible = true;
		if (source["hostVisible"].is_boolean())
			hostVisible = source["hostVisible"].get<bool>();

		std::vector<Constant> constants;

		const json& jsonConstants = source["constants"];
		for (size_t i = 0; i < jsonConstants.size(); ++i)
		{
			const json& jsonConstant = jsonConstants[i];
			const string& type = jsonConstant["type"].get<string>();

			Constant newConstant;
			newConstant.name = jsonConstant["name"].get<string>();
			newConstant.type = (ConstantElementType)str::ListIndex(type, elementTypeNames, static_cast<size_t>(ConstantElementType::Count));
			if (newConstant.type == ConstantElementType::Count)
			{
				ALIMER_LOGERROR("Unknown element type {} in constant buffer JSON", type);
				break;
			}
			if (jsonConstant["numElements"].is_number())
				newConstant.numElements = jsonConstant["numElements"].get<uint32_t>();

			constants.push_back(newConstant);
		}

		if (!Define(constants, hostVisible))
			return false;

		for (uint32_t i = 0; i < constants.size() && i < jsonConstants.size(); ++i)
		{
			const json& value = jsonConstants[i]["value"];
			AttributeType attrType = elementToAttribute[ecast(constants[i].type)];

			if (value.is_array())
			{
				for (uint32_t j = 0; j < value.size(); ++j)
				{
					Attribute::FromJSON(attrType, const_cast<void*>(GetConstantValue(i, j)), value[j]);
				}
			}
			else
				Attribute::FromJSON(attrType, const_cast<void*>(GetConstantValue(i)), value);
		}

		_dirty = true;
		Apply();
		return true;
	}

	void ConstantBuffer::SaveJSON(json& dest)
	{
		dest.object();
		dest["usage"] = resourceUsageNames[ecast(_resourceUsage)];
		dest["constants"].array();

		for (uint32_t i = 0; i < _constants.size(); ++i)
		{
			const Constant& constant = _constants[i];
			AttributeType attrType = elementToAttribute[ecast(constant.type)];

			json jsonConstant = json::object();
			jsonConstant["name"] = constant.name;
			jsonConstant["type"] = elementTypeNames[ecast(constant.type)];
			if (constant.numElements != 1)
				jsonConstant["numElements"] = (int)constant.numElements;

			if (constant.numElements == 1)
			{
				Attribute::ToJSON(attrType, jsonConstant["value"], GetConstantValue(i));
			}
			else
			{
				jsonConstant["value"] = json::array();
				for (uint32_t j = 0; j < constant.numElements; ++j)
					Attribute::ToJSON(attrType, jsonConstant["value"][j], GetConstantValue(i, j));
			}

			dest["constants"].push_back(jsonConstant);
		}
	}

	bool ConstantBuffer::Define(const std::vector<Constant>& srcConstants, bool hostVisible)
	{
		return Define(
			static_cast<uint32_t>(srcConstants.size()), 
			srcConstants.empty() ? nullptr : srcConstants.data(),
			hostVisible);
	}

	bool ConstantBuffer::Define(uint32_t numConstants, const Constant* srcConstants, bool hostVisible)
	{
		ALIMER_PROFILE(DefineConstantBuffer);

		Release();

		if (!numConstants)
		{
			ALIMER_LOGERROR("Can not define constant buffer with no constants");
			return false;
		}

		_constants.clear();
		_size = 0;
		_stride = 0;
		_resourceUsage = hostVisible ? ResourceUsage::Dynamic : ResourceUsage::Default;

		while (numConstants--)
		{
			Constant newConstant;
			newConstant.type = srcConstants->type;
			newConstant.name = srcConstants->name;
			newConstant.numElements = srcConstants->numElements;
			newConstant.elementSize = GetConstantElementSize(newConstant.type);
			// If element crosses 16 byte boundary or is larger than 16 bytes, align to next 16 bytes
			if ((newConstant.elementSize <= 16 && ((_size + newConstant.elementSize - 1) >> 4) != (_size >> 4)) ||
				(newConstant.elementSize > 16 && (_size & 15)))
			{
				_size += 16 - (_size & 15);
			}

			newConstant.offset = _size;
			_constants.push_back(newConstant);

			_size += newConstant.elementSize * newConstant.numElements;
			++srcConstants;
		}

		// Align the final buffer size to a multiple of 16 bytes
		if (_size & 15)
		{
			_size += 16 - (_size & 15);
		}

		return Create(true, nullptr);
	}

	bool ConstantBuffer::SetConstant(uint32_t index, const void* data, uint32_t numElements)
	{
		if (index >= _constants.size())
			return false;

		const Constant& constant = _constants[index];

		if (!numElements || numElements > constant.numElements)
			numElements = constant.numElements;

		memcpy(_shadowData.get() + constant.offset, data, numElements * constant.elementSize);
		_dirty = true;
		return true;
	}

	bool ConstantBuffer::SetConstant(const string& name, const void* data, uint32_t numElements)
	{
		for (uint32_t i = 0; i < _constants.size(); ++i)
		{
			if (_constants[i].name == name)
			{
				return SetConstant(i, data, numElements);
			}
		}

		return false;
	}

	bool ConstantBuffer::Apply()
	{
		return _dirty ? SetData(_shadowData.get()) : true;
	}

	bool ConstantBuffer::SetData(const void* data, bool copyToShadow)
	{
		if (copyToShadow)
		{
			memcpy(_shadowData.get(), data, _size);
		}

		_dirty = false;
		return Buffer::SetData(0, _size, data);
	}

	uint32_t ConstantBuffer::FindConstantIndex(const string& name) const
	{
		for (uint32_t i = 0; i < _constants.size(); ++i)
		{
			if (_constants[i].name == name)
				return i;
		}

		return NPOS;
	}

	const void* ConstantBuffer::GetConstantValue(uint32_t index, uint32_t elementIndex) const
	{
		return (index < _constants.size() && elementIndex < _constants[index].numElements) ? _shadowData.get() +
			_constants[index].offset + elementIndex * _constants[index].elementSize : nullptr;
	}

	const void* ConstantBuffer::GetConstantValue(const string& name, uint32_t elementIndex) const
	{
		return GetConstantValue(FindConstantIndex(name), elementIndex);
	}
}
