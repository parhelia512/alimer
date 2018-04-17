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

#include "../Debug/Profiler.h"
#include "../Graphics/Texture.h"
#include "Batch.h"
#include <algorithm>

namespace Alimer
{
	inline bool CompareBatchState(Batch& lhs, Batch& rhs)
	{
		return lhs.sortKey < rhs.sortKey;
	}

	inline bool CompareBatchDistanceFrontToBack(Batch& lhs, Batch& rhs)
	{
		return lhs.distance < rhs.distance;
	}

	inline bool CompareBatchDistanceBackToFront(Batch& lhs, Batch& rhs)
	{
		return lhs.distance > rhs.distance;
	}

	void BatchQueue::Clear()
	{
		batches.clear();
		additiveBatches.clear();
	}

	void BatchQueue::Sort(std::vector<Matrix3x4>& instanceTransforms)
	{
		switch (sort)
		{
		case SORT_STATE:
			std::sort(batches.begin(), batches.end(), CompareBatchState);
			std::sort(additiveBatches.begin(), additiveBatches.end(), CompareBatchState);
			break;

		case SORT_FRONT_TO_BACK:
			std::sort(batches.begin(), batches.end(), CompareBatchDistanceFrontToBack);
			// After drawing the base batches, the Z buffer has been prepared. Additive batches can be sorted per state now
			std::sort(additiveBatches.begin(), additiveBatches.end(), CompareBatchState);
			break;

		case SORT_BACK_TO_FRONT:
			std::sort(batches.begin(), batches.end(), CompareBatchDistanceBackToFront);
			std::sort(additiveBatches.begin(), additiveBatches.end(), CompareBatchDistanceBackToFront);
			break;

		default:
			break;
		}

		// Build instances where adjacent batches have same state
		BuildInstances(batches, instanceTransforms);
		BuildInstances(additiveBatches, instanceTransforms);
	}

	void BatchQueue::BuildInstances(std::vector<Batch>& batches, std::vector<Matrix3x4>& instanceTransforms)
	{
		Batch* start = nullptr;
		for (auto it = batches.begin(), end = batches.end(); it != end; ++it)
		{
			Batch* current = &*it;
			if (start && current->type == GEOM_STATIC && current->pass == start->pass && current->geometry == start->geometry &&
				current->lights == start->lights)
			{
				if (start->type == GEOM_INSTANCED)
				{
					instanceTransforms.push_back(*current->worldMatrix);
					++start->instanceCount;
				}
				else
				{
					// Begin new instanced batch
					start->type = GEOM_INSTANCED;
					uint32_t instanceStart = static_cast<uint32_t>(instanceTransforms.size());
					instanceTransforms.push_back(*start->worldMatrix);
					instanceTransforms.push_back(*current->worldMatrix);
					start->instanceStart = instanceStart; // Overwrites non-instance world matrix
					start->instanceCount = 2; // Overwrites sort key / distance
				}
			}
			else
				start = (current->type == GEOM_STATIC) ? current : nullptr;
		}
	}

	ShadowMap::ShadowMap()
	{
		// Construct texture but do not define its size yet
		texture = new Texture();
	}

	ShadowMap::~ShadowMap()
	{
	}

	void ShadowMap::Clear()
	{
		allocator.Reset(texture->GetWidth(), texture->GetHeight(), 0, 0, false);
		shadowViews.clear();
		used = false;
	}

	void ShadowView::Clear()
	{
		shadowQueue.Clear();
	}

}
