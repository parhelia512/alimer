//
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

#include "Time.h"
#include "Debug/Log.h"
#include "Debug/Profiler.h"

namespace Alimer
{
	Time::Time()
		: _timer(new Timer())
	{
		// Register self as a subsystem.
		RegisterSubsystem(this);

		_lastFrameTime = _timer->GetMicroseconds();
	}

	Time::~Time()
	{
		
	}

	void Time::Update()
	{
		uint64_t currentFrameTime = _timer->GetMicroseconds();
		uint64_t timeDelta = currentFrameTime - _lastFrameTime;

		_elapsedTime = timeDelta * 1.0 / 1000000.0;

		_lastFrameTime = currentFrameTime;
		_frameCount++;
	}
}
