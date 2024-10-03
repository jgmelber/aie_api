// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_FILTER__HPP__
#define __AIE_API_DETAIL_AIE2_FILTER__HPP__

#include "shuffle_mode.hpp"

#include "../utils.hpp"
#include "../vector.hpp"

namespace aie::detail {

template <typename T, unsigned Elems, FilterOp Op>
struct filter_bits_impl<4, T, Elems, Op>
{
    using vector_type = vector<T, Elems>;
    using result_type = vector<T, Elems / 2>;

    __aie_inline
    static result_type run(const vector_type &v, unsigned step)
    {
        using next_type   = utils::get_next_integer_type_t<T>;
        using filter_impl = filter_bits_impl<8, next_type, Elems / 2, Op>;

        REQUIRES_MSG(step > 1, "Sub-byte vector filter is not supported");
        return filter_impl::run(v.template cast_to<next_type>(), step / 2).template cast_to<T>();
    }
};

template <typename T, unsigned Elems>
struct filter_bits_impl<4, T, Elems, FilterOp::Dynamic>
{
    using vector_type = vector<T, Elems>;
    using result_type = vector<T, Elems / 2>;

    __aie_inline
    static result_type run(const vector_type &v, const filter_mode<4, Elems> &mode)
    {
        using next_type = utils::get_next_integer_type_t<T>;
        using filter_impl = filter_bits_impl<8, next_type, Elems / 2, FilterOp::Dynamic>;

        // filter_mode cannot be constructed with sub-byte steps, so we can
        // operate directly in int8 vectors
        filter_mode<8, Elems / 2> m{mode};
        return filter_impl::run(v.template cast_to<next_type>(), m).template cast_to<T>();
    }
};

template <unsigned TypeBits, typename T, unsigned Elems, FilterOp Op>
struct filter_bits_impl_common
{
    static constexpr unsigned native_elems = 512 / TypeBits;

    using vector_type = vector<T, Elems>;
    using result_type = vector<T, Elems / 2>;
    using  filter_tag = std::conditional_t<Op == FilterOp::Even, filter_even_tag, filter_odd_tag>;
    using   impl_type = filter_bits_impl_common<TypeBits, T, Elems, FilterOp::Dynamic>;

    __aie_inline
    static result_type run(const vector_type &v, unsigned step)
    {
        const filter_mode<TypeBits, Elems> f{step, filter_tag{}};
        return impl_type::run(v, f);
    }
};

template <unsigned TypeBits, typename T, unsigned Elems>
struct filter_bits_impl_common<TypeBits, T, Elems, FilterOp::Dynamic>
{
    static constexpr unsigned native_elems = 512 / TypeBits;
    static constexpr unsigned      num_ops = Elems / (2 * native_elems);
    using vector_type = vector<T, Elems>;
    using result_type = vector<T, Elems / 2>;

    __aie_inline
    static result_type run(const vector_type &v, const filter_mode<TypeBits, Elems> &filter)
    {
        result_type result;
        if constexpr (vector_type::bits() <= 512) {
            vector<T, native_elems> tmp = v.template grow<native_elems>();
            tmp = ::shuffle(tmp, tmp, filter.mode);

            result = tmp.template extract<Elems / 2>(0);
        }
        else if constexpr (vector_type::bits() == 1024) {
            result = ::shuffle(v.template extract<native_elems>(0),
                               v.template extract<native_elems>(1),
                               filter.mode);
        }
        else if constexpr (vector_type::bits() > 1024) {
            
            if (!chess_manifest(filter.step * TypeBits <= 512)) {
                if (filter.step * TypeBits >= 1024) {
                    unsigned native_blocks = filter.step / native_elems;

                    unsigned out_idx = 0;
                    unsigned offset = filter.is_even ? 0 : native_blocks;
                    for (unsigned i = 0; i < Elems / filter.step / 2; ++i)
                        for (unsigned j = 0; j < native_blocks; ++j)
                            result.insert(out_idx++, v.template extract<native_elems>(offset + i * 2 * native_blocks + j));

                    return result;
                }
            }

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                vector<T, native_elems> tmp = ::shuffle(v.template extract<native_elems>(2 * idx + 0),
                                                        v.template extract<native_elems>(2 * idx + 1),
                                                        filter.mode);

                result.insert(idx, tmp);
            });
        }

        return result;
    }
};


template <typename T, unsigned Elems, FilterOp Op> struct filter_bits_impl< 8, T, Elems, Op> : public filter_bits_impl_common< 8, T, Elems, Op> {};
template <typename T, unsigned Elems, FilterOp Op> struct filter_bits_impl<16, T, Elems, Op> : public filter_bits_impl_common<16, T, Elems, Op> {};
template <typename T, unsigned Elems, FilterOp Op> struct filter_bits_impl<32, T, Elems, Op> : public filter_bits_impl_common<32, T, Elems, Op> {};
template <typename T, unsigned Elems, FilterOp Op> struct filter_bits_impl<64, T, Elems, Op> : public filter_bits_impl_common<64, T, Elems, Op> {};

}

#endif
