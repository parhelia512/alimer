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

#pragma once

#include "../Base/Timer.h"
#include "../Object/Object.h"

namespace Alimer
{
	/// Time module.
	class ALIMER_API Time final : public Object
	{
		ALIMER_OBJECT(Time, Object);

	public:
		/// Constructor.
		Time();
		/// Destructor.
		~Time() override;

		/// Tick one frame.
		void Update();

		/// Get timer microseconds.
		uint64_t GetMilliseconds() const { return _timer->GetMilliseconds(); }
		/// Get timer microseconds.
		uint64_t GetMicroseconds() const { return _timer->GetMicroseconds(); }

		/// Get current frame delta time.
		double GetElapsedSeconds() const { return _elapsedTime; }

		/// Get total number of Update method calls.
		uint32_t GetFrameCount() const { return _frameCount; }

	private:
		/// Timer.
		std::unique_ptr<Timer> _timer;

		/// Last frame time.
		uint64_t _lastFrameTime;

		/// Elased time since last frame in seconds.
		double _elapsedTime{ };

		uint32_t _frameCount{};
	};
}
