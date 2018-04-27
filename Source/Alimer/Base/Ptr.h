// For conditions of distribution and use, see copyright notice in License.txt

#pragma once

#include "../AlimerConfig.h"

#include <cassert>
#include <cstddef>

namespace Alimer
{

	class RefCounted;
	template <class T> class WeakPtr;

	/// Reference count structure. Used in both intrusive and non-intrusive reference counting.
	struct ALIMER_API RefCount
	{
		/// Construct with zero refcounts.
		RefCount() :
			refs(0),
			weakRefs(0),
			expired(false)
		{
		}

		/// Number of strong references. These keep the object alive.
		unsigned refs;
		/// Number of weak references.
		unsigned weakRefs;
		/// Expired flag. The object is no longer safe to access after this is set true.
		bool expired;
	};

	/// Base class for intrusively reference counted objects that can be pointed to with SharedPtr and WeakPtr. These are not copy-constructible and not assignable.
	class ALIMER_API RefCounted
	{
	public:
		/// Construct. The reference count is not allocated yet; it will be allocated on demand.
		RefCounted();

		/// Destruct. If no weak references, destroy also the reference count, else mark it expired.
		virtual ~RefCounted();

		/// Add a strong reference. Allocate the reference count structure first if necessary.
		void AddRef();
		/// Release a strong reference. Destroy the object when the last strong reference is gone.
		void ReleaseRef();

		/// Return the number of strong references.
		unsigned Refs() const { return refCount ? refCount->refs : 0; }
		/// Return the number of weak references.
		unsigned WeakRefs() const { return refCount ? refCount->weakRefs : 0; }
		/// Return pointer to the reference count structure. Allocate if not allocated yet.
		RefCount* RefCountPtr();

	private:
		/// Prevent copy construction and assignment.
		RefCounted(const RefCounted&) = delete;
		RefCounted& operator=(const RefCounted&) = delete;
		/// Prevent move construction and assignment.
		RefCounted(const RefCounted&&) = delete;
		RefCounted& operator=(const RefCounted&&) = delete;

		/// Reference count structure, allocated on demand.
		RefCount* refCount;
	};

	/// Pointer which holds a strong reference to a RefCounted subclass and allows shared ownership.
	template <class T> class SharedPtr
	{
	public:
		/// Construct a null pointer.
		SharedPtr() :
			ptr(nullptr)
		{
		}

		/// Copy-construct.
		SharedPtr(const SharedPtr<T>& ptr_) :
			ptr(nullptr)
		{
			*this = ptr_;
		}

		/// Construct from a raw pointer.
		SharedPtr(T* ptr_) :
			ptr(nullptr)
		{
			*this = ptr_;
		}

		/// Destruct. Release the object reference and destroy the object if was the last strong reference.
		~SharedPtr()
		{
			Reset();
		}

		/// Assign a raw pointer.
		SharedPtr<T>& operator = (T* rhs)
		{
			if (Get() == rhs)
				return *this;

			Reset();
			ptr = rhs;
			if (ptr)
				ptr->AddRef();
			return *this;
		}

		/// Assign another shared pointer.
		SharedPtr<T>& operator = (const SharedPtr<T>& rhs)
		{
			if (*this == rhs)
				return *this;

			Reset();
			ptr = rhs.ptr;
			if (ptr)
				ptr->AddRef();
			return *this;
		}

		/// Release the object reference and reset to null. Destroy the object if was the last reference.
		void Reset()
		{
			if (ptr)
			{
				ptr->ReleaseRef();
				ptr = nullptr;
			}
		}

		/// Perform a static cast from a shared pointer of another type.
		template <class U> void StaticCast(const SharedPtr<U>& rhs)
		{
			*this = static_cast<T*>(rhs.ptr);
		}

		/// Perform a dynamic cast from a weak pointer of another type.
		template <class U> void DynamicCast(const WeakPtr<U>& rhs)
		{
			Reset();
			T* rhsObject = dynamic_cast<T*>(rhs.ptr);
			if (rhsObject)
				*this = rhsObject;
		}

		/// Test for equality with another shared pointer.
		bool operator == (const SharedPtr<T>& rhs) const { return ptr == rhs.ptr; }
		/// Test for equality with a raw pointer.
		bool operator == (T* rhs) const { return ptr == rhs; }
		/// Test for inequality with another shared pointer.
		bool operator != (const SharedPtr<T>& rhs) const { return !(*this == rhs); }
		/// Test for inequality with a raw pointer.
		bool operator != (T* rhs) const { return !(*this == rhs); }
		/// Point to the object.
		T* operator -> () const { assert(ptr); return ptr; }
		/// Dereference the object.
		T& operator * () const { assert(ptr); return ptr; }
		/// Convert to the object.
		operator T* () const { return ptr; }

		/// Return the object.
		T* Get() const { return ptr; }
		/// Return the number of strong references.
		unsigned Refs() const { return ptr ? ptr->Refs() : 0; }
		/// Return the number of weak references.
		unsigned WeakRefs() const { return ptr ? ptr->WeakRefs() : 0; }
		/// Return whether is a null pointer.
		bool IsNull() const { return ptr == nullptr; }

	private:
		/// %Object pointer.
		T * ptr;
	};

	/// Perform a static cast between shared pointers of two types.
	template <class T, class U> SharedPtr<T> StaticCast(const SharedPtr<U>& rhs)
	{
		SharedPtr<T> ret;
		ret.StaticCast(rhs);
		return ret;
	}

	/// Perform a dynamic cast between shared pointers of two types.
	template <class T, class U> SharedPtr<T> DynamicCast(const SharedPtr<U>& rhs)
	{
		SharedPtr<T> ret;
		ret.DynamicCast(rhs);
		return ret;
	}

	/// Pointer which holds a weak reference to a RefCounted subclass. Can track destruction but does not keep the object alive.
	template <class T> class WeakPtr
	{
	public:
		/// Construct a null pointer.
		WeakPtr() :
			ptr(nullptr),
			refCount(nullptr)
		{
		}

		/// Copy-construct.
		WeakPtr(const WeakPtr<T>& ptr_) :
			ptr(nullptr),
			refCount(nullptr)
		{
			*this = ptr_;
		}

		/// Construct from a shared pointer.
		WeakPtr(const SharedPtr<T>& ptr_) :
			ptr(nullptr),
			refCount(nullptr)
		{
			*this = ptr_;
		}

		/// Construct from a raw pointer.
		WeakPtr(T* ptr_) :
			ptr(nullptr),
			refCount(nullptr)
		{
			*this = ptr_;
		}

		/// Destruct. Release the object reference.
		~WeakPtr()
		{
			Reset();
		}

		/// Assign another weak pointer.
		WeakPtr<T>& operator = (const WeakPtr<T>& rhs)
		{
			if (*this == rhs)
				return *this;

			Reset();
			ptr = rhs.ptr;
			refCount = rhs.refCount;
			if (refCount)
				++(refCount->weakRefs);
			return *this;
		}

		/// Assign a shared pointer.
		WeakPtr<T>& operator = (const SharedPtr<T>& rhs)
		{
			if (*this == rhs)
				return *this;

			Reset();
			ptr = rhs.Get();
			refCount = ptr ? ptr->RefCountPtr() : nullptr;
			if (refCount)
				++(refCount->weakRefs);
			return *this;
		}

		/// Assign a raw pointer.
		WeakPtr<T>& operator = (T* rhs)
		{
			if (Get() == rhs)
				return *this;

			Reset();
			ptr = rhs;
			refCount = ptr ? ptr->RefCountPtr() : nullptr;
			if (refCount)
				++(refCount->weakRefs);
			return *this;
		}

		/// Release the weak object reference and reset to null.
		void Reset()
		{
			if (refCount)
			{
				--(refCount->weakRefs);
				// If expired and no more weak references, destroy the reference count
				if (refCount->expired && refCount->weakRefs == 0)
					delete refCount;
				ptr = nullptr;
				refCount = nullptr;
			}
		}

		/// Perform a static cast from a weak pointer of another type.
		template <class U> void StaticCast(const WeakPtr<U>& rhs)
		{
			*this = static_cast<T*>(rhs.ptr);
		}

		/// Perform a dynamic cast from a weak pointer of another type.
		template <class U> void DynamicCast(const WeakPtr<U>& rhs)
		{
			Reset();
			T* rhsObject = dynamic_cast<T*>(rhs.ptr);
			if (rhsObject)
				*this = rhsObject;
		}

		/// Test for equality with another weak pointer.
		bool operator == (const WeakPtr<T>& rhs) const { return ptr == rhs.ptr && refCount == rhs.refCount; }
		/// Test for equality with a shared pointer.
		bool operator == (const SharedPtr<T>& rhs) const { return ptr == rhs.Get(); }
		/// Test for equality with a raw pointer.
		bool operator == (T* rhs) const { return ptr == rhs; }
		/// Test for inequality with another weak pointer.
		bool operator != (const WeakPtr<T>& rhs) const { return !(*this == rhs); }
		/// Test for inequality with a shared pointer.
		bool operator != (const SharedPtr<T>& rhs) const { return !(*this == rhs); }
		/// Test for inequality with a raw pointer.
		bool operator != (T* rhs) const { return !(*this == rhs); }
		/// Point to the object.
		T* operator -> () const { T* ret = Get(); assert(ret); return ret; }
		/// Dereference the object.
		T& operator * () const { T* ret = Get(); assert(ret); return ret; }
		/// Convert to the object.
		operator T* () const { return Get(); }

		/// Return the object or null if it has been destroyed.
		T* Get() const
		{
			if (refCount && !refCount->expired)
				return ptr;
			else
				return nullptr;
		}

		/// Return the number of strong references.
		unsigned Refs() const { return refCount ? refCount->refs : 0; }
		/// Return the number of weak references.
		unsigned WeakRefs() const { return refCount ? refCount->weakRefs : 0; }
		/// Return whether is a null pointer.
		bool IsNull() const { return ptr == nullptr; }
		/// Return whether the object has been destroyed. Returns false if is a null pointer.
		bool IsExpired() const { return refCount && refCount->expired; }

	private:
		/// %Object pointer.
		T * ptr;
		/// The object's weak reference count structure.
		RefCount* refCount;
	};

	/// Perform a static cast between weak pointers of two types.
	template <class T, class U> WeakPtr<T> StaticCast(const WeakPtr<U>& rhs)
	{
		WeakPtr<T> ret;
		ret.StaticCast(rhs);
		return ret;
	}

	/// Perform a dynamic cast between weak pointers of two types.
	template <class T, class U> WeakPtr<T> DynamicCast(const WeakPtr<U>& rhs)
	{
		WeakPtr<T> ret;
		ret.DynamicCast(rhs);
		return ret;
	}
}
