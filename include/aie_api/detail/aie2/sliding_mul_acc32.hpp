// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_SLIDING_MUL_ACC32_HPP__
#define __AIE_API_DETAIL_AIE2_SLIDING_MUL_ACC32_HPP__

#include "../mul.hpp"
#include "../vector.hpp"
#include "../shuffle.hpp"
#include "../filter.hpp"
#include "../utils.hpp"
#include "../vector_accum_cast.hpp"

namespace aie::detail {

template <MulMacroOp MulOp, unsigned Lanes, typename CoeffType, typename DataType>
static constexpr auto sliding_mul_acc32_get_mul_op()
{
    if      constexpr (type_bits_v<CoeffType> == 16) {
        if constexpr (MulOp == MulMacroOp::Mul)     return [](auto &&... args) __aie_inline { return ::mul_conv_16x4(args...); };
        if constexpr (MulOp == MulMacroOp::NegMul)  return [](auto &&... args) __aie_inline { return ::negmul_conv_16x4(args...); };
        if constexpr (MulOp == MulMacroOp::Add_Mul) return [](auto &&... args) __aie_inline { return ::mac_conv_16x4(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_Mul) return [](auto &&... args) __aie_inline { return ::msc_conv_16x4(args...); };
    }
    else if constexpr (type_bits_v<CoeffType> == 8) {
        if constexpr (MulOp == MulMacroOp::Mul)     return [](auto &&... args) __aie_inline { return ::mul_conv_32x8(args...); };
        if constexpr (MulOp == MulMacroOp::NegMul)  return [](auto &&... args) __aie_inline { return ::negmul_conv_32x8(args...); };
        if constexpr (MulOp == MulMacroOp::Add_Mul) return [](auto &&... args) __aie_inline { return ::mac_conv_32x8(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_Mul) return [](auto &&... args) __aie_inline { return ::msc_conv_32x8(args...); };
    }
}

template <MulMacroOp MulOp, unsigned Lanes, typename CoeffType, typename DataType>
static constexpr auto sliding_mul_acc32_get_mul_conf_op()
{
    if      constexpr (type_bits_v<CoeffType> == 16) {
        if constexpr (MulOp == MulMacroOp::Mul)     return [](auto &&... args) __aie_inline { return ::mul_conv_16x4_conf(args...); };
        if constexpr (MulOp == MulMacroOp::NegMul)  return [](auto &&... args) __aie_inline { return ::negmul_conv_16x4_conf(args...); };
        if constexpr (MulOp == MulMacroOp::Add_Mul) return [](auto &&... args) __aie_inline { return ::mac_conv_16x4_conf(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_Mul) return [](auto &&... args) __aie_inline { return ::msc_conv_16x4_conf(args...); };
    }
    else if constexpr (type_bits_v<CoeffType> == 8) {
        if constexpr (MulOp == MulMacroOp::Mul)     return [](auto &&... args) __aie_inline { return ::mul_conv_32x8_conf(args...); };
        if constexpr (MulOp == MulMacroOp::NegMul)  return [](auto &&... args) __aie_inline { return ::negmul_conv_32x8_conf(args...); };
        if constexpr (MulOp == MulMacroOp::Add_Mul) return [](auto &&... args) __aie_inline { return ::mac_conv_32x8_conf(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_Mul) return [](auto &&... args) __aie_inline { return ::msc_conv_32x8_conf(args...); };
    }
}

template <unsigned Lanes, unsigned Points, typename CoeffType, typename DataType>
struct sliding_mul_bits_impl<Lanes, Points, 1, 1, 1, 32, 8, 8, CoeffType, DataType>
{
    using coeff_type = CoeffType;
    using  data_type = DataType;
    using accum_tag     = accum_tag_for_mul_types<data_type, coeff_type, 32>;
    using accum_type    = accum<accum_tag, Lanes>;
    using accum_type32  = accum<accum_tag, 32>;

    static constexpr unsigned columns_per_mul = 8;
    static constexpr unsigned         num_mul = utils::ceildiv(Points, columns_per_mul);

    template <MulMacroOp MulOp, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type run(const vector<coeff_type, N_Coeff> &coeff,
                          unsigned coeff_start,
                          bool coeff_sign,
                          const vector<data_type, N_Data> &data,
                          unsigned data_start,
                          bool data_sign,
                          const Acc &... acc)
    {
        constexpr unsigned data_elems = std::max(N_Data, 64u);

        data_start = data_start % data_elems;

        vector<coeff_type, 64> coeff2 = coeff.template grow_replicate<64>();

        if constexpr (Points % columns_per_mul) {
            coeff2 = ::sel(shuffle_down_rotate<coeff_type, 64>::run(coeff2, coeff_start), zeros<coeff_type, 64>::run(), (~0ull) << Points);
            coeff_start = 0;
        }

        constexpr auto mac_op = sliding_mul_acc32_get_mul_op<add_to_op<MulOp>(), Lanes, coeff_type, data_type>();
        constexpr auto mul_op = sliding_mul_acc32_get_mul_op<             MulOp, Lanes, coeff_type, data_type>();

        accum_type ret(acc...);

        utils::unroll_for<unsigned, 0, std::max(1u, Lanes / 32)>([&](auto idx_y) __aie_inline {
            const unsigned  data_start_local = (data_start + idx_y * 32) % data_elems;
            const unsigned coeff_start_local = coeff_start % N_Coeff;

            accum_type32 tmp;

            tmp = mul_op(shuffle_down_rotate<data_type,  data_elems>::run( data.template grow_replicate<data_elems>(),  data_start_local).template extract<64>(0),
                         data_sign,
                         shuffle_down_rotate<coeff_type,         64>::run(coeff2,                                      coeff_start_local),
                         coeff_sign,
                         utils::get_nth<0>(ret.template grow_extract<32>(idx_y), acc)...);

            utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
                const unsigned data_start_local  =  (data_start + columns_per_mul * idx + idx_y * 32) % data_elems;
                const unsigned coeff_start_local = (coeff_start + columns_per_mul * idx) % N_Coeff;

                tmp = mac_op(shuffle_down_rotate<data_type,  data_elems>::run( data.template grow_replicate<data_elems>(),  data_start_local).template extract<64>(0),
                             data_sign,
                             shuffle_down_rotate<coeff_type,         64>::run(coeff2,                                      coeff_start_local),
                             coeff_sign,
                             tmp);
            });

            if constexpr (Lanes <= 32)
                ret = tmp.template extract<Lanes>(0);
            else
                ret.insert(idx_y, tmp);
        });

        return ret;
    }
};

template <unsigned Lanes, unsigned Points, typename CoeffType, typename DataType>
struct sliding_mul_bits_impl<Lanes, Points, 1, 1, 1, 32, 16, 16, CoeffType, DataType>
{
    using coeff_type = CoeffType;
    using  data_type = DataType;
    using  accum_tag = accum_tag_for_mul_types<data_type, coeff_type, 32>;
    using accum_type   = accum<accum_tag, Lanes>;
    template <unsigned Lanes2>
    using accum_internal_type = accum<accum_tag_for_mul_types<coeff_type, data_type, 64>, Lanes2>;

    static constexpr unsigned columns_per_mul = 4;
    static constexpr unsigned         num_mul = utils::ceildiv(Points, columns_per_mul);

    template <MulMacroOp MulOp, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type run(const vector<coeff_type, N_Coeff> &coeff,
                          unsigned coeff_start,
                          bool coeff_sign,
                          const vector<data_type, N_Data> &data,
                          unsigned data_start,
                          bool data_sign,
                          const Acc &... acc)
    {
        constexpr unsigned data_elems = std::max(N_Data, 32u);

        data_start = data_start % data_elems;

        vector<coeff_type, 32> coeff2 = coeff.template grow_replicate<32>();

        if constexpr (Points % columns_per_mul) {
            coeff2 = ::sel(shuffle_down_rotate<coeff_type, 32>::run(coeff2, coeff_start), zeros<coeff_type, 32>::run(), (~0u) << Points);
            coeff_start = 0;
        }

        constexpr auto mac_op = sliding_mul_acc32_get_mul_op<add_to_op<MulOp>(), 16, coeff_type, data_type>();
        constexpr auto mul_op = sliding_mul_acc32_get_mul_op<             MulOp, 16, coeff_type, data_type>();

        accum_internal_type<Lanes>  acc_internal;
        accum_type                  res;

        //No intrinsic for this with 32b accumulators, so internally we work with 64b accumulators by shuffling the upper bits in/out
        if constexpr (sizeof...(Acc) == 1) {
            const vector<int32, 16> z = zeros<int32, 16>::run();
            utils::unroll_for<unsigned, 0, std::max(1u, Lanes / 16)>([&](auto idx_y) __aie_inline {
                vector<int32, 16> tmp = accum_to_vector_cast<int32, acc32, 16>::run((acc.template grow_extract<16>(idx_y))...);

                const auto [in_vec_lo, in_vec_hi] = interleave_zip<int32, 16>::run(tmp, z, 1);

                if constexpr (Lanes <= 16)
                    acc_internal = (vector_to_accum_cast<acc64, int32, 32>::run(concat_vector(in_vec_lo, in_vec_hi))).template extract<Lanes>(0);
                else
                    acc_internal.insert(idx_y, vector_to_accum_cast<acc64, int32, 32>::run(concat_vector(in_vec_lo, in_vec_hi)));
            });
        }

        utils::unroll_for<unsigned, 0, std::max(1u, Lanes / 16)>([&](auto idx_y) __aie_inline {
            const unsigned  data_start_local = (data_start + idx_y * 16) % data_elems;
            const unsigned coeff_start_local = coeff_start % N_Coeff;
            accum_internal_type<16> tmp;

            tmp = mul_op(shuffle_down_rotate<data_type,  data_elems>::run( data.template grow_replicate<data_elems>(),  data_start_local).template extract<32>(0),
                         data_sign,
                         shuffle_down_rotate<coeff_type,         32>::run(coeff2,                                      coeff_start_local),
                         coeff_sign,
                         utils::get_nth<0>(acc_internal.template grow_extract<16>(idx_y), acc)...);

            utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
                const unsigned data_start_local  =  (data_start + columns_per_mul * idx + idx_y * 16) % data_elems;
                const unsigned coeff_start_local = (coeff_start + columns_per_mul * idx) % N_Coeff;

                tmp = mac_op(shuffle_down_rotate<data_type,  data_elems>::run( data.template grow_replicate<data_elems>(),  data_start_local).template extract<32>(0),
                             data_sign,
                             shuffle_down_rotate<coeff_type,         32>::run(coeff2,                                      coeff_start_local),
                             coeff_sign,
                             tmp);
            });

            const vector<int32, 32> vec1 = accum_to_vector_cast<int32, acc64, 16>::run(tmp);
            const vector<int32, 16> vec2 = filter<int32, 32, FilterOp::Even>::run(vec1, 1);

            if constexpr (Lanes <= 16)
                res = (vector_to_accum_cast<acc32, int32, 16>::run(vec2)).template extract<Lanes>(0);
            else
                res.insert(idx_y, vector_to_accum_cast<acc32, int32, 16>::run(vec2));
        });

        return res;
    }
};

}

#endif
