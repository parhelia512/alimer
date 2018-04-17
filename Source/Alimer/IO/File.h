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

#include "Stream.h"

namespace Alimer
{
	/// File open mode.
	enum class FileMode : uint32_t
	{
		Read = 0,
		Write = 1,
		ReadWrite = 2
	};

	class PackageFile;

	/// Filesystem file.
	class ALIMER_API File : public Stream
	{
	public:
		/// Construct.
		File() = default;
		/// Construct and open a file.
		File(const std::string& fileName, FileMode mode = FileMode::Read);
		/// Destruct. Close the file if open.
		~File();

		/// Read bytes from the file. Return number of bytes actually read.
		size_t Read(void* dest, size_t numBytes) override;
		/// Set position in bytes from the beginning of the file.
		size_t Seek(size_t newPosition) override;
		/// Write bytes to the file. Return number of bytes actually written.
		size_t Write(const void* data, size_t numBytes) override;
		/// Return whether read operations are allowed.
		bool IsReadable() const override;
		/// Return whether write operations are allowed.
		bool IsWritable() const override;

		/// Open a file. Return true on success.
		bool Open(const std::string& fileName, FileMode mode = FileMode::Read);
		/// Close the file.
		void Close();
		/// Flush any buffered output to the file.
		void Flush();

		/// Return the open mode.
		FileMode GetMode() const { return _mode; }
		/// Return whether is open.
		bool IsOpen() const;
		/// Return the file handle.
		void* GetHandle() const { return _handle; }

		using Stream::Read;
		using Stream::Write;

	private:
		/// Open mode.
		FileMode _mode{ FileMode::Read };
		/// File handle.
		void* _handle = nullptr;
		/// Synchronization needed before read -flag.
		bool _readSyncNeeded{};
		/// Synchronization needed before write -flag.
		bool _writeSyncNeeded{};
	};

}
