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

#include "Timer.h"

#include <ctime>

#ifdef _WIN32
#	include <Windows.h>
#	include <MMSystem.h>
#else
#	include <sys/time.h>
#	include <unistd.h>
#endif

namespace Alimer
{

	/// \cond PRIVATE
	class TimerInitializer
	{
	public:
		TimerInitializer()
		{
			HiresTimer::Initialize();
#ifdef _WIN32
			timeBeginPeriod(1);
#endif
		}

		~TimerInitializer()
		{
#ifdef _WIN32
			timeEndPeriod(1);
#endif
		}
	};
	/// \endcond

	static TimerInitializer initializer;

	bool HiresTimer::supported = false;
	long long HiresTimer::frequency = 1000;

	Timer::Timer()
	{
		Reset();
	}

	unsigned Timer::ElapsedMSec()
	{
#ifdef _WIN32
		unsigned currentTime = timeGetTime();
#else
		struct timeval time;
		gettimeofday(&time, 0);
		unsigned currentTime = time.tv_sec * 1000 + time.tv_usec / 1000;
#endif

		return currentTime - startTime;
	}

	void Timer::Reset()
	{
#ifdef _WIN32
		startTime = timeGetTime();
#else
		struct timeval time;
		gettimeofday(&time, 0);
		startTime = time.tv_sec * 1000 + time.tv_usec / 1000;
#endif
	}

	HiresTimer::HiresTimer()
	{
		Reset();
	}

	long long HiresTimer::ElapsedUSec()
	{
		long long currentTime;

#ifdef _WIN32
		if (supported)
		{
			LARGE_INTEGER counter;
			QueryPerformanceCounter(&counter);
			currentTime = counter.QuadPart;
		}
		else
			currentTime = timeGetTime();
#else
		struct timeval time;
		gettimeofday(&time, 0);
		currentTime = time.tv_sec * 1000000LL + time.tv_usec;
#endif

		long long elapsedTime = currentTime - startTime;

		// Correct for possible weirdness with changing internal frequency
		if (elapsedTime < 0)
			elapsedTime = 0;

		return (elapsedTime * 1000000LL) / frequency;
	}

	void HiresTimer::Reset()
	{
#ifdef _WIN32
		if (supported)
		{
			LARGE_INTEGER counter;
			QueryPerformanceCounter(&counter);
			startTime = counter.QuadPart;
		}
		else
			startTime = timeGetTime();
#else
		struct timeval time;
		gettimeofday(&time, 0);
		startTime = time.tv_sec * 1000000LL + time.tv_usec;
#endif
	}

	void HiresTimer::Initialize()
	{
#ifdef _WIN32
		LARGE_INTEGER frequency_;
		if (QueryPerformanceFrequency(&frequency_))
		{
			frequency = frequency_.QuadPart;
			supported = true;
		}
#else
		frequency = 1000000;
		supported = true;
#endif
	}

	std::string TimeStamp()
	{
		time_t sysTime;
		time(&sysTime);
		std::string dateTime = ctime(&sysTime);
		return str::Replace(dateTime, "\n", "");
	}
}
