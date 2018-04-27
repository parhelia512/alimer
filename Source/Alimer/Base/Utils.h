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
#include <string>
#include <functional>
#include <type_traits>

namespace Alimer
{
	template <typename FlagBitsType> struct FlagTraits
	{
		enum { allFlags = 0 };
	};

	template <typename BitType, typename MaskType = uint32_t>
	class Flags
	{
	public:
		ALIMER_CONSTEXPR Flags()
			: _mask(0)
		{
		}

		Flags(BitType bit)
			: _mask(static_cast<MaskType>(bit))
		{
		}

		Flags(Flags<BitType> const& rhs)
			: _mask(rhs._mask)
		{
		}

		explicit Flags(MaskType flags)
			: _mask(flags)
		{
		}

		Flags<BitType> & operator=(Flags<BitType> const& rhs)
		{
			_mask = rhs._mask;
			return *this;
		}

		Flags<BitType> & operator|=(Flags<BitType> const& rhs)
		{
			_mask |= rhs._mask;
			return *this;
		}

		Flags<BitType> & operator&=(Flags<BitType> const& rhs)
		{
			_mask &= rhs._mask;
			return *this;
		}

		Flags<BitType> & operator^=(Flags<BitType> const& rhs)
		{
			_mask ^= rhs._mask;
			return *this;
		}

		Flags<BitType> operator|(Flags<BitType> const& rhs) const
		{
			Flags<BitType> result(*this);
			result |= rhs;
			return result;
		}

		Flags<BitType> operator&(Flags<BitType> const& rhs) const
		{
			Flags<BitType> result(*this);
			result &= rhs;
			return result;
		}

		Flags<BitType> operator^(Flags<BitType> const& rhs) const
		{
			Flags<BitType> result(*this);
			result ^= rhs;
			return result;
		}

		bool operator!() const
		{
			return !_mask;
		}

		Flags<BitType> operator~() const
		{
			Flags<BitType> result(*this);
			result._mask ^= FlagTraits<BitType>::allFlags;
			return result;
		}

		bool operator==(Flags<BitType> const& rhs) const
		{
			return _mask == rhs._mask;
		}

		bool operator!=(Flags<BitType> const& rhs) const
		{
			return _mask != rhs._mask;
		}

		explicit operator bool() const
		{
			return !!_mask;
		}

		explicit operator MaskType() const
		{
			return _mask;
		}

	private:
		MaskType _mask;
	};

	template <typename BitType>
	Flags<BitType> operator|(BitType bit, Flags<BitType> const& flags)
	{
		return flags | bit;
	}

	template <typename BitType>
	Flags<BitType> operator&(BitType bit, Flags<BitType> const& flags)
	{
		return flags & bit;
	}

	template <typename BitType>
	Flags<BitType> operator^(BitType bit, Flags<BitType> const& flags)
	{
		return flags ^ bit;
	}

	// avoid unreferenced parameter warning
	// preferred solution: omit the parameter's name from the declaration
	template <class T>
	void Unused(T const&)
	{
	}

	template <typename T, unsigned int N>
	void SafeRelease(T(&resourceBlock)[N])
	{
		for (unsigned int i = 0; i < N; i++)
		{
			SafeRelease(resourceBlock[i]);
		}
	}

	template <typename T>
	void SafeRelease(T& resource)
	{
		if (resource)
		{
			resource->Release();
			resource = nullptr;
		}
	}

	template <typename T>
	void SafeDelete(T *&resource)
	{
		delete resource;
		resource = nullptr;
	}

	template <typename T>
	void SafeDeleteContainer(T& resource)
	{
		for (auto &element : resource)
		{
			SafeDelete(element);
		}
		resource.clear();
	}

	template <typename T>
	void SafeDeleteArray(T*& resource)
	{
		delete[] resource;
		resource = nullptr;
	}

	// Enum cast
	template <typename T>
	constexpr typename std::underlying_type<T>::type ecast(T x)
	{
		return static_cast<typename std::underlying_type<T>::type>(x);
	}
}

// Put this in the declarations for a class to be uncopyable and unassignable.
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
	TypeName(const TypeName&) = delete; \
	TypeName& operator=(const TypeName&) = delete

// Put this in the declarations for a class to be uncopyable and unassignable.
#define DISALLOW_COPY_MOVE_AND_ASSIGN(TypeName) \
	TypeName(const TypeName&) = delete; \
	TypeName& operator=(const TypeName&) = delete; \
	TypeName(const TypeName&&) = delete; \
	TypeName& operator=(const TypeName&&) = delete;

// Utility to enable bitmask operators on enum classes.
// To use define an enum class with valid bitmask values and an underlying type
// then use the macro to enable support:
//  enum class MyBitmask : uint32_t {
//    Foo = 1 << 0,
//    Bar = 1 << 1,
//  };
//  ALIMER_BITMASK(MyBitmask);
//  MyBitmask value = ~(MyBitmask::Foo | MyBitmask::Bar);
#define ALIMER_BITMASK(enum_class) \
  inline enum_class operator|(enum_class lhs, enum_class rhs) { \
	typedef typename std::underlying_type<enum_class>::type enum_type; \
	return static_cast<enum_class>(static_cast<enum_type>(lhs) |       \
								   static_cast<enum_type>(rhs));       \
  }                                                                    \
  inline enum_class& operator|=(enum_class& lhs, enum_class rhs) {     \
	typedef typename std::underlying_type<enum_class>::type enum_type; \
	lhs = static_cast<enum_class>(static_cast<enum_type>(lhs) |        \
								  static_cast<enum_type>(rhs));        \
	return lhs;                                                        \
  }                                                                    \
  inline enum_class operator&(enum_class lhs, enum_class rhs) {        \
	typedef typename std::underlying_type<enum_class>::type enum_type; \
	return static_cast<enum_class>(static_cast<enum_type>(lhs) &       \
								   static_cast<enum_type>(rhs));       \
  }                                                                    \
  inline enum_class& operator&=(enum_class& lhs, enum_class rhs) {     \
	typedef typename std::underlying_type<enum_class>::type enum_type; \
	lhs = static_cast<enum_class>(static_cast<enum_type>(lhs) &        \
								  static_cast<enum_type>(rhs));        \
	return lhs;                                                        \
  }                                                                    \
  inline enum_class operator^(enum_class lhs, enum_class rhs) {        \
	typedef typename std::underlying_type<enum_class>::type enum_type; \
	return static_cast<enum_class>(static_cast<enum_type>(lhs) ^       \
								   static_cast<enum_type>(rhs));       \
  }                                                                    \
  inline enum_class& operator^=(enum_class& lhs, enum_class rhs) {     \
	typedef typename std::underlying_type<enum_class>::type enum_type; \
	lhs = static_cast<enum_class>(static_cast<enum_type>(lhs) ^        \
								  static_cast<enum_type>(rhs));        \
	return lhs;                                                        \
  }                                                                    \
  inline enum_class operator~(enum_class lhs) {                        \
	typedef typename std::underlying_type<enum_class>::type enum_type; \
	return static_cast<enum_class>(~static_cast<enum_type>(lhs));      \
  }                                                                    \
  inline bool any(enum_class lhs) {                                    \
	typedef typename std::underlying_type<enum_class>::type enum_type; \
	return static_cast<enum_type>(lhs) != 0;                           \
  }