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

#include "../AlimerConfig.h"
#include <vector>
#include <string>

namespace Alimer
{

	/// Return files.
	static const unsigned SCAN_FILES = 0x1;
	/// Return directories.
	static const unsigned SCAN_DIRS = 0x2;
	/// Return also hidden files.
	static const unsigned SCAN_HIDDEN = 0x4;

	/// Set the current working directory.
	ALIMER_API bool SetCurrentDir(const std::string& pathName);
	/// Create a directory.
	ALIMER_API bool CreateDir(const std::string& pathName);
	/// Copy a file. Return true on success.
	ALIMER_API bool CopyFile(const std::string& srcFileName, const std::string& destFileName);
	/// Rename a file. Return true on success.
	ALIMER_API bool RenameFile(const std::string& srcFileName, const std::string& destFileName);
	/// Delete a file. Return true on success.
	ALIMER_API bool DeleteFile(const std::string& fileName);
	/// Return the absolute current working directory.
	ALIMER_API std::string CurrentDir();
	/// Return the file's last modified time as seconds since epoch, or 0 if can not be accessed.
	ALIMER_API unsigned LastModifiedTime(const std::string& fileName);
	/// Set the file's last modified time as seconds since epoch. Return true on success.
	ALIMER_API bool SetLastModifiedTime(const std::string& fileName, unsigned newTime);
	/// Check if a file exists.
	ALIMER_API bool FileExists(const std::string& fileName);
	/// Check if a directory exists.
	ALIMER_API bool DirectoryExists(const std::string& pathName);
	/// Scan a directory for specified files.
	ALIMER_API void ScanDir(std::vector<std::string>& result, const std::string& pathName, const std::string& filter, unsigned flags = SCAN_FILES, bool recursive = false);
	/// Return the executable's directory.
	ALIMER_API std::string GetExecutableDir();
	/// Split a full path to path, filename and extension. The extension will be converted to lowercase by default.
	ALIMER_API void SplitPath(const std::string& fullPath, std::string& pathName, std::string& fileName, std::string& extension, bool lowercaseExtension = true);
	/// Return the path from a full path.
	ALIMER_API std::string GetPath(const std::string& fullPath);
	/// Return the filename from a full path.
	ALIMER_API std::string GetFileName(const std::string& fullPath);
	/// Return the extension from a full path, converted to lowercase by default.
	ALIMER_API std::string GetExtension(const std::string& fullPath, bool lowercaseExtension = true);
	/// Return the filename and extension from a full path. The case of the extension is preserved by default, so that the file can be opened in case-sensitive operating systems.
	ALIMER_API std::string GetFileNameAndExtension(const std::string& fullPath, bool lowercaseExtension = false);
	/// Replace the extension of a file name with another.
	ALIMER_API std::string ReplaceExtension(const std::string& fullPath, const std::string& newExtension);
	/// Add a slash at the end of the path if missing and convert to normalized format (use slashes.)
	ALIMER_API std::string AddTrailingSlash(const std::string& pathName);
	/// Remove the slash from the end of a path if exists and convert to normalized format (use slashes.)
	ALIMER_API std::string RemoveTrailingSlash(const std::string& pathName);
	/// Return the parent path, or the path itself if not available.
	ALIMER_API std::string ParentPath(const std::string& pathName);
	/// Convert a path to normalized (internal) format which uses slashes.
	ALIMER_API std::string NormalizePath(const std::string& pathName);
	/// Convert a path to the format required by the operating system.
	ALIMER_API std::string NativePath(const std::string& pathName);
	/// Convert a path to the format required by the operating system in wide characters.
	ALIMER_API std::wstring WideNativePath(const std::string& pathName);
	/// Return whether a path is absolute.
	ALIMER_API bool IsAbsolutePath(const std::string& pathName);
}
