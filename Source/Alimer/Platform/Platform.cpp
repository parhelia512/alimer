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

#include "../Debug/Log.h"
#include "../Platform/Platform.h"

namespace Alimer
{
	PlatformID GetPlatformID()
	{
#if ALIMER_PLATFORM_ANDROID
		return PlatformID::Android;
#elif ALIMER_PLATFORM_APPLE_IOS
		return PlatformID::iOS;
#elif ALIMER_PLATFORM_APPLE_TV
		return PlatformID::AppleTV;
#elif ALIMER_PLATFORM_APPLE_OSX
		return PlatformID::macOS;
#elif ALIMER_PLATFORM_WINDOWS
		return PlatformID::Windows;
#elif ALIMER_PLATFORM_UWP
		return PlatformID::WindowsUniversal;
#elif ALIMER_PLATFORM_WEB
		return PlatformID::Web;
#elif ALIMER_PLATFORM_LINUX
		return PlatformID::Linux;
#else
		return PLATFORM_UNKNOWN;
#endif
	}

	PlatformFamily GetPlatformFamily()
	{
#if ALIMER_PLATFORM_ANDROID || ALIMER_PLATFORM_APPLE_IOS || ALIMER_PLATFORM_APPLE_TV
		return PlatformFamily::Mobile;
#elif ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP || ALIMER_PLATFORM_LINUX || ALIMER_PLATFORM_APPLE_OSX
		return PlatformFamily::Desktop;
#elif ALIMER_PLATFORM_WEB
		return PlatformFamily::Console;
#else
		return PlatformFamily::Unknown;
#endif
	}

	const char* GetPlatformName()
	{
		PlatformID platform = GetPlatformID();
		switch (platform)
		{
		case PlatformID::Windows:
			return "Windows";
		case PlatformID::WindowsUniversal:
			return "UWP";
		case PlatformID::Linux:
			return "Linux";
		case PlatformID::macOS:
			return "macOS";
		case PlatformID::Android:
			return "Android";
		case PlatformID::iOS:
			return "iOS";
		case PlatformID::AppleTV:
			return "AppleTV";
		case PlatformID::Web:
			return "Web";
		default:
			return "Unknown";
		}
	}
}
