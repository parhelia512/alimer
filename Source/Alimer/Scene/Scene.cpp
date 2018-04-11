// For conditions of distribution and use, see copyright notice in License.txt

#include "../Debug/Log.h"
#include "../Debug/Profiler.h"
#include "../IO/Stream.h"
#include "../Object/ObjectResolver.h"
#include "../Resource/JSONFile.h"
#include "Scene.h"
#include "SpatialNode.h"

#include "../Debug/DebugNew.h"

namespace Alimer
{

	Scene::Scene() :
		nextNodeId(1)
	{
		// Register self to allow finding by ID
		AddNode(this);

		DefineLayer(LAYER_DEFAULT, "Default");
		DefineTag(TAG_NONE, "None");
	}

	Scene::~Scene()
	{
		// Node destructor will also remove children. But at that point the node<>id maps have been destroyed 
		// so must tear down the scene tree already here
		RemoveAllChildren();
		RemoveNode(this);
		assert(nodes.empty());
	}

	void Scene::RegisterObject()
	{
		RegisterFactory<Scene>();
		CopyBaseAttributes<Scene, Node>();
		RegisterAttribute("layerNames", &Scene::LayerNamesAttr, &Scene::SetLayerNamesAttr);
		RegisterAttribute("tagNames", &Scene::TagNamesAttr, &Scene::SetTagNamesAttr);
	}

	void Scene::Save(Stream& dest)
	{
		ALIMER_PROFILE(SaveScene);

		LOGINFO("Saving scene to " + dest.Name());

		dest.WriteFileID("SCNE");
		Node::Save(dest);
	}

	bool Scene::Load(Stream& source)
	{
		ALIMER_PROFILE(LoadScene);

		LOGINFO("Loading scene from " + source.Name());

		std::string fileId = source.ReadFileID();
		if (fileId != "SCNE")
		{
			LOGERROR("File is not a binary scene file");
			return false;
		}

		StringHash ownType = source.Read<StringHash>();
		unsigned ownId = source.Read<unsigned>();
		if (ownType != TypeStatic())
		{
			LOGERROR("Mismatching type of scene root node in scene file");
			return false;
		}

		Clear();

		ObjectResolver resolver;
		resolver.StoreObject(ownId, this);
		Node::Load(source, resolver);
		resolver.Resolve();

		return true;
	}

	bool Scene::LoadJSON(const JSONValue& source)
	{
		ALIMER_PROFILE(LoadSceneJSON);

		StringHash ownType(source["type"].GetString());
		unsigned ownId = (unsigned)source["id"].GetNumber();

		if (ownType != TypeStatic())
		{
			LOGERROR("Mismatching type of scene root node in scene file");
			return false;
		}

		Clear();

		ObjectResolver resolver;
		resolver.StoreObject(ownId, this);
		Node::LoadJSON(source, resolver);
		resolver.Resolve();

		return true;
	}

	bool Scene::LoadJSON(Stream& source)
	{
		LOGINFO("Loading scene from " + source.Name());

		JSONFile json;
		bool success = json.Load(source);
		LoadJSON(json.Root());
		return success;
	}

	bool Scene::SaveJSON(Stream& dest)
	{
		ALIMER_PROFILE(SaveSceneJSON);

		LOGINFO("Saving scene to " + dest.Name());

		JSONFile json;
		Node::SaveJSON(json.Root());
		return json.Save(dest);
	}

	Node* Scene::Instantiate(Stream& source)
	{
		ALIMER_PROFILE(Instantiate);

		ObjectResolver resolver;
		StringHash childType(source.Read<StringHash>());
		unsigned childId = source.Read<unsigned>();

		Node* child = CreateChild(childType);
		if (child)
		{
			resolver.StoreObject(childId, child);
			child->Load(source, resolver);
			resolver.Resolve();
		}

		return child;
	}

	Node* Scene::InstantiateJSON(const JSONValue& source)
	{
		ALIMER_PROFILE(InstantiateJSON);

		ObjectResolver resolver;
		StringHash childType(source["type"].GetString());
		unsigned childId = (unsigned)source["id"].GetNumber();

		Node* child = CreateChild(childType);
		if (child)
		{
			resolver.StoreObject(childId, child);
			child->LoadJSON(source, resolver);
			resolver.Resolve();
		}

		return child;
	}

	Node* Scene::InstantiateJSON(Stream& source)
	{
		JSONFile json;
		json.Load(source);
		return InstantiateJSON(json.Root());
	}

	void Scene::DefineLayer(unsigned char index, const String& name)
	{
		if (index >= 32)
		{
			LOGERROR("Can not define more than 32 layers");
			return;
		}

		if (layerNames.size() <= index)
			layerNames.resize(index + 1);
		layerNames[index] = name;
		layers[name] = index;
	}

	void Scene::DefineTag(unsigned char index, const String& name)
	{
		if (tagNames.size() <= index)
			tagNames.resize(index + 1);
		tagNames[index] = name;
		tags[name] = index;
	}

	void Scene::Clear()
	{
		RemoveAllChildren();
		nextNodeId = 1;
	}

	Node* Scene::FindNode(unsigned id) const
	{
		auto it = nodes.find(id);
		return it != nodes.end() ? it->second : nullptr;
	}

	void Scene::AddNode(Node* node)
	{
		if (!node || node->ParentScene() == this)
			return;

		while (nodes.find(nextNodeId) != nodes.end())
		{
			++nextNodeId;
			if (!nextNodeId)
				++nextNodeId;
		}

		Scene* oldScene = node->ParentScene();
		if (oldScene)
		{
			uint32_t oldId = node->GetId();
			oldScene->nodes.erase(oldId);
		}
		nodes[nextNodeId] = node;
		node->SetScene(this);
		node->SetId(nextNodeId);

		++nextNodeId;

		// If node has children, add them to the scene as well
		if (node->NumChildren())
		{
			const auto& children = node->Children();
			for (auto it = children.begin(); it != children.end(); ++it)
				AddNode(*it);
		}
	}

	void Scene::RemoveNode(Node* node)
	{
		if (!node || node->ParentScene() != this)
			return;

		nodes.erase(node->GetId());
		node->SetScene(nullptr);
		node->SetId(0);

		// If node has children, remove them from the scene as well
		if (node->NumChildren())
		{
			const auto& children = node->Children();
			for (auto it = children.begin(); it != children.end(); ++it)
				RemoveNode(*it);
		}
	}

	void Scene::SetLayerNamesAttr(JSONValue names)
	{
		layerNames.clear();
		layers.clear();

		const JSONArray& array = names.GetArray();
		for (size_t i = 0; i < array.size(); ++i)
		{
			const String& name = array[i].GetString();
			layerNames.push_back(name);
			layers[name] = (unsigned char)i;
		}
	}

	JSONValue Scene::LayerNamesAttr() const
	{
		JSONValue ret;

		ret.SetEmptyArray();
		for (auto it = layerNames.begin(); it != layerNames.end(); ++it)
			ret.Push(*it);

		return ret;
	}

	void Scene::SetTagNamesAttr(JSONValue names)
	{
		tagNames.clear();
		tags.clear();

		const JSONArray& array = names.GetArray();
		for (size_t i = 0; i < array.size(); ++i)
		{
			const String& name = array[i].GetString();
			tagNames.push_back(name);
			tags[name] = (uint8_t)i;
		}
	}

	JSONValue Scene::TagNamesAttr() const
	{
		JSONValue ret;

		ret.SetEmptyArray();
		for (auto it = tagNames.begin(); it != tagNames.end(); ++it)
			ret.Push(*it);

		return ret;
	}

	void RegisterSceneLibrary()
	{
		static bool registered = false;
		if (registered)
			return;
		registered = true;

		Node::RegisterObject();
		Scene::RegisterObject();
		SpatialNode::RegisterObject();
	}

}