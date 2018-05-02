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

#include "../Object/Object.h"
using namespace std;

namespace Alimer
{
	map<StringHash, Object*> Object::_subsystems;
	map<StringHash, unique_ptr<ObjectFactory> > Object::_factories;

	TypeInfo::TypeInfo(const char* typeName, const TypeInfo* baseTypeInfo) 
		: _type(typeName)
		, _typeName(typeName)
		, _baseTypeInfo(baseTypeInfo)
	{
	}

	bool TypeInfo::IsTypeOf(StringHash type) const
	{
		const TypeInfo* current = this;
		while (current)
		{
			if (current->GetType() == type)
				return true;

			current = current->GetBaseTypeInfo();
		}

		return false;
	}

	bool TypeInfo::IsTypeOf(const TypeInfo* typeInfo) const
	{
		const TypeInfo* current = this;
		while (current)
		{
			if (current == typeInfo)
				return true;

			current = current->GetBaseTypeInfo();
		}

		return false;
	}

	ObjectFactory::~ObjectFactory()
	{
	}

	void Object::SubscribeToEvent(Event& event, EventHandler* handler)
	{
		event.Subscribe(handler);
	}

	bool Object::IsInstanceOf(StringHash type) const
	{
		return GetTypeInfo()->IsTypeOf(type);
	}

	bool Object::IsInstanceOf(const TypeInfo* typeInfo) const
	{
		return GetTypeInfo()->IsTypeOf(typeInfo);
	}

	void Object::UnsubscribeFromEvent(Event& event)
	{
		event.Unsubscribe(this);
	}

	void Object::SendEvent(Event& event)
	{
		event.Send(this);
	}

	bool Object::IsSubscribedToEvent(const Event& event) const
	{
		return event.HasReceiver(this);
	}

	void Object::RegisterSubsystem(Object* subsystem)
	{
		if (!subsystem)
			return;

		_subsystems[subsystem->GetType()] = subsystem;
	}

	void Object::RemoveSubsystem(Object* subsystem)
	{
		if (!subsystem)
			return;

		auto it = _subsystems.find(subsystem->GetType());
		if (it != _subsystems.end() && it->second == subsystem)
			_subsystems.erase(it);
	}

	void Object::RemoveSubsystem(StringHash type)
	{
		_subsystems.erase(type);
	}

	Object* Object::GetSubsystem(StringHash type)
	{
		auto it = _subsystems.find(type);
		return it != _subsystems.end() ? it->second : nullptr;
	}

	void Object::RegisterFactory(ObjectFactory* factory)
	{
		if (!factory)
			return;

		_factories[factory->GetType()].reset(factory);
	}

	Object* Object::Create(StringHash type)
	{
		auto it = _factories.find(type);
		return it != _factories.end() ? it->second->Create() : nullptr;
	}

	const string& Object::GetTypeNameFromType(StringHash type)
	{
		auto it = _factories.find(type);
		return it != _factories.end() ? it->second->GetTypeName() : str::EMPTY;
	}
}
