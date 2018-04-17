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

#include "../Base/Utils.h"
#include "../Object/Object.h"
#include "../Math/Size.h"
#include "../Math/IntVector2.h"

#if ALIMER_PLATFORM_WINDOWS
struct HWND__;
struct HINSTANCE__;
#endif

namespace Alimer
{
	enum class WindowFlags : uint32_t
	{
		None = 0,
		Fullscreen = 1 << 0,
		Borderless = 1 << 1,
		Resizable = 1 << 2,
		Closeable = 1 << 3,
		Default = Resizable | Closeable
	};
	ALIMER_BITMASK(WindowFlags);

	/// Window resized event.
	class ALIMER_API WindowResizeEvent : public Event
	{
	public:
		/// New window size.
		Size size;
	};


#if ALIMER_PLATFORM_WINDOWS
	using WindowHandle = HWND__*;
	using WindowInstance = HINSTANCE__*;
#endif

	/// Operating system window, Win32 implementation.
	class ALIMER_API Window : public Object
	{
		ALIMER_OBJECT(Window, Object);

	public:
		/// Construct and register subsystem. The window is not yet opened.
		Window();
		/// Destruct. Close window if open.
		~Window();

		/// Set window title.
		void SetTitle(const std::string& newTitle);
		/// Set window size. Open the window if not opened yet. Return true on success.
		bool SetSize(const Size& size, bool fullscreen, bool resizable);
		/// Set window position.
		void SetPosition(const IntVector2& position);
		/// Set mouse cursor visible. Default is true. When hidden, the mouse cursor is confined to the window and kept centered; relative mouse motion can be read "endlessly" but absolute mouse position should not be used.
		void SetMouseVisible(bool enable);
		/// Move the mouse cursor to a window top-left relative position.
		void SetMousePosition(const IntVector2& position);
		/// Close the window.
		void Close();
		/// Minimize the window.
		void Minimize();
		/// Maximize the window.
		void Maximize();
		/// Restore window size.
		void Restore();
		/// Pump window messages from the operating system.
		void PumpMessages();

		/// Return window title.
		const std::string& GetTitle() const { return _title; }
		/// Return window client area size.
		const Size& GetSize() const { return _size; }
		/// Return window client area width.
		uint32_t GetWidth() const { return _size.width; }
		/// Return window client area height.
		uint32_t GetHeight() const { return _size.height; }
		/// Return window position.
		IntVector2 Position() const;
		/// Return last known mouse cursor position relative to window top-left.
		const IntVector2& MousePosition() const { return mousePosition; }
		/// Return whether window is open.
		bool IsOpen() const { return _handle != nullptr; }
		/// Return whether is resizable.
		bool IsResizable() const { return resizable; }
		/// Return whether is fullscren.
		bool IsFullscreen() const { return fullscreen; }
		/// Return whether is currently minimized.
		bool IsMinimized() const { return minimized; }
		/// Return whether has input focus.
		bool HasFocus() const { return focus; }
		/// Return whether mouse cursor is visible.
		bool IsMouseVisible() const { return mouseVisible; }
		/// Return window handle. Can be cast to a HWND.
		WindowHandle GetHandle() const { return _handle; }

		/// Handle a window message. Return true if handled and should not be passed to the default window procedure.
		bool OnWindowMessage(unsigned msg, unsigned wParam, unsigned lParam);

		/// Close requested event.
		Event closeRequestEvent;
		/// Gained focus event.
		Event gainFocusEvent;
		/// Lost focus event.
		Event loseFocusEvent;
		/// Minimized event.
		Event minimizeEvent;
		/// Restored after minimization -event.
		Event restoreEvent;
		/// Size changed event.
		WindowResizeEvent resizeEvent;

	private:
		void PlatformInitialize();

		/// Change display mode. If width and height are zero, will restore desktop resolution.
		void SetDisplayMode(uint32_t width, uint32_t height);
		/// Update mouse visibility and clipping region to the OS.
		void UpdateMouseVisible();
		/// Update mouse clipping region.
		void UpdateMouseClipping();
		/// Refresh the internally tracked mouse cursor position.
		void UpdateMousePosition();
		/// Verify window size from the window client rect.
		Size GetClientRectSize() const;

		/// Window handle.
		WindowHandle _handle{};

		/// Window instance.
		WindowInstance _instance{};

		/// Window title.
		std::string _title;
		/// Current client area size.
		Size _size;
		/// Last stored windowed mode position.
		IntVector2 savedPosition;
		/// Current mouse cursor position.
		IntVector2 mousePosition;
		/// Window style flags.
		unsigned windowStyle;
		/// Current minimization state.
		bool minimized;
		/// Current focus state.
		bool focus;
		/// Resizable flag.
		bool resizable;
		/// Fullscreen flag.
		bool fullscreen;
		/// Performing window resize flag. Used internally to suppress resize events during it.
		bool inResize;
		/// Mouse visible flag as requested by the application.
		bool mouseVisible;
		/// Internal mouse visible flag. The mouse is automatically shown when the window is unfocused, while mouseVisible represents the application's desired state. Used to prevent multiple calls to OS mouse visibility functions, which utilize a counter.
		bool mouseVisibleInternal;
	};
}