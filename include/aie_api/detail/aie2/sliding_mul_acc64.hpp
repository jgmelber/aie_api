// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_SLIDING_MUL_ACC64_HPP__
#define __AIE_API_DETAIL_AIE2_SLIDING_MUL_ACC64_HPP__

#include <algorithm>

#include "../mul.hpp"
#include "../shuffle.hpp"
#include "../vector.hpp"
#include "../vector_accum_cast.hpp"

#include "emulated_mul_intrinsics.hpp"

namespace aie::detail {

template <MulMacroOp MulOp, unsigned Lanes, int DataStep, typename CoeffType, typename DataType>
static constexpr auto sliding_mul_acc48_get_mul_op()
{
    constexpr auto sub_mask = [&](){
        if      constexpr (has_conj1<MulOp>() && has_conj2<MulOp>()) return OP_TERM_NEG_COMPLEX_CONJUGATE_X_Y;
        else if constexpr (has_conj1<MulOp>())                       return OP_TERM_NEG_COMPLEX_CONJUGATE_X;
        else if constexpr (has_conj2<MulOp>())                       return OP_TERM_NEG_COMPLEX_CONJUGATE_Y;
        else                                                         return OP_TERM_NEG_COMPLEX;
    }();

    if constexpr (std::is_same_v<CoeffType, cint32> || std::is_same_v<DataType, cint32>) {
        if constexpr (MulOp == MulMacroOp::Mul || MulOp == MulMacroOp::MulConj1 || MulOp == MulMacroOp::MulConj1Conj2 || MulOp == MulMacroOp::MulConj2) {
            if      constexpr (MulOp == MulMacroOp::Mul)           return [](auto &&... args) __aie_inline { return ::mul_elem_8(args...); };
            else if constexpr (MulOp == MulMacroOp::MulConj1)      return [](auto &&... args) __aie_inline { return ::mul_elem_8_cn(args...); };
            else if constexpr (MulOp == MulMacroOp::MulConj1Conj2) return [](auto &&... args) __aie_inline { return ::mul_elem_8_cc(args...); };
            else if constexpr (MulOp == MulMacroOp::MulConj2)      return [](auto &&... args) __aie_inline { return ::mul_elem_8_nc(args...); };
        }
        if constexpr (MulOp == MulMacroOp::NegMul || MulOp == MulMacroOp::NegMulConj1 || MulOp == MulMacroOp::NegMulConj1Conj2 || MulOp == MulMacroOp::NegMulConj2) {
            if      constexpr (MulOp == MulMacroOp::NegMul)           return [](auto &&... args) __aie_inline { return ::negmul_elem_8(args...); };
            else if constexpr (MulOp == MulMacroOp::NegMulConj1)      return [](auto &&... args) __aie_inline { return ::negmul_elem_8_cn(args...); };
            else if constexpr (MulOp == MulMacroOp::NegMulConj1Conj2) return [](auto &&... args) __aie_inline { return ::negmul_elem_8_cc(args...); };
            else if constexpr (MulOp == MulMacroOp::NegMulConj2)      return [](auto &&... args) __aie_inline { return ::negmul_elem_8_nc(args...); };
        }
        if constexpr (std::is_same_v<CoeffType, cint32> && std::is_same_v<DataType, cint32>) {
            if constexpr (MulOp == MulMacroOp::Add_Mul || MulOp == MulMacroOp::Add_MulConj1 || MulOp == MulMacroOp::Add_MulConj1Conj2 || MulOp == MulMacroOp::Add_MulConj2)
                return [sub_mask](auto &&... args) __aie_inline { return ::mac_elem_8_conf(args..., sub_mask, /*sub_mul=*/0, /*sub_acc1=*/0); };
            if constexpr (MulOp == MulMacroOp::Sub_Mul || MulOp == MulMacroOp::Sub_MulConj1 || MulOp == MulMacroOp::Sub_MulConj1Conj2 || MulOp == MulMacroOp::Sub_MulConj2)
                // Using mac rather than msc due to CRVO-11362
                return [sub_mask](auto &&... args) __aie_inline { return ::mac_elem_8_conf(args..., sub_mask, /*sub_mul=*/1, /*sub_acc1=*/0); };
        }
        else {
            if constexpr (MulOp == MulMacroOp::Add_Mul || MulOp == MulMacroOp::Add_MulConj1 || MulOp == MulMacroOp::Add_MulConj1Conj2 || MulOp == MulMacroOp::Add_MulConj2)
                return [sub_mask](auto &&... args) __aie_inline { return ::mac_elem_8_conf(args..., /*shift16=*/0, sub_mask, /*sub_mul=*/0, /*sub_acc1=*/0); };
            if constexpr (MulOp == MulMacroOp::Sub_Mul || MulOp == MulMacroOp::Sub_MulConj1 || MulOp == MulMacroOp::Sub_MulConj1Conj2 || MulOp == MulMacroOp::Sub_MulConj2)
                // Using mac rather than msc due to CRVO-11362
                return [sub_mask](auto &&... args) __aie_inline { return ::mac_elem_8_conf(args..., /*shift16=*/0, sub_mask, /*sub_mul=*/1, /*sub_acc1=*/0); };
        }
    }
    else {
        if constexpr (DataStep == 16) {
            if constexpr (MulOp == MulMacroOp::Mul)               return [](auto &&... args) __aie_inline { return ::mul_elem_16_2_conf(args..., /*sub_mul=*/0); };
            if constexpr (MulOp == MulMacroOp::NegMul)            return [](auto &&... args) __aie_inline { return ::negmul_elem_16_2_conf(args..., /*sub_mul=*/0); };
            if constexpr (MulOp == MulMacroOp::Add_Mul)           return [](auto &&... args) __aie_inline { return ::mac_elem_16_2_conf(args..., /*shift16=*/0, /*sub_mul=*/0, /*sub_acc1=*/0); };
            if constexpr (MulOp == MulMacroOp::Sub_Mul)           return [](auto &&... args) __aie_inline { return ::msc_elem_16_2_conf(args..., /*shift16=*/0, /*sub_mul=*/0, /*sub_acc1=*/0); };
        }
        else if constexpr (std::is_same_v<DataType, int32> || std::is_same_v<CoeffType, int32>) {
            // Emulated mul intrinsics
            if constexpr (MulOp == MulMacroOp::Mul)     return [](auto &&... args) __aie_inline { return mul_conv_16x4_conf(args..., /*sub_mul=*/0); };
            if constexpr (MulOp == MulMacroOp::NegMul)  return [](auto &&... args) __aie_inline { return negmul_conv_16x4_conf(args..., /*sub_mul=*/0); };
            if constexpr (MulOp == MulMacroOp::Add_Mul) return [](auto &&... args) __aie_inline { return mac_conv_16x4_conf(args..., /*sub_mul=*/0, /*sub_acc1=*/0); };
            if constexpr (MulOp == MulMacroOp::Sub_Mul) return [](auto &&... args) __aie_inline { return msc_conv_16x4_conf(args..., /*sub_mul=*/0, /*sub_acc1=*/0); };
        }
        else {
            if constexpr (MulOp == MulMacroOp::Mul || MulOp == MulMacroOp::MulConj1 || MulOp == MulMacroOp::MulConj1Conj2 || MulOp == MulMacroOp::MulConj2)
                return [](auto &&... args) __aie_inline { return ::mul_conv_16x4_conf(args..., /*sub_mul=*/0); };
            if constexpr (MulOp == MulMacroOp::NegMul || MulOp == MulMacroOp::NegMulConj1 || MulOp == MulMacroOp::NegMulConj1Conj2 || MulOp == MulMacroOp::NegMulConj2)
                return [](auto &&... args) __aie_inline { return ::negmul_conv_16x4_conf(args..., /*sub_mul=*/0); };
            if constexpr (MulOp == MulMacroOp::Add_Mul || MulOp == MulMacroOp::Add_MulConj1 || MulOp == MulMacroOp::Add_MulConj1Conj2 || MulOp == MulMacroOp::Add_MulConj2)
                return [](auto &&... args) __aie_inline { return ::mac_conv_16x4_conf(args..., /*shift16=*/0, /*sub_mul=*/0, /*sub_acc1=*/0); };
            if constexpr (MulOp == MulMacroOp::Sub_Mul || MulOp == MulMacroOp::Sub_MulConj1 || MulOp == MulMacroOp::Sub_MulConj1Conj2 || MulOp == MulMacroOp::Sub_MulConj2)
                return [](auto &&... args) __aie_inline { return ::msc_conv_16x4_conf(args..., /*shift16=*/0, /*sub_mul=*/0, /*sub_acc1=*/0); };
        }
    }
}

template <MulMacroOp MulOp, unsigned Lanes, int DataStep, typename CoeffType, typename DataType>
static constexpr auto sliding_mul_acc48_get_mul_conf_op()
{
    if constexpr (std::is_same_v<CoeffType, cint32> || std::is_same_v<DataType, cint32>) {
        if constexpr (MulOp == MulMacroOp::Mul || MulOp == MulMacroOp::MulConj1 || MulOp == MulMacroOp::MulConj1Conj2 || MulOp == MulMacroOp::MulConj2)
            return [](auto &&... args) __aie_inline { return ::mul_elem_8_conf(args...); };
        if constexpr (MulOp == MulMacroOp::NegMul || MulOp == MulMacroOp::NegMulConj1 || MulOp == MulMacroOp::NegMulConj1Conj2 || MulOp == MulMacroOp::NegMulConj2)
            return [](auto &&... args) __aie_inline { return ::negmul_elem_8_conf(args...); };
        if constexpr (MulOp == MulMacroOp::Add_Mul || MulOp == MulMacroOp::Add_MulConj1 || MulOp == MulMacroOp::Add_MulConj1Conj2 || MulOp == MulMacroOp::Add_MulConj2)
            return [](auto &&... args) __aie_inline { return ::mac_elem_8_conf(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_Mul || MulOp == MulMacroOp::Sub_MulConj1 || MulOp == MulMacroOp::Sub_MulConj1Conj2 || MulOp == MulMacroOp::Sub_MulConj2)
            return [](auto &&... args) __aie_inline { return ::msc_elem_8_conf(args...); };
    }
    else {
        if constexpr (DataStep == 16) {
            if constexpr (MulOp == MulMacroOp::Mul)               return [](auto &&... args) __aie_inline { return ::mul_elem_16_2_conf(args...); };
            if constexpr (MulOp == MulMacroOp::NegMul)            return [](auto &&... args) __aie_inline { return ::negmul_elem_16_2_conf(args...); };
            if constexpr (MulOp == MulMacroOp::Add_Mul)           return [](auto &&... args) __aie_inline { return ::mac_elem_16_2_conf(args...); };
            if constexpr (MulOp == MulMacroOp::Sub_Mul)           return [](auto &&... args) __aie_inline { return ::msc_elem_16_2_conf(args...); };
        }
        else if constexpr (std::is_same_v<DataType, int32> || std::is_same_v<CoeffType, int32>) {
            // Emulated mul intrinsics
            if constexpr (MulOp == MulMacroOp::Mul)               return [](auto &&... args) __aie_inline { return mul_conv_16x4_conf(args...); };
            if constexpr (MulOp == MulMacroOp::NegMul)            return [](auto &&... args) __aie_inline { return negmul_conv_16x4_conf(args...); };
            if constexpr (MulOp == MulMacroOp::Add_Mul)           return [](auto &&... args) __aie_inline { return mac_conv_16x4_conf(args...); };
            if constexpr (MulOp == MulMacroOp::Sub_Mul)           return [](auto &&... args) __aie_inline { return msc_conv_16x4_conf(args...); };
        }
        else {
            if constexpr (MulOp == MulMacroOp::Mul || MulOp == MulMacroOp::MulConj1 || MulOp == MulMacroOp::MulConj1Conj2 || MulOp == MulMacroOp::MulConj2)
                return [](auto &&... args) __aie_inline { return ::mul_conv_16x4_conf(args...); };
            if constexpr (MulOp == MulMacroOp::NegMul || MulOp == MulMacroOp::NegMulConj1 || MulOp == MulMacroOp::NegMulConj1Conj2 || MulOp == MulMacroOp::NegMulConj2)
                return [](auto &&... args) __aie_inline { return ::negmul_conv_16x4_conf(args...); };
            if constexpr (MulOp == MulMacroOp::Add_Mul || MulOp == MulMacroOp::Add_MulConj1 || MulOp == MulMacroOp::Add_MulConj1Conj2 || MulOp == MulMacroOp::Add_MulConj2)
                return [](auto &&... args) __aie_inline { return ::mac_conv_16x4_conf(args...); };
            if constexpr (MulOp == MulMacroOp::Sub_Mul || MulOp == MulMacroOp::Sub_MulConj1 || MulOp == MulMacroOp::Sub_MulConj1Conj2 || MulOp == MulMacroOp::Sub_MulConj2)
                return [](auto &&... args) __aie_inline { return ::msc_conv_16x4_conf(args...); };
        }
    }
}

// Emulated mul_conv_16x4 intrinsics pay a cost for zeroization support
// If possible, use versions that don't require zeroization
template <MulMacroOp MulOp, unsigned Lanes, int DataStep, typename CoeffType, typename DataType>
static constexpr auto sliding_mul_acc48_get_mul_op_no_conf()
{
    if constexpr (std::is_same_v<DataType, int32> || std::is_same_v<CoeffType, int32>) {
        // Emulated mul intrinsics
        if constexpr (MulOp == MulMacroOp::Mul)     return [](auto &&... args) __aie_inline { return mul_conv_16x4(args...); };
        if constexpr (MulOp == MulMacroOp::NegMul)  return [](auto &&... args) __aie_inline { return negmul_conv_16x4(args...); };
        if constexpr (MulOp == MulMacroOp::Add_Mul) return [](auto &&... args) __aie_inline { return mac_conv_16x4(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_Mul) return [](auto &&... args) __aie_inline { return msc_conv_16x4(args...); };
    }
}


template <unsigned Lanes, unsigned Points, typename CoeffType, typename DataType>
struct sliding_mul_bits_impl<Lanes, Points, 1, 1, 1, 64, 16, 16, CoeffType, DataType>
{
    using coeff_type   = CoeffType;
    using data_type    = DataType;
    using accum_tag    = accum_tag_for_mul_types<data_type, coeff_type, 64>;
    using accum_type   = accum<accum_tag, Lanes>;
    using accum_type16 = accum<accum_tag, 16>;

    static constexpr unsigned columns_per_mul = 4;
    static constexpr unsigned         num_mul = Points / columns_per_mul;
    static_assert(Points >= columns_per_mul);
    static_assert(Points % columns_per_mul  == 0);

    static constexpr unsigned native_data_elems = native_vector_length_v<data_type>;

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
        constexpr unsigned data_elems = std::max(N_Data, native_data_elems);

        data_start = data_start % data_elems;

        constexpr auto mac_op = sliding_mul_acc48_get_mul_op<add_to_op<MulOp>(), 16, 1, coeff_type, data_type>();
        constexpr auto mul_op = sliding_mul_acc48_get_mul_op<MulOp, 16, 1, coeff_type, data_type>();

        accum_type ret(acc...);

        utils::unroll_for<unsigned, 0, std::max(1u, Lanes / 16)>([&](auto idx_y) __aie_inline {
            const unsigned  data_start_local = (data_start + idx_y * 16) % data_elems;
            const unsigned coeff_start_local = coeff_start % N_Coeff;
            accum_type16 tmp;

            tmp = mul_op(shuffle_down_rotate<data_type,  data_elems>::run( data.template grow_replicate<data_elems>(),  data_start_local).template extract<32>(0),
                         data_sign,
                         shuffle_down_rotate<coeff_type,         32>::run(coeff.template grow_replicate<32>(),         coeff_start_local),
                         coeff_sign,
                         utils::get_nth<0>(ret.template grow_extract<16>(idx_y), acc)...,
                         utils::get_nth<0>(zero_acc, acc)...);

            utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
                const unsigned data_start_local  =  (data_start + columns_per_mul * idx + idx_y * 16) % data_elems;
                const unsigned coeff_start_local = (coeff_start + columns_per_mul * idx) % N_Coeff;
                tmp = mac_op(shuffle_down_rotate<data_type,  data_elems>::run( data.template grow_replicate<data_elems>(),  data_start_local).template extract<32>(0),
                             data_sign,
                             shuffle_down_rotate<coeff_type,         32>::run(coeff.template grow_replicate<32>(),         coeff_start_local),
                             coeff_sign,
                             tmp, false);
            });

            if constexpr (Lanes <= 16)
                ret = tmp.template extract<Lanes>(0);
            else
                ret.insert(idx_y, tmp);
        });

        return ret;
    }
};

template <unsigned Lanes, unsigned Points, typename CoeffType, typename DataType>
struct sliding_mul_bits_impl<Lanes, Points, 1, 16, 1, 64, 16, 16, CoeffType, DataType>
{
    using coeff_type   = CoeffType;
    using data_type    = DataType;
    using accum_tag    = accum_tag_for_mul_types<data_type, coeff_type, 64>;
    using accum_type   = accum<accum_tag, Lanes>;
    using accum_type16 = accum<accum_tag, 16>;

    static constexpr unsigned columns_per_mul = 2;
    static constexpr unsigned         num_mul = Points / columns_per_mul;
    static_assert(Points >= columns_per_mul);
    static_assert(Points % columns_per_mul == 0);

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
        constexpr unsigned data_elems = std::max(N_Data, 32u);

        constexpr auto mac_op = sliding_mul_acc48_get_mul_op<add_to_op<MulOp>(), 16, 16, coeff_type, data_type>();
        constexpr auto mul_op = sliding_mul_acc48_get_mul_op<             MulOp, 16, 16, coeff_type, data_type>();

        accum_type ret(acc...);

        utils::unroll_for<unsigned, 0, std::max(1u, Lanes / 16)>([&](auto idx_y) __aie_inline {
            const unsigned data_start_local = (data_start + idx_y * 16) % data_elems;
            accum_type16 tmp;

            tmp = mul_op(shuffle_down_rotate<data_type,  data_elems>::run( data.template grow_replicate<data_elems>(), data_start_local).template extract<32>(0),
                         data_sign,
                         ::concat(broadcast<int16, 16>::run(coeff[coeff_start       % N_Coeff]),
                                  broadcast<int16, 16>::run(coeff[(coeff_start + 1) % N_Coeff])),
                         coeff_sign,
                         utils::get_nth<0>(ret.template grow_extract<16>(idx_y), acc)...,
                         utils::get_nth<0>(zero_acc, acc)...);

            utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
                const unsigned data_start_local = (data_start + idx_y * 16 + idx * 32) % data_elems;
                tmp = mac_op(shuffle_down_rotate<data_type,  data_elems>::run( data.template grow_replicate<data_elems>(), data_start_local).template extract<32>(0),
                             data_sign,
                             ::concat(broadcast<int16, 16>::run(coeff[(coeff_start + idx * columns_per_mul    ) % N_Coeff]),
                                      broadcast<int16, 16>::run(coeff[(coeff_start + idx * columns_per_mul + 1) % N_Coeff])),
                             coeff_sign,
                             tmp, false);
            });

            if constexpr (Lanes <= 16)
                ret = tmp.template extract<Lanes>(0);
            else
                ret.insert(idx_y, tmp);
        });

        return ret;
    }
};

template <unsigned Lanes, unsigned Points, typename CoeffType, typename DataType>
    requires(!is_complex_v<CoeffType> && !is_complex_v<DataType>)
struct sliding_mul_bits_impl<Lanes, Points, 1, 1, 1, 64, 16, 32, CoeffType, DataType>
{
    using coeff_type   = CoeffType;
    using data_type    = DataType;
    using accum_tag    = accum_tag_for_mul_types<data_type, coeff_type, 64>;
    using accum_type   = accum<accum_tag, Lanes>;
    using accum_type16 = accum<accum_tag, 16>;

    static constexpr unsigned columns_per_mul = 4;
    static constexpr unsigned         num_mul = Points / columns_per_mul;
    static_assert(Points >= columns_per_mul);
    static_assert(Points % columns_per_mul  == 0);

    static constexpr unsigned native_data_elems = native_vector_length_v<data_type>;

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

        data_start = data_start % data_elems;

        constexpr auto mul_op_conf = sliding_mul_acc48_get_mul_op<MulOp, 16, 1, coeff_type, data_type>();
        // Non-conf functions can be more efficiently emulated
        constexpr auto mac_op = sliding_mul_acc48_get_mul_op_no_conf<add_to_op<MulOp>(), 16, 1, coeff_type, data_type>();
        constexpr auto mul_op = sliding_mul_acc48_get_mul_op_no_conf<MulOp, 16, 1, coeff_type, data_type>();

        const vector<data_type, data_elems>  data2 = data.template grow_replicate<data_elems>();
        const vector<coeff_type, 32>        coeff2 = coeff.template grow_replicate<32>();

        accum_type ret(acc...);

        utils::unroll_for<unsigned, 0, std::max(1u, Lanes / 16)>([&](auto idx_y) __aie_inline {
            const unsigned  data_start_local = (data_start + idx_y * 16) % data_elems;
            const unsigned coeff_start_local = coeff_start % N_Coeff;
            accum_type16 tmp;

            vector<data_type, data_elems>  data_local = shuffle_down_rotate<data_type,  data_elems>::run( data2, data_start_local);

            if constexpr (data_elems > native_data_elems) {
                if (chess_manifest(zero_acc == false))
                    tmp = mul_op(data_local.template extract<native_data_elems>(0), data_local.template extract<native_data_elems>(1), data_sign,
                                 shuffle_down_rotate<coeff_type, 32>::run(coeff2, coeff_start_local), coeff_sign,
                                 utils::get_nth<0>(ret.template grow_extract<16>(idx_y), acc)...);
                else
                    tmp = mul_op_conf(data_local.template extract<native_data_elems>(0), data_local.template extract<native_data_elems>(1), data_sign,
                                      shuffle_down_rotate<coeff_type, 32>::run(coeff2, coeff_start_local), coeff_sign,
                                      utils::get_nth<0>(ret.template grow_extract<16>(idx_y), acc)...,
                                      utils::get_nth<0>(zero_acc, acc)...);
            }
            else {
                if (chess_manifest(zero_acc == false))
                    tmp = mul_op(data_local, data_local, data_sign,
                                 shuffle_down_rotate<coeff_type, 32>::run(coeff2, coeff_start_local), coeff_sign,
                                 utils::get_nth<0>(ret.template grow_extract<16>(idx_y), acc)...);
                else
                    tmp = mul_op_conf(data_local, data_local, data_sign,
                                      shuffle_down_rotate<coeff_type, 32>::run(coeff2, coeff_start_local), coeff_sign,
                                      utils::get_nth<0>(ret.template grow_extract<16>(idx_y), acc)...,
                                      utils::get_nth<0>(zero_acc, acc)...);
            }

            utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
                const unsigned data_start_local  =  (data_start + columns_per_mul * idx + idx_y * 16) % data_elems;
                const unsigned coeff_start_local = (coeff_start + columns_per_mul * idx) % N_Coeff;

                vector<data_type, data_elems>  data_local = shuffle_down_rotate<data_type,  data_elems>::run( data2, data_start_local);

                if constexpr (data_elems > native_data_elems) {
                    tmp = mac_op(data_local.template extract<native_data_elems>(0), data_local.template extract<native_data_elems>(1), data_sign,
                                 shuffle_down_rotate<coeff_type, 32>::run(coeff2, coeff_start_local), coeff_sign,
                                 tmp);
                }
                else {
                    tmp = mac_op(data_local, data_local, data_sign,
                                 shuffle_down_rotate<coeff_type, 32>::run(coeff2, coeff_start_local), coeff_sign,
                                 tmp);
                }
            });

            if constexpr (Lanes <= 16)
                ret = tmp.template extract<Lanes>(0);
            else
                ret.insert(idx_y, tmp);
        });

        return ret;
    }
};

template <unsigned Lanes, unsigned Points, typename CoeffType, typename DataType>
    requires(!is_complex_v<CoeffType> && !is_complex_v<DataType>)
struct sliding_mul_bits_impl<Lanes, Points, 1, 1, 1, 64, 32, 16, CoeffType, DataType>
{
    using coeff_type   = CoeffType;
    using data_type    = DataType;
    using accum_tag    = accum_tag_for_mul_types<data_type, coeff_type, 64>;
    using accum_type   = accum<accum_tag, Lanes>;
    using accum_type16 = accum<accum_tag, 16>;

    static constexpr unsigned columns_per_mul = 4;
    static constexpr unsigned         num_mul = Points / columns_per_mul;
    static_assert(Points >= columns_per_mul);
    static_assert(Points % columns_per_mul  == 0);

    static constexpr unsigned native_data_elems = native_vector_length_v<data_type>;
    static constexpr unsigned native_coeff_elems = native_vector_length_v<coeff_type>;

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
        constexpr unsigned coeff_elems = std::max(N_Coeff, native_coeff_elems);

        data_start = data_start % data_elems;
        coeff_start = coeff_start % coeff_elems;

        constexpr auto mul_op_conf = sliding_mul_acc48_get_mul_op<MulOp, 16, 1, coeff_type, data_type>();
        // Non-conf functions can be more efficiently emulated
        constexpr auto mul_op = sliding_mul_acc48_get_mul_op_no_conf<MulOp, 16, 1, coeff_type, data_type>();
        constexpr auto mac_op = sliding_mul_acc48_get_mul_op_no_conf<add_to_op<MulOp>(), 16, 1, coeff_type, data_type>();

        const vector<data_type,  data_elems>  data2  = data.template grow_replicate<data_elems>();
        const vector<coeff_type, coeff_elems> coeff2 = coeff.template grow_replicate<coeff_elems>();

        accum_type ret(acc...);

        utils::unroll_for<unsigned, 0, std::max(1u, Lanes / 16)>([&](auto idx_y) __aie_inline {
            const unsigned  data_start_local = (data_start + idx_y * 16) % data_elems;
            const unsigned coeff_start_local = coeff_start % coeff_elems;
            accum_type16 tmp;

            vector<data_type, data_elems>   data_local  = shuffle_down_rotate<data_type, data_elems>::run( data2, data_start_local);
            vector<coeff_type, coeff_elems> coeff_local = shuffle_down_rotate<coeff_type, coeff_elems>::run( coeff2, coeff_start_local);

            if constexpr (coeff_elems > native_coeff_elems) {
                if (chess_manifest(zero_acc == false))
                    tmp = mul_op(data_local.template extract<native_data_elems>(0),
                                 data_sign,
                                 coeff_local.template extract<native_coeff_elems>(0), coeff_local.template extract<native_coeff_elems>(1),
                                 coeff_sign,
                                 utils::get_nth<0>(ret.template grow_extract<16>(idx_y), acc)...);
                else
                    tmp = mul_op_conf(data_local.template extract<native_data_elems>(0),
                                      data_sign,
                                      coeff_local.template extract<native_coeff_elems>(0), coeff_local.template extract<native_coeff_elems>(1),
                                      coeff_sign,
                                      utils::get_nth<0>(ret.template grow_extract<16>(idx_y), acc)...,
                                      utils::get_nth<0>(zero_acc, acc)...);
            }
            else {
                if (chess_manifest(zero_acc == false))
                    tmp = mul_op(data_local.template extract<native_data_elems>(0),
                                 data_sign,
                                 coeff_local, coeff_local,
                                 coeff_sign,
                                 utils::get_nth<0>(ret.template grow_extract<16>(idx_y), acc)...);
                else
                    tmp = mul_op_conf(data_local.template extract<native_data_elems>(0),
                                      data_sign,
                                      coeff_local, coeff_local,
                                      coeff_sign,
                                      utils::get_nth<0>(ret.template grow_extract<16>(idx_y), acc)...,
                                      utils::get_nth<0>(zero_acc, acc)...);
            }

            utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
                const unsigned data_start_local  =  (data_start + columns_per_mul * idx + idx_y * 16) % data_elems;
                const unsigned coeff_start_local = (coeff_start + columns_per_mul * idx) % coeff_elems;

                vector<data_type, data_elems>   data_local  = shuffle_down_rotate<data_type, data_elems>::run( data2, data_start_local);
                vector<coeff_type, coeff_elems> coeff_local = shuffle_down_rotate<coeff_type, coeff_elems>::run(coeff2, coeff_start_local);

                if constexpr (coeff_elems > native_coeff_elems) {
                    tmp = mac_op(data_local.template extract<native_data_elems>(0),
                                 data_sign,
                                 coeff_local.template extract<native_coeff_elems>(0), coeff_local.template extract<native_coeff_elems>(1),
                                 coeff_sign,
                                 tmp);
                }
                else {
                    tmp = mac_op(data_local.template extract<native_data_elems>(0),
                                 data_sign,
                                 coeff_local, coeff_local,
                                 coeff_sign,
                                 tmp);
                }
            });

            if constexpr (Lanes <= 16)
                ret = tmp.template extract<Lanes>(0);
            else
                ret.insert(idx_y, tmp);
        });

        return ret;
    }
};

template <unsigned Lanes, unsigned Points, typename CoeffType, typename DataType>
    requires(!is_complex_v<CoeffType> && !is_complex_v<DataType>)
struct sliding_mul_bits_impl<Lanes, Points, 1, 1, 1, 64, 32, 32, CoeffType, DataType>
{
    using coeff_type   = CoeffType;
    using data_type    = DataType;
    using accum_tag    = accum_tag_for_mul_types<data_type, coeff_type, 64>;
    using accum_type   = accum<accum_tag, Lanes>;
    using accum_type16 = accum<accum_tag, 16>;

    static constexpr unsigned columns_per_mul = 4;
    static constexpr unsigned         num_mul = Points / columns_per_mul;
    static_assert(Points >= columns_per_mul);
    static_assert(Points % columns_per_mul  == 0);

    static constexpr unsigned native_data_elems = native_vector_length_v<data_type>;

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

        data_start = data_start % data_elems;

        constexpr auto mul_op_conf = sliding_mul_acc48_get_mul_op<MulOp, 16, 1, coeff_type, data_type>();
        // Non-conf functions can be more efficiently emulated
        constexpr auto mul_op      = sliding_mul_acc48_get_mul_op_no_conf<MulOp, 16, 1, coeff_type, data_type>();
        constexpr auto mac_op      = sliding_mul_acc48_get_mul_op_no_conf<add_to_op<MulOp>(), 16, 1, coeff_type, data_type>();

        const vector<data_type, data_elems>  data2 = data.template grow_replicate<data_elems>();
        const vector<coeff_type, 16>        coeff2 = coeff.template grow_replicate<16>();

        accum_type ret(acc...);

        utils::unroll_for<unsigned, 0, std::max(1u, Lanes / 16)>([&](auto idx_y) __aie_inline {
            const unsigned  data_start_local = (data_start + idx_y * 16) % data_elems;
            const unsigned coeff_start_local = coeff_start % N_Coeff;
            accum_type16 tmp;

            vector<data_type, data_elems>  data_local = shuffle_down_rotate<data_type,  data_elems>::run( data2, data_start_local);
            vector<coeff_type, 16>        coeff_local = shuffle_down_rotate<coeff_type,         16>::run(coeff2, coeff_start_local);

            if constexpr (data_elems > native_data_elems) {
                if (chess_manifest(zero_acc == false))
                    tmp = mul_op(data_local.template extract<native_data_elems>(0), data_local.template extract<native_data_elems>(1),
                                 data_sign,
                                 coeff_local, vector<coeff_type, 16>(),
                                 coeff_sign,
                                 utils::get_nth<0>(ret.template grow_extract<16>(idx_y), acc)...);
                else
                    tmp = mul_op_conf(data_local.template extract<native_data_elems>(0), data_local.template extract<native_data_elems>(1),
                                      data_sign,
                                      coeff_local, vector<coeff_type, 16>(),
                                      coeff_sign,
                                      utils::get_nth<0>(ret.template grow_extract<16>(idx_y), acc)...,
                                      utils::get_nth<0>(zero_acc, acc)...);
            }
            else {
                if (chess_manifest(zero_acc == false))
                    tmp = mul_op(data_local, data_local,
                                 data_sign,
                                 coeff_local, vector<coeff_type, 16>(),
                                 coeff_sign,
                                 utils::get_nth<0>(ret.template grow_extract<16>(idx_y), acc)...);
                else
                    tmp = mul_op_conf(data_local, data_local,
                                      data_sign,
                                      coeff_local, vector<coeff_type, 16>(),
                                      coeff_sign,
                                      utils::get_nth<0>(ret.template grow_extract<16>(idx_y), acc)...,
                                      utils::get_nth<0>(zero_acc, acc)...);
            }

            utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
                const unsigned data_start_local  =  (data_start + columns_per_mul * idx + idx_y * 16) % data_elems;
                const unsigned coeff_start_local = (coeff_start + columns_per_mul * idx) % N_Coeff;

                vector<data_type, data_elems>  data_local = shuffle_down_rotate<data_type,  data_elems>::run( data2, data_start_local);
                vector<coeff_type, 16>        coeff_local = shuffle_down_rotate<coeff_type,         16>::run(coeff2, coeff_start_local);

                if constexpr (data_elems > native_data_elems) {
                    tmp = mac_op(data_local.template extract<native_data_elems>(0), data_local.template extract<native_data_elems>(1),
                                 data_sign,
                                 coeff_local, vector<coeff_type, 16>(),
                                 coeff_sign,
                                 tmp);
                }
                else {
                    tmp = mac_op(data_local, data_local,
                                 data_sign,
                                 coeff_local, vector<coeff_type, 16>(),
                                 coeff_sign,
                                 tmp);
                }
            });

            if constexpr (Lanes <= 16)
                ret = tmp.template extract<Lanes>(0);
            else
                ret.insert(idx_y, tmp);
        });

        return ret;
    }
};

#if __AIE_API_COMPLEX_VECTOR_SUPPORT__
template <unsigned Lanes, unsigned Points>
struct sliding_mul_bits_impl<Lanes, Points, 1, 1, 1, 64, 32, 16, cint16, int16>
{
    using coeff_type       = cint16;
    using data_type        =  int16;
    using accum_type       = accum<cacc64, Lanes>;
    using accum_type_half  = accum<cacc64, 8>;
    using accum_type_quart = accum<cacc64, 4>;
    using accum_int        = accum<acc64, 16>;

    static constexpr unsigned columns_per_mul = 4;
    static constexpr unsigned         num_mul = Points / columns_per_mul;
    static_assert(Points >= columns_per_mul);
    static_assert(Points % columns_per_mul == 0);

    static constexpr unsigned native_data_elems = native_vector_length_v<data_type>;

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

        data_start = data_start % data_elems;

        constexpr auto mul_op      = sliding_mul_acc48_get_mul_op<MulOp, 16, 1, coeff_type, data_type>();
        constexpr auto mac_op      = sliding_mul_acc48_get_mul_op<add_to_op<MulOp>(), 16, 1, coeff_type, data_type>();
        constexpr auto mul_conf_op = sliding_mul_acc48_get_mul_conf_op<MulOp, 16, 1, coeff_type, data_type>();
        constexpr auto mac_conf_op = sliding_mul_acc48_get_mul_conf_op<add_to_op<MulOp>(), 16, 1, coeff_type, data_type>();

        constexpr unsigned num_conj = has_conj2<MulOp>() ? 1 : 0;

        // Unzipping coeff
        vector<int16, 32> coef_re, coef_im;
        std::tie(coef_re, coef_im) = grow_all<32>(unzip_complex(coeff));

        // Unzipping input accumulator
        accum_int acc_re, acc_im;
        if constexpr (sizeof... (Acc) > 0)
            std::tie(acc_re, acc_im) = grow_all<accum_int::size()>(unzip_complex(acc...));

        const unsigned coeff_start_local = coeff_start % N_Coeff;

        auto data_shuffle = shuffle_down_rotate<int16, data_elems>::run(data.template grow_replicate<data_elems>(), data_start).template extract<32>(0);

        acc_re = mul_op(data_shuffle,
                        data_sign,
                        shuffle_down_rotate<int16, 32>::run(coef_re, coeff_start_local),
                        true,
                        utils::get_nth<0>(acc_re, acc)...,
                        utils::get_nth<0>(zero_acc, acc)...);

        acc_im = mul_conf_op(data_shuffle,
                             data_sign,
                             shuffle_down_rotate<int16, 32>::run(coef_im, coeff_start_local),
                             true,
                             utils::get_nth<0>(acc_im, acc)...,
                             utils::get_nth<0>(zero_acc, acc)...,
                             utils::get_nth<0>(0, acc)...,       // shift16
                             num_conj,                           // sub_mul
                             utils::get_nth<0>(0, acc)...);      // sub_acc
                             //negation of mul result for  -(coef.im*data.im)

        utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
            const unsigned data_start_local  = ( data_start + columns_per_mul * idx) % data_elems;
            const unsigned coeff_start_local = (coeff_start + columns_per_mul * idx) % N_Coeff;

            data_shuffle = shuffle_down_rotate<int16, data_elems>::run(data.template grow_replicate<data_elems>(), data_start_local).template extract<32>(0);

            acc_re = mac_op(data_shuffle,
                            data_sign,
                            shuffle_down_rotate<int16, 32>::run(coef_re, coeff_start_local),
                            true,
                            acc_re, false);

            acc_im = mac_conf_op(data_shuffle,
                                 data_sign,
                                 shuffle_down_rotate<int16, 32>::run(coef_im, coeff_start_local),
                                 true,
                                 acc_im,
                                 0,
                                 0,
                                 num_conj,
                                 0);
                                 //negation of mul result for  -(coef.im*data.im)
        });

        // Re-shuffling real and imaginary
        return combine_into_complex(acc_re.template extract<Lanes>(0), acc_im.template extract<Lanes>(0));
    }
};

template <unsigned Lanes, unsigned Points>
struct sliding_mul_bits_impl<Lanes, Points, 1, 1, 1, 64, 16, 32, int16, cint16>
{
    using coeff_type       =  int16;
    using data_type        = cint16;
    using accum_type       = accum<cacc64, Lanes>;
    using accum_type_half  = accum<cacc64, 8>;
    using accum_type_quart = accum<cacc64, 4>;
    using accum_int        = accum<acc64, 16>;

    static constexpr unsigned columns_per_mul = 4;
    static constexpr unsigned         num_mul = Points / columns_per_mul;
    static_assert(Points >= columns_per_mul);
    static_assert(Points % columns_per_mul  == 0);

    static constexpr unsigned native_data_elems = native_vector_length_v<data_type>;

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

        data_start = data_start % data_elems;

        constexpr auto mul_op      = sliding_mul_acc48_get_mul_op<MulOp, 16, 1, coeff_type, data_type>();
        constexpr auto mac_op      = sliding_mul_acc48_get_mul_op<add_to_op<MulOp>(), 16, 1, coeff_type, data_type>();
        constexpr auto mul_conf_op = sliding_mul_acc48_get_mul_conf_op<MulOp, 16, 1, coeff_type, data_type>();
        constexpr auto mac_conf_op = sliding_mul_acc48_get_mul_conf_op<add_to_op<MulOp>(), 16, 1, coeff_type, data_type>();

        constexpr unsigned num_conj = has_conj1<MulOp>()? 1 : 0;

        constexpr unsigned int16_data_elems = std::max(N_Data, 32u);
        constexpr unsigned coef_elems = std::max(N_Coeff, 32u);

        // Unzipping data
        vector<int16, int16_data_elems> data_re, data_im;
        std::tie(data_re, data_im) = grow_all<int16_data_elems>(unzip_complex(data));

        // Unzipping input accumulator
        accum_int acc_re, acc_im;
        if constexpr (sizeof...(Acc) > 0)
            std::tie(acc_re, acc_im) = grow_all<accum_int::size()>(unzip_complex(acc)...);

        const unsigned coeff_start_local = coeff_start % N_Coeff;

        auto coeff_shuffle = shuffle_down_rotate<int16, coef_elems>::run(coeff.template grow_replicate<coef_elems>(), coeff_start_local);

        acc_re = mul_op(shuffle_down_rotate<int16, int16_data_elems>::run(data_re, data_start),
                        true,
                        coeff_shuffle,
                        coeff_sign,
                        utils::get_nth<0>(acc_re, acc)...,
                        utils::get_nth<0>(zero_acc, acc)...);

        acc_im = mul_conf_op(shuffle_down_rotate<int16, int16_data_elems>::run(data_im, data_start),
                             true,
                             coeff_shuffle,
                             coeff_sign,
                             utils::get_nth<0>(acc_im, acc)...,
                             utils::get_nth<0>(zero_acc, acc)...,
                             utils::get_nth<0>(0, acc)...,       // shift16
                             num_conj,                           // sub_mul
                             utils::get_nth<0>(0, acc)...);      // sub_acc
                             //negation of mul result for  -(coef.im*data.im)

        utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
            const unsigned data_start_local  = ( data_start + columns_per_mul * idx) % data_elems;
            const unsigned coeff_start_local = (coeff_start + columns_per_mul * idx) % N_Coeff;

            // TODO: The shift value in the shift_bytes instruction cannot go beyond 64. Does this ever happen here?
            coeff_shuffle = shuffle_down_rotate<int16, coef_elems>::run(coeff.template grow_replicate<coef_elems>(), coeff_start_local);

            acc_re = mac_op(shuffle_down_rotate<int16, 32>::run(data_re, data_start_local),
                            true,
                            coeff_shuffle,
                            coeff_sign,
                            acc_re, false);

            acc_im = mac_conf_op(shuffle_down_rotate<int16, 32>::run(data_im, data_start_local),
                                 true,
                                 coeff_shuffle,
                                 coeff_sign,
                                 acc_im,
                                 0,
                                 0,
                                 num_conj,
                                 0);
                                 //negation of mul result for  -(coef.im*data.im)
        });

        // Re-shuffling real and imaginary
        return combine_into_complex(acc_re.template extract<Lanes>(0),
                                    acc_im.template extract<Lanes>(0));
    }
};

template <unsigned Lanes, unsigned Points>
struct sliding_mul_bits_impl<Lanes, Points, 1, 1, 1, 64, 32, 32, cint16, cint16>
{
    using coeff_type       = cint16;
    using data_type        = cint16;
    using accum_tag        = cacc64;
    using real_accum_tag   = acc64;
    using accum_type_half  = accum<accum_tag, 8>;
    using accum_type_quart = accum<accum_tag, 4>;
    using accum_int        = accum<acc64, 16>;

    static constexpr unsigned   lanes_per_mul = 16;
    static constexpr unsigned columns_per_mul = 4;
    static constexpr unsigned         num_mul = Points / columns_per_mul;
    static_assert(Points >= columns_per_mul);
    static_assert(Points % columns_per_mul  == 0);

    using accum_type       = accum<accum_tag, Lanes>;
    using real_accum_type  = accum<real_accum_tag, Lanes>;
    using partial_sum_type = std::array<real_accum_type, 2>;

    static constexpr unsigned native_data_elems = native_vector_length_v<data_type>;

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
                    return std::array{v.template cast_to<real_accum_tag>()...};
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
        constexpr auto mul_op      = sliding_mul_acc48_get_mul_op<MulOp, lanes_per_mul, 1, coeff_type, data_type>();
        constexpr auto mac_op      = sliding_mul_acc48_get_mul_op<add_to_op<MulOp>(), lanes_per_mul, 1, coeff_type, data_type>();
        constexpr auto mul_conf_op = sliding_mul_acc48_get_mul_conf_op<MulOp, lanes_per_mul, 1, coeff_type, data_type>();
        constexpr auto mac_conf_op = sliding_mul_acc48_get_mul_conf_op<add_to_op<MulOp>(), lanes_per_mul, 1, coeff_type, data_type>();

        // Even though 'coeff' argument comes first in this function,
        // Conj1 refers to data conjugation and Conj2 to coefficient's
        constexpr bool conjData  = has_conj1<MulOp>();
        constexpr bool conjCoeff = has_conj2<MulOp>();

        // Unzipping data/coef
        // For small number of lanes, prefer using grow_replicate first to reduce the risk of replicating small vectors
        // Replicating small vectors requires shuffle operations since VMOV does not support less than 512b registers
        auto unzip_and_grow = [](const auto &v) __aie_inline {
            return v.size() <= 16 ? unzip_complex(v.template grow_replicate<32>())
                                  : grow_all<32>(unzip_complex(v));
        };

        vector<int16, 32> coef_re, coef_im;
        std::tie(coef_re, coef_im) = unzip_and_grow(coeff);

        vector<int16, 32> data_re, data_im;
        std::tie(data_re, data_im) = unzip_and_grow(data);

        // Unzipping input accumulator
        accum<real_accum_tag, lanes_per_mul> acc_re, acc_im;
        if constexpr (N > 0) {
            std::tie(acc_re, acc_im) = utils::apply_tuple(
                    [](auto... a) __aie_inline {
                        return std::array{a.template grow<lanes_per_mul>()...};
                    },
                    acc);
        }

        const unsigned data_start_local  =  data_start % 32;
        const unsigned coeff_start_local = coeff_start % N_Coeff;

        auto shift_vector = [](auto v, unsigned shift) __aie_inline {
            return shuffle_down_rotate<int16, 32>::run(v, shift);
        };

        vector data_re_l = shift_vector(data_re,  data_start_local);
        vector data_im_l = shift_vector(data_im,  data_start_local);
        vector coef_re_l = shift_vector(coef_re, coeff_start_local);
        vector coef_im_l = shift_vector(coef_im, coeff_start_local);

        // Operations are reordered so that the coeff and data components are reused as much as possible
        // E.g. real components for data and coeff first, and imaginary components last when allowed.
        if constexpr (N > 0) {
            acc_re = mul_op(data_re_l, coef_re_l,
                            acc_re, zero_acc);
            acc_im = mul_conf_op(data_re_l, coef_im_l,
                                 acc_im, zero_acc,
                                 0, conjCoeff, 0);
        }
        else {
            acc_re = mul_op(data_re_l, coef_re_l);
            acc_im = mul_conf_op(data_re_l, coef_im_l, conjCoeff);
        }

        acc_im = mac_conf_op(data_im_l, coef_re_l,
                             acc_im, 0, 0, conjData, 0);

        acc_re = mac_conf_op(data_im_l, coef_im_l,
                             acc_re, 0, 0, 1 - (conjCoeff ^ conjData), 0); //negation of mul result for  -(coef.im*data.im)

        utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
            data_re_l = shift_vector(data_re_l, columns_per_mul);
            data_im_l = shift_vector(data_im_l, columns_per_mul);

            // When reusing the same coefficients across consecutive sliding_mul operations,
            // chess tries to reuse shifted vectors, leading to spilling and restoring them to/from the stack.
            coef_re_l = __aie_api_duplicate(shift_vector(coef_re_l, columns_per_mul));
            coef_im_l = __aie_api_duplicate(shift_vector(coef_im_l, columns_per_mul));

            acc_re = mac_op(data_re_l, coef_re_l, acc_re, false);

            acc_im = mac_conf_op(data_re_l, coef_im_l,
                                 acc_im, 0, 0, conjCoeff, 0);

            acc_im = mac_conf_op(data_im_l, coef_re_l,
                                 acc_im, 0, 0, conjData, 0);

            constexpr bool subMul = (conjCoeff == conjData); // negation of mul result for -(coef.im*data.im)
            acc_re = mac_conf_op(data_im_l, coef_im_l,
                                 acc_re, 0, 0, subMul, 0);
        });

        // Re-shuffling real and imaginary
        return {acc_re.template extract<Lanes>(0), acc_im.template extract<Lanes>(0)};
    }
};

template <unsigned Lanes, unsigned Points, unsigned CoeffBits, unsigned DataBits, typename CoeffType,
          typename DataType>
    requires((CoeffBits == 64 || DataBits == 64) && is_complex_v<CoeffType> && is_complex_v<DataType>)
struct sliding_mul_bits_impl<Lanes, Points, 1, 1, 1, 64, CoeffBits, DataBits, CoeffType, DataType>
{
    using coeff_type  = CoeffType;
    using data_type   = DataType;
    using accum_tag   = accum_tag_for_mul_types<data_type, coeff_type, 64>;
    using accum_type  = accum<accum_tag, Lanes>;
    using accum_type8 = accum<accum_tag, 8>;

    static constexpr unsigned columns_per_mul = 1;
    static constexpr unsigned         num_mul = Points / columns_per_mul;
    static_assert(Points >= columns_per_mul);
    static_assert(Points % columns_per_mul  == 0);

    static constexpr unsigned native_data_elems  = native_vector_length_v<data_type>;
    static constexpr unsigned native_coeff_elems = native_vector_length_v<coeff_type>;

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

        data_start = data_start % data_elems;

        constexpr auto mac_op = sliding_mul_acc48_get_mul_op<add_to_op<MulOp>(), 8, 1, coeff_type, data_type>();
        constexpr auto mul_op = sliding_mul_acc48_get_mul_op<MulOp, 8, 1, coeff_type, data_type>();

        accum_type ret(acc...);

        utils::unroll_for<unsigned, 0, std::max(1u, Lanes / 8)>([&](auto idx_y) __aie_inline {
            const unsigned data_start_local = (data_start + idx_y * 8) % data_elems;
            const unsigned coeff_start_local = coeff_start % N_Coeff;
            accum_type8 tmp;

            if constexpr (DataBits < 64) {
                tmp = mul_op(broadcast<coeff_type, native_coeff_elems>::run(coeff[coeff_start_local]),
                             shuffle_down_rotate<data_type,  data_elems>::run( data.template grow_replicate<data_elems>(), data_start_local).template extract<native_data_elems>(0),
                             utils::get_nth<0>(ret.template grow_extract<8>(idx_y), acc)...,
                             utils::get_nth<0>(zero_acc, acc)...);
            }
            else {
                tmp = mul_op(shuffle_down_rotate<data_type,  data_elems>::run( data.template grow_replicate<data_elems>(), data_start_local).template extract<native_data_elems>(0),
                             broadcast<coeff_type, native_coeff_elems>::run(coeff[coeff_start_local]),
                             utils::get_nth<0>(ret.template grow_extract<8>(idx_y), acc)...,
                             utils::get_nth<0>(zero_acc, acc)...);
            }

            utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
                const unsigned data_start_local  =  (data_start + columns_per_mul * idx + idx_y * 8) % data_elems;
                const unsigned coeff_start_local = (coeff_start + columns_per_mul * idx) % N_Coeff;

                if constexpr (DataBits < 64) {
                    tmp = mac_op(broadcast<coeff_type, native_coeff_elems>::run(coeff[coeff_start_local]),
                                 shuffle_down_rotate<data_type,  data_elems>::run( data.template grow_replicate<data_elems>(), data_start_local).template extract<native_data_elems>(0),
                                 tmp, false);
                }
                else {
                    tmp = mac_op(shuffle_down_rotate<data_type,  data_elems>::run( data.template grow_replicate<data_elems>(), data_start_local).template extract<native_data_elems>(0),
                                 broadcast<coeff_type, native_coeff_elems>::run(coeff[coeff_start_local]),
                                 tmp, false);
                }
            });

            if constexpr (Lanes <= 8)
                ret = tmp.template extract<Lanes>(0);
            else
                ret.insert(idx_y, tmp);
        });

        return ret;
    }
};

/* Complex * real
 */
template <unsigned Lanes, unsigned Points, unsigned CoeffTypeBits, unsigned DataTypeBits, typename CoeffType,
          typename DataType>
    requires(is_complex_v<CoeffType> && !is_complex_v<DataType>)
struct sliding_mul_bits_impl<Lanes, Points, 1, 1, 1, 64, CoeffTypeBits, DataTypeBits, CoeffType, DataType>
{
    // Complex * real can be decomposed in two real * real multiplications.
    // This means we always use 16x4 multiplication mode, hence 4 columns per multiply operation
    static constexpr unsigned lanes_per_mul   = 16;
    static constexpr unsigned columns_per_mul = 4;
    static constexpr unsigned num_mul         = Points / columns_per_mul;
    static_assert(Points >= columns_per_mul);
    static_assert(Points % columns_per_mul == 0);

    using coeff_type = CoeffType;
    using data_type  = DataType;
    using accum_type = accum<cacc64, Lanes>;
    using accum_int  = accum<acc64, lanes_per_mul>;

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
        using coeff_int_t = utils::get_complex_component_type_t<CoeffType>;
        using sub_op = sliding_mul_bits_impl<lanes_per_mul,
                                             columns_per_mul,
                                             1,
                                             1,
                                             1,
                                             64,
                                             CoeffTypeBits / 2,
                                             DataTypeBits,
                                             coeff_int_t,
                                             data_type>;

        constexpr MulMacroOp MacOp = add_to_op<MulOp>();
        constexpr unsigned data_elems = std::max(N_Data, native_vector_length_v<DataType>);

        using rotate_data = shuffle_down_rotate<DataType, data_elems>;

        accum_type result;

        // Unzipping coeff
        vector<coeff_int_t, N_Coeff> coeff_re, coeff_im;
        std::tie(coeff_re, coeff_im) = unzip_complex(coeff);

        // Unzipping accum
        accum_int acc_re, acc_im;
        if constexpr (sizeof... (Acc) > 0)
            std::tie(acc_re, acc_im) = grow_all<accum_int::size()>(unzip_complex(acc...));

        vector data_grown = data.template grow_replicate<data_elems>();

        utils::unroll_for<unsigned, 0, std::max(1u, Lanes / lanes_per_mul)>([&](auto idx_y) __aie_inline {
            const unsigned data_start_y  = (data_start + idx_y * lanes_per_mul) % data_elems;
            const unsigned coeff_start_y = coeff_start % N_Coeff;

            vector<DataType, data_elems> data_shuffle
                = rotate_data::run(data_grown, data_start_y);

            accum_int local_re, local_im;
            local_re = sub_op::template run<MulOp>(coeff_re,
                                                   coeff_start_y,
                                                   coeff_sign,
                                                   data_shuffle,
                                                   0,
                                                   data_sign,
                                                   zero_acc,
                                                   utils::get_nth<0>(acc_re, acc)
                                                        .template grow_extract<lanes_per_mul>(idx_y)...);

            local_im = sub_op::template run<MulOp>(coeff_im,
                                                   coeff_start_y,
                                                   coeff_sign,
                                                   data_shuffle,
                                                   0,
                                                   data_sign,
                                                   zero_acc,
                                                   utils::get_nth<0>(acc_im, acc)
                                                        .template grow_extract<lanes_per_mul>(idx_y)...);

            utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
                const unsigned data_start_x  = (data_start + columns_per_mul * idx + idx_y * lanes_per_mul) % data_elems;
                const unsigned coeff_start_x = (coeff_start + columns_per_mul * idx) % N_Coeff;

                vector<DataType, data_elems> data_shuffle = rotate_data::run(data_grown, data_start_x);

                local_re = sub_op::template run<MacOp>(coeff_re,
                                                       coeff_start_x,
                                                       coeff_sign,
                                                       data_shuffle,
                                                       0,
                                                       data_sign,
                                                       0,
                                                       local_re);

                local_im = sub_op::template run<MacOp>(coeff_im,
                                                       coeff_start_x,
                                                       coeff_sign,
                                                       data_shuffle,
                                                       0,
                                                       data_sign,
                                                       0,
                                                       local_im);
            });

            auto tmp = combine_into_complex(local_re, local_im);
            if constexpr (Lanes <= lanes_per_mul)
                result = tmp.template extract<Lanes>(0);
            else
                result.insert(idx_y, tmp);
        });

        return result;
    }
};

/* Real * complex
 */
template <unsigned Lanes,
          unsigned Points,
          unsigned CoeffTypeBits,
          unsigned DataTypeBits,
          typename CoeffType,
          typename DataType>
    requires(!is_complex_v<CoeffType> && is_complex_v<DataType>)
struct sliding_mul_bits_impl<Lanes, Points, 1, 1, 1, 64, CoeffTypeBits, DataTypeBits, CoeffType, DataType>
{
    // Real * complex can be decomposed in two real * real multiplications.
    // This means we always use 16x4 multiplication mode, hence 4 columns per multiply operation
    static constexpr unsigned lanes_per_mul   = 16;
    static constexpr unsigned columns_per_mul = 4;
    static constexpr unsigned num_mul         = Points / columns_per_mul;
    static_assert(Points >= columns_per_mul);
    static_assert(Points % columns_per_mul == 0);

    using coeff_type = CoeffType;
    using data_type  = DataType;
    using accum_type = accum<cacc64, Lanes>;
    using accum_int  = accum<acc64, lanes_per_mul>;

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
        using data_int_t  = utils::get_complex_component_type_t<DataType>;
        using sub_op = sliding_mul_bits_impl<lanes_per_mul,
                                             columns_per_mul,
                                             1,
                                             1,
                                             1,
                                             64,
                                             CoeffTypeBits,
                                             DataTypeBits / 2,
                                             coeff_type,
                                             data_int_t>;

        constexpr MulMacroOp MacOp = add_to_op<MulOp>();
        constexpr unsigned coeff_elems = std::max(N_Coeff, native_vector_length_v<CoeffType>);
        constexpr unsigned data_elems  = std::max(N_Data, native_vector_length_v<data_int_t>);

        using rotate_coeff = shuffle_down_rotate<CoeffType, coeff_elems>;

        accum_type result;

        // Unzipping data
        vector<data_int_t, data_elems> data_re, data_im;
        std::tie(data_re, data_im) = grow_all<data_elems>(unzip_complex(data));

        // Unzipping accum
        accum_int acc_re, acc_im;
        if constexpr (sizeof... (Acc) > 0)
            std::tie(acc_re, acc_im) = grow_all<accum_int::size()>(unzip_complex(acc)...);

        vector coeff_grown = coeff.template grow_replicate<coeff_elems>();

        utils::unroll_for<unsigned, 0, std::max(1u, Lanes / lanes_per_mul)>([&](auto idx_y) __aie_inline {
            const unsigned data_start_y  = (data_start + idx_y * lanes_per_mul) % N_Data;
            const unsigned coeff_start_y = coeff_start % coeff_elems;

            vector<CoeffType, coeff_elems> coeff_shuffle
                = rotate_coeff::run(coeff_grown, coeff_start_y);

            accum_int local_re, local_im;
            local_re = sub_op::template run<MulOp>(coeff_shuffle,
                                                   0,
                                                   coeff_sign,
                                                   data_re,
                                                   data_start_y,
                                                   data_sign,
                                                   zero_acc,
                                                   utils::get_nth<0>(acc_re, acc)
                                                        .template grow_extract<lanes_per_mul>(idx_y)...);

            local_im = sub_op::template run<MulOp>(coeff_shuffle,
                                                   0,
                                                   coeff_sign,
                                                   data_im,
                                                   data_start_y,
                                                   data_sign,
                                                   zero_acc,
                                                   utils::get_nth<0>(acc_im, acc)
                                                        .template grow_extract<lanes_per_mul>(idx_y)...);

            utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
                const unsigned data_start_x  = (data_start + columns_per_mul * idx + idx_y * lanes_per_mul) % N_Data;
                const unsigned coeff_start_x = (coeff_start + columns_per_mul * idx) % coeff_elems;

                vector<CoeffType, coeff_elems> coeff_shuffle = rotate_coeff::run(coeff_grown, coeff_start_x);

                local_re = sub_op::template run<MacOp>(coeff_shuffle,
                                                       0,
                                                       coeff_sign,
                                                       data_re,
                                                       data_start_x,
                                                       data_sign,
                                                       0,
                                                       local_re);

                local_im = sub_op::template run<MacOp>(coeff_shuffle,
                                                       0,
                                                       coeff_sign,
                                                       data_im,
                                                       data_start_x,
                                                       data_sign,
                                                       0,
                                                       local_im);
            });

            auto tmp = combine_into_complex(local_re, local_im);
            if constexpr (Lanes <= lanes_per_mul)
                result = tmp.template extract<Lanes>(0);
            else
                result.insert(idx_y, tmp);
        });

        return result;
    }
};
#endif // __AIE_API_COMPLEX_VECTOR_SUPPORT__

}

#endif
