// For conditions of distribution and use, see copyright notice in License.txt

#pragma once

#include "../AlimerConfig.h"

#include <cassert>
#include <cstddef>

namespace Alimer
{

	/// Pointer which takes ownership of an object and deletes it when the pointer goes out of scope. Ownership can be transferred to another pointer, in which case the source pointer becomes null.
	template <class T> class AutoPtr
	{
	public:
		/// Construct a null pointer.
		AutoPtr() :
			ptr(nullptr)
		{
		}

		/// Copy-construct. Ownership is transferred, making the source pointer null.
		AutoPtr(const AutoPtr<T>& ptr_) :
			ptr(ptr_.ptr)
		{
			// Trick the compiler so that the AutoPtr can be copied to containers; the latest copy stays non-null
			const_cast<AutoPtr<T>&>(ptr_).ptr = nullptr;
		}

		/// Construct with a raw pointer; take ownership of the object.
		AutoPtr(T* ptr_) :
			ptr(ptr_)
		{
		}

		/// Destruct. Delete the object pointed to.
		~AutoPtr()
		{
			delete ptr;
		}

		/// Assign from a pointer. Existing object is deleted and ownership is transferred from the source pointer, which becomes null.
		AutoPtr<T>& operator = (const AutoPtr<T>& rhs)
		{
			delete ptr;
			ptr = rhs.ptr;
			const_cast<AutoPtr<T>&>(rhs).ptr = nullptr;
			return *this;
		}

		/// Assign a new object. Existing object is deleted.
		AutoPtr<T>& operator = (T* rhs)
		{
			delete ptr;
			ptr = rhs;
			return *this;
		}

		/// Detach the object from the pointer without destroying it and return it. The pointer becomes null.
		T* Detach()
		{
			T* ret = ptr;
			ptr = nullptr;
			return ret;
		}

		/// Reset to null. Destroys the object.
		void Reset()
		{
			*this = nullptr;
		}

		/// Point to the object.
		T* operator -> () const { assert(ptr); return ptr; }
		/// Dereference the object.
		T& operator * () const { assert(ptr); return *ptr; }
		/// Convert to the object.
		operator T* () const { return ptr; }

		/// Return the object.
		T* Get() const { return ptr; }
		/// Return whether is a null pointer.
		bool IsNull() const { return ptr == nullptr; }

	private:
		/// %Object pointer.
		T * ptr;
	};

}
