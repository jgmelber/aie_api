// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_SHUFFLE__HPP__
#define __AIE_API_DETAIL_AIE2_SHUFFLE__HPP__

#include "../../vector.hpp"
#include "../../mask.hpp"

namespace aie::detail {

template <typename T, unsigned Elems>
struct shuffle_down_fill_bits_impl<4, T, Elems>
{
    using vector_type = vector<T, Elems>;

    __aie_inline
    static vector_type run(const vector_type &v, const vector_type &fill, unsigned n)
    {
        using next_type = utils::get_next_integer_type_t<T>;
        vector_type ret;

        if (chess_manifest(n == 0))
            return v;

        if (chess_manifest(n % 2 == 0))
            return shuffle_down_fill_bits_impl<8, next_type, Elems / 2>::run(v.template cast_to<next_type>(), fill.template cast_to<next_type>(), n / 2).template cast_to<T>();

        if constexpr (vector_type::bits() <= 512) {
            ret = shuffle_down_fill_bits_impl<8, next_type, Elems>::run(v.unpack(), fill.unpack(), n).template pack<T>();

            return ret;
        }
        else if constexpr (vector_type::bits() <= 1024) {
            return shuffle_down_fill_bits_impl_scalar<4, T, Elems>::run(v, fill, n);
        }
    }
};

template <typename T, unsigned Elems>
struct shuffle_down_bits_impl<4, T, Elems>
{
    using vector_type = vector<T, Elems>;

    __aie_inline
    static vector_type run(const vector_type &v, unsigned n)
    {
        using next_type = utils::get_next_integer_type_t<T>;
        vector_type ret;

        if (chess_manifest(n == 0))
            return v;

        if (chess_manifest(n % 2 == 0))
            return shuffle_down_bits_impl<8, next_type, Elems / 2>::run(v.template cast_to<next_type>(), n / 2).template cast_to<T>();

        if constexpr (vector_type::bits() <= 512) {
            ret = shuffle_down_bits_impl<8, next_type, Elems>::run(v.unpack(), n).template pack<T>();

            return ret;
        }
        else if constexpr (vector_type::bits() <= 1024) {
            return shuffle_down_bits_impl_scalar<4, T, Elems>::run(v, n);
        }
    }
};

template <unsigned TypeBits, typename T, unsigned Elems> requires(TypeBits >= 4)
struct shuffle_up_rotate_bits_impl<TypeBits, T, Elems>
{
    using vector_type = vector<T, Elems>;

    __aie_inline
    static vector_type run(const vector_type &v, unsigned n)
    {
        return shuffle_down_rotate_bits_impl<TypeBits, T, Elems>::run(v, Elems - n);
    }
};

template <unsigned TypeBits, typename T, unsigned Elems> requires(TypeBits >= 4)
struct shuffle_up_fill_bits_impl<TypeBits, T, Elems>
{
    using vector_type = vector<T, Elems>;

    __aie_inline
    static vector_type run(const vector_type &v, const vector_type &fill, unsigned n)
    {
        return shuffle_down_fill_bits_impl<TypeBits, T, Elems>::run(fill, v, Elems - n);
    }
};

template <unsigned TypeBits, typename T, unsigned Elems> requires(TypeBits >= 4)
struct shuffle_up_bits_impl<TypeBits, T, Elems>
{
    using vector_type = vector<T, Elems>;

    __aie_inline
    static vector_type run(const vector_type &v, unsigned n)
    {
        return shuffle_up_fill_bits_impl<TypeBits, T, Elems>::run(v, vector_type(), n);
    }
};

template <unsigned TypeBits, typename T, unsigned Elems> requires(TypeBits >= 4)
struct shuffle_down_rotate_bits_impl<TypeBits, T, Elems>
{
    using vector_type = vector<T, Elems>;

    __aie_inline
    static vector_type run(const vector_type &v, unsigned n)
    {
        return shuffle_down_fill_bits_impl<TypeBits, T, Elems>::run(v, v, n);
    }
};

template <unsigned TypeBits, typename T, unsigned Elems> requires(TypeBits >= 8)
struct shuffle_down_fill_bits_impl<TypeBits, T, Elems>
{
    using vector_type = vector<T, Elems>;

    __aie_inline
    static vector_type run(const vector_type &v, const vector_type &fill, unsigned n)
    {
        vector_type ret;

        if (chess_manifest(n == 0))
            return v;

        if constexpr (vector_type::bits() == 128) {
            ret = shuffle_down_fill_bits_impl<TypeBits, T, Elems * 4>::run(concat_vector(            v,           fill, vector_type(), vector_type()),
                                                                           concat_vector(vector_type(),  vector_type(), vector_type(), vector_type()), n).template extract<Elems>(0);
        }
        else if constexpr (vector_type::bits() == 256) {
            ret = shuffle_down_fill_bits_impl<TypeBits, T, Elems * 2>::run(concat_vector(            v,           fill),
                                                                           concat_vector(vector_type(),  vector_type()), n).template extract<Elems>(0);
        }
        else if constexpr (vector_type::bits() == 512) {
            ret = SHIFT_BYTES(v, fill, n * sizeof(T));
        }
        else if constexpr (vector_type::bits() == 1024) {
            if (chess_manifest(n >= Elems / 2)) {
                ret.template insert<Elems / 2>(0, SHIFT_BYTES(   v.template extract<Elems / 2>(1),
                                                              fill.template extract<Elems / 2>(0), (n - Elems / 2) * sizeof(T)));
                ret.template insert<Elems / 2>(1, SHIFT_BYTES(fill.template extract<Elems / 2>(0),
                                                              fill.template extract<Elems / 2>(1), (n - Elems / 2) * sizeof(T)));
            }
            else if (chess_manifest(n < Elems / 2)) {
                ret.template insert<Elems / 2>(0, SHIFT_BYTES(   v.template extract<Elems / 2>(0),
                                                                 v.template extract<Elems / 2>(1), n * sizeof(T)));
                ret.template insert<Elems / 2>(1, SHIFT_BYTES(   v.template extract<Elems / 2>(1),
                                                              fill.template extract<Elems / 2>(0), n * sizeof(T)));
            }
            else {
                vector<T, Elems / 2> tmp1a, tmp1b, tmp2a, tmp2b;

                mask<Elems / 2> mask1(true), mask2(true);

                mask1 <<= Elems - n;
                mask2 <<= Elems - n;

                tmp1a = SHIFT_BYTES(v.template extract<Elems / 2>(0),    v.template extract<Elems / 2>(1), n * sizeof(T));
                tmp1b = SHIFT_BYTES(vector<T, Elems / 2>(),           fill.template extract<Elems / 2>(0), (n - Elems / 2) * sizeof(T));

                tmp2a = SHIFT_BYTES(v.template extract<Elems / 2>(1), fill.template extract<Elems / 2>(0), n * sizeof(T));
                tmp2b = SHIFT_BYTES(vector<T, Elems / 2>(),           fill.template extract<Elems / 2>(1), (n - Elems / 2) * sizeof(T));

                if constexpr (Elems > 64) {
                    ret.template insert<Elems / 2>(0, ::sel(tmp1a, tmp1b, mask1.to_uint64()));
                    ret.template insert<Elems / 2>(1, ::sel(tmp2a, tmp2b, mask2.to_uint64()));
                }
                else {
                    ret.template insert<Elems / 2>(0, ::sel(tmp1a, tmp1b, mask1.to_uint32()));
                    ret.template insert<Elems / 2>(1, ::sel(tmp2a, tmp2b, mask2.to_uint32()));
                }
            }
        }

        return ret;
    }
};

template <unsigned TypeBits, typename T, unsigned Elems> requires(TypeBits >= 8)
struct shuffle_down_bits_impl<TypeBits, T, Elems>
{
    using vector_type = vector<T, Elems>;

    __aie_inline
    static vector_type run(const vector_type &v, unsigned n)
    {
        vector_type ret;

        if (chess_manifest(n == 0))
            return v;

        if constexpr (vector_type::bits() == 128) {
            ret = shuffle_down_bits_impl<TypeBits, T, Elems * 4>::run(v.template grow<Elems * 4>(), n).template extract<Elems>(0);
        }
        else if constexpr (vector_type::bits() == 256) {
            ret = shuffle_down_bits_impl<TypeBits, T, Elems * 2>::run(v.template grow<Elems * 2>(), n).template extract<Elems>(0);
        }
        else if constexpr (vector_type::bits() == 512) {
            ret = SHIFT_BYTES(v, v, n * sizeof(T));
        }
        else if constexpr (vector_type::bits() == 1024) {
            ret.template insert<Elems / 2>(0, SHIFT_BYTES(v.template extract<Elems / 2>(0), v.template extract<Elems / 2>(1), n * sizeof(T)));
            ret.template insert<Elems / 2>(1, SHIFT_BYTES(v.template extract<Elems / 2>(1), vector<T, Elems / 2>(),           n * sizeof(T)));
        }

        return ret;
    }
};

}

#endif
