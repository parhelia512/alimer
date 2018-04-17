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

#include "Thread.h"

#ifdef _WIN32
#	include <Windows.h>
#else
#	include <unistd.h>
#endif

namespace Alimer
{
#ifdef _WIN32
	DWORD WINAPI ThreadFunctionStatic(void* data)
	{
		Thread* thread = static_cast<Thread*>(data);
		thread->ThreadFunction();
		return 0;
	}
#else
	void* ThreadFunctionStatic(void* data)
	{
		Thread* thread = static_cast<Thread*>(data);
		thread->ThreadFunction();
		pthread_exit(nullptr);
		return nullptr;
	}
#endif

	ThreadID Thread::mainThreadID = Thread::CurrentThreadID();

	Thread::Thread() :
		handle(nullptr),
		shouldRun(false)
	{
	}

	Thread::~Thread()
	{
		Stop();
	}

	bool Thread::Run()
	{
		// Check if already running
		if (handle)
			return false;

		shouldRun = true;
#ifdef _WIN32
		handle = CreateThread(nullptr, 0, ThreadFunctionStatic, this, 0, nullptr);
#else
		handle = new pthread_t;
		pthread_attr_t type;
		pthread_attr_init(&type);
		pthread_attr_setdetachstate(&type, PTHREAD_CREATE_JOINABLE);
		pthread_create((pthread_t*)handle, &type, ThreadFunctionStatic, this);
#endif
		return handle != nullptr;
	}

	void Thread::Stop()
	{
		// Check if already stopped
		if (!handle)
			return;

		shouldRun = false;
#ifdef _WIN32
		WaitForSingleObject((HANDLE)handle, INFINITE);
		CloseHandle((HANDLE)handle);
#else
		pthread_t* t = (pthread_t*)handle;
		if (t)
			pthread_join(*t, 0);
		delete t;
#endif
		handle = nullptr;
	}

	void Thread::SetPriority(int priority)
	{
#ifdef _WIN32
		if (handle)
			SetThreadPriority((HANDLE)handle, priority);
#endif
#if defined(__linux__) && !defined(ANDROID)
		pthread_t* t = (pthread_t*)handle;
		if (t)
			pthread_setschedprio(*t, priority);
#endif
	}

	void Thread::Sleep(unsigned mSec)
	{
#ifdef _WIN32
		::Sleep(mSec);
#else
		usleep(mSec * 1000);
#endif
	}

	void Thread::SetMainThread()
	{
		mainThreadID = CurrentThreadID();
	}

	ThreadID Thread::CurrentThreadID()
	{
#ifdef _WIN32
		return GetCurrentThreadId();
#else
		return pthread_self();
#endif
	}

	bool Thread::IsMainThread()
	{
		return CurrentThreadID() == mainThreadID;
	}

	}
