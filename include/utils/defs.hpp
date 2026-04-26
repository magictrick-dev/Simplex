#pragma once
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <cstddef>
#include <cstdio>

typedef float real32_t;
typedef double real64_t;

#ifndef SIMPLEX_PEDANTIC_ASSERTIONS
#   define SIMPLEX_PEDANTIC_ASSERTIONS 1
#endif
#if defined(SIMPLEX_PEDANTIC_ASSERTIONS) && SIMPLEX_PEDANTIC_ASSERTIONS != 0
#   define SIMPLEX_CHECK_PTR(ptr) assert((ptr) != NULL)
#   define SIMPLEX_NO_IMPLEMENTATION(reason) assert(!"" reason)
#   define SIMPLEX_NO_REACH(reason) assert(!"" reason)
#else
#   defined SIMPLEX_CHECK_PTR(ptr)
#   define SIMPLEX_NO_IMPLEMENTATION(reason)
#   define SIMPLEX_NO_REACH(reason)
#endif