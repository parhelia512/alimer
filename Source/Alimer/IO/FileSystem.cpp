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

#include "../Base/Vector.h"
#include "File.h"
#include "FileSystem.h"

#include <cstdio>
#include <cstring>
#include <sys/stat.h>
#include <memory>

#ifdef _WIN32
#	undef DeleteFile
#	include <Windows.h>
#	include <sys/types.h>
#	include <sys/utime.h>
#else
#	include <dirent.h>
#	include <errno.h>
#	include <unistd.h>
#	include <utime.h>
#	include <sys/wait.h>
#	define MAX_PATH 256
#endif

#if defined(__APPLE__)
#	include <mach-o/dyld.h>
#endif

using namespace std;

namespace Alimer
{
#ifndef _WIN32
	bool SetCurrentDir(const string& pathName)
	{
		if (chdir(NativePath(pathName).c_str()) != 0)
			return false;

		return true;
	}
#endif

	bool CreateDir(const string& pathName)
	{
#ifdef _WIN32
		bool success = (CreateDirectoryW(WideNativePath(RemoveTrailingSlash(pathName)).c_str(), 0) == TRUE) ||
			(GetLastError() == ERROR_ALREADY_EXISTS);
#else
		bool success = mkdir(NativePath(RemoveTrailingSlash(pathName)).c_str(), S_IRWXU) == 0 || errno == EEXIST;
#endif

		return success;
	}

	bool CopyFile(const string& srcFileName, const string& destFileName)
	{
		File srcFile(srcFileName, FILE_READ);
		if (!srcFile.IsOpen())
			return false;
		File destFile(destFileName, FILE_WRITE);
		if (!destFile.IsOpen())
			return false;

		/// \todo Should use a fixed-size buffer to allow copying very large files
		size_t fileSize = srcFile.Size();
		std::unique_ptr<uint8_t[]> buffer(new uint8_t[fileSize]);

		size_t bytesRead = srcFile.Read(buffer.get(), fileSize);
		size_t bytesWritten = destFile.Write(buffer.get(), fileSize);
		return bytesRead == fileSize && bytesWritten == fileSize;
	}

	bool RenameFile(const string& srcFileName, const string& destFileName)
	{
#ifdef _WIN32
		return MoveFileW(WideNativePath(srcFileName).c_str(), WideNativePath(destFileName).c_str()) != 0;
#else
		return rename(NativePath(srcFileName).c_str(), NativePath(destFileName).c_str()) == 0;
#endif
	}

	bool DeleteFile(const string& fileName)
	{
#ifdef _WIN32
		return DeleteFileW(WideNativePath(fileName).c_str()) != 0;
#else
		return remove(NativePath(fileName).c_str()) == 0;
#endif
	}

	string CurrentDir()
	{
#ifdef _WIN32
		wchar_t path[MAX_PATH];
		path[0] = 0;
		GetCurrentDirectoryW(MAX_PATH, path);
		wstring wPath(path);
		return AddTrailingSlash(string(wPath.begin(), wPath.end()));
#else
		char path[MAX_PATH];
		path[0] = 0;
		getcwd(path, MAX_PATH);
		return AddTrailingSlash(string(path));
#endif
	}

	unsigned LastModifiedTime(const string& fileName)
	{
		if (fileName.empty())
			return 0;

#ifdef _WIN32
		struct _stat st;
		if (!_stat(fileName.c_str(), &st))
			return (unsigned)st.st_mtime;

		return 0;
#else
		struct stat st;
		if (!stat(fileName.c_str(), &st))
			return (unsigned)st.st_mtime;
		else
			return 0;
#endif
	}

	bool SetLastModifiedTime(const string& fileName, unsigned newTime)
	{
		if (fileName.empty())
			return false;

#ifdef WIN32
		struct _stat oldTime;
		struct _utimbuf newTimes;
		if (_stat(fileName.c_str(), &oldTime) != 0)
			return false;
		newTimes.actime = oldTime.st_atime;
		newTimes.modtime = newTime;
		return _utime(fileName.c_str(), &newTimes) == 0;
#else
		struct stat oldTime;
		struct utimbuf newTimes;
		if (stat(fileName.c_str(), &oldTime) != 0)
			return false;
		newTimes.actime = oldTime.st_atime;
		newTimes.modtime = newTime;
		return utime(fileName.c_str(), &newTimes) == 0;
#endif
	}

#ifndef WIN32
	bool FileExists(const string& fileName)
	{
		string fixedName = NativePath(RemoveTrailingSlash(fileName));

		struct stat st;
		if (stat(fixedName.CString(), &st) || st.st_mode & S_IFDIR)
			return false;

		return true;
	}

	bool DirectoryExists(const string& pathName)
	{
		// Always return true for the root directory
		if (pathName == "/")
			return true;

		string fixedName = NativePath(RemoveTrailingSlash(pathName));

		struct stat st;
		if (stat(fixedName.CString(), &st) || !(st.st_mode & S_IFDIR))
			return false;

		return true;
	}
#endif

	static void ScanDirInternal(
		vector<string>& result,
		string path,
		const string& startPath,
		const string& filter, unsigned flags, bool recursive)
	{
		path = AddTrailingSlash(path);
		string deltaPath;
		if (path.length() > startPath.length())
			deltaPath = path.substr(startPath.length());

		string filterExtension = filter.substr(filter.find('.'));
		if (filterExtension.find('*') != string::npos)
			filterExtension.clear();

#ifdef _WIN32
		WIN32_FIND_DATAA info;
		HANDLE handle = FindFirstFileA(string(path + "*").c_str(), &info);
		if (handle != INVALID_HANDLE_VALUE)
		{
			do
			{
				string fileName(info.cFileName);
				if (!fileName.empty())
				{
					if (info.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN && !(flags & SCAN_HIDDEN))
						continue;
					if (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						if (flags & SCAN_DIRS)
							result.push_back(deltaPath + fileName);
						if (recursive && fileName != "." && fileName != "..")
							ScanDirInternal(result, path + fileName, startPath, filter, flags, recursive);
					}
					else if (flags & SCAN_FILES)
					{
						if (filterExtension.empty() || str::EndsWith(fileName, filterExtension))
							result.push_back(deltaPath + fileName);
					}
				}
			} while (FindNextFileA(handle, &info));

			FindClose(handle);
		}
#else
		DIR *dir;
		struct dirent *de;
		struct stat st;
		dir = opendir(NativePath(path).CString());
		if (dir)
		{
			while ((de = readdir(dir)))
			{
				/// \todo Filename may be unnormalized Unicode on Mac OS X. Re-normalize as necessary
				string fileName(de->d_name);
				bool normalEntry = fileName != "." && fileName != "..";
				if (normalEntry && !(flags & SCAN_HIDDEN) && fileName.StartsWith("."))
					continue;
				string pathAndName = path + fileName;
				if (!stat(pathAndName.CString(), &st))
				{
					if (st.st_mode & S_IFDIR)
					{
						if (flags & SCAN_DIRS)
							result.Push(deltaPath + fileName);
						if (recursive && normalEntry)
							ScanDirInternal(result, path + fileName, startPath, filter, flags, recursive);
					}
					else if (flags & SCAN_FILES)
					{
						if (filterExtension.IsEmpty() || fileName.EndsWith(filterExtension))
							result.Push(deltaPath + fileName);
					}
				}
			}
			closedir(dir);
		}
#endif
	}

	void ScanDir(
		vector<string>& result,
		const string& pathName,
		const string& filter, unsigned flags, bool recursive)
	{
		string initialPath = AddTrailingSlash(pathName);
		ScanDirInternal(result, initialPath, initialPath, filter, flags, recursive);
	}

#if !ALIMER_PLATFORM_WINDOWS && !ALIMER_PLATFORM_UWP
	string ExecutableDir()
	{
#if defined(__APPLE__)
		char exeName[MAX_PATH];
		memset(exeName, 0, MAX_PATH);
		unsigned size = MAX_PATH;
		_NSGetExecutablePath(exeName, &size);
		ret = Path(String(exeName));
#elif defined(__linux__)
		char exeName[MAX_PATH];
		memset(exeName, 0, MAX_PATH);
		pid_t pid = getpid();
		String link = "/proc/" + String(pid) + "/exe";
		readlink(link.CString(), exeName, MAX_PATH);
		ret = Path(String(exeName));
#endif

		// Sanitate /./ construct away
		str::Replace(ret, "/./", "/");
		return ret;
	}
#endif

	void SplitPath(const string& fullPath, string& pathName, string& fileName, string& extension, bool lowercaseExtension)
	{
		string fullPathCopy = NormalizePath(fullPath);

		size_t extPos = fullPathCopy.find_last_of('.');
		size_t pathPos = fullPathCopy.find_last_of('/');

		if (extPos != string::npos
			&& (pathPos == string::npos || extPos > pathPos))
		{
			extension = fullPathCopy.substr(extPos);
			if (lowercaseExtension)
				extension = str::ToLower(extension);
			fullPathCopy = fullPathCopy.substr(0, extPos);
		}
		else
			extension.clear();

		pathPos = fullPathCopy.find_last_of('/');
		if (pathPos != string::npos)
		{
			fileName = fullPathCopy.substr(pathPos + 1);
			pathName = fullPathCopy.substr(0, pathPos + 1);
		}
		else
		{
			fileName = fullPathCopy;
			pathName.clear();
		}
	}

	string GetPath(const string& fullPath)
	{
		string path, file, extension;
		SplitPath(fullPath, path, file, extension);
		return path;
	}

	string GetFileName(const string& fullPath)
	{
		string path, file, extension;
		SplitPath(fullPath, path, file, extension);
		return file;
	}

	string GetExtension(const string& fullPath, bool lowercaseExtension)
	{
		string path, file, extension;
		SplitPath(fullPath, path, file, extension, lowercaseExtension);
		return extension;
	}

	string GetFileNameAndExtension(const string& fileName, bool lowercaseExtension)
	{
		string path, file, extension;
		SplitPath(fileName, path, file, extension, lowercaseExtension);
		return file + extension;
	}

	string ReplaceExtension(const string& fullPath, const string& newExtension)
	{
		string path, file, extension;
		SplitPath(fullPath, path, file, extension);
		return path + file + newExtension;
	}

	string AddTrailingSlash(const string& pathName)
	{
		string ret = str::Trim(pathName);
		str::Replace(ret, "\\", "/");
		if (!ret.empty() && ret.back() != '/')
			ret += '/';
		return ret;
	}

	string RemoveTrailingSlash(const string& pathName)
	{
		string ret = str::Trim(pathName);
		str::Replace(ret, "\\", "/");
		if (!ret.empty() && ret.back() == '/')
			ret.resize(ret.length() - 1);
		return ret;
	}

	string ParentPath(const string& path)
	{
		size_t pos = RemoveTrailingSlash(path).find_last_of('/');
		if (pos != string::npos)
			return path.substr(0, pos + 1);

		return string();
	}

	string NormalizePath(const string& pathName)
	{
		string result = pathName;
		str::Replace(result, "\\", "/");
		return result;
	}

	std::string NativePath(const std::string& pathName)
	{
#ifdef _WIN32
		string result = pathName;
		str::Replace(result, "/", "\\");
		return result;
#else
		return pathName;
#endif
	}

	wstring WideNativePath(const string& pathName)
	{
#ifdef _WIN32
		string result = pathName;
		str::Replace(result, "/", "\\");
		return wstring(result.begin(), result.end());
#else
		return wstring(pathName.begin(), pathName.end());
#endif
	}

	bool IsAbsolutePath(const string& pathName)
	{
		if (pathName.empty())
			return false;

		string path = NormalizePath(pathName);

		if (path[0] == '/')
			return true;

#ifdef _WIN32
		if (path.length() > 1 && IsAlpha(path[0]) && path[1] == ':')
			return true;
#endif

		return false;
	}

}
