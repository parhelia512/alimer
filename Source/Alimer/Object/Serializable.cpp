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

#include "../IO/ObjectRef.h"
#include "../IO/Stream.h"
#include "ObjectResolver.h"
#include "Serializable.h"

using namespace std;

namespace Alimer
{
	map<StringHash, AttibuteVector> Serializable::_classAttributes;

	void Serializable::Load(Stream& source, ObjectResolver& resolver)
	{
		const auto* attributes = GetAttributes();
		if (!attributes)
			return; // Nothing to do

		size_t numAttrs = source.ReadVLE();
		for (size_t i = 0; i < numAttrs; ++i)
		{
			// Skip attribute if wrong type or extra data
			AttributeType type = (AttributeType)source.Read<unsigned char>();
			bool skip = true;

			if (i < attributes->size())
			{
				const shared_ptr<Attribute> attr = attributes->at(i);
				if (attr->GetType() == type)
				{
					// Store object refs to the resolver instead of immediately setting
					if (type != ATTR_OBJECTREF)
						attr->FromBinary(this, source);
					else
						resolver.StoreObjectRef(this, attr, source.Read<ObjectRef>());

					skip = false;
				}
			}

			if (skip)
				Attribute::Skip(type, source);
		}
	}

	void Serializable::Save(Stream& dest)
	{
		const auto* attributes = GetAttributes();
		if (!attributes)
			return;

		dest.WriteVLE(static_cast<uint32_t>(attributes->size()));
		for (auto it = attributes->begin(); it != attributes->end(); ++it)
		{
			const shared_ptr<Attribute> attr = *it;
			dest.WriteUByte((uint8_t)attr->GetType());
			attr->ToBinary(this, dest);
		}
	}

	void Serializable::LoadJSON(const json& source, ObjectResolver& resolver)
	{
		const auto* attributes = GetAttributes();
		if (!attributes || !source.is_object() || !source.size())
			return;

		const json& object = source;

		for (auto it = attributes->begin(); it != attributes->end(); ++it)
		{
			const shared_ptr<Attribute> attr = *it;
			auto jsonIt = object.find(attr->GetName());
			if (jsonIt != object.end())
			{
				// Store object refs to the resolver instead of immediately setting
				if (attr->GetType() != ATTR_OBJECTREF)
					attr->FromJSON(this, jsonIt.value());
				else
					resolver.StoreObjectRef(this, attr, ObjectRef(jsonIt.value().get<uint32_t>()));
			}
		}
	}

	void Serializable::SaveJSON(json& dest)
	{
		const auto* attributes = GetAttributes();
		if (!attributes)
			return;

		for (size_t i = 0; i < attributes->size(); ++i)
		{
			const shared_ptr<Attribute> attr = attributes->at(i);
			// For better readability, do not save default-valued attributes to JSON
			if (!attr->IsDefault(this))
				attr->ToJSON(this, dest[attr->GetName()]);
		}
	}

	void Serializable::SetAttributeValue(Attribute* attr, const void* source)
	{
		if (attr)
			attr->FromValue(this, source);
	}

	void Serializable::GetAttributeValue(Attribute* attr, void* dest)
	{
		if (attr)
			attr->ToValue(this, dest);
	}

	const AttibuteVector* Serializable::GetAttributes() const
	{
		auto it = _classAttributes.find(Type());
		return it != _classAttributes.end() ? &it->second : nullptr;
	}

	std::shared_ptr<Attribute> Serializable::FindAttribute(const std::string& name) const
	{
		return FindAttribute(name.c_str());
	}

	std::shared_ptr<Attribute> Serializable::FindAttribute(const char* name) const
	{
		const auto* attributes = GetAttributes();
		if (!attributes)
			return nullptr;

		for (size_t i = 0; i < attributes->size(); ++i)
		{
			const shared_ptr<Attribute> attr = attributes->at(i);
			if (attr->GetName() == name)
				return attr;
		}

		return nullptr;
	}

	void Serializable::RegisterAttribute(StringHash type, const std::shared_ptr<Attribute>& attr)
	{
		auto& attributes = _classAttributes[type];
		for (size_t i = 0; i < attributes.size(); ++i)
		{
			if (attributes[i]->GetName() == attr->GetName())
			{
				attributes.insert(attributes.begin() + i, attr);
				return;
			}
		}

		attributes.push_back(attr);
	}

	void Serializable::CopyBaseAttributes(StringHash type, StringHash baseType)
	{
		// Make sure the types are different, which may not be true if the OBJECT macro has been omitted
		if (type != baseType)
		{
			auto& attributes = _classAttributes[baseType];
			for (size_t i = 0; i < attributes.size(); ++i)
			{
				RegisterAttribute(type, attributes[i]);
			}
		}
	}

	void Serializable::CopyBaseAttribute(StringHash type, StringHash baseType, const std::string& name)
	{
		// Make sure the types are different, which may not be true if the OBJECT macro has been omitted
		if (type == baseType)
			return;

		auto& attributes = _classAttributes[baseType];
		for (size_t i = 0; i < attributes.size(); ++i)
		{
			if (attributes[i]->GetName() == name)
			{
				RegisterAttribute(type, attributes[i]);
				break;
			}
		}
	}

	void Serializable::Skip(Stream& source)
	{
		size_t numAttrs = source.ReadVLE();
		for (size_t i = 0; i < numAttrs; ++i)
		{
			AttributeType type = (AttributeType)source.Read<unsigned char>();
			Attribute::Skip(type, source);
		}
	}

}
