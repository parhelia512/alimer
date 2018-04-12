// For conditions of distribution and use, see copyright notice in License.txt

#include "../Debug/Log.h"
#include "../IO/Stream.h"
#include "../Object/ObjectResolver.h"
#include "../Resource/JSONFile.h"
#include "Scene.h"

#include <algorithm>

namespace Alimer
{
	static std::vector<SharedPtr<Node> > noChildren;

	Node::Node() :
		flags(NF_ENABLED),
		layer(LAYER_DEFAULT),
		tag(TAG_NONE),
		parent(nullptr),
		scene(nullptr),
		id(0)
	{
	}

	Node::~Node()
	{
		RemoveAllChildren();
		// At the time of destruction the node should not have a parent, or be in a scene
		assert(!parent);
		assert(!scene);
	}

	void Node::RegisterObject()
	{
		RegisterFactory<Node>();
		RegisterRefAttribute("name", &Node::Name, &Node::SetName);
		RegisterAttribute("enabled", &Node::IsEnabled, &Node::SetEnabled, true);
		RegisterAttribute("temporary", &Node::IsTemporary, &Node::SetTemporary, false);
		RegisterAttribute("layer", &Node::Layer, &Node::SetLayer, LAYER_DEFAULT);
		RegisterAttribute("tag", &Node::Tag, &Node::SetTag, TAG_NONE);
	}

	void Node::Load(Stream& source, ObjectResolver& resolver)
	{
		// Type and id has been read by the parent
		Serializable::Load(source, resolver);

		size_t numChildren = source.ReadVLE();
		for (size_t i = 0; i < numChildren; ++i)
		{
			StringHash childType(source.Read<StringHash>());
			unsigned childId = source.Read<unsigned>();
			Node* child = CreateChild(childType);
			if (child)
			{
				resolver.StoreObject(childId, child);
				child->Load(source, resolver);
			}
			else
			{
				// If child is unknown type, skip all its attributes and children
				SkipHierarchy(source);
			}
		}
	}

	void Node::Save(Stream& dest)
	{
		// Write type and ID first, followed by attributes and child nodes
		dest.Write(Type());
		dest.Write(GetId());
		Serializable::Save(dest);
		dest.WriteVLE(NumPersistentChildren());

		for (auto it = children.begin(); it != children.end(); ++it)
		{
			Node* child = *it;
			if (!child->IsTemporary())
				child->Save(dest);
		}
	}

	void Node::LoadJSON(const JSONValue& source, ObjectResolver& resolver)
	{
		// Type and id has been read by the parent
		Serializable::LoadJSON(source, resolver);

		const JSONArray& children = source["children"].GetArray();
		if (children.size())
		{
			for (auto it = children.begin(); it != children.end(); ++it)
			{
				const JSONValue& childJSON = *it;
				StringHash childType(childJSON["type"].GetString());
				unsigned childId = (unsigned)childJSON["id"].GetNumber();
				Node* child = CreateChild(childType);
				if (child)
				{
					resolver.StoreObject(childId, child);
					child->LoadJSON(childJSON, resolver);
				}
			}
		}
	}

	void Node::SaveJSON(JSONValue& dest)
	{
		dest["type"] = TypeName();
		dest["id"] = GetId();
		Serializable::SaveJSON(dest);

		if (NumPersistentChildren())
		{
			dest["children"].SetEmptyArray();
			for (auto it = children.begin(); it != children.end(); ++it)
			{
				Node* child = *it;
				if (!child->IsTemporary())
				{
					JSONValue childJSON;
					child->SaveJSON(childJSON);
					dest["children"].Push(childJSON);
				}
			}
		}
	}

	bool Node::SaveJSON(Stream& dest)
	{
		JSONFile json;
		SaveJSON(json.Root());
		return json.Save(dest);
	}

	void Node::SetName(const String& newName)
	{
		SetName(newName.CString());
	}

	void Node::SetName(const char* newName)
	{
		name = newName;
	}

	void Node::SetLayer(unsigned char newLayer)
	{
		if (layer < 32)
			layer = newLayer;
		else
			LOGERROR("Can not set layer 32 or higher");
	}

	void Node::SetLayerName(const std::string& newLayerName)
	{
		if (!scene)
			return;

		const auto& layers = scene->Layers();
		auto it = layers.find(newLayerName);
		if (it != layers.end())
			layer = it->second;
		else
			LOGERROR("Layer " + newLayerName + " not defined in the scene");
	}

	void Node::SetTag(unsigned char newTag)
	{
		tag = newTag;
	}

	void Node::SetTagName(const std::string& newTagName)
	{
		if (!scene)
			return;

		const auto& tags = scene->Tags();
		auto it = tags.find(newTagName);
		if (it != tags.end())
			tag = it->second;
		else
			LOGERROR("Tag " + newTagName + " not defined in the scene");
	}

	void Node::SetEnabled(bool enable)
	{
		SetFlag(NF_ENABLED, enable);
		OnSetEnabled(TestFlag(NF_ENABLED));
	}

	void Node::SetEnabledRecursive(bool enable)
	{
		SetEnabled(enable);
		for (Node* child : children)
		{
			child->SetEnabledRecursive(enable);
		}
	}

	void Node::SetTemporary(bool enable)
	{
		SetFlag(NF_TEMPORARY, enable);
	}

	void Node::SetParent(Node* newParent)
	{
		if (newParent)
			newParent->AddChild(this);
		else
			LOGERROR("Could not set null parent");
	}

	Node* Node::CreateChild(StringHash childType)
	{
		SharedPtr<Object> newObject = Create(childType);
		if (!newObject)
		{
			LOGERRORF("Could not create child node of unknown type %s", childType.ToString().c_str());
			return nullptr;
		}
		Node* child = dynamic_cast<Node*>(newObject.Get());
		if (!child)
		{
			LOGERROR(newObject->TypeName() + " is not a Node subclass, could not add as a child");
			return nullptr;
		}

		AddChild(child);
		return child;
	}

	Node* Node::CreateChild(StringHash childType, const String& childName)
	{
		return CreateChild(childType, childName.CString());
	}

	Node* Node::CreateChild(StringHash childType, const char* childName)
	{
		Node* child = CreateChild(childType);
		if (child)
			child->SetName(childName);
		return child;
	}

	void Node::AddChild(Node* child)
	{
		// Check for illegal or redundant parent assignment
		if (!child || child->parent == this)
			return;

		if (child == this)
		{
			LOGERROR("Attempted parenting node to self");
			return;
		}

		// Check for possible cyclic parent assignment
		Node* current = parent;
		while (current)
		{
			if (current == child)
			{
				LOGERROR("Attempted cyclic node parenting");
				return;
			}
			current = current->parent;
		}

		Node* oldParent = child->parent;
		if (oldParent)
		{
			auto it = std::find(oldParent->children.begin(), oldParent->children.end(), child);
			if (it != end(oldParent->children))
				oldParent->children.erase(it);
		}

		children.push_back(child);
		child->parent = this;
		child->OnParentSet(this, oldParent);
		if (scene)
			scene->AddNode(child);
	}

	void Node::RemoveChild(Node* child)
	{
		if (!child || child->parent != this)
			return;

		for (size_t i = 0; i < children.size(); ++i)
		{
			if (children[i] == child)
			{
				RemoveChild(i);
				break;
			}
		}
	}

	void Node::RemoveChild(size_t index)
	{
		if (index >= children.size())
			return;

		Node* child = children[index];
		// Detach from both the parent and the scene (removes id assignment)
		child->parent = nullptr;
		child->OnParentSet(this, nullptr);
		if (scene)
			scene->RemoveNode(child);
		children.erase(children.begin() + index);
	}

	void Node::RemoveAllChildren()
	{
		for (auto it = children.begin(); it != children.end(); ++it)
		{
			Node* child = *it;
			child->parent = nullptr;
			child->OnParentSet(this, nullptr);
			if (scene)
				scene->RemoveNode(child);
			it->Reset();
		}

		children.clear();
	}

	void Node::RemoveSelf()
	{
		if (parent)
			parent->RemoveChild(this);
		else
			delete this;
	}

	const String& Node::LayerName() const
	{
		if (!scene)
			return String::EMPTY;

		const auto& layerNames = scene->LayerNames();
		return layer < layerNames.size() ? layerNames[layer] : String::EMPTY;
	}

	const String& Node::TagName() const
	{
		if (!scene)
			return String::EMPTY;

		const auto& tagNames = scene->TagNames();
		return tag < tagNames.size() ? tagNames[layer] : String::EMPTY;
	}

	size_t Node::NumPersistentChildren() const
	{
		size_t ret = 0;

		for (auto it = children.begin(); it != children.end(); ++it)
		{
			Node* child = *it;
			if (!child->IsTemporary())
				++ret;
		}

		return ret;
	}

	void Node::AllChildren(std::vector<Node*>& result) const
	{
		for (auto it = children.begin(); it != children.end(); ++it)
		{
			Node* child = *it;
			result.push_back(child);
			child->AllChildren(result);
		}
	}

	Node* Node::FindChild(const String& childName, bool recursive) const
	{
		return FindChild(childName.CString(), recursive);
	}

	Node* Node::FindChild(const char* childName, bool recursive) const
	{
		for (auto it = children.begin(); it != children.end(); ++it)
		{
			Node* child = *it;
			if (child->name == childName)
				return child;
			else if (recursive && child->children.size())
			{
				Node* childResult = child->FindChild(childName, recursive);
				if (childResult)
					return childResult;
			}
		}

		return nullptr;
	}

	Node* Node::FindChild(StringHash childType, bool recursive) const
	{
		for (auto it = children.begin(); it != children.end(); ++it)
		{
			Node* child = *it;
			if (child->Type() == childType)
				return child;
			else if (recursive && child->children.size())
			{
				Node* childResult = child->FindChild(childType, recursive);
				if (childResult)
					return childResult;
			}
		}

		return nullptr;
	}

	Node* Node::FindChild(StringHash childType, const String& childName, bool recursive) const
	{
		return FindChild(childType, childName.CString(), recursive);
	}

	Node* Node::FindChild(StringHash childType, const char* childName, bool recursive) const
	{
		for (auto it = children.begin(); it != children.end(); ++it)
		{
			Node* child = *it;
			if (child->Type() == childType && child->name == childName)
				return child;
			else if (recursive && child->children.size())
			{
				Node* childResult = child->FindChild(childType, childName, recursive);
				if (childResult)
					return childResult;
			}
		}

		return nullptr;
	}

	Node* Node::FindChildByLayer(unsigned layerMask, bool recursive) const
	{
		for (auto it = children.begin(); it != children.end(); ++it)
		{
			Node* child = *it;
			if (child->LayerMask() && layerMask)
				return child;
			else if (recursive && child->children.size())
			{
				Node* childResult = child->FindChildByLayer(layerMask, recursive);
				if (childResult)
					return childResult;
			}
		}

		return nullptr;
	}

	Node* Node::FindChildByTag(unsigned char tag_, bool recursive) const
	{
		for (auto it = children.begin(); it != children.end(); ++it)
		{
			Node* child = *it;
			if (child->tag == tag_)
				return child;
			else if (recursive && child->children.size())
			{
				Node* childResult = child->FindChildByTag(tag_, recursive);
				if (childResult)
					return childResult;
			}
		}

		return nullptr;
	}

	Node* Node::FindChildByTag(const String& tagName, bool recursive) const
	{
		return FindChildByTag(tagName.CString(), recursive);
	}

	Node* Node::FindChildByTag(const char* tagName, bool recursive) const
	{
		for (auto it = children.begin(); it != children.end(); ++it)
		{
			Node* child = *it;
			if (!String::Compare(child->TagName().CString(), tagName))
				return child;
			else if (recursive && child->children.size())
			{
				Node* childResult = child->FindChildByTag(tagName, recursive);
				if (childResult)
					return childResult;
			}
		}

		return nullptr;
	}

	void Node::FindChildren(Vector<Node*>& result, StringHash childType, bool recursive) const
	{
		for (auto it = children.begin(); it != children.end(); ++it)
		{
			Node* child = *it;
			if (child->Type() == childType)
				result.Push(child);
			if (recursive && child->children.size())
				child->FindChildren(result, childType, recursive);
		}
	}

	void Node::FindChildrenByLayer(Vector<Node*>& result, unsigned layerMask, bool recursive) const
	{
		for (auto it = children.begin(); it != children.end(); ++it)
		{
			Node* child = *it;
			if (child->LayerMask() & layerMask)
				result.Push(child);
			if (recursive && child->children.size())
				child->FindChildrenByLayer(result, layerMask, recursive);
		}
	}

	void Node::FindChildrenByTag(Vector<Node*>& result, unsigned char tag_, bool recursive) const
	{
		for (auto it = children.begin(); it != children.end(); ++it)
		{
			Node* child = *it;
			if (child->tag == tag_)
				result.Push(child);
			if (recursive && child->children.size())
				child->FindChildrenByTag(result, tag_, recursive);
		}
	}

	void Node::FindChildrenByTag(Vector<Node*>& result, const String& tagName, bool recursive) const
	{
		FindChildrenByTag(result, tagName.CString(), recursive);
	}

	void Node::FindChildrenByTag(Vector<Node*>& result, const char* tagName, bool recursive) const
	{
		for (auto it = children.begin(); it != children.end(); ++it)
		{
			Node* child = *it;
			if (!String::Compare(child->TagName().CString(), tagName))
				result.Push(child);
			if (recursive && child->children.size())
				child->FindChildrenByTag(result, tagName, recursive);
		}
	}

	void Node::SetScene(Scene* newScene)
	{
		Scene* oldScene = scene;
		scene = newScene;
		OnSceneSet(scene, oldScene);
	}

	void Node::SetId(unsigned newId)
	{
		id = newId;
	}

	void Node::SkipHierarchy(Stream& source)
	{
		Serializable::Skip(source);

		size_t numChildren = source.ReadVLE();
		for (size_t i = 0; i < numChildren; ++i)
		{
			source.Read<StringHash>(); // StringHash childType
			source.Read<unsigned>(); // unsigned childId
			SkipHierarchy(source);
		}
	}

	void Node::OnParentSet(Node*, Node*)
	{
	}

	void Node::OnSceneSet(Scene*, Scene*)
	{
	}

	void Node::OnSetEnabled(bool)
	{
	}

}
