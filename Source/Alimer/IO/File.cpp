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

#include "File.h"
#include "FileSystem.h"

#include <cstdio>
#ifdef _WIN32
#include <Windows.h>
#endif
using namespace std;

namespace Alimer
{
#ifdef _WIN32
	static const wchar_t* openModes[] =
	{
		L"rb",
		L"wb",
		L"r+b",
		L"w+b"
	};

	
#else
	static const char* openModes[] =
	{
		"rb",
		"wb",
		"r+b",
		"w+b"
	};
#endif

	File::File() :
		mode(FILE_READ),
		handle(nullptr),
		readSyncNeeded(false),
		writeSyncNeeded(false)
	{
	}

	File::File(const string& fileName, FileMode mode) :
		mode(FILE_READ),
		handle(nullptr),
		readSyncNeeded(false),
		writeSyncNeeded(false)
	{
		Open(fileName, mode);
	}

	File::~File()
	{
		Close();
	}

	bool File::Open(const string& fileName, FileMode fileMode)
	{
		Close();

		if (fileName.empty())
			return false;

#ifdef _WIN32
		handle = _wfopen(WideNativePath(fileName).c_str(), openModes[fileMode]);
#else
		handle = fopen(NativePath(fileName).CString(), openModes[fileMode]);
#endif

		// If file did not exist in readwrite mode, retry with write-update mode
		if (mode == FILE_READWRITE && !handle)
		{
#ifdef _WIN32
			handle = _wfopen(WideNativePath(fileName).c_str(), openModes[fileMode + 1]);
#else
			handle = fopen(NativePath(fileName).c_str(), openModes[fileMode + 1]);
#endif
		}

		if (!handle)
			return false;

		_name = fileName;
		mode = fileMode;
		_position = 0;
		readSyncNeeded = false;
		writeSyncNeeded = false;

		fseek((FILE*)handle, 0, SEEK_END);
		_size = ftell((FILE*)handle);
		fseek((FILE*)handle, 0, SEEK_SET);
		return true;
	}

	size_t File::Read(void* dest, size_t numBytes)
	{
		if (!handle || mode == FILE_WRITE)
			return 0;

		if (numBytes + _position > _size)
			numBytes = _size - _position;
		if (!numBytes)
			return 0;

		// Need to reassign the position due to internal buffering when transitioning from writing to reading
		if (readSyncNeeded)
		{
			fseek((FILE*)handle, (long)_position, SEEK_SET);
			readSyncNeeded = false;
		}

		size_t ret = fread(dest, numBytes, 1, (FILE*)handle);
		if (ret != 1)
		{
			// If error, return to the position where the read began
			fseek((FILE*)handle, (long)_position, SEEK_SET);
			return 0;
		}

		writeSyncNeeded = true;
		_position += numBytes;
		return numBytes;
	}

	size_t File::Seek(size_t newPosition)
	{
		if (!handle)
			return 0;

		// Allow sparse seeks if writing
		if (mode == FILE_READ && newPosition > _size)
			newPosition = _size;

		fseek((FILE*)handle, (long)newPosition, SEEK_SET);
		_position = newPosition;
		readSyncNeeded = false;
		writeSyncNeeded = false;
		return _position;
	}

	size_t File::Write(const void* data, size_t numBytes)
	{
		if (!handle || mode == FILE_READ)
			return 0;

		if (!numBytes)
			return 0;

		// Need to reassign the position due to internal buffering when transitioning from reading to writing
		if (writeSyncNeeded)
		{
			fseek((FILE*)handle, (long)_position, SEEK_SET);
			writeSyncNeeded = false;
		}

		if (fwrite(data, numBytes, 1, (FILE*)handle) != 1)
		{
			// If error, return to the position where the write began
			fseek((FILE*)handle, (long)_position, SEEK_SET);
			return 0;
		}

		readSyncNeeded = true;
		_position += numBytes;
		if (_position > _size)
			_size = _position;

		return _size;
	}

	bool File::IsReadable() const
	{
		return handle != 0 && mode != FILE_WRITE;
	}

	bool File::IsWritable() const
	{
		return handle != 0 && mode != FILE_READ;
	}

	void File::Close()
	{
		if (handle)
		{
			fclose((FILE*)handle);
			handle = 0;
			_position = 0;
			_size = 0;
		}
	}

	void File::Flush()
	{
		if (handle)
			fflush((FILE*)handle);
	}

	bool File::IsOpen() const
	{
		return handle != 0;
	}

}
