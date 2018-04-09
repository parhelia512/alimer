// For conditions of distribution and use, see copyright notice in License.txt

#pragma once

#include "../AlimerConfig.h"

#ifdef TURSO3D_D3D11
    #include "D3D11/D3D11VertexBuffer.h"
#endif
#ifdef TURSO3D_OPENGL
    #include "GL/GLVertexBuffer.h"
#endif