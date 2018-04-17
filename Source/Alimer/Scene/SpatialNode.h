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

#include "../Math/Matrix3x4.h"
#include "Node.h"

namespace Alimer
{
	/// Transform space for translations and rotations.
	enum TransformSpace
	{
		TS_LOCAL = 0,
		TS_PARENT,
		TS_WORLD
	};

	/// Base class for scene nodes with position in three-dimensional space.
	class ALIMER_API SpatialNode : public Node
	{
		OBJECT(SpatialNode);

	public:
		/// Construct.
		SpatialNode();

		/// Register factory and attributes.
		static void RegisterObject();

		/// Set position in parent space.
		void SetPosition(const Vector3& newPosition);
		/// Set rotation in parent space.
		void SetRotation(const Quaternion& newRotation);
		/// Set forward direction in parent space.
		void SetDirection(const Vector3& newDirection);
		/// Set scale in parent space.
		void SetScale(const Vector3& newScale);
		/// Set uniform scale in parent space.
		void SetScale(float newScale);
		/// Set transform in parent space.
		void SetTransform(const Vector3& newPosition, const Quaternion& newRotation);
		/// Set transform in parent space.
		void SetTransform(const Vector3& newPosition, const Quaternion& newRotation, const Vector3& newScale);
		/// Set transform in parent space.
		void SetTransform(const Vector3& newPosition, const Quaternion& newRotation, float newScale);
		/// Set position in world space.
		void SetWorldPosition(const Vector3& newPosition);
		/// Set rotation in world space
		void SetWorldRotation(const Quaternion& newRotation);
		/// Set forward direction in world space.
		void SetWorldDirection(const Vector3& newDirection);
		/// Set scale in world space.
		void SetWorldScale(const Vector3& newScale);
		/// Set uniform scale in world space.
		void SetWorldScale(float newScale);
		/// Set transform in world space.
		void SetWorldTransform(const Vector3& newPosition, const Quaternion& newRotation);
		/// Set transform in world space.
		void SetWorldTransform(const Vector3& newPosition, const Quaternion& newRotation, const Vector3& newScale);
		/// Set transform in world space.
		void SetWorldTransform(const Vector3& newPosition, const Quaternion& newRotation, float newScale);
		/// Move the scene node in the chosen transform space.
		void Translate(const Vector3& delta, TransformSpace space = TS_LOCAL);
		/// Rotate the scene node in the chosen transform space.
		void Rotate(const Quaternion& delta, TransformSpace space = TS_LOCAL);
		/// Rotate around a point in the chosen transform space.
		void RotateAround(const Vector3& point, const Quaternion& delta, TransformSpace space = TS_LOCAL);
		/// Rotate around the X axis.
		void Pitch(float angle, TransformSpace space = TS_LOCAL);
		/// Rotate around the Y axis.
		void Yaw(float angle, TransformSpace space = TS_LOCAL);
		/// Rotate around the Z axis.
		void Roll(float angle, TransformSpace space = TS_LOCAL);
		/// Look at a target position in the chosen transform space. Note that the up vector is always specified in world space. Return true if successful, or false if resulted in an illegal rotation, in which case the current rotation remains.
		bool LookAt(const Vector3& target, const Vector3& up = Vector3::UP, TransformSpace space = TS_WORLD);
		/// Apply a scale change.
		void ApplyScale(const Vector3& delta);
		/// Apply an uniform scale change.
		void ApplyScale(float delta);

		/// Return the parent spatial node, or null if it is not spatial.
		SpatialNode* SpatialParent() const { return TestFlag(NF_SPATIAL_PARENT) ? static_cast<SpatialNode*>(GetParent()) : nullptr; }
		/// Return position in parent space.
		const Vector3& Position() const { return _position; }
		/// Return rotation in parent space.
		const Quaternion& Rotation() const { return _rotation; }
		/// Return forward direction in parent space.
		Vector3 Direction() const { return _rotation * Vector3::FORWARD; }
		/// Return scale in parent space.
		const Vector3& Scale() const { return _scale; }
		/// Return transform matrix in parent space.
		Matrix3x4 Transform() const { return Matrix3x4(_position, _rotation, _scale); }
		/// Return position in world space.
		Vector3 WorldPosition() const { return WorldTransform().Translation(); }
		/// Return rotation in world space.
		Quaternion WorldRotation() const { return WorldTransform().Rotation(); }
		/// Return forward direction in world space.
		Vector3 WorldDirection() const { return WorldRotation() * Vector3::FORWARD; }
		/// Return scale in world space. As it is calculated from the world transform matrix, it may not be meaningful or accurate in all cases.
		Vector3 WorldScale() const { return WorldTransform().Scale(); }
		/// Return world transform matrix.
		const Matrix3x4& WorldTransform() const { if (TestFlag(NF_WORLD_TRANSFORM_DIRTY)) UpdateWorldTransform(); return _worldTransform; }
		/// Convert a local space position to world space.
		Vector3 LocalToWorld(const Vector3& point) const { return WorldTransform() * point; }
		/// Convert a local space vector (either position or direction) to world space.
		Vector3 LocalToWorld(const Vector4& vector) const { return WorldTransform() * vector; }
		/// Convert a world space position to local space.
		Vector3 WorldToLocal(const Vector3& point) const { return WorldTransform().Inverse() * point; }
		/// Convert a world space vector (either position or direction) to world space.
		Vector3 WorldToLocal(const Vector4& vector) const { return WorldTransform().Inverse() * vector; }

	protected:
		/// Handle being assigned to a new parent node.
		virtual void OnParentSet(Node* newParent, Node* oldParent);
		/// Handle the transform matrix changing.
		virtual void OnTransformChanged();

	private:
		/// Update world transform matrix from spatial parent chain.
		void UpdateWorldTransform() const;

		/// World transform matrix.
		mutable Matrix3x4 _worldTransform;
		/// Parent space position.
		Vector3 _position;
		/// Parent space rotation.
		Quaternion _rotation;
		/// Parent space scale.
		Vector3 _scale;
	};
}
