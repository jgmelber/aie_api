// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_CONJ__HPP__
#define __AIE_API_DETAIL_AIE2_CONJ__HPP__

namespace aie::detail {

template <unsigned TypeBits, typename T, unsigned Elems>
struct conj_bits_impl_common
{
    using vector_type = vector<T, Elems>;
    using component_type = utils::get_complex_component_type_t<T>;

    static vector_type run(const vector_type &v)
    {
        // Cast the input vector to a vector of the underlying component type
        // containing twice as many elements. ::addsub is used to add the real
        // components to, and subtract the imaginary components from, the zero
        // vector. The result is cast back to the input type to return the
        // conjugate.
        //
        // Example:
        //   input                   [a1 + b1i, a2 + b2i, ... aN + bNi]
        //   cast to component type  [a1 , b1 , a2 , b2 , ... aN , bN ]
        //   addsub mask              +    -    +    -        +    -
        //   zero                    [0  , 0  , 0  , 0  , ... 0  , 0  ]
        //   addsub result           [a1 ,-b1 , a2 ,-b2 , ... aN ,-bN ]
        //   cast back to input type [a1 - b1i, a2 - b2i, ... aN - bNi]
        constexpr unsigned elems_per_vector = 512 / TypeBits;
        constexpr unsigned num_components   = 2 * elems_per_vector;
        constexpr unsigned num_it           = Elems < elems_per_vector? 1 : Elems / elems_per_vector;

        const vector zero = zeros<component_type, num_components>::run();
        const auto values = v.template cast_to<component_type>();

        vector_type ret;

        utils::unroll_times<num_it>([&](auto idx) __aie_inline {
            vector<component_type, num_components> tmp = ::addsub(zero, values.template grow_extract<num_components>(idx), 0xaaaaaaaa);

            ret.insert(idx, tmp.template cast_to<T>().template extract<(Elems < elems_per_vector)? Elems : elems_per_vector>(0));
        });

        return ret;
    }
};

template <unsigned Elems> struct conj_bits_impl<32, cint16, Elems> : public conj_bits_impl_common<32, cint16, Elems> {};
template <unsigned Elems> struct conj_bits_impl<64, cint32, Elems> : public conj_bits_impl_common<64, cint32, Elems> {};

}

#endif
