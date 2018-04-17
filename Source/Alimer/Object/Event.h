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

#include "../Base/Ptr.h"
#include <memory>
#include <vector>

namespace Alimer
{

	class Event;

	/// Internal helper class for invoking event handler functions.
	class ALIMER_API EventHandler
	{
	public:
		/// Construct with receiver object pointer.
		EventHandler(RefCounted* receiver);
		/// Destruct.
		virtual ~EventHandler() = default;

		/// Invoke the handler function. Implemented by subclasses.
		virtual void Invoke(Event& event) = 0;

		/// Return the receiver object.
		RefCounted* Receiver() const { return receiver.Get(); }

	protected:
		/// Receiver object.
		WeakPtr<RefCounted> receiver;
	};

	/// Template implementation of the event handler invoke helper, stores a function pointer of specific class.
	template <class T, class U> class EventHandlerImpl : public EventHandler
	{
	public:
		typedef void (T::*HandlerFunctionPtr)(U&);

		/// Construct with receiver and function pointers.
		EventHandlerImpl(RefCounted* receiver_, HandlerFunctionPtr function_) :
			EventHandler(receiver_),
			function(function_)
		{
			assert(function);
		}

		/// Invoke the handler function.
		void Invoke(Event& event) override
		{
			T* typedReceiver = static_cast<T*>(receiver.Get());
			U& typedEvent = static_cast<U&>(event);
			(typedReceiver->*function)(typedEvent);
		}

	private:
		/// Pointer to the event handler function.
		HandlerFunctionPtr function;
	};

	/// Notification and data passing mechanism, to which objects can subscribe by specifying a handler function. Subclass to include event-specific data.
	class ALIMER_API Event
	{
	public:
		/// Construct.
		Event();
		/// Destruct.
		virtual ~Event();

		/// Send the event.
		void Send(RefCounted* sender);
		/// Subscribe to the event. The event takes ownership of the handler data. If there is already handler data for the same receiver, it is overwritten.
		void Subscribe(EventHandler* handler);
		/// Unsubscribe from the event.
		void Unsubscribe(RefCounted* receiver);

		/// Return whether has at least one valid receiver.
		bool HasReceivers() const;
		/// Return whether has a specific receiver.
		bool HasReceiver(const RefCounted* receiver) const;
		/// Return current sender.
		RefCounted* Sender() const { return currentSender; }

	private:
		/// Prevent copy construction.
		Event(const Event&) = delete;
		/// Prevent assignment.
		Event& operator = (const Event&) = delete;

		/// Event handlers.
		std::vector<std::unique_ptr<EventHandler>> handlers;
		/// Current sender.
		WeakPtr<RefCounted> currentSender;
	};

}
