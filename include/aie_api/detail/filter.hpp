// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_FILTER__HPP__
#define __AIE_API_DETAIL_FILTER__HPP__

#include "vector.hpp"

namespace aie::detail {

// Tag type used to disambiguate between the types of filtering when
// constructing a FilterMode object.
struct filter_odd_tag {};
struct filter_even_tag {};

template <unsigned TypeBits, unsigned Elems>
class filter_mode;

enum class FilterOp {
    Odd,
    Even,
    Dynamic
};

template <unsigned TypeBits, typename T, unsigned Elems, FilterOp Op>
struct filter_bits_impl
{
#ifdef __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
    using        vector_type = vector<T, Elems>;
    using return_vector_type = vector<T, Elems / 2>;

    // TODO: CRVO-9533 default scalar implementation for dynamic filters
    static_assert(Op != FilterOp::Dynamic, "Not implemented");

    static return_vector_type run(const vector_type &v, unsigned n)
    {
        return_vector_type ret;

        const unsigned start = [=]() constexpr {
            if      constexpr (Op == FilterOp::Odd)
                return n;
            else if constexpr (Op == FilterOp::Even)
                return 0;
        }();

        unsigned out = 0;

        for (unsigned i = start; i < Elems; i += n) {
            for (unsigned j = 0; j < n; ++j)
                ret[out++] = v[i++];
        }

        return ret;
    }
#endif
};

template <unsigned TypeBits, typename T, unsigned Elems, FilterOp Op>
struct filter_bits
{
    using        vector_type = vector<T, Elems>;
    using return_vector_type = vector<T, Elems / 2>;

    static return_vector_type run(const vector_type &v, unsigned n)
    {
        return filter_bits_impl<TypeBits, T, Elems, Op>::run(v, n);
    }
};

template <unsigned TypeBits, typename T, unsigned Elems>
struct filter_bits<TypeBits, T, Elems, FilterOp::Dynamic>
{
    using        vector_type = vector<T, Elems>;
    using return_vector_type = vector<T, Elems / 2>;

    static return_vector_type run(const vector_type &v, const filter_mode<TypeBits, Elems> &mode)
    {
        return filter_bits_impl<TypeBits, T, Elems, FilterOp::Dynamic>::run(v, mode);
    }
};

template <typename T, unsigned Elems, FilterOp Op>
using filter = filter_bits<type_bits_v<T>, T, Elems, Op>;

}

#if __AIE_ARCH__ == 10

#include "aie1/filter.hpp"

#elif __AIE_ARCH__ == 20 || __AIE_ARCH__ == 21

#include "aie2/filter.hpp"

#endif

#endif
