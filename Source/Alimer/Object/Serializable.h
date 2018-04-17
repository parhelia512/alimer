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

#include "Attribute.h"
#include "Object.h"

namespace Alimer
{
	class ObjectResolver;
	using AttibuteVector = std::vector<std::shared_ptr<Attribute>>;

	/// Base class for objects with automatic serialization using attributes.
	class ALIMER_API Serializable : public Object
	{
	public:
		/// Load from binary stream. Store object ref attributes to be resolved later.
		virtual void Load(Stream& source, ObjectResolver& resolver);
		/// Save to binary stream.
		virtual void Save(Stream& dest);
		/// Load from JSON data. Optionally store object ref attributes to be resolved later.
		virtual void LoadJSON(const json& source, ObjectResolver& resolver);
		/// Save as JSON data.
		virtual void SaveJSON(json& dest);

		/// Return id for referring to the object in serialization.
		virtual uint32_t GetId() const { return 0; }

		/// Set attribute value from memory.
		void SetAttributeValue(Attribute* attr, const void* source);
		/// Copy attribute value to memory.
		void GetAttributeValue(Attribute* attr, void* dest);

		/// Set attribute value, template version. Return true if value was right type.
		template <class T> bool SetAttributeValue(Attribute* attr, const T& source)
		{
			AttributeImpl<T>* typedAttr = dynamic_cast<AttributeImpl<T>*>(attr);
			if (typedAttr)
			{
				typedAttr->SetValue(this, source);
				return true;
			}

			return false;
		}

		/// Copy attribute value, template version. Return true if value was right type.
		template <class T> bool SetAttributeValue(Attribute* attr, T& dest)
		{
			AttributeImpl<T>* typedAttr = dynamic_cast<AttributeImpl<T>*>(attr);
			if (typedAttr)
			{
				typedAttr->Value(this, dest);
				return true;
			}

			return false;
		}

		/// Return attribute value, template version.
		template <class T> T GetAttributeValue(Attribute* attr)
		{
			AttributeImpl<T>* typedAttr = dynamic_cast<AttributeImpl<T>*>(attr);
			return typedAttr ? typedAttr->GetValue(this) : T();
		}

		/// Return the attribute descriptions. Default implementation uses per-class registration.
		virtual const AttibuteVector* GetAttributes() const;
		/// Return an attribute description by name, or null if does not exist.
		std::shared_ptr<Attribute> FindAttribute(const std::string& name) const;
		/// Return an attribute description by name, or null if does not exist.
		std::shared_ptr<Attribute> FindAttribute(const char* name) const;

		/// Register a per-class attribute. If an attribute with the same name already exists, it will be replaced.
		static void RegisterAttribute(StringHash type, const std::shared_ptr<Attribute>& attr);
		/// Copy all base class attributes.
		static void CopyBaseAttributes(StringHash type, StringHash baseType);
		/// Copy one base class attribute.
		static void CopyBaseAttribute(StringHash type, StringHash baseType, const std::string& name);
		/// Skip binary data of an object's all attributes.
		static void Skip(Stream& source);

		/// Register a per-class attribute, template version. Should not be used for base class attributes unless the type is explicitly specified, as by default the attribute will be re-registered to the base class redundantly.
		template <class T, class U> static void RegisterAttribute(const char* name, U(T::*getFunction)() const, void (T::*setFunction)(U), const U& defaultValue = U(), const char** enumNames = 0)
		{
			RegisterAttribute(T::GetTypeStatic(), std::make_shared<AttributeImpl<U>>(name, new AttributeAccessorImpl<T, U>(getFunction, setFunction), defaultValue, enumNames));
		}

		/// Register a per-class attribute with reference access, template version. Should not be used for base class attributes unless the type is explicitly specified, as by default the attribute will be re-registered to the base class redundantly.
		template <class T, class U> static void RegisterRefAttribute(const char* name, const U& (T::*getFunction)() const, void (T::*setFunction)(const U&), const U& defaultValue = U(), const char** enumNames = 0)
		{
			RegisterAttribute(T::GetTypeStatic(), std::make_shared<AttributeImpl<U>>(name, new RefAttributeAccessorImpl<T, U>(getFunction, setFunction), defaultValue, enumNames));
		}

		/// Register a per-class attribute with mixed reference access, template version. Should not be used for base class attributes unless the type is explicitly specified, as by default the attribute will be re-registered to the base class redundantly.
		template <class T, class U> static void RegisterMixedRefAttribute(const char* name, U(T::*getFunction)() const, void (T::*setFunction)(const U&), const U& defaultValue = U(), const char** enumNames = 0)
		{
			RegisterAttribute(T::GetTypeStatic(), std::make_shared<AttributeImpl<U>>(name, new MixedRefAttributeAccessorImpl<T, U>(getFunction, setFunction), defaultValue, enumNames));
		}

		/// Copy all base class attributes, template version.
		template <class T, class U> static void CopyBaseAttributes()
		{
			CopyBaseAttributes(T::GetTypeStatic(), U::GetTypeStatic());
		}

		/// Copy one base class attribute, template version.
		template <class T, class U> static void CopyBaseAttribute(const std::string& name)
		{
			CopyBaseAttribute(T::GetTypeStatic(), U::GetTypeStatic(), name);
		}

	private:
		/// Per-class attributes.
		static std::map<StringHash, AttibuteVector> _classAttributes;
	};

}
