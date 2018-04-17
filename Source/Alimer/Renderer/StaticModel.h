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

#include "GeometryNode.h"

namespace Alimer
{
	/// %Scene node that renders an unanimated model.
	class ALIMER_API StaticModel : public GeometryNode
	{
		ALIMER_OBJECT(StaticModel, GeometryNode);

	public:
		/// Construct.
		StaticModel();
		/// Destruct.
		~StaticModel();

		/// Register factory and attributes.
		static void RegisterObject();

		/// Prepare object for rendering. Reset framenumber and light list and calculate distance from camera, and check for LOD level changes. Called by Renderer.
		void OnPrepareRender(unsigned frameNumber, Camera* camera) override;

		/// Set the model resource.
		void SetModel(Model* model);
		/// Set LOD bias. Values higher than 1 use higher quality LOD (acts if distance is smaller.)
		void SetLodBias(float bias);

		/// Return the model resource.
		Model* GetModel() const;
		/// Return LOD bias.
		float LodBias() const { return lodBias; }

	private:
		/// Set model attribute. Used in serialization.
		void SetModelAttr(const ResourceRef& model);
		/// Return model attribute. Used in serialization.
		ResourceRef ModelAttr() const;

		/// Current model resource.
		SharedPtr<Model> model;
		/// LOD bias value.
		float lodBias;
		/// Lod levels flag.
		bool hasLodLevels;
	};

}
