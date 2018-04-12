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
#include "../IO/ObjectRef.h"
#include "ObjectResolver.h"
#include "Serializable.h"

namespace Alimer
{
	void ObjectResolver::StoreObject(uint32_t oldId, Serializable* object)
	{
		if (object)
			_objects[oldId] = object;
	}

	void ObjectResolver::StoreObjectRef(Serializable* object, const std::shared_ptr<Attribute>& attr, const ObjectRef& value)
	{
		if (object && attr && attr->GetType() == ATTR_OBJECTREF)
		{
			_objectRefs.emplace_back(object, attr, value.id);
		}
	}

	void ObjectResolver::Resolve()
	{
		for (auto it = _objectRefs.begin(); it != _objectRefs.end(); ++it)
		{
			auto refIt = _objects.find(it->oldId);
			// See if we can find the referred to object
			if (refIt != end(_objects))
			{
				auto typedAttr = std::static_pointer_cast<AttributeImpl<ObjectRef>>(it->attr);
				typedAttr->SetValue(it->object, ObjectRef(refIt->second->GetId()));
			}
			else
			{
				LOGWARNING("Could not resolve object reference " + std::to_string(it->oldId));
			}
		}
	}

}
