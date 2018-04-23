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
		{ SDL_SCANCODE_A, Key::A },
		{ SDL_SCANCODE_B, Key::B },
		{ SDL_SCANCODE_C, Key::C },
		{ SDL_SCANCODE_D, Key::D },
		{ SDL_SCANCODE_E, Key::E },
		{ SDL_SCANCODE_F, Key::F },
		{ SDL_SCANCODE_G, Key::G },
		{ SDL_SCANCODE_H, Key::H },
		{ SDL_SCANCODE_I, Key::I },
		{ SDL_SCANCODE_J, Key::J },
		{ SDL_SCANCODE_K, Key::K },
		{ SDL_SCANCODE_L, Key::L },
		{ SDL_SCANCODE_M, Key::M },
		{ SDL_SCANCODE_N, Key::N },
		{ SDL_SCANCODE_O, Key::O },
		{ SDL_SCANCODE_P, Key::P },
		{ SDL_SCANCODE_Q, Key::Q },
		{ SDL_SCANCODE_R, Key::R },
		{ SDL_SCANCODE_S, Key::S },
		{ SDL_SCANCODE_T, Key::T },
		{ SDL_SCANCODE_U, Key::U },
		{ SDL_SCANCODE_V, Key::V },
		{ SDL_SCANCODE_W, Key::W },
		{ SDL_SCANCODE_X, Key::X },
		{ SDL_SCANCODE_Y, Key::Y },
		{ SDL_SCANCODE_Z, Key::Z },

		{ SDL_SCANCODE_0, Key::D0 },
		{ SDL_SCANCODE_1, Key::D1 },
		{ SDL_SCANCODE_2, Key::D2 },
		{ SDL_SCANCODE_3, Key::D3 },
		{ SDL_SCANCODE_4, Key::D4 },
		{ SDL_SCANCODE_5, Key::D5 },
		{ SDL_SCANCODE_6, Key::D6 },
		{ SDL_SCANCODE_7, Key::D7 },
		{ SDL_SCANCODE_8, Key::D8 },
		{ SDL_SCANCODE_9, Key::D9 },

		{ SDL_SCANCODE_KP_0, Key::Numpad0 },
		{ SDL_SCANCODE_KP_1, Key::Numpad1 },
		{ SDL_SCANCODE_KP_2, Key::Numpad2 },
		{ SDL_SCANCODE_KP_3, Key::Numpad3 },
		{ SDL_SCANCODE_KP_4, Key::Numpad4 },
		{ SDL_SCANCODE_KP_5, Key::Numpad5 },
		{ SDL_SCANCODE_KP_6, Key::Numpad6 },
		{ SDL_SCANCODE_KP_7, Key::Numpad7 },
		{ SDL_SCANCODE_KP_8, Key::Numpad8 },
		{ SDL_SCANCODE_KP_9, Key::Numpad9 },

		{ SDL_SCANCODE_RETURN, Key::Return },
		{ SDL_SCANCODE_ESCAPE, Key::Escape },
		{ SDL_SCANCODE_BACKSPACE, Key::Backspace },
		{ SDL_SCANCODE_TAB, Key::Tab },
		{ SDL_SCANCODE_SPACE, Key::Space },
		{ SDL_SCANCODE_UP, Key::Up },
		{ SDL_SCANCODE_DOWN, Key::Down },
		{ SDL_SCANCODE_LEFT, Key::Left },
		{ SDL_SCANCODE_RIGHT, Key::Right },
		{ SDL_SCANCODE_INSERT, Key::Insert },
		{ SDL_SCANCODE_DELETE, Key::Delete },
		{ SDL_SCANCODE_HOME, Key::Home },
		{ SDL_SCANCODE_END, Key::End },
		{ SDL_SCANCODE_PAGEUP, Key::PageUp },
		{ SDL_SCANCODE_PAGEDOWN, Key::PageDown },
		{ SDL_SCANCODE_PRINTSCREEN, Key::Print },
		{ SDL_SCANCODE_KP_PLUS, Key::Plus },
		{ SDL_SCANCODE_KP_MINUS, Key::Minus },

		{ SDL_SCANCODE_F1, Key::F1 },
		{ SDL_SCANCODE_F2, Key::F2 },
		{ SDL_SCANCODE_F3, Key::F3 },
		{ SDL_SCANCODE_F4, Key::F4 },
		{ SDL_SCANCODE_F5, Key::F5 },
		{ SDL_SCANCODE_F6, Key::F6 },
		{ SDL_SCANCODE_F7, Key::F7 },
		{ SDL_SCANCODE_F8, Key::F8 },
		{ SDL_SCANCODE_F9, Key::F9 },
		{ SDL_SCANCODE_F10, Key::F10 },
		{ SDL_SCANCODE_F11, Key::F11 },
		{ SDL_SCANCODE_F12, Key::F12 },
		{ SDL_SCANCODE_F13, Key::F13 },
		{ SDL_SCANCODE_F14, Key::F14 },
		{ SDL_SCANCODE_F15, Key::F15 },
		{ SDL_SCANCODE_F16, Key::F16 },
		{ SDL_SCANCODE_F17, Key::F17 },
		{ SDL_SCANCODE_F18, Key::F18 },
		{ SDL_SCANCODE_F19, Key::F19 },
		{ SDL_SCANCODE_F20, Key::F20 },
		{ SDL_SCANCODE_F21, Key::F21 },
		{ SDL_SCANCODE_F22, Key::F22 },
		{ SDL_SCANCODE_F23, Key::F23 },
		{ SDL_SCANCODE_F24, Key::F23 },
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
		if (!InitializeBeforeRun())
		{
			ErrorExit();
			return _exitCode;
		}

		OnStart();
		if (_exitCode)
			return _exitCode;

		Input* input = Input::GetInput();

		// Start the event loop.
		SDL_Event evt;
		while (!_exiting)
		{
			input->BeginFrame();

			while (SDL_PollEvent(&evt))
			{
				switch (evt.type) {
				case SDL_QUIT:
					_exiting = true;
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

				case SDL_MOUSEMOTION: {
					const SDL_MouseMotionEvent& motionEvt = evt.motion;
					input->OnMouseMove(motionEvt.x, motionEvt.y, motionEvt.xrel, motionEvt.yrel);
					break;
				}

				case SDL_MOUSEBUTTONDOWN:
				case SDL_MOUSEBUTTONUP: {
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

			RunFrame();
		}

		Exit();
		return _exitCode;
	}

	void Application::PlatformExit()
	{
		if (_exiting)
			return;

		OnStop();

		// Close main window.
		_window->Close();

		// quit all subsystems and quit application.
		SDL_QuitSubSystem(SDL_INIT_EVERYTHING);
		SDL_Quit();

		_exiting = true;
	}
}
