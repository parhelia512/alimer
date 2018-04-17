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

#include "Ptr.h"

namespace Alimer
{

	RefCounted::RefCounted() :
		refCount(nullptr)
	{
	}

	RefCounted::~RefCounted()
	{
		if (refCount)
		{
			assert(refCount->refs == 0);
			if (refCount->weakRefs == 0)
				delete refCount;
			else
				refCount->expired = true;
		}
	}

	void RefCounted::AddRef()
	{
		if (!refCount)
			refCount = new RefCount();

		++(refCount->refs);
	}

	void RefCounted::ReleaseRef()
	{
		assert(refCount && refCount->refs > 0);
		--(refCount->refs);
		if (refCount->refs == 0)
			delete this;
	}

	RefCount* RefCounted::RefCountPtr()
	{
		if (!refCount)
			refCount = new RefCount();

		return refCount;
	}

}
