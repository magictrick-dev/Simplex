#pragma once
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <cstddef>
#include <cstdio>

typedef int8_t  bool8_t;
typedef int16_t bool16_t;
typedef int32_t bool32_t;
typedef int64_t bool64_t;
typedef float   real32_t;
typedef double  real64_t;

#ifndef SIMPLEX_PEDANTIC_ASSERTIONS
#   define SIMPLEX_PEDANTIC_ASSERTIONS 1
#endif
#if defined(SIMPLEX_PEDANTIC_ASSERTIONS) && SIMPLEX_PEDANTIC_ASSERTIONS != 0
#   define SIMPLEX_ASSERT(stm) assert((stm))
#   define SIMPLEX_CHECK_PTR(ptr) assert((ptr) != NULL)
#   define SIMPLEX_NO_IMPLEMENTATION(reason) assert(!"" reason)
#   define SIMPLEX_NO_REACH(reason) assert(!"" reason)
#else
#   define SIMPLEX_ASSERT(stm)
#   define SIMPLEX_CHECK_PTR(ptr)
#   define SIMPLEX_NO_IMPLEMENTATION(reason)
#   define SIMPLEX_NO_REACH(reason)
#endif
