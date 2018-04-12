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

#include "../../PlatformIncl.h"

#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
#include "../../Base/String.h"
#include "../FileSystem.h"
using namespace std;

namespace Alimer
{
	inline std::wstring ToWide(const std::string& str)
	{
		int len;
		int slength = (int)str.length() + 1;
		len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, nullptr, 0);
		wchar_t* buf = new wchar_t[len];
		MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, buf, len);
		std::wstring r(buf);
		delete[] buf;
		return r;
	}

	inline std::string ToMultibyte(const std::wstring& str)
	{
		int len;
		int slength = (int)str.length() + 1;
		len = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), slength, nullptr, 0, nullptr, nullptr);
		char* buf = new char[len];
		WideCharToMultiByte(CP_UTF8, 0, str.c_str(), slength, buf, len, nullptr, nullptr);
		std::string r(buf);
		delete[] buf;
		return r;
	}

	wstring ToWinNativePath(const string& pathName)
	{
		string result = pathName;
		str::Replace(result, "/", "\\");
		return ToWide(result);
	}

	bool SetCurrentDir(const string& pathName)
	{
		if (SetCurrentDirectoryW(ToWinNativePath(pathName).c_str()) == FALSE)
		{
			return false;
		}

		return true;
	}

	bool FileExists(const string& fileName)
	{
		string fixedName = NativePath(RemoveTrailingSlash(fileName));

		DWORD attributes = GetFileAttributesW(ToWide(fixedName).c_str());
		if (attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY))
			return true;

		return false;
	}

	bool DirectoryExists(const string& pathName)
	{
		string fixedName = NativePath(RemoveTrailingSlash(pathName));

		DWORD attributes = GetFileAttributesW(ToWide(fixedName).c_str());
		return (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY));
	}

	string GetExecutableDir()
	{
		wchar_t exeName[MAX_PATH];
		exeName[0] = 0;
		GetModuleFileNameW(0, exeName, MAX_PATH);

		// Convert to multbyte.
		string result = GetPath(ToMultibyte(exeName));

		// Sanitate /./ construct away
		str::Replace(result, "/./", "/");
		return result;
	}
}

#endif