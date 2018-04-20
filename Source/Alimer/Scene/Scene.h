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

#include "Node.h"

namespace Alimer
{
	/// %Scene root node, which also represents the whole scene.
	class ALIMER_API Scene : public Node
	{
		ALIMER_OBJECT(Scene, Node);

	public:
		/// Construct.
		Scene();
		/// Destruct. The whole node tree is destroyed.
		~Scene();

		/// Register factory and attributes.
		static void RegisterObject();

		/// Save scene to binary stream.
		void Save(Stream& dest) override;

		/// Load scene from a binary stream. Existing nodes will be destroyed. Return true on success.
		bool Load(Stream& source);
		/// Load scene from JSON data. Existing nodes will be destroyed. Return true on success.
		bool LoadJSON(const json& source);
		/// Load scene from JSON text data read from a binary stream. Existing nodes will be destroyed. Return true if the JSON was correctly parsed; otherwise the data may be partial.
		bool LoadJSON(Stream& source);
		/// Save scene as JSON text data to a binary stream. Return true on success.
		bool SaveJSON(Stream& dest);
		/// Instantiate node(s) from binary stream and return the root node.
		Node* Instantiate(Stream& source);
		/// Instantiate node(s) from JSON data and return the root node.
		Node* InstantiateJSON(const json& source);
		/// Load JSON data as text from a binary stream, then instantiate node(s) from it and return the root node.
		Node* InstantiateJSON(Stream& source);
		/// Define a layer name. There can be 32 different layers (indices 0-31.)
		void DefineLayer(uint8_t index, const std::string& name);
		/// Define a tag name.
		void DefineTag(uint8_t index, const std::string& name);
		/// Destroy child nodes recursively, leaving the scene empty.
		void Clear();

		/// Find node by id.
		Node* FindNode(uint32_t id) const;
		/// Return the layer names.
		const std::vector<std::string>& LayerNames() const { return _layerNames; }
		/// Return the layer name-to-index map.
		const std::unordered_map<std::string, uint8_t>& Layers() const { return _layers; }
		/// Return the tag names.
		const std::vector<std::string>& TagNames() const { return _tagNames; }
		/// Return the tag name-to-index map.
		const std::unordered_map<std::string, uint8_t>& Tags() const { return _tags; }

		/// Add node to the scene. This assigns a scene-unique id to it. Called internally.
		void AddNode(Node* node);
		/// Remove node from the scene. This removes the id mapping but does not destroy the node. Called internally.
		void RemoveNode(Node* node);

		using Node::Load;
		using Node::LoadJSON;
		using Node::SaveJSON;

	private:
		/// Set layer names. Used in serialization.
		void SetLayerNamesAttr(json names);
		/// Return layer names. Used in serialization.
		json GetLayerNamesAttr() const;
		/// Set tag names. Used in serialization.
		void SetTagNamesAttr(json names);
		/// Return tag names. Used in serialization.
		json GetTagNamesAttr() const;

		/// Map from id's to nodes.
		std::map<uint32_t, Node*> _nodesMap;
		/// Next free node id.
		uint32_t _nextNodeId;
		/// List of layer names by index.
		std::vector<std::string> _layerNames;
		/// Map from layer names to indices.
		std::unordered_map<std::string, uint8_t> _layers;
		/// List of tag names by index.
		std::vector<std::string> _tagNames;
		/// Map from tag names to indices.
		std::unordered_map<std::string, uint8_t> _tags;
	};

	/// Register Scene related object factories and attributes.
	ALIMER_API void RegisterSceneLibrary();
}

