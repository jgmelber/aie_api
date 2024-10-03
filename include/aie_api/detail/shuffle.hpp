// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_SHUFFLE__HPP__
#define __AIE_API_DETAIL_SHUFFLE__HPP__

#include "vector.hpp"
#include "broadcast.hpp"

namespace aie::detail {

template <unsigned TypeBits, typename T, unsigned Elems>
struct shuffle_up_bits_impl_scalar
{
#ifdef __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v, unsigned n)
    {
        vector_type ret;

        for (unsigned i = 0; i < Elems - n; ++i)
            ret[i + n] = v[i];

        return ret;
    }
#endif
};

template <unsigned TypeBits, typename T, unsigned Elems>
struct shuffle_up_fill_bits_impl_scalar
{
#ifdef __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v, const vector_type &fill, unsigned n)
    {
        vector_type ret;

        for (unsigned i = 0; i < Elems - n; ++i)
            ret[i + n] = v[i];

        for (unsigned i = 0; i < n; ++i)
            ret[i] = fill[i + Elems - n];

        return ret;
    }
#endif
};

template <unsigned TypeBits, typename T, unsigned Elems>
struct shuffle_up_rotate_bits_impl_scalar
{
#ifdef __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v, unsigned n)
    {
        vector_type ret;

        for (unsigned i = 0; i < Elems - n; ++i)
            ret[i + n] = v[i];

        for (unsigned i = 0; i < n; ++i)
            ret[i] = v[i + Elems - n];

        return ret;
    }
#endif
};

template <unsigned TypeBits, typename T, unsigned Elems>
struct shuffle_up_replicate_bits_impl_scalar
{
#ifdef __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v, unsigned n)
    {
        vector_type ret;

        for (unsigned i = 0; i < Elems - n; ++i)
            ret[i + n] = v[i];

        for (unsigned i = 0; i < n; ++i)
            ret[i] = v[0];

        return ret;
    }
#endif
};

template <unsigned TypeBits, typename T, unsigned Elems>
struct shuffle_down_replicate_bits_impl_scalar
{
#ifdef __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v, unsigned n)
    {
        vector_type ret;

        for (unsigned i = 0; i < Elems - n; ++i)
            ret[i] = v[i + n];

        for (unsigned i = Elems - n; i < Elems; ++i)
            ret[i] = v[Elems - 1];

        return ret;
    }
#endif
};

template <unsigned TypeBits, typename T, unsigned Elems>
struct shuffle_up_bits_impl           : public shuffle_up_bits_impl_scalar<TypeBits, T, Elems> {};

template <unsigned TypeBits, typename T, unsigned Elems>
struct shuffle_up_fill_bits_impl      : public shuffle_up_fill_bits_impl_scalar<TypeBits, T, Elems> {};

template <unsigned TypeBits, typename T, unsigned Elems>
struct shuffle_up_rotate_bits_impl    : public shuffle_up_rotate_bits_impl_scalar<TypeBits, T, Elems> {};

template <unsigned TypeBits, typename T, unsigned Elems>
struct shuffle_up_replicate_bits_impl : public shuffle_up_replicate_bits_impl_scalar<TypeBits, T, Elems> {};


template <unsigned TypeBits, typename T, unsigned Elems>
struct shuffle_down_bits_impl_scalar
{
#ifdef __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v, unsigned n)
    {
        vector_type ret;

        for (unsigned i = 0; i < Elems - n; ++i)
            ret[i] = v[i + n];

        return ret;
    }
#endif
};

template <unsigned TypeBits, typename T, unsigned Elems>
struct shuffle_down_fill_bits_impl_scalar
{
#ifdef __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v, const vector_type &fill, unsigned n)
    {
        vector_type ret;

        for (unsigned i = 0; i < Elems - n; ++i)
            ret[i] = v[i + n];

        for (unsigned i = 0; i < n; ++i)
            ret[i + Elems - n] = fill[i];

        return ret;
    }
#endif
};

template <unsigned TypeBits, typename T, unsigned Elems>
struct shuffle_down_rotate_bits_impl_scalar
{
#ifdef __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v, unsigned n)
    {
        vector_type ret;

        for (unsigned i = 0; i < Elems - n; ++i)
            ret[i] = v[i + n];

        for (unsigned i = 0; i < n; ++i)
            ret[i + Elems - n] = v[i];

        return ret;
    }
#endif
};

template <unsigned TypeBits, typename T, unsigned Elems>
struct shuffle_down_bits_impl        : public shuffle_down_bits_impl_scalar<TypeBits, T, Elems> {};

template <unsigned TypeBits, typename T, unsigned Elems>
struct shuffle_down_fill_bits_impl   : public shuffle_down_fill_bits_impl_scalar<TypeBits, T, Elems> {};

template <unsigned TypeBits, typename T, unsigned Elems>
struct shuffle_down_rotate_bits_impl : public shuffle_down_rotate_bits_impl_scalar<TypeBits, T, Elems> {};

template <unsigned TypeBits, typename T, unsigned Elems>
struct shuffle_down_replicate_bits_impl : public shuffle_down_replicate_bits_impl_scalar<TypeBits, T, Elems> {};

template <unsigned TypeBits, typename T, unsigned Elems>
struct shuffle_up_bits
{
    using vector_type = vector<T, Elems>;

    __aie_inline
    static vector_type run(const vector_type &v, unsigned n)
    {
        return shuffle_up_bits_impl<TypeBits, T, Elems>::run(v, n);
    }
};

template <unsigned TypeBits, typename T, unsigned Elems>
struct shuffle_up_fill_bits
{
    using vector_type = vector<T, Elems>;

    __aie_inline
    static vector_type run(const vector_type &v, const vector_type &fill, unsigned n)
    {
        return shuffle_up_fill_bits_impl<TypeBits, T, Elems>::run(v, fill, n);
    }
};

template <unsigned TypeBits, typename T, unsigned Elems>
struct shuffle_up_replicate_bits
{
    using vector_type = vector<T, Elems>;

    __aie_inline
    static vector_type run(const vector_type &v, unsigned n)
    {
#if __AIE_ARCH__ == 20
        return shuffle_up_fill_bits_impl<TypeBits, T, Elems>::run(v, broadcast<T, Elems>::run(v[0]),  n);
#else
        return shuffle_up_replicate_bits_impl<TypeBits, T, Elems>::run(v, n);
#endif
    }
};

template <unsigned TypeBits, typename T, unsigned Elems>
struct shuffle_up_rotate_bits
{
    using vector_type = vector<T, Elems>;

    __aie_inline
    static vector_type run(const vector_type &v, unsigned n)
    {
        return shuffle_up_rotate_bits_impl<TypeBits, T, Elems>::run(v, n);
    }
};

template <unsigned TypeBits, typename T, unsigned Elems>
struct shuffle_down_bits
{
    using vector_type = vector<T, Elems>;

    __aie_inline
    static vector_type run(const vector_type &v, unsigned n)
    {
        return shuffle_down_bits_impl<TypeBits, T, Elems>::run(v, n);
    }
};

template <unsigned TypeBits, typename T, unsigned Elems>
struct shuffle_down_fill_bits
{
    using vector_type = vector<T, Elems>;

    __aie_inline
    static vector_type run(const vector_type &v, const vector_type &fill, unsigned n)
    {
        return shuffle_down_fill_bits_impl<TypeBits, T, Elems>::run(v, fill, n);
    }
};

template <unsigned TypeBits, typename T, unsigned Elems>
struct shuffle_down_rotate_bits
{
    using vector_type = vector<T, Elems>;

    __aie_inline
    static vector_type run(const vector_type &v, unsigned n)
    {
        return shuffle_down_rotate_bits_impl<TypeBits, T, Elems>::run(v, n);
    }
};

template <unsigned TypeBits, typename T, unsigned Elems>
struct shuffle_down_replicate_bits
{
    using vector_type = vector<T, Elems>;

    __aie_inline
    static vector_type run(const vector_type &v, unsigned n)
    {
#if __AIE_ARCH__ == 20
        return shuffle_down_fill_bits_impl<TypeBits, T, Elems>::run(v, broadcast<T, Elems>::run(v[Elems - 1]), n);
#else
        return shuffle_down_replicate_bits_impl<TypeBits, T, Elems>::run(v, n);
#endif
    }
};

template <typename T, unsigned Elems>
using shuffle_up             = shuffle_up_bits<type_bits_v<T>, T, Elems>;

template <typename T, unsigned Elems>
using shuffle_up_fill        = shuffle_up_fill_bits<type_bits_v<T>, T, Elems>;

template <typename T, unsigned Elems>
using shuffle_up_rotate      = shuffle_up_rotate_bits<type_bits_v<T>, T, Elems>;

template <typename T, unsigned Elems>
using shuffle_up_replicate   = shuffle_up_replicate_bits<type_bits_v<T>, T, Elems>;

template <typename T, unsigned Elems>
using shuffle_down           = shuffle_down_bits<type_bits_v<T>, T, Elems>;

template <typename T, unsigned Elems>
using shuffle_down_fill      = shuffle_down_fill_bits<type_bits_v<T>, T, Elems>;

template <typename T, unsigned Elems>
using shuffle_down_rotate    = shuffle_down_rotate_bits<type_bits_v<T>, T, Elems>;

template <typename T, unsigned Elems>
using shuffle_down_replicate = shuffle_down_replicate_bits<type_bits_v<T>, T, Elems>;

}

#if __AIE_ARCH__ == 10

#include "aie1/shuffle.hpp"
#include "aie1/shuffle_replicate.hpp"

#elif __AIE_ARCH__ == 20

#include "aie2/shuffle.hpp"

#endif

#endif
