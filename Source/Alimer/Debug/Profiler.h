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

#include "../Math/Math.h"
#include "../Object/Object.h"
#include "../Thread/Timer.h"
#include <memory>

namespace Alimer
{
	/// Profiling data for one block in the profiling tree.
	class ALIMER_API ProfilerBlock
	{
	public:
		/// Construct-
		ProfilerBlock(ProfilerBlock* parent, const char* name);
		/// Destruct.
		~ProfilerBlock();

		/// Start time measurement and increment call count.
		void Begin();
		/// End time measurement.
		void End();
		/// Process stats at the end of frame.
		void EndFrame();
		/// Begin an interval lasting several frames.
		void BeginInterval();
		/// Return a child block; create if necessary.
		ProfilerBlock* FindOrCreateChild(const char* name);

		/// Block name.
		const char* name;
		/// Hires timer for time measurement.
		HiresTimer timer;
		/// Parent block.
		ProfilerBlock* parent;
		/// Child blocks.
		std::vector<std::unique_ptr<ProfilerBlock>> _children;
		/// Current frame's accumulated time.
		long long time;
		/// Current frame's longest call.
		long long maxTime;
		/// Current frame's call count.
		unsigned count;
		/// Previous frame's accumulated time.
		long long frameTime;
		/// Previous frame's longest call.
		long long frameMaxTime;
		/// Previous frame's call count.
		unsigned frameCount;
		/// Current interval's accumulated time.
		long long intervalTime;
		/// Current interval's longest call.
		long long intervalMaxTime;
		/// Current interval's call count.
		unsigned intervalCount;
		/// Accumulated time since start.
		long long totalTime;
		/// Longest call since start.
		long long totalMaxTime;
		/// Call count since start.
		unsigned totalCount;
	};

	/// Hierarchical performance profiler subsystem.
	class ALIMER_API Profiler : public Object
	{
		OBJECT(Profiler);

	public:
		/// Construct.
		Profiler();
		/// Destruct.
		~Profiler();

		/// Begin a profiling block. The name must be persistent; string literals are recommended.
		void BeginBlock(const char* name);
		/// End the current profiling block.
		void EndBlock();
		/// Begin the next profiling frame.
		void BeginFrame();
		/// End the current profiling frame.
		void EndFrame();
		/// Begin a profiler interval.
		void BeginInterval();

		/// Output results into a string.
		std::string OutputResults(bool showUnused = false, bool showTotal = false, size_t maxDepth = M_MAX_UNSIGNED) const;
		/// Return the current profiling block.
		const ProfilerBlock* CurrentBlock() const { return current; }
		/// Return the root profiling block.
		const ProfilerBlock* GetRootBlock() const { return _root.get(); }

	private:
		/// Output results recursively.
		void OutputResults(ProfilerBlock* block, std::string& output, size_t depth, size_t maxDepth, bool showUnused, bool showTotal) const;

		/// Current profiling block.
		ProfilerBlock* current;
		/// Root profiling block.
		std::unique_ptr<ProfilerBlock> _root;
		/// Frames in the current interval.
		size_t intervalFrames;
		/// Total frames since start.
		size_t totalFrames;
	};

	/// Helper class for automatically beginning and ending a profiling block
	class ALIMER_API AutoProfileBlock
	{
	public:
		/// Construct and begin a profiling block. The name must be persistent; string literals are recommended.
		AutoProfileBlock(const char* name)
		{
			_profiler = Object::Subsystem<Profiler>();
			if (_profiler)
				_profiler->BeginBlock(name);
		}

		/// Destruct. End the profiling block.
		~AutoProfileBlock()
		{
			if (_profiler)
				_profiler->EndBlock();
		}

	private:
		/// Profiler subsystem.
		Profiler * _profiler;
	};

#ifdef ALIMER_PROFILING
#	define ALIMER_PROFILE(name) Alimer::AutoProfileBlock profile_ ## name (#name)
#else
#	define ALIMER_PROFILE(name)
#endif

}
