// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

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

#if __AIE_ARCH__ == 20
        constexpr unsigned accum_bits = 1024;
        if constexpr (VecBits <= accum_bits)
            ret = accum_cast_helper<DstClass, DstBits, DstElems>(v) ;
        else
            ret = accum_cast_helper<DstClass, DstBits, DstElems>(utils::tuple_to_array(v.template split<2 * native_elems>()));
#else
        constexpr unsigned accum_bits = 2048;
        constexpr unsigned split_ratio = accum_bits / native_elems;
        if constexpr (VecBits <= 1024)
            ret = accum_cast_helper<DstClass, DstBits, DstElems>(v) ;
        else
            //ret = accum_cast_helper<DstClass, DstBits, DstElems>(utils::tuple_to_array(v.template split<2 * native_elems>()));
        {
            constexpr unsigned num_ops = std::max(1u, Elems / (2 * native_elems));
            auto arr = utils::tuple_to_array(v.template split<2 * native_elems>());
            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                ret.insert(idx, accum_cast_helper<DstClass, DstBits, 1024 / DstBits>(arr[idx]));
            });
        }

#endif

        return ret;
#endif
    }
};

template <typename DstT, AccumClass Class, unsigned Bits, unsigned Elems>
struct accum_to_vector_cast_bits_impl
{
    using  accum_tag = accum_tag_t<Class, Bits>;
    using accum_type = accum<accum_tag, Elems>;

    static constexpr unsigned vector_elems = accum_type::bits() / type_bits_v<DstT>;
    using vector_type = vector<DstT, vector_elems>;

    static vector_type run(const accum_type &acc)
    {
        using vec_storage_t = vector_storage_t<DstT, vector_elems>;
        constexpr unsigned num_cast_chunks = utils::num_elems_v<vec_storage_t>;

        if constexpr (num_cast_chunks == 1) {
            return vector_cast_helper<DstT, vector_elems>(acc);
        }
        else {
            constexpr unsigned acc_chunk_elems = Elems / num_cast_chunks;
            constexpr unsigned vec_chunk_elems = vector_elems / num_cast_chunks;

            return utils::apply_tuple(
                    [](auto &&... accs) -> vec_storage_t {
                        return {vector_cast_helper<DstT, vec_chunk_elems>(accs)...};
                    },
                    acc.template split<acc_chunk_elems>());
        }
    }
};

}

#endif
