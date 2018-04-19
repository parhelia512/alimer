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
#include "Log.h"

#include <spdlog/spdlog.h>

#if ALIMER_PLATFORM_WINDOWS
#	ifdef _MSC_VER
#		include "spdlog/sinks/msvc_sink.h"

namespace spdlog_private
{
	using AlimerPlatformSink = spdlog::sinks::msvc_sink_mt;
}

#	else
#	include "spdlog/sinks/wincolor_sink.h"

namespace spdlog_private
{
	using AlimerPlatformSink = spdlog::sinks::wincolor_stdout_sink_mt;
}

#	endif
#elif ALIMER_PLATFORM_LINUX || ALIMER_PLATFORM_APPLE
#	include "spdlog/sinks/stdout_sinks.h"

namespace spdlog_private
{
	using AlimerPlatformSink = spdlog::sinks::stdout_sink_mt;
}

#elif ALIMER_PLATFORM_ANDROID
#	include "spdlog/sinks/android_sink.h"

namespace spdlog_private
{
	using AlimerPlatformSink = spdlog::sinks::android_sink_mt;
}

#else
#	include "spdlog/sinks/stdout_sinks.h"
namespace spdlog_private
{
	using AlimerPlatformSink = spdlog::sinks::stdout_sink_mt;
}
#endif

#include "spdlog/sinks/dist_sink.h"
#include "spdlog/sinks/file_sinks.h"

using namespace spdlog;

namespace Alimer
{
	Log::Log()
	{
		// Create custom log container
		auto logContainer = std::make_shared<sinks::dist_sink_mt>();;
		logContainer->add_sink(std::make_shared<spdlog_private::AlimerPlatformSink>());
		logContainer->add_sink(std::make_shared<sinks::daily_file_sink_mt>("AlimerLog", 23, 59));
#ifdef _DEBUG
#if ALIMER_PLATFORM_WINDOWS
		AllocConsole();
		logContainer->add_sink(std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>());
#else
		logContainer->add_sink(std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>());
#endif
#endif

		_logger = spdlog::create(ALIMER_LOG, logContainer);

#ifdef _DEBUG
		SetLevel(LogLevel::Debug);
#else
		SetLevel(LogLevel::Info);
#endif

		RegisterSubsystem(this);
	}

	Log::~Log()
	{
		RemoveSubsystem(this);
	}

	void Log::SetLevel(LogLevel newLevel)
	{
		_level = newLevel;
		spdlog::set_level(static_cast<spdlog::level::level_enum>(newLevel));
	}
}
