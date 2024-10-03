// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_VECTOR_ACCUM_CAST__HPP__
#define __AIE_API_DETAIL_AIE2_VECTOR_ACCUM_CAST__HPP__

#include "../vector.hpp"
#include "../accum.hpp"

namespace aie::detail {

template <typename DstTag, typename T, unsigned Elems>
struct vector_to_accum_cast_bits_impl
{
    using vector_type = vector<T, Elems>;

    static auto run(const vector_type &v)
    {
#if 1
        constexpr unsigned native_cast_bits  = 1024;
        constexpr unsigned native_cast_elems = native_cast_bits / type_bits_v<T>;

        constexpr unsigned TotalBits     = Elems * type_bits_v<T>;
        constexpr unsigned AccumElemBits = to_native_value_bits<DstTag>();
        constexpr unsigned DstElems      = (AccumElemBits <= type_bits_v<T> ? Elems * (type_bits_v<T> / AccumElemBits) :
                                                                              Elems / ( AccumElemBits / type_bits_v<T>));

        constexpr AccumClass DstClass = accum_class_for_tag_v<DstTag>;
        constexpr unsigned   DstBits  = to_native_accum_bits<DstTag>();

        accum<DstTag, DstElems> ret;

        if constexpr (TotalBits <= native_cast_bits) {
            ret = accum_cast_helper<DstClass, DstBits, DstElems>(v);
        }
        else {
            constexpr unsigned native_cast_accum_elems = native_cast_bits / AccumElemBits;
            constexpr unsigned num_ops = Elems / native_cast_elems;

            auto arr = utils::tuple_to_array(v.template split<native_cast_elems>());

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                ret.insert(idx, accum_cast_helper<DstClass, DstBits, native_cast_accum_elems>(arr[idx]));
            });
        }

        return ret;

#else
        constexpr unsigned VecBits =  Elems * type_bits_v<T>;
        constexpr unsigned native_elems = std::min(Elems, native_vector_length_v<T>);

        constexpr unsigned AccumBits = to_native_value_bits<DstTag>();
        constexpr unsigned DstElems  = (AccumBits <= type_bits_v<T> ? Elems * (type_bits_v<T> / AccumBits) :
                                                                      Elems / (     AccumBits / type_bits_v<T>));

        constexpr AccumClass DstClass = accum_class_for_tag_v<DstTag>;
        constexpr unsigned   DstBits  = to_native_accum_bits<DstTag>();

        accum<DstTag, DstElems> ret;

        constexpr unsigned accum_bits = 1024;
        if constexpr (VecBits <= accum_bits)
            ret = accum_cast_helper<DstClass, DstBits, DstElems>(v) ;
        else
            ret = accum_cast_helper<DstClass, DstBits, DstElems>(utils::tuple_to_array(v.template split<2 * native_elems>()));

        return ret;
#endif
    }
};

template <typename DstT, AccumClass Class, unsigned Bits, unsigned Elems>
struct accum_to_vector_cast_bits_impl
{
    using  accum_tag = accum_tag_t<Class, Bits>;
    using accum_type = accum<accum_tag, Elems>;

    static auto run(const accum_type &acc)
    {
        constexpr unsigned VecElems = accum_type::bits() / type_bits_v<DstT>;
        vector<DstT, VecElems> ret;

        if constexpr (accum_type::bits() <= 512) {
            ret = vector_cast_helper<DstT, VecElems>(acc);
        }
        else {
            constexpr unsigned chunks          = accum_type::bits() / 512;
            constexpr unsigned elems_per_chunk = Elems / chunks;

            const std::array acc_array = utils::tuple_to_array(acc.template split<elems_per_chunk>());

            ret = vector_cast_helper<DstT, VecElems>(acc_array);
        }
        return ret;
    }
};

}

#endif
