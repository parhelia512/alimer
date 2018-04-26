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

#include "../../Debug/Log.h"
#include "../../Math/Math.h"
#include "../Input.h"
#include "../Window.h"

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_syswm.h>

namespace Alimer
{
	void Window::PlatformInitialize()
	{
		uint32_t sdlFlags = SDL_WINDOW_SHOWN;
		if (_resizable)
			sdlFlags |= SDL_WINDOW_RESIZABLE;

		if (_fullscreen)
			sdlFlags |= SDL_WINDOW_FULLSCREEN;

		_handle = SDL_CreateWindow(_title.c_str(),
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			_width,
			_height,
			sdlFlags);

		if (_handle)
		{
			SDL_SetWindowData(_handle, "owner", this);
			//RegisterWindow(_handle);
		}
		else
		{
			const char* sdlError = SDL_GetError();
			ALIMER_LOGERROR("Window: Failed to create window. SDL error: \"{}\"", sdlError);
			return;
		}

		SDL_SysWMinfo wmInfo = {};
		if (SDL_GetWindowWMInfo(_handle, &wmInfo))
		{
#if ALIMER_PLATFORM_WINDOWS
			_platformData.hwnd = wmInfo.info.win.window;
			_platformData.hdc = wmInfo.info.win.hdc;
			_platformData.hInstance = wmInfo.info.win.hinstance;
#elif ALIMER_PLATFORM_LINUX
			_platformData.display = wmInfo.info.x11.display;
			_platformData.window = wmInfo.info.x11.window;
#endif
		}
	}

	void Window::SetTitle(const std::string& newTitle)
	{
		_title = newTitle;
		SDL_SetWindowTitle(_handle, _title.c_str());
	}

	void Window::SetPosition(const IntVector2& position)
	{
		SDL_SetWindowPosition(_handle, position.x, position.y);
	}

	void Window::Close()
	{
		if (_handle)
		{
			//DeregisterWindow(_handle);
			SDL_DestroyWindow(_handle);
			_handle = nullptr;
		}
	}

	void Window::Minimize()
	{
		SDL_MinimizeWindow(_handle);
	}

	void Window::Maximize()
	{
		SDL_MaximizeWindow(_handle);
	}

	void Window::Restore()
	{
		SDL_RestoreWindow(_handle);
	}

	IntVector2 Window::GetPosition() const
	{
		int w, h;
		SDL_GetWindowPosition(_handle, &w, &h);
		return IntVector2(w, h);
	}

	Size Window::GetClientRectSize() const
	{
		int w, h;
		SDL_GetWindowSize(_handle, &w, &h);
		return Size(w, h);
	}

	bool Window::HasFocus() const
	{
		return SDL_GetKeyboardFocus() == _handle
			|| SDL_GetMouseFocus() == _handle;
	}

	bool Window::IsMinimized() const
	{
		return SDL_GetWindowFlags(_handle) & SDL_WINDOW_MINIMIZED;
	}

	bool Cursor::IsVisible()
	{
		return SDL_ShowCursor(SDL_QUERY) == SDL_ENABLE;
	}

	void Cursor::SetVisible(bool visible)
	{
		if (visible)
			SDL_ShowCursor(SDL_ENABLE);
		else
			SDL_ShowCursor(SDL_DISABLE);
	}
}
