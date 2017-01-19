#pragma once

#ifdef NDEBUG
#ifdef _WIN32
#include <Windows.h>
#define DxAssert(x, y) if (x != y) { MessageBox(NULL, "A", "B", MB_OK); }
#else
#define DxAssert(x, y) x
#endif // WIN32
#else
#include <assert.h>
#ifdef _WIN32
#include <Windows.h>
#define DxAssert(x, y) if (x != y) { MessageBox(NULL, "A", "B", MB_OK); assert(x == y); }
#else
#define DxAssert(x, y) assert(x == y)
#endif // WIN32
#endif // NDEBUG

