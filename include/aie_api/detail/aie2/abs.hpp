// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_ABS__HPP__
#define __AIE_API_DETAIL_AIE2_ABS__HPP__

#include "../broadcast.hpp"

namespace aie::detail {

template <unsigned TypeBits, typename T, unsigned Elems>
struct abs_bits_common_impl
{
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v)
    {
        vector_type ret;

        constexpr unsigned native_elems = max_intrinsic_vector_elems_v<T, Elems>;
        using native_op = abs_bits_impl<TypeBits, T, native_elems>;

        if constexpr (vector_type::is_signed()) {
            if constexpr (Elems < native_elems) {
                ret = native_op::run(v.template grow<native_elems>()).template extract<Elems>(0);
            }
            else if constexpr (Elems == native_elems) {
                ret = ::abs(v);
            }
            else {
                utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                    ret.template insert<native_elems>(idx, native_op::run(v.template extract<native_elems>(idx)));
                });
            }
        }
        else {
            ret = v;
        }

        return ret;
    }
};

template <typename T, unsigned Elems> struct abs_bits_impl<8,  T, Elems> : public abs_bits_common_impl<8,  T, Elems> {};
template <typename T, unsigned Elems> struct abs_bits_impl<16, T, Elems> : public abs_bits_common_impl<16, T, Elems> {};
template <typename T, unsigned Elems> struct abs_bits_impl<32, T, Elems> : public abs_bits_common_impl<32, T, Elems> {};

}

#endif
