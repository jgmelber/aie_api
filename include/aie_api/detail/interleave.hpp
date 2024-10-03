// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_INTERLEAVE__HPP__
#define __AIE_API_DETAIL_INTERLEAVE__HPP__

#include "reverse.hpp"
#include "vector.hpp"

namespace aie::detail {

// Tag type used to disambiguate between the types of interleave when
// constructing a shuffle_mode object
struct shuffle_zip_tag {};
struct shuffle_unzip_tag {};

template <unsigned TypeBits, unsigned Elems>
class shuffle_mode;

template <typename T, unsigned Elems>
static std::pair<vector<T, Elems>, vector<T, Elems>> zip_serial(const vector<T, Elems> &a, const vector<T, Elems> &b, unsigned step)
{
    vector<T, Elems> ret1, ret2;

    if (step == Elems)
        return std::make_pair(a, b);

    unsigned out_idx = 0;
    unsigned in1_idx = 0;
    unsigned in2_idx = 0;

    for (unsigned i = 0; i < (Elems / step / 2); ++i) {
        for (unsigned s = 0; s < step; ++s)
            ret1[out_idx++] = a[in1_idx++];

        for (unsigned s = 0; s < step; ++s)
            ret1[out_idx++] = b[in2_idx++];
    }

    out_idx = 0;

    for (unsigned i = 0; i < (Elems / step / 2); ++i) {
        for (unsigned s = 0; s < step; ++s)
            ret2[out_idx++] = a[in1_idx++];

        for (unsigned s = 0; s < step; ++s)
            ret2[out_idx++] = b[in2_idx++];
    }

    return std::make_pair(ret1, ret2);
}

template <typename T, unsigned Elems>
static std::pair<vector<T, Elems>, vector<T, Elems>> unzip_serial(const vector<T, Elems> &a, const vector<T, Elems> &b, unsigned step)
{
    vector<T, Elems> ret1, ret2;

    if (step == Elems)
        return std::make_pair(a, b);

    unsigned out1_idx = 0;
    unsigned out2_idx = 0;
    unsigned in_idx = 0;

    for (unsigned i = 0; i < (Elems / step / 2); ++i) {
        for (unsigned s = 0; s < step; ++s)
            ret1[out1_idx++] = a[in_idx++];

        for (unsigned s = 0; s < step; ++s)
            ret2[out2_idx++] = a[in_idx++];
    }

    in_idx = 0;

    for (unsigned i = 0; i < (Elems / step / 2); ++i) {
        for (unsigned s = 0; s < step; ++s)
            ret1[out1_idx++] = b[in_idx++];

        for (unsigned s = 0; s < step; ++s)
            ret2[out2_idx++] = b[in_idx++];
    }

    return std::make_pair(ret1, ret2);
}

template <unsigned TypeBits, typename T, unsigned Elems>
struct interleave_bits_zip
{
#ifdef __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
    using vector_type = vector<T, Elems>;

    static std::pair<vector_type, vector_type> run(const vector_type &v1, const vector_type &v2, unsigned step)
    {
        return zip_serial(v1, v2, step);
    }
#endif
};

template <unsigned TypeBits, typename T, unsigned Elems>
struct interleave_bits_unzip
{
#ifdef __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
    using vector_type = vector<T, Elems>;

    static std::pair<vector_type, vector_type> run(const vector_type &v1, const vector_type &v2, unsigned step)
    {
        return unzip_serial(v1, v2, step);
    }
#endif
};

// TODO: CRVO-9533 scalar implementation for dynamic interleave
template <unsigned TypeBits, typename T, unsigned Elems>
struct interleave_bits;

template <unsigned TypeBits, typename T, unsigned Elems>
struct interleave_bits_custom
{
#ifdef __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
    using vector_type = vector<T, Elems>;

    template <typename... Select>
    static std::pair<vector_type, vector_type> run(const vector_type &a, const vector_type &b, Select... select)
    {
        vector_type ret1, ret2;

        static_assert(sizeof...(Select) == Elems * 2);

        const std::array<unsigned, sizeof...(Select)> idxs{ unsigned(select)... };

        for (unsigned i = 0; i < Elems; ++i) {
            unsigned idx = idxs[i];

            if (idx < Elems)
                ret1[i] = a[idx];
            else
                ret1[i] = b[idx - Elems];
        }

        for (unsigned i = 0; i < Elems; ++i) {
            unsigned idx = idxs[i + Elems];

            if (idx < Elems)
                ret2[i] = a[idx];
            else
                ret2[i] = b[idx - Elems];
        }

        return std::make_pair(ret1, ret2);
    }
#endif
};

template <unsigned TypeBits, typename T, unsigned Elems, unsigned... Select>
struct interleave_bits_custom_static
{
#ifdef __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
    using vector_type = vector<T, Elems>;

    static_assert(sizeof...(Select) == Elems * 2);

    static std::pair<vector_type, vector_type> run(const vector_type &a, const vector_type &b)
    {
        return interleave_bits_custom<TypeBits, T, Elems>::run(a, b, Select...);
    }
#endif
};

template <unsigned TypeBits, typename T, unsigned Elems>
struct interleave_bits_butterfly
{
#ifdef __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
    using vector_type = vector<T, Elems>;

    static std::pair<vector_type, vector_type> run(const vector_type &a, const vector_type &b, unsigned radix)
    {
        std::pair<vector_type, vector_type> ret;

        if (radix == 16) {
            ret.first  = a;
            ret.second = reverse<T, 16>::run(b);
        }
        else if (radix == 8) {
            ret = interleave_bits_custom<TypeBits, T, Elems>::run(a, b, 0u,  1u,  2u,  3u,  4u,  5u,  6u,  7u, 23u, 22u, 21u, 20u, 19u, 18u, 17u, 16u,
                                                                        8u,  9u, 10u, 11u, 12u, 13u, 14u, 15u, 31u, 30u, 29u, 28u, 27u, 26u, 25u, 24u);
        }
        else if (radix == 4) {
            ret = interleave_bits_custom<TypeBits, T, Elems>::run(a, b, 0u,  1u,  2u,  3u, 23u, 22u, 21u, 20u,  8u,  9u, 10u, 11u, 31u, 30u, 29u, 28u,
                                                                        4u,  5u,  6u,  7u, 19u, 18u, 17u, 16u, 12u, 13u, 14u, 15u, 27u, 26u, 25u, 24u);
        }
        else if (radix == 2) {
            ret = interleave_bits_custom<TypeBits, T, Elems>::run(a, b, 0u,  1u, 19u, 18u,  4u,  5u, 23u, 22u,  8u,  9u, 27u, 26u, 12u, 13u, 31u, 30u,
                                                                        2u,  3u, 17u, 16u,  6u,  7u, 21u, 20u, 10u, 11u, 25u, 24u, 14u, 15u, 29u, 28u);
        }
        else if (radix == 1) {
            ret = interleave_bits_custom<TypeBits, T, Elems>::run(a, b, 0u, 17u,  2u, 19u,  4u, 21u,  6u, 23u,  8u, 25u, 10u, 27u, 12u, 29u, 14u, 31u,
                                                                        1u, 16u,  3u, 18u,  5u, 20u,  7u, 22u,  9u, 24u, 11u, 26u, 13u, 28u, 15u, 30u);
        }

        return ret;
    }
#endif
};

template <unsigned TypeBits, typename T, unsigned Elems>
struct interleave_bits_butterfly_half
{
#ifdef __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
    using vector_type = vector<T, Elems>;

    static std::pair<vector_type, vector_type> run(const vector_type &a, const vector_type &b, unsigned radix)
    {
        std::pair<vector_type, vector_type> ret;

        if (radix == 16) {
            ret.first  = a;
            ret.second = reverse<T, 16>::run(b);
        }
        else if (radix == 8) {
            ret = interleave_bits_custom<TypeBits, T, Elems>::run(a, b, 0u,  1u,  2u,  3u,  4u,  5u,  6u,  7u, 23u, 22u, 21u, 20u, 19u, 18u, 17u, 16u,
                                                                        8u,  9u, 10u, 11u, 12u, 13u, 14u, 15u, 31u, 30u, 29u, 28u, 27u, 26u, 25u, 24u);
        }
        else if (radix == 4) {
            ret = interleave_bits_custom<TypeBits, T, Elems>::run(a, b, 0u,  1u,  2u,  3u, 23u, 22u, 21u, 20u,  8u,  9u, 10u, 11u, 31u, 30u, 29u, 28u,
                                                                        4u,  5u,  6u,  7u, 19u, 18u, 17u, 16u, 12u, 13u, 14u, 15u, 27u, 26u, 25u, 24u);
        }
        else if (radix == 2) {
            ret = interleave_bits_custom<TypeBits, T, Elems>::run(a, b, 0u,  1u, 19u, 18u,  4u,  5u, 23u, 22u,  8u,  9u, 27u, 26u, 12u, 13u, 31u, 30u,
                                                                        2u,  3u, 17u, 16u,  6u,  7u, 21u, 20u, 10u, 11u, 25u, 24u, 14u, 15u, 29u, 28u);
        }
        else if (radix == 1) {
            ret = interleave_bits_custom<TypeBits, T, Elems>::run(a, b, 0u, 17u,  2u, 19u,  4u, 21u,  6u, 23u,  8u, 25u, 10u, 27u, 12u, 29u, 14u, 31u,
                                                                        1u, 16u,  3u, 18u,  5u, 20u,  7u, 22u,  9u, 24u, 11u, 26u, 13u, 28u, 15u, 30u);
        }

        return ret;
    }
#endif
};

template <unsigned TypeBits, typename T, unsigned Elems>
struct interleave_bits_crossover
{
#ifdef __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
    using vector_type = vector<T, Elems>;

    static std::pair<vector_type, vector_type> run(const vector_type &a, const vector_type &b, unsigned radix)
    {
        std::pair<vector_type, vector_type> ret;

        if (radix == 16) {
            ret = interleave_bits_custom<TypeBits, T, Elems>::run(a, b,  0u, 16u,  1u, 17u,  2u, 18u,  3u, 19u,  4u, 20u,  5u, 21u,  6u, 22u,  7u, 23u,
                                                                        31u, 15u, 30u, 14u, 29u, 13u, 28u, 12u, 27u, 11u, 26u, 10u, 25u,  9u, 24u,  8u);
        }
        else if (radix == 8) {
            ret = interleave_bits_custom<TypeBits, T, Elems>::run(a, b,  0u, 16u,  1u, 17u,  2u, 18u,  3u, 19u,  8u, 24u,  9u, 25u, 10u, 26u, 11u, 27u,
                                                                        23u,  7u, 22u,  6u, 21u,  5u, 20u,  4u, 31u, 15u, 30u, 14u, 29u, 13u, 28u, 12u);
        }
        else if (radix == 4) {
            ret = interleave_bits_custom<TypeBits, T, Elems>::run(a, b,  0u, 16u,  1u, 17u,  4u, 20u,  5u, 21u,  8u, 24u,  9u, 25u, 12u, 28u, 13u, 29u,
                                                                        19u,  3u, 18u,  2u, 23u,  7u, 22u,  6u, 27u, 11u, 26u, 10u, 31u, 15u, 30u, 14u);
        }
        else if (radix == 2) {
            ret = interleave_bits_custom<TypeBits, T, Elems>::run(a, b,  0u, 16u,  2u, 18u,  4u, 20u,  6u, 22u,  8u, 24u, 10u, 26u, 12u, 28u, 14u, 30u,
                                                                        17u,  1u, 19u,  3u, 21u,  5u, 23u,  7u, 25u,  9u, 27u, 11u, 29u, 13u, 31u, 15u);
        }
        else if (radix == 1) {
            ret.first  = a;
            ret.second = b;
        }

        return ret;
    }
#endif
};

template <typename T, unsigned Elems>
using interleave_zip = interleave_bits_zip<type_bits_v<T>, T, Elems>;

template <typename T, unsigned Elems>
using interleave_unzip = interleave_bits_unzip<type_bits_v<T>, T, Elems>;

template <typename T, unsigned Elems> requires(type_bits_v<T> * Elems <= 1024)
using interleave = interleave_bits<type_bits_v<T>, T, Elems>;

template <typename T, unsigned Elems> requires(type_bits_v<T> * Elems <= 1024)
using interleave_butterfly = interleave_bits_butterfly<type_bits_v<T>, T, Elems>;

template <typename T, unsigned Elems> requires(type_bits_v<T> * Elems <= 1024)
using interleave_butterfly_half = interleave_bits_butterfly_half<type_bits_v<T>, T, Elems>;

template <typename T, unsigned Elems> requires(type_bits_v<T> * Elems <= 1024)
using interleave_crossover = interleave_bits_crossover<type_bits_v<T>, T, Elems>;

template <typename T, unsigned Elems, unsigned... Select> requires(type_bits_v<T> * Elems <= 1024)
using interleave_custom = interleave_bits_custom<type_bits_v<T>, T, Elems>;

template <typename T, unsigned Elems, unsigned... Select> requires(type_bits_v<T> * Elems <= 1024)
using interleave_custom_static = interleave_bits_custom_static<type_bits_v<T>, T, Elems, Select...>;

}

#if __AIE_ARCH__ == 10

#include "aie1/interleave.hpp"

#elif __AIE_ARCH__ == 20

#include "aie2/interleave.hpp"

#endif

#endif
