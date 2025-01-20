// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_BROADCAST__HPP__
#define __AIE_API_DETAIL_BROADCAST__HPP__

#include "utils.hpp"

#include "../concepts.hpp"
#include "../accum.hpp"
#include "../vector.hpp"

namespace aie::detail {

template <unsigned TypeBits, typename T, unsigned Elems>
struct broadcast_bits_impl
{
#ifdef __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
    using vector_type = vector<T, Elems>;

    static vector_type run(T a)
    {
        vector_type ret;

        for (unsigned i = 0; i < Elems; ++i)
            ret[i] = a;

        return ret;
    }
#endif
};

template <unsigned TypeBits, typename T, unsigned Elems>
struct zeros_bits_impl
{
#ifdef __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
    using vector_type = vector<T, Elems>;

    static vector_type run()
    {
        vector_type ret;

        for (unsigned i = 0; i < Elems; ++i) {
            if constexpr (vector_type::is_complex())
                ret[i] = T({0, 0});
            else
                ret[i] = T(0);
        }

        return ret;
    }
#endif
};

template <unsigned TypeBits, typename T, unsigned Elems>
struct broadcast_bits
{
    using vector_type = vector<T, Elems>;

    static vector_type run(const T &a)
    {
        return broadcast_bits_impl<TypeBits, T, Elems>::run(a);
    }

    template <unsigned Elems2>
    static vector_type run(vector_elem_const_ref<T, Elems2> a)
    {
#if __AIE_ARCH__ == 10
        return broadcast_bits_impl<TypeBits, T, Elems>::run(a);
#else
        return run(a.get());
#endif
    }
};

template <typename T, unsigned Elems>
using broadcast = broadcast_bits<type_bits_v<T>, T, Elems>;

template <unsigned TypeBits, typename T, unsigned Elems>
struct zeros_bits
{
    using vector_type = vector<T, Elems>;

    __aie_inline
    static vector_type run()
    {
#if __AIE_ARCH__ == 10
        if constexpr (vector_type::bits() == 128) {
            const vector<int32, 4> tmp(*(v4int32 *)ZERO);
            return vector_cast<T>(tmp);
        }
        else if constexpr (vector_type::bits() == 256) {
            const vector<int32, 8> tmp(*(v8int32 *)ZERO);
            return vector_cast<T>(tmp);
        }
        else if constexpr (vector_type::bits() == 512) {
            const vector<int32, 8> tmp(*(v8int32 *)ZERO);
            return vector_cast<T>(concat_vector(tmp, tmp));
        }
        else if constexpr (vector_type::bits() == 1024) {
            const vector<int32, 8> tmp(*(v8int32 *)ZERO);
            return vector_cast<T>(concat_vector(tmp, tmp, tmp, tmp));
        }
#else
        if constexpr (vector_type::is_complex())
            return zeros_bits_impl<TypeBits, T, Elems>::run();
        else
            return zeros_bits_impl<TypeBits, T, Elems>::run();
#endif
    }
};

template <typename T, unsigned Elems>
using zeros = zeros_bits<type_bits_v<T>, T, Elems>;


template <AccumClass Class>
struct zeros_type_for_accum;

template <AccumClass Class>
using zeros_type_for_accum_t = typename zeros_type_for_accum<Class>::type;

template <> struct zeros_type_for_accum<AccumClass::Int>  { using type = int32;    };
template <> struct zeros_type_for_accum<AccumClass::CInt> { using type = cint32;   };
#if __AIE_ARCH__ == 10
template <> struct zeros_type_for_accum<AccumClass::FP>   { using type = float;    };
#elif __AIE_ARCH__ == 20
template <> struct zeros_type_for_accum<AccumClass::FP>   { using type = float;    };
#elif __AIE_ARCH__ == 21
template <> struct zeros_type_for_accum<AccumClass::FP>   { using type = bfloat16; };
#endif
#if __AIE_ARCH__ == 10 || __AIE_API_COMPLEX_FP32_EMULATION__
template <> struct zeros_type_for_accum<AccumClass::CFP>  { using type = cfloat;   };
#endif

template <AccumClass Class, unsigned AccumBits, unsigned Elems>
struct zeros_acc_bits_impl;

template <AccumClass Class, unsigned AccumBits, unsigned Elems>
struct zeros_acc_bits
{
    using accum_tag  = accum_tag_t<Class, AccumBits>;
    using accum_type = accum<accum_tag, Elems>;

    __aie_inline
    static accum_type run()
    {
#if __AIE_ARCH__ == 10
        using zeros_type = zeros_type_for_accum_t<Class>;
        accum_type ret;

        constexpr unsigned num_accums = std::max(1u, (unsigned)(type_bits_v<zeros_type> * Elems) / 1024);

        accum<accum_tag, Elems / num_accums> tmp;
        tmp.from_vector(zeros<zeros_type, Elems / num_accums>::run());
        utils::unroll_for<unsigned, 0, num_accums>([&](auto idx) __aie_inline {
            ret.template insert<Elems / num_accums>(idx, tmp);
        });

        return ret;
#else
        return zeros_acc_bits_impl<Class, AccumBits, Elems>::run();
#endif
    }
};

template <AccumClass Class, unsigned AccumBits, unsigned Elems>
using zeros_acc = zeros_acc_bits<Class, AccumBits, Elems>;


}

#if __AIE_ARCH__ == 10

#include "aie1/broadcast.hpp"

#elif __AIE_ARCH__ == 20 || __AIE_ARCH__ == 21

#include "aie2/broadcast.hpp"

#endif

#endif
