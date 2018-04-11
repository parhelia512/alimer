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

#include "../IO/Console.h"
#include "../IO/File.h"
#include "../Thread/Thread.h"
#include "../Thread/Timer.h"
#include "Log.h"

#include <cstdio>
#include <ctime>

namespace Alimer
{
	const char* logLevelPrefixes[] =
	{
		"DEBUG",
		"INFO",
		"WARNING",
		"ERROR",
		nullptr
	};

	Log::Log() :
#ifdef _DEBUG
		level(LOG_DEBUG),
#else
		level(LOG_INFO),
#endif
		timeStamp(false),
		inWrite(false),
		quiet(false)
	{
		RegisterSubsystem(this);
	}

	Log::~Log()
	{
		Close();
		RemoveSubsystem(this);
	}

	void Log::Open(const String& fileName)
	{
		if (fileName.IsEmpty())
			return;

		if (_logFile && _logFile->IsOpen())
		{
			if (_logFile->Name() == fileName)
				return;

			Close();
		}

		_logFile = std::make_unique<File>();
		if (_logFile->Open(fileName, FILE_WRITE))
		{
			LOGINFO("Opened log file " + fileName);
		}
		else
		{
			_logFile.reset();
			LOGERROR("Failed to create log file " + fileName);
		}
	}

	void Log::Close()
	{
		if (_logFile && _logFile->IsOpen())
		{
			_logFile->Close();
			_logFile.reset();
		}
	}

	void Log::SetLevel(int newLevel)
	{
		assert(newLevel >= LOG_DEBUG && newLevel < LOG_NONE);

		level = newLevel;
	}

	void Log::SetTimeStamp(bool enable)
	{
		timeStamp = enable;
	}

	void Log::SetQuiet(bool enable)
	{
		quiet = enable;
	}

	void Log::EndFrame()
	{
		std::lock_guard<std::mutex> lock(_logMutex);

		// Process messages accumulated from other threads (if any)
		while (!_threadMessages.empty())
		{
			const StoredLogMessage& stored = _threadMessages.front();

			if (stored.level != LOG_RAW)
				Write(stored.level, stored.message);
			else
				WriteRaw(stored.message, stored.error);

			_threadMessages.pop_front();
		}
	}

	void Log::Write(int msgLevel, const String& message)
	{
		assert(msgLevel >= LOG_DEBUG && msgLevel < LOG_NONE);

		Log* instance = Subsystem<Log>();
		if (!instance)
			return;

		// If not in the main thread, store message for later processing
		if (!Thread::IsMainThread())
		{
			std::lock_guard<std::mutex> lock(instance->_logMutex);
			instance->_threadMessages.push_back(StoredLogMessage(message, msgLevel, false));
			return;
		}

		// Do not log if message level excluded or if currently sending a log event
		if (instance->level > msgLevel || instance->inWrite)
			return;

		String formattedMessage = logLevelPrefixes[msgLevel];
		formattedMessage += ": " + message;
		instance->lastMessage = message;

		if (instance->timeStamp)
			formattedMessage = "[" + TimeStamp() + "] " + formattedMessage;

		if (instance->quiet)
		{
			// If in quiet mode, still print the error message to the standard error stream
			if (msgLevel == LOG_ERROR)
				PrintUnicodeLine(formattedMessage, true);
		}
		else
			PrintUnicodeLine(formattedMessage, msgLevel == LOG_ERROR);

		if (instance->_logFile)
		{
			instance->_logFile->WriteLine(formattedMessage);
			instance->_logFile->Flush();
		}

		instance->inWrite = true;

		LogMessageEvent& event = instance->logMessageEvent;
		event.message = formattedMessage;
		event.level = msgLevel;
		instance->SendEvent(event);

		instance->inWrite = false;
	}

	void Log::WriteRaw(const String& message, bool error)
	{
		Log* instance = Subsystem<Log>();
		if (!instance)
			return;

		// If not in the main thread, store message for later processing
		if (!Thread::IsMainThread())
		{
			std::lock_guard<std::mutex> lock(instance->_logMutex);
			instance->_threadMessages.push_back(StoredLogMessage(message, LOG_RAW, error));
			return;
		}

		// Prevent recursion during log event
		if (instance->inWrite)
			return;

		instance->lastMessage = message;

		if (instance->quiet)
		{
			// If in quiet mode, still print the error message to the standard error stream
			if (error)
				PrintUnicode(message, true);
		}
		else
			PrintUnicode(message, error);

		if (instance->_logFile)
		{
			instance->_logFile->Write(message.CString(), message.Length());
			instance->_logFile->Flush();
		}

		instance->inWrite = true;

		LogMessageEvent& event = instance->logMessageEvent;
		event.message = message;
		event.level = error ? LOG_ERROR : LOG_INFO;
		instance->SendEvent(event);

		instance->inWrite = false;
	}

}
