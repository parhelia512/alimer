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

#include "Application.h"

namespace Alimer
{
	static Application* __appInstance = nullptr;

	Application::Application()
		: _exitCode(EXIT_SUCCESS)
		, _engine(new Engine())
	{
		__appInstance = this;
		RegisterSubsystem(this);
	}

	Application::~Application()
	{
		RemoveSubsystem(this);
		__appInstance = nullptr;
	}

	Application* Application::GetInstance()
	{
		return __appInstance;
	}

	int Application::Run()
	{
#if !defined(__GNUC__) || __EXCEPTIONS
		try
		{
#endif
			Setup();
			if (_exitCode)
				return _exitCode;

			return PlatformRun();
#if !defined(__GNUC__) || __EXCEPTIONS
		}
		catch (std::bad_alloc&)
		{
			//ErrorDialog(GetTypeName(), "An out-of-memory error occurred. The application will now exit.");
			return EXIT_FAILURE;
		}
#endif
	}

	void Application::Exit()
	{
		PlatformExit();
	}

	void Application::ErrorExit(const std::string& /*message*/)
	{
		// Exit the engine.
		_engine->Exit(); 
		_exitCode = EXIT_FAILURE;

		// TODO: Error message.
	}
}