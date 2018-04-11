// For conditions of distribution and use, see copyright notice in License.txt

#pragma once

#include "../AlimerConfig.h"

namespace Alimer
{

class VectorBase;
class String;

/// Swap two values.
template<class T> inline void Swap(T& first, T& second)
{
    T temp = first;
    first = second;
    second = temp;
}

/// Swap two vectors.
template<> ALIMER_API void Swap<VectorBase>(VectorBase& first, VectorBase& second);

/// Swap two strings.
template<> ALIMER_API void Swap<String>(String& first, String& second);

}
