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

#include "../Object/Serializable.h"

namespace Alimer
{
	class Scene;
	class ObjectResolver;

	static const unsigned short NF_ENABLED = 0x1;
	static const unsigned short NF_TEMPORARY = 0x2;
	static const unsigned short NF_SPATIAL = 0x4;
	static const unsigned short NF_SPATIAL_PARENT = 0x8;
	static const unsigned short NF_WORLD_TRANSFORM_DIRTY = 0x10;
	static const unsigned short NF_BOUNDING_BOX_DIRTY = 0x20;
	static const unsigned short NF_OCTREE_UPDATE_QUEUED = 0x40;
	static const unsigned short NF_GEOMETRY = 0x80;
	static const unsigned short NF_LIGHT = 0x100;
	static const unsigned short NF_CASTSHADOWS = 0x200;
	static const unsigned char LAYER_DEFAULT = 0x0;
	static const uint8_t TAG_NONE = 0x0;
	static const unsigned LAYERMASK_ALL = 0xffffffff;

	/// Base class for scene nodes.
	class ALIMER_API Node : public Serializable
	{
		ALIMER_OBJECT(Node, Serializable);

	public:
		/// Construct.
		Node();
		/// Destruct. Destroy any child nodes.
		~Node();

		/// Register factory and attributes.
		static void RegisterObject();

		/// Load from binary stream. Store node references to be resolved later.
		void Load(Stream& source, ObjectResolver& resolver) override;
		/// Save to binary stream.
		void Save(Stream& dest) override;
		/// Load from JSON data. Store node references to be resolved later.
		void LoadJSON(const json& source, ObjectResolver& resolver) override;
		/// Save as JSON data.
		void SaveJSON(json& dest) override;
		/// Return unique id within the scene, or 0 if not in a scene.
		uint32_t GetId() const override { return _id; }

		/// Save as JSON text data to a binary stream. Return true on success.
		bool SaveJSON(Stream& dest);
		/// Set name. Is not required to be unique within the scene.
		void SetName(const std::string& newName);
		/// Set node's layer. Usage is subclass specific, for example rendering nodes selectively. Default is 0.
		void SetLayer(uint8_t newLayer);
		/// Set node's layer by name. The layer name must have been registered to the scene root beforehand.
		void SetLayerName(const std::string& newLayerName);
		/// Set node's tag, which can be used to search for specific nodes.
		void SetTag(uint8_t newTag);
		/// Set node's tag by name. The tag name must have been registered to the scene root beforehand.
		void SetTagName(const std::string& newTagName);
		/// Set enabled status. Meaning is subclass specific.
		void SetEnabled(bool enable);
		/// Set enabled status recursively in the child hierarchy.
		void SetEnabledRecursive(bool enable);
		/// Set temporary mode. Temporary scene nodes are not saved.
		void SetTemporary(bool enable);
		/// Reparent the node.
		void SetParent(Node* newParent);
		/// Create child node of specified type. A registered object factory for the type is required.
		Node* CreateChild(StringHash childType);
		/// Create named child node of specified type.
		Node* CreateChild(StringHash childType, const std::string& childName);
		/// Add node as a child. Same as calling SetParent for the child node.
		void AddChild(Node* child);
		/// Remove child node. Will delete it if there are no other strong references to it.
		void RemoveChild(Node* child);
		/// Remove child node by index.
		void RemoveChild(size_t index);
		/// Remove all child nodes.
		void RemoveAllChildren();
		/// Remove self immediately. As this will delete the node (if no other strong references exist) no operations on the node are permitted after calling this.
		void RemoveSelf();
		/// Create child node of the specified type, template version.
		template <class T> T* CreateChild() { return static_cast<T*>(CreateChild(T::GetTypeStatic())); }
		/// Create named child node of the specified type, template version.
		template <class T> T* CreateChild(const std::string& childName) { return static_cast<T*>(CreateChild(T::GetTypeStatic(), childName)); }
		/// Create named child node of the specified type, template version.
		template <class T> T* CreateChild(const char* childName) { return static_cast<T*>(CreateChild(T::GetTypeStatic(), childName)); }

		/// Return name.
		const std::string& GetName() const { return _name; }
		/// Return layer.
		uint8_t GetLayer() const { return _layer; }
		/// Return layer name, or empty if not registered in the scene root.
		const std::string& GetLayerName() const;
		/// Return bitmask corresponding to layer.
		uint32_t GetLayerMask() const { return 1 << _layer; }
		/// Return tag.
		uint8_t GetTag() const { return _tag; }
		/// Return tag name, or empty if not registered in the scene root.
		const std::string& GetTagName() const;
		/// Return enabled status.
		bool IsEnabled() const { return TestFlag(NF_ENABLED); }
		/// Return whether is temporary.
		bool IsTemporary() const { return TestFlag(NF_TEMPORARY); }
		/// Return parent node.
		Node* GetParent() const { return _parent; }
		/// Return the scene that the node belongs to.
		Scene* GetParentScene() const { return _scene; }
		/// Return number of immediate child nodes.
		uint32_t NumChildren() const { return static_cast<uint32_t>(_children.size()); }
		/// Return number of immediate child nodes that are not temporary.
		uint32_t NumPersistentChildren() const;
		/// Return immediate child node by index.
		Node* GetChild(size_t index) const { return index < _children.size() ? _children[index].Get() : nullptr; }
		/// Return all immediate child nodes.
		const std::vector<SharedPtr<Node> >& GetChildren() const { return _children; }
		/// Return child nodes recursively.
		void AllChildren(std::vector<Node*>& result) const;
		/// Return first child node that matches name.
		Node* FindChild(const std::string& childName, bool recursive = false) const;
		/// Return first child node that matches name.
		Node* FindChild(const char* childName, bool recursive = false) const;
		/// Return first child node of specified type.
		Node* FindChild(StringHash childType, bool recursive = false) const;
		/// Return first child node that matches type and name.
		Node* FindChild(StringHash childType, const std::string& childName, bool recursive = false) const;
		/// Return first child node that matches type and name.
		Node* FindChild(StringHash childType, const char* childName, bool recursive = false) const;
		/// Return first child node that matches layer mask.
		Node* FindChildByLayer(unsigned layerMask, bool recursive = false) const;
		/// Return first child node that matches tag.
		Node* FindChildByTag(unsigned char tag, bool recursive = false) const;
		/// Return first child node that matches tag name.
		Node* FindChildByTag(const std::string& tagName, bool recursive = false) const;
		/// Return first child node that matches tag name.
		Node* FindChildByTag(const char* tagName, bool recursive = false) const;
		/// Find child nodes of specified type.
		void FindChildren(std::vector<Node*>& result, StringHash childType, bool recursive = false) const;
		/// Find child nodes that match layer mask.
		void FindChildrenByLayer(std::vector<Node*>& result, uint32_t layerMask, bool recursive = false) const;
		/// Find child nodes that match tag.
		void FindChildrenByTag(std::vector<Node*>& result, uint8_t tag, bool recursive = false) const;
		/// Find child nodes that match tag name.
		void FindChildrenByTag(std::vector<Node*>& result, const std::string& tagName, bool recursive = false) const;
		/// Find child nodes that match tag name.
		void FindChildrenByTag(std::vector<Node*>& result, const char* tagName, bool recursive = false) const;
		/// Return first child node of specified type, template version.
		template <class T> T* FindChild(bool recursive = false) const { return static_cast<T*>(FindChild(T::GetTypeStatic(), recursive)); }
		/// Return first child node that matches type and name, template version.
		template <class T> T* FindChild(const std::string& childName, bool recursive = false) const { return static_cast<T*>(FindChild(T::GetTypeStatic(), childName, recursive)); }
		/// Return first child node that matches type and name, template version.
		template <class T> T* FindChild(const char* childName, bool recursive = false) const { return static_cast<T*>(FindChild(T::GetTypeStatic(), childName, recursive)); }
		/// Find child nodes of specified type, template version.
		template <class T> void FindChildren(std::vector<T*>& result, bool recursive = false) const { return FindChildren(reinterpret_cast<std::vector<T*>&>(result), recursive); }

		/// Set bit flag. Called internally.
		void SetFlag(uint16_t bit, bool set) const { if (set) _flags |= bit; else _flags &= ~bit; }
		/// Test bit flag. Called internally.
		bool TestFlag(uint16_t bit) const { return (_flags & bit) != 0; }
		/// Return bit flags. Used internally eg. by octree queries.
		uint16_t GetFlags() const { return _flags; }
		/// Assign node to a new scene. Called internally.
		void SetScene(Scene* newScene);
		/// Assign new id. Called internally.
		void SetId(uint32_t newId);

		/// Skip the binary data of a node hierarchy, in case the node could not be created.
		static void SkipHierarchy(Stream& source);

	protected:
		/// Handle being assigned to a new parent node.
		virtual void OnParentSet(Node* newParent, Node* oldParent);
		/// Handle being assigned to a new scene.
		virtual void OnSceneSet(Scene* newScene, Scene* oldScene);
		/// Handle the enabled status changing.
		virtual void OnSetEnabled(bool newEnabled);

	private:
		/// Parent node.
		Node* _parent = nullptr;
		/// Parent scene.
		Scene* _scene = nullptr;
		/// Child nodes.
		std::vector<SharedPtr<Node> > _children;
		/// Id within the scene.
		uint32_t _id{};
		/// %Node name.
		std::string _name;
		/// %Node flags. Used to hold several boolean values (some subclass-specific) to reduce memory use.
		mutable uint16_t _flags;
		/// Layer number.
		uint8_t _layer;
		/// Tag number.
		uint8_t _tag;
	};

}
