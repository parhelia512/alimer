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

#include "../AlimerConfig.h"
#include <cstdlib>
#include <string>

namespace Alimer
{

	/// Exit the application with an error message to the console.
	ALIMER_API void ErrorExit(const std::string& message = "", int exitCode = EXIT_FAILURE);
	/// Open a console window.
	ALIMER_API void OpenConsoleWindow();
	/// Print Unicode text to the console. Will not be printed to the MSVC output window.
	ALIMER_API void PrintUnicode(const std::string& str, bool error = false);
	/// Print Unicode text to the console with a newline appended. Will not be printed to the MSVC output window.
	ALIMER_API void PrintUnicodeLine(const std::string& str, bool error = false);
	/// Print ASCII text to the console with a newline appended. Uses printf() to allow printing into the MSVC output window.
	ALIMER_API void PrintLine(const std::string& str, bool error = false);
	/// Read input from the console window. Return empty if no input.
	ALIMER_API std::string ReadLine();

}
