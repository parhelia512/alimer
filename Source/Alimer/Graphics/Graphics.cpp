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
#include "Graphics.h"
#include "GraphicsImpl.h"

namespace Alimer
{
	bool Graphics::IsBackendSupported(GraphicsDeviceType deviceType)
	{
		switch (deviceType)
		{
		case GraphicsDeviceType::Empty:
			return true;

		case GraphicsDeviceType::D3D11:
#ifdef ALIMER_D3D11
			//if (GraphicsD3D11::IsSupported())
			//{
				return true;
			//}
#endif
			return false;

		case GraphicsDeviceType::OpenGL:
#ifdef ALIMER_OPENGL
			if (GraphicsGL::IsSupported())
			{
				return true;
			}
#endif
			return false;

		default:
			return false;
		}
	}
}
