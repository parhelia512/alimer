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
#include "../IO/File.h"
#include "JSONFile.h"

namespace Alimer
{
	void JSONFile::RegisterObject()
	{
		RegisterFactory<JSONFile>();
	}

	bool JSONFile::BeginLoad(Stream& source)
	{
		ALIMER_PROFILE(LoadJSONFile);

		size_t dataSize = source.Size() - source.Position();
		std::unique_ptr<char[]> buffer(new char[dataSize]);
		if (source.Read(buffer.get(), dataSize) != dataSize)
			return false;

		const char* pos = buffer.get();
		const char* end = pos + dataSize;

		// Remove any previous content
		/// \todo If fails, log the line number on which the error occurred
		_root = json::parse(pos, end);
		if (!_root.is_object())
		{
			LOGERROR("Parsing JSON from " + source.GetName() + " failed; data may be partial");
			return false;
		}

		return true;
	}

	bool JSONFile::Save(Stream& dest)
	{
		ALIMER_PROFILE(SaveJSONFile);

		std::string buffer = _root.dump(4);
		return dest.Write(buffer.data(), buffer.length()) == buffer.length();
	}
}
