// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_MUL__HPP__
#define __AIE_API_DETAIL_MUL__HPP__

#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <climits>

#include "broadcast.hpp"
#include "filter.hpp"
#include "interleave.hpp"
#include "shuffle.hpp"
#include "complex_traits.hpp"

#include "../accum.hpp"
#include "../expr.hpp"
#include "../vector.hpp"

namespace aie {

template <typename T1, typename T2>
struct is_valid_mul_op
{
    using type1 = operand_base_type_t<T1>;
    using type2 = operand_base_type_t<T2>;

    static constexpr bool value()
    {
        using namespace detail::utils;
        bool result = false;
#if __AIE_ARCH__ == 10
        if constexpr (is_one_of_v<type2, int8, uint8>)
            result = result || is_one_of_v<type1, int8, uint8, int16>;
        if constexpr (std::is_same_v<type2, int16>)
            result = result || is_one_of_v<type1, int8, uint8, int16, int32, cint16, cint32>;
        if constexpr (std::is_same_v<type2, int32>)
            result = result || is_one_of_v<type1, int16, int32, cint16, cint32>;
        if constexpr (is_one_of_v<type2, float, cfloat>)
            result = result || is_one_of_v<type1, float, cfloat>;
        if constexpr (std::is_same_v<type2, cint16>)
            result = result || is_one_of_v<type1, int16, int32, cint16, cint32>;
        if constexpr (std::is_same_v<type2, cint32>)
            result = result || is_one_of_v<type1, int16, int32, cint16, cint32>;
#elif __AIE_ARCH__ == 20 || __AIE_ARCH__ == 21 || __AIE_ARCH__ == 22
        if constexpr (is_one_of_v<type2, int4, uint4>)
            result = result || is_one_of_v<type1, int4, uint4, int8, uint8>;
        if constexpr (is_one_of_v<type2, int8, uint8>)
            result = result || is_one_of_v<type1, int4, uint4, int8, uint8, int16, uint16>;
        if constexpr (is_one_of_v<type2, int16, uint16>)
            result = result || is_one_of_v<type1, int8, uint8, int16, uint16, int32, uint32, cint16, cint32>;
        if constexpr (is_one_of_v<type2, int32, uint32>)
            result = result || is_one_of_v<type1, int16, uint16, int32, uint32, cint16, cint32>;
#if __AIE_API_COMPLEX_FP32_EMULATION__
#if __AIE_API_CBF16_SUPPORT__
        if constexpr (is_one_of_v<type2, bfloat16, cbfloat16>)
            result = result || is_one_of_v<type1, bfloat16, cbfloat16>;
#endif
        if constexpr (is_one_of_v<type2, float, cfloat>)
            result = result || is_one_of_v<type1, float, cfloat>;
#endif
        if constexpr (std::is_same_v<type2, float>)
            result = result || std::is_same_v<type1, float>;
        if constexpr (std::is_same_v<type2, cint16>)
            result = result || is_one_of_v<type1, int16, uint16, int32, uint32, cint16, cint32>;
        if constexpr (std::is_same_v<type2, cint32>)
            result = result || is_one_of_v<type1, int16, uint16, int32, uint32, cint16, cint32>;
#if __AIE_API_BF8_SUPPORT__
        if constexpr (std::is_same_v<type2, bfloat8>)
            result = result || std::is_same_v<type1, bfloat8>;
#endif
#if __AIE_API_FP8_SUPPORT__
        if constexpr (std::is_same_v<type2, float8>)
            result = result || std::is_same_v<type1, float8>;
#endif
#if __AIE_API_FP16_SUPPORT__
#if __AIE_API_MIXED_BF16_FP16_SUPPORT__
        if constexpr (is_one_of_v<type2, bfloat16, float16>)
            result = result || is_one_of_v<type1, bfloat16, float16>;
#endif
        if constexpr (std::is_same_v<type2, float16>)
            result = result || std::is_same_v<type1, float16>;
#endif
        if constexpr (std::is_same_v<type2, bfloat16>)
            result = result || std::is_same_v<type1, bfloat16>;
#endif
        return result;
    }
};

template <typename Parent, Operation Op, typename T2>
struct is_valid_mul_op<unary_op<Parent, Op>, T2>
{
    static constexpr bool value()
    {
        return is_valid_mul_op<typename unary_op<Parent, Op>::value_type, T2>::value();
    }
};

template <typename T, typename Parent, Operation Op>
struct is_valid_mul_op<T, unary_op<Parent, Op>>
{
    static constexpr bool value()
    {
        return is_valid_mul_op<T, typename unary_op<Parent, Op>::value_type>::value();
    }
};

template <typename Parent1, typename Parent2, Operation Op, typename T2>
struct is_valid_mul_op<binary_op<Parent1, Parent2, Op>, T2>
{
    static constexpr bool value()
    {
        return is_valid_mul_op<typename binary_op<Parent1, Parent2, Op>::value_type, T2>::value();
    }
};

template <typename T, typename Parent1, typename Parent2, Operation Op>
struct is_valid_mul_op<T, binary_op<Parent1, Parent2, Op>>
{
    static constexpr bool value()
    {
        return is_valid_mul_op<T, typename binary_op<Parent1, Parent2, Op>::value_type>::value();
    }
};


} // namespace aie

namespace aie::detail {

enum class MulMacroOp
{
    Unavailable = -1,
    Mul = 0,
    NegMul,

    Add_Mul,
    Add_NegMul,
    Sub_Mul    = Add_NegMul ,   // Synonym of Add_NegMul
#if __AIE_ARCH__ == 10
    MulSym,
    NegMulSym,
    MulAntisym,
    NegMulAntisym,

    Add_MulSym,
    Add_NegMulSym,
    Sub_MulSym = Add_NegMulSym,
    Add_MulAntisym,
    Add_NegMulAntisym,
    Sub_MulAntisym = Add_NegMulAntisym,

    MulAbs1,
    MulAbs1Conj2,
    MulConj1,
    MulConj1Conj2,
    MulConj2,
    NegMulAbs1,
    NegMulAbs1Conj2,
    NegMulConj1,
    NegMulConj1Conj2,
    NegMulConj2,

    MulSymConj1,
    MulSymConj1Conj2,
    MulSymConj2,
    NegMulSymConj1,
    NegMulSymConj1Conj2,
    NegMulSymConj2,

    MulAntisymConj1,
    MulAntisymConj1Conj2,
    MulAntisymConj2,
    NegMulAntisymConj1,
    NegMulAntisymConj1Conj2,
    NegMulAntisymConj2,

    MulMax,
    MulMin,

    Add_MulAbs1,
    Add_MulAbs1Conj2,
    Add_MulConj1,
    Add_MulConj1Conj2,
    Add_MulConj2,
    Sub_MulAbs1,
    Sub_MulAbs1Conj2,
    Sub_MulConj1,
    Sub_MulConj1Conj2,
    Sub_MulConj2,

    Add_MulSymConj1,
    Add_MulSymConj1Conj2,
    Add_MulSymConj2,
    Sub_MulSymConj1,
    Sub_MulSymConj1Conj2,
    Sub_MulSymConj2,

    Add_MulAntisymConj1,
    Add_MulAntisymConj1Conj2,
    Add_MulAntisymConj2,
    Sub_MulAntisymConj1,
    Sub_MulAntisymConj1Conj2,
    Sub_MulAntisymConj2,

    Add_MulMax,
    Add_MulMin,
    Sub_MulMax,
    Sub_MulMin,
#elif __AIE_ARCH__ == 20 || __AIE_ARCH__ == 21 || __AIE_ARCH__ == 22
    MulConj1,
    MulConj1Conj2,
    MulConj2,
    NegMulConj1,
    NegMulConj1Conj2,
    NegMulConj2,

    Add_MulConj1,
    Add_MulConj1Conj2,
    Add_MulConj2,
    Sub_MulConj1,
    Sub_MulConj1Conj2,
    Sub_MulConj2,
#endif
};

template <MulMacroOp MulOp>
static constexpr MulMacroOp swap_conjugate_order()
{
#if __AIE_ARCH__ == 10
    if      constexpr (MulOp == MulMacroOp::MulConj1)            return MulMacroOp::MulConj2;
    else if constexpr (MulOp == MulMacroOp::MulConj2)            return MulMacroOp::MulConj1;
    else if constexpr (MulOp == MulMacroOp::NegMulConj1)         return MulMacroOp::NegMulConj2;
    else if constexpr (MulOp == MulMacroOp::NegMulConj2)         return MulMacroOp::NegMulConj1;

    else if constexpr (MulOp == MulMacroOp::MulSymConj1)         return MulMacroOp::MulSymConj2;
    else if constexpr (MulOp == MulMacroOp::MulSymConj2)         return MulMacroOp::MulSymConj1;
    else if constexpr (MulOp == MulMacroOp::NegMulSymConj1)      return MulMacroOp::NegMulSymConj2;
    else if constexpr (MulOp == MulMacroOp::NegMulSymConj2)      return MulMacroOp::NegMulSymConj1;

    else if constexpr (MulOp == MulMacroOp::MulAntisymConj1)     return MulMacroOp::MulAntisymConj2;
    else if constexpr (MulOp == MulMacroOp::MulAntisymConj2)     return MulMacroOp::MulAntisymConj1;
    else if constexpr (MulOp == MulMacroOp::NegMulAntisymConj1)  return MulMacroOp::NegMulAntisymConj2;
    else if constexpr (MulOp == MulMacroOp::NegMulAntisymConj2)  return MulMacroOp::NegMulAntisymConj1;

    else if constexpr (MulOp == MulMacroOp::Add_MulConj1)        return MulMacroOp::Add_MulConj2;
    else if constexpr (MulOp == MulMacroOp::Add_MulConj2)        return MulMacroOp::Add_MulConj1;
    else if constexpr (MulOp == MulMacroOp::Sub_MulConj1)        return MulMacroOp::Sub_MulConj2;
    else if constexpr (MulOp == MulMacroOp::Sub_MulConj2)        return MulMacroOp::Sub_MulConj1;

    else if constexpr (MulOp == MulMacroOp::Add_MulSymConj1)     return MulMacroOp::Add_MulSymConj2;
    else if constexpr (MulOp == MulMacroOp::Add_MulSymConj2)     return MulMacroOp::Add_MulSymConj1;
    else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj1)     return MulMacroOp::Sub_MulSymConj2;
    else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj2)     return MulMacroOp::Sub_MulSymConj1;

    else if constexpr (MulOp == MulMacroOp::Add_MulAntisymConj1) return MulMacroOp::Add_MulAntisymConj2;
    else if constexpr (MulOp == MulMacroOp::Add_MulAntisymConj2) return MulMacroOp::Add_MulAntisymConj1;
    else if constexpr (MulOp == MulMacroOp::Sub_MulAntisymConj1) return MulMacroOp::Sub_MulAntisymConj2;
    else if constexpr (MulOp == MulMacroOp::Sub_MulAntisymConj2) return MulMacroOp::Sub_MulAntisymConj1;

#elif __AIE_ARCH__ == 20 || __AIE_ARCH__ == 21 || __AIE_ARCH__ == 22
    if      constexpr (MulOp == MulMacroOp::MulConj1)            return MulMacroOp::MulConj2;
    else if constexpr (MulOp == MulMacroOp::MulConj2)            return MulMacroOp::MulConj1;
    else if constexpr (MulOp == MulMacroOp::NegMulConj1)         return MulMacroOp::NegMulConj2;
    else if constexpr (MulOp == MulMacroOp::NegMulConj2)         return MulMacroOp::NegMulConj1;

    else if constexpr (MulOp == MulMacroOp::Add_MulConj1)        return MulMacroOp::Add_MulConj2;
    else if constexpr (MulOp == MulMacroOp::Add_MulConj2)        return MulMacroOp::Add_MulConj1;
    else if constexpr (MulOp == MulMacroOp::Sub_MulConj1)        return MulMacroOp::Sub_MulConj2;
    else if constexpr (MulOp == MulMacroOp::Sub_MulConj2)        return MulMacroOp::Sub_MulConj1;
#endif
    else                                                         return MulOp;
};

template <MulMacroOp MulOp>
static constexpr MulMacroOp remove_conj1()
{
#if __AIE_ARCH__ == 10
    if      constexpr (MulOp == MulMacroOp::MulConj1)                 return MulMacroOp::Mul;
    else if constexpr (MulOp == MulMacroOp::MulConj1Conj2)            return MulMacroOp::MulConj2;
    else if constexpr (MulOp == MulMacroOp::NegMulConj1)              return MulMacroOp::NegMul;
    else if constexpr (MulOp == MulMacroOp::NegMulConj1Conj2)         return MulMacroOp::NegMulConj2;

    else if constexpr (MulOp == MulMacroOp::MulSymConj1)              return MulMacroOp::MulSym;
    else if constexpr (MulOp == MulMacroOp::MulSymConj1Conj2)         return MulMacroOp::MulSymConj2;
    else if constexpr (MulOp == MulMacroOp::NegMulSymConj1)           return MulMacroOp::NegMulSym;
    else if constexpr (MulOp == MulMacroOp::NegMulSymConj1Conj2)      return MulMacroOp::NegMulSymConj2;

    else if constexpr (MulOp == MulMacroOp::MulAntisymConj1)          return MulMacroOp::MulAntisym;
    else if constexpr (MulOp == MulMacroOp::MulAntisymConj1Conj2)     return MulMacroOp::MulAntisymConj2;
    else if constexpr (MulOp == MulMacroOp::NegMulAntisymConj1)       return MulMacroOp::NegMulAntisym;
    else if constexpr (MulOp == MulMacroOp::NegMulAntisymConj1Conj2)  return MulMacroOp::NegMulAntisymConj2;

    else if constexpr (MulOp == MulMacroOp::Add_MulConj1)             return MulMacroOp::Add_Mul;
    else if constexpr (MulOp == MulMacroOp::Add_MulConj1Conj2)        return MulMacroOp::Add_MulConj2;
    else if constexpr (MulOp == MulMacroOp::Sub_MulConj1)             return MulMacroOp::Sub_Mul;
    else if constexpr (MulOp == MulMacroOp::Sub_MulConj1Conj2)        return MulMacroOp::Sub_MulConj2;

    else if constexpr (MulOp == MulMacroOp::Add_MulSymConj1)          return MulMacroOp::Add_MulSym;
    else if constexpr (MulOp == MulMacroOp::Add_MulSymConj1Conj2)     return MulMacroOp::Add_MulSymConj2;
    else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj1)          return MulMacroOp::Sub_MulSym;
    else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj1Conj2)     return MulMacroOp::Sub_MulSymConj2;

    else if constexpr (MulOp == MulMacroOp::Add_MulAntisymConj1)      return MulMacroOp::Add_MulAntisym;
    else if constexpr (MulOp == MulMacroOp::Add_MulAntisymConj1Conj2) return MulMacroOp::Add_MulAntisymConj2;
    else if constexpr (MulOp == MulMacroOp::Sub_MulAntisymConj1)      return MulMacroOp::Sub_MulAntisym;
    else if constexpr (MulOp == MulMacroOp::Sub_MulAntisymConj1Conj2) return MulMacroOp::Sub_MulAntisymConj2;

#elif __AIE_ARCH__ == 20 || __AIE_ARCH__ == 21 || __AIE_ARCH__ == 22
    if      constexpr (MulOp == MulMacroOp::MulConj1)                 return MulMacroOp::Mul;
    else if constexpr (MulOp == MulMacroOp::MulConj1Conj2)            return MulMacroOp::MulConj2;
    else if constexpr (MulOp == MulMacroOp::NegMulConj1)              return MulMacroOp::NegMul;
    else if constexpr (MulOp == MulMacroOp::NegMulConj1Conj2)         return MulMacroOp::NegMulConj2;

    else if constexpr (MulOp == MulMacroOp::Add_MulConj1)             return MulMacroOp::Add_Mul;
    else if constexpr (MulOp == MulMacroOp::Add_MulConj1Conj2)        return MulMacroOp::Add_MulConj2;
    else if constexpr (MulOp == MulMacroOp::Sub_MulConj1)             return MulMacroOp::Sub_Mul;
    else if constexpr (MulOp == MulMacroOp::Sub_MulConj1Conj2)        return MulMacroOp::Sub_MulConj2;
#endif
    else                                                              return MulOp;
};

template <MulMacroOp MulOp>
static constexpr MulMacroOp remove_conj2()
{
#if __AIE_ARCH__ == 10
    if      constexpr (MulOp == MulMacroOp::MulAbs1Conj2)             return MulMacroOp::MulAbs1;
    else if constexpr (MulOp == MulMacroOp::MulConj1Conj2)            return MulMacroOp::MulConj1;
    else if constexpr (MulOp == MulMacroOp::MulConj2)                 return MulMacroOp::Mul;
    else if constexpr (MulOp == MulMacroOp::NegMulAbs1Conj2)          return MulMacroOp::NegMulAbs1;
    else if constexpr (MulOp == MulMacroOp::NegMulConj1Conj2)         return MulMacroOp::NegMulConj1;
    else if constexpr (MulOp == MulMacroOp::NegMulConj2)              return MulMacroOp::NegMul;

    else if constexpr (MulOp == MulMacroOp::MulSymConj1Conj2)         return MulMacroOp::MulSymConj1;
    else if constexpr (MulOp == MulMacroOp::MulSymConj2)              return MulMacroOp::MulSym;
    else if constexpr (MulOp == MulMacroOp::NegMulSymConj1Conj2)      return MulMacroOp::NegMulSymConj1;
    else if constexpr (MulOp == MulMacroOp::NegMulSymConj2)           return MulMacroOp::NegMulSym;

    else if constexpr (MulOp == MulMacroOp::MulAntisymConj1Conj2)     return MulMacroOp::MulAntisymConj1;
    else if constexpr (MulOp == MulMacroOp::MulAntisymConj2)          return MulMacroOp::MulAntisym;
    else if constexpr (MulOp == MulMacroOp::NegMulAntisymConj1Conj2)  return MulMacroOp::NegMulAntisymConj1;
    else if constexpr (MulOp == MulMacroOp::NegMulAntisymConj2)       return MulMacroOp::NegMulAntisym;

    else if constexpr (MulOp == MulMacroOp::Add_MulAbs1Conj2)         return MulMacroOp::Add_MulAbs1;
    else if constexpr (MulOp == MulMacroOp::Add_MulConj1Conj2)        return MulMacroOp::Add_MulConj1;
    else if constexpr (MulOp == MulMacroOp::Add_MulConj2)             return MulMacroOp::Add_Mul;
    else if constexpr (MulOp == MulMacroOp::Sub_MulAbs1Conj2)         return MulMacroOp::Sub_MulAbs1;
    else if constexpr (MulOp == MulMacroOp::Sub_MulConj1Conj2)        return MulMacroOp::Sub_MulConj1;
    else if constexpr (MulOp == MulMacroOp::Sub_MulConj2)             return MulMacroOp::Sub_Mul;

    else if constexpr (MulOp == MulMacroOp::Add_MulSymConj1Conj2)     return MulMacroOp::Add_MulSymConj1;
    else if constexpr (MulOp == MulMacroOp::Add_MulSymConj2)          return MulMacroOp::Add_MulSym;
    else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj1Conj2)     return MulMacroOp::Sub_MulSymConj1;
    else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj2)          return MulMacroOp::Sub_MulSym;

    else if constexpr (MulOp == MulMacroOp::Add_MulAntisymConj1Conj2) return MulMacroOp::Add_MulAntisymConj1;
    else if constexpr (MulOp == MulMacroOp::Add_MulAntisymConj2)      return MulMacroOp::Add_MulAntisym;
    else if constexpr (MulOp == MulMacroOp::Sub_MulAntisymConj1Conj2) return MulMacroOp::Sub_MulAntisymConj1;
    else if constexpr (MulOp == MulMacroOp::Sub_MulAntisymConj2)      return MulMacroOp::Sub_MulAntisym;

#elif __AIE_ARCH__ == 20 || __AIE_ARCH__ == 21 || __AIE_ARCH__ == 22
    if      constexpr (MulOp == MulMacroOp::MulConj1Conj2)            return MulMacroOp::MulConj1;
    else if constexpr (MulOp == MulMacroOp::MulConj2)                 return MulMacroOp::Mul;
    else if constexpr (MulOp == MulMacroOp::NegMulConj1Conj2)         return MulMacroOp::NegMulConj1;
    else if constexpr (MulOp == MulMacroOp::NegMulConj2)              return MulMacroOp::NegMul;

    else if constexpr (MulOp == MulMacroOp::Add_MulConj1Conj2)        return MulMacroOp::Add_MulConj1;
    else if constexpr (MulOp == MulMacroOp::Add_MulConj2)             return MulMacroOp::Add_Mul;
    else if constexpr (MulOp == MulMacroOp::Sub_MulConj1Conj2)        return MulMacroOp::Sub_MulConj1;
    else if constexpr (MulOp == MulMacroOp::Sub_MulConj2)             return MulMacroOp::Sub_Mul;
#endif
    else                                                              return MulOp;
};

template <MulMacroOp Op>
constexpr bool has_abs()
{
#if __AIE_ARCH__ == 10
    return (Op == MulMacroOp::MulAbs1          ||
            Op == MulMacroOp::MulAbs1Conj2     ||
            Op == MulMacroOp::NegMulAbs1       ||
            Op == MulMacroOp::NegMulAbs1Conj2  ||
            Op == MulMacroOp::Add_MulAbs1      ||
            Op == MulMacroOp::Add_MulAbs1Conj2 ||
            Op == MulMacroOp::Sub_MulAbs1      ||
            Op == MulMacroOp::Sub_MulAbs1Conj2);
#elif __AIE_ARCH__ == 20 || __AIE_ARCH__ == 21 || __AIE_ARCH__ == 22
    return false;
#endif
}

template <MulMacroOp Op>
constexpr bool has_conj1()
{
#if __AIE_ARCH__ == 10
    return (Op == MulMacroOp::MulConj1                 ||
            Op == MulMacroOp::MulConj1Conj2            ||
            Op == MulMacroOp::NegMulConj1              ||
            Op == MulMacroOp::NegMulConj1Conj2         ||
            Op == MulMacroOp::MulSymConj1              ||
            Op == MulMacroOp::MulSymConj1Conj2         ||
            Op == MulMacroOp::NegMulSymConj1           ||
            Op == MulMacroOp::NegMulSymConj1Conj2      ||
            Op == MulMacroOp::MulAntisymConj1          ||
            Op == MulMacroOp::MulAntisymConj1Conj2     ||
            Op == MulMacroOp::NegMulAntisymConj1       ||
            Op == MulMacroOp::NegMulAntisymConj1Conj2  ||
            Op == MulMacroOp::Add_MulConj1             ||
            Op == MulMacroOp::Add_MulConj1Conj2        ||
            Op == MulMacroOp::Add_MulSymConj1          ||
            Op == MulMacroOp::Add_MulSymConj1Conj2     ||
            Op == MulMacroOp::Add_MulAntisymConj1      ||
            Op == MulMacroOp::Add_MulAntisymConj1Conj2 ||
            Op == MulMacroOp::Sub_MulConj1             ||
            Op == MulMacroOp::Sub_MulConj1Conj2)       ||
            Op == MulMacroOp::Sub_MulSymConj1          ||
            Op == MulMacroOp::Sub_MulSymConj1Conj2     ||
            Op == MulMacroOp::Sub_MulAntisymConj1      ||
            Op == MulMacroOp::Sub_MulAntisymConj1Conj2;
#elif __AIE_ARCH__ == 20 || __AIE_ARCH__ == 21 || __AIE_ARCH__ == 22
    return (Op == MulMacroOp::MulConj1          ||
            Op == MulMacroOp::MulConj1Conj2     ||
            Op == MulMacroOp::NegMulConj1       ||
            Op == MulMacroOp::NegMulConj1Conj2  ||
            Op == MulMacroOp::Add_MulConj1      ||
            Op == MulMacroOp::Add_MulConj1Conj2 ||
            Op == MulMacroOp::Sub_MulConj1      ||
            Op == MulMacroOp::Sub_MulConj1Conj2);
#endif
}

template <MulMacroOp Op>
constexpr bool has_conj2()
{
#if __AIE_ARCH__ == 10
    return (Op == MulMacroOp::MulConj2                 ||
            Op == MulMacroOp::MulConj1Conj2            ||
            Op == MulMacroOp::NegMulConj2              ||
            Op == MulMacroOp::NegMulConj1Conj2         ||
            Op == MulMacroOp::MulSymConj2              ||
            Op == MulMacroOp::MulSymConj1Conj2         ||
            Op == MulMacroOp::NegMulSymConj2           ||
            Op == MulMacroOp::NegMulSymConj1Conj2      ||
            Op == MulMacroOp::MulAntisymConj2          ||
            Op == MulMacroOp::MulAntisymConj1Conj2     ||
            Op == MulMacroOp::NegMulAntisymConj2       ||
            Op == MulMacroOp::NegMulAntisymConj1Conj2  ||
            Op == MulMacroOp::Add_MulConj2             ||
            Op == MulMacroOp::Add_MulConj1Conj2        ||
            Op == MulMacroOp::Add_MulSymConj2          ||
            Op == MulMacroOp::Add_MulSymConj1Conj2     ||
            Op == MulMacroOp::Add_MulAntisymConj2      ||
            Op == MulMacroOp::Add_MulAntisymConj1Conj2 ||
            Op == MulMacroOp::Sub_MulConj2             ||
            Op == MulMacroOp::Sub_MulConj1Conj2)       ||
            Op == MulMacroOp::Sub_MulSymConj2          ||
            Op == MulMacroOp::Sub_MulSymConj1Conj2     ||
            Op == MulMacroOp::Sub_MulAntisymConj2      ||
            Op == MulMacroOp::Sub_MulAntisymConj1Conj2;
#elif __AIE_ARCH__ == 20 || __AIE_ARCH__ == 21 || __AIE_ARCH__ == 22
    return (Op == MulMacroOp::MulConj2          ||
            Op == MulMacroOp::MulConj1Conj2     ||
            Op == MulMacroOp::NegMulConj2       ||
            Op == MulMacroOp::NegMulConj1Conj2  ||
            Op == MulMacroOp::Add_MulConj2      ||
            Op == MulMacroOp::Add_MulConj1Conj2 ||
            Op == MulMacroOp::Sub_MulConj2      ||
            Op == MulMacroOp::Sub_MulConj1Conj2);
#endif
}

template <Operation Op1, Operation Op2>
constexpr MulMacroOp to_mul_macro_op()
{
#if __AIE_ARCH__ == 10
    if      constexpr (Op1 == Operation::Abs  && Op2 == Operation::None) return MulMacroOp::MulAbs1;
    else if constexpr (Op1 == Operation::Abs  && Op2 == Operation::Conj) return MulMacroOp::MulAbs1Conj2;
    else if constexpr (Op1 == Operation::Conj && Op2 == Operation::None) return MulMacroOp::MulConj1;
    else if constexpr (Op1 == Operation::Conj && Op2 == Operation::Conj) return MulMacroOp::MulConj1Conj2;
    else if constexpr (Op1 == Operation::Min  && Op2 == Operation::None) return MulMacroOp::MulMin;
    else if constexpr (Op1 == Operation::Max  && Op2 == Operation::None) return MulMacroOp::MulMax;
    else if constexpr (Op1 == Operation::None && Op2 == Operation::Conj) return MulMacroOp::MulConj2;
    else if constexpr (Op1 == Operation::None && Op2 == Operation::None) return MulMacroOp::Mul;
#elif __AIE_ARCH__ == 20 || __AIE_ARCH__ == 21 || __AIE_ARCH__ == 22
    static_assert(Op1 == Operation::None || Op1 == Operation::Conj);
    static_assert(Op2 == Operation::None || Op2 == Operation::Conj);

    if      constexpr (Op1 == Operation::Conj && Op2 == Operation::None) return MulMacroOp::MulConj1;
    else if constexpr (Op1 == Operation::Conj && Op2 == Operation::Conj) return MulMacroOp::MulConj1Conj2;
    else if constexpr (Op1 == Operation::None && Op2 == Operation::Conj) return MulMacroOp::MulConj2;
    else if constexpr (Op1 == Operation::None && Op2 == Operation::None) return MulMacroOp::Mul;
#endif
}

template <Operation AccOp, Operation Op1, Operation Op2>
constexpr MulMacroOp to_mul_macro_op()
{
#if __AIE_ARCH__ == 10
    if      constexpr (Op1 == Operation::Abs  && Op2 == Operation::None) return (AccOp == Operation::Acc_Add)? MulMacroOp::Add_MulAbs1       : MulMacroOp::Sub_MulAbs1;
    else if constexpr (Op1 == Operation::Abs  && Op2 == Operation::Conj) return (AccOp == Operation::Acc_Add)? MulMacroOp::Add_MulAbs1Conj2  : MulMacroOp::Sub_MulAbs1Conj2;
    else if constexpr (Op1 == Operation::Conj && Op2 == Operation::None) return (AccOp == Operation::Acc_Add)? MulMacroOp::Add_MulConj1      : MulMacroOp::Sub_MulConj1;
    else if constexpr (Op1 == Operation::Conj && Op2 == Operation::Conj) return (AccOp == Operation::Acc_Add)? MulMacroOp::Add_MulConj1Conj2 : MulMacroOp::Sub_MulConj1Conj2;
    else if constexpr (Op1 == Operation::Min  && Op2 == Operation::None) return (AccOp == Operation::Acc_Add)? MulMacroOp::Add_MulMin        : MulMacroOp::Sub_MulMin;
    else if constexpr (Op1 == Operation::Max  && Op2 == Operation::None) return (AccOp == Operation::Acc_Add)? MulMacroOp::Add_MulMax        : MulMacroOp::Sub_MulMax;
    else if constexpr (Op1 == Operation::None && Op2 == Operation::Conj) return (AccOp == Operation::Acc_Add)? MulMacroOp::Add_MulConj2      : MulMacroOp::Sub_MulConj2;
    else if constexpr (Op1 == Operation::None && Op2 == Operation::None) return (AccOp == Operation::Acc_Add)? MulMacroOp::Add_Mul           : MulMacroOp::Sub_Mul;
#elif __AIE_ARCH__ == 20 || __AIE_ARCH__ == 21 || __AIE_ARCH__ == 22
    static_assert(Op1 == Operation::None || Op1 == Operation::Conj);
    static_assert(Op2 == Operation::None || Op2 == Operation::Conj);

    if      constexpr (Op1 == Operation::Conj && Op2 == Operation::None) return (AccOp == Operation::Acc_Add)? MulMacroOp::Add_MulConj1      : MulMacroOp::Sub_MulConj1;
    else if constexpr (Op1 == Operation::Conj && Op2 == Operation::Conj) return (AccOp == Operation::Acc_Add)? MulMacroOp::Add_MulConj1Conj2 : MulMacroOp::Sub_MulConj1Conj2;
    else if constexpr (Op1 == Operation::None && Op2 == Operation::Conj) return (AccOp == Operation::Acc_Add)? MulMacroOp::Add_MulConj2      : MulMacroOp::Sub_MulConj2;
    else if constexpr (Op1 == Operation::None && Op2 == Operation::None) return (AccOp == Operation::Acc_Add)? MulMacroOp::Add_Mul           : MulMacroOp::Sub_Mul;
#endif
}

template <Operation Op1, Operation Op2>
constexpr MulMacroOp to_mul_sym_macro_op()
{
#if __AIE_ARCH__ == 10
    if      constexpr (Op1 == Operation::Conj && Op2 == Operation::None) return MulMacroOp::MulSymConj1;
    else if constexpr (Op1 == Operation::Conj && Op2 == Operation::Conj) return MulMacroOp::MulSymConj1Conj2;
    else if constexpr (Op1 == Operation::None && Op2 == Operation::Conj) return MulMacroOp::MulSymConj2;
    else if constexpr (Op1 == Operation::None && Op2 == Operation::None) return MulMacroOp::MulSym;
#else
    UNREACHABLE_MSG("Not supported in the current architecture");
    return MulMacroOp::Unavailable;
#endif
}

template <Operation AccOp, Operation Op1, Operation Op2>
constexpr MulMacroOp to_mul_sym_macro_op()
{
#if __AIE_ARCH__ == 10
    if      constexpr (Op1 == Operation::Conj && Op2 == Operation::None) return (AccOp == Operation::Acc_Add)? MulMacroOp::Add_MulSymConj1      : MulMacroOp::Sub_MulSymConj1;
    else if constexpr (Op1 == Operation::Conj && Op2 == Operation::Conj) return (AccOp == Operation::Acc_Add)? MulMacroOp::Add_MulSymConj1Conj2 : MulMacroOp::Sub_MulSymConj1Conj2;
    else if constexpr (Op1 == Operation::None && Op2 == Operation::Conj) return (AccOp == Operation::Acc_Add)? MulMacroOp::Add_MulSymConj2      : MulMacroOp::Sub_MulSymConj2;
    else if constexpr (Op1 == Operation::None && Op2 == Operation::None) return (AccOp == Operation::Acc_Add)? MulMacroOp::Add_MulSym           : MulMacroOp::Sub_MulSym;
#else
    UNREACHABLE_MSG("Not supported in the current architecture");
    return MulMacroOp::Unavailable;
#endif
}

template <Operation Op1, Operation Op2>
constexpr MulMacroOp to_mul_antisym_macro_op()
{
#if __AIE_ARCH__ == 10
    if      constexpr (Op1 == Operation::Conj && Op2 == Operation::None) return MulMacroOp::MulAntisymConj1;
    else if constexpr (Op1 == Operation::Conj && Op2 == Operation::Conj) return MulMacroOp::MulAntisymConj1Conj2;
    else if constexpr (Op1 == Operation::None && Op2 == Operation::Conj) return MulMacroOp::MulAntisymConj2;
    else if constexpr (Op1 == Operation::None && Op2 == Operation::None) return MulMacroOp::MulAntisym;
#else
    UNREACHABLE_MSG("Not supported in the current architecture");
    return MulMacroOp::Unavailable;
#endif
}

template <Operation AccOp, Operation Op1, Operation Op2>
constexpr MulMacroOp to_mul_antisym_macro_op()
{
#if __AIE_ARCH__ == 10
    if      constexpr (Op1 == Operation::Conj && Op2 == Operation::None) return (AccOp == Operation::Acc_Add)? MulMacroOp::Add_MulAntisymConj1      : MulMacroOp::Sub_MulAntisymConj1;
    else if constexpr (Op1 == Operation::Conj && Op2 == Operation::Conj) return (AccOp == Operation::Acc_Add)? MulMacroOp::Add_MulAntisymConj1Conj2 : MulMacroOp::Sub_MulAntisymConj1Conj2;
    else if constexpr (Op1 == Operation::None && Op2 == Operation::Conj) return (AccOp == Operation::Acc_Add)? MulMacroOp::Add_MulAntisymConj2      : MulMacroOp::Sub_MulAntisymConj2;
    else if constexpr (Op1 == Operation::None && Op2 == Operation::None) return (AccOp == Operation::Acc_Add)? MulMacroOp::Add_MulAntisym           : MulMacroOp::Sub_MulAntisym;
#else
    UNREACHABLE_MSG("Not supported in the current architecture");
    return MulMacroOp::Unavailable;
#endif
}

template <Operation Op1, Operation Op2>
constexpr MulMacroOp to_negmul_macro_op()
{
#if __AIE_ARCH__ == 10
    if      constexpr (Op1 == Operation::Abs  && Op2 == Operation::None) return MulMacroOp::NegMulAbs1;
    else if constexpr (Op1 == Operation::Abs  && Op2 == Operation::Conj) return MulMacroOp::NegMulAbs1Conj2;
    else if constexpr (Op1 == Operation::Conj && Op2 == Operation::None) return MulMacroOp::NegMulConj1;
    else if constexpr (Op1 == Operation::Conj && Op2 == Operation::Conj) return MulMacroOp::NegMulConj1Conj2;
    else if constexpr (Op1 == Operation::None && Op2 == Operation::Conj) return MulMacroOp::NegMulConj2;
    else if constexpr (Op1 == Operation::None && Op2 == Operation::None) return MulMacroOp::NegMul;
#elif __AIE_ARCH__ == 20 || __AIE_ARCH__ == 21 || __AIE_ARCH__ == 22
    static_assert(Op1 == Operation::None || Op1 == Operation::Conj);
    static_assert(Op2 == Operation::None || Op2 == Operation::Conj);

    if      constexpr (Op1 == Operation::Conj && Op2 == Operation::None) return MulMacroOp::NegMulConj1;
    else if constexpr (Op1 == Operation::Conj && Op2 == Operation::Conj) return MulMacroOp::NegMulConj1Conj2;
    else if constexpr (Op1 == Operation::None && Op2 == Operation::Conj) return MulMacroOp::NegMulConj2;
    else if constexpr (Op1 == Operation::None && Op2 == Operation::None) return MulMacroOp::NegMul;
#endif
}

template <Operation Op1, Operation Op2>
constexpr MulMacroOp to_negmul_sym_macro_op()
{
#if __AIE_ARCH__ == 10
    if      constexpr (Op1 == Operation::Conj && Op2 == Operation::None) return MulMacroOp::NegMulSymConj1;
    else if constexpr (Op1 == Operation::Conj && Op2 == Operation::Conj) return MulMacroOp::NegMulSymConj1Conj2;
    else if constexpr (Op1 == Operation::None && Op2 == Operation::Conj) return MulMacroOp::NegMulSymConj2;
    else if constexpr (Op1 == Operation::None && Op2 == Operation::None) return MulMacroOp::NegMulSym;
#else
    UNREACHABLE_MSG("Not supported in the current architecture");
    return MulMacroOp::Unavailable;
#endif
}

template <Operation Op1, Operation Op2>
constexpr MulMacroOp to_negmul_antisym_macro_op()
{
#if __AIE_ARCH__ == 10
    if      constexpr (Op1 == Operation::Conj && Op2 == Operation::None) return MulMacroOp::NegMulAntisymConj1;
    else if constexpr (Op1 == Operation::Conj && Op2 == Operation::Conj) return MulMacroOp::NegMulAntisymConj1Conj2;
    else if constexpr (Op1 == Operation::None && Op2 == Operation::Conj) return MulMacroOp::NegMulAntisymConj2;
    else if constexpr (Op1 == Operation::None && Op2 == Operation::None) return MulMacroOp::NegMulAntisym;
#else
    UNREACHABLE_MSG("Not supported in the current architecture");
    return MulMacroOp::Unavailable;
#endif
}

template <MulMacroOp Op>
constexpr MulMacroOp add_to_op()
{
    if      constexpr (Op == MulMacroOp::Mul                     || Op == MulMacroOp::Add_Mul)                  return MulMacroOp::Add_Mul;
    else if constexpr (Op == MulMacroOp::NegMul                  || Op == MulMacroOp::Add_NegMul)               return MulMacroOp::Add_NegMul;
#if __AIE_ARCH__ == 10
    else if constexpr (Op == MulMacroOp::MulSym                  || Op == MulMacroOp::Add_MulSym)               return MulMacroOp::Add_MulSym;
    else if constexpr (Op == MulMacroOp::NegMulSym               || Op == MulMacroOp::Add_NegMulSym)            return MulMacroOp::Add_NegMulSym;
    else if constexpr (Op == MulMacroOp::MulAntisym              || Op == MulMacroOp::Add_MulAntisym)           return MulMacroOp::Add_MulAntisym;
    else if constexpr (Op == MulMacroOp::NegMulAntisym           || Op == MulMacroOp::Add_NegMulAntisym)        return MulMacroOp::Add_NegMulAntisym;
    else if constexpr (Op == MulMacroOp::MulAbs1                 || Op == MulMacroOp::Add_MulAbs1)              return MulMacroOp::Add_MulAbs1;
    else if constexpr (Op == MulMacroOp::MulAbs1Conj2            || Op == MulMacroOp::Add_MulAbs1Conj2)         return MulMacroOp::Add_MulAbs1Conj2;
    else if constexpr (Op == MulMacroOp::MulConj1                || Op == MulMacroOp::Add_MulConj1)             return MulMacroOp::Add_MulConj1;
    else if constexpr (Op == MulMacroOp::MulConj2                || Op == MulMacroOp::Add_MulConj2)             return MulMacroOp::Add_MulConj2;
    else if constexpr (Op == MulMacroOp::MulConj1Conj2           || Op == MulMacroOp::Add_MulConj1Conj2)        return MulMacroOp::Add_MulConj1Conj2;
    else if constexpr (Op == MulMacroOp::MulSymConj1             || Op == MulMacroOp::Add_MulSymConj1)          return MulMacroOp::Add_MulSymConj1;
    else if constexpr (Op == MulMacroOp::MulSymConj2             || Op == MulMacroOp::Add_MulSymConj2)          return MulMacroOp::Add_MulSymConj2;
    else if constexpr (Op == MulMacroOp::MulSymConj1Conj2        || Op == MulMacroOp::Add_MulSymConj1Conj2)     return MulMacroOp::Add_MulSymConj1Conj2;
    else if constexpr (Op == MulMacroOp::MulAntisymConj1         || Op == MulMacroOp::Add_MulAntisymConj1)      return MulMacroOp::Add_MulAntisymConj1;
    else if constexpr (Op == MulMacroOp::MulAntisymConj2         || Op == MulMacroOp::Add_MulAntisymConj2)      return MulMacroOp::Add_MulAntisymConj2;
    else if constexpr (Op == MulMacroOp::MulAntisymConj1Conj2    || Op == MulMacroOp::Add_MulAntisymConj1Conj2) return MulMacroOp::Add_MulAntisymConj1Conj2;
    else if constexpr (Op == MulMacroOp::NegMulAbs1              || Op == MulMacroOp::Sub_MulAbs1)              return MulMacroOp::Sub_MulAbs1;
    else if constexpr (Op == MulMacroOp::NegMulAbs1Conj2         || Op == MulMacroOp::Sub_MulAbs1Conj2)         return MulMacroOp::Sub_MulAbs1Conj2;
    else if constexpr (Op == MulMacroOp::NegMulConj1             || Op == MulMacroOp::Sub_MulConj1)             return MulMacroOp::Sub_MulConj1;
    else if constexpr (Op == MulMacroOp::NegMulConj2             || Op == MulMacroOp::Sub_MulConj2)             return MulMacroOp::Sub_MulConj2;
    else if constexpr (Op == MulMacroOp::NegMulConj1Conj2        || Op == MulMacroOp::Sub_MulConj1Conj2)        return MulMacroOp::Sub_MulConj1Conj2;
    else if constexpr (Op == MulMacroOp::NegMulSymConj1          || Op == MulMacroOp::Sub_MulSymConj1)          return MulMacroOp::Sub_MulSymConj1;
    else if constexpr (Op == MulMacroOp::NegMulSymConj2          || Op == MulMacroOp::Sub_MulSymConj2)          return MulMacroOp::Sub_MulSymConj2;
    else if constexpr (Op == MulMacroOp::NegMulSymConj1Conj2     || Op == MulMacroOp::Sub_MulSymConj1Conj2)     return MulMacroOp::Sub_MulSymConj1Conj2;
    else if constexpr (Op == MulMacroOp::NegMulAntisymConj1      || Op == MulMacroOp::Sub_MulAntisymConj1)      return MulMacroOp::Sub_MulAntisymConj1;
    else if constexpr (Op == MulMacroOp::NegMulAntisymConj2      || Op == MulMacroOp::Sub_MulAntisymConj2)      return MulMacroOp::Sub_MulAntisymConj2;
    else if constexpr (Op == MulMacroOp::NegMulAntisymConj1Conj2 || Op == MulMacroOp::Sub_MulAntisymConj1Conj2) return MulMacroOp::Sub_MulAntisymConj1Conj2;
    else if constexpr (Op == MulMacroOp::MulMax                  || Op == MulMacroOp::Add_MulMax)               return MulMacroOp::Add_MulMax;
    else if constexpr (Op == MulMacroOp::MulMin                  || Op == MulMacroOp::Add_MulMin)               return MulMacroOp::Add_MulMin;
#elif __AIE_ARCH__ == 20 || __AIE_ARCH__ == 21 || __AIE_ARCH__ == 22
    else if constexpr (Op == MulMacroOp::MulConj1                || Op == MulMacroOp::Add_MulConj1)             return MulMacroOp::Add_MulConj1;
    else if constexpr (Op == MulMacroOp::MulConj2                || Op == MulMacroOp::Add_MulConj2)             return MulMacroOp::Add_MulConj2;
    else if constexpr (Op == MulMacroOp::MulConj1Conj2           || Op == MulMacroOp::Add_MulConj1Conj2)        return MulMacroOp::Add_MulConj1Conj2;
    else if constexpr (Op == MulMacroOp::NegMulConj1             || Op == MulMacroOp::Sub_MulConj1)             return MulMacroOp::Sub_MulConj1;
    else if constexpr (Op == MulMacroOp::NegMulConj2             || Op == MulMacroOp::Sub_MulConj2)             return MulMacroOp::Sub_MulConj2;
    else if constexpr (Op == MulMacroOp::NegMulConj1Conj2        || Op == MulMacroOp::Sub_MulConj1Conj2)        return MulMacroOp::Sub_MulConj1Conj2;
#endif
}

template <MulMacroOp Op>
constexpr MulMacroOp remove_acc_from_op()
{
    if      constexpr (Op == MulMacroOp::Add_Mul)                  return MulMacroOp::Mul;
    else if constexpr (Op == MulMacroOp::Add_NegMul)               return MulMacroOp::NegMul;
#if __AIE_ARCH__ == 10
    else if constexpr (Op == MulMacroOp::Add_MulSym)               return MulMacroOp::MulSym;
    else if constexpr (Op == MulMacroOp::Add_NegMulSym)            return MulMacroOp::NegMulSym;
    else if constexpr (Op == MulMacroOp::Add_MulAntisym)           return MulMacroOp::MulAntisym;
    else if constexpr (Op == MulMacroOp::Add_NegMulAntisym)        return MulMacroOp::NegMulAntisym;
    else if constexpr (Op == MulMacroOp::Add_MulAbs1)              return MulMacroOp::MulAbs1;
    else if constexpr (Op == MulMacroOp::Add_MulAbs1Conj2)         return MulMacroOp::MulAbs1Conj2;
    else if constexpr (Op == MulMacroOp::Add_MulConj1)             return MulMacroOp::MulConj1;
    else if constexpr (Op == MulMacroOp::Add_MulConj2)             return MulMacroOp::MulConj2;
    else if constexpr (Op == MulMacroOp::Add_MulConj1Conj2)        return MulMacroOp::MulConj1Conj2;
    else if constexpr (Op == MulMacroOp::Add_MulSymConj1)          return MulMacroOp::MulSymConj1;
    else if constexpr (Op == MulMacroOp::Add_MulSymConj2)          return MulMacroOp::MulSymConj2;
    else if constexpr (Op == MulMacroOp::Add_MulSymConj1Conj2)     return MulMacroOp::MulSymConj1Conj2;
    else if constexpr (Op == MulMacroOp::Add_MulAntisymConj1)      return MulMacroOp::MulAntisymConj1;
    else if constexpr (Op == MulMacroOp::Add_MulAntisymConj2)      return MulMacroOp::MulAntisymConj2;
    else if constexpr (Op == MulMacroOp::Add_MulAntisymConj1Conj2) return MulMacroOp::MulAntisymConj1Conj2;
    else if constexpr (Op == MulMacroOp::Sub_MulAbs1)              return MulMacroOp::MulAbs1;
    else if constexpr (Op == MulMacroOp::Sub_MulAbs1Conj2)         return MulMacroOp::MulAbs1Conj2;
    else if constexpr (Op == MulMacroOp::Sub_MulConj1)             return MulMacroOp::MulConj1;
    else if constexpr (Op == MulMacroOp::Sub_MulConj2)             return MulMacroOp::MulConj2;
    else if constexpr (Op == MulMacroOp::Sub_MulConj1Conj2)        return MulMacroOp::MulConj1Conj2;
    else if constexpr (Op == MulMacroOp::Sub_MulSymConj1)          return MulMacroOp::MulSymConj1;
    else if constexpr (Op == MulMacroOp::Sub_MulSymConj2)          return MulMacroOp::MulSymConj2;
    else if constexpr (Op == MulMacroOp::Sub_MulSymConj1Conj2)     return MulMacroOp::MulSymConj1Conj2;
    else if constexpr (Op == MulMacroOp::Sub_MulAntisymConj1)      return MulMacroOp::MulAntisymConj1;
    else if constexpr (Op == MulMacroOp::Sub_MulAntisymConj2)      return MulMacroOp::MulAntisymConj2;
    else if constexpr (Op == MulMacroOp::Sub_MulAntisymConj1Conj2) return MulMacroOp::MulAntisymConj1Conj2;
    else if constexpr (Op == MulMacroOp::Add_MulMax)               return MulMacroOp::MulMax;
    else if constexpr (Op == MulMacroOp::Add_MulMin)               return MulMacroOp::MulMin;
#elif __AIE_ARCH__ == 20 || __AIE_ARCH__ == 21 || __AIE_ARCH__ == 22
    else if constexpr (Op == MulMacroOp::Add_MulConj1)             return MulMacroOp::MulConj1;
    else if constexpr (Op == MulMacroOp::Add_MulConj2)             return MulMacroOp::MulConj2;
    else if constexpr (Op == MulMacroOp::Add_MulConj1Conj2)        return MulMacroOp::MulConj1Conj2;
    else if constexpr (Op == MulMacroOp::Sub_MulConj1)             return MulMacroOp::MulConj1;
    else if constexpr (Op == MulMacroOp::Sub_MulConj2)             return MulMacroOp::MulConj2;
    else if constexpr (Op == MulMacroOp::Sub_MulConj1Conj2)        return MulMacroOp::MulConj1Conj2;
#endif
    else return Op;
}

#if __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
template <MulMacroOp MulOp>
static constexpr auto get_scalar_mul_op()
{
    if constexpr (MulOp == MulMacroOp::Mul)
        return [](const auto &a, const auto &b) { return a * b; };
    else if constexpr (MulOp == MulMacroOp::NegMul)
        return [](const auto &a, const auto &b) { return -(a * b); };
    else if constexpr (MulOp == MulMacroOp::Add_Mul)
        return [](const auto &a, const auto &b, const auto &acc) { return acc + a * b; };
    else if constexpr (MulOp == MulMacroOp::Add_NegMul || MulOp == MulMacroOp::Sub_Mul)
        return [](const auto &a, const auto &b, const auto &acc) { return acc -(a * b); };
#if __AIE_ARCH__ == 10
    else if constexpr (MulOp == MulMacroOp::MulAbs1)
        return [](const auto &a, const auto &b)                    { return std::abs(a) * b; };
    else if constexpr (MulOp == MulMacroOp::MulAbs1Conj2)
        return [](const auto &a, const auto &_b)                   { auto b = { _b.real, (decltype(_b.imag))-_b.imag }; return std::abs(a) * b; };
    else if constexpr (MulOp == MulMacroOp::MulConj1)
        return [](const auto &_a, const auto &b)                   { auto a = { _a.real, (decltype(_a.imag))-_a.imag }; return a * b; };
    else if constexpr (MulOp == MulMacroOp::MulConj1Conj2)
        return [](const auto &_a, const auto &_b)                  { auto a = { _a.real, (decltype(_a.imag))-_a.imag }; auto b = { _b.real, (decltype(_b.imag))-_b.imag }; return a * b; };
    else if constexpr (MulOp == MulMacroOp::NegMulAbs1)
        return [](const auto &a, const auto &b)                    { return -(std::abs(a) * b); };
    else if constexpr (MulOp == MulMacroOp::NegMulAbs1Conj2)
        return [](const auto &a, const auto &_b)                   { auto b = { _b.real, (decltype(_b.imag))-_b.imag }; return -(std::abs(a) * b); };
    else if constexpr (MulOp == MulMacroOp::NegMulConj1)
        return [](const auto &_a, const auto &b)                   { auto a = { _a.real, (decltype(_a.imag))-_a.imag }; return -(a * b); };
    else if constexpr (MulOp == MulMacroOp::MulConj1Conj2)
        return [](const auto &_a, const auto &_b)                  { auto a = { _a.real, (decltype(_a.imag))-_a.imag }; auto b = { _b.real, (decltype(_b.imag))-_b.imag }; return -(a * b); };
    else if constexpr (MulOp == MulMacroOp::MulMax)
        return [](const auto &a, const auto &m1, const auto &m2)   { return a * std::max(m1, m2); };
    else if constexpr (MulOp == MulMacroOp::MulMin)
        return [](const auto &a, const auto &m1, const auto &m2)   { return a * std::min(m1, m2); };
    else if constexpr (MulOp == MulMacroOp::Add_MulAbs1)
        return [](const auto &a, const auto &b, const auto &acc)   { return acc + std::abs(a) * b; };
    else if constexpr (MulOp == MulMacroOp::Add_MulAbs1Conj2)
        return [](const auto &a, const auto &_b, const auto &acc)  { auto b = { _b.real, (decltype(_b.imag))-_b.imag }; return acc + std::abs(a) * b; };
    else if constexpr (MulOp == MulMacroOp::Add_MulConj1)
        return [](const auto &_a, const auto &b, const auto &acc)  { auto a = { _a.real, (decltype(_a.imag))-_a.imag }; return acc + a * b; };
    else if constexpr (MulOp == MulMacroOp::Add_MulConj1Conj2)
        return [](const auto &_a, const auto &_b, const auto &acc) { auto a = { _a.real, (decltype(_a.imag))-_a.imag }; auto b = { _b.real, (decltype(_b.imag))-_b.imag }; return acc + a * b; };
    else if constexpr (MulOp == MulMacroOp::Sub_MulAbs1)
        return [](const auto &a, const auto &b, const auto &acc)  { return acc -(std::abs(a) * b); };
    else if constexpr (MulOp == MulMacroOp::Sub_MulAbs1Conj2)
        return [](const auto &a, const auto &_b, const auto &acc)  { auto b = { _b.real, (decltype(_b.imag))-_b.imag }; return acc - std::abs(a) * b; };
    else if constexpr (MulOp == MulMacroOp::Sub_MulConj1)
        return [](const auto &_a, const auto &b, const auto &acc)  { auto a = { _a.real, (decltype(_a.imag))-_a.imag }; return acc - a * b; };
    else if constexpr (MulOp == MulMacroOp::Sub_MulConj1Conj2)
        return [](const auto &_a, const auto &_b, const auto &acc) { auto a = { _a.real, (decltype(_a.imag))-_a.imag }; auto b = { _b.real, (decltype(_b.imag))-_b.imag }; return acc - a * b; };
    else if constexpr (MulOp == MulMacroOp::Add_MulMax)
        return [](const auto &m1, const auto &m2, const auto &a, const auto &acc) { return acc - a * std::max(m1, m2); };
    else if constexpr (MulOp == MulMacroOp::Sub_MulMax)
        return [](const auto &m1, const auto &m2, const auto &a, const auto &acc) { return acc + a * std::max(m1, m2); };
    else if constexpr (MulOp == MulMacroOp::Add_MulMin)
        return [](const auto &m1, const auto &m2, const auto &a, const auto &acc) { return acc - a * std::min(m1, m2); };
    else if constexpr (MulOp == MulMacroOp::Sub_MulMin)
        return [](const auto &m1, const auto &m2, const auto &a, const auto &acc) { return acc + a * std::min(m1, m2); };
#elif __AIE_ARCH__ == 20 || __AIE_ARCH__ == 21 || __AIE_ARCH__ == 22
    else if constexpr (MulOp == MulMacroOp::MulConj1)
        return [](const auto &_a, const auto &b)                   { auto a = { _a.real, (decltype(_a.imag))-_a.imag }; return a * b; };
    else if constexpr (MulOp == MulMacroOp::MulConj1Conj2)
        return [](const auto &_a, const auto &_b)                  { auto a = { _a.real, (decltype(_a.imag))-_a.imag }; auto b = { _b.real, (decltype(_b.imag))-_b.imag }; return a * b; };
    else if constexpr (MulOp == MulMacroOp::NegMulConj1)
        return [](const auto &_a, const auto &b)                   { auto a = { _a.real, (decltype(_a.imag))-_a.imag }; return -(a * b); };
    else if constexpr (MulOp == MulMacroOp::MulConj1Conj2)
        return [](const auto &_a, const auto &_b)                  { auto a = { _a.real, (decltype(_a.imag))-_a.imag }; auto b = { _b.real, (decltype(_b.imag))-_b.imag }; return -(a * b); };
    else if constexpr (MulOp == MulMacroOp::Add_MulConj1)
        return [](const auto &_a, const auto &b, const auto &acc)  { auto a = { _a.real, (decltype(_a.imag))-_a.imag }; return acc + a * b; };
    else if constexpr (MulOp == MulMacroOp::Add_MulConj1Conj2)
        return [](const auto &_a, const auto &_b, const auto &acc) { auto a = { _a.real, (decltype(_a.imag))-_a.imag }; auto b = { _b.real, (decltype(_b.imag))-_b.imag }; return acc + a * b; };
    else if constexpr (MulOp == MulMacroOp::Sub_MulConj1)
        return [](const auto &_a, const auto &b, const auto &acc)  { auto a = { _a.real, (decltype(_a.imag))-_a.imag }; return acc - a * b; };
    else if constexpr (MulOp == MulMacroOp::Sub_MulConj1Conj2)
        return [](const auto &_a, const auto &_b, const auto &acc) { auto a = { _a.real, (decltype(_a.imag))-_a.imag }; auto b = { _b.real, (decltype(_b.imag))-_b.imag }; return acc - a * b; };
#endif
}
#endif

template <typename T>
__aie_inline
constexpr bool get_mul_sign(T v)
{
        if constexpr (is_op_v<T> && T::is_operation(Operation::Sign))
            return v.parent2();
        else if constexpr (is_tbs_op_v<T>)
            return detail::is_signed_v<typename T::elem_type>; 
        else
            return detail::is_signed_v<typename T::value_type>;
}

template <typename T1, typename T2>
__aie_inline
constexpr bool eval_sub_mul(T1 v1, T2 v2)
{
        if      constexpr ((is_op_v<T1> && T1::is_operation(Operation::Neg)) &&
                           (is_op_v<T2> && T2::is_operation(Operation::Neg)))   return v1.parent2() != v2.parent2();
        else if constexpr (is_op_v<T1> && T1::is_operation(Operation::Neg))     return v1.parent2();
        else if constexpr (is_op_v<T2> && T2::is_operation(Operation::Neg))     return v2.parent2();
        else                                                                    return false;
}

template <typename T>
__aie_inline
constexpr Operation evaluate_mul_operation()
{
    //TODO: Simplify based on evaluating on parent ops if parent isn't a vector
    if constexpr (T::is_operation(Operation::Sign, Operation::Neg))
        return Operation::None;
    else
        return T::operation;
}

// AIE2+: avoid compiling unsupported fallback; surface guidance via static_assert below.
#define AIE_API_MUL_BITS_FALLBACK_UNSUPPORTED                                                     \
    "This multiply or multiply-accumulate is not supported for these operand types and "          \
    "accumulator choice on this architecture."

// Always false when instantiated; name is chosen so diagnostics list operand types and accum tag clearly.
template <typename Operand1, typename Operand2, typename AccumulatorTag>
inline constexpr bool unsupported_mul_mac_configuration_v = false;

template <MulMacroOp MulOp, unsigned AccumBits, unsigned Type1Bits, typename T1, unsigned Type2Bits, typename T2>
struct mul_bits_impl
{
#if __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;

    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;

    using    accum_tag = accum_tag_for_mul_types<T1, T2, AccumBits>;
    template <unsigned Elems>
    using   accum_type = accum<accum_tag, Elems>;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign,
                                 const vector_type2<Elems> &v2, bool v2_sign,
#if __AIE_ARCH__ != 10
                                 bool sub_mul, bool sub_acc,
                                 bool zero_acc,
#endif
                                 const Acc &... acc)
    {
#if __AIE_ARCH__ == 10
        accum_type<Elems> ret;
        auto mul_op = get_scalar_mul_op<MulOp>();

        for (unsigned i = 0; i < Elems; ++i)
            ret[i] = mul_op(v1[i], v2[i], acc...);

        return ret;
#else
        static_assert(unsupported_mul_mac_configuration_v<T1, T2, accum_tag>,
                      AIE_API_MUL_BITS_FALLBACK_UNSUPPORTED);
#endif
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(T1 a,                         bool a_sign,
                                 const vector_type2<Elems> &v, bool v_sign,
#if __AIE_ARCH__ != 10
                                 bool sub_mul, bool sub_acc,
                                 bool zero_acc,
#endif
                                 const Acc &... acc)
    {
#if __AIE_ARCH__ == 10
        accum_type<Elems> ret;
        auto mul_op = get_scalar_mul_op<MulOp>();

        for (unsigned i = 0; i < Elems; ++i)
            ret[i] = mul_op(a, v[i], acc...);

        return ret;
#else
        static_assert(unsupported_mul_mac_configuration_v<T1, T2, accum_tag>,
                      AIE_API_MUL_BITS_FALLBACK_UNSUPPORTED);
#endif
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign,
                                 T2 a,                         bool a_sign,
#if __AIE_ARCH__ != 10
                                 bool sub_mul, bool sub_acc,
                                 bool zero_acc,
#endif
                                 const Acc &... acc)
    {
#if __AIE_ARCH__ == 10
        accum_type<Elems> ret;
        auto mul_op = get_scalar_mul_op<MulOp>();

        for (unsigned i = 0; i < Elems; ++i)
            ret[i] = mul_op(v[i], a, acc...);

        return ret;
#else
        static_assert(unsupported_mul_mac_configuration_v<T1, T2, accum_tag>,
                      AIE_API_MUL_BITS_FALLBACK_UNSUPPORTED);
#endif
    }
#endif
};

#undef AIE_API_MUL_BITS_FALLBACK_UNSUPPORTED

template <MulMacroOp MulOp, unsigned AccumBits, unsigned Type1Bits, typename T1, unsigned Type2Bits, typename T2>
struct mul_maxmin_bits_impl
{
#if __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;

    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;

    using    accum_tag = accum_tag_for_mul_types<T1, T2, AccumBits>;

    template <unsigned Elems>
    using   accum_type = accum<accum_tag, Elems>;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static auto run(const vector_type1<Elems> &m1,
                    const vector_type1<Elems> &m2,
                    const vector_type2<Elems> &v, const Acc &... acc)
    {
        // Add subscript operator for accumulators
        accum_type<Elems> ret;
        auto mul_op = get_scalar_mul_op<MulOp>();

        for (unsigned i = 0; i < Elems; ++i)
            ret[i] = mul_op(m1[i], m2[i], v[i], acc...);

        return ret;
    }
#endif
};

template <unsigned AccumBits, unsigned TypeBits, typename T, unsigned Elems>
struct mul_reduce_bits_impl
{
#if __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
    using vector_type = vector<T, Elems>;

    static T run(const vector_type &v)
    {
        T ret = v[0];

        for (unsigned i = 1; i < Elems; ++i)
            ret *= (T)v[i];

        return ret;
    }
#endif
};

// TODO: do scalar implementation
template <unsigned OutElems, unsigned Points, int CoeffStep, int DataStepX, int DataStepY, unsigned AccumBits, unsigned CoeffTypeBits, unsigned DataTypeBits, typename CoeffType, typename DataType>
struct sliding_mul_bits_impl;

template <unsigned OutElems, unsigned Points, bool HasOddPoints, int CoeffStep, int DataStepX, int DataStepY, unsigned AccumBits, unsigned CoeffTypeBits, unsigned DataTypeBits, typename CoeffType, typename DataType>
struct sliding_mul_sym_bits_impl;

template <unsigned OutElems, unsigned Points, int CoeffStep, int DataStep, unsigned AccumBits, unsigned CoeffTypeBits, unsigned DataTypeBits, typename CoeffType, typename DataType>
struct sliding_mul_sym_uct_bits_impl;

template <unsigned OutElems, unsigned Points, unsigned Channels, int CoeffStep, int DataStepX, int DataStepY, unsigned AccumBits, unsigned CoeffTypeBits, unsigned DataTypeBits, typename CoeffType, typename DataType>
struct sliding_mul_ch_bits_impl;

template <MulMacroOp MulOp, unsigned AccumBits, unsigned Type1Bits, typename T1, unsigned Type2Bits, typename T2>
struct mul_bits
{
    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;
    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static auto run(const vector_type1<Elems> &v1, bool v1_sign,
                    const vector_type2<Elems> &v2, bool v2_sign,
                    bool sub_mul, bool sub_acc,
                    bool zero_acc, const Acc &... acc)
    {
#if __AIE_ARCH__ == 10
        return mul_bits_impl<MulOp, AccumBits, Type1Bits, T1, Type2Bits, T2>::run(v1, v1_sign, v2, v2_sign, acc...);
#else
        return mul_bits_impl<MulOp, AccumBits, Type1Bits, T1, Type2Bits, T2>::run(v1, v1_sign, v2, v2_sign, sub_mul, sub_acc, zero_acc, acc...);
#endif
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static auto run(const vector_type1<Elems> &v1, bool v1_sign,
                    const vector_type2<Elems> &v2, bool v2_sign,
                    const Acc &... acc)
    {
        return run(v1, v1_sign, v2, v2_sign, false, false, false, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static auto run(const vector_type1<Elems> &v1, bool v1_sign,
                    const vector_type2<Elems> &v2, bool v2_sign,
                    bool sub_mul,
                    const Acc &... acc)
    {
        return run(v1, v1_sign, v2, v2_sign, sub_mul, false, false, acc...);
    }

    template <unsigned Elems, unsigned Elems2, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static auto run(vector_elem_const_ref<T1, Elems> a, bool a_sign,
                    const vector_type2<Elems2> &v,      bool v_sign,
                    bool sub_mul, bool sub_acc,
                    bool zero_acc, const Acc &... acc)
    {
#if __AIE_ARCH__ == 10
        return mul_bits_impl<MulOp, AccumBits, Type1Bits, T1, Type2Bits, T2>::run(a, a_sign, v, v_sign, acc...);
#else
        return mul_bits_impl<MulOp, AccumBits, Type1Bits, T1, Type2Bits, T2>::run(a, a_sign, v, v_sign, sub_mul, sub_acc, zero_acc, acc...);
#endif
    }

    template <unsigned Elems, unsigned Elems2, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static auto run(vector_elem_const_ref<T1, Elems> a, bool a_sign,
                    const vector_type2<Elems2> &v,      bool v_sign,
                    const Acc &... acc)
    {
        return run(a, a_sign, v, v_sign, false, false, false, acc...);
    }

    template <unsigned Elems, unsigned Elems2, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static auto run(vector_elem_const_ref<T1, Elems> a, bool a_sign,
                    const vector_type2<Elems2> &v,      bool v_sign,
                    bool sub_mul,
                    const Acc &... acc)
    {
        return run(a, a_sign, v, v_sign, sub_mul, false, false, acc...);
    }

    template <unsigned Elems, unsigned Elems2, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static auto run(const vector_type1<Elems> &v,        bool v_sign,
                    vector_elem_const_ref<T2, Elems2> a, bool a_sign,
                    bool sub_mul, bool sub_acc,
                    bool zero_acc, const Acc &... acc)
    {
#if __AIE_ARCH__ == 10
        if (!chess_const(a.offset))
            return mul_bits_impl<MulOp, AccumBits, Type1Bits, T1, Type2Bits, T2>::run(v, v_sign, a.get(), a_sign, acc...);
        else
            return mul_bits_impl<MulOp, AccumBits, Type1Bits, T1, Type2Bits, T2>::run(v, v_sign, a, a_sign, acc...);
#else
        return mul_bits_impl<MulOp, AccumBits, Type1Bits, T1, Type2Bits, T2>::run(v, v_sign, a, a_sign, sub_mul, sub_acc, zero_acc, acc...);
#endif
    }

    template <unsigned Elems, unsigned Elems2, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static auto run(const vector_type1<Elems> &v,        bool v_sign,
                    vector_elem_const_ref<T2, Elems2> a, bool a_sign,
                    const Acc &... acc)
    {
        return run(v, v_sign, a, a_sign, false, false, false, acc...);
    }

    template <unsigned Elems, unsigned Elems2, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static auto run(const vector_type1<Elems> &v,        bool v_sign,
                    vector_elem_const_ref<T2, Elems2> a, bool a_sign,
                    bool sub_mul,
                    const Acc &... acc)
    {
        return run(v, v_sign, a, a_sign, sub_mul, false, false, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static auto run(const T1 &a,                  bool a_sign,
                    const vector_type2<Elems> &v, bool v_sign,
                    bool sub_mul, bool sub_acc,
                    bool zero_acc, const Acc &... acc)
    {
#if __AIE_ARCH__ == 10
        return mul_bits_impl<MulOp, AccumBits, Type1Bits, T1, Type2Bits, T2>::run(a, a_sign, v, v_sign, acc...);
#else
        return mul_bits_impl<MulOp, AccumBits, Type1Bits, T1, Type2Bits, T2>::run(a, a_sign, v, v_sign, sub_mul, sub_acc, zero_acc, acc...);
#endif
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static auto run(const T1 &a,                  bool a_sign,
                    const vector_type2<Elems> &v, bool v_sign,
                    const Acc &... acc)
    {
        return run(a, a_sign, v, v_sign, false, false, false, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static auto run(const T1 &a,                  bool a_sign,
                    const vector_type2<Elems> &v, bool v_sign,
                    bool sub_mul,
                    const Acc &... acc)
    {
        return run(a, a_sign, v, v_sign, sub_mul, false, false, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static auto run(const vector_type1<Elems> &v, bool v_sign,
                    const T2 &a,                  bool a_sign,
                    bool sub_mul, bool sub_acc,
                    bool zero_acc, const Acc &... acc)
    {
#if __AIE_ARCH__ == 10
        return mul_bits_impl<MulOp, AccumBits, Type1Bits, T1, Type2Bits, T2>::run(v, v_sign, a, a_sign, acc...);
#else
        return mul_bits_impl<MulOp, AccumBits, Type1Bits, T1, Type2Bits, T2>::run(v, v_sign, a, a_sign, sub_mul, sub_acc, zero_acc, acc...);
#endif
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static auto run(const vector_type1<Elems> &v, bool v_sign,
                    const T2 &a,                  bool a_sign,
                    const Acc &... acc)
    {
        return run(v, v_sign, a, a_sign, false, false, false, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static auto run(const vector_type1<Elems> &v, bool v_sign,
                    const T2 &a,                  bool a_sign,
                    bool sub_mul,
                    const Acc &... acc)
    {
        return run(v, v_sign, a, a_sign, sub_mul, false, false, acc...);
    }
};

template <MulMacroOp MulOp, unsigned AccumBits, unsigned Type1Bits, typename T1, unsigned Type2Bits, typename T2>
struct mul_maxmin_bits
{
    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;
    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static auto run(const vector_type1<Elems> &m1,
                    const vector_type1<Elems> &m2,
                    const vector_type2<Elems> &v, const Acc &... acc)
    {
        return mul_maxmin_bits_impl<MulOp, AccumBits, Type1Bits, T1, Type2Bits, T2>::run(m1, m2, v, acc...);
    }
};

template <unsigned AccumBits, unsigned TypeBits, typename T, unsigned Elems>
struct mul_reduce_bits
{
    using vector_type = vector<T, Elems>;

    __aie_inline
    static T run(const vector_type &v)
    {
        return mul_reduce_bits_impl<AccumBits, TypeBits, T, Elems>::run(v);
    }
};

template <unsigned OutElems, unsigned Points, int CoeffStep, int DataStepX, int DataStepY, unsigned AccumBits, unsigned CoeffTypeBits, unsigned DataTypeBits, typename CoeffType, typename DataType>
struct sliding_mul_bits
{
    using impl_type = sliding_mul_bits_impl<OutElems, Points, CoeffStep, DataStepX, DataStepY, AccumBits, CoeffTypeBits, DataTypeBits, CoeffType, DataType>;

    using  data_type = typename impl_type::data_type;
    using coeff_type = typename impl_type::coeff_type;
    using accum_type = typename impl_type::accum_type;

    static constexpr unsigned columns_per_mul = impl_type::columns_per_mul;
    static constexpr unsigned   lanes_per_mul = impl_type::lanes_per_mul;
    static constexpr unsigned         num_mul = impl_type::num_mul;
    static constexpr unsigned           lanes = OutElems;
    static constexpr unsigned          points = Points;

    template <MulMacroOp Op, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static auto run(const vector<CoeffType, N_Coeff> &coeff,
                    unsigned coeff_start,
                    bool coeff_sign,
                    const vector<DataType, N_Data> &data,
                    unsigned data_start,
                    bool data_sign,
                    bool zero_acc,
                    const Acc &... acc)
    {
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

#if __AIE_ARCH__ == 10
        return impl_type::template run<Op>(coeff, coeff_start, coeff_sign, data, data_start, data_sign, acc...);
#else
        return impl_type::template run<Op>(coeff, coeff_start, coeff_sign, data, data_start, data_sign, zero_acc, acc...);
#endif
    }
};

template <unsigned OutElems, unsigned Points, int CoeffStep, int DataStepX, int DataStepY, unsigned AccumBits, unsigned CoeffTypeBits, unsigned DataTypeBits, typename CoeffType, typename DataType>
struct sliding_mul_sym_bits
{
    using impl_type = sliding_mul_sym_bits_impl<OutElems, Points, (Points % 2) == 1, CoeffStep, DataStepX, DataStepY, AccumBits, CoeffTypeBits, DataTypeBits, CoeffType, DataType>;

    using  data_type = typename impl_type::data_type;
    using coeff_type = typename impl_type::coeff_type;
    using accum_type = typename impl_type::accum_type;

    static constexpr unsigned columns_per_mul = impl_type::columns_per_mul;
    static constexpr unsigned   lanes_per_mul = impl_type::lanes_per_mul;
    static constexpr unsigned         num_mul = impl_type::num_mul;
    static constexpr unsigned           lanes = OutElems;
    static constexpr unsigned          points = Points;

    template <MulMacroOp Op, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static auto run(const vector<CoeffType, N_Coeff> &coeff,
                    unsigned coeff_start,
                    const vector<DataType, N_Data> &data,
                    unsigned data_start,
                    bool /*zero_acc*/,
                    const Acc &... acc)
    {
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        if constexpr (Points % 2 == 0)
            return impl_type::template run<Op>(coeff, coeff_start, data, data_start, data_start + (Points - 1) * DataStepX, acc...);
        else
            return impl_type::template run<Op>(coeff, coeff_start, data, data_start,                                        acc...);
    }

    template <MulMacroOp Op, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static auto run(const vector<CoeffType, N_Coeff> &coeff,
                    unsigned coeff_start,
                    const vector<DataType, N_Data> &data,
                    unsigned ldata_start,
                    unsigned rdata_start,
                    bool /*zero_acc*/,
                    const Acc &... acc)
    {
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        return impl_type::template run<Op>(coeff, coeff_start, data, ldata_start, rdata_start, acc...);
    }

    template <MulMacroOp Op, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static auto run_2buff(const vector<CoeffType, N_Coeff> &coeff,
                          unsigned coeff_start,
                          const vector<DataType, N_Data> &ldata,
                          unsigned ldata_start,
                          const vector<DataType, N_Data> &rdata,
                          unsigned rdata_start,
                          bool /*zero_acc*/,
                          const Acc &... acc)
    {
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        return impl_type::template run<Op>(coeff, coeff_start, ldata, ldata_start, rdata, rdata_start, acc...);
    }
};

template <unsigned OutElems, unsigned Points, int CoeffStep, int DataStep, unsigned AccumBits, unsigned CoeffTypeBits, unsigned DataTypeBits, typename CoeffType, typename DataType>
struct sliding_mul_sym_uct_bits
{
    using impl_type = sliding_mul_sym_uct_bits_impl<OutElems, Points, CoeffStep, DataStep, AccumBits, CoeffTypeBits, DataTypeBits, CoeffType, DataType>;

    using  data_type = typename impl_type::data_type;
    using coeff_type = typename impl_type::coeff_type;
    using accum_type = typename impl_type::accum_type;

    static constexpr unsigned columns_per_mul = impl_type::columns_per_mul;
    static constexpr unsigned   lanes_per_mul = impl_type::lanes_per_mul;
    static constexpr unsigned         num_mul = impl_type::num_mul;
    static constexpr unsigned           lanes = OutElems;
    static constexpr unsigned          points = Points;

    template <MulMacroOp Op, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static auto run(const vector<CoeffType, N_Coeff> &coeff,
                    unsigned coeff_start,
                    const vector<DataType, N_Data> &data,
                    unsigned data_start,
                    unsigned uct_shift,
                    bool /*zero_acc*/,
                    const Acc &... acc)
    {
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        return impl_type::template run<Op>(coeff, coeff_start, data, data_start, uct_shift, acc...);
    }

    template <MulMacroOp Op, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static auto run_2buff(const vector<CoeffType, N_Coeff> &coeff,
                          unsigned coeff_start,
                          const vector<DataType, N_Data> &ldata,
                          unsigned ldata_start,
                          const vector<DataType, N_Data> &rdata,
                          unsigned rdata_start,
                          unsigned uct_shift,
                          bool /*zero_acc*/,
                          const Acc &... acc)
    {
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        return impl_type::template run<Op>(coeff, coeff_start, ldata, ldata_start, rdata, rdata_start, uct_shift, acc...);
    }
};

template <typename CoeffType, unsigned CoeffBits, typename DataType, unsigned DataBits> struct sliding_mul_ch_accum_tag;
template <typename CoeffType,                     typename DataType>                    struct sliding_mul_ch_accum_tag<CoeffType,  8, DataType,  8> { using type = acc32; };
#if __AIE_ARCH__ == 21 || __AIE_ARCH__ == 22
template <typename CoeffType,                     typename DataType>                    struct sliding_mul_ch_accum_tag<CoeffType, 16, DataType, 16> { using type = acc64; };
#endif

template <typename CoeffType, typename DataType>
using sliding_mul_ch_accum_tag_t = typename sliding_mul_ch_accum_tag<CoeffType, type_bits_v<CoeffType>, DataType, type_bits_v<DataType>>::type;

template <unsigned Outputs, unsigned Channels, unsigned Points, int CoeffStep, int DataStepX, int DataStepY, unsigned AccumBits, unsigned CoeffTypeBits, unsigned DataTypeBits, typename CoeffType, typename DataType>
struct sliding_mul_ch_bits
{
    using impl_type = sliding_mul_ch_bits_impl<Outputs, Channels, Points, CoeffStep, DataStepX, DataStepY, AccumBits, CoeffTypeBits, DataTypeBits, CoeffType, DataType>;

    using  data_type = typename impl_type::data_type;
    using coeff_type = typename impl_type::coeff_type;
    using accum_type = typename impl_type::accum_type;

    static constexpr unsigned columns_per_mul = impl_type::columns_per_mul;
    static constexpr unsigned   lanes_per_mul = impl_type::lanes_per_mul;
    static constexpr unsigned         num_mul = impl_type::num_mul;
    static constexpr unsigned           lanes = Outputs * Channels;
    static constexpr unsigned          points = Points;
    static constexpr unsigned        channels = Channels;

    template <MulMacroOp Op, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static auto run(const vector<CoeffType, N_Coeff> &coeff,
                    unsigned coeff_start,
                    bool coeff_sign,
                    const vector<DataType, N_Data> &data,
                    unsigned data_start,
                    bool data_sign,
                    bool zero_acc,
                    const Acc &... acc)
    {
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        return impl_type::template run<Op>(coeff, coeff_start, coeff_sign, data, data_start, data_sign, zero_acc, acc...);
    }
};

#if __AIE_ARCH__ >= 20

// Distribute real and imaginary elements of a complex vector into two vectors
template <ComplexVector VecIn>
__aie_inline
static std::array<remove_complex_t<VecIn>, 2> unzip_complex(const VecIn &in) {
    using real_type = typename remove_complex<VecIn>::value_type;
    constexpr unsigned elems = VecIn::size();

    auto [lo, hi] = interleave_unzip<real_type, 2 * elems>::run(in.template cast_to<real_type>(), 1);
    return std::array{lo ,hi};
}

// Splits complex components and distribute accumulator halves into their own
// elements
template <Accum AccIn>
    requires(AccIn::value_class() == AccumClass::CInt)
__aie_inline
static std::array<remove_complex_t<AccIn>, 2> unzip_complex(const AccIn &in)
{
    using result_tag = typename remove_complex_t<AccIn>::value_type;
    constexpr unsigned elems = AccIn::size();

    // The interleave for unzip_complex and for combine_into_complex (zip) operations should be done with granularity
    // and operand types as close to each other as possible.
    // Using different granularities and/or different input/output types (e.g. complex acc) may interfere with the
    // patterns of the optimization pass, limiting the efficiency of consecutive operations (zip followed by unzip can
    // sometimes be optimized away).
    auto [lo, hi] = interleave_unzip<result_tag, 2 * elems>::run(in.template cast_to<result_tag>(), 1);
    return std::array{lo, hi};
}

// Combines two accumulators containing real and imaginary values respectively
// into a single complex accumulator
template <Accum AccIn>
    requires(AccIn::value_class() == AccumClass::Int)
__aie_inline
static auto combine_into_complex(const AccIn &real, const AccIn &imag) ->
    accum<accum_tag_t<AccumClass::CInt, AccIn::accum_min_bits()>, AccIn::size()>
{
    using result_tag = accum_tag_t<AccumClass::CInt, AccIn::accum_min_bits()>;
    using T          = typename AccIn::value_type;

    constexpr unsigned Elems       = AccIn::size();
#if __AIE_ARCH__ == 20
    constexpr unsigned NativeElems = 512 / AccIn::value_bits();

    accum<result_tag, Elems> result;
    utils::unroll_times<std::max(1u, AccIn::bits() / 512)>([&](unsigned idx){
        auto [lo, hi] = interleave_zip<T, NativeElems>::run(real.template grow_extract<NativeElems>(idx),
                                                            imag.template grow_extract<NativeElems>(idx),
                                                            1);
        result.insert(idx, concat(lo, hi).template cast_to<result_tag>()
                                         .template extract<std::min(Elems, NativeElems)>(0));
    });
#else
    auto [lo, hi] = interleave_zip<T, Elems>::run(real, imag, 1);
    auto result = lo.template grow<2 * Elems>(); // Must use grow+insert here, if we replace with two inserts, the
                                                 // compiler wont optimize correctly because the undef_*() type will not
                                                 // match the component types.
    result.insert(1, hi);
#endif
    return result.template cast_to<result_tag>();
}

template <Vector VecIn>
__aie_inline
static auto combine_into_complex(const VecIn &real, const VecIn &imag) ->
    vector<add_complex_t<typename VecIn::value_type>, VecIn::size()>
{
    using result_elem_t = add_complex_t<typename VecIn::value_type>;
    using input_elem_t  = typename VecIn::value_type;

    constexpr unsigned Elems       = VecIn::size();
#if __AIE_ARCH__ == 20
    constexpr unsigned NativeElems = 512 / VecIn::type_bits();

    vector<result_elem_t, Elems> result;
    utils::unroll_times<std::max(1u, VecIn::bits() / 512)>([&](unsigned idx){
        using zip = interleave_zip<input_elem_t, NativeElems>;
        auto [lo, hi] = zip::run(real.template grow_extract<NativeElems>(idx),
                                 imag.template grow_extract<NativeElems>(idx),
                                 1);
        result.insert(idx, concat(lo, hi).template cast_to<result_elem_t>()
                                         .template extract<std::min(Elems, NativeElems)>(0));
    });
#else
    using zip = interleave_zip<input_elem_t, Elems>;
    auto [lo, hi] = zip::run(real, imag, 1);
    auto result = lo.template grow<2 * Elems>(); // Must use grow+insert here, if we replace with two inserts, the
                                                 // compiler wont optimize correctly because the undef_*() type will not
                                                 // match the component types.
    result.insert(1, hi);
#endif
    return result.template cast_to<result_elem_t>();
}

template <typename T>
    requires((Vector<T> || Accum<T>) && !T::is_complex())
__aie_inline
static auto combine_into_complex(const std::array<T, 2> &t)
{
    return combine_into_complex(t.front(), t.back());
}

template <unsigned ElemsOut, Vector VecIn, size_t N>
__aie_inline
static inline std::array<vector<typename VecIn::value_type, ElemsOut>, N> grow_all(const std::array<VecIn, N> &in) {
    auto do_grow = [](const auto &... v) __aie_inline {
        return std::array{v.template grow_replicate<ElemsOut>()...};
    };

    return utils::apply_tuple(do_grow, in);
}

template <unsigned ElemsOut, Accum AccIn, size_t N>
__aie_inline
static inline std::array<accum<typename AccIn::value_type, ElemsOut>, N> grow_all(const std::array<AccIn, N> &in) {
    auto do_grow = [](const auto &... v) __aie_inline {
        return std::array{v.template grow<ElemsOut>()...};
    };

    return utils::apply_tuple(do_grow, in);
}

// AIE2 and later architectures have limited support for permute patterns. These specializations handle patterns in
// which the DataStep is 2. They are implemented by unzipping data first and then calling sliding_mul with DataStep = 1

template <unsigned OutElems, unsigned Points, int CoeffStep, unsigned AccumBits, unsigned CoeffTypeBits, unsigned DataTypeBits, typename CoeffType, typename DataType>
struct sliding_mul_bits_impl<OutElems, Points, CoeffStep, 2, 2, AccumBits, CoeffTypeBits, DataTypeBits, CoeffType, DataType>
{
    using impl_type = sliding_mul_bits_impl<OutElems, Points, CoeffStep, 1, 1, AccumBits, CoeffTypeBits, DataTypeBits, CoeffType, DataType>;

    using  data_type = typename impl_type::data_type;
    using coeff_type = typename impl_type::coeff_type;
    using accum_type = typename impl_type::accum_type;

    static constexpr unsigned columns_per_mul = impl_type::columns_per_mul;
    static constexpr unsigned   lanes_per_mul = impl_type::lanes_per_mul;
    static constexpr unsigned         num_mul = impl_type::num_mul;
    static constexpr unsigned           lanes = OutElems;
    static constexpr unsigned          points = Points;

    template <MulMacroOp Op, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static auto run(const vector<CoeffType, N_Coeff> &coeff,
                    unsigned coeff_start,
                    bool coeff_sign,
                    const vector<DataType, N_Data> &data,
                    unsigned data_start,
                    bool data_sign,
                    bool zero_acc,
                    const Acc &... acc)
    {
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        constexpr unsigned native_data_elems = native_vector_length_v<DataType>;
        constexpr unsigned data_elems = std::max(N_Data, native_data_elems);

        vector<data_type, native_data_elems> data2;
        vector<data_type, data_elems> tmp;

        data_start = data_start % data_elems;

        tmp = shuffle_down_rotate<data_type, data_elems>::run(data.template grow_replicate<data_elems>(),
                                                              data_start);

        data2 = filter<DataType, data_elems, FilterOp::Even>::run(tmp, 1).template grow_replicate<native_data_elems>();

        return impl_type::template run<Op>(coeff, coeff_start, coeff_sign, data2, 0, data_sign, zero_acc, acc...);
    }
};

template <unsigned OutElems, unsigned Points, unsigned AccumBits, unsigned CoeffTypeBits, unsigned DataTypeBits, typename CoeffType, typename DataType>
struct sliding_mul_bits_impl<OutElems, Points, 2, 1, 1, AccumBits, CoeffTypeBits, DataTypeBits, CoeffType, DataType>
{
    using impl_type = sliding_mul_bits_impl<OutElems, Points, 1, 1, 1, AccumBits, CoeffTypeBits, DataTypeBits, CoeffType, DataType>;

    using  data_type = typename impl_type::data_type;
    using coeff_type = typename impl_type::coeff_type;
    using accum_type = typename impl_type::accum_type;

    static constexpr unsigned columns_per_mul = impl_type::columns_per_mul;
    static constexpr unsigned   lanes_per_mul = impl_type::lanes_per_mul;
    static constexpr unsigned         num_mul = impl_type::num_mul;
    static constexpr unsigned           lanes = OutElems;
    static constexpr unsigned          points = Points;

    template <MulMacroOp Op, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static auto run(const vector<CoeffType, N_Coeff> &coeff,
            unsigned coeff_start,
            bool coeff_sign,
            const vector<DataType, N_Data> &data,
            unsigned data_start,
            bool data_sign,
            bool zero_acc,
            const Acc &... acc)
    {
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        constexpr unsigned native_coeff_elems = native_vector_length_v<CoeffType>;
        constexpr unsigned coeff_elems = std::max(N_Coeff, native_coeff_elems);

        vector<coeff_type, native_coeff_elems> coeff2;

        vector<coeff_type, coeff_elems> tmp;

        coeff_start = coeff_start % coeff_elems;

        tmp = shuffle_down_rotate<coeff_type, coeff_elems>::run(coeff.template grow_replicate<coeff_elems>(),
                coeff_start);

        coeff2 = filter<CoeffType, coeff_elems, FilterOp::Even>::run(tmp, 1).template grow_replicate<native_coeff_elems>();

        return impl_type::template run<Op>(coeff2, 0, coeff_sign, data, data_start, data_sign, zero_acc, acc...);
    }
};

template <unsigned Outputs, unsigned Channels, unsigned Points, int CoeffStep, unsigned AccumBits, unsigned CoeffTypeBits, unsigned DataTypeBits, typename CoeffType, typename DataType>
struct sliding_mul_ch_bits_impl<Outputs, Channels, Points, CoeffStep, 2, 2, AccumBits, CoeffTypeBits, DataTypeBits, CoeffType, DataType>
{
    using  impl_type = sliding_mul_ch_bits_impl<Outputs, Channels, Points, CoeffStep, 1, 1, AccumBits, CoeffTypeBits, DataTypeBits, CoeffType, DataType>;

    using  data_type = typename impl_type::data_type;
    using coeff_type = typename impl_type::coeff_type;
    using accum_type = typename impl_type::accum_type;

    static constexpr unsigned columns_per_mul = impl_type::columns_per_mul;
    static constexpr unsigned   lanes_per_mul = impl_type::lanes_per_mul;
    static constexpr unsigned         num_mul = impl_type::num_mul;
    static constexpr unsigned           lanes = Outputs * Channels;
    static constexpr unsigned          points = Points;
    static constexpr unsigned        channels = Channels;

    template <MulMacroOp Op, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static auto run(const vector<CoeffType, N_Coeff> &coeff,
                    unsigned coeff_start,
                    bool coeff_sign,
                    const vector<DataType, N_Data> &data,
                    unsigned data_start,
                    bool data_sign,
                    bool zero_acc,
                    const Acc &... acc)
    {
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        constexpr unsigned native_data_elems = native_vector_length_v<DataType>;
        constexpr unsigned data_elems = std::max(N_Data, native_data_elems);

        vector<data_type, native_data_elems> data2;
        vector<data_type, data_elems> tmp;

        data_start = data_start % data_elems;

        tmp = shuffle_down_rotate<data_type, data_elems>::run(data.template grow_replicate<data_elems>(),
                                                              data_start * Channels);

        data2 = filter<DataType, data_elems, FilterOp::Even>::run(tmp, Channels).template grow_replicate<native_data_elems>();

        return impl_type::template run<Op>(coeff, coeff_start, coeff_sign, data2, 0, data_sign, zero_acc, acc...);
    }
};

template <unsigned Outputs, unsigned Channels, unsigned Points, unsigned AccumBits, unsigned CoeffTypeBits, unsigned DataTypeBits, typename CoeffType, typename DataType>
struct sliding_mul_ch_bits_impl<Outputs, Channels, Points, 2, 1, 1, AccumBits, CoeffTypeBits, DataTypeBits, CoeffType, DataType>
{
    using  impl_type = sliding_mul_ch_bits_impl<Outputs, Channels, Points, 1, 1, 1, AccumBits, CoeffTypeBits, DataTypeBits, CoeffType, DataType>;

    using  data_type = typename impl_type::data_type;
    using coeff_type = typename impl_type::coeff_type;
    using accum_type = typename impl_type::accum_type;

    static constexpr unsigned columns_per_mul = impl_type::columns_per_mul;
    static constexpr unsigned   lanes_per_mul = impl_type::lanes_per_mul;
    static constexpr unsigned         num_mul = impl_type::num_mul;
    static constexpr unsigned           lanes = Outputs * Channels;
    static constexpr unsigned          points = Points;
    static constexpr unsigned        channels = Channels;

    template <MulMacroOp Op, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static auto run(const vector<CoeffType, N_Coeff> &coeff,
            unsigned coeff_start,
            bool coeff_sign,
            const vector<DataType, N_Data> &data,
            unsigned data_start,
            bool data_sign,
            bool zero_acc,
            const Acc &... acc)
    {
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        constexpr unsigned native_coeff_elems = native_vector_length_v<DataType>;
        constexpr unsigned coeff_elems = std::max(N_Coeff, native_coeff_elems);

        vector<coeff_type, native_coeff_elems> coeff2;

        vector<coeff_type, coeff_elems> tmp;

        coeff_start = coeff_start % coeff_elems;

        tmp = shuffle_down_rotate<coeff_type, coeff_elems>::run(coeff.template grow_replicate<coeff_elems>(),
                coeff_start * Channels);

        coeff2 = filter<CoeffType, coeff_elems, FilterOp::Even>::run(tmp, Channels).template grow_replicate<native_coeff_elems>();

        return impl_type::template run<Op>(coeff2, 0, coeff_sign, data, data_start, data_sign, zero_acc, acc...);
    }
};

#endif

template <MulMacroOp MulOp, unsigned AccumBits, typename T1, typename T2>
using             mul  = mul_bits<MulOp, AccumBits, type_bits_v<T1>, T1, type_bits_v<T2>, T2>;

template <MulMacroOp MulOp, unsigned AccumBits, typename T1, typename T2>
using      mul_maxmin = mul_maxmin_bits<MulOp, AccumBits, type_bits_v<T1>, T1, type_bits_v<T2>, T2>;

template <unsigned AccumBits, typename T, unsigned Elems>
using      mul_reduce = mul_reduce_bits<AccumBits, type_bits_v<T>, T, Elems>;

template <unsigned OutElems, unsigned Points, int CoeffStep, int DataStepX, int DataStepY, unsigned AccumBits, typename CoeffType, typename DataType>
using     sliding_mul = sliding_mul_bits    <OutElems, Points, CoeffStep, DataStepX, DataStepY, AccumBits, type_bits_v<CoeffType>, type_bits_v<DataType>, CoeffType, DataType>;

template <unsigned OutElems, unsigned Points, int CoeffStep, int DataStepX, int DataStepY, unsigned AccumBits, typename CoeffType, typename DataType>
using sliding_mul_sym = sliding_mul_sym_bits<OutElems, Points, CoeffStep, DataStepX, DataStepY, AccumBits, type_bits_v<CoeffType>, type_bits_v<DataType>, CoeffType, DataType>;

template <unsigned OutElems, unsigned Points, int CoeffStep, int DataStep, unsigned AccumBits, typename CoeffType, typename DataType>
using sliding_mul_sym_uct = sliding_mul_sym_uct_bits<OutElems, Points, CoeffStep, DataStep, AccumBits, type_bits_v<CoeffType>, type_bits_v<DataType>, CoeffType, DataType>;

template <unsigned Outputs, unsigned Channels, unsigned Points, int CoeffStep, int DataStepX, int DataStepY, unsigned AccumBits, typename CoeffType, typename DataType>
using   sliding_mul_ch = sliding_mul_ch_bits  <Outputs, Channels, Points, CoeffStep, DataStepX, DataStepY, AccumBits, type_bits_v<CoeffType>, type_bits_v<DataType>, CoeffType, DataType>;

template <unsigned N, typename T>
__aie_inline
auto mul_vector_or_scalar(T scalar)
{
#if __AIE_ARCH__ == 22
    // AIE2PS adds support for automatic broadcast of operand B in mul intrinsics to reduce vector register pressure
    return scalar;
#else
    return broadcast<T, N>::run(scalar);
#endif
}

template <typename ImplType>
concept ComplexEmulationWithUnzip = requires {
    typename ImplType::accum_type;       // The complex accumulator result
    typename ImplType::partial_sum_type; // The accumulator result split in real and imaginary parts

};

template <unsigned Lanes, unsigned Points, int CoeffStep, int DataStepX, int DataStepY, typename CoeffType, typename DataType, AccumElemBaseType AccumTag>
class partial_sliding_mul {
public:
    using accum_type      = accum<AccumTag, Lanes>;
    using real_accum_type = remove_complex_t<accum_type>;

    static constexpr bool split_complex_components()
    {
        return ComplexEmulationWithUnzip<partial_sliding_mul::impl_type>;
    }

    partial_sliding_mul() = default;
    partial_sliding_mul(const partial_sliding_mul &) = default;

    __aie_inline
    explicit partial_sliding_mul(const accum_type &acc)
        : sum()
        , zero_acc(false)
    {
        if constexpr (split_complex_components()) {
            sum = unzip_complex(acc);
        }
        else {
            sum = acc;
        }
    }

    __aie_inline
    explicit partial_sliding_mul(const std::array<real_accum_type, 2> &acc)
        requires(accum_type::is_complex())
        : sum()
        , zero_acc(false)
    {
        if constexpr (split_complex_components()) {
            sum = acc;
        }
        else {
            sum = combine_into_complex(acc);
        }
    }

    template <unsigned N_Coeff, unsigned N_Data>
    __aie_inline
    void mul(const vector<CoeffType, N_Coeff> &coeff,
             unsigned coeff_start,
             const vector<DataType, N_Data>  &data,
             unsigned data_start)
    {
        constexpr MulMacroOp Op = MulMacroOp::Mul;
        if constexpr(split_complex_components()) {
            // Must disambiguate return type when input accumulator is not passed as argument
            std::array<real_accum_type, 0> in;
            sum = impl_type::template run<Op>(coeff, coeff_start, coeff.is_signed(),
                                              data, data_start, data.is_signed(),
                                              true, in);
        }
        else {
            sum = impl_type::template run<Op>(coeff, coeff_start, coeff.is_signed(),
                                              data, data_start, data.is_signed(),
                                              true);
        }
        zero_acc = false;
    }


    template <unsigned N_Coeff, unsigned N_Data>
    __aie_inline
    void mac(const vector<CoeffType, N_Coeff> &coeff,
             unsigned coeff_start,
             const vector<DataType, N_Data>  &data,
             unsigned data_start)
    {
        constexpr MulMacroOp Op = MulMacroOp::Add_Mul;
        sum = impl_type::template run<Op>(coeff, coeff_start, coeff.is_signed(),
                                          data, data_start, data.is_signed(),
                                          zero_acc,
                                          sum);
        zero_acc = false;
    }

    accum_type to_accum() const
    {
        if constexpr (split_complex_components()) {
            return combine_into_complex(sum);
        } else {
            return sum;
        }
    }

    std::array<real_accum_type, 2> to_accum_components() const
        requires(accum_type::is_complex())
    {
        if constexpr (split_complex_components()) {
            return sum;
        } else {
            return unzip_complex(sum);
        }
    }

    template <ElemBaseType T>
    vector<T, Lanes> to_vector(int shift) const
        requires(accum_type::is_complex() == vector<T, Lanes>::is_complex())
    {
        // It is more efficient to perform SRS first, and then shuffle to get the desired result representation
        if constexpr (split_complex_components()) {
            using real_t = remove_complex_t<T>;
            return combine_into_complex(to_vector_components<real_t>(shift));
        } else {
            return to_accum().template to_vector<T>(shift);
        }
    }

    template <ElemBaseType T>
    std::array<vector<T, Lanes>, 2> to_vector_components(int shift) const
        requires(accum_type::is_complex() && !vector<T, Lanes>::is_complex())
    {
        // It is more efficient to perform SRS first, and then shuffle to get the desired result representation
        if constexpr (split_complex_components()) {
            return utils::apply_tuple(
                        [shift] (auto &&... cs) __aie_inline {
                            return std::array{cs.template to_vector<T>(shift)...};
                        },
                        to_accum_components());
        } else {
            using complex_t = add_complex_t<T>;
            return unzip_complex(to_vector<complex_t>(shift));
        }
    }
private:
    static constexpr unsigned accum_bits = detail::to_native_accum_bits_for_mul_types_tag<CoeffType, DataType, AccumTag>();

    using impl_type = sliding_mul_bits_impl<Lanes, Points, CoeffStep, DataStepX, DataStepY, accum_bits, type_bits_v<CoeffType>, type_bits_v<DataType>, CoeffType, DataType>;

    std::conditional_t<split_complex_components(), std::array<real_accum_type, 2>, accum_type> sum;
    bool zero_acc = true;
};

} // aie::detail

#if __AIE_ARCH__ == 10

#include "aie1/mul.hpp"
#include "aie1/mul_reduce.hpp"
#include "aie1/sliding_mul.hpp"
#include "aie1/sliding_mul_sym.hpp"

#elif __AIE_ARCH__ == 20

#include "aie2/mul.hpp"
#include "aie2/mul_reduce.hpp"
#include "aie2/sliding_mul.hpp"

#elif __AIE_ARCH__ == 21 || __AIE_ARCH__ == 22

#include "aie2p/mul.hpp"
#include "aie2/mul_reduce.hpp"
#include "aie2p/sliding_mul.hpp"

#if __AIE_ARCH__ == 22
#include "aie2ps/mul.hpp"
#endif

#endif

#endif
