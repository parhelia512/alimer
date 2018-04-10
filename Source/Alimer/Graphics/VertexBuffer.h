// For conditions of distribution and use, see copyright notice in License.txt

#pragma once

#include "../AlimerConfig.h"

#ifdef ALIMER_D3D11
    #include "D3D11/D3D11VertexBuffer.h"
#endif
#ifdef ALIMER_OPENGL
    #include "GL/GLVertexBuffer.h"
#endif