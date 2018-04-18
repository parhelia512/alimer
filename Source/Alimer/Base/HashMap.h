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

#include "../PlatformDef.h"
#include <memory>
#include <unordered_map>

namespace Alimer
{
	using Hash = uint64_t;

	struct UnityHasher
	{
		inline size_t operator()(uint64_t hash) const
		{
			return hash;
		}
	};

	template <typename T>
	using HashMap = std::unordered_map<Hash, T, UnityHasher>;

	class Hasher
	{
	public:
		template <typename T>
		inline void Data(const T *data, size_t size)
		{
			size /= sizeof(*data);
			for (size_t i = 0; i < size; i++)
			{
				_value = (_value * 0x100000001b3ull) ^ data[i];
			}
		}

		inline void UInt32(uint32_t value)
		{
			_value = (_value * 0x100000001b3ull) ^ value;
		}

		inline void SInt32(int32_t value)
		{
			UInt32(uint32_t(value));
		}

		inline void Float(float value)
		{
			union
			{
				float f32;
				uint32_t u32;
			} u;
			u.f32 = value;
			UInt32(u.u32);
		}

		inline void UInt64(uint64_t value)
		{
			UInt32(value & 0xffffffffu);
			UInt32(value >> 32);
		}

		inline void Pointer(const void *ptr)
		{
			UInt64(reinterpret_cast<uintptr_t>(ptr));
		}

		inline void String(const char *str)
		{
			char c;
			while ((c = *str++) != '\0')
			{
				UInt32(uint8_t(c));
			}
		}

		inline Hash GetValue() const
		{
			return _value;
		}

	private:
		Hash _value = 0xcbf29ce484222325ull;
	};
}