// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

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

template <unsigned TypeBits, typename T, unsigned Elems, FilterOp Op>
struct filter_accum_bits_impl_common
{
    static constexpr unsigned native_elems = 512 / TypeBits;

    using accum_type   = accum<T, Elems>;
    using result_type  = accum<T, Elems / 2>;
    using cast_acc_tag = std::conditional_t<accum_type::is_floating_point(), accfloat, acc32>;

    using value_type   = std::conditional_t<accum_type::is_floating_point(), float, uint32>;
    using vector_type  = vector<value_type, accum_type::bits() / type_bits_v<value_type>>;

    using  filter_tag = std::conditional_t<Op == FilterOp::Even, filter_even_tag, filter_odd_tag>;
    using   impl_type = filter_bits_impl_common<vector_type::type_bits(), value_type, vector_type::size(), FilterOp::Dynamic>;

    __aie_inline
    static result_type run(const accum_type &acc, unsigned step)
    {
        const filter_mode<TypeBits, Elems> f{step, filter_tag{}};
        return filter_accum_bits_impl_common<TypeBits, T, Elems, FilterOp::Dynamic>::run(acc, f);
    }
};

template <unsigned TypeBits, typename T, unsigned Elems>
struct filter_accum_bits_impl_common<TypeBits, T, Elems, FilterOp::Dynamic>
{
    static constexpr unsigned native_elems = 512 / TypeBits;
    static constexpr unsigned num_ops      = Elems / (2 * native_elems);

    using accum_type  = accum<T, Elems>;
    using result_type = accum<T, Elems / 2>;

    using cast_acc_tag = std::conditional_t<accum_type::is_floating_point(), accfloat, acc32>;
    using cast_vec_elem_t = std::conditional_t<accum_type::is_floating_point(), float, uint32>;

    template <unsigned E2>
    __aie_inline
    static auto to_vector(const accum<T, E2> &acc)
    {
        return acc.template cast_to<cast_acc_tag>().template to_vector<cast_vec_elem_t>();
    }

    template <unsigned E2>
    __aie_inline
    static auto to_accum(const vector<cast_vec_elem_t, E2> &v)
    {
        accum<cast_acc_tag, E2> acc;
        acc.from_vector(v);
        return acc.template cast_to<T>();
    }

    __aie_inline
    static result_type run(const accum_type &acc, const filter_mode<TypeBits, Elems> &filter)
    {
        if constexpr (Elems <= native_elems) {
            auto tmp = to_vector(acc.template grow<native_elems>());
            tmp = ::shuffle(tmp, tmp, filter.mode);
            return to_accum(tmp).template extract<Elems / 2>(0);
        }
        else if constexpr (Elems == 2 * native_elems) {
            auto v = to_vector(acc.template extract<native_elems>(0));
            auto w = to_vector(acc.template extract<native_elems>(1));
            w = ::shuffle(v, w, filter.mode);
            return to_accum(w);
        }
        else {
            if (!chess_manifest(filter.step * TypeBits <= 512)) {
                auto v = to_vector(acc);
                using filter_op = filter_bits_impl_common<decltype(v)::value_bits(),
                                                          cast_vec_elem_t,
                                                          decltype(v)::size(),
                                                          FilterOp::Dynamic>;
                auto w = filter_op::run(v, filter);
                return to_accum(w);
            }

            using native_impl = filter_accum_bits_impl_common<TypeBits, T, native_elems, FilterOp::Dynamic>;

            result_type result;
            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                result.insert(idx,
                              native_impl::run(acc.template extract<2 * native_elems>(idx), filter));
            });
            return result;
        }
    }
};

template <unsigned Elems, FilterOp Op> struct filter_bits_impl< 32, accfloat,  Elems, Op> : public filter_accum_bits_impl_common< 32, accfloat,  Elems, Op> {};
template <unsigned Elems, FilterOp Op> struct filter_bits_impl< 32, acc32,     Elems, Op> : public filter_accum_bits_impl_common< 32, acc32,     Elems, Op> {};
template <unsigned Elems, FilterOp Op> struct filter_bits_impl< 64, acc64,     Elems, Op> : public filter_accum_bits_impl_common< 64, acc64,     Elems, Op> {};
#if __AIE_API_COMPLEX_VECTOR_SUPPORT__
template <unsigned Elems, FilterOp Op> struct filter_bits_impl< 64, caccfloat, Elems, Op> : public filter_accum_bits_impl_common< 64, caccfloat, Elems, Op> {};
template <unsigned Elems, FilterOp Op> struct filter_bits_impl<128, cacc64,    Elems, Op> : public filter_accum_bits_impl_common<128, cacc64,    Elems, Op> {};
#endif

}

#endif
