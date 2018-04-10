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

#include "../Base/Ptr.h"
#include "../AlimerConfig.h"

namespace Alimer
{
	class Graphics;

	/// Base class for objects that allocate GPU resources.
	class ALIMER_API GPUObject
	{
	public:
		/// Construct. Acquire the %Graphics subsystem if available and register self.
		GPUObject();
		/// Destruct. Unregister from the %Graphics subsystem.
		virtual ~GPUObject();

		/// Release the GPU resource.
		virtual void Release();
		/// Recreate the GPU resource after data loss. Not called on all rendering API's.
		virtual void Recreate();
		/// Return whether the contents have been lost due to graphics context having been destroyed.
		virtual bool IsDataLost() const { return dataLost; }

		/// Set data lost state. Not needed on all rendering API's.
		void SetDataLost(bool enable) { dataLost = enable; }

	protected:
		/// %Graphics subsystem pointer.
		WeakPtr<Graphics> graphics;

	private:
		/// Data lost flag.
		bool dataLost;
	};

}
