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
#include "../../PlatformIncl.h"
#include <mutex>

#ifndef DPI_ENUMS_DECLARED
typedef enum
{
	PROCESS_DPI_UNAWARE = 0,
	PROCESS_SYSTEM_DPI_AWARE = 1,
	PROCESS_PER_MONITOR_DPI_AWARE = 2
} PROCESS_DPI_AWARENESS;
typedef enum
{
	MDT_EFFECTIVE_DPI = 0,
	MDT_ANGULAR_DPI = 1,
	MDT_RAW_DPI = 2,
	MDT_DEFAULT = MDT_EFFECTIVE_DPI
} MONITOR_DPI_TYPE;
#endif /*DPI_ENUMS_DECLARED*/

namespace Alimer
{
	typedef BOOL(WINAPI * PFN_SetProcessDPIAware)(void);
	static HMODULE s_userDLL = nullptr;
	static HMODULE s_shCoreDLL = nullptr;

	static PFN_SetProcessDPIAware SetProcessDPIAware_;
	static BOOL(WINAPI* registerTouchWindow)(HWND, ULONG) = nullptr;
	static BOOL(WINAPI* getTouchInputInfo)(HTOUCHINPUT, UINT, PTOUCHINPUT, int) = nullptr;
	static BOOL(WINAPI* closeTouchInputHandle)(HTOUCHINPUT) = nullptr;

	// ShCore
	typedef HRESULT(WINAPI * PFN_SetProcessDpiAwareness)(PROCESS_DPI_AWARENESS);
	typedef HRESULT(WINAPI * PFN_GetDpiForMonitor)(HMONITOR, MONITOR_DPI_TYPE, UINT*, UINT*);

	static PFN_SetProcessDpiAwareness  SetProcessDpiAwareness_;
	static PFN_GetDpiForMonitor GetDpiForMonitor_;

	std::once_flag functionsInitialized;
	uint32_t _windowCount = 0; // Windows owned by the Engine

	static LRESULT CALLBACK WndProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);

	const WCHAR AppWndClass[] = L"AlimerWindow";

	static inline void RegisterWndClass(HINSTANCE hInstance, HBRUSH backgroundBrush)
	{
		// Register main class.
		WNDCLASSEXW wc = {};
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wc.lpfnWndProc = WndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hInstance;
		wc.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
		wc.hIconSm = nullptr;
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wc.hbrBackground = backgroundBrush;
		wc.lpszMenuName = nullptr;
		wc.lpszClassName = AppWndClass;

		if (!RegisterClassExW(&wc))
		{
			LOGERROR("[Win32] - Could not register window class");
			return;
		}
	}

	void Window::PlatformInitialize()
	{
		_instance = GetModuleHandleW(nullptr);

		std::call_once(functionsInitialized, []()
		{
			s_userDLL = GetModuleHandleW(L"user32.dll");
			SetProcessDPIAware_ = reinterpret_cast<PFN_SetProcessDPIAware>(::GetProcAddress(s_userDLL, "SetProcessDPIAware"));
			registerTouchWindow = (BOOL(WINAPI*)(HWND, ULONG))(void*)GetProcAddress(s_userDLL, "RegisterTouchWindow");
			getTouchInputInfo = (BOOL(WINAPI*)(HTOUCHINPUT, UINT, PTOUCHINPUT, int))(void*)GetProcAddress(s_userDLL, "GetTouchInputInfo");
			closeTouchInputHandle = (BOOL(WINAPI*)(HTOUCHINPUT))(void*)GetProcAddress(s_userDLL, "CloseTouchInputHandle");

			s_shCoreDLL = ::LoadLibraryW(L"Shcore.dll");
			if (s_shCoreDLL)
			{
				SetProcessDpiAwareness_ = reinterpret_cast<PFN_SetProcessDpiAwareness>(::GetProcAddress(s_shCoreDLL, "SetProcessDpiAwareness"));
				GetDpiForMonitor_ = reinterpret_cast<PFN_GetDpiForMonitor>(::GetProcAddress(s_shCoreDLL, "GetDpiForMonitor"));

				if (SetProcessDpiAwareness_)
				{
					// We only check for E_INVALIDARG because we would get
					// E_ACCESSDENIED if the DPI was already set previously
					// and S_OK means the call was successful
					if (SetProcessDpiAwareness_(PROCESS_PER_MONITOR_DPI_AWARE) == E_INVALIDARG)
					{
						LOGERROR("Failed to set process DPI awareness");
					}
				}
			}
			else
			{
				// Fall back to SetProcessDPIAware if SetProcessDpiAwareness
				// is not available on this system
				if (SetProcessDPIAware_ != nullptr)
				{
					if (!SetProcessDPIAware_())
					{
						LOGERROR("Failed to set process DPI awareness");
					}
				}
			}
		});

		if (_windowCount == 0)
		{
			const HBRUSH backgroundBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
			RegisterWndClass(_instance, backgroundBrush);
		}
	}

	void Window::SetTitle(const std::string& newTitle)
	{
		_title = newTitle;
		if (_handle)
		{
			wchar_t titleBuffer[256] = L"";

			if (!newTitle.empty() && MultiByteToWideChar(CP_UTF8, 0, newTitle.c_str(), -1, titleBuffer, 256) == 0)
			{
				LOGERROR("Failed to convert UTF-8 to wide char");
				return;
			}

			SetWindowTextW(_handle, titleBuffer);
		}
	}

	bool Window::SetSize(const Size& size_, bool fullscreen_, bool resizable_)
	{
		inResize = true;
		IntVector2 position = savedPosition;

		if (!fullscreen_)
		{
			windowStyle = WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX;
			if (resizable_)
				windowStyle |= WS_THICKFRAME | WS_MAXIMIZEBOX;

			// Return to desktop resolution if was fullscreen
			if (fullscreen)
				SetDisplayMode(0, 0);
		}
		else
		{
			// When switching to fullscreen, save last windowed mode position
			if (!fullscreen)
				savedPosition = Position();

			windowStyle = WS_POPUP | WS_VISIBLE;
			position = IntVector2::ZERO;
			/// \todo Handle failure to set mode
			SetDisplayMode(size_.width, size_.height);
		}

		DWORD dwExStyle = WS_EX_APPWINDOW;

		RECT rect = { 0, 0, static_cast<LONG>(size_.width), static_cast<LONG>(size_.height) };
		AdjustWindowRectEx(&rect, windowStyle, FALSE, dwExStyle);

		if (!_handle)
		{
			// Create a window.
			wchar_t titleBuffer[256] = L"";

			if (!_title.empty() &&
				MultiByteToWideChar(CP_UTF8, 0, _title.c_str(), -1, titleBuffer, 256) == 0)
			{
				LOGERROR("Failed to convert UTF-8 to wide char");
				return false;
			}

			_handle = CreateWindowExW(
				dwExStyle,
				AppWndClass,
				titleBuffer,
				windowStyle,
				position.x,
				position.y,
				rect.right - rect.left,
				rect.bottom - rect.top,
				0, 0,
				_instance,
				nullptr);

			if (!_handle)
			{
				LOGERROR("Failed to create window");
				inResize = false;
				return false;
			}

			_windowCount++;

			// Enable touch input if available
			if (registerTouchWindow)
				registerTouchWindow(_handle, TWF_FINETOUCH | TWF_WANTPALM);

			minimized = false;
			focus = false;

			SetWindowLongPtrW(_handle, GWLP_USERDATA, (LONG_PTR)this);
			ShowWindow(_handle, SW_SHOW);
		}
		else
		{
			SetWindowLongPtrW(_handle, GWL_STYLE, windowStyle);

			if (!fullscreen_ && (savedPosition.x == M_MIN_INT || savedPosition.y == M_MIN_INT))
			{
				WINDOWPLACEMENT placement;
				placement.length = sizeof(placement);
				GetWindowPlacement(_handle, &placement);
				position = IntVector2(placement.rcNormalPosition.left, placement.rcNormalPosition.top);
			}

			SetWindowPos(_handle, nullptr, position.x, position.y, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
			ShowWindow(_handle, SW_SHOW);
		}

		fullscreen = fullscreen_;
		resizable = resizable_;
		inResize = false;

		Size newSize = GetClientRectSize();
		if (newSize != _size)
		{
			_size = newSize;
			resizeEvent.size = newSize;
			SendEvent(resizeEvent);
		}

		UpdateMouseVisible();
		UpdateMousePosition();

		return true;
	}

	void Window::SetPosition(const IntVector2& position)
	{
		if (_handle)
			SetWindowPos((HWND)_handle, nullptr, position.x, position.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}

	void Window::SetMouseVisible(bool enable)
	{
		if (enable != mouseVisible)
		{
			mouseVisible = enable;
			UpdateMouseVisible();
		}
	}

	void Window::SetMousePosition(const IntVector2& position)
	{
		if (_handle)
		{
			mousePosition = position;
			POINT screenPosition;
			screenPosition.x = position.x;
			screenPosition.y = position.y;
			ClientToScreen((HWND)_handle, &screenPosition);
			SetCursorPos(screenPosition.x, screenPosition.y);
		}
	}

	void Window::Close()
	{
		if (_handle)
		{
			// Return to desktop resolution if was fullscreen, else save last windowed mode position
			if (fullscreen)
				SetDisplayMode(0, 0);
			else
				savedPosition = Position();

			DestroyWindow((HWND)_handle);
			_handle = nullptr;
		}
	}

	void Window::Minimize()
	{
		if (_handle)
			ShowWindow((HWND)_handle, SW_MINIMIZE);
	}

	void Window::Maximize()
	{
		if (_handle)
			ShowWindow((HWND)_handle, SW_MAXIMIZE);
	}

	void Window::Restore()
	{
		if (_handle)
			ShowWindow((HWND)_handle, SW_RESTORE);
	}

	void Window::PumpMessages()
	{
		if (!_handle)
			return;

		MSG msg;

		while (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	IntVector2 Window::Position() const
	{
		if (!_handle)
			return IntVector2::ZERO;

		RECT rect;
		GetWindowRect((HWND)_handle, &rect);
		return IntVector2(rect.left, rect.top);
	}

	bool Window::OnWindowMessage(unsigned msg, unsigned wParam, unsigned lParam)
	{
		Input* input = Subsystem<Input>();
		bool handled = false;

		// Skip emulated mouse events that are caused by touch
		bool emulatedMouse = (GetMessageExtraInfo() & 0xffffff00) == 0xff515700;

		switch (msg)
		{
		case WM_DESTROY:
			_handle = nullptr;
			break;

		case WM_CLOSE:
			SendEvent(closeRequestEvent);
			handled = true;
			break;

		case WM_ACTIVATE:
		{
			bool newFocus = LOWORD(wParam) != WA_INACTIVE;
			if (newFocus != focus)
			{
				focus = newFocus;
				if (focus)
				{
					SendEvent(gainFocusEvent);
					if (input)
						input->OnGainFocus();
					if (minimized)
						Restore();

					// If fullscreen, automatically restore mouse focus
					if (fullscreen)
						UpdateMouseVisible();
				}
				else
				{
					SendEvent(loseFocusEvent);
					if (input)
						input->OnLoseFocus();

					// If fullscreen, minimize on focus loss
					if (fullscreen)
						ShowWindow((HWND)_handle, SW_MINIMIZE);

					// Stop mouse cursor hiding & clipping
					UpdateMouseVisible();
				}
			}
		}
		break;

		case WM_SIZE:
		{
			bool newMinimized = (wParam == SIZE_MINIMIZED);
			if (newMinimized != minimized)
			{
				minimized = newMinimized;
				if (minimized)
				{
					// If is fullscreen, restore desktop resolution
					if (fullscreen)
						SetDisplayMode(0, 0);

					SendEvent(minimizeEvent);
				}
				else
				{
					// If should be fullscreen, restore mode now
					if (fullscreen)
					{
						SetDisplayMode(_size.width, _size.height);
					}

					SendEvent(restoreEvent);
				}
			}

			if (!minimized && !inResize)
			{
				Size newSize = GetClientRectSize();
				if (newSize != _size)
				{
					_size = newSize;
					resizeEvent.size = newSize;
					SendEvent(resizeEvent);
				}
			}

			// If mouse is currently hidden, update the clip region
			if (!mouseVisibleInternal)
				UpdateMouseClipping();
		}
		break;

		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			if (input)
				input->OnKey(wParam, (lParam >> 16) & 0xff, true);
			handled = (msg == WM_KEYDOWN);
			break;

		case WM_KEYUP:
		case WM_SYSKEYUP:
			if (input)
				input->OnKey(wParam, (lParam >> 16) & 0xff, false);
			handled = (msg == WM_KEYUP);
			break;

		case WM_CHAR:
			if (input)
				input->OnChar(wParam);
			handled = true;
			break;

		case WM_MOUSEMOVE:
			if (input && !emulatedMouse)
			{
				IntVector2 newPosition;
				newPosition.x = (int)(short)LOWORD(lParam);
				newPosition.y = (int)(short)HIWORD(lParam);

				// Do not transmit mouse move when mouse should be hidden, but is not due to no input focus
				if (mouseVisibleInternal == mouseVisible)
				{
					IntVector2 delta = newPosition - mousePosition;
					input->OnMouseMove(newPosition, delta);
					// Recenter in hidden mouse cursor mode to allow endless relative motion
					if (!mouseVisibleInternal && delta != IntVector2::ZERO)
					{
						Size halfSize = GetSize() / 2;
						SetMousePosition(IntVector2(halfSize.width, halfSize.height));
					}
					else
					{
						mousePosition = newPosition;
					}
				}
				else
					mousePosition = newPosition;
			}
			handled = true;
			break;

		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
			if (input && !emulatedMouse)
			{
				unsigned button = (msg == WM_LBUTTONDOWN) ? MOUSEB_LEFT : (msg == WM_MBUTTONDOWN) ? MOUSEB_MIDDLE : MOUSEB_RIGHT;
				input->OnMouseButton(button, true);
				// Make sure we track the button release even if mouse moves outside the window
				SetCapture((HWND)_handle);
				// Re-establish mouse cursor hiding & clipping
				if (!mouseVisible && mouseVisibleInternal)
					UpdateMouseVisible();
			}
			handled = true;
			break;

		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
			if (input && !emulatedMouse)
			{
				unsigned button = (msg == WM_LBUTTONUP) ? MOUSEB_LEFT : (msg == WM_MBUTTONUP) ? MOUSEB_MIDDLE : MOUSEB_RIGHT;
				input->OnMouseButton(button, false);
				// End capture when there are no more mouse buttons held down
				if (!input->MouseButtons())
					ReleaseCapture();
			}
			handled = true;
			break;

		case WM_TOUCH:
			if (input && LOWORD(wParam))
			{
				std::vector<TOUCHINPUT> touches(LOWORD(wParam));
				if (getTouchInputInfo((HTOUCHINPUT)lParam, (unsigned)touches.size(), &touches[0], sizeof(TOUCHINPUT)))
				{
					for (auto it = touches.begin(); it != touches.end(); ++it)
					{
						// Translate touch points inside window
						POINT point;
						point.x = it->x / 100;
						point.y = it->y / 100;
						ScreenToClient((HWND)_handle, &point);
						IntVector2 position(point.x, point.y);

						if (it->dwFlags & (TOUCHEVENTF_DOWN || TOUCHEVENTF_UP))
							input->OnTouch(it->dwID, true, position, 1.0f);
						else if (it->dwFlags & TOUCHEVENTF_UP)
							input->OnTouch(it->dwID, false, position, 1.0f);

						// Mouse cursor will move to the position of the touch on next move, so move beforehand
						// to prevent erratic jumps in hidden mouse mode
						if (!mouseVisibleInternal)
							mousePosition = position;
					}
				}
			}

			closeTouchInputHandle((HTOUCHINPUT)lParam);
			handled = true;
			break;

		case WM_SYSCOMMAND:
			// Prevent system bell sound from Alt key combinations
			if ((wParam & 0xff00) == SC_KEYMENU)
				handled = true;
			break;
		}

		return handled;
	}

	Size Window::GetClientRectSize() const
	{
		if (!_handle)
			return Size::Empty;

		RECT rect;
		GetClientRect((HWND)_handle, &rect);
		return Size(rect.right, rect.bottom);
	}

	void Window::SetDisplayMode(uint32_t width, uint32_t height)
	{
		if (width && height)
		{
			DEVMODEW screenMode;
			screenMode.dmSize = sizeof(DEVMODEW);
			screenMode.dmPelsWidth = width;
			screenMode.dmPelsHeight = height;
			screenMode.dmBitsPerPel = 32;
			screenMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
			ChangeDisplaySettingsW(&screenMode, CDS_FULLSCREEN);
		}
		else
			ChangeDisplaySettingsW(nullptr, CDS_FULLSCREEN);
	}

	void Window::UpdateMouseVisible()
	{
		if (!_handle)
			return;

		// When the window is unfocused, mouse should never be hidden
		bool newMouseVisible = HasFocus() ? mouseVisible : true;
		if (newMouseVisible != mouseVisibleInternal)
		{
			ShowCursor(newMouseVisible ? TRUE : FALSE);
			mouseVisibleInternal = newMouseVisible;
		}

		UpdateMouseClipping();
	}

	void Window::UpdateMouseClipping()
	{
		if (mouseVisibleInternal)
			ClipCursor(nullptr);
		else
		{
			RECT mouseRect;
			POINT point;
			Size windowSize = GetSize();

			point.x = point.y = 0;
			ClientToScreen((HWND)_handle, &point);
			mouseRect.left = point.x;
			mouseRect.top = point.y;
			mouseRect.right = point.x + windowSize.width;
			mouseRect.bottom = point.y + windowSize.height;
			ClipCursor(&mouseRect);
		}
	}

	void Window::UpdateMousePosition()
	{
		POINT screenPosition;
		GetCursorPos(&screenPosition);
		ScreenToClient((HWND)_handle, &screenPosition);
		mousePosition.x = screenPosition.x;
		mousePosition.y = screenPosition.y;
	}

	LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		Window* window = reinterpret_cast<Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
		bool handled = false;
		// When the window is just opening and has not assigned the userdata yet, let the default procedure handle the messages
		if (window)
			handled = window->OnWindowMessage(msg, (unsigned)wParam, (unsigned)lParam);
		return handled ? 0 : DefWindowProcW(hwnd, msg, wParam, lParam);
	}
}