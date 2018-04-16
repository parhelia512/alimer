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

#include "../IO/JSONValue.h"
#include "../IO/ObjectRef.h"
#include "../IO/ResourceRef.h"
#include "../Math/BoundingBox.h"
#include "../Math/Color.h"
#include "../Math/IntRect.h"
#include "../Math/Matrix3x4.h"
#include "Attribute.h"

namespace Alimer
{
	static const String _typeNames[] =
	{
		"bool",
		"byte",
		"unsigned",
		"int",
		"IntVector2",
		"IntRect",
		"float",
		"Vector2",
		"Vector3",
		"Vector4",
		"Quaternion",
		"Color",
		"Rect",
		"BoundingBox",
		"Matrix3",
		"Matrix3x4",
		"Matrix4",
		"String",
		"ResourceRef",
		"ResourceRefList",
		"ObjectRef",
		"JSONValue",
		""
	};

	static const uint32_t _byteSizes[] =
	{
		sizeof(bool),
		sizeof(unsigned char),
		sizeof(unsigned),
		sizeof(int),
		sizeof(IntVector2),
		sizeof(IntRect),
		sizeof(float),
		sizeof(Vector2),
		sizeof(Vector3),
		sizeof(Vector4),
		sizeof(Quaternion),
		sizeof(Color),
		sizeof(Rect),
		sizeof(BoundingBox),
		sizeof(Matrix3),
		sizeof(Matrix3x4),
		sizeof(Matrix4),
		0,
		0,
		0,
		sizeof(unsigned),
		0,
		0
	};

	AttributeAccessor::~AttributeAccessor()
	{
	}

	Attribute::Attribute(const char* name, AttributeAccessor* accessor, const char** enumNames)
		: _name(name)
		, _accessor(accessor)
		, _enumNames(enumNames)
	{
	}

	void Attribute::FromValue(Serializable* instance, const void* source)
	{
		_accessor->Set(instance, source);
	}

	void Attribute::ToValue(Serializable* instance, void* dest)
	{
		_accessor->Get(instance, dest);
	}

	void Attribute::Skip(AttributeType type, Stream& source)
	{
		if (_byteSizes[type])
		{
			source.Seek(source.Position() + _byteSizes[type]);
			return;
		}

		switch (type)
		{
		case ATTR_STRING:
			source.Read<String>();
			break;

		case ATTR_RESOURCEREF:
			source.Read<ResourceRef>();
			break;

		case ATTR_RESOURCEREFLIST:
			source.Read<ResourceRefList>();
			break;

		case ATTR_OBJECTREF:
			source.Read<ObjectRef>();
			break;

		case ATTR_JSONVALUE:
			source.Read<JSONValue>();
			break;

		default:
			break;
		}
	}

	const String& Attribute::GetTypeName() const
	{
		return _typeNames[GetType()];
	}

	uint32_t Attribute::GetByteSize() const
	{
		return _byteSizes[GetType()];
	}

	void Attribute::FromJSON(AttributeType type, void* dest, const JSONValue& source)
	{
		switch (type)
		{
		case ATTR_BOOL:
			*(reinterpret_cast<bool*>(dest)) = source.GetBool();
			break;

		case ATTR_BYTE:
			*(reinterpret_cast<unsigned char*>(dest)) = (unsigned char)source.GetNumber();
			break;

		case ATTR_UNSIGNED:
			*(reinterpret_cast<unsigned*>(dest)) = (unsigned)source.GetNumber();
			break;

		case ATTR_INT:
			*(reinterpret_cast<int*>(dest)) = (int)source.GetNumber();
			break;

		case ATTR_INTVECTOR2:
			reinterpret_cast<IntVector2*>(dest)->FromString(source.GetString());
			break;

		case ATTR_INTRECT:
			reinterpret_cast<IntRect*>(dest)->FromString(source.GetStdString());
			break;

		case ATTR_FLOAT:
			*(reinterpret_cast<float*>(dest)) = (float)source.GetNumber();
			break;

		case ATTR_VECTOR2:
			reinterpret_cast<Vector2*>(dest)->FromString(source.GetString());
			break;

		case ATTR_VECTOR3:
			reinterpret_cast<Vector3*>(dest)->FromString(source.GetString());
			break;

		case ATTR_VECTOR4:
			reinterpret_cast<Vector4*>(dest)->FromString(source.GetString());
			break;

		case ATTR_QUATERNION:
			reinterpret_cast<Vector4*>(dest)->FromString(source.GetString());
			break;

		case ATTR_COLOR:
			reinterpret_cast<Color*>(dest)->FromString(source.GetString());
			break;

		case ATTR_RECT:
			reinterpret_cast<Rect*>(dest)->FromString(source.GetString());
			break;

		case ATTR_BOUNDINGBOX:
			reinterpret_cast<Rect*>(dest)->FromString(source.GetString());
			break;

		case ATTR_MATRIX3:
			reinterpret_cast<Matrix3*>(dest)->FromString(source.GetString());
			break;

		case ATTR_MATRIX3X4:
			reinterpret_cast<Matrix3x4*>(dest)->FromString(source.GetString());
			break;

		case ATTR_MATRIX4:
			reinterpret_cast<Matrix4*>(dest)->FromString(source.GetString());
			break;

		case ATTR_STRING:
			*(reinterpret_cast<String*>(dest)) = source.GetString();
			break;

		case ATTR_RESOURCEREF:
			reinterpret_cast<ResourceRef*>(dest)->FromString(source.GetStdString());
			break;

		case ATTR_RESOURCEREFLIST:
			reinterpret_cast<ResourceRefList*>(dest)->FromString(source.GetStdString());
			break;

		case ATTR_OBJECTREF:
			reinterpret_cast<ObjectRef*>(dest)->id = (unsigned)source.GetNumber();
			break;

		case ATTR_JSONVALUE:
			*(reinterpret_cast<JSONValue*>(dest)) = source;
			break;

		default:
			break;
		}
	}

	void Attribute::ToJSON(AttributeType type, JSONValue& dest, const void* source)
	{
		switch (type)
		{
		case ATTR_BOOL:
			dest = *(reinterpret_cast<const bool*>(source));
			break;

		case ATTR_BYTE:
			dest = *(reinterpret_cast<const unsigned char*>(source));
			break;

		case ATTR_UNSIGNED:
			dest = *(reinterpret_cast<const unsigned*>(source));
			break;

		case ATTR_INT:
			dest = *(reinterpret_cast<const int*>(source));
			break;

		case ATTR_INTVECTOR2:
			dest = reinterpret_cast<const IntVector2*>(source)->ToString();
			break;

		case ATTR_INTRECT:
			dest = reinterpret_cast<const IntRect*>(source)->ToString();
			break;

		case ATTR_FLOAT:
			dest = *(reinterpret_cast<const float*>(source));
			break;

		case ATTR_VECTOR2:
			dest = reinterpret_cast<const Vector2*>(source)->ToString();
			break;

		case ATTR_VECTOR3:
			dest = reinterpret_cast<const Vector3*>(source)->ToString();
			break;

		case ATTR_VECTOR4:
			dest = reinterpret_cast<const Vector4*>(source)->ToString();
			break;

		case ATTR_QUATERNION:
			dest = reinterpret_cast<const Quaternion*>(source)->ToString();
			break;

		case ATTR_COLOR:
			dest = reinterpret_cast<const Color*>(source)->ToString();
			break;

		case ATTR_RECT:
			dest = reinterpret_cast<const Rect*>(source)->ToString();
			break;

		case ATTR_BOUNDINGBOX:
			dest = reinterpret_cast<const BoundingBox*>(source)->ToString();
			break;

		case ATTR_MATRIX3:
			dest = reinterpret_cast<const Matrix3*>(source)->ToString();
			break;

		case ATTR_MATRIX3X4:
			dest = reinterpret_cast<const Matrix3x4*>(source)->ToString();
			break;

		case ATTR_MATRIX4:
			dest = reinterpret_cast<const Matrix4*>(source)->ToString();
			break;

		case ATTR_STRING:
			dest = *(reinterpret_cast<const String*>(source));
			break;

		case ATTR_RESOURCEREF:
			dest = reinterpret_cast<const ResourceRef*>(source)->ToString();
			break;

		case ATTR_RESOURCEREFLIST:
			dest = reinterpret_cast<const ResourceRefList*>(source)->ToString();
			break;

		case ATTR_OBJECTREF:
			dest = reinterpret_cast<const ObjectRef*>(source)->id;
			break;

		case ATTR_JSONVALUE:
			dest = *(reinterpret_cast<const JSONValue*>(source));
			break;

		default:
			break;
		}
	}

	AttributeType Attribute::TypeFromName(const String& name)
	{
		return (AttributeType)String::ListIndex(name, &_typeNames[0], MAX_ATTR_TYPES);
	}

	AttributeType Attribute::TypeFromName(const char* name)
	{
		return (AttributeType)String::ListIndex(name, &_typeNames[0], MAX_ATTR_TYPES);
	}

	template<> ALIMER_API AttributeType AttributeImpl<bool>::GetType() const
	{
		return ATTR_BOOL;
	}

	template<> ALIMER_API AttributeType AttributeImpl<int>::GetType() const
	{
		return ATTR_INT;
	}

	template<> ALIMER_API AttributeType AttributeImpl<unsigned>::GetType() const
	{
		return ATTR_UNSIGNED;
	}

	template<> ALIMER_API AttributeType AttributeImpl<uint8_t>::GetType() const
	{
		return ATTR_BYTE;
	}

	template<> ALIMER_API AttributeType AttributeImpl<float>::GetType() const
	{
		return ATTR_FLOAT;
	}

	template<> ALIMER_API AttributeType AttributeImpl<std::string>::GetType() const
	{
		return ATTR_STRING;
	}

	template<> ALIMER_API AttributeType AttributeImpl<Vector2>::GetType() const
	{
		return ATTR_VECTOR2;
	}

	template<> ALIMER_API AttributeType AttributeImpl<Vector3>::GetType() const
	{
		return ATTR_VECTOR3;
	}

	template<> ALIMER_API AttributeType AttributeImpl<Vector4>::GetType() const
	{
		return ATTR_VECTOR4;
	}

	template<> ALIMER_API AttributeType AttributeImpl<Quaternion>::GetType() const
	{
		return ATTR_QUATERNION;
	}

	template<> ALIMER_API AttributeType AttributeImpl<Color>::GetType() const
	{
		return ATTR_COLOR;
	}

	template<> ALIMER_API AttributeType AttributeImpl<BoundingBox>::GetType() const
	{
		return ATTR_BOUNDINGBOX;
	}

	template<> ALIMER_API AttributeType AttributeImpl<ResourceRef>::GetType() const
	{
		return ATTR_RESOURCEREF;
	}

	template<> ALIMER_API AttributeType AttributeImpl<ResourceRefList>::GetType() const
	{
		return ATTR_RESOURCEREFLIST;
	}

	template<> ALIMER_API AttributeType AttributeImpl<ObjectRef>::GetType() const
	{
		return ATTR_OBJECTREF;
	}

	template<> ALIMER_API AttributeType AttributeImpl<JSONValue>::GetType() const
	{
		return ATTR_JSONVALUE;
	}

}
