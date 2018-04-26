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

#include "../Base/Utils.h"
#include "Profiler.h"

#include <cstdio>
#include <cstring>
using namespace std;

namespace Alimer
{
	static const int LINE_MAX_LENGTH = 256;
	static const int NAME_MAX_LENGTH = 30;

	ProfilerBlock::ProfilerBlock(ProfilerBlock* parent_, const char* name_) 
		: name(name_)
		, parent(parent_)
		, count(0)
		, frameCount(0)
		, intervalTime(0)
		, intervalCount(0)
		, totalCount(0)
	{
	}

	ProfilerBlock::~ProfilerBlock()
	{
		for (auto i = _children.begin(); i != _children.end(); ++i)
		{
			delete *i;
			*i = nullptr;
		}
		_children.clear();
	}

	void ProfilerBlock::Begin()
	{
		timer.Reset();
		++count;
	}

	void ProfilerBlock::End()
	{
		uint64_t currentTime = timer.GetMilliseconds();
		if (currentTime > _maxTime)
		{
			_maxTime = currentTime;
		}
		_time += currentTime;
	}

	void ProfilerBlock::EndFrame()
	{
		_frameTime = _time;
		_frameMaxTime = _maxTime;
		frameCount = count;
		intervalTime += _time;
		if (_maxTime > _intervalMaxTime)
			_intervalMaxTime = _maxTime;
		intervalCount += count;
		_totalTime += _time;
		if (_maxTime > _totalMaxTime)
		{
			_totalMaxTime = _maxTime;
		}
		totalCount += count;
		_time = 0;
		_maxTime = 0;
		count = 0;

		for (auto it = _children.begin(); it != _children.end(); ++it)
			(*it)->EndFrame();
	}

	void ProfilerBlock::BeginInterval()
	{
		intervalTime = 0;
		_intervalMaxTime = 0;
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
				return *it;
		}

		for (auto it = _children.begin(); it != _children.end(); ++it)
		{
			if (!str::Compare((*it)->name, name_))
				return *it;
		}

		ProfilerBlock* newBlock = new ProfilerBlock(this, name_);
		_children.push_back(newBlock);

		return newBlock;
	}

	Profiler::Profiler() 
	{
		_threadId = this_thread::get_id();
		_current = _root = new ProfilerBlock(nullptr, "Root");
		RegisterSubsystem(this);
	}

	Profiler::~Profiler()
	{
		SafeDelete(_root);
		RemoveSubsystem(this);
	}

	void Profiler::BeginBlock(const char* name)
	{
		// Currently profiling is a no-op if attempted from outside main thread
		if (_threadId != this_thread::get_id())
			return;

		_current = _current->FindOrCreateChild(name);
		_current->Begin();
	}

	void Profiler::EndBlock()
	{
		if (_threadId != this_thread::get_id())
			return;

		if (_current != _root)
		{
			_current->End();
			_current = _current->parent;
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
		if (_current != _root)
		{
			EndBlock();
			++_intervalFrames;
			++_totalFrames;
			_root->EndFrame();
			_current = _root;
		}
	}

	void Profiler::BeginInterval()
	{
		_root->BeginInterval();
		_intervalFrames = 0;
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

		OutputResults(_root, output, 0, maxDepth, showUnused, showTotal);

		return output;
	}

	void Profiler::OutputResults(ProfilerBlock* block, string& output, size_t depth, size_t maxDepth, bool showUnused, bool showTotal) const
	{
		char line[LINE_MAX_LENGTH];
		char indentedName[LINE_MAX_LENGTH];

		uint32_t currentInterval = _intervalFrames;
		if (!currentInterval)
			++currentInterval;

		if (depth >= maxDepth)
			return;

		// Do not print the root block as it does not collect any actual data
		if (block != _root)
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
					float max = block->_intervalMaxTime / 1000.0f;
					float frame = block->intervalTime / currentInterval / 1000.0f;
					float all = block->intervalTime / 1000.0f;

					sprintf(line, "%s %5u %8.3f %8.3f %8.3f %9.3f\n", indentedName, Min(block->intervalCount, 99999),
						avg, max, frame, all);
				}
				else
				{
					float avg = (block->frameCount ? block->_frameTime / block->frameCount : 0.0f) / 1000.0f;
					float max = block->_frameMaxTime / 1000.0f;
					float all = block->_frameTime / 1000.0f;

					float totalAvg = (block->totalCount ? block->_totalTime / block->totalCount : 0.0f) / 1000.0f;
					float totalMax = block->_totalMaxTime / 1000.0f;
					float totalAll = block->_totalTime / 1000.0f;

					sprintf(line, "%s %5u %8.3f %8.3f %9.3f  %7u %9.3f %9.3f %11.3f\n", indentedName, Min(block->frameCount, 99999),
						avg, max, all, Min(block->totalCount, 99999), totalAvg, totalMax, totalAll);
				}

				output += line;
			}

			++depth;
		}

		for (auto it = block->_children.begin(); it != block->_children.end(); ++it)
		{
			OutputResults(*it, output, depth, maxDepth, showUnused, showTotal);
		}
	}

}
