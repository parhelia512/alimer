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
#include "Resource.h"

namespace Alimer
{
	bool Resource::BeginLoad(Stream&)
	{
		return false;
	}

	bool Resource::EndLoad()
	{
		// Resources that do not need access to main-thread critical objects do not need to override this
		return true;
	}

	bool Resource::Save(Stream&)
	{
		ALIMER_LOGERROR("Save not supported for " + GetTypeName());
		return false;
	}

	bool Resource::Load(Stream& source)
	{
		bool success = BeginLoad(source);
		if (success)
			success &= EndLoad();

		return success;
	}

	void Resource::SetName(const std::string& newName)
	{
		_name = newName;
		_nameHash = StringHash(newName);
	}
}
