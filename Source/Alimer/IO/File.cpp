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

#include "../Base/Utils.h"
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


	File::File(const string& fileName, FileMode mode)
	{
		Open(fileName, mode);
	}

	File::~File()
	{
		Close();
	}

	bool File::Open(const string& fileName, FileMode mode)
	{
		Close();

		if (fileName.empty())
			return false;

#ifdef _WIN32
		_handle = _wfopen(WideNativePath(fileName).c_str(), openModes[ecast(mode)]);
#else
		_handle = fopen(NativePath(fileName).CString(), openModes[ecast(mode)]);
#endif

		// If file did not exist in readwrite mode, retry with write-update mode
		if (mode == FileMode::ReadWrite && !_handle)
		{
#ifdef _WIN32
			_handle = _wfopen(WideNativePath(fileName).c_str(), openModes[ecast(mode) + 1]);
#else
			_handle = fopen(NativePath(fileName).c_str(), openModes[ecast(mode) + 1]);
#endif
		}

		if (!_handle)
			return false;

		_name = fileName;
		_mode = mode;
		_position = 0;
		_readSyncNeeded = false;
		_writeSyncNeeded = false;

		fseek((FILE*)_handle, 0, SEEK_END);
		_size = ftell((FILE*)_handle);
		fseek((FILE*)_handle, 0, SEEK_SET);
		return true;
	}

	size_t File::Read(void* dest, size_t numBytes)
	{
		if (!_handle || _mode == FileMode::Write)
			return 0;

		if (numBytes + _position > _size)
			numBytes = _size - _position;
		if (!numBytes)
			return 0;

		// Need to reassign the position due to internal buffering when transitioning from writing to reading
		if (_readSyncNeeded)
		{
			fseek((FILE*)_handle, (long)_position, SEEK_SET);
			_readSyncNeeded = false;
		}

		size_t ret = fread(dest, numBytes, 1, (FILE*)_handle);
		if (ret != 1)
		{
			// If error, return to the position where the read began
			fseek((FILE*)_handle, (long)_position, SEEK_SET);
			return 0;
		}

		_writeSyncNeeded = true;
		_position += numBytes;
		return numBytes;
	}

	size_t File::Seek(size_t newPosition)
	{
		if (!_handle)
			return 0;

		// Allow sparse seeks if writing
		if (_mode == FileMode::Read && newPosition > _size)
			newPosition = _size;

		fseek((FILE*)_handle, (long)newPosition, SEEK_SET);
		_position = newPosition;
		_readSyncNeeded = false;
		_writeSyncNeeded = false;
		return _position;
	}

	size_t File::Write(const void* data, size_t numBytes)
	{
		if (!_handle || _mode == FileMode::Read)
			return 0;

		if (!numBytes)
			return 0;

		// Need to reassign the position due to internal buffering when transitioning from reading to writing
		if (_writeSyncNeeded)
		{
			fseek((FILE*)_handle, (long)_position, SEEK_SET);
			_writeSyncNeeded = false;
		}

		if (fwrite(data, numBytes, 1, (FILE*)_handle) != 1)
		{
			// If error, return to the position where the write began
			fseek((FILE*)_handle, (long)_position, SEEK_SET);
			return 0;
		}

		_readSyncNeeded = true;
		_position += numBytes;
		if (_position > _size)
			_size = _position;

		return _size;
	}

	bool File::IsReadable() const
	{
		return _handle != nullptr && _mode != FileMode::Write;
	}

	bool File::IsWritable() const
	{
		return _handle != nullptr && _mode != FileMode::Read;
	}

	void File::Close()
	{
		if (_handle)
		{
			fclose((FILE*)_handle);
			_handle = nullptr;
			_position = 0;
			_size = 0;
		}
	}

	void File::Flush()
	{
		if (_handle)
			fflush((FILE*)_handle);
	}

	bool File::IsOpen() const
	{
		return _handle != nullptr;
	}
}
