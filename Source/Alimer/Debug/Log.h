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

#include "../Object/Object.h"
#include <list>
#include <mutex>
#include <memory>

namespace Alimer
{

	/// Fictional message level to indicate a stored raw message.
	static const int LOG_RAW = -1;
	/// Debug message level. By default only shown in debug mode.
	static const int LOG_DEBUG = 0;
	/// Informative message level.
	static const int LOG_INFO = 1;
	/// Warning message level.
	static const int LOG_WARNING = 2;
	/// Error message level.
	static const int LOG_ERROR = 3;
	/// Disable all log messages.
	static const int LOG_NONE = 4;

	class File;

	/// Stored log message from another thread.
	struct ALIMER_API StoredLogMessage
	{
		/// Construct undefined.
		StoredLogMessage()
		{
		}

		/// Construct with parameters.
		StoredLogMessage(const std::string& message_, int level_, bool error_) 
			: message(message_)
			, level(level_)
			, error(error_)
		{
		}

		/// Message text.
		std::string message;
		/// Message level. -1 for raw messages.
		int level;
		/// Error flag for raw messages.
		bool error;
	};

	/// %Log message event.
	class ALIMER_API LogMessageEvent : public Event
	{
	public:
		/// Message.
		std::string message;
		/// Message level.
		int level;
	};

	/// Logging subsystem.
	class ALIMER_API Log : public Object
	{
		ALIMER_OBJECT(Log, Object);

	public:
		/// Construct and register subsystem.
		Log();
		/// Destruct. Close the log file if open.
		~Log();

		/// Open the log file.
		void Open(const std::string& fileName);
		/// Close the log file.
		void Close();
		/// Set logging level.
		void SetLevel(int newLevel);
		/// Set whether to timestamp log messages.
		void SetTimeStamp(bool enable);
		/// Set quiet mode, ie. only output error messages to the standard error stream.
		void SetQuiet(bool enable);
		/// Process threaded log messages at the end of a frame.
		void EndFrame();

		/// Return logging level.
		int Level() const { return level; }
		/// Return whether log messages are timestamped.
		bool HasTimeStamp() const { return timeStamp; }
		/// Return last log message.
		std::string LastMessage() const { return lastMessage; }

		/// Write to the log. If logging level is higher than the level of the message, the message is ignored.
		static void Write(int msgLevel, const std::string& message);
		/// Write raw output to the log.
		static void WriteRaw(const std::string& message, bool error = false);

		/// %Log message event.
		LogMessageEvent logMessageEvent;

	private:
		/// Mutex for threaded operation.
		std::mutex _logMutex;
		/// %Log messages from other threads.
		std::list<StoredLogMessage> _threadMessages;
		/// %Log file.
		UniquePtr<File> _logFile;
		/// Last log message.
		std::string lastMessage;
		/// Logging level.
		int level;
		/// Use timestamps flag.
		bool timeStamp;
		/// In write flag to prevent recursion.
		bool inWrite;
		/// Quite mode flag.
		bool quiet;
	};

}

#ifdef ALIMER_LOGGING

#	define LOGDEBUG(message) Alimer::Log::Write(Alimer::LOG_DEBUG, message)
#	define LOGINFO(message) Alimer::Log::Write(Alimer::LOG_INFO, message)
#	define LOGWARNING(message) Alimer::Log::Write(Alimer::LOG_WARNING, message)
#	define LOGERROR(message) Alimer::Log::Write(Alimer::LOG_ERROR, message)
#	define LOGRAW(message) Alimer::Log::WriteRaw(message)
#	define LOGDEBUGF(format, ...) Alimer::Log::Write(Alimer::LOG_DEBUG, Alimer::str::Format(format, ##__VA_ARGS__))
#	define LOGINFOF(format, ...) Alimer::Log::Write(Alimer::LOG_INFO, Alimer::str::Format(format, ##__VA_ARGS__))
#	define LOGWARNINGF(format, ...) Alimer::Log::Write(Alimer::LOG_WARNING, Alimer::str::Format(format, ##__VA_ARGS__))
#	define LOGERRORF(format, ...) Alimer::Log::Write(Alimer::LOG_ERROR, Alimer::str::Format(format, ##__VA_ARGS__))
#	define LOGRAWF(format, ...) Alimer::Log::WriteRaw(Alimer::str::Format(format, ##__VA_ARGS__))

#else

#	define LOGDEBUG(message)
#	define LOGINFO(message)
#	define LOGWARNING(message)
#	define LOGERROR(message)
#	define LOGRAW(message)
#	define LOGDEBUGF(format, ...)
#	define LOGINFOF(format, ...)
#	define LOGWARNINGF(format, ...)
#define LOGERRORF(format, ...)
#define LOGRAWF(format, ...)

#endif
