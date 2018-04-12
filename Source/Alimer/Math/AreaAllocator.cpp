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

#include "AreaAllocator.h"

namespace Alimer
{

	AreaAllocator::AreaAllocator()
	{
		Reset(0, 0);
	}

	AreaAllocator::AreaAllocator(int width, int height, bool fastMode_)
	{
		Reset(width, height, fastMode_);
	}

	AreaAllocator::AreaAllocator(int width, int height, int maxWidth, int maxHeight, bool fastMode_)
	{
		Reset(width, height, maxWidth, maxHeight, fastMode_);
	}

	void AreaAllocator::Reset(int width, int height, int maxWidth, int maxHeight, bool fastMode_)
	{
		doubleWidth = true;
		size = IntVector2(width, height);
		maxSize = IntVector2(maxWidth, maxHeight);
		fastMode = fastMode_;

		freeAreas.clear();
		IntRect initialArea(0, 0, width, height);
		freeAreas.push_back(initialArea);
	}

	bool AreaAllocator::Allocate(int width, int height, int& x, int& y)
	{
		if (width < 0)
			width = 0;
		if (height < 0)
			height = 0;

		std::vector<IntRect>::iterator best;
		int bestFreeArea;

		for (;;)
		{
			best = freeAreas.end();
			bestFreeArea = M_MAX_INT;
			for (auto i = freeAreas.begin(); i != freeAreas.end(); ++i)
			{
				int freeWidth = i->Width();
				int freeHeight = i->Height();

				if (freeWidth >= width && freeHeight >= height)
				{
					// Calculate rank for free area. Lower is better
					int freeArea = freeWidth * freeHeight;

					if (freeArea < bestFreeArea)
					{
						best = i;
						bestFreeArea = freeArea;
					}
				}
			}

			if (best == freeAreas.end())
			{
				if (doubleWidth && size.x < maxSize.x)
				{
					int oldWidth = size.x;
					size.x <<= 1;
					// If no allocations yet, simply expand the single free area
					IntRect& first = freeAreas.front();
					if (freeAreas.size() == 1 && first.left == 0 && first.top == 0 && first.right == oldWidth && first.bottom == size.y)
					{
						first.right = size.x;
					}
					else
					{
						IntRect newArea(oldWidth, 0, size.x, size.y);
						freeAreas.push_back(newArea);
					}
				}
				else if (!doubleWidth && size.y < maxSize.y)
				{
					int oldHeight = size.y;
					size.y <<= 1;
					IntRect& first = freeAreas.front();
					if (freeAreas.size() == 1 && first.left == 0 && first.top == 0 && first.right == size.x && first.bottom == oldHeight)
					{
						first.bottom = size.y;
					}
					else
					{
						IntRect newArea(0, oldHeight, size.x, size.y);
						freeAreas.push_back(newArea);
					}
				}
				else
					return false;

				doubleWidth = !doubleWidth;
			}
			else
				break;
		}

		IntRect reserved(best->left, best->top, best->left + width, best->top + height);
		x = best->left;
		y = best->top;

		if (fastMode)
		{
			// Reserve the area by splitting up the remaining free area
			best->left = reserved.right;
			if (best->Height() > 2 * height || height >= size.y / 2)
			{
				IntRect splitArea(reserved.left, reserved.bottom, best->right, best->bottom);
				best->bottom = reserved.bottom;
				freeAreas.push_back(splitArea);
			}
		}
		else
		{
			// Remove the reserved area from all free areas
			for (size_t i = 0; i < freeAreas.size();)
			{
				if (SplitRect(freeAreas[i], reserved))
					freeAreas.erase(freeAreas.begin() + i);
				else
					++i;
			}

			Cleanup();
		}

		return true;
	}

	bool AreaAllocator::SplitRect(IntRect original, const IntRect& reserve)
	{
		if (reserve.right > original.left && reserve.left < original.right && reserve.bottom > original.top &&
			reserve.top < original.bottom)
		{
			// Check for splitting from the right
			if (reserve.right < original.right)
			{
				IntRect newRect = original;
				newRect.left = reserve.right;
				freeAreas.push_back(newRect);
			}
			// Check for splitting from the left
			if (reserve.left > original.left)
			{
				IntRect newRect = original;
				newRect.right = reserve.left;
				freeAreas.push_back(newRect);
			}
			// Check for splitting from the bottom
			if (reserve.bottom < original.bottom)
			{
				IntRect newRect = original;
				newRect.top = reserve.bottom;
				freeAreas.push_back(newRect);
			}
			// Check for splitting from the top
			if (reserve.top > original.top)
			{
				IntRect newRect = original;
				newRect.bottom = reserve.top;
				freeAreas.push_back(newRect);
			}

			return true;
		}

		return false;
	}

	void AreaAllocator::Cleanup()
	{
		// Remove rects which are contained within another rect
		for (size_t i = 0; i < freeAreas.size();)
		{
			bool erased = false;
			for (size_t j = i + 1; j < freeAreas.size();)
			{
				if ((freeAreas[i].left >= freeAreas[j].left) &&
					(freeAreas[i].top >= freeAreas[j].top) &&
					(freeAreas[i].right <= freeAreas[j].right) &&
					(freeAreas[i].bottom <= freeAreas[j].bottom))
				{
					freeAreas.erase(freeAreas.begin() + i);
					erased = true;
					break;
				}
				if ((freeAreas[j].left >= freeAreas[i].left) &&
					(freeAreas[j].top >= freeAreas[i].top) &&
					(freeAreas[j].right <= freeAreas[i].right) &&
					(freeAreas[j].bottom <= freeAreas[i].bottom))
				{
					freeAreas.erase(freeAreas.begin() + j);
				}
				else
					++j;
			}
			if (!erased)
				++i;
		}
	}

}
