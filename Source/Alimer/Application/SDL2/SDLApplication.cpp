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

	static const std::unordered_map<SDL_Keycode, Key> _keyMap =
	{
		{ SDL_SCANCODE_ESCAPE, Key::Escape },
		{ SDL_SCANCODE_RETURN, Key::Return },
		{ SDL_SCANCODE_TAB, Key::Tab },
		{ SDL_SCANCODE_SPACE, Key::Space },
		{ SDL_SCANCODE_BACKSPACE, Key::Backspace },
	};

	static Key TranslateKey(SDL_Scancode code)
	{
		auto i = _keyMap.find(code);

		if (i != _keyMap.end())
		{
			return i->second;
		}

		return Key::None;
	}

	static MouseButton TranslateMouseButton(uint32_t sdlButton)
	{
		switch (sdlButton)
		{
		case SDL_BUTTON_LEFT: return MouseButton::Left;
		case SDL_BUTTON_MIDDLE: return MouseButton::Middle;
		case SDL_BUTTON_RIGHT: return MouseButton::Right;
		case SDL_BUTTON_X1: return MouseButton::X1;
		case SDL_BUTTON_X2: return MouseButton::X2;
		default: return MouseButton::None;
		}
	}

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

		Input* input = Input::GetInput();

		// Start the event loop.
		SDL_Event evt;
		while (!_engine->IsExiting())
		{
			input->BeginFrame();

			while (SDL_PollEvent(&evt))
			{
				switch (evt.type) {
				case SDL_QUIT:
					Exit();
					break;

				case SDL_KEYDOWN: {
					const SDL_KeyboardEvent& keyEvent = evt.key;
					Key key = TranslateKey(keyEvent.keysym.scancode);

					input->PostKeyEvent(
						key,
						true);
					break;
				}

				case SDL_KEYUP: {
					const SDL_KeyboardEvent& keyEvent = evt.key;
					Key key = TranslateKey(keyEvent.keysym.scancode);

					input->PostKeyEvent(
						key,
						false);
					break;
				}

				case SDL_MOUSEBUTTONDOWN:
				case SDL_MOUSEBUTTONUP:
				{
					const SDL_MouseButtonEvent& mouseEvent = evt.button;
					MouseButton button = TranslateMouseButton(mouseEvent.button);
					input->OnMouse(
						mouseEvent.x,
						mouseEvent.y,
						button,
						mouseEvent.type == SDL_MOUSEBUTTONDOWN);
				}
				break;
				}
			}

			
			_engine->RunFrame();
		}

		//Exit();
		return _exitCode;
	}

	void Application::PlatformExit()
	{
		Stop();
		_engine->Exit();
		SDL_QuitSubSystem(SDL_INIT_EVERYTHING);
		SDL_Quit();
	}
}
