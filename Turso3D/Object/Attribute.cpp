// For conditions of distribution and use, see copyright notice in License.txt

#include "../IO/JSONValue.h"
#include "../Math/Quaternion.h"
#include "Attribute.h"

#include "../Debug/DebugNew.h"

namespace Turso3D
{

AttributeAccessor::~AttributeAccessor()
{
}

Attribute::Attribute(const char* name_, AttributeAccessor* accessor_, const char** enumNames_) :
    name(name_),
    accessor(accessor_),
    enumNames(enumNames_)
{
}

Attribute::~Attribute()
{
}

void Attribute::FromValue(Serializable* instance, const void* source) const
{
    accessor->Set(instance, source);
}

void Attribute::ToValue(Serializable* instance, void* dest) const
{
    accessor->Get(instance, dest);
}

void Attribute::Skip(AttributeType type, Deserializer& source)
{
    switch (type)
    {
    case ATTR_BOOL:
        source.Read<bool>();
        break;

    case ATTR_INT:
        source.Read<int>();
        break;

    case ATTR_UNSIGNED:
        source.Read<unsigned>();
        break;
        
    case ATTR_FLOAT:
        source.Read<float>();
        break;

    case ATTR_STRING:
        source.Read<String>();
        break;

    case ATTR_VECTOR3:
        source.Read<Vector3>();
        break;
        
    case ATTR_QUATERNION:
        source.Read<Quaternion>();
        break;
        
    default:
        break;
    }
}

template<> void AttributeImpl<bool>::FromJSON(Serializable* instance, const JSONValue& source) const
{
    SetValue(instance, source.GetBool());
}

template<> void AttributeImpl<int>::FromJSON(Serializable* instance, const JSONValue& source) const
{
    SetValue(instance, (int)source.GetNumber());
}

template<> void AttributeImpl<unsigned>::FromJSON(Serializable* instance, const JSONValue& source) const
{
    SetValue(instance, (unsigned)source.GetNumber());
}

template<> void AttributeImpl<float>::FromJSON(Serializable* instance, const JSONValue& source) const
{
    SetValue(instance, (float)source.GetNumber());
}

template<> void AttributeImpl<String>::FromJSON(Serializable* instance, const JSONValue& source) const
{
    SetValue(instance, source.GetString());
}

template<> void AttributeImpl<Vector3>::FromJSON(Serializable* instance, const JSONValue& source) const
{
    SetValue(instance, Vector3(source.GetString()));
}

template<> void AttributeImpl<Quaternion>::FromJSON(Serializable* instance, const JSONValue& source) const
{
    SetValue(instance, Quaternion(source.GetString()));
}

template<> void AttributeImpl<bool>::ToJSON(Serializable* instance, JSONValue& dest) const
{
    dest = Value(instance);
}

template<> void AttributeImpl<int>::ToJSON(Serializable* instance, JSONValue& dest) const
{
    dest = Value(instance);
}

template<> void AttributeImpl<unsigned>::ToJSON(Serializable* instance, JSONValue& dest) const
{
    dest = Value(instance);
}

template<> void AttributeImpl<float>::ToJSON(Serializable* instance, JSONValue& dest) const
{
    dest = Value(instance);
}

template<> void AttributeImpl<String>::ToJSON(Serializable* instance, JSONValue& dest) const
{
    dest = Value(instance);
}

template<> void AttributeImpl<Vector3>::ToJSON(Serializable* instance, JSONValue& dest) const
{
    dest = Value(instance).ToString();
}

template<> void AttributeImpl<Quaternion>::ToJSON(Serializable* instance, JSONValue& dest) const
{
    dest = Value(instance).ToString();
}

template<> AttributeType AttributeImpl<bool>::Type() const
{
    return ATTR_BOOL;
}

template<> AttributeType AttributeImpl<int>::Type() const
{
    return ATTR_INT;
}

template<> AttributeType AttributeImpl<unsigned>::Type() const
{
    return ATTR_UNSIGNED;
}

template<> AttributeType AttributeImpl<float>::Type() const
{
    return ATTR_FLOAT;
}

template<> AttributeType AttributeImpl<String>::Type() const
{
    return ATTR_STRING;
}

template<> AttributeType AttributeImpl<Vector3>::Type() const
{
    return ATTR_VECTOR3;
}

template<> AttributeType AttributeImpl<Quaternion>::Type() const
{
    return ATTR_QUATERNION;
}

}
