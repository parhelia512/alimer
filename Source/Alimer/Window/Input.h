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

#include "../Math/IntVector2.h"
#include "../Object/Object.h"

namespace Alimer
{
	/// Defines Keyboard key
	enum class Key : uint8_t
	{
		None = 0,
		A,
		B,
		C,
		D,
		E,
		F,
		G,
		H,
		I,
		J,
		K,
		L,
		M,
		N,
		O,
		P,
		Q,
		R,
		S,
		T,
		U,
		V,
		W,
		X,
		Y,
		Z,
		D0,
		D1,
		D2,
		D3,
		D4,
		D5,
		D6,
		D7,
		D8,
		D9,
		Numpad0,
		Numpad1,
		Numpad2,
		Numpad3,
		Numpad4,
		Numpad5,
		Numpad6,
		Numpad7,
		Numpad8,
		Numpad9,
		Return,
		Escape,
		Backspace,
		Tab,
		Space,
		Up,
		Down,
		Left,
		Right,
		Insert,
		Delete,
		Home,
		End,
		PageUp,
		PageDown,
		Print,
		Plus,
		Minus,
		F1,
		F2,
		F3,
		F4,
		F5,
		F6,
		F7,
		F8,
		F9,
		F10,
		F11,
		F12,
		F13,
		F14,
		F15,
		F16,
		F17,
		F18,
		F19,
		F20,
		F21,
		F22,
		F23,
		F24,
		Count
	};

	/// Defines input mouse buttons.
	enum class MouseButton
	{
		None = 0,
		Left,
		Middle,
		Right,
		X1,
		X2,
		Count
	};

	/// Finger touch.
	struct ALIMER_API Touch
	{
		/// Construct.
		Touch() :
			delta(IntVector2::ZERO),
			lastDelta(IntVector2::ZERO)
		{
		}

		/// Zero-based touch id.
		unsigned id;
		/// Operating system id, which may be an arbitrary number.
		unsigned internalId;
		/// Position within window.
		IntVector2 position;
		/// Accumulated delta on this frame.
		IntVector2 delta;
		/// Delta from last move event.
		IntVector2 lastDelta;
		/// Current finger pressure.
		float pressure;
	};

	/// Key press or release event.
	class ALIMER_API KeyEvent : public Event
	{
	public:
		/// Key code.
		Key key;
		/// Pressed flag.
		bool pressed;
		/// Repeat flag.
		bool repeat;
	};

	/// Unicode character input event.
	class ALIMER_API CharInputEvent : public Event
	{
	public:
		/// Unicode codepoint.
		unsigned unicodeChar;
	};

	/// Mouse button press or release event.
	class ALIMER_API MouseButtonEvent : public Event
	{
	public:
		/// Button index.
		MouseButton button;
		/// Bitmask of currently held down buttons.
		uint32_t buttons;
		/// Pressed flag.
		bool pressed;
		/// Mouse position within window.
		IntVector2 position;
	};

	/// Mouse move event.
	class ALIMER_API MouseMoveEvent : public Event
	{
	public:
		/// Bitmask of currently held down buttons.
		unsigned buttons;
		/// Mouse position within window.
		IntVector2 position;
		/// Delta from last position.
		IntVector2 delta;
	};

	/// Touch begin event.
	class ALIMER_API TouchBeginEvent : public Event
	{
	public:
		/// Zero-based touch id.
		unsigned id;
		/// Touch position within window.
		IntVector2 position;
		/// Finger pressure between 0-1.
		float pressure;
	};

	/// Touch move event.
	class ALIMER_API TouchMoveEvent : public Event
	{
	public:
		/// Zero-based touch id.
		unsigned id;
		/// Touch position within window.
		IntVector2 position;
		/// Delta from last position.
		IntVector2 delta;
		/// Finger pressure between 0-1.
		float pressure;
	};

	/// Touch end event.
	class ALIMER_API TouchEndEvent : public Event
	{
	public:
		/// Zero-based touch id.
		unsigned id;
		/// Touch position within window.
		IntVector2 position;
	};

	/// Cursor platform API.
	class ALIMER_API Cursor
	{
	public:
		static bool IsVisible();
		static void SetVisible(bool visible);
	};

	/// %Input subsystem for reading keyboard/mouse/etc. input. Updated from OS window messages by the Window class.
	class ALIMER_API Input final : public Object
	{
		ALIMER_OBJECT(Input, Object);
		friend class Application;

	private:
		/// Construct and register subsystem.
		Input();
		/// Destruct.
		~Input() override;

	public:
		/// Singleton Input instance.
		static Input* GetInput();

		/// Return whether key is down by key code.
		static bool IsKeyDown(Key key);
		/// Return current mouse position.
		const IntVector2& MousePosition() const;
		/// Return accumulated mouse movement since last frame.
		IntVector2 GetMouseMove() const { return _mouseMove; }
		/// Return pressed down mouse buttons bitmask.
		unsigned GetMouseButtons() const { return _mouseButtons; }
		/// Return whether a mouse button is down.
		bool IsMouseButtonDown(unsigned button) const;
		/// Return whether a mouse button was pressed on this frame.
		bool IsMouseButtonPress(unsigned button) const;
		/// Return number of active touches.
		size_t NumTouches() const { return touches.size(); }
		/// Return an active touch by id, or null if not found.
		const Touch* FindTouch(unsigned id) const;
		/// Return all touches.
		const std::vector<Touch>& Touches() const { return touches; }

		/// Post key event.
		void PostKeyEvent(Key key, bool pressed);
		/// React to char input. Called by window message handling.
		void OnChar(unsigned unicodeChar);
		/// React to a mouse move. Called by window message handling.
		void OnMouseMove(int32_t x, int32_t y, int32_t deltaX, int32_t deltaY);
		/// React to a mouse button. Called by window message handling.
		void OnMouse(int32_t x, int32_t y, MouseButton button, bool pressed);
		/// React to a touch. Called by window message handling.
		void OnTouch(unsigned internalId, bool pressed, const IntVector2& position, float pressure);
		/// React to gaining focus. Called by window message handling.
		void OnGainFocus();
		/// React to losing focus. Called by window message handling.
		void OnLoseFocus();

		/// Key press/release event.
		KeyEvent keyEvent;
		/// Unicode char input event.
		CharInputEvent charInputEvent;
		/// Mouse button press/release event.
		MouseButtonEvent mouseButtonEvent;
		/// Mouse move event.
		MouseMoveEvent mouseMoveEvent;
		/// Touch begin event.
		TouchBeginEvent touchBeginEvent;
		/// Touch move event.
		TouchMoveEvent touchMoveEvent;
		/// Touch end event.
		TouchEndEvent touchEndEvent;

	private:
		void BeginFrame();

		bool IsKeyState(Key key, bool down);

		/// Key code held down status.
		std::map<Key, bool> _keyDown;
		/// Key code pressed status.
		std::map<Key, bool> _keyPressed;
		/// Active touches.
		std::vector<Touch> touches;
		/// Accumulated mouse move since last frame.
		IntVector2 _mouseMove;
		/// Mouse buttons bitmask.
		uint32_t _mouseButtons;
		/// Mouse buttons pressed bitmask.
		uint32_t _mouseButtonsPressed;
	};
}
