// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_MAX_MIN_REDUCE__HPP__
#define __AIE_API_DETAIL_AIE2_MAX_MIN_REDUCE__HPP__

#include "../max_min.hpp"
#include "../shuffle.hpp"
#include "../vector.hpp"
#include "../utils.hpp"

namespace aie::detail {

template <typename T, unsigned Elems, MaxMinOperation Op>
struct max_min_bits_reduce_impl<4, T, Elems, Op>
{
    using vector_type = vector<T, Elems>;

    __aie_inline
    static T run(const vector_type &v)
    {
        using next_type = utils::get_next_integer_type_t<T>;

        return (T) max_min_bits_reduce_impl<8, next_type, Elems, Op>::run(v.unpack());
    }
};

template <typename T, unsigned Elems, MaxMinOperation Op>
struct max_min_bits_reduce_impl<8, T, Elems, Op>
{
    static constexpr unsigned native_elems = native_vector_length_v<T>;
    static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);

    using vector_type = vector<T, Elems>;
    using native_impl = max_min_bits_reduce_impl<8, T, native_elems, Op>;

    static constexpr auto op = get_max_min_op<Op>();

    __aie_inline
    static T run(const vector_type &v)
    {
        constexpr auto op = get_max_min_op<Op>();

        if constexpr (Elems <= native_elems) {
            vector<T, native_elems> tmp = v.template grow<native_elems>();

            utils::unroll_times<utils::log2(Elems)>([&](auto idx) __aie_inline {
                tmp = op(tmp, SHIFT_BYTES(tmp, vector<T, native_elems>(), Elems >> (idx + 1)));
            });

            return (T) tmp[0];
        }
        else {
            using cmp_impl = max_min_bits_impl<8, T, native_elems, Op>;

            vector<T, native_elems> tmp = cmp_impl::run(v.template extract<native_elems>(0),
                                                        v.template extract<native_elems>(1));

            utils::unroll_times<num_ops - 2>([&](unsigned idx) __aie_inline {
                tmp = cmp_impl::run(tmp,
                                    v.template extract<native_elems>(2 + idx));
            });

            return native_impl::run(tmp);
        }
    }
};

template <typename T, unsigned Elems, MaxMinOperation Op>
struct max_min_bits_reduce_impl<16, T, Elems, Op>
{
    static constexpr unsigned native_elems = native_vector_length_v<T>;
    static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);

    using vector_type = vector<T, Elems>;
    using native_impl = max_min_bits_reduce_impl<16, T, native_elems, Op>;

    static constexpr auto op = get_max_min_op<Op>();

    __aie_inline
    static T run(const vector_type &v)
    {
        if constexpr (Elems <= native_elems) {
            vector<T, native_elems> tmp = v.template grow<native_elems>();

            utils::unroll_times<utils::log2(Elems)>([&](auto idx) __aie_inline {
                tmp = op(tmp, SHIFT_BYTES(tmp, vector<T, native_elems>(), (Elems >> (idx + 1)) * sizeof(T)));
            });

            return (T) tmp[0];
        }
        else {
            using cmp_impl = max_min_bits_impl<16, T, native_elems, Op>;

            vector<T, native_elems> tmp = cmp_impl::run(v.template extract<native_elems>(0),
                                                        v.template extract<native_elems>(1));

            utils::unroll_times<num_ops - 2>([&](unsigned idx) __aie_inline {
                tmp = cmp_impl::run(tmp,
                                    v.template extract<native_elems>(2 + idx));
            });

            return native_impl::run(tmp);
        }
    }
};

template <typename T, unsigned Elems, MaxMinOperation Op>
struct max_min_bits_reduce_impl<32, T, Elems, Op>
{
    static constexpr unsigned native_elems = native_vector_length_v<T>;
    static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);

    using vector_type = vector<T, Elems>;
    using native_impl = max_min_bits_reduce_impl<32, T, native_elems, Op>;

    static constexpr auto op = get_max_min_op<Op>();

    __aie_inline
    static T run(const vector_type &v)
    {
        if constexpr (Elems <= native_elems) {
            vector<T, native_elems> tmp = v.template grow<native_elems>();

            utils::unroll_times<utils::log2(Elems)>([&](auto idx) __aie_inline {
                tmp = op(tmp, SHIFT_BYTES(tmp, vector<T, native_elems>(), (Elems >> (idx + 1)) * sizeof(T)));
            });

            return (T) tmp[0];
        }
        else {
            using cmp_impl = max_min_bits_impl<32, T, native_elems, Op>;

            vector<T, native_elems> tmp = cmp_impl::run(v.template extract<native_elems>(0),
                                                        v.template extract<native_elems>(1));

            utils::unroll_times<num_ops - 2>([&](unsigned idx) __aie_inline {
                tmp = cmp_impl::run(tmp,
                                    v.template extract<native_elems>(2 + idx));
            });

            return native_impl::run(tmp);
        }
    }
};

}

#endif
