// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

/**
 * @file
 * @brief Basic types exposed to users.
 */

#pragma once

#ifndef __AIE_API_TYPES__HPP__
#define __AIE_API_TYPES__HPP__

#include "detail/config.hpp"

#include <type_traits>
#include <cstdint>

using int8  = int8_t;
using int16 = int16_t;
using int32 = int32_t;

using uint8  = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;

#if __AIE_API_REGISTER_ATTR_DEFINED__ == 0

#define __aie_register(...) chess_storage(__VA_ARGS__)

#endif

#if __AIE_ARCH__ == 10

struct  exact_acc48 {};
struct  exact_acc80 {};
struct exact_cacc48 {};
struct exact_cacc80 {};

#elif __AIE_ARCH__ == 20

struct  exact_acc32 {};
struct  exact_acc64 {};

#endif

/**
 * @ingroup group_basic_types_accum
 * Internal tag used to signal that the default accumulator precision is needed. This type is not meant to be directly
 * used by AIE API users.
 */
struct accauto {};

using cint16_t = cint16;
using cint32_t = cint32;

#if AIE_API_MATH_VERSION < 100

struct     acc72 {};
struct    cacc72 {};
struct     acc80 {};
struct    cacc80 {};

#if !__AIE_API_COMPLEX_FP32_EMULATION__
struct caccfloat {};
#endif

#endif

using cfloat_t = cfloat;

#if AIE_API_ML_VERSION <= 100

struct int4_t {};

struct bfloat16 {};

#endif

#if !__AIE_API_CBF16_SUPPORT__
struct cbfloat16 {};
#endif

using int4  = int4_t;
using uint4 = uint4_t;

#if AIE_API_ML_VERSION >= 200

namespace aie {

// TODO: CRVO-2167: bfloat16 constructor is not constexpr
#if AIE_API_NATIVE == 0
constexpr
#endif
static inline bfloat16 abs(bfloat16 a)
{
#if AIE_API_NATIVE == 0
    // Clear upper bit to compute the absolute
    return (bfloat16)(__builtin_bit_cast(int16, a) & 0x7fff);
#else
    return (bfloat16)std::abs((float)a);
#endif
}

#if AIE_API_NATIVE == 0
constexpr
#endif
static inline int4_t abs(const int4_t &a)
{
#if AIE_API_NATIVE == 0
    // Clear upper bit to compute the absolute
    return (int4_t)(((int)a) & 0x7);
#else
    return (int4_t)std::abs((int)a);
#endif
}

#if AIE_API_NATIVE == 0
constexpr
#endif
static inline uint4_t abs(const uint4_t &a)
{
    return a;
}

}

#if __AIE_API_TERM_NEG_COMPLEX_DEFINES__ == 0

#define OP_TERM_NEG_COMPLEX                     0x0A
#define OP_TERM_NEG_COMPLEX_CONJUGATE_X         0xA0
#define OP_TERM_NEG_COMPLEX_CONJUGATE_Y         0x50
#define OP_TERM_NEG_COMPLEX_CONJUGATE_X_Y       0xFA
#define OP_TERM_NEG_COMPLEX_CONJUGATE_BUTTERFLY 0xC6
#define OP_TERM_NEG_COMPLEX_BUTTERFLY           0x9C

#endif

#endif

#endif
