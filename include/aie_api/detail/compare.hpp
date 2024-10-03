// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_COMPARE__HPP__
#define __AIE_API_DETAIL_COMPARE__HPP__

#include "../mask.hpp"
#include "vector.hpp"

namespace aie::detail {

enum class CmpOp
{
    LT,
    GE,
    EQ,
    NEQ,
    LE,
    GT
};

template <CmpOp op>
constexpr bool is_compare_commutative() {
    return op == CmpOp::EQ || op == CmpOp::NEQ;
}

template <CmpOp op>
constexpr CmpOp with_swapped_operands() {
    if constexpr (op == CmpOp::LT)
        return CmpOp::GT;
    else if constexpr (op == CmpOp::GE)
        return CmpOp::LE;
    else if constexpr (op == CmpOp::LE)
        return CmpOp::GE;
    else if constexpr (op == CmpOp::GT)
        return CmpOp::LT;
    else if (is_compare_commutative<op>())
        return op;
}

template <typename T, unsigned Elems>
struct contains
{
#ifdef __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
    using vector_type = vector<T, Elems>;

    static bool run(T a, const vector_type &v)
    {
        for (unsigned i = 0; i < Elems; ++i) {
            if (v[i] == a)
                return true;
        }

        return false;
    }
#endif
};

template <unsigned TypeBits, typename T, unsigned Elems>
struct equal_bits_impl
{
#ifdef __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
    using vector_type = vector<T, Elems>;

    static bool run(const vector_type &v1, const vector_type &v2)
    {
        for (unsigned i = 0; i < Elems; ++i) {
            if (v1[i] != v2[i])
                return false;
        }

        return true;
    }

    static bool run(T a, const vector_type &v)
    {
        for (unsigned i = 0; i < Elems; ++i) {
            if (a != v[i])
                return false;
        }

        return true;
    }


    static bool run(const vector_type &v, T a)
    {
        for (unsigned i = 0; i < Elems; ++i) {
            if (v[i] != a)
                return false;
        }

        return true;
    }
#endif
};

template <CmpOp Op, unsigned TypeBits, typename T, unsigned Elems>
struct cmp_bits_impl
{
#ifdef __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
    using vector_type = vector<T, Elems>;
    using   mask_type = mask<Elems>;

    static constexpr auto get_op()
    {
        if      constexpr (Op == CmpOp::LT)  return [](auto a, auto b) { return a  < b;  };
        else if constexpr (Op == CmpOp::GE)  return [](auto a, auto b) { return a >= b; };
        else if constexpr (Op == CmpOp::EQ)  return [](auto a, auto b) { return a == b; };
        else if constexpr (Op == CmpOp::NEQ) return [](auto a, auto b) { return a != b; };
    }

    static mask_type run(const vector_type &v1, const vector_type &v2)
    {
        constexpr auto op = get_op();
        mask_type ret;

        for (unsigned i = 0; i < Elems; ++i) {
            if (op(v1[i], v2[i]))
                ret.set(i);
        }

        return ret;
    }

    static mask_type run(T a, const vector_type &v)
    {
        constexpr auto op = get_op();
        mask_type ret;

        for (unsigned i = 0; i < Elems; ++i) {
            if (op(v[i], a))
                ret.set(i);
        }

        return ret;
    }

    static mask_type run(const vector_type &v, T a)
    {
        constexpr auto op = get_op();
        mask_type ret;

        for (unsigned i = 0; i < Elems; ++i) {
            if (a, op(v[i]))
                ret.set(i);
        }

        return ret;
    }
#endif
};

// Default implementation compares against a vector of zeros.
// Specializations can take advantage of hardware support for comparisons against zero.
template <CmpOp Op, unsigned TypeBits, typename T, unsigned Elems>
struct cmp_zero_bits_impl
{
    using vector_type = vector<T, Elems>;
    using   mask_type = mask<Elems>;

    static mask_type run(const vector_type &v)
    {
        return cmp_bits_impl<Op, TypeBits, T, Elems>::run(v, zeros<T, Elems>::run());
    }
};

template <CmpOp Op, typename T, unsigned Elems>
struct cmp_impl
{
    using vector_type = vector<T, Elems>;
    using mask_type = mask<Elems>;

    __aie_inline
    static mask_type run(const vector_type &v1, const vector_type &v2)
    {
        return cmp_bits_impl<Op, type_bits_v<T>, T, Elems>::run(v1, v2);
    }

    __aie_inline
    static mask_type run(const T &a, const vector_type &v)
    {
#if __AIE_ARCH__ == 20
        // zero-comparison assumes zero is on the right hand side,
        // so we need to replace the compare operation with an equivalent
        if (chess_manifest(a == scalar_zero())) {
            constexpr CmpOp equivalent = with_swapped_operands<Op>();
            return cmp_zero_bits_impl<equivalent, type_bits_v<T>, T, Elems>::run(v);
        }
#endif

        return cmp_bits_impl<Op, type_bits_v<T>, T, Elems>::run(a, v);
    }

    __aie_inline
    static mask_type run(const vector_type &v, const T &a)
    {
#if __AIE_ARCH__ == 20
        if (chess_manifest(a == scalar_zero()))
            return cmp_zero_bits_impl<Op, type_bits_v<T>, T, Elems>::run(v);
#endif

        return cmp_bits_impl<Op, type_bits_v<T>, T, Elems>::run(v, a);
    }

    template <unsigned Elems2>
    __aie_inline
    static mask_type run(vector_elem_const_ref<T, Elems2> a, const vector_type &v)
    {
#if __AIE_ARCH__ == 20
        // zero-comparison assumes zero is on the right hand side,
        // so we need to replace the compare operation with an equivalent
        if (chess_manifest(a == scalar_zero())) {
            constexpr CmpOp equivalent = with_swapped_operands<Op>();
            return cmp_zero_bits_impl<equivalent, type_bits_v<T>, T, Elems>::run(v);
        }
#endif

        return cmp_bits_impl<Op, type_bits_v<T>, T, Elems>::run(a, v);
    }

    template <unsigned Elems2>
    __aie_inline
    static mask_type run(const vector_type &v, vector_elem_const_ref<T, Elems2> a)
    {
#if __AIE_ARCH__ == 20
        if (chess_manifest(a == scalar_zero()))
            return cmp_zero_bits_impl<Op, type_bits_v<T>, T, Elems>::run(v);
#endif

        return cmp_bits_impl<Op, type_bits_v<T>, T, Elems>::run(v, a);
    }

    static constexpr T scalar_zero()
    {
        if constexpr (is_complex_v<T>)
            return T(0, 0);
        else
            return T(0);
    }
};

template <typename T, unsigned Elems>
struct equal
{
    using vector_type = vector<T, Elems>;

    __aie_inline
    static bool run(const vector_type &v1, const vector_type &v2)
    {
        return equal_bits_impl<type_bits_v<T>, T, Elems>::run(v1, v2);
    }

    __aie_inline
    static bool run(const T &a, const vector_type &v)
    {
        return equal_bits_impl<type_bits_v<T>, T, Elems>::run(a, v);
    }

    template <unsigned Elems2>
    __aie_inline
    static bool run(vector_elem_const_ref<T, Elems2> a, const vector_type &v)
    {
        return equal_bits_impl<type_bits_v<T>, T, Elems>::run(a, v);
    }

    __aie_inline
    static bool run(const vector_type &v, const T &a)
    {
        return equal_bits_impl<type_bits_v<T>, T, Elems>::run(v, a);
    }

    template <unsigned Elems2>
    __aie_inline
    static bool run(const vector_type &v, vector_elem_const_ref<T, Elems2> a)
    {
        return equal_bits_impl<type_bits_v<T>, T, Elems>::run(v, a);
    }
};

template <typename T, unsigned Elems>
struct not_equal
{
    using vector_type = vector<T, Elems>;

    __aie_inline
    static bool run(const vector_type &v1, const vector_type &v2)
    {
        return !equal<T, Elems>::run(v1, v2);
    }

    __aie_inline
    static bool run(const T &a, const vector_type &v)
    {
        return !equal<T, Elems>::run(a, v);
    }

    template <unsigned Elems2>
    __aie_inline
    static bool run(vector_elem_const_ref<T, Elems2> a, const vector_type &v)
    {
        return !equal<T, Elems>::run(a, v);
    }

    __aie_inline
    static bool run(const vector_type &v, const T &a)
    {
        return !equal<T, Elems>::run(v, a);
    }

    template <unsigned Elems2>
    __aie_inline
    static bool run(const vector_type &v, vector_elem_const_ref<T, Elems2> a)
    {
        return !equal<T, Elems>::run(v, a);
    }
};

template <typename T, unsigned Elems>
using lt = cmp_impl<CmpOp::LT, T, Elems>;

template <typename T, unsigned Elems>
using ge = cmp_impl<CmpOp::GE, T, Elems>;

template <typename T, unsigned Elems>
using eq = cmp_impl<CmpOp::EQ, T, Elems>;

template <typename T, unsigned Elems>
using neq = cmp_impl<CmpOp::NEQ, T, Elems>;

}

#if __AIE_ARCH__ == 10

#include "aie1/compare.hpp"

#elif __AIE_ARCH__ == 20

#include "aie2/compare.hpp"

#endif

#endif
