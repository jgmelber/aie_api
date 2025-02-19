// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_ADD__HPP__
#define __AIE_API_DETAIL_ADD__HPP__

#include "vector.hpp"

namespace aie::detail {

enum class AddSubOperation
{
    Add,
    Sub,
};

template <unsigned TypeBits, typename T, unsigned Elems, AddSubOperation Op>
struct add_sub_bits_impl
{
#ifdef __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v1, const vector_type &v2)
    {
        vector_type ret;

        for (unsigned i = 0; i < vector_type::size(); ++i) {
            if (Op == AddSubOperation::Add)
                ret[i] = v1[i] + v2[i];
            else
                ret[i] = v1[i] - v2[i];
        }

        return ret;
    }

    static vector_type run(T a, const vector_type &v)
    {
        vector_type ret;

        for (unsigned i = 0; i < vector_type::size(); ++i) {
            if (Op == AddSubOperation::Add)
                ret[i] = a + v[i];
            else
                ret[i] = a - v[i];
        }

        return ret;
    }

    static vector_type run(const vector_type &v, T a)
    {
        static_assert(Op == AddSubOperation::Sub);

        return run(-a, v);
    }
#endif
};

template <AccumElemBaseType AccumTag, unsigned Elems, AddSubOperation Op>
struct add_sub_accum_bits_impl
{
#ifdef __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
    using  accum_type = accum<AccumTag, Elems>;

    static accum_type run(const accum_type &acc1, bool zero_acc1, const accum_type &acc2)
    {
        accum_type ret;

        for (unsigned i = 0; i < accum_type::size(); ++i) {
            if (Op == AddSubOperation::Add)
                ret[i] = (zero_acc1? 0 : acc1[i]) + acc2[i];
            else
                ret[i] = (zero_acc1? 0 : acc1[i]) - acc2[i];
        }

        return ret;
    }
#endif
};

template <unsigned AccumBits, unsigned TypeBits, typename T, unsigned Elems, AddSubOperation Op>
struct add_sub_accum_vector_bits_impl
{
#ifdef __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
    using vector_type = vector<T, Elems>;
    using   accum_tag = accum_tag_for_type<T, AccumBits>;
    using  accum_type = accum<accum_tag, Elems>;

    static accum_type run(const accum_type &acc, bool zero_acc, const vector_type &v)
    {
        accum_type ret;

        for (unsigned i = 0; i < vector_type::size(); ++i) {
            if (Op == AddSubOperation::Add)
                ret[i] = (zero_acc? 0 : acc[i]) + v[i];
            else
                ret[i] = (zero_acc? 0 : acc[i]) - v[i];
        }

        return ret;
    }

    static accum_type run(const accum_type &acc, bool zero_acc, T a)
    {
        accum_type ret;

        for (unsigned i = 0; i < vector_type::size(); ++i) {
            if (Op == AddSubOperation::Add)
                ret[i] = (zero_acc? 0 : acc[i]) + a;
            else
                ret[i] = (zero_acc? 0 : acc[i]) - a;
        }

        return ret;
    }
#endif
};

template <unsigned TypeBits, typename T, unsigned Elems>
struct add_reduce_bits_impl
{
#ifdef __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
    using vector_type = vector<T, Elems>;

    static T run(const vector_type &v)
    {
        T ret = v[0];

        for (unsigned i = 1; i < Elems; ++i) {
            const T a = v[i];
            ret += a;
        }

        return ret;
    }
#endif
};

template <unsigned TypeBits, typename T, unsigned Elems, unsigned N>
struct add_reduce_v_bits_impl
{
#ifdef __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
    using vector_type = vector<T, Elems>;

    template <typename... Vectors>
    static vector_type run(Vectors &&... vectors)
    {
        vector_type ret;

        utils::unroll_times<N>([&](auto idx) __aie_inline {
            ret[idx] = add_reduce_bits_impl<TypeBits, T, Elems>::run(utils::get_nth<idx>(std::forward<Vectors>(vectors)...));
        });

        return ret;
    }
#endif
};

template <unsigned TypeBits, typename T, unsigned Elems, AddSubOperation Op>
struct add_sub_bits
{
    using vector_type = vector<T, Elems>;

    __aie_inline
    static vector_type run(const vector_type &v1, const vector_type &v2)
    {
        return add_sub_bits_impl<TypeBits, T, Elems, Op>::run(v1, v2);
    }

    template <unsigned Elems2>
    __aie_inline
    static vector_type run(vector_elem_const_ref<T, Elems2> a, const vector_type &v)
    {
        return add_sub_bits_impl<TypeBits, T, Elems, Op>::run(a, v);
    }

    template <unsigned Elems2>
    __aie_inline
    static vector_type run(const vector_type &v, vector_elem_const_ref<T, Elems2> a)
    {
        return add_sub_bits_impl<TypeBits, T, Elems, Op>::run(v, a);
    }

    __aie_inline
    static vector_type run(const T &a, const vector_type &v)
    {
        return add_sub_bits_impl<TypeBits, T, Elems, Op>::run(a, v);
    }

    __aie_inline
    static vector_type run(const vector_type &v, const T &a)
    {
        return add_sub_bits_impl<TypeBits, T, Elems, Op>::run(v, a);
    }
};

template <AccumElemBaseType AccumTag, unsigned Elems, AddSubOperation Op>
struct add_sub_accum_bits
{
    using accum_type = accum<AccumTag, Elems>;

    __aie_inline
    static accum_type run(const accum_type &acc1, bool zero_acc1, const accum_type &acc2)
    {
        return add_sub_accum_bits_impl<AccumTag, Elems, Op>::run(acc1, zero_acc1, acc2);
    }
};

template <unsigned AccumBits, unsigned TypeBits, typename T, unsigned Elems, AddSubOperation Op>
struct add_sub_accum_vector_bits
{
    using vector_type = vector<T, Elems>;
    using   accum_tag = accum_tag_for_type<T, AccumBits>;
    using  accum_type = accum<accum_tag, Elems>;

    __aie_inline
    static accum_type run(const accum_type &acc, bool zero_acc, const vector_type &v)
    {
        return add_sub_accum_vector_bits_impl<AccumBits, TypeBits, T, Elems, Op>::run(acc, zero_acc, v);
    }

    template <unsigned Elems2>
    __aie_inline
    static accum_type run(const accum_type &acc, bool zero_acc, vector_elem_const_ref<T, Elems2> a)
    {
        return add_sub_accum_vector_bits_impl<AccumBits, TypeBits, T, Elems, Op>::run(acc, zero_acc, a);
    }

    __aie_inline
    static accum_type run(const accum_type &acc, bool zero_acc, const T &a)
    {
        return add_sub_accum_vector_bits_impl<AccumBits, TypeBits, T, Elems, Op>::run(acc, zero_acc, a);
    }
};

template <unsigned TypeBits, typename T, unsigned Elems>
struct add_reduce_bits
{
    using vector_type = vector<T, Elems>;

    __aie_inline
    static T run(const vector_type &v)
    {
        return add_reduce_bits_impl<TypeBits, T, Elems>::run(v);
    }
};

template <unsigned TypeBits, typename T, unsigned Elems>
struct add_reduce_v_bits
{
    using vector_type = vector<T, Elems>;

    template <typename... Vectors>
    __aie_inline
    static vector_type run(Vectors &&... vectors)
    {
        return add_reduce_v_bits_impl<TypeBits, T, Elems, sizeof...(Vectors)>::run(std::forward<Vectors>(vectors)...);
    }
};

template <typename T, unsigned Elems>
using add              = add_sub_bits<type_bits_v<T>, T, Elems, AddSubOperation::Add>;

template <AccumElemBaseType AccumTag, unsigned Elems>
using add_accum        = add_sub_accum_bits<accum_native_type_t<AccumTag>, Elems, AddSubOperation::Add>;

template <unsigned AccumBits, typename T, unsigned Elems>
using add_accum_vector = add_sub_accum_vector_bits<AccumBits, type_bits_v<T>, T, Elems, AddSubOperation::Add>;

template <typename T, unsigned Elems>
using add_reduce       = add_reduce_bits<type_bits_v<T>, T, Elems>;

template <typename T, unsigned Elems>
using add_reduce_v     = add_reduce_v_bits<type_bits_v<T>, T, Elems>;

template <typename T, unsigned Elems>
using sub              = add_sub_bits<type_bits_v<T>, T, Elems, AddSubOperation::Sub>;

template <AccumElemBaseType AccumTag, unsigned Elems>
using sub_accum        = add_sub_accum_bits<accum_native_type_t<AccumTag>, Elems, AddSubOperation::Sub>;

template <unsigned AccumBits, typename T, unsigned Elems>
using sub_accum_vector = add_sub_accum_vector_bits<AccumBits, type_bits_v<T>, T, Elems, AddSubOperation::Sub>;

}

#if __AIE_ARCH__ == 10

#include "aie1/add.hpp"
#include "aie1/add_accum.hpp"
#include "aie1/add_reduce.hpp"

#elif __AIE_ARCH__ == 20

#include "aie2/add.hpp"
#include "aie2/add_accum.hpp"
#include "aie2/add_reduce.hpp"

#elif __AIE_ARCH__ == 21

#include "aie2/add.hpp"
#include "aie2p/add_accum.hpp"
#include "aie2/add_reduce.hpp"

#endif

#endif
