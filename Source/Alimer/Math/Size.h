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

#include "Math/Vector2.h"

namespace Alimer
{
	/**
	* Defines an integer size which specify width and height.
	*/
	class ALIMER_API Size
	{
	public:
		/**
		* The width-component of the size.
		*/
		uint32_t width;

		/**
		* The height-component of the size.
		*/
		uint32_t height;

		/**
		* Constructor.
		*/
		Size() : width(0), height(0) {}

		/**
		* Copy-construct from another size.
		*
		* @param copy The copy size.
		*/
		Size(const Size& copy)
			: width(copy.width), height(copy.height)
		{
		}

		/**
		* Constructs a size initialized to the specified values.
		*
		* @param width_ The width value.
		* @param height_ The height value.
		*/
		Size(uint32_t width_, uint32_t height_)
			: width(width_), height(height_)
		{
		}

		// Comparison operators
		inline bool operator==(const Size& size) const
		{
			return width == size.width && height == size.height;
		}

		inline bool operator!=(const Size& size) const
		{
			return width != size.width || height != size.height;
		}

		// Assignment operators
		Size& operator = (const Size& rhs) { width = rhs.width; height = rhs.height; return *this; }

		Size operator +(const Size& rhs) const { return Size(width + rhs.width, height + rhs.height); }
		Size operator -(const Size& rhs) const { return Size(width - rhs.width, height - rhs.height); }
		Size operator *(uint32_t rhs) const { return Size(width * rhs, height * rhs); }
		Size operator *(const Size& rhs) const { return Size(width * rhs.width, height * rhs.height); }
		Size operator /(uint32_t rhs) const { return Size(width / rhs, height / rhs); }
		Size operator /(const Size& rhs) const { return Size(width / rhs.width, height / rhs.height); }

		Size& operator +=(const Size& rhs)
		{
			width += rhs.width;
			height += rhs.height;
			return *this;
		}

		Size& operator -=(const Size& rhs)
		{
			width -= rhs.width;
			height -= rhs.height;
			return *this;
		}

		Size& operator *=(uint32_t rhs)
		{
			width *= rhs;
			height *= rhs;
			return *this;
		}

		Size& operator *=(const Size& rhs)
		{
			width *= rhs.width;
			height *= rhs.height;
			return *this;
		}

		Size& operator /=(uint32_t rhs)
		{
			width /= rhs;
			height /= rhs;
			return *this;
		}

		Size& operator /=(const Size& rhs)
		{
			width /= rhs.width;
			height /= rhs.height;
			return *this;
		}

		bool IsEmpty() const { return width == 0 && height == 0; }

		/// Return as string.
		std::string ToString() const;

		/// Empty size.
		static const Size Empty;

		/// Size with values (1, 1).
		static const Size One;
	};

	/**
	* Defines a floating size which specify width and height.
	*/
	class ALIMER_API SizeF
	{
	public:
		/**
		* The width-component of the size.
		*/
		float width;

		/**
		* The height-component of the size.
		*/
		float height;

		/**
		* Constructor.
		*/
		SizeF() : width(0.0f), height(0.0f) {}

		/**
		* Copy-construct from another size.
		*
		* @param copy The copy size.
		*/
		SizeF(const SizeF& copy)
			: width(copy.width), height(copy.height)
		{
		}

		/**
		* Constructs a size initialized to the specified values.
		*
		* @param width_ The width value.
		* @param height_ The height value.
		*/
		SizeF(float width_, float height_)
			: width(width_), height(height_)
		{
		}

		/**
		* Constructs a size initialized from given Vector2.
		*
		* @param vector The Vector2 value.
		*/
		SizeF(const Vector2& vector)
			: width(vector.x), height(vector.y)
		{
		}

		// Comparison operators
		inline bool operator==(const SizeF& size) const
		{
			return width == size.width && height == size.height;
		}

		inline bool operator!=(const SizeF& size) const
		{
			return width != size.width || height != size.height;
		}

		// Assignment operators
		SizeF& operator = (const SizeF& rhs) { width = rhs.width; height = rhs.height; return *this; }

		SizeF operator +(const SizeF& rhs) const { return SizeF(width + rhs.width, height + rhs.height); }
		SizeF operator -() const { return SizeF(-width, -height); }
		SizeF operator -(const SizeF& rhs) const { return SizeF(width - rhs.width, height - rhs.height); }
		SizeF operator *(float rhs) const { return SizeF(width * rhs, height * rhs); }
		SizeF operator *(const SizeF& rhs) const { return SizeF(width * rhs.width, height * rhs.height); }
		SizeF operator /(float rhs) const { return SizeF(width / rhs, height / rhs); }
		SizeF operator /(const SizeF& rhs) const { return SizeF(width / rhs.width, height / rhs.height); }

		SizeF& operator +=(const SizeF& rhs)
		{
			width += rhs.width;
			height += rhs.height;
			return *this;
		}

		SizeF& operator -=(const SizeF& rhs)
		{
			width -= rhs.width;
			height -= rhs.height;
			return *this;
		}

		SizeF& operator *=(float rhs)
		{
			width *= rhs;
			height *= rhs;
			return *this;
		}

		SizeF& operator *=(const SizeF& rhs)
		{
			width *= rhs.width;
			height *= rhs.height;
			return *this;
		}

		SizeF& operator /=(float rhs)
		{
			width /= rhs;
			height /= rhs;
			return *this;
		}

		SizeF& operator /=(const SizeF& rhs)
		{
			width /= rhs.width;
			height /= rhs.height;
			return *this;
		}

		bool IsEmpty() const { return width == 0.0f && height == 0.0f; }

		/// Return as string.
		std::string ToString() const;

		/// Empty size.
		static const SizeF Empty;

		/// Size with values (1.0f, 1.0f).
		static const SizeF One;
	};
}
