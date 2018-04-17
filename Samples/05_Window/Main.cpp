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

#include "Alimer.h"

#ifdef _MSC_VER
#include <crtdbg.h>
#endif

#include <cstdio>
#include <cstdlib>

using namespace Alimer;

class WindowTest : public Object
{
	ALIMER_OBJECT(WindowTest, Object);

public:
	void Run()
	{
		input = std::make_unique<Input>();
		window = std::make_unique<Window>();
		window->SetTitle("Window test");
		window->SetSize(Size(800, 600), false, true);
		printf("Window opened\n");

		SubscribeToEvent(window->closeRequestEvent, &WindowTest::HandleCloseRequest);
		SubscribeToEvent(window->resizeEvent, &WindowTest::HandleResize);
		SubscribeToEvent(window->gainFocusEvent, &WindowTest::HandleGainFocus);
		SubscribeToEvent(window->loseFocusEvent, &WindowTest::HandleLoseFocus);
		SubscribeToEvent(window->minimizeEvent, &WindowTest::HandleMinimize);
		SubscribeToEvent(window->restoreEvent, &WindowTest::HandleRestore);
		SubscribeToEvent(input->mouseButtonEvent, &WindowTest::HandleMouseButton);
		SubscribeToEvent(input->mouseMoveEvent, &WindowTest::HandleMouseMove);
		SubscribeToEvent(input->keyEvent, &WindowTest::HandleKey);
		SubscribeToEvent(input->touchBeginEvent, &WindowTest::HandleTouchBegin);
		SubscribeToEvent(input->touchMoveEvent, &WindowTest::HandleTouchMove);
		SubscribeToEvent(input->touchEndEvent, &WindowTest::HandleTouchEnd);

		while (window->IsOpen())
		{
			input->Update();
			Thread::Sleep(1);
		}

		printf("Window closed\n");
	}

	void HandleCloseRequest(Event& /* event */)
	{
		printf("Close button pressed\n");
		window->Close();
	}

	void HandleResize(WindowResizeEvent& event)
	{
		printf("Window resized to %d %d\n", event.size.width, event.size.height);
	}

	void HandleGainFocus(Event& /* event */)
	{
		printf("Window gained focus\n");
	}

	void HandleLoseFocus(Event& /* event */)
	{
		printf("Window lost focus\n");
	}

	void HandleMinimize(Event& /* event */)
	{
		printf("Window minimized\n");
	}

	void HandleRestore(Event& /* event */)
	{
		printf("Window restored\n");
	}

	void HandleMouseMove(MouseMoveEvent& event)
	{
		printf("Mouse position %d %d delta %d %d\n", event.position.x, event.position.y, event.delta.x, event.delta.y);
	}

	void HandleMouseButton(MouseButtonEvent& event)
	{
		printf("Mouse button %d state %d\n", event.button, event.pressed ? 1 : 0);
	}

	void HandleKey(KeyEvent& event)
	{
		printf("Key code %d rawcode %d state %d\n", event.keyCode, event.rawKeyCode, event.pressed ? 1 : 0);
	}

	void HandleTouchBegin(TouchBeginEvent& event)
	{
		printf("Touch begin id %d position %d %d\n", event.id, event.position.x, event.position.y);
	}

	void HandleTouchMove(TouchMoveEvent& event)
	{
		printf("Touch move id %d position %d %d\n", event.id, event.position.x, event.position.y);
	}

	void HandleTouchEnd(TouchEndEvent& event)
	{
		printf("Touch end id %d position %d %d\n", event.id, event.position.x, event.position.y);
	}

	std::unique_ptr<Input> input;
	std::unique_ptr<Window> window;
};

int main()
{
#ifdef _MSC_VER
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	WindowTest test;
	test.Run();

	return 0;
}