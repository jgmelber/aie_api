// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_BLEND__HPP__
#define __AIE_API_DETAIL_BLEND__HPP__

#include "../mask.hpp"

namespace aie::detail {

template <unsigned TypeBits, typename T, unsigned Elems>
struct select_bits_impl
{
#ifdef __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v1, const vector_type &v2, const mask<Elems> &m)
    {
        vector_type ret;

        for (unsigned i = 0; i < Elems; ++i) {
            if (m.test(i))
                ret[i] = v2[i];
            else
                ret[i] = v1[i];
        }

        return ret;
    }


    static vector_type run(T a, const vector_type &v, const mask<Elems> &m)
    {
        vector_type ret;

        for (unsigned i = 0; i < Elems; ++i) {
            if (m.test(i))
                ret[i] = v[i];
            else
                ret[i] = a;
        }

        return ret;
    }

    static vector_type run(const vector_type &v, T a, const mask<Elems> &m)
    {
        vector_type ret;

        for (unsigned i = 0; i < Elems; ++i) {
            if (m.test(i))
                ret[i] = a;
            else
                ret[i] = v[i];
        }

        return ret;
    }

    static vector_type run(T a, T b, const mask<Elems> &m)
    {
        vector_type ret;

        for (unsigned i = 0; i < Elems; ++i) {
            if (m.test(i))
                ret[i] = b;
            else
                ret[i] = a;
        }

        return ret;
    }
#endif
};

template <unsigned TypeBits, typename T, unsigned Elems>
struct select_bits
{
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v1, const vector_type &v2, const mask<Elems> &m)
    {
        return select_bits_impl<TypeBits, T, Elems>::run(v1, v2, m);
    }

    static vector_type run(const T &a, const vector_type &v, const mask<Elems> &m)
    {
        return select_bits_impl<TypeBits, T, Elems>::run(a, v, m);
    }

    static vector_type run(const vector_type &v, const T &a, const mask<Elems> &m)
    {
        return select_bits_impl<TypeBits, T, Elems>::run(v, a, m);
    }

    template <unsigned Elems2>
    static vector_type run(vector_elem_const_ref<T, Elems2> a, const vector_type &v, const mask<Elems> &m)
    {
        return select_bits_impl<TypeBits, T, Elems>::run(a, v, m);
    }

    template <unsigned Elems2>
    static vector_type run(const vector_type &v, vector_elem_const_ref<T, Elems2> a, const mask<Elems> &m)
    {
        return select_bits_impl<TypeBits, T, Elems>::run(v, a, m);
    }

    template <unsigned Elems2>
    static vector_type run(vector_elem_const_ref<T, Elems2> a, vector_elem_const_ref<T, Elems> b, const mask<Elems> &m)
    {
        return select_bits_impl<TypeBits, T, Elems>::run(a, b, m);
    }

    static vector_type run(const T &a, const T &b, const mask<Elems> &m)
    {
        return select_bits_impl<TypeBits, T, Elems>::run(a, b, m);
    }
};

template <typename T, unsigned Elems>
using select = select_bits<type_bits_v<T>, T, Elems>;

}

#if __AIE_ARCH__ == 10

#include "aie1/blend.hpp"

#elif __AIE_ARCH__ == 20

#include "aie2/blend.hpp"

#endif

#endif
