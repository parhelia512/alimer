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

#include "../Base/String.h"
#include "../Base/Vector.h"
#include "../Base/WString.h"

namespace Alimer
{

	/// Return files.
	static const unsigned SCAN_FILES = 0x1;
	/// Return directories.
	static const unsigned SCAN_DIRS = 0x2;
	/// Return also hidden files.
	static const unsigned SCAN_HIDDEN = 0x4;

	/// Set the current working directory.
	ALIMER_API bool SetCurrentDir(const String& pathName);
	/// Create a directory.
	ALIMER_API bool CreateDir(const String& pathName);
	/// Copy a file. Return true on success.
	ALIMER_API bool CopyFile(const String& srcFileName, const String& destFileName);
	/// Rename a file. Return true on success.
	ALIMER_API bool RenameFile(const String& srcFileName, const String& destFileName);
	/// Delete a file. Return true on success.
	ALIMER_API bool DeleteFile(const String& fileName);
	/// Return the absolute current working directory.
	ALIMER_API String CurrentDir();
	/// Return the file's last modified time as seconds since epoch, or 0 if can not be accessed.
	ALIMER_API unsigned LastModifiedTime(const String& fileName);
	/// Set the file's last modified time as seconds since epoch. Return true on success.
	ALIMER_API bool SetLastModifiedTime(const String& fileName, unsigned newTime);
	/// Check if a file exists.
	ALIMER_API bool FileExists(const String& fileName);
	/// Check if a directory exists.
	ALIMER_API bool DirExists(const String& pathName);
	/// Scan a directory for specified files.
	ALIMER_API void ScanDir(Vector<String>& result, const String& pathName, const String& filter, unsigned flags = SCAN_FILES, bool recursive = false);
	/// Return the executable's directory.
	ALIMER_API String ExecutableDir();
	/// Split a full path to path, filename and extension. The extension will be converted to lowercase by default.
	ALIMER_API void SplitPath(const String& fullPath, String& pathName, String& fileName, String& extension, bool lowercaseExtension = true);
	/// Return the path from a full path.
	ALIMER_API String Path(const String& fullPath);
	/// Return the filename from a full path.
	ALIMER_API String FileName(const String& fullPath);
	/// Return the extension from a full path, converted to lowercase by default.
	ALIMER_API String Extension(const String& fullPath, bool lowercaseExtension = true);
	/// Return the filename and extension from a full path. The case of the extension is preserved by default, so that the file can be opened in case-sensitive operating systems.
	ALIMER_API String FileNameAndExtension(const String& fullPath, bool lowercaseExtension = false);
	/// Replace the extension of a file name with another.
	ALIMER_API String ReplaceExtension(const String& fullPath, const String& newExtension);
	/// Add a slash at the end of the path if missing and convert to normalized format (use slashes.)
	ALIMER_API String AddTrailingSlash(const String& pathName);
	/// Remove the slash from the end of a path if exists and convert to normalized format (use slashes.)
	ALIMER_API String RemoveTrailingSlash(const String& pathName);
	/// Return the parent path, or the path itself if not available.
	ALIMER_API String ParentPath(const String& pathName);
	/// Convert a path to normalized (internal) format which uses slashes.
	ALIMER_API String NormalizePath(const String& pathName);
	/// Convert a path to the format required by the operating system.
	ALIMER_API String NativePath(const String& pathName);
	/// Convert a path to the format required by the operating system in wide characters.
	ALIMER_API WString WideNativePath(const String& pathName);
	/// Return whether a path is absolute.
	ALIMER_API bool IsAbsolutePath(const String& pathName);

}
