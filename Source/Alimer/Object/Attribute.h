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

#include "../IO/Stream.h"
#include <memory>
#include <string>

namespace Alimer
{
	class JSONValue;
	class Serializable;
	class Stream;

	/// Supported attribute types.
	enum AttributeType
	{
		ATTR_BOOL = 0,
		ATTR_BYTE,
		ATTR_UNSIGNED,
		ATTR_INT,
		ATTR_INTVECTOR2,
		ATTR_INTRECT,
		ATTR_FLOAT,
		ATTR_VECTOR2,
		ATTR_VECTOR3,
		ATTR_VECTOR4,
		ATTR_QUATERNION,
		ATTR_COLOR,
		ATTR_RECT,
		ATTR_BOUNDINGBOX,
		ATTR_MATRIX3,
		ATTR_MATRIX3X4,
		ATTR_MATRIX4,
		ATTR_STRING,
		ATTR_RESOURCEREF,
		ATTR_RESOURCEREFLIST,
		ATTR_OBJECTREF,
		ATTR_JSONVALUE,
		MAX_ATTR_TYPES
	};

	/// Helper class for accessing serializable variables via getter and setter functions.
	class ALIMER_API AttributeAccessor
	{
	public:
		/// Destruct.
		virtual ~AttributeAccessor();

		/// Get the current value of the variable.
		virtual void Get(const Serializable* instance, void* dest) = 0;
		/// Set new value for the variable.
		virtual void Set(Serializable* instance, const void* source) = 0;
	};

	/// Description of an automatically serializable variable.
	class ALIMER_API Attribute
	{
	public:
		/// Construct.
		Attribute(const char* name, AttributeAccessor* accessor, const char** enumNames = 0);

		/// Deserialize from a binary stream.
		virtual void FromBinary(Serializable* instance, Stream& source) = 0;
		/// Serialize to a binary stream.
		virtual void ToBinary(Serializable* instance, Stream& dest) = 0;
		/// Deserialize from JSON.
		virtual void FromJSON(Serializable* instance, const JSONValue& source) = 0;
		/// Serialize to JSON.
		virtual void ToJSON(Serializable* instance, JSONValue& dest) = 0;
		/// Return type.
		virtual AttributeType GetType() const = 0;
		/// Return whether is default value.
		virtual bool IsDefault(Serializable* instance) = 0;

		/// Set from a value in memory.
		void FromValue(Serializable* instance, const void* source);
		/// Copy to a value in memory.
		void ToValue(Serializable* instance, void* dest);

		/// Return variable name.
		const std::string& GetName() const { return _name; }
		/// Return zero-based enum names, or null if none.
		const char** GetEnumNames() const { return _enumNames; }
		/// Return type name.
		const String& GetTypeName() const;
		/// Return byte size of the attribute data, or 0 if it can be variable.
		uint32_t GetByteSize() const;

		/// Skip binary data of an attribute.
		static void Skip(AttributeType type, Stream& source);
		/// Serialize attribute value to JSON.
		static void ToJSON(AttributeType type, JSONValue& dest, const void* source);
		/// Deserialize attribute value from JSON.
		static void FromJSON(AttributeType type, void* dest, const JSONValue& source);
		/// Return attribute type from type name.
		static AttributeType TypeFromName(const String& name);
		/// Return attribute type from type name.
		static AttributeType TypeFromName(const char* name);

	protected:
		/// Variable name.
		std::string _name;
		/// Attribute accessor.
		std::unique_ptr<AttributeAccessor> _accessor;
		/// Enum names.
		const char** _enumNames;

	private:
		/// Prevent copy construction.
		Attribute(const Attribute&) = delete;
		/// Prevent assignment.
		Attribute& operator = (const Attribute&) = delete;
	};

	/// Template implementation of an attribute description with specific type.
	template <class T> class AttributeImpl : public Attribute
	{
	public:
		/// Construct.
		AttributeImpl(const char* name, AttributeAccessor* accessor, const T& defaultValue, const char** enumNames = 0)
			: Attribute(name, accessor, enumNames)
			, _defaultValue(defaultValue)
		{
		}

		/// Deserialize from a binary stream.
		void FromBinary(Serializable* instance, Stream& source) override
		{
			T value = source.Read<T>();
			_accessor->Set(instance, &value);
		}

		/// Serialize to a binary stream.
		void ToBinary(Serializable* instance, Stream& dest) override
		{
			T value;
			_accessor->Get(instance, &value);
			dest.Write<T>(value);
		}

		/// Return whether is default value.
		bool IsDefault(Serializable* instance) override { return GetValue(instance) == _defaultValue; }

		/// Deserialize from JSON.
		void FromJSON(Serializable* instance, const JSONValue& source) override
		{
			T value;
			Attribute::FromJSON(GetType(), &value, source);
			_accessor->Set(instance, &value);
		}

		/// Serialize to JSON.
		void ToJSON(Serializable* instance, JSONValue& dest) override
		{
			T value;
			_accessor->Get(instance, &value);
			Attribute::ToJSON(GetType(), dest, &value);
		}

		/// Return type.
		AttributeType GetType() const override;

		/// Set new attribute value.
		void SetValue(Serializable* instance, const T& source) { _accessor->Set(instance, &source); }
		/// Copy current attribute value.
		void GetValue(Serializable* instance, T& dest) { _accessor->Get(instance, &dest); }

		/// Return current attribute value.
		T GetValue(Serializable* instance)
		{
			T ret;
			_accessor->Get(instance, &ret);
			return ret;
		}

		/// Return default value.
		const T& GetDefaultValue() const { return _defaultValue; }

	private:
		/// Default value.
		T _defaultValue;
	};

	/// Template implementation for accessing serializable variables.
	template <class T, class U> class AttributeAccessorImpl : public AttributeAccessor
	{
	public:
		typedef U(T::*GetFunctionPtr)() const;
		typedef void (T::*SetFunctionPtr)(U);

		/// Construct with function pointers.
		AttributeAccessorImpl(GetFunctionPtr getPtr, SetFunctionPtr setPtr) :
			get(getPtr),
			set(setPtr)
		{
			assert(get);
			assert(set);
		}

		/// Get current value of the variable.
		void Get(const Serializable* instance, void* dest) override
		{
			assert(instance);

			U& value = *(reinterpret_cast<U*>(dest));
			const T* classInstance = static_cast<const T*>(instance);
			value = (classInstance->*get)();
		}

		/// Set new value for the variable.
		void Set(Serializable* instance, const void* source) override
		{
			assert(instance);

			const U& value = *(reinterpret_cast<const U*>(source));
			T* classInstance = static_cast<T*>(instance);
			(classInstance->*set)(value);
		}

	private:
		/// Getter function pointer.
		GetFunctionPtr get;
		/// Setter function pointer.
		SetFunctionPtr set;
	};

	/// Template implementation for accessing serializable variables via functions that use references.
	template <class T, class U> class RefAttributeAccessorImpl : public AttributeAccessor
	{
	public:
		typedef const U& (T::*GetFunctionPtr)() const;
		typedef void (T::*SetFunctionPtr)(const U&);

		/// Set new value for the variable.
		RefAttributeAccessorImpl(GetFunctionPtr getPtr, SetFunctionPtr setPtr) :
			get(getPtr),
			set(setPtr)
		{
			assert(get);
			assert(set);
		}

		/// Get current value of the variable.
		void Get(const Serializable* instance, void* dest) override
		{
			assert(instance);

			U& value = *(reinterpret_cast<U*>(dest));
			const T* classPtr = static_cast<const T*>(instance);
			value = (classPtr->*get)();
		}

		/// Set new value for the variable.
		void Set(Serializable* instance, const void* source) override
		{
			assert(instance);

			const U& value = *(reinterpret_cast<const U*>(source));
			T* classPtr = static_cast<T*>(instance);
			(classPtr->*set)(value);
		}

	private:
		/// Getter function pointer.
		GetFunctionPtr get;
		/// Setter function pointer.
		SetFunctionPtr set;
	};

	/// Template implementation for accessing serializable variables via functions where the setter uses reference, but the getter does not.
	template <class T, class U> class MixedRefAttributeAccessorImpl : public AttributeAccessor
	{
	public:
		typedef U(T::*GetFunctionPtr)() const;
		typedef void (T::*SetFunctionPtr)(const U&);

		/// Set new value for the variable.
		MixedRefAttributeAccessorImpl(GetFunctionPtr getPtr, SetFunctionPtr setPtr) :
			get(getPtr),
			set(setPtr)
		{
			assert(get);
			assert(set);
		}

		/// Get current value of the variable.
		void Get(const Serializable* instance, void* dest) override
		{
			assert(instance);

			U& value = *(reinterpret_cast<U*>(dest));
			const T* classPtr = static_cast<const T*>(instance);
			value = (classPtr->*get)();
		}

		/// Set new value for the variable.
		void Set(Serializable* instance, const void* source) override
		{
			assert(instance);

			const U& value = *(reinterpret_cast<const U*>(source));
			T* classPtr = static_cast<T*>(instance);
			(classPtr->*set)(value);
		}

	private:
		/// Getter function pointer.
		GetFunctionPtr get;
		/// Setter function pointer.
		SetFunctionPtr set;
	};

}
