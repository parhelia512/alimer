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
#include "../IO/JSONValue.h"
#include "../Object/Attribute.h"
#include "ConstantBuffer.h"
#include "GraphicsDefs.h"

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

	bool ConstantBuffer::LoadJSON(const JSONValue& source)
	{
		ResourceUsage usage_ = USAGE_DEFAULT;
		if (source.Contains("usage"))
			usage_ = (ResourceUsage)String::ListIndex(source["usage"].GetString(), resourceUsageNames, USAGE_DEFAULT);

		std::vector<Constant> constants;

		const JSONValue& jsonConstants = source["constants"];
		for (size_t i = 0; i < jsonConstants.Size(); ++i)
		{
			const JSONValue& jsonConstant = jsonConstants[i];
			const String& type = jsonConstant["type"].GetString();

			Constant newConstant;
			newConstant.name = jsonConstant["name"].GetString();
			newConstant.type = (ElementType)String::ListIndex(type, elementTypeNames, MAX_ELEMENT_TYPES);
			if (newConstant.type == MAX_ELEMENT_TYPES)
			{
				LOGERRORF("Unknown element type %s in constant buffer JSON", type.CString());
				break;
			}
			if (jsonConstant.Contains("numElements"))
				newConstant.numElements = (int)jsonConstant["numElements"].GetNumber();

			constants.push_back(newConstant);
		}

		if (!Define(usage_, constants))
			return false;

		for (uint32_t i = 0; i < constants.size() && i < jsonConstants.Size(); ++i)
		{
			const JSONValue& value = jsonConstants[i]["value"];
			AttributeType attrType = elementToAttribute[constants[i].type];

			if (value.IsArray())
			{
				for (size_t j = 0; j < value.Size(); ++j)
				{
					Attribute::FromJSON(attrType, const_cast<void*>(GetConstantValue(i, j)), value[j]);
				}
			}
			else
				Attribute::FromJSON(attrType, const_cast<void*>(GetConstantValue(i)), value);
		}

		dirty = true;
		Apply();
		return true;
	}

	void ConstantBuffer::SaveJSON(JSONValue& dest)
	{
		dest.SetEmptyObject();
		dest["usage"] = resourceUsageNames[usage];
		dest["constants"].SetEmptyArray();

		for (size_t i = 0; i < _constants.size(); ++i)
		{
			const Constant& constant = _constants[i];
			AttributeType attrType = elementToAttribute[constant.type];

			JSONValue jsonConstant;
			jsonConstant["name"] = constant.name;
			jsonConstant["type"] = elementTypeNames[constant.type];
			if (constant.numElements != 1)
				jsonConstant["numElements"] = (int)constant.numElements;

			if (constant.numElements == 1)
			{
				Attribute::ToJSON(attrType, jsonConstant["value"], GetConstantValue(i));
			}
			else
			{
				jsonConstant["value"].Resize(constant.numElements);
				for (size_t j = 0; j < constant.numElements; ++j)
					Attribute::ToJSON(attrType, jsonConstant["value"][j], GetConstantValue(i, j));
			}

			dest["constants"].Push(jsonConstant);
		}
	}

	bool ConstantBuffer::Define(ResourceUsage usage_, const std::vector<Constant>& srcConstants)
	{
		return Define(
			usage_, 
			static_cast<uint32_t>(srcConstants.size()), 
			srcConstants.empty() ? nullptr : srcConstants.data());
	}

	bool ConstantBuffer::Define(ResourceUsage usage_, uint32_t numConstants, const Constant* srcConstants)
	{
		ALIMER_PROFILE(DefineConstantBuffer);

		Release();

		if (!numConstants)
		{
			LOGERROR("Can not define constant buffer with no constants");
			return false;
		}
		if (usage_ == USAGE_RENDERTARGET)
		{
			LOGERROR("Rendertarget usage is illegal for constant buffers");
			return false;
		}

		_constants.clear();
		_byteSize = 0;
		usage = usage_;

		while (numConstants--)
		{
			if (srcConstants->type == ELEM_UBYTE4)
			{
				LOGERROR("UBYTE4 type is not supported in constant buffers");
				_constants.clear();
				_byteSize = 0;
				return false;
			}

			Constant newConstant;
			newConstant.type = srcConstants->type;
			newConstant.name = srcConstants->name;
			newConstant.numElements = srcConstants->numElements;
			newConstant.elementSize = elementSizes[newConstant.type];
			// If element crosses 16 byte boundary or is larger than 16 bytes, align to next 16 bytes
			if ((newConstant.elementSize <= 16 && ((_byteSize + newConstant.elementSize - 1) >> 4) != (_byteSize >> 4)) ||
				(newConstant.elementSize > 16 && (_byteSize & 15)))
				_byteSize += 16 - (_byteSize & 15);
			newConstant.offset = _byteSize;
			_constants.push_back(newConstant);

			_byteSize += newConstant.elementSize * newConstant.numElements;
			++srcConstants;
		}

		// Align the final buffer size to a multiple of 16 bytes
		if (_byteSize & 15)
			_byteSize += 16 - (_byteSize & 15);

		_shadowData.reset(new uint8_t[_byteSize]);

		if (usage != USAGE_IMMUTABLE)
			return Create();

		return true;
	}

	bool ConstantBuffer::SetConstant(uint32_t index, const void* data, uint32_t numElements)
	{
		if (index >= _constants.size())
			return false;

		const Constant& constant = _constants[index];

		if (!numElements || numElements > constant.numElements)
			numElements = constant.numElements;

		memcpy(_shadowData.get() + constant.offset, data, numElements * constant.elementSize);
		dirty = true;
		return true;
	}

	bool ConstantBuffer::SetConstant(const String& name, const void* data, uint32_t numElements)
	{
		return SetConstant(name.CString(), data, numElements);
	}

	bool ConstantBuffer::SetConstant(const char* name, const void* data, uint32_t numElements)
	{
		for (size_t i = 0; i < _constants.size(); ++i)
		{
			if (_constants[i].name == name)
				return SetConstant(i, data, numElements);
		}

		return false;
	}

	bool ConstantBuffer::Apply()
	{
		return dirty ? SetData(_shadowData.get()) : true;
	}

	size_t ConstantBuffer::FindConstantIndex(const String& name) const
	{
		return FindConstantIndex(name.CString());
	}

	size_t ConstantBuffer::FindConstantIndex(const char* name) const
	{
		for (size_t i = 0; i < _constants.size(); ++i)
		{
			if (_constants[i].name == name)
				return i;
		}

		return static_cast<uint32_t>(-1);
	}

	const void* ConstantBuffer::GetConstantValue(size_t index, size_t elementIndex) const
	{
		return (index < _constants.size() && elementIndex < _constants[index].numElements) ? _shadowData.get() +
			_constants[index].offset + elementIndex * _constants[index].elementSize : nullptr;
	}

	const void* ConstantBuffer::GetConstantValue(const String& name, size_t elementIndex) const
	{
		return GetConstantValue(FindConstantIndex(name), elementIndex);
	}

	const void* ConstantBuffer::GetConstantValue(const char* name, size_t elementIndex) const
	{
		return GetConstantValue(FindConstantIndex(name), elementIndex);
	}
}
