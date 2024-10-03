// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_MAX_MIN__HPP__
#define __AIE_API_DETAIL_MAX_MIN__HPP__

#include <algorithm>

#include "vector.hpp"
#include "../mask.hpp"

namespace aie::detail {

enum class MaxMinOperation
{
    Max,
    Min,
    MaxCmp,
    MinCmp,
    MaxDiff,
};

template <unsigned TypeBits, typename T, unsigned Elems, MaxMinOperation Op>
struct max_min_bits_impl
{
#ifdef __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v1, const vector_type &v2)
    {
        vector_type ret;

        for (unsigned i = 0; i < Elems; ++i) {
            if constexpr (Op == MaxMinOperation::Max || Op == MaxMinOperation::MaxCmp)
                ret[i] = std::max(v1[i], v2[i]);
            else if constexpr (Op == MaxMinOperation::Min || Op == MaxMinOperation::MinCmp)
                ret[i] = std::min(v1[i], v2[i]);
            else if constexpr (Op == MaxMinOperation::MaxDiff)
                ret[i] = std::max(v1[i] - v2[i], 0);
        }

        return ret;
    }

    static vector_type run(T a, const vector_type &v)
    {
        vector_type ret;

        for (unsigned i = 0; i < Elems; ++i) {
            if constexpr (Op == MaxMinOperation::Max)
                ret[i] = std::max(a, v[i]);
            else if constexpr (Op == MaxMinOperation::Min)
                ret[i] = std::min(a, v[i]);
            else if constexpr (Op == MaxMinOperation::MaxDiff)
                ret[i] = std::max(a - v[i], 0);
        }

        return ret;
    }
#endif
};

template <unsigned TypeBits, typename T, unsigned Elems, MaxMinOperation Op>
struct max_min_cmp_bits_impl
{
#ifdef __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
    using vector_type = vector<T, Elems>;

    static auto run(const vector_type &v1, const vector_type &v2)
    {
        vector_type ret;
        mask<Elems> m;

        for (unsigned i = 0; i < Elems; ++i) {
            if constexpr (Op == MaxMinOperation::MaxCmp) {
                if (v1[i] < v2[i]) {
                    ret[i] = v2[i];
                    m.set(i);
                }
                else {
                    ret[i] = v1[i];
                }
            }
            else if constexpr (Op == MaxMinOperation::MinCmp) {
                if (v1[i] < v2[i]) {
                    ret[i] = v1[i];
                }
                else {
                    ret[i] = v2[i];
                    m.set(i);
                }
            }
        }

        return std::make_tuple(ret, m);
    }
#endif
};

template <unsigned TypeBits, typename T, unsigned Elems, MaxMinOperation Op>
struct max_min_bits_reduce_impl
{
#ifdef __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
    using vector_type = vector<T, Elems>;

    static T run(const vector_type &v)
    {
        T ret = v[0];

        for (unsigned i = 1; i < Elems; ++i) {
            if constexpr (Op == MaxMinOperation::Max)
                ret = std::max((T)v[i], ret);
            else if constexpr (Op == MaxMinOperation::Min)
                ret = std::min((T)v[i], ret);
        }

        return ret;
    }
#endif
};

template <unsigned TypeBits, typename T, unsigned Elems, MaxMinOperation Op>
struct max_min_bits
{
    using vector_type = vector<T, Elems>;

    __aie_inline
    static vector_type run(const vector_type &v1, const vector_type &v2)
    {
        return max_min_bits_impl<TypeBits, T, Elems, Op>::run(v1, v2);
    }

    __aie_inline
    static vector_type run(const T &a, const vector_type &v)
    {
        return max_min_bits_impl<TypeBits, T, Elems, Op>::run(a, v);
    }

    __aie_inline
    static vector_type run(const vector_type &v, const T &a)
    {
        return max_min_bits_impl<TypeBits, T, Elems, Op>::run(v, a);
    }

    template <unsigned Elems2>
    __aie_inline
    static vector_type run(vector_elem_const_ref<T, Elems2> a, const vector_type &v)
    {
        return max_min_bits_impl<TypeBits, T, Elems, Op>::run(a, v);
    }

    template <unsigned Elems2>
    __aie_inline
    static vector_type run(const vector_type &v, vector_elem_const_ref<T, Elems2> a)
    {
        return max_min_bits_impl<TypeBits, T, Elems, Op>::run(v, a);
    }

    __aie_inline
    static vector_type run(const vector_type &v1, const vector_type &v2, bool sign)
    {
        return max_min_bits_impl<TypeBits, T, Elems, Op>::run(v1, v2, sign);
    }

    __aie_inline
    static vector_type run(const T &a, const vector_type &v, bool sign)
    {
        return max_min_bits_impl<TypeBits, T, Elems, Op>::run(a, v, sign);
    }

    __aie_inline
    static vector_type run(const vector_type &v, const T &a, bool sign)
    {
        return max_min_bits_impl<TypeBits, T, Elems, Op>::run(v, a, sign);
    }

    template <unsigned Elems2>
    __aie_inline
    static vector_type run(vector_elem_const_ref<T, Elems2> a, const vector_type &v, bool sign)
    {
        return max_min_bits_impl<TypeBits, T, Elems, Op>::run(a, v, sign);
    }

    template <unsigned Elems2>
    __aie_inline
    static vector_type run(const vector_type &v, vector_elem_const_ref<T, Elems2> a, bool sign)
    {
        return max_min_bits_impl<TypeBits, T, Elems, Op>::run(v, a, sign);
    }
};

template <unsigned TypeBits, typename T, unsigned Elems, MaxMinOperation Op>
struct max_min_cmp_bits
{
    using vector_type = vector<T, Elems>;

    __aie_inline
    static auto run(const vector_type &v1, const vector_type &v2)
    {
        return max_min_cmp_bits_impl<TypeBits, T, Elems, Op>::run(v1, v2);
    }
};

template <unsigned TypeBits, typename T, unsigned Elems, MaxMinOperation Op>
struct max_min_reduce_bits
{
    using vector_type = vector<T, Elems>;

    __aie_inline
    static T run(const vector_type &v)
    {
        return max_min_bits_reduce_impl<TypeBits, T, Elems, Op>::run(v);
    }
};

template <typename T, unsigned Elems>
using max = max_min_bits<type_bits_v<T>, T, Elems, MaxMinOperation::Max>;

template <typename T, unsigned Elems>
using min = max_min_bits<type_bits_v<T>, T, Elems, MaxMinOperation::Min>;

template <typename T, unsigned Elems>
using max_cmp = max_min_cmp_bits<type_bits_v<T>, T, Elems, MaxMinOperation::MaxCmp>;

template <typename T, unsigned Elems>
using min_cmp = max_min_cmp_bits<type_bits_v<T>, T, Elems, MaxMinOperation::MinCmp>;

template <typename T, unsigned Elems>
using maxdiff = max_min_bits<type_bits_v<T>, T, Elems, MaxMinOperation::MaxDiff>;

template <typename T, unsigned Elems>
using max_reduce = max_min_reduce_bits<type_bits_v<T>, T, Elems, MaxMinOperation::Max>;

template <typename T, unsigned Elems>
using min_reduce = max_min_reduce_bits<type_bits_v<T>, T, Elems, MaxMinOperation::Min>;

}

#if __AIE_ARCH__ == 10

#include "aie1/max_min.hpp"
#include "aie1/max_min_cmp.hpp"
#include "aie1/max_min_reduce.hpp"

#elif __AIE_ARCH__ == 20

#include "aie2/max_min.hpp"
#include "aie2/max_min_cmp.hpp"
#include "aie2/max_min_reduce.hpp"

#endif

#endif
