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

#include "Profiler.h"

#include <cstdio>
#include <cstring>
using namespace std;

namespace Alimer
{
	static const int LINE_MAX_LENGTH = 256;
	static const int NAME_MAX_LENGTH = 30;

	ProfilerBlock::ProfilerBlock(ProfilerBlock* parent_, const char* name_) :
		name(name_),
		parent(parent_),
		time(0),
		maxTime(0),
		count(0),
		frameTime(0),
		frameMaxTime(0),
		frameCount(0),
		intervalTime(0),
		intervalMaxTime(0),
		intervalCount(0),
		totalTime(0),
		totalMaxTime(0),
		totalCount(0)
	{
	}

	ProfilerBlock::~ProfilerBlock()
	{
	}

	void ProfilerBlock::Begin()
	{
		timer.Reset();
		++count;
	}

	void ProfilerBlock::End()
	{
		uint64_t currentTime = timer.GetMilliseconds();
		if (currentTime > maxTime)
			maxTime = currentTime;
		time += currentTime;
	}

	void ProfilerBlock::EndFrame()
	{
		frameTime = time;
		frameMaxTime = maxTime;
		frameCount = count;
		intervalTime += time;
		if (maxTime > intervalMaxTime)
			intervalMaxTime = maxTime;
		intervalCount += count;
		totalTime += time;
		if (maxTime > totalMaxTime)
			totalMaxTime = maxTime;
		totalCount += count;
		time = 0;
		maxTime = 0;
		count = 0;

		for (auto it = _children.begin(); it != _children.end(); ++it)
			(*it)->EndFrame();
	}

	void ProfilerBlock::BeginInterval()
	{
		intervalTime = 0;
		intervalMaxTime = 0;
		intervalCount = 0;

		for (auto it = _children.begin(); it != _children.end(); ++it)
			(*it)->BeginInterval();
	}

	ProfilerBlock* ProfilerBlock::FindOrCreateChild(const char* name_)
	{
		// First check using string pointers only, then resort to actual strcmp
		for (auto it = _children.begin(); it != _children.end(); ++it)
		{
			if ((*it)->name == name_)
				return it->get();
		}

		for (auto it = _children.begin(); it != _children.end(); ++it)
		{
			if (!str::Compare((*it)->name, name_))
				return it->get();
		}

		ProfilerBlock* newBlock = new ProfilerBlock(this, name_);
		_children.push_back(UniquePtr<ProfilerBlock>(newBlock));

		return newBlock;
	}

	Profiler::Profiler() 
		: intervalFrames(0)
		, totalFrames(0)
		, _root(new ProfilerBlock(nullptr, "Root"))
	{
		_threadId = this_thread::get_id();
		current = _root.get();
		RegisterSubsystem(this);
	}

	Profiler::~Profiler()
	{
		RemoveSubsystem(this);
	}

	void Profiler::BeginBlock(const char* name)
	{
		// Currently profiling is a no-op if attempted from outside main thread
		if (_threadId != this_thread::get_id())
			return;

		current = current->FindOrCreateChild(name);
		current->Begin();
	}

	void Profiler::EndBlock()
	{
		if (_threadId != this_thread::get_id())
			return;

		if (current != _root.get())
		{
			current->End();
			current = current->parent;
		}
	}

	void Profiler::BeginFrame()
	{
		// End the previous frame if any
		EndFrame();

		BeginBlock("RunFrame");
	}

	void Profiler::EndFrame()
	{
		if (current != _root.get())
		{
			EndBlock();
			++intervalFrames;
			++totalFrames;
			_root->EndFrame();
			current = _root.get();
		}
	}

	void Profiler::BeginInterval()
	{
		_root->BeginInterval();
		intervalFrames = 0;
	}

	string Profiler::OutputResults(bool showUnused, bool showTotal, size_t maxDepth) const
	{
		string output;

		if (!showTotal)
			output += string("Block                            Cnt     Avg      Max     Frame     Total\n\n");
		else
		{
			output += string("Block                                       Last frame                       Whole execution time\n\n");
			output += string("                                 Cnt     Avg      Max      Total      Cnt      Avg       Max        Total\n\n");
		}

		if (!maxDepth)
			maxDepth = 1;

		OutputResults(_root.get(), output, 0, maxDepth, showUnused, showTotal);

		return output;
	}

	void Profiler::OutputResults(ProfilerBlock* block, string& output, size_t depth, size_t maxDepth, bool showUnused, bool showTotal) const
	{
		char line[LINE_MAX_LENGTH];
		char indentedName[LINE_MAX_LENGTH];

		size_t currentInterval = intervalFrames;
		if (!currentInterval)
			++currentInterval;

		if (depth >= maxDepth)
			return;

		// Do not print the root block as it does not collect any actual data
		if (block != _root.get())
		{
			if (showUnused || block->intervalCount || (showTotal && block->totalCount))
			{
				memset(indentedName, ' ', NAME_MAX_LENGTH);
				indentedName[depth] = 0;
				strcat(indentedName, block->name);
				indentedName[strlen(indentedName)] = ' ';
				indentedName[NAME_MAX_LENGTH] = 0;

				if (!showTotal)
				{
					float avg = (block->intervalCount ? block->intervalTime / block->intervalCount : 0.0f) / 1000.0f;
					float max = block->intervalMaxTime / 1000.0f;
					float frame = block->intervalTime / currentInterval / 1000.0f;
					float all = block->intervalTime / 1000.0f;

					sprintf(line, "%s %5u %8.3f %8.3f %8.3f %9.3f\n", indentedName, Min(block->intervalCount, 99999),
						avg, max, frame, all);
				}
				else
				{
					float avg = (block->frameCount ? block->frameTime / block->frameCount : 0.0f) / 1000.0f;
					float max = block->frameMaxTime / 1000.0f;
					float all = block->frameTime / 1000.0f;

					float totalAvg = (block->totalCount ? block->totalTime / block->totalCount : 0.0f) / 1000.0f;
					float totalMax = block->totalMaxTime / 1000.0f;
					float totalAll = block->totalTime / 1000.0f;

					sprintf(line, "%s %5u %8.3f %8.3f %9.3f  %7u %9.3f %9.3f %11.3f\n", indentedName, Min(block->frameCount, 99999),
						avg, max, all, Min(block->totalCount, 99999), totalAvg, totalMax, totalAll);
				}

				output += string(line);
			}

			++depth;
		}

		for (auto it = block->_children.begin(); it != block->_children.end(); ++it)
			OutputResults(it->get(), output, depth, maxDepth, showUnused, showTotal);
	}

}
