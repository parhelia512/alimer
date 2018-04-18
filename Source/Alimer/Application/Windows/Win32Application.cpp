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

#ifdef ALIMER_STATIC
#include <windows.h>
#include "../Application.h"
#include "../../Debug/Log.h"

namespace Alimer
{
	bool Win32PlatformRun()
	{
		HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
		if (hr == RPC_E_CHANGED_MODE) {
			hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
		}

		if (hr == S_FALSE || FAILED(hr))
		{
			LOGERRORF("Failed to initialize COM, error: %d", hr);
			return false;
		}

#if ALIMER_DEBUG
		//!AllocConsole();
#endif

		// Enable high DPI as SDL not support.
		if (HMODULE shCoreLibrary = ::LoadLibraryW(L"Shcore.dll"))
		{
			typedef HRESULT(WINAPI*SetProcessDpiAwarenessType)(size_t value);
			if (auto fn = GetProcAddress(shCoreLibrary, "SetProcessDpiAwareness"))
			{
				((SetProcessDpiAwarenessType)fn)(2);    // PROCESS_PER_MONITOR_DPI_AWARE
			}

			FreeLibrary(shCoreLibrary);
		}
		else
		{
			if (HMODULE user32Library = ::LoadLibraryW(L"user32.dll"))
			{
				typedef BOOL(WINAPI * PFN_SetProcessDPIAware)(void);

				if (auto fn = GetProcAddress(user32Library, "SetProcessDPIAware"))
				{
					((PFN_SetProcessDPIAware)fn)();
				}

				FreeLibrary(user32Library);
			}
		}

		return true;
	}
}

#endif