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
#include "../IO/Stream.h"
#include "../Object/ObjectResolver.h"
#include "../Resource/JSONFile.h"
#include "Scene.h"
#include "SpatialNode.h"

using namespace std;

namespace Alimer
{
	Scene::Scene()
		: _nextNodeId(1)
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
		assert(_nodesMap.empty());
	}

	void Scene::RegisterObject()
	{
		RegisterFactory<Scene>();
		CopyBaseAttributes<Scene, Node>();
		RegisterAttribute("layerNames", &Scene::GetLayerNamesAttr, &Scene::SetLayerNamesAttr);
		RegisterAttribute("tagNames", &Scene::GetTagNamesAttr, &Scene::SetTagNamesAttr);
	}

	void Scene::Save(Stream& dest)
	{
		ALIMER_PROFILE(SaveScene);

		ALIMER_LOGINFO("Saving scene to " + dest.GetName());

		dest.WriteFileID("SCNE");
		Node::Save(dest);
	}

	bool Scene::Load(Stream& source)
	{
		ALIMER_PROFILE(LoadScene);

		ALIMER_LOGINFO("Loading scene from " + source.GetName());

		std::string fileId = source.ReadFileID();
		if (fileId != "SCNE")
		{
			ALIMER_LOGERROR("File is not a binary scene file");
			return false;
		}

		StringHash ownType = source.ReadStringHash();
		uint32_t ownId = source.ReadUInt();
		if (ownType != GetTypeStatic())
		{
			ALIMER_LOGERROR("Mismatching type of scene root node in scene file");
			return false;
		}

		Clear();

		ObjectResolver resolver;
		resolver.StoreObject(ownId, this);
		Node::Load(source, resolver);
		resolver.Resolve();

		return true;
	}

	bool Scene::LoadJSON(const json& source)
	{
		ALIMER_PROFILE(LoadSceneJSON);

		StringHash ownType(source["type"].get<string>());
		uint32_t ownId = source["id"].get<uint32_t>();

		if (ownType != GetTypeStatic())
		{
			ALIMER_LOGERROR("Mismatching type of scene root node in scene file");
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
		ALIMER_LOGINFO("Loading scene from " + source.GetName());

		JSONFile json;
		bool success = json.Load(source);
		LoadJSON(json.GetRoot());
		return success;
	}

	bool Scene::SaveJSON(Stream& dest)
	{
		ALIMER_PROFILE(SaveSceneJSON);

		ALIMER_LOGINFO("Saving scene to " + dest.GetName());

		JSONFile json;
		Node::SaveJSON(json.GetRoot());
		return json.Save(dest);
	}

	Node* Scene::Instantiate(Stream& source)
	{
		ALIMER_PROFILE(Instantiate);

		ObjectResolver resolver;
		StringHash childType = source.ReadStringHash();
		uint32_t childId = source.ReadUInt();

		Node* child = CreateChild(childType);
		if (child)
		{
			resolver.StoreObject(childId, child);
			child->Load(source, resolver);
			resolver.Resolve();
		}

		return child;
	}

	Node* Scene::InstantiateJSON(const json& source)
	{
		ALIMER_PROFILE(InstantiateJSON);

		ObjectResolver resolver;
		StringHash childType(source["type"].get<string>());
		uint32_t childId = source["id"].get<uint32_t>();

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
		return InstantiateJSON(json.GetRoot());
	}

	void Scene::DefineLayer(uint8_t index, const std::string& name)
	{
		if (index >= 32)
		{
			ALIMER_LOGERROR("Can not define more than 32 layers");
			return;
		}

		if (_layerNames.size() <= index)
			_layerNames.resize(index + 1);
		_layerNames[index] = name;
		_layers[name] = index;
	}

	void Scene::DefineTag(uint8_t index, const std::string& name)
	{
		if (_tagNames.size() <= index)
			_tagNames.resize(index + 1);
		_tagNames[index] = name;
		_tags[name] = index;
	}

	void Scene::Clear()
	{
		RemoveAllChildren();
		_nextNodeId = 1;
	}

	Node* Scene::FindNode(uint32_t id) const
	{
		auto it = _nodesMap.find(id);
		return it != _nodesMap.end() ? it->second : nullptr;
	}

	void Scene::AddNode(Node* node)
	{
		if (!node || node->GetParentScene() == this)
			return;

		while (_nodesMap.find(_nextNodeId) != _nodesMap.end())
		{
			++_nextNodeId;
			if (!_nextNodeId)
			{
				++_nextNodeId;
			}
		}

		Scene* oldScene = node->GetParentScene();
		if (oldScene)
		{
			uint32_t oldId = node->GetId();
			oldScene->_nodesMap.erase(oldId);
		}
		_nodesMap[_nextNodeId] = node;
		node->SetScene(this);
		node->SetId(_nextNodeId);

		++_nextNodeId;

		// If node has children, add them to the scene as well
		if (node->NumChildren())
		{
			const auto& children = node->GetChildren();
			for (auto it = children.begin(); it != children.end(); ++it)
				AddNode(*it);
		}
	}

	void Scene::RemoveNode(Node* node)
	{
		if (!node || node->GetParentScene() != this)
			return;

		_nodesMap.erase(node->GetId());
		node->SetScene(nullptr);
		node->SetId(0);

		// If node has children, remove them from the scene as well
		if (node->NumChildren())
		{
			const auto& children = node->GetChildren();
			for (auto it = children.begin(); it != children.end(); ++it)
				RemoveNode(*it);
		}
	}

	void Scene::SetLayerNamesAttr(json names)
	{
		_layerNames.clear();
		_layers.clear();

		for (size_t i = 0; i < names.size(); ++i)
		{
			const string& name = names[i].get<string>();
			_layerNames.push_back(name);
			_layers[name] = (uint8_t)i;
		}
	}

	json Scene::GetLayerNamesAttr() const
	{
		json ret = json::array();
		for (auto it = _layerNames.begin(); it != _layerNames.end(); ++it)
		{
			ret.push_back(*it);
		}

		return ret;
	}

	void Scene::SetTagNamesAttr(json names)
	{
		_tagNames.clear();
		_tags.clear();

		for (size_t i = 0; i < names.size(); ++i)
		{
			const string& name = names[i].get<string>();
			_tagNames.push_back(name);
			_tags[name] = (uint8_t)i;
		}
	}

	json Scene::GetTagNamesAttr() const
	{
		json ret = json::array();

		for (auto it = _tagNames.begin(); it != _tagNames.end(); ++it)
			ret.push_back(*it);

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
