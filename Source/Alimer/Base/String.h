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

#include "StdHeaders.h"

namespace Alimer
{
	static const size_t CONVERSION_BUFFER_LENGTH = 256;

	namespace str
	{
		/// Empty string.
		static const std::string EMPTY;

		std::string Replace(const std::string& str, const std::string& find, const std::string& replace,
			uint32_t maxReplacements = std::numeric_limits<uint32_t>::max());

		std::string Join(const std::vector<std::string>& collection, const std::string& glue);
		std::vector<std::string> Split(const std::string& value, const std::string& separator, bool keepEmpty = false);
		/// Trim string.
		std::string Trim(const std::string& source);
		/// Return string in uppercase.
		std::string ToUpper(const std::string& source);
		/// Return string in lowercase.
		std::string ToLower(const std::string& source);

		/// Return whether starts with a string.
		bool StartsWith(const std::string& str, const std::string& value, bool caseSensitive = true);
		/// Return whether ends with a string.
		bool EndsWith(const std::string& str, const std::string& value, bool caseSensitive = true);

		/// Parse a bool from the string. Is considered true if t/y/1 are found case-insensitively.
		bool ToBool(const char* str);
		/// Parse an integer from the string.
		int ToInt(const char* str);
		/// Parse an unsigned integer from the string.
		unsigned ToUInt(const char* str);
		/// parse a float from the string.
		float ToFloat(const char* str);

		/// Return comparision result with a string.
		int Compare(const std::string& str1, const std::string& str2, bool caseSensitive = true);
		int Compare(const char* str1, const char* str2, bool caseSensitive = true);

		/// Return a formatted string.
		std::string Format(const char* format, ...);

		/// Return an index to a string list corresponding to the given string, or a default value if not found. The string list must be empty-terminated.
		size_t ListIndex(const std::string& value, const std::string* strings, size_t defaultIndex, bool caseSensitive = false);
		/// Return an index to a string list corresponding to the given C string, or a default value if not found. The string list must be empty-terminated.
		size_t ListIndex(const char* value, const std::string* strings, size_t defaultIndex, bool caseSensitive = false);
		/// Return an index to a C string list corresponding to the given string, or a default value if not found. The string list must be null-terminated.
		size_t ListIndex(const std::string& value, const char** strings, size_t defaultIndex, bool caseSensitive = false);
		/// Return an index to a C string list corresponding to the given C string, or a default value if not found. The string list must be null-terminated.
		size_t ListIndex(const char* value, const char** strings, size_t defaultIndex, bool caseSensitive = false);

		/// Encode Unicode character to UTF8. Pointer will be incremented.
		void EncodeUTF8(char*& dest, unsigned unicodeChar);

		/// Append Unicode character at the end as UTF8.
		std::string& AppendUTF8(std::string& dest, unsigned unicodeChar);

		/// Return the amount of substrings split by a separator char.
		size_t CountElements(const char* str, char separator);
	}


	/// Convert a char to uppercase.
	inline char ToUpper(char c) { return (c >= 'a' && c <= 'z') ? c - 0x20 : c; }
	/// Convert a char to lowercase.
	inline char ToLower(char c) { return (c >= 'A' && c <= 'Z') ? c + 0x20 : c; }
	/// Return whether a char is an alphabet letter.
	inline bool IsAlpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
	/// Return whether a char is a digit.
	inline bool IsDigit(char c) { return c >= '0' && c <= '9'; }

}
