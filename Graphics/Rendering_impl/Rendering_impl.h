#pragma once

#define D3D11_IMPL 1
//#define OPENGL_IMPL 2

#if D3D11_IMPL && OPENGL_IMPL
#error Multiple rendering implementation have been set.
#endif


