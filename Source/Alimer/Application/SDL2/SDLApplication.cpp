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

#include "Application/Application.h"
#include "Debug/Log.h"
#include "Window/Input.h"

#define SDL_MAIN_HANDLED
#include <SDL.h>

namespace Alimer
{
#if ALIMER_PLATFORM_WINDOWS
	extern bool Win32PlatformRun();
#endif

	int Application::PlatformRun()
	{
#if ALIMER_PLATFORM_WINDOWS
		Win32PlatformRun();
#endif

		int result = SDL_Init(
			SDL_INIT_VIDEO
			| SDL_INIT_GAMECONTROLLER
			| SDL_INIT_HAPTIC
			| SDL_INIT_TIMER);
		if (result < 0)
		{
			SDL_ShowSimpleMessageBox(
				SDL_MESSAGEBOX_ERROR,
				"SDL Init Errors",
				SDL_GetError(), nullptr);
			return EXIT_FAILURE;
		}

		SDL_EventState(SDL_DROPFILE, SDL_ENABLE);

		// Now init engine.
		if (!_engine->Initialize(_settings))
		{
			ErrorExit();
			return _exitCode;
		}

		Start();
		if (_exitCode)
			return _exitCode;

		Input* input = GetSubsystem<Input>();

		// Start the event loop.
		SDL_Event evt;
		while (!_engine->IsExiting())
		{
			while (SDL_PollEvent(&evt))
			{
				switch (evt.type)
				{
				case SDL_QUIT:
					Exit();
					break;
				}
			}

			input->Update();
			_engine->RunFrame();
		}

		//Exit();
		return _exitCode;
	}

	void Application::PlatformExit()
	{
		_engine->Exit();
		SDL_QuitSubSystem(SDL_INIT_EVERYTHING);
		SDL_Quit();
	}
}
