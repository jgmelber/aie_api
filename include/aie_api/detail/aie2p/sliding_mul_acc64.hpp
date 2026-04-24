// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2P_SLIDING_MUL_ACC64_HPP__
#define __AIE_API_DETAIL_AIE2P_SLIDING_MUL_ACC64_HPP__

#include <algorithm>

#include "../mul.hpp"
#include "../vector.hpp"

namespace aie::detail {

template <MulMacroOp MulOp, unsigned Lanes, int DataStep, typename CoeffType, typename DataType>
static constexpr auto sliding_mul_acc48_get_mul_op()
{
    if constexpr (utils::is_one_of_v<CoeffType, int16, uint16> && utils::is_one_of_v<DataType, int16, uint16>) {
        if constexpr (MulOp == MulMacroOp::Mul)     return [](auto &&... args) __aie_inline { return ::mul_conv_32x4_conf(args..., /*sub_mul=*/0); };
        if constexpr (MulOp == MulMacroOp::NegMul)  return [](auto &&... args) __aie_inline { return ::negmul_conv_32x4_conf(args..., /*sub_mul=*/0); };
        if constexpr (MulOp == MulMacroOp::Add_Mul) return [](auto &&... args) __aie_inline { return ::mac_conv_32x4_conf(args..., /*shift16=*/0, /*sub_mul=*/0, /*sub_acc1=*/0); };
        if constexpr (MulOp == MulMacroOp::Sub_Mul) return [](auto &&... args) __aie_inline { return ::msc_conv_32x4_conf(args..., /*shift16=*/0, /*sub_mul=*/0, /*sub_acc1=*/0); };
    }
    else {
        //return [](auto &&... args) __aie_inline { return mul<MulOp, 64, DataType, CoeffType>::run(args...); };
        if constexpr (MulOp == MulMacroOp::Mul || MulOp == MulMacroOp::NegMul)
            return [](auto v1, bool v1_sign, auto v2, bool v2_sign) __aie_inline { return mul<MulOp, 64, DataType, CoeffType>::run(v1, v1_sign, v2, v2_sign); };
        if constexpr (MulOp == MulMacroOp::Add_Mul || MulOp == MulMacroOp::Sub_Mul)
            return [](auto v1, bool v1_sign, auto v2, bool v2_sign, auto acc, bool zero_acc) __aie_inline { return mul<MulOp, 64, DataType, CoeffType>::run(v1, v1_sign, v2, v2_sign, false, false, zero_acc, acc); };
    }
}

template <MulMacroOp MulOp, unsigned Lanes, int DataStep, typename CoeffType, typename DataType>
static constexpr auto sliding_mul_acc48_get_mul_conf_op()
{
    if constexpr (utils::is_one_of_v<CoeffType, int16, uint16> && utils::is_one_of_v<DataType, int16, uint16>) {
        if constexpr (MulOp == MulMacroOp::Mul)     return [](auto &&... args) __aie_inline { return ::mul_conv_32x4_conf(args...); };
        if constexpr (MulOp == MulMacroOp::NegMul)  return [](auto &&... args) __aie_inline { return ::negmul_conv_32x4_conf(args...); };
        if constexpr (MulOp == MulMacroOp::Add_Mul) return [](auto &&... args) __aie_inline { return ::mac_conv_32x4_conf(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_Mul) return [](auto &&... args) __aie_inline { return ::msc_conv_32x4_conf(args...); };
    }
    else {
        return [](auto &&... args) __aie_inline { return mul<MulOp, 64, DataType, CoeffType>::run(args...); };
    }
}

// 16b coeff * 16b data - leverage convolution mode
template <unsigned Lanes, unsigned Points, typename CoeffType, typename DataType>
struct sliding_mul_bits_impl<Lanes, Points, 1, 1, 1, 64, 16, 16, CoeffType, DataType>
{
    using coeff_type   = CoeffType;
    using data_type    = DataType;
    using accum_tag    = accum_tag_for_mul_types<data_type, coeff_type, 64>;
    using accum_type   = accum<accum_tag, Lanes>;
    using accum_type32 = accum<accum_tag, 32>;

    static constexpr unsigned   lanes_per_mul = 32;
    static constexpr unsigned columns_per_mul = 4;
    static constexpr unsigned         num_mul = Points / columns_per_mul;
    static_assert(Points >= columns_per_mul);
    static_assert(Points % columns_per_mul == 0);

    static constexpr unsigned native_data_elems  = 64;
    static constexpr unsigned native_coeff_elems = native_vector_length_v<coeff_type>;

    template <typename T, unsigned Elems, unsigned OutElems = Elems>
    static auto prepare_data(vector<T, Elems> input, unsigned offset)
    {
        // Avoid 1024b shuffle_down_rotate where possible
        if constexpr (Elems <= lanes_per_mul) {
            auto tmp = shuffle_down_rotate<T, lanes_per_mul>::run(input.template grow_replicate<lanes_per_mul>(), offset);
            if constexpr (Lanes <= 16)
                return tmp.template grow<OutElems>();
            else
                return tmp.template grow_replicate<OutElems>();
        }
        else {
            return shuffle_down_rotate<T, OutElems>::run(input.template grow_replicate<OutElems>(), offset);
        }
    };

    template <typename T, unsigned Elems>
    static auto prepare_coeff(vector<T, Elems> input, unsigned offset)
    {
        // Avoid 1024b shuffle_down_rotate where possible
        if constexpr (Elems <= lanes_per_mul) {
            return shuffle_down_rotate<T, native_coeff_elems>::run(input.template grow_replicate<native_coeff_elems>(), offset);
        }
        else {
            return shuffle_down_rotate<T, Elems>::run(input, offset).template extract<native_coeff_elems>(0);
        }
    };


    template <MulMacroOp MulOp, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires(is_accum_v<Acc> && ...)
    static accum_type run(const vector<coeff_type, N_Coeff> &coeff,
                          unsigned coeff_start,
                          bool coeff_sign,
                          const vector<data_type, N_Data> &data,
                          unsigned data_start,
                          bool data_sign,
                          bool zero_acc,
                          const Acc &... acc)
    {
        constexpr auto mac_op = sliding_mul_acc48_get_mul_op<add_to_op<MulOp>(), lanes_per_mul, 1, coeff_type, data_type>();
        constexpr auto mul_op = sliding_mul_acc48_get_mul_op<MulOp,              lanes_per_mul, 1, coeff_type, data_type>();

        accum_type ret(acc...);

        utils::unroll_for<unsigned, 0, std::max(1u, Lanes / lanes_per_mul)>([&](auto idx_y) __aie_inline {
            const unsigned  data_start_local = (data_start + idx_y * lanes_per_mul) % N_Data;
            const unsigned coeff_start_local = coeff_start % N_Coeff;
            accum_type32 tmp;

            auto  data_tmp = prepare_data<data_type,   N_Data,  64>(data, data_start_local);
            auto coeff_tmp = prepare_coeff<coeff_type, N_Coeff>(coeff, coeff_start_local);
            
            tmp = mul_op(data_tmp,  data_sign,
                         coeff_tmp, coeff_sign,
                         utils::get_nth<0>(ret.template grow_extract<lanes_per_mul>(idx_y), acc)...,
                         utils::get_nth<0>(zero_acc, acc)...);

            utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
                const unsigned  data_start_local =  (data_start + columns_per_mul * idx + idx_y * lanes_per_mul) % N_Data;
                const unsigned coeff_start_local = (coeff_start + columns_per_mul * idx) % N_Coeff;
                
                auto  data_tmp = prepare_data<data_type,   N_Data,  64>(data, data_start_local);
                auto coeff_tmp = prepare_coeff<coeff_type, N_Coeff>(coeff, coeff_start_local);

                tmp = mac_op(data_tmp,  data_sign,
                             coeff_tmp, coeff_sign,
                             tmp, false);
            });

            if constexpr (Lanes <= lanes_per_mul)
                ret = tmp.template extract<Lanes>(0);
            else
                ret.insert(idx_y, tmp);
        });

        return ret;
    }
};

// 32b coeff * 16b data
template <unsigned Lanes, unsigned Points, typename CoeffType, typename DataType>
    requires(!is_complex_v<CoeffType> && !is_complex_v<DataType>)
struct sliding_mul_bits_impl<Lanes, Points, 1, 1, 1, 64, 32, 16, CoeffType, DataType>
{
    using coeff_type   = CoeffType;
    using data_type    = DataType;
    using accum_tag    = accum_tag_for_mul_types<data_type, coeff_type, 64>;
    using accum_type   = accum<accum_tag, Lanes>;
    using accum_type32 = accum<accum_tag, 32>;

    static constexpr unsigned   lanes_per_mul = 32;
    static constexpr unsigned columns_per_mul = 1;
    static constexpr unsigned         num_mul = Points / columns_per_mul;
    static_assert(Points >= columns_per_mul);
    static_assert(Points % columns_per_mul == 0);

    static constexpr unsigned native_data_elems  = native_vector_length_v<data_type>;

    template <typename T, unsigned Elems>
    static auto prepare_data(vector<T, Elems> input, unsigned offset)
    {
        // Avoid 1024b shuffle_down_rotate where possible
        if constexpr (Elems <= 32) {
            return shuffle_down_rotate<T, 32>::run(input.template grow_replicate<32>(), offset);
        }
        else {
            return shuffle_down_rotate<T, Elems>::run(input, offset).template extract<32>(0);
        }
    };

    template <MulMacroOp MulOp, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires(is_accum_v<Acc> && ...)
    __aie_inline
    static accum_type run(const vector<coeff_type, N_Coeff> &coeff,
                          unsigned                           coeff_start,
                          bool                               coeff_sign,
                          const vector<data_type, N_Data>   &data,
                          unsigned                           data_start,
                          bool                               data_sign,
                          bool                               zero_acc,
                          const Acc &...                     acc)
    {
        constexpr unsigned data_elems = std::max(N_Data, native_data_elems);

        data_start = data_start % N_Data;

        constexpr auto mac_op = sliding_mul_acc48_get_mul_op<add_to_op<MulOp>(), lanes_per_mul, 1, coeff_type, data_type>();
        constexpr auto mul_op = sliding_mul_acc48_get_mul_op<MulOp,              lanes_per_mul, 1, coeff_type, data_type>();

        accum_type ret(acc...);

        auto  data_local = prepare_data(data, data_start % data_elems);

        auto shift_data = [](const auto &data_local, unsigned shift) __aie_inline {
            return shuffle_down_rotate<data_type, lanes_per_mul>::run(data_local, shift);
        };

        utils::unroll_for<unsigned, 0, std::max(1u, Lanes / lanes_per_mul)>([&](auto idx_y) __aie_inline {
            accum_type32 tmp;

            const unsigned coeff_start_local = coeff_start % N_Coeff;

            auto coeff_local = coeff.get(coeff_start_local);
            if (idx_y > 0)
                data_local = shift_data(data_local, lanes_per_mul);

            tmp = mul_op(data_local,   data_sign,
                         coeff_local, coeff_sign,
                         utils::get_nth<0>(ret.template grow_extract<lanes_per_mul>(idx_y), acc)...,
                         utils::get_nth<0>(zero_acc, acc)...);

            utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
                const unsigned coeff_start_local = (coeff_start + columns_per_mul * idx) % N_Coeff;

                data_local       = shift_data(data_local, columns_per_mul);
                auto coeff_local = coeff.get(coeff_start_local);

                tmp = mac_op(data_local,   data_sign,
                             coeff_local, coeff_sign,
                             tmp, false);
            });

            if constexpr (Lanes <= lanes_per_mul)
                ret = tmp.template extract<Lanes>(0);
            else
                ret.insert(idx_y, tmp);
        });

        return ret;
    }
};

// 16b coeff * 32b data
// 32b coeff * 32b data
template <unsigned Lanes, unsigned Points, unsigned CoeffBits, typename CoeffType, typename DataType>
    requires(!is_complex_v<CoeffType> && !is_complex_v<DataType> && (CoeffBits == 16 || CoeffBits == 32))
struct sliding_mul_bits_impl<Lanes, Points, 1, 1, 1, 64, CoeffBits, 32, CoeffType, DataType>
{
    using coeff_type   = CoeffType;
    using data_type    = DataType;
    using accum_tag    = accum_tag_for_mul_types<data_type, coeff_type, 64>;
    using accum_type   = accum<accum_tag, Lanes>;
    using accum_type32 = accum<accum_tag, 32>;

    static constexpr unsigned   lanes_per_mul = 32;
    static constexpr unsigned columns_per_mul = 1;
    static constexpr unsigned         num_mul = Points / columns_per_mul;
    static_assert(Points >= columns_per_mul);
    static_assert(Points % columns_per_mul  == 0);

    static constexpr unsigned native_data_elems  = native_vector_length_v<data_type>;

    template <MulMacroOp MulOp, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires(is_accum_v<Acc> && ...)
    __aie_inline
    static accum_type run(const vector<coeff_type, N_Coeff> &coeff,
                          unsigned                           coeff_start,
                          bool                               coeff_sign,
                          const vector<data_type, N_Data>   &data,
                          unsigned                           data_start,
                          bool                               data_sign,
                          bool                               zero_acc,
                          const Acc &...                     acc)
    {
        constexpr auto mac_op = sliding_mul_acc48_get_mul_op<add_to_op<MulOp>(), lanes_per_mul, 1, coeff_type, data_type>();
        constexpr auto mul_op = sliding_mul_acc48_get_mul_op<MulOp,              lanes_per_mul, 1, coeff_type, data_type>();

        accum_type ret(acc...);

        utils::unroll_for<unsigned, 0, std::max(1u, Lanes / lanes_per_mul)>([&](auto idx_y) __aie_inline {
            accum_type32 tmp;

            const unsigned  data_start_local = (data_start + idx_y * lanes_per_mul) % N_Data;
            const unsigned coeff_start_local = coeff_start % N_Coeff;

            auto data_local  = shuffle_down_rotate<data_type, N_Data>::run(data, data_start_local);
            auto coeff_local = coeff.get(coeff_start_local);

            tmp = mul_op(data_local.template grow_replicate<lanes_per_mul>(),  data_sign,
                         coeff_local,                                         coeff_sign,
                         utils::get_nth<0>(ret.template grow_extract<lanes_per_mul>(idx_y), acc)...,
                         utils::get_nth<0>(zero_acc, acc)...);

            utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
                const unsigned data_start_local  =  (data_start + columns_per_mul * idx + idx_y * lanes_per_mul) % N_Data;
                const unsigned coeff_start_local = (coeff_start + columns_per_mul * idx) % N_Coeff;

                auto data_local  = shuffle_down_rotate<data_type, N_Data>::run(data, data_start_local);
                auto coeff_local = coeff.get(coeff_start_local);

                tmp = mac_op(data_local.template grow_replicate<lanes_per_mul>(),  data_sign,
                             coeff_local,                                         coeff_sign,
                             tmp, false);
            });

            if constexpr (Lanes <= lanes_per_mul)
                ret = tmp.template extract<Lanes>(0);
            else
                ret.insert(idx_y, tmp);
        });

        return ret;
    }
};

#if __AIE_API_COMPLEX_VECTOR_SUPPORT__
// 16b coeff * c16b data - leverage convolution mode
template <unsigned Lanes, unsigned Points>
struct sliding_mul_bits_impl<Lanes, Points, 1, 1, 1, 64, 16, 32, int16, cint16>
{
    using coeff_type      = int16;
    using data_type       = cint16;
    using real_data_type  = int16;
    using accum_tag       = accum_tag_for_mul_types<data_type, coeff_type, 64>;
    using real_accum_tag  = accum_tag_for_mul_types<real_data_type, coeff_type, 64>;
    using accum_type32    = accum<accum_tag, 32>;
    using real_accum_type = accum<real_accum_tag, 32>;

    using accum_type       = accum<accum_tag, Lanes>;
    using partial_sum_type = std::array<accum<real_accum_tag, Lanes>, 2>;

    static constexpr unsigned   lanes_per_mul = 32;
    static constexpr unsigned columns_per_mul = 4;
    static constexpr unsigned         num_mul = Points / columns_per_mul;
    static_assert(Points >= columns_per_mul);
    static_assert(Points % columns_per_mul == 0);

    static constexpr unsigned native_data_elems  = 64;
    static constexpr unsigned native_coeff_elems = native_vector_length_v<coeff_type>;

    template <typename T, unsigned Elems, unsigned OutElems = Elems>
    static auto prepare_data(vector<T, Elems> input, unsigned offset)
    {
        // Avoid 1024b shuffle_down_rotate where possible
        if constexpr (Elems <= lanes_per_mul) {
            auto tmp = shuffle_down_rotate<T, lanes_per_mul>::run(input.template grow_replicate<lanes_per_mul>(), offset);
            if constexpr (Lanes <= 16)
                return tmp.template grow<OutElems>();
            else
                return tmp.template grow_replicate<OutElems>();
        }
        else {
            return shuffle_down_rotate<T, OutElems>::run(input.template grow_replicate<OutElems>(), offset);
        }
    };

    template <MulMacroOp MulOp, unsigned N_Coeff, unsigned N_Data, Accum... Acc>
    __aie_inline
    static accum_type run(const vector<coeff_type, N_Coeff> &coeff,
                          unsigned                           coeff_start,
                          bool                               coeff_sign,
                          const vector<data_type, N_Data>   &data,
                          unsigned                           data_start,
                          bool                               data_sign,
                          bool                               zero_acc,
                          const Acc &...                     acc)
    {
        // Make sure input accumulator has same accumulator element width
        static_assert((std::is_same_v<accum_native_type_t<real_accum_tag>,
                                      accum_native_type_t<typename remove_complex_t<Acc>::value_type>>
                       && ...));

        // Unzipping input accumulator
        // The following logic is done to cast the accumulator to real_accum_tag when it expands to something
        std::array<real_accum_type, sizeof...(Acc) * 2> acc_in;
        if constexpr (sizeof...(Acc) > 0) {
            acc_in = utils::apply_tuple(
                [](const auto &...v) __aie_inline {
                    return std::array{v.template grow_replicate<lanes_per_mul>().template cast_to<real_accum_tag>()...};
                },
                unzip_complex(acc)...);
        }
        std::array out = run<MulOp>(coeff, coeff_start, coeff_sign, data, data_start, data_sign, zero_acc,
                                    acc_in);

        // Re-shuffling real and imaginary
        return combine_into_complex(out);
    }

    template <MulMacroOp MulOp, unsigned N_Coeff, unsigned N_Data, Accum Acc, size_t N>
    __aie_inline
    static partial_sum_type run(const vector<coeff_type, N_Coeff> &coeff,
                                unsigned                           coeff_start,
                                bool                               coeff_sign,
                                const vector<data_type, N_Data>   &data,
                                unsigned                           data_start,
                                bool                               data_sign,
                                bool                               zero_acc,
                                const std::array<Acc, N>          &acc)
    {
        data_start = data_start % N_Data;

        constexpr auto mul_op      = sliding_mul_acc48_get_mul_op<MulOp,                   lanes_per_mul, 1, coeff_type, real_data_type>();
        constexpr auto mac_op      = sliding_mul_acc48_get_mul_op<add_to_op<MulOp>(),      lanes_per_mul, 1, coeff_type, real_data_type>();
        constexpr auto mul_conf_op = sliding_mul_acc48_get_mul_conf_op<MulOp,              lanes_per_mul, 1, coeff_type, real_data_type>();
        constexpr auto mac_conf_op = sliding_mul_acc48_get_mul_conf_op<add_to_op<MulOp>(), lanes_per_mul, 1, coeff_type, real_data_type>();

        constexpr unsigned num_conj = has_conj1<MulOp>()? 1 : 0;

        // Unzipping data
        vector<int16, N_Data> data_re, data_im;
        std::tie(data_re, data_im) = unzip_complex(data);

        // Unzipping input accumulator
        real_accum_type acc_re, acc_im;
        if constexpr (N > 0) {
            std::tie(acc_re, acc_im) = acc;
        }

        const unsigned coeff_start_local = coeff_start % N_Coeff;

        auto coeff_shuffle = shuffle_down_rotate<int16, native_coeff_elems>::run(coeff.template grow_replicate<native_coeff_elems>(), coeff_start_local);

        if constexpr (N > 0) {
            acc_re = mul_op(prepare_data<int16, N_Data, native_data_elems>(data_re, data_start),
                            true,
                            coeff_shuffle,
                            coeff_sign,
                            acc_re,
                            zero_acc);

            acc_im = mul_conf_op(prepare_data<int16, N_Data, native_data_elems>(data_im, data_start),
                                 true,
                                 coeff_shuffle,
                                 coeff_sign,
                                 acc_im,
                                 zero_acc,
                                 0,        // shift16
                                 num_conj, // sub_mul - negation of mul result for  -(coef.im*data.im)
                                 0);       // sub_acc
        }
        else {
            acc_re = mul_op(prepare_data<int16, N_Data, native_data_elems>(data_re, data_start),
                            true,
                            coeff_shuffle,
                            coeff_sign);

            acc_im = mul_conf_op(prepare_data<int16, N_Data, native_data_elems>(data_im, data_start),
                                 true,
                                 coeff_shuffle,
                                 coeff_sign,
                                 num_conj);      // sub_acc
        }

        utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
            const unsigned data_start_local  = ( data_start + columns_per_mul * idx) % N_Data;
            const unsigned coeff_start_local = (coeff_start + columns_per_mul * idx) % N_Coeff;

            coeff_shuffle = shuffle_down_rotate<int16, native_coeff_elems>::run(coeff.template grow_replicate<native_coeff_elems>(), coeff_start_local);

            acc_re = mac_op(prepare_data<int16, N_Data, native_data_elems>(data_re, data_start_local),
                            true,
                            coeff_shuffle,
                            coeff_sign,
                            acc_re, false);

            acc_im = mac_conf_op(prepare_data<int16, N_Data, native_data_elems>(data_im, data_start_local),
                                 true,
                                 coeff_shuffle,
                                 coeff_sign,
                                 acc_im,
                                 false,
                                 0,
                                 num_conj, //negation of mul result for  -(coef.im*data.im)
                                 0);
        });

        return {acc_re.template extract<Lanes>(0), acc_im.template extract<Lanes>(0)};
    }
};

// 16b coeff * c32b data
// 32b coeff * c16b data
// 32b coeff * c32b data
template <unsigned Lanes, unsigned Points, unsigned CoeffBits, unsigned DataBits, typename CoeffType, typename DataType>
    requires(!is_complex_v<CoeffType> && is_complex_v<DataType>)
struct sliding_mul_bits_impl<Lanes, Points, 1, 1, 1, 64, CoeffBits, DataBits, CoeffType, DataType>
{
    using coeff_type        = CoeffType;
    using data_type         = DataType;
    using real_data_type    = std::conditional_t<std::is_same_v<data_type, cint16>, int16, int32>;
    using accum_tag         = accum_tag_for_mul_types<data_type, coeff_type, 64>;
    using accum_type        = accum<accum_tag, Lanes>;
    using real_accum_tag    = accum_tag_for_mul_types<real_data_type, coeff_type, 64>;
    using real_accum_type32 = accum<real_accum_tag, 32>;

    static constexpr unsigned   lanes_per_mul = 16;
    static constexpr unsigned columns_per_mul = 1;
    static constexpr unsigned         num_mul = Points / columns_per_mul;
    static_assert(Points >= columns_per_mul);
    static_assert(Points % columns_per_mul == 0);

    template <unsigned Elems, unsigned OutElems = Elems>
    static auto prepare_data(vector<data_type, Elems> input, unsigned offset)
    {
        constexpr unsigned native_elems = native_vector_length_v<data_type>;
        // Avoid 1024b shuffle_down_rotate where possible
        if constexpr (Elems <= native_elems) {
            auto tmp = shuffle_down_rotate<data_type, native_elems>::run(input.template grow_replicate<native_elems>(), offset);
            return tmp.template grow_replicate<OutElems>();
        }
        else {
            return shuffle_down_rotate<data_type, Elems>::run(input, offset).template grow_replicate<std::max(OutElems, Elems)>();
        }
    };

    // Do not apply this optimization to AIE2p, until shuffle optimizations are ported there.
    static constexpr bool hoist_16b_interleaves = __AIE_ARCH__ == 22 && std::is_same_v<data_type, cint32>;

    // Splits a vector into high and low 16b halves
    // This is done when the multiplication emulation splits the input data in 16bit halves
    // The aim is to perform the interleave once for the input vector instead of many times after each of the shift
    // operations
    // The compiler is expected to match the emulation interleave with one of these and delete them altogether.
    // This optimization is currently not applied correctly in AIE2p due to missing rewrite rules (CRVO-11725).
    template <unsigned N>
    __aie_inline
    static vector<data_type, N> decompose_to_16b(vector<data_type, N> a)
    {
        if constexpr (!hoist_16b_interleaves)
            return a;
        else {
            constexpr unsigned n = N * (a.is_complex()? 4 : 2);
            vector<uint16, n / 2> lo, hi;
            std::tie(lo, hi) = a.template cast_to<uint16>().template split<n / 2>();
            std::tie(lo, hi) = interleave_unzip<uint16, n / 2>::run(lo, hi, 1);
            return concat(lo, hi).template cast_to<data_type>();
        }
    }

    template <unsigned N>
    __aie_inline
    static vector<data_type, N> compose_to_32b(vector<data_type, N> a)
    {
        if constexpr (!hoist_16b_interleaves)
            return a;
        else {
            constexpr unsigned n = N * (a.is_complex()? 4 : 2);
            vector<uint16, n / 2> lo, hi;
            std::tie(lo, hi) = a.template cast_to<uint16>().template split<n / 2>();
            std::tie(lo, hi) = interleave_zip<uint16, n / 2>::run(lo, hi, 1);
            return concat(lo, hi).template cast_to<data_type>();
        }
    }

    template <unsigned N>
    __aie_inline
    static vector<data_type, N> shift_data(vector<data_type, N> a, unsigned elems)
    {
        if constexpr (!hoist_16b_interleaves)
            return shuffle_down_rotate<data_type, N>::run(a, elems);
        else {
            // If upper and lower 16b halves have been separated, we need to
            // rotate them separately too.
            constexpr unsigned n = N * (a.is_complex()? 4 : 2);
            using shift_op = shuffle_down_rotate<uint16, n / 2>;

            vector<uint16, n / 2> lo, hi;
            std::tie(lo, hi) = a.template cast_to<uint16>().template split<n / 2>();
            lo = shift_op::run(lo, elems * (a.is_complex()? 2 : 1));
            hi = shift_op::run(hi, elems * (a.is_complex()? 2 : 1));
            return concat(lo, hi).template cast_to<data_type>();;
        }
    }

    template <MulMacroOp MulOp, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires(is_accum_v<Acc> && ...)
    __aie_inline
    static accum_type run(const vector<coeff_type, N_Coeff> &coeff,
                          unsigned                           coeff_start,
                          bool                               coeff_sign,
                          const vector<data_type, N_Data>   &data,
                          unsigned                           data_start,
                          bool                               data_sign,
                          bool                               zero_acc,
                          const Acc &...                     acc)
    {
        constexpr auto mac_op = sliding_mul_acc48_get_mul_op<add_to_op<MulOp>(), 2 * lanes_per_mul, 1, coeff_type, real_data_type>();
        constexpr auto mul_op = sliding_mul_acc48_get_mul_op<MulOp,              2 * lanes_per_mul, 1, coeff_type, real_data_type>();

        accum_type ret(acc...);

        constexpr unsigned N_local = std::max(lanes_per_mul, N_Data);
        vector<data_type, N_local> data_local = decompose_to_16b(prepare_data<N_Data, N_local>(data, data_start % N_Data));

        utils::unroll_for<unsigned, 0, std::max(1u, Lanes / lanes_per_mul)>([&](auto idx_y) __aie_inline {
            real_accum_type32 tmp;

            const unsigned coeff_start_local = coeff_start % N_Coeff;

            if (idx_y > 0)
                data_local   = shift_data(data_local, lanes_per_mul);
            auto coeff_local = coeff.get(coeff_start_local);

            tmp = mul_op(compose_to_32b(data_local).template extract<lanes_per_mul>(0).template cast_to<real_data_type>(),
                         data_sign,
                         coeff_local,
                         coeff_sign,
                         utils::get_nth<0>(ret.template grow_extract<lanes_per_mul>(idx_y).template cast_to<real_accum_tag>(), acc)...,
                         utils::get_nth<0>(zero_acc, acc)...);

            utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
                const unsigned coeff_start_local = (coeff_start + columns_per_mul * idx) % N_Coeff;

                data_local       = shift_data(data_local, columns_per_mul);
                auto coeff_local = coeff.get(coeff_start_local);

                tmp = mac_op(compose_to_32b(data_local).template extract<lanes_per_mul>(0).template cast_to<real_data_type>(),
                             data_sign,
                             coeff_local,
                             coeff_sign,
                             tmp,
                             false);
            });

            if constexpr (Lanes <= lanes_per_mul)
                ret = tmp.template cast_to<accum_tag>().template extract<Lanes>(0);
            else
                ret.insert(idx_y, tmp.template cast_to<accum_tag>());
        });

        return ret;
    }
};

// c16b coeff * 32b data
// c32b coeff * 16b data
// c32b coeff * 32b data
template <unsigned Lanes, unsigned Points, unsigned CoeffBits, unsigned DataBits, typename CoeffType, typename DataType>
    requires(is_complex_v<CoeffType> && !is_complex_v<DataType>)
struct sliding_mul_bits_impl<Lanes, Points, 1, 1, 1, 64, CoeffBits, DataBits, CoeffType, DataType>
{
    using coeff_type        = CoeffType;
    using data_type         = DataType;
    using real_coeff_type   = std::conditional_t<std::is_same_v<coeff_type, cint16>, int16, int32>;
    using accum_tag         = accum_tag_for_mul_types<data_type, coeff_type, 64>;
    using accum_type        = accum<accum_tag, Lanes>;
    using real_accum_tag    = accum_tag_for_mul_types<data_type, real_coeff_type, 64>;
    using real_accum_type32 = accum<real_accum_tag, 32>;

    static constexpr unsigned   lanes_per_mul = 16;
    static constexpr unsigned columns_per_mul = 1;
    static constexpr unsigned         num_mul = Points / columns_per_mul;
    static_assert(Points >= columns_per_mul);
    static_assert(Points % columns_per_mul == 0);

    template <typename T, unsigned Elems, unsigned OutElems = Elems>
    static auto prepare_data(vector<T, Elems> input, unsigned offset)
    {
        constexpr unsigned native_elems = native_vector_length_v<T>;
        // Avoid 1024b shuffle_down_rotate where possible
        if constexpr (Elems <= native_vector_length_v<T>) {
            auto tmp = shuffle_down_rotate<T, native_elems>::run(input.template grow_replicate<native_elems>(), offset);
            return concat(interleave_zip<T, native_elems>::run(tmp, tmp, 1)).template extract<OutElems>(0);
        }
        else {
            auto tmp = shuffle_down_rotate<T, OutElems>::run(input, offset).template extract<native_elems>(0);
            return concat(interleave_zip<T, native_elems>::run(tmp, tmp, 1)).template extract<OutElems>(0);
        }
    };

    template <MulMacroOp MulOp, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires(is_accum_v<Acc> && ...)
    __aie_inline
    static accum_type run(const vector<coeff_type, N_Coeff> &coeff,
                          unsigned                           coeff_start,
                          bool                               coeff_sign,
                          const vector<data_type, N_Data>   &data,
                          unsigned                           data_start,
                          bool                               data_sign,
                          bool                               zero_acc,
                          const Acc &...                     acc)
    {
        constexpr auto mac_op = sliding_mul_acc48_get_mul_op<add_to_op<MulOp>(), 2 * lanes_per_mul, 1, real_coeff_type, data_type>();
        constexpr auto mul_op = sliding_mul_acc48_get_mul_op<MulOp,              2 * lanes_per_mul, 1, real_coeff_type, data_type>();

        accum<real_accum_tag, Lanes * 2> ret(acc.template cast_to<real_accum_tag>()...);

        utils::unroll_for<unsigned, 0, std::max(1u, Lanes / lanes_per_mul)>([&](auto idx_y) __aie_inline {
            real_accum_type32 tmp;

            const unsigned  data_start_local = (data_start + idx_y * lanes_per_mul) % N_Data;
            const unsigned coeff_start_local = coeff_start % N_Coeff;

            auto data_local  = prepare_data<data_type, N_Data, 2 * lanes_per_mul>(data, data_start_local);
            auto coeff_val   = coeff.get(coeff_start_local);
            auto coeff_local = broadcast<coeff_type, lanes_per_mul>::run(coeff_val);

            tmp = mul_op(data_local,                                       data_sign,
                         coeff_local.template cast_to<real_coeff_type>(), coeff_sign,
                         utils::get_nth<0>(ret.template cast_to<real_accum_tag>().template grow_extract<2 * lanes_per_mul>(idx_y), acc)...,
                         utils::get_nth<0>(zero_acc, acc)...);

            utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
                const unsigned data_start_local  =  (data_start + columns_per_mul * idx + idx_y * lanes_per_mul) % N_Data;
                const unsigned coeff_start_local = (coeff_start + columns_per_mul * idx) % N_Coeff;

                auto data_local  = prepare_data<data_type, N_Data, 2 * lanes_per_mul>(data, data_start_local);
                auto coeff_val   = coeff.get(coeff_start_local);
                auto coeff_local = broadcast<coeff_type, lanes_per_mul>::run(coeff_val);

                tmp = mac_op(data_local,                                       data_sign,
                             coeff_local.template cast_to<real_coeff_type>(), coeff_sign,
                             tmp, false);
            });

            if constexpr (Lanes <= lanes_per_mul)
                ret = tmp.template extract<Lanes * 2>(0);
            else
                ret.insert(idx_y, tmp);
        });

        return ret.template cast_to<accum_tag>();
    }
};

// c16b coeff * 16b data - leverage convolution mode
template <unsigned Lanes, unsigned Points>
struct sliding_mul_bits_impl<Lanes, Points, 1, 1, 1, 64, 32, 16, cint16, int16>
{
    using coeff_type      = cint16;
    using real_coeff_type = int16;
    using data_type       = int16;
    using accum_tag       = accum_tag_for_mul_types<data_type, coeff_type, 64>;
    using real_accum_tag  = accum_tag_for_mul_types<data_type, real_coeff_type, 64>;
    using accum_type      = accum<accum_tag, Lanes>;
    using accum_type32    = accum<accum_tag, 32>;
    using real_accum_type = accum<real_accum_tag, 32>;

    static constexpr unsigned   lanes_per_mul = 32;
    static constexpr unsigned columns_per_mul = 4;
    static constexpr unsigned         num_mul = Points / columns_per_mul;
    static_assert(Points >= columns_per_mul);
    static_assert(Points % columns_per_mul  == 0);

    static constexpr unsigned native_data_elems  = 64;
    static constexpr unsigned native_coeff_elems = native_vector_length_v<real_coeff_type>;

    template <typename T, unsigned Elems, unsigned OutElems = Elems>
    static auto prepare_data(vector<T, Elems> input, unsigned offset)
    {
        // Avoid 1024b shuffle_down_rotate where possible
        if constexpr (Elems <= lanes_per_mul) {
            auto tmp = shuffle_down_rotate<T, lanes_per_mul>::run(input.template grow_replicate<lanes_per_mul>(), offset);
            if constexpr (Lanes <= 16)
                return tmp.template grow<OutElems>();
            else
                return tmp.template grow_replicate<OutElems>();
        }
        else {
            return shuffle_down_rotate<T, OutElems>::run(input.template grow_replicate<OutElems>(), offset);
        }
    };

    template <MulMacroOp MulOp, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires(is_accum_v<Acc> && ...)
    __aie_inline
    static accum_type run(const vector<coeff_type, N_Coeff> &coeff,
                          unsigned                           coeff_start,
                          bool                               coeff_sign,
                          const vector<data_type, N_Data>   &data,
                          unsigned                           data_start,
                          bool                               data_sign,
                          bool                               zero_acc,
                          const Acc &...                     acc)
    {
        constexpr unsigned data_elems = std::max(N_Data, native_data_elems);

        data_start = data_start % N_Data;

        constexpr auto mul_op      = sliding_mul_acc48_get_mul_op<MulOp,                   lanes_per_mul, 1, real_coeff_type, data_type>();
        constexpr auto mac_op      = sliding_mul_acc48_get_mul_op<add_to_op<MulOp>(),      lanes_per_mul, 1, real_coeff_type, data_type>();
        constexpr auto mul_conf_op = sliding_mul_acc48_get_mul_conf_op<MulOp,              lanes_per_mul, 1, real_coeff_type, data_type>();
        constexpr auto mac_conf_op = sliding_mul_acc48_get_mul_conf_op<add_to_op<MulOp>(), lanes_per_mul, 1, real_coeff_type, data_type>();

        constexpr unsigned num_conj = has_conj2<MulOp>()? 1 : 0;

        // Unzipping coeff
        vector<int16, native_coeff_elems> coeff_re, coeff_im;
        std::tie(coeff_re, coeff_im) = grow_all<native_coeff_elems>(unzip_complex(coeff));

        // Unzipping input accumulator
        real_accum_type acc_re, acc_im;
        if constexpr (sizeof...(Acc) > 0)
            std::tie(acc_re, acc_im) = grow_all<real_accum_type::size()>(unzip_complex(acc)...);

        const unsigned coeff_start_local = coeff_start % N_Coeff;

        auto data_shuffle = prepare_data<int16, N_Data, native_data_elems>(data, data_start);

        acc_re = mul_op(data_shuffle,
                        true,
                        shuffle_down_rotate<int16, native_coeff_elems>::run(coeff_re, coeff_start_local),
                        coeff_sign,
                        utils::get_nth<0>(acc_re, acc)...,
                        utils::get_nth<0>(zero_acc, acc)...);

        acc_im = mul_conf_op(data_shuffle,
                             true,
                             shuffle_down_rotate<int16, native_coeff_elems>::run(coeff_im, coeff_start_local),
                             coeff_sign,
                             utils::get_nth<0>(acc_im, acc)...,
                             utils::get_nth<0>(zero_acc, acc)...,
                             utils::get_nth<0>(0, acc)...,       // shift16
                             num_conj,                           // sub_mul - negation of mul result for  -(coef.im*data.im)
                             utils::get_nth<0>(0, acc)...);      // sub_acc

        utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
            const unsigned data_start_local  = ( data_start + columns_per_mul * idx) % data_elems;
            const unsigned coeff_start_local = (coeff_start + columns_per_mul * idx) % N_Coeff;

            auto data_shuffle = prepare_data<int16, N_Data, native_data_elems>(data, data_start_local);

            acc_re = mac_op(data_shuffle,
                            true,
                            shuffle_down_rotate<int16, native_coeff_elems>::run(coeff_re, coeff_start_local),
                            coeff_sign,
                            acc_re, false);

            acc_im = mac_conf_op(data_shuffle,
                                 true,
                                 shuffle_down_rotate<int16, native_coeff_elems>::run(coeff_im, coeff_start_local),
                                 coeff_sign,
                                 acc_im,
                                 false,
                                 0,
                                 num_conj, //negation of mul result for  -(coef.im*data.im)
                                 0);
        });

        // Re-shuffling real and imaginary
        return combine_into_complex(acc_re.template extract<Lanes>(0),
                                    acc_im.template extract<Lanes>(0));
    }
};

// c16b coeff * c16b data
// c16b coeff * c32b data
// c32b coeff * c16b data
// c32b coeff * c32b data
template <unsigned Lanes, unsigned Points, unsigned CoeffBits, unsigned DataBits, typename CoeffType, typename DataType>
    requires(is_complex_v<CoeffType> && is_complex_v<DataType>)
struct sliding_mul_bits_impl<Lanes, Points, 1, 1, 1, 64, CoeffBits, DataBits, CoeffType, DataType>
{
    using coeff_type   = CoeffType;
    using data_type    = DataType;
    using accum_tag    = accum_tag_for_mul_types<data_type, coeff_type, 64>;
    using accum_type   = accum<accum_tag, Lanes>;
    using accum_type16 = accum<accum_tag, 16>;

    static constexpr unsigned   lanes_per_mul = 16;
    static constexpr unsigned columns_per_mul = 1;
    static constexpr unsigned         num_mul = Points / columns_per_mul;
    static_assert(Points >= columns_per_mul);
    static_assert(Points % columns_per_mul == 0);

    static constexpr unsigned native_data_elems = native_vector_length_v<data_type>;

    // Provide two ways to access coefficients vector elements.
    // stack:  accesses each element through the stack, effectively replacing
    //         VEXTRACT with LDx and thus reducing the load of the shuffle unit
    //         at the cost of higher initialization
    //         (vector load -> vector store -> scalar load)
    template <unsigned N_Coeff>
    struct coeff_lookup_stack {
        __aie_inline
        explicit coeff_lookup_stack(const vector<coeff_type, N_Coeff> &c) :
            coeffs()
        {
            c.store(coeffs);
        }

        __aie_inline
        coeff_type operator[](unsigned i) const { return coeffs[i]; }

        alignas(vector_decl_align) coeff_type coeffs[N_Coeff];
    };
    // vector: accesses each element by extracting the element form the vector directly
    template <unsigned N_Coeff>
    struct coeff_lookup_vector {
        __aie_inline
        explicit coeff_lookup_vector(const vector<coeff_type, N_Coeff> &c) :
            coeffs(c)
        {
        }

        __aie_inline
        coeff_type operator[](unsigned i) const { return coeffs.get(i); }

        const vector<coeff_type, N_Coeff> &coeffs;
    };

    template <typename T, unsigned Elems>
    static auto prepare_data(vector<T, Elems> input, unsigned offset) {
        constexpr unsigned native_elems = native_vector_length_v<T>;

        // Avoid 1024b shuffle_down_rotate where possible
        if constexpr (Elems <= native_elems) {
            //FIXME: hoist grow_replicate
            return shuffle_down_rotate<T, native_elems>::run(input.template grow_replicate<native_elems>(), offset)
                        .template grow_replicate<lanes_per_mul>();
        }
        else {
            return shuffle_down_rotate<T, Elems>::run(input, offset).template extract<lanes_per_mul>(0);
        }
    };

    template <MulMacroOp MulOp, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires(is_accum_v<Acc> && ...)
    __aie_inline
    static accum_type run(const vector<coeff_type, N_Coeff> &coeff,
                          unsigned                           coeff_start,
                          bool                               coeff_sign,
                          const vector<data_type, N_Data>   &data,
                          unsigned                           data_start,
                          bool                               data_sign,
                          bool                               zero_acc,
                          const Acc &...                     acc)
    {
        constexpr bool flip_args = (DataBits < 64);
        using Type1 = std::conditional_t<flip_args, data_type, coeff_type>;
        using Type2 = std::conditional_t<flip_args, coeff_type, data_type>;
        constexpr auto mac_op = [&](auto &data, auto &coeff, auto &&...args) __aie_inline {
            constexpr auto op = sliding_mul_acc48_get_mul_op<add_to_op<MulOp>(), lanes_per_mul, 1, Type1, Type2>();
            if constexpr (flip_args)
                return op(coeff, true, data, true, args...);
            else
                return op(data, true, coeff, true, args...);
        };
        constexpr auto mul_op = [&](auto &data, auto &coeff, auto &&...args) __aie_inline {
            constexpr auto op = sliding_mul_acc48_get_mul_op<MulOp, lanes_per_mul, 1, Type1, Type2>();
            if constexpr (flip_args)
                return op(coeff, true, data, true, args...);
            else
                return op(data, true, coeff, true, args...);
        };

        accum_type ret(acc...);

        // Loading coefficients via scalar load allows better balance of shuffle and vmac units
        // 2 * vshift / vmac vs (2 * vshift + 1 * vextract) / vmac
        // However, it has higher initialisation cost: storing the vector and loading the first scalar element.
        // We should only do this when the cost is amortized with the number of multiplications.
        constexpr bool load_via_stack = num_mul > 3;
        using coeff_lookup_t = std::conditional_t<load_via_stack, coeff_lookup_stack<N_Coeff>,
                                                                  coeff_lookup_vector<N_Coeff>>;
        coeff_lookup_t coeff_tmp{coeff};

        utils::unroll_for<unsigned, 0, std::max(1u, Lanes / lanes_per_mul)>([&](auto idx_y) __aie_inline {
            // When multiply-accumulate operations are run back to back, there are two cycles between
            // reading the input accumulator and writing to the output accumulator.
            // Using a single accumulator limits these operations to run every other cycle. Computing
            // a partial sum with two accumulators and adding them up at the end allows us to perform a multiplication
            // every cycle in long chains.
            constexpr unsigned num_accums = num_mul > 2? 2 : 1;
            std::array<accum_type16, num_accums> tmp;

            const unsigned data_start_local = (data_start + idx_y * lanes_per_mul) % N_Data;
            const unsigned coeff_start_local = coeff_start % N_Coeff;

            auto  data_local = prepare_data(data, data_start_local);
            auto coeff_local = coeff_tmp[coeff_start_local];

            tmp[0] = mul_op(data_local, coeff_local,
                            utils::get_nth<0>(ret.template grow_extract<lanes_per_mul>(idx_y), acc)...,
                            utils::get_nth<0>(zero_acc, acc)...);

            utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
                const unsigned data_start_local  =  (data_start + columns_per_mul * idx + idx_y * lanes_per_mul) % N_Data;
                const unsigned coeff_start_local = (coeff_start + columns_per_mul * idx) % N_Coeff;

                accum_type16 &acc = tmp[idx % tmp.size()];
                const bool zero_acc = idx < tmp.size();

                auto  data_local = prepare_data(data, data_start_local);
                auto coeff_local = coeff_tmp[coeff_start_local];

                acc = mac_op(data_local, coeff_local, acc, zero_acc);
            });

            utils::unroll_for<unsigned, 1, tmp.size()>([&](auto idx) __aie_inline {
                tmp[0] = add_accum<accum_tag, 16>::run(tmp[idx], false, tmp[0]);
            });

            if constexpr (Lanes <= lanes_per_mul)
                ret = tmp[0].template extract<Lanes>(0);
            else
                ret.insert(idx_y, tmp[0]);
        });

        return ret;
    }
};
#endif // __AIE_API_COMPLEX_VECTOR_SUPPORT__

}

#endif
