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

#include "../Base/StringHash.h"
#include "../Base/Vector.h"

namespace Alimer
{

	class Stream;

	/// Typed resource reference for serialization.
	struct ALIMER_API ResourceRef
	{
		/// Resource type.
		StringHash type;
		/// Resource name.
		std::string name;

		/// Construct.
		ResourceRef()
		{
		}

		// Copy-construct.
		ResourceRef(const ResourceRef& ref)
			: type(ref.type)
			, name(ref.name)
		{
		}

		/// Construct from a string.
		ResourceRef(const std::string& str)
		{
			FromString(str);
		}

		/// Construct with type and resource name.
		ResourceRef(StringHash type, const std::string& name_ = "")
			: type(type)
			, name(name_)
		{
		}

		/// Set from a string that contains the type and name separated by a semicolon. Return true on success.
		bool FromString(const std::string& str);
		/// Deserialize from a binary stream.
		void FromBinary(Stream& source);

		/// Return as a string.
		std::string ToString() const;
		/// Serialize to a binary stream.
		void ToBinary(Stream& dest) const;

		/// Test for equality with another reference.
		bool operator == (const ResourceRef& rhs) const { return type == rhs.type && name == rhs.name; }
		/// Test for inequality with another reference.
		bool operator != (const ResourceRef& rhs) const { return !(*this == rhs); }
	};

	/// %List of typed resource references for serialization.
	struct ALIMER_API ResourceRefList
	{
		/// Resource type.
		StringHash type;
		/// List of resource names.
		std::vector<std::string> names;

		/// Construct.
		ResourceRefList()
		{
		}

		// Copy-construct.
		ResourceRefList(const ResourceRefList& refList)
			: type(refList.type)
			, names(refList.names)
		{
		}

		/// Construct from a string.
		ResourceRefList(const std::string& str)
		{
			FromString(str);
		}

		/// Construct with type and name list.
		ResourceRefList(StringHash type, const std::vector<std::string>& names_ = std::vector<std::string>())
			: type(type)
			, names(names_)
		{
		}

		/// Set from a string that contains the type and names separated by semicolons. Return true on success.
		bool FromString(const std::string& str);
		/// Deserialize from a binary stream.
		void FromBinary(Stream& source);

		/// Return as a string.
		std::string ToString() const;
		/// Deserialize from a binary stream.
		void ToBinary(Stream& dest) const;

		/// Test for equality with another reference list.
		bool operator == (const ResourceRefList& rhs) const { return type == rhs.type && names == rhs.names; }
		/// Test for inequality with another reference list.
		bool operator != (const ResourceRefList& rhs) const { return !(*this == rhs); }
	};

}
