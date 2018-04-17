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

#include "../Object/Object.h"
#include "ResourceRef.h"
#include "Stream.h"
using namespace std;

namespace Alimer
{
	bool ResourceRef::FromString(const string& str)
	{
		auto values = str::Split(str, ";");
		if (values.size() == 2)
		{
			type = values[0];
			name = values[1];
			return true;
		}

		return false;
	}

	void ResourceRef::FromBinary(Stream& source)
	{
		type = source.Read<StringHash>();
		name = source.ReadString();
	}

	string ResourceRef::ToString() const
	{
		return Object::GetTypeNameFromType(type) + ";" + name;
	}

	void ResourceRef::ToBinary(Stream& dest) const
	{
		dest.WriteStringHash(type);
		dest.WriteString(name);
	}

	bool ResourceRefList::FromString(const string& str)
	{
		auto values = str::Split(str, ";");
		if (values.size() >= 1)
		{
			type = values[0];
			names.clear();
			for (size_t i = 1; i < values.size(); ++i)
			{
				names.push_back(values[i]);
			}

			return true;
		}

		return false;
	}

	void ResourceRefList::FromBinary(Stream& source)
	{
		type = source.Read<StringHash>();
		size_t num = source.ReadVLE();
		names.clear();
		for (size_t i = 0; i < num && !source.IsEof(); ++i)
		{
			names.push_back(source.ReadString());
		}
	}

	string ResourceRefList::ToString() const
	{
		string ret(Object::GetTypeNameFromType(type));
		for (auto it = names.begin(); it != names.end(); ++it)
		{
			ret += ";";
			ret += *it;
		}
		return ret;
	}

	void ResourceRefList::ToBinary(Stream& dest) const
	{
		dest.WriteStringHash(type);
		dest.WriteVLE(static_cast<uint32_t>(names.size()));
		for (auto it = names.begin(); it != names.end(); ++it)
		{
			dest.WriteString(*it);
		}
	}

}
