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

#include <spdlog/spdlog.h>
#include "../Object/Object.h"

namespace Alimer
{
	enum class LogLevel : uint8_t
	{
		Trace = 0,
		Debug = 1,
		Info = 2,
		Warn = 3,
		Error = 4,
		Critical = 5,
		Off = 6
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

		/// Set logging level.
		void SetLevel(LogLevel newLevel);

		/// Return logging level.
		LogLevel GetLevel() const { return _level; }

	private:
		LogLevel _level;
		std::shared_ptr<spdlog::logger> _logger;
	};

}

#ifdef ALIMER_LOGGING

#	define ALIMER_LOG "Log"
#	define ALIMER_LOGTRACE(...) spdlog::get(ALIMER_LOG)->trace(__VA_ARGS__)
#	define ALIMER_LOGDEBUG(...) spdlog::get(ALIMER_LOG)->debug(__VA_ARGS__)
#	define ALIMER_LOGINFO(...) spdlog::get(ALIMER_LOG)->info(__VA_ARGS__)
#	define ALIMER_LOGWARN(...) spdlog::get(ALIMER_LOG)->warn(__VA_ARGS__)
#	define ALIMER_LOGERROR(...) spdlog::get(ALIMER_LOG)->error(__VA_ARGS__)
#	define ALIMER_LOGCRITICAL(...) spdlog::get(ALIMER_LOG)->critical(__VA_ARGS__)

#else

#	define ALIMER_LOGTRACE(...) ((void)0)
#	define ALIMER_LOGDEBUG(...) ((void)0)
#	define ALIMER_LOGINFO(...) ((void)0)
#	define ALIMER_LOGWARN(...) ((void)0)
#	define ALIMER_LOGERROR(...) ((void)0)
#	define ALIMER_LOGCRITICAL(...) ((void)0)
#endif
