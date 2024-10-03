// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_VECTOR_ACCUM_CAST__HPP__
#define __AIE_API_DETAIL_VECTOR_ACCUM_CAST__HPP__

#include "accum.hpp"
#include "vector.hpp"

namespace aie::detail {

template <typename DstTag, typename T, unsigned Elems>
struct vector_to_accum_cast_bits_impl;

template <typename DstT, AccumClass Class, unsigned Bits, unsigned Elems>
struct accum_to_vector_cast_bits_impl;

template <typename DstTag, typename T, unsigned Elems>
struct vector_to_accum_cast_bits
{
    using vector_type = vector<T,  Elems>;

    static auto run(const vector_type &v)
    {
        return vector_to_accum_cast_bits_impl<DstTag, T, Elems>::run(v);
    }
};

template <typename DstTag, typename T, unsigned Elems>
using vector_to_accum_cast = vector_to_accum_cast_bits<DstTag, T, Elems>;

template <typename DstT, AccumClass Class, unsigned Bits, unsigned Elems>
struct accum_to_vector_cast_bits
{
    using  accum_tag = accum_tag_t<Class, Bits>;
    using accum_type = accum<accum_tag, Elems>;

    static auto run(const accum_type &acc)
    {
        return accum_to_vector_cast_bits_impl<DstT, Class, Bits, Elems>::run(acc);
    }
};

template <typename DstT, typename Tag, unsigned Elems>
using accum_to_vector_cast = accum_to_vector_cast_bits<DstT,
                                                       accum_class_for_tag_v<Tag>,
                                                       accum_bits_for_tag_v<Tag>,
                                                       Elems>;

}

#if __AIE_ARCH__ == 20

#include "aie2/vector_accum_cast.hpp"

#endif

#endif
