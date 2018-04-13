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

#include "Console.h"
#include "Base/String.h"

#ifdef _WIN32
// This is needed for _fdopen on MinGW
#	undef __STRICT_ANSI__
#	include "../PlatformIncl.h"
#	include <io.h>
#else
#	include <unistd.h>
#endif

#include <cstdio>
#include <fcntl.h>

using namespace std;

namespace Alimer
{
#ifdef _WIN32
	static bool consoleOpened = false;

	inline std::wstring ToWide(const std::string& str)
	{
		int len;
		int slength = (int)str.length() + 1;
		len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, nullptr, 0);
		wchar_t* buf = new wchar_t[len];
		MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, buf, len);
		std::wstring r(buf);
		delete[] buf;
		return r;
	}

#endif
	static string currentLine;

	void ErrorExit(const string& message, int exitCode)
	{
		if (!message.empty())
			PrintLine(message, true);

		exit(exitCode);
	}

	void OpenConsoleWindow()
	{
#ifdef _WIN32
		if (consoleOpened)
			return;

		AllocConsole();

		HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		int hCrt = _open_osfhandle((size_t)hOut, _O_TEXT);
		FILE* outFile = _fdopen(hCrt, "w");
		setvbuf(outFile, 0, _IONBF, 1);
		*stdout = *outFile;

		HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
		hCrt = _open_osfhandle((size_t)hIn, _O_TEXT);
		FILE* inFile = _fdopen(hCrt, "r");
		setvbuf(inFile, 0, _IONBF, 128);
		*stdin = *inFile;

		consoleOpened = true;
#endif
	}

	void PrintUnicode(const string& str, bool error)
	{
#if !defined(ANDROID) && !defined(IOS)
#ifdef _WIN32
		HANDLE stream = GetStdHandle(error ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE);
		if (stream == INVALID_HANDLE_VALUE)
			return;

		wstring strW = ToWide(str);
		DWORD charsWritten;
		WriteConsoleW(stream, strW.c_str(), (DWORD)strW.length(), &charsWritten, 0);
#else
		fprintf(error ? stderr : stdout, "%s", str.CString());
#endif
#endif
	}

	void PrintUnicodeLine(const string& str, bool error)
	{
		PrintUnicode(str + '\n', error);
	}

	void PrintLine(const string& str, bool error)
	{
#if !defined(ANDROID) && !defined(IOS)
		fprintf(error ? stderr : stdout, "%s\n", str.c_str());
#endif
	}

	string ReadLine()
	{
		string ret;

#ifdef _WIN32
		HANDLE input = GetStdHandle(STD_INPUT_HANDLE);
		HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
		if (input == INVALID_HANDLE_VALUE
			|| output == INVALID_HANDLE_VALUE)
		{
			return ret;
		}

		// Use char-based input
		SetConsoleMode(input, ENABLE_PROCESSED_INPUT);

		INPUT_RECORD record;
		DWORD events = 0;
		DWORD readEvents = 0;

		if (!GetNumberOfConsoleInputEvents(input, &events))
			return ret;

		while (events--)
		{
			ReadConsoleInputW(input, &record, 1, &readEvents);
			if (record.EventType == KEY_EVENT && record.Event.KeyEvent.bKeyDown)
			{
				unsigned c = record.Event.KeyEvent.uChar.UnicodeChar;
				if (c)
				{
					if (c == '\b')
					{
						PrintUnicode("\b \b");
						size_t length = currentLine.length();
						if (length)
							currentLine = currentLine.substr(0, length - 1);
					}
					else if (c == '\r')
					{
						PrintUnicode("\n");
						ret = currentLine;
						currentLine.clear();
						return ret;
					}
					else
					{
						// We have disabled echo, so echo manually
						wchar_t out = (wchar_t)c;
						DWORD charsWritten;
						WriteConsoleW(output, &out, 1, &charsWritten, 0);

						// AppendUTF8
						char temp[7];
						char* dest = temp;
						String::EncodeUTF8(dest, c);
						*dest = 0;
						currentLine.append(temp);
					}
				}
			}
		}
#else
		int flags = fcntl(STDIN_FILENO, F_GETFL);
		fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
		for (;;)
		{
			int ch = fgetc(stdin);
			if (ch >= 0 && ch != '\n')
				ret += (char)ch;
			else
				break;
		}
#endif

		return ret;
	}

}
