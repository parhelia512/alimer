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

#include "Input.h"
#include "Window.h"

namespace Alimer
{
	Input::Input()
		: _mouseButtons(0)
		, _mouseButtonsPressed(0)
	{
		RegisterSubsystem(this);
	}

	Input::~Input()
	{
		RemoveSubsystem(this);
	}

	Input* Input::GetInput()
	{
		static Input input;
		return &input;
	}

	/*void Input::Update()
	{
		// Clear accumulated input from last frame
		_mouseButtonsPressed = 0;
		mouseMove = IntVector2::ZERO;
		keyPressed.clear();
		rawKeyPress.clear();
		for (auto it = touches.begin(); it != touches.end(); ++it)
			it->delta = IntVector2::ZERO;
	}*/

	bool Input::IsKeyDown(Key key)
	{
		return GetInput()->IsKeyState(key, true);
	}

	const IntVector2& Input::MousePosition() const
	{
		Window* window = GetSubsystem<Window>();
		return window ? window->GetMousePosition() : IntVector2::ZERO;
	}

	bool Input::IsMouseButtonDown(unsigned button) const
	{
		return (_mouseButtons & (1 << button)) != 0;
	}

	bool Input::IsMouseButtonPress(unsigned button) const
	{
		return (_mouseButtonsPressed & (1 << button)) != 0;
	}

	const Touch* Input::FindTouch(unsigned id) const
	{
		for (auto it = touches.begin(); it != touches.end(); ++it)
		{
			if (it->id == id)
				return &(*it);
		}

		return nullptr;
	}

	void Input::PostKeyEvent(Key key, bool pressed)
	{
		bool wasDown = IsKeyDown(key);

		_keyDown[key] = pressed;
		if (pressed)
		{
			_keyPressed[key] = true;
		}

		keyEvent.key = key;
		keyEvent.pressed = pressed;
		keyEvent.repeat = wasDown;
		SendEvent(keyEvent);
	}

	void Input::OnChar(unsigned unicodeChar)
	{
		charInputEvent.unicodeChar = unicodeChar;
		SendEvent(charInputEvent);
	}

	void Input::OnMouseMove(const IntVector2& position, const IntVector2& delta)
	{
		mouseMove += delta;

		mouseMoveEvent.position = position;
		mouseMoveEvent.delta = delta;
		SendEvent(mouseMoveEvent);
	}

	void Input::OnMouse(uint32_t x, uint32_t y, MouseButton button, bool pressed)
	{
		uint32_t bit = 1 << ecast(button);

		if (pressed)
		{
			_mouseButtons |= bit;
			_mouseButtonsPressed |= bit;
		}
		else
			_mouseButtons &= ~bit;

		mouseButtonEvent.button = button;
		mouseButtonEvent.buttons = _mouseButtons;
		mouseButtonEvent.pressed = pressed;
		mouseButtonEvent.position = MousePosition();
		SendEvent(mouseButtonEvent);
	}

	void Input::OnTouch(unsigned internalId, bool pressed, const IntVector2& position, float pressure)
	{
		if (pressed)
		{
			bool found = false;

			// Ongoing touch
			for (auto it = touches.begin(); it != touches.end(); ++it)
			{
				if (it->internalId == internalId)
				{
					found = true;
					it->lastDelta = position - it->position;

					if (it->lastDelta != IntVector2::ZERO || pressure != it->pressure)
					{
						it->delta += it->lastDelta;
						it->position = position;
						it->pressure = pressure;

						touchMoveEvent.id = it->id;
						touchMoveEvent.position = it->position;
						touchMoveEvent.delta = it->lastDelta;
						touchMoveEvent.pressure = it->pressure;
						SendEvent(touchMoveEvent);
					}

					break;
				}
			}

			// New touch
			if (!found)
			{
				// Use the first gap in current touches, or insert to the end if no gaps
				size_t insertIndex = touches.size();
				unsigned newId = (unsigned)touches.size();

				for (size_t i = 0; i < touches.size(); ++i)
				{
					if (touches[i].id > i)
					{
						insertIndex = i;
						newId = i ? touches[i - 1].id + 1 : 0;
						break;
					}
				}

				Touch newTouch;
				newTouch.id = newId;
				newTouch.internalId = internalId;
				newTouch.position = position;
				newTouch.pressure = pressure;
				touches.insert(touches.begin() + insertIndex, newTouch);

				touchBeginEvent.id = newTouch.id;
				touchBeginEvent.position = newTouch.position;
				touchBeginEvent.pressure = newTouch.pressure;
				SendEvent(touchBeginEvent);
			}
		}
		else
		{
			// End touch
			for (auto it = touches.begin(); it != touches.end(); ++it)
			{
				if (it->internalId == internalId)
				{
					it->position = position;
					it->pressure = pressure;

					touchEndEvent.id = it->id;
					touchEndEvent.position = it->position;
					SendEvent(touchEndEvent);
					touches.erase(it);
					break;
				}
			}
		}
	}

	void Input::OnGainFocus()
	{
	}

	void Input::OnLoseFocus()
	{
		_mouseButtons = 0;
		_mouseButtonsPressed = 0;
		mouseMove = IntVector2::ZERO;
		_keyDown.clear();
		_keyPressed.clear();
	}

	void Input::BeginFrame()
	{
		// Clear accumulated input from last frame
		_mouseButtonsPressed = 0;
		mouseMove = IntVector2::ZERO;
		_keyPressed.clear();
		for (auto& touch : touches)
		{
			touch.delta = IntVector2::ZERO;
		}
	}

	bool Input::IsKeyState(Key key, bool down)
	{
		auto it = _keyDown.find(key);
		return it != _keyDown.end() ? it->second : false;
	}
}
