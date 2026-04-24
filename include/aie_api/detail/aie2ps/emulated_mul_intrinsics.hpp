// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2PS_EMULATED_MUL_INTRINSICS__HPP__
#define __AIE_API_DETAIL_AIE2PS_EMULATED_MUL_INTRINSICS__HPP__

#include <algorithm>

#include "../accum.hpp"
#include "../vector.hpp"
#include "../blend.hpp"
#include "../interleave.hpp"
#include "../shuffle.hpp"

// TODO: These are temporary implementations for the emulation intrinsics that will come in the future

// mul, negmul, mac, msc
// vec * vec, vec * scalar

namespace aie::detail
{

template <MulMacroOp MulOp, typename T, unsigned Elems, typename... Acc>
    requires(utils::is_one_of_v<T, float8, bfloat8> && (is_accum_v<Acc> && ...))
inline __aie_inline
accum<accfloat, Elems> mul_elem_common_t(vector<T, Elems> a, vector<T, Elems> b, Acc... acc)
{
    constexpr auto mul_op = [](){
        if constexpr (MulOp == MulMacroOp::Mul)     return [](auto &&... args) __aie_inline { return ::mul_8x8_8x8(args...); };
        if constexpr (MulOp == MulMacroOp::NegMul)  return [](auto &&... args) __aie_inline { return ::negmul_8x8_8x8(args...); };
        if constexpr (MulOp == MulMacroOp::Add_Mul) return [](auto &&... args) __aie_inline { return ::mac_8x8_8x8(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_Mul) return [](auto &&... args) __aie_inline { return ::msc_8x8_8x8(args...); };
    }();

    constexpr unsigned native_elems = 8;
    constexpr unsigned num_ops      = std::max(1u, Elems / native_elems);

    accum<accfloat, Elems> res;
    vector<T, 64> zero = zeros<T, 64>::run();

    utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
        auto a_tmp = ::sel(zero, a.template grow_extract<64>(idx / 8), 255ull << (8 * (idx % 8)));

        auto b_tmp = ::sel(zero, (typename vector<T, 64>::native_type)::broadcast_to_v16uint32(::ext_mask64(b.template grow_extract<64>(idx / 8), idx, 1)), 0x8040201008040201ull);

        accum<accfloat, 64> tmp = mul_op(a_tmp, b_tmp, utils::get_nth<0>(acc.template grow_extract<64>(idx / 8))...);
        res.insert(idx, tmp.template extract<8>(idx % 8));
    });

    return res.template extract<Elems>(0);
}

template <MulMacroOp MulOp, typename T, unsigned Elems, typename... Acc>
    requires(utils::is_one_of_v<T, float8, bfloat8> && (is_accum_v<Acc> && ...))
inline __aie_inline
accum<accfloat, Elems> mul_elem_common_t(vector<T, Elems> a, T b, Acc... acc)
{
    constexpr auto mul_op = [](){
        if constexpr (MulOp == MulMacroOp::Mul)     return [](auto &&... args) __aie_inline { return ::mul_8x8_8x8(args...); };
        if constexpr (MulOp == MulMacroOp::NegMul)  return [](auto &&... args) __aie_inline { return ::negmul_8x8_8x8(args...); };
        if constexpr (MulOp == MulMacroOp::Add_Mul) return [](auto &&... args) __aie_inline { return ::mac_8x8_8x8(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_Mul) return [](auto &&... args) __aie_inline { return ::msc_8x8_8x8(args...); };
    }();

    constexpr unsigned native_elems = 8;
    constexpr unsigned num_ops      = std::max(1u, Elems / native_elems);

    accum<accfloat, Elems> res;
    vector<T, 64> zero = zeros<T, 64>::run();
    vector<T, 64> b_tmp = ::sel(zero, broadcast<T, 64>::run(b), 0x8040201008040201ull);

    utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
        auto a_tmp = ::sel(zero, a.template grow_extract<64>(idx / 8), 255ull << (8 * (idx % 8)));

        accum<accfloat, 64> tmp = mul_op(a_tmp, b_tmp, utils::get_nth<0>(acc.template grow_extract<64>(idx / 8))...);
        res.insert(idx, tmp.template extract<8>(idx % 8));
    });

    return res.template extract<Elems>(0);
}


template <MulMacroOp MulOp, typename T, unsigned Elems, typename... Acc>
    requires(utils::is_one_of_v<T, float8, bfloat8> && (is_accum_v<Acc> && ...))
inline __aie_inline
accum<accfloat, Elems> mul_elem_conf_common_t(vector<T, Elems> a, vector<T, Elems> b, bool sub_mul, bool sub_acc, bool zero_acc, Acc... acc)
{
    constexpr auto mul_op = [](){
        if constexpr (MulOp == MulMacroOp::Mul)     return [](auto &&... args) __aie_inline { return ::mul_8x8_8x8_conf(args...); };
        if constexpr (MulOp == MulMacroOp::NegMul)  return [](auto &&... args) __aie_inline { return ::negmul_8x8_8x8_conf(args...); };
        if constexpr (MulOp == MulMacroOp::Add_Mul) return [](auto &&... args) __aie_inline { return ::mac_8x8_8x8_conf(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_Mul) return [](auto &&... args) __aie_inline { return ::msc_8x8_8x8_conf(args...); };
    }();

    constexpr unsigned native_elems = 8;
    constexpr unsigned num_ops      = std::max(1u, Elems / native_elems);

    accum<accfloat, Elems> res;
    vector<T, 64> zero = zeros<T, 64>::run();

    utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
        auto a_tmp = ::sel(zero, a.template grow_extract<64>(idx / 8), 255ull << (8 * (idx % 8)));

        auto b_tmp = ::sel(zero, (typename vector<T, 64>::native_type)::broadcast_to_v16uint32(::ext_mask64(b.template grow_extract<64>(idx / 8), idx, 1)), 0x8040201008040201ull);

        accum<accfloat, 64> tmp = mul_op(a_tmp, b_tmp,
                                         utils::get_nth<0>(acc.template grow_extract<64>(idx / 8))...,
                                         utils::get_nth<0>(zero_acc, acc)...,
                                         sub_mul,
                                         utils::get_nth<0>(sub_acc, acc)...);
        res.insert(idx, tmp.template extract<8>(idx % 8));
    });

    return res.template extract<Elems>(0);
}

template <MulMacroOp MulOp, typename T, unsigned Elems, typename... Acc>
    requires(utils::is_one_of_v<T, float8, bfloat8> && (is_accum_v<Acc> && ...))
inline __aie_inline
accum<accfloat, Elems> mul_elem_conf_common_t(vector<T, Elems> a, T b, bool sub_mul, bool sub_acc, bool zero_acc, Acc... acc)
{
    constexpr auto mul_op = [](){
        if constexpr (MulOp == MulMacroOp::Mul)     return [](auto &&... args) __aie_inline { return ::mul_8x8_8x8_conf(args...); };
        if constexpr (MulOp == MulMacroOp::NegMul)  return [](auto &&... args) __aie_inline { return ::negmul_8x8_8x8_conf(args...); };
        if constexpr (MulOp == MulMacroOp::Add_Mul) return [](auto &&... args) __aie_inline { return ::mac_8x8_8x8_conf(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_Mul) return [](auto &&... args) __aie_inline { return ::msc_8x8_8x8_conf(args...); };
    }();

    constexpr unsigned native_elems = 8;
    constexpr unsigned num_ops      = std::max(1u, Elems / native_elems);

    accum<accfloat, Elems> res;
    vector<T, 64> zero = zeros<T, 64>::run();
    vector<T, 64> b_tmp = ::sel(zero, broadcast<T, 64>::run(b), 0x8040201008040201ull);

    utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
        auto a_tmp = ::sel(zero, a.template grow_extract<64>(idx / 8), 255ull << (8 * (idx % 8)));

        accum<accfloat, 64> tmp = mul_op(a_tmp, b_tmp,
                                         utils::get_nth<0>(acc.template grow_extract<64>(idx / 8))...,
                                         utils::get_nth<0>(zero_acc, acc)...,
                                         sub_mul,
                                         utils::get_nth<0>(sub_acc, acc)...);
        res.insert(idx, tmp.template extract<8>(idx % 8));
    });

    return res.template extract<Elems>(0);
}

} // namespace aie::detail

template <typename T, unsigned Elems> requires(aie::detail::utils::is_one_of_v<T, float8, bfloat8>)
aie::accum<accfloat, Elems> mul_elem_t(aie::vector<T, Elems> a, aie::vector<T, Elems> b)
{
    return mul_elem_common_t<aie::detail::MulMacroOp::Mul>(a, b);
}

template <typename T, unsigned Elems> requires(aie::detail::utils::is_one_of_v<T, float8, bfloat8>)
aie::accum<accfloat, Elems> mul_elem_t(aie::vector<T, Elems> a, T b)
{
    return mul_elem_common_t<aie::detail::MulMacroOp::Mul>(a, b);
}

template <typename T, unsigned Elems> requires(aie::detail::utils::is_one_of_v<T, float8, bfloat8>)
aie::accum<accfloat, Elems> negmul_elem_t(aie::vector<T, Elems> a, aie::vector<T, Elems> b)
{
    return mul_elem_common_t<aie::detail::MulMacroOp::NegMul>(a, b);
}

template <typename T, unsigned Elems> requires(aie::detail::utils::is_one_of_v<T, float8, bfloat8>)
aie::accum<accfloat, Elems> negmul_elem_t(aie::vector<T, Elems> a, T b)
{
    return mul_elem_common_t<aie::detail::MulMacroOp::NegMul>(a, b);
}

template <typename T, unsigned Elems> requires(aie::detail::utils::is_one_of_v<T, float8, bfloat8>)
aie::accum<accfloat, Elems> mac_elem_t(aie::vector<T, Elems> a, aie::vector<T, Elems> b, aie::accum<accfloat, Elems> acc)
{
    return mul_elem_common_t<aie::detail::MulMacroOp::Add_Mul>(a, b, acc);
}

template <typename T, unsigned Elems> requires(aie::detail::utils::is_one_of_v<T, float8, bfloat8>)
aie::accum<accfloat, Elems> mac_elem_t(aie::vector<T, Elems> a, T b, aie::accum<accfloat, Elems> acc)
{
    return mul_elem_common_t<aie::detail::MulMacroOp::Add_Mul>(a, b, acc);
}

template <typename T, unsigned Elems> requires(aie::detail::utils::is_one_of_v<T, float8, bfloat8>)
aie::accum<accfloat, Elems> msc_elem_t(aie::vector<T, Elems> a, aie::vector<T, Elems> b, aie::accum<accfloat, Elems> acc)
{
    return mul_elem_common_t<aie::detail::MulMacroOp::Sub_Mul>(a, b, acc);
}

template <typename T, unsigned Elems> requires(aie::detail::utils::is_one_of_v<T, float8, bfloat8>)
aie::accum<accfloat, Elems> msc_elem_t(aie::vector<T, Elems> a, T b, aie::accum<accfloat, Elems> acc)
{
    return mul_elem_common_t<aie::detail::MulMacroOp::Sub_Mul>(a, b, acc);
}

#if __AIE_API_CBF16_SUPPORT__
__aie_inline
inline aie::accum<caccfloat, 8> mul_2x8_8x4_t(aie::vector<cbfloat16, 16> a,
                                              aie::vector<cbfloat16, 32> b)
{
    aie::vector<bfloat16, 32> tmp_re = ::shuffle(a.cast_to<bfloat16>(), DINTLV_lo_16o32);
    aie::vector<bfloat16, 32> tmp_im = ::shuffle(a.cast_to<bfloat16>(), DINTLV_hi_16o32);
    aie::vector<bfloat16, 32> A;
    A.insert(0, tmp_re.extract<8>(0));
    A.insert(1, tmp_im.extract<8>(0));
    A.insert(2, tmp_re.extract<8>(1));
    A.insert(3, tmp_im.extract<8>(1));

    aie::accum<caccfloat, 8> result;

    aie::accum<accfloat, 32> acc = ::mul_4x8_8x8(A, b.cast_to<bfloat16>());

    aie::accum<accfloat, 16> acc_lo = acc.extract<16>(0);
    aie::accum<accfloat, 16> acc_hi = acc.extract<16>(1);

    {
        aie::accum<accfloat, 16> shifted1 = ::shift(acc_lo, acc_lo, 7);
        aie::accum<accfloat, 16> shifted2 = ::neg(::shift(acc_lo, acc_lo, 9));
        aie::accum<accfloat, 16> tmp;
                                 tmp.from_vector(aie::vector<float, 16>(::sel(shifted1.to_vector<float>(),
                                                                              shifted2.to_vector<float>(), 0x5555)));
        aie::accum<accfloat, 16> tmp2     = ::add(acc_lo, tmp);
        result.insert(0, tmp2.extract<8>(0).cast_to<caccfloat>());
    }
    {
        aie::accum<accfloat, 16> shifted1 = ::shift(acc_hi, acc_hi, 7);
        aie::accum<accfloat, 16> shifted2 = ::neg(::shift(acc_hi, acc_hi, 9));
        aie::accum<accfloat, 16> tmp;
                                 tmp.from_vector(aie::vector<float, 16>(::sel(shifted1.to_vector<float>(),
                                                                              shifted2.to_vector<float>(), 0x5555)));
        aie::accum<accfloat, 16> tmp2     = ::add(acc_hi, tmp);
        result.insert(1, tmp2.extract<8>(0).cast_to<caccfloat>());
    }

    return result;
}

__aie_inline
inline aie::accum<caccfloat, 8> mac_2x8_8x4_conf_t(aie::vector<cbfloat16, 16> a,
                                                   aie::vector<cbfloat16, 32> b,
                                                   aie::accum<caccfloat, 8> acc,
                                                   bool zero_acc)
{
    return ::add_conf(acc, mul_2x8_8x4_t(a, b), zero_acc, 0, 0);
}

__aie_inline
inline aie::accum<caccfloat, 8> mul_2x8_8x4_t(aie::vector< bfloat16, 16> a,
                                              aie::vector<cbfloat16, 32> b)
{
    aie::accum<accfloat, 32> acc = ::mul_4x8_8x8(a.grow<32>(), b.cast_to<bfloat16>());
    return acc.extract<16>(0).cast_to<caccfloat>();
}

__aie_inline
inline aie::accum<caccfloat, 8> mac_2x8_8x4_conf_t(aie::vector< bfloat16, 16> a,
                                                   aie::vector<cbfloat16, 32> b,
                                                   aie::accum<caccfloat, 8> acc,
                                                   bool zero_acc)
{
    return ::add_conf(acc, mul_2x8_8x4_t(a, b), zero_acc, 0, 0);
}

__aie_inline
inline aie::accum<caccfloat, 8> mul_2x8_8x4_t(aie::vector<cbfloat16, 16> a,
                                              aie::vector< bfloat16, 32> b)
{
    aie::vector<bfloat16, 32> zero = aie::zeros<bfloat16, 32>();
    auto [tmp1, tmp2] = aie::detail::interleave_zip<bfloat16, 32>::run(b, zero, 1);
    aie::vector<bfloat16, 64> tmp;
    tmp.insert(0, tmp1);
    tmp.insert(1, tmp2);
    return mul_2x8_8x4_t(a, tmp.cast_to<cbfloat16>());
}

__aie_inline
inline aie::accum<caccfloat, 8> mac_2x8_8x4_conf_t(aie::vector<cbfloat16, 16> a,
                                                   aie::vector< bfloat16, 32> b,
                                                   aie::accum<caccfloat, 8> acc,
                                                   bool zero_acc)
{
    return ::add_conf(acc, mul_2x8_8x4_t(a, b), zero_acc, 0, 0);
}

#endif

#endif
