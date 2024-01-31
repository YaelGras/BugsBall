#pragma once

#define PHYSX_IMPL 1

#if D3D11_IMPL && OPENGL_IMPL
#error Multiple rendering implementation have been set.
#endif