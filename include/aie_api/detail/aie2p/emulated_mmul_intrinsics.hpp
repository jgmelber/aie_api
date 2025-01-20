// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2P_EMULATED_MMUL_INTRINSICS__HPP__
#define __AIE_API_DETAIL_AIE2P_EMULATED_MMUL_INTRINSICS__HPP__

template <typename T1, typename T2> requires(aie::detail::utils::is_one_of_v<T1, int32, uint32> &&
                                                     aie::detail::utils::is_one_of_v<T2, int32, uint32>)
inline v32acc64 mul_4x2_2x8_32bx32b(aie::vector<T1, 16> a, bool a_sign, aie::vector<T2, 16> b, bool b_sign)
{
    using hi_type = std::conditional_t<aie::detail::is_signed_v<T2>, v32int16, v32uint16>;

    v32uint16 b_lo = (v32uint16) ::shuffle(b, b, T16_32x2_lo);
    hi_type   b_hi = (hi_type)   ::shuffle(b, b, T16_32x2_hi);

    v32acc64 acc = ::mul_4x2_2x8(a, a_sign, b_hi, b_sign);
    acc = ::mac_4x2_2x8_conf(a, a_sign, b_lo, false, acc, 0, 1, 0, 0); // 1 to upshift input acc by 16

    return acc;
}

template <typename T1, typename T2> requires(aie::detail::utils::is_one_of_v<T1, int32, uint32> &&
                                                     aie::detail::utils::is_one_of_v<T2, int32, uint32>)
inline v32acc64 mac_4x2_2x8_32bx32b(aie::vector<T1, 16> a, bool a_sign, aie::vector<T2, 16> b, bool b_sign, v32acc64 acc, bool acc_zero)
{
    using hi_type = std::conditional_t<aie::detail::is_signed_v<T2>, v32int16, v32uint16>;

    v32uint16 b_lo = (v32uint16) ::shuffle(b, b, T16_32x2_lo);
    hi_type   b_hi = (hi_type)   ::shuffle(b, b, T16_32x2_hi);

    v32acc64 acc_hi = ::mul_4x2_2x8(a, a_sign, b_hi, b_sign);

    if (chess_manifest(acc_zero == false)) {
        acc = ::addmac_4x2_2x8_conf(a, a_sign, b_lo, false, acc_hi, acc, 0, 1, 0, 0, 0); // 1 to upshift acc_hi by 16
    }
    else {
        auto acc2 = ::mac_4x2_2x8_conf(a, a_sign, b_lo, false, acc_hi, 0, 1, 0, 0); // 1 to upshift acc_hi by 16
        acc = ::add_conf(acc, acc2, acc_zero, 0, 0, 0);
    }

    return acc;
}

template <typename T1, typename T2> requires(aie::detail::utils::is_one_of_v<T1, int32, uint32> &&
                                                     aie::detail::utils::is_one_of_v<T2, int16, uint16>)
inline v32acc64 mul_4x4_4x8_32bx16b(aie::vector<T1, 16> a, bool a_sign, aie::vector<T2, 32> b, bool b_sign)
{
    using hi_type = std::conditional_t<aie::detail::is_signed_v<T1>, v32int16, v32uint16>;

    v32uint16 a_lo = (v32uint16) ::shuffle(a, a, T16_32x2_lo);
    hi_type   a_hi = (hi_type)   ::shuffle(a, a, T16_32x2_hi);

    v32acc64 acc = ::mul_4x4_4x8(a_hi, a_sign, b, b_sign);
    acc = ::mac_4x4_4x8_conf(a_lo, false, b, b_sign, acc, 0, 1, 0, 0); // 1 to upshift input acc by 16

    return acc;
}

template <typename T1, typename T2> requires(aie::detail::utils::is_one_of_v<T1, int32, uint32> &&
                                                     aie::detail::utils::is_one_of_v<T2, int16, uint16>)
inline v32acc64 mac_4x4_4x8_32bx16b(aie::vector<T1, 16> a, bool a_sign, aie::vector<T2, 32> b, bool b_sign, v32acc64 acc, bool acc_zero)
{
    using hi_type = std::conditional_t<aie::detail::is_signed_v<T1>, v32int16, v32uint16>;

    v32uint16 a_lo = (v32uint16) ::shuffle(a, a, T16_32x2_lo);
    hi_type   a_hi = (hi_type)   ::shuffle(a, a, T16_32x2_hi);

    v32acc64 acc_hi = ::mul_4x4_4x8(a_hi, a_sign, b, b_sign);

    if (chess_manifest(acc_zero == false)) {
        acc = ::addmac_4x4_4x8_conf(a_lo, false, b, b_sign, acc_hi, acc, 0, 1, 0, 0, 0); // 1 to upshift acc_hi by 16
    }
    else {
        auto acc2 = ::mac_4x4_4x8_conf(a_lo, false, b, b_sign, acc_hi, 0, 1, 0, 0); // 1 to upshift acc_hi by 16
        acc = ::add_conf(acc, acc2, acc_zero, 0, 0, 0);
    }

    return acc;
}

template <typename T1, typename T2> requires(aie::detail::utils::is_one_of_v<T1, int16, uint16> &&
                                                     aie::detail::utils::is_one_of_v<T2, int32, uint32>)
inline v32acc64 mul_4x4_4x8_16bx32b(aie::vector<T1, 32> a, bool a_sign, aie::vector<T2, 32> b, bool b_sign)
{
    using hi_type = std::conditional_t<aie::detail::is_signed_v<T2>, v32int16, v32uint16>;

    v32uint16 b_lo = (v32uint16) ::shuffle(b.template extract<16>(0), b.template extract<16>(1), T16_32x2_lo);
    hi_type   b_hi = (hi_type)   ::shuffle(b.template extract<16>(0), b.template extract<16>(1), T16_32x2_hi);

    v32acc64 acc = ::mul_4x4_4x8(a, a_sign, b_hi, b_sign);
    acc = ::mac_4x4_4x8_conf(a, a_sign, b_lo, false, acc, 0, 1, 0, 0); // 1 to upshift input acc by 16

    return acc;
}

template <typename T1, typename T2> requires(aie::detail::utils::is_one_of_v<T1, int16, uint16> &&
                                                     aie::detail::utils::is_one_of_v<T2, int32, uint32>)
inline v32acc64 mac_4x4_4x8_16bx32b(aie::vector<T1, 32> a, bool a_sign, aie::vector<T2, 32> b, bool b_sign, v32acc64 acc, bool acc_zero)
{
    using hi_type = std::conditional_t<aie::detail::is_signed_v<T2>, v32int16, v32uint16>;

    v32uint16 b_lo = (v32uint16) ::shuffle(b.template extract<16>(0), b.template extract<16>(1), T16_32x2_lo);
    hi_type   b_hi = (hi_type)   ::shuffle(b.template extract<16>(0), b.template extract<16>(1), T16_32x2_hi);

    v32acc64 acc_hi = ::mul_4x4_4x8(a, a_sign, b_hi, b_sign);

    if (chess_manifest(acc_zero == false)) {
        acc = ::addmac_4x4_4x8_conf(a, a_sign, b_lo, false, acc_hi, acc, 0, 1, 0, 0, 0); // 1 to upshift acc_hi by 16
    }
    else {
        auto acc2 = ::mac_4x4_4x8_conf(a, a_sign, b_lo, false, acc_hi, 0, 1, 0, 0); // 1 to upshift acc_hi by 16
        acc = ::add_conf(acc, acc2, acc_zero, 0, 0, 0);
    }

    return acc;
}

inline v32bfloat16 extract_v4bfloat16_broadcast_to_v32bfloat16(v32bfloat16 v, int idx) {
    return  broadcast_to_v32bfloat16( extract_v4bfloat16( v, idx ));
}

inline v32bfloat16 extract_v8bfloat16_broadcast_to_v32bfloat16( v32bfloat16 v, int idx ) {
    return (v32bfloat16) broadcast_elem_128( (v16int32)v, idx );
}

inline aie::accum<accfloat, 32> mul_8x8_8x4_bf16(v64bfloat16 x, v32bfloat16 y)
{
    aie::vector<bfloat16, 32> xl = ::extract_v32bfloat16(x, 0);
    aie::vector<bfloat16, 32> xh = ::extract_v32bfloat16(x, 1);
    aie::vector<bfloat16, 32> xa = ::shuffle(xl, xh, T16_8x8_lo);
    aie::vector<bfloat16, 32> xb = ::shuffle(xl, xh, T16_8x8_hi);

    aie::vector<bfloat16, 32> x0 = ::shuffle(::extract_v8bfloat16_broadcast_to_v32bfloat16(xa, 0), T16_4x8);
    aie::vector<bfloat16, 32> x1 = ::shuffle(::extract_v8bfloat16_broadcast_to_v32bfloat16(xa, 1), T16_4x8);
    aie::vector<bfloat16, 32> x2 = ::shuffle(::extract_v8bfloat16_broadcast_to_v32bfloat16(xa, 2), T16_4x8);
    aie::vector<bfloat16, 32> x3 = ::shuffle(::extract_v8bfloat16_broadcast_to_v32bfloat16(xa, 3), T16_4x8);
    aie::vector<bfloat16, 32> x4 = ::shuffle(::extract_v8bfloat16_broadcast_to_v32bfloat16(xb, 0), T16_4x8);
    aie::vector<bfloat16, 32> x5 = ::shuffle(::extract_v8bfloat16_broadcast_to_v32bfloat16(xb, 1), T16_4x8);
    aie::vector<bfloat16, 32> x6 = ::shuffle(::extract_v8bfloat16_broadcast_to_v32bfloat16(xb, 2), T16_4x8);
    aie::vector<bfloat16, 32> x7 = ::shuffle(::extract_v8bfloat16_broadcast_to_v32bfloat16(xb, 3), T16_4x8);

    aie::vector<bfloat16, 32> y0 = ::extract_v4bfloat16_broadcast_to_v32bfloat16(y, 0);
    aie::vector<bfloat16, 32> y1 = ::extract_v4bfloat16_broadcast_to_v32bfloat16(y, 1);
    aie::vector<bfloat16, 32> y2 = ::extract_v4bfloat16_broadcast_to_v32bfloat16(y, 2);
    aie::vector<bfloat16, 32> y3 = ::extract_v4bfloat16_broadcast_to_v32bfloat16(y, 3);
    aie::vector<bfloat16, 32> y4 = ::extract_v4bfloat16_broadcast_to_v32bfloat16(y, 4);
    aie::vector<bfloat16, 32> y5 = ::extract_v4bfloat16_broadcast_to_v32bfloat16(y, 5);
    aie::vector<bfloat16, 32> y6 = ::extract_v4bfloat16_broadcast_to_v32bfloat16(y, 6);
    aie::vector<bfloat16, 32> y7 = ::extract_v4bfloat16_broadcast_to_v32bfloat16(y, 7);

    v32accfloat acc = ::mul_elem_32(x0, y0);
    acc             = ::mac_elem_32(x1, y1, acc);
    acc             = ::mac_elem_32(x2, y2, acc);
    acc             = ::mac_elem_32(x3, y3, acc);
    acc             = ::mac_elem_32(x4, y4, acc);
    acc             = ::mac_elem_32(x5, y5, acc);
    acc             = ::mac_elem_32(x6, y6, acc);
    acc             = ::mac_elem_32(x7, y7, acc);

    return acc;
}

inline aie::accum<accfloat, 32> mac_8x8_8x4_bf16(v64bfloat16 x, aie::vector<bfloat16, 32> y, v32accfloat acc, bool zero)
{
    aie::vector<bfloat16, 32> xl = ::extract_v32bfloat16(x, 0);
    aie::vector<bfloat16, 32> xh = ::extract_v32bfloat16(x, 1);
    aie::vector<bfloat16, 32> xa = ::shuffle(xl, xh, T16_8x8_lo);
    aie::vector<bfloat16, 32> xb = ::shuffle(xl, xh, T16_8x8_hi);

    aie::vector<bfloat16, 32> x0 = ::shuffle(::extract_v8bfloat16_broadcast_to_v32bfloat16(xa, 0), T16_4x8);
    aie::vector<bfloat16, 32> x1 = ::shuffle(::extract_v8bfloat16_broadcast_to_v32bfloat16(xa, 1), T16_4x8);
    aie::vector<bfloat16, 32> x2 = ::shuffle(::extract_v8bfloat16_broadcast_to_v32bfloat16(xa, 2), T16_4x8);
    aie::vector<bfloat16, 32> x3 = ::shuffle(::extract_v8bfloat16_broadcast_to_v32bfloat16(xa, 3), T16_4x8);
    aie::vector<bfloat16, 32> x4 = ::shuffle(::extract_v8bfloat16_broadcast_to_v32bfloat16(xb, 0), T16_4x8);
    aie::vector<bfloat16, 32> x5 = ::shuffle(::extract_v8bfloat16_broadcast_to_v32bfloat16(xb, 1), T16_4x8);
    aie::vector<bfloat16, 32> x6 = ::shuffle(::extract_v8bfloat16_broadcast_to_v32bfloat16(xb, 2), T16_4x8);
    aie::vector<bfloat16, 32> x7 = ::shuffle(::extract_v8bfloat16_broadcast_to_v32bfloat16(xb, 3), T16_4x8);

    aie::vector<bfloat16, 32> y0 = ::extract_v4bfloat16_broadcast_to_v32bfloat16(y, 0);
    aie::vector<bfloat16, 32> y1 = ::extract_v4bfloat16_broadcast_to_v32bfloat16(y, 1);
    aie::vector<bfloat16, 32> y2 = ::extract_v4bfloat16_broadcast_to_v32bfloat16(y, 2);
    aie::vector<bfloat16, 32> y3 = ::extract_v4bfloat16_broadcast_to_v32bfloat16(y, 3);
    aie::vector<bfloat16, 32> y4 = ::extract_v4bfloat16_broadcast_to_v32bfloat16(y, 4);
    aie::vector<bfloat16, 32> y5 = ::extract_v4bfloat16_broadcast_to_v32bfloat16(y, 5);
    aie::vector<bfloat16, 32> y6 = ::extract_v4bfloat16_broadcast_to_v32bfloat16(y, 6);
    aie::vector<bfloat16, 32> y7 = ::extract_v4bfloat16_broadcast_to_v32bfloat16(y, 7);

    acc = ::mac_elem_32_conf(x0, y0, acc, zero, 0, 0);
    acc = ::mac_elem_32     (x1, y1, acc);
    acc = ::mac_elem_32     (x2, y2, acc);
    acc = ::mac_elem_32     (x3, y3, acc);
    acc = ::mac_elem_32     (x4, y4, acc);
    acc = ::mac_elem_32     (x5, y5, acc);
    acc = ::mac_elem_32     (x6, y6, acc);
    acc = ::mac_elem_32     (x7, y7, acc);

    return acc;
}

inline aie::accum<accfloat, 32> mul_4x8_8x8_bf16(aie::vector<bfloat16, 32> x, aie::vector<bfloat16, 64> y)
{
    aie::vector<bfloat16, 32> xt = ::shuffle(x, x, T16_4x8);
    aie::vector<bfloat16, 32> x0 = ::shuffle(::extract_v4bfloat16_broadcast_to_v32bfloat16(xt, 0), T16_8x4);
    aie::vector<bfloat16, 32> x1 = ::shuffle(::extract_v4bfloat16_broadcast_to_v32bfloat16(xt, 1), T16_8x4);
    aie::vector<bfloat16, 32> x2 = ::shuffle(::extract_v4bfloat16_broadcast_to_v32bfloat16(xt, 2), T16_8x4);
    aie::vector<bfloat16, 32> x3 = ::shuffle(::extract_v4bfloat16_broadcast_to_v32bfloat16(xt, 3), T16_8x4);
    aie::vector<bfloat16, 32> x4 = ::shuffle(::extract_v4bfloat16_broadcast_to_v32bfloat16(xt, 4), T16_8x4);
    aie::vector<bfloat16, 32> x5 = ::shuffle(::extract_v4bfloat16_broadcast_to_v32bfloat16(xt, 5), T16_8x4);
    aie::vector<bfloat16, 32> x6 = ::shuffle(::extract_v4bfloat16_broadcast_to_v32bfloat16(xt, 6), T16_8x4);
    aie::vector<bfloat16, 32> x7 = ::shuffle(::extract_v4bfloat16_broadcast_to_v32bfloat16(xt, 7), T16_8x4);

    aie::vector<bfloat16, 32> y0 = ::extract_v8bfloat16_broadcast_to_v32bfloat16(y.extract<32>(0), 0);
    aie::vector<bfloat16, 32> y1 = ::extract_v8bfloat16_broadcast_to_v32bfloat16(y.extract<32>(0), 1);
    aie::vector<bfloat16, 32> y2 = ::extract_v8bfloat16_broadcast_to_v32bfloat16(y.extract<32>(0), 2);
    aie::vector<bfloat16, 32> y3 = ::extract_v8bfloat16_broadcast_to_v32bfloat16(y.extract<32>(0), 3);
    aie::vector<bfloat16, 32> y4 = ::extract_v8bfloat16_broadcast_to_v32bfloat16(y.extract<32>(1), 0);
    aie::vector<bfloat16, 32> y5 = ::extract_v8bfloat16_broadcast_to_v32bfloat16(y.extract<32>(1), 1);
    aie::vector<bfloat16, 32> y6 = ::extract_v8bfloat16_broadcast_to_v32bfloat16(y.extract<32>(1), 2);
    aie::vector<bfloat16, 32> y7 = ::extract_v8bfloat16_broadcast_to_v32bfloat16(y.extract<32>(1), 3);

    v32accfloat acc = ::mul_elem_32(x0, y0);
    acc             = ::mac_elem_32(x1, y1, acc);
    acc             = ::mac_elem_32(x2, y2, acc);
    acc             = ::mac_elem_32(x3, y3, acc);
    acc             = ::mac_elem_32(x4, y4, acc);
    acc             = ::mac_elem_32(x5, y5, acc);
    acc             = ::mac_elem_32(x6, y6, acc);
    acc             = ::mac_elem_32(x7, y7, acc);

    return acc;
}

inline aie::accum<accfloat, 32> mac_4x8_8x8_bf16(aie::vector<bfloat16, 32> x, aie::vector<bfloat16, 64> y, aie::accum<accfloat, 32> acc, bool zero)
{
    aie::vector<bfloat16, 32> xt = ::shuffle(x, x, T16_4x8);
    aie::vector<bfloat16, 32> x0 = ::shuffle(::extract_v4bfloat16_broadcast_to_v32bfloat16(xt, 0), T16_8x4);
    aie::vector<bfloat16, 32> x1 = ::shuffle(::extract_v4bfloat16_broadcast_to_v32bfloat16(xt, 1), T16_8x4);
    aie::vector<bfloat16, 32> x2 = ::shuffle(::extract_v4bfloat16_broadcast_to_v32bfloat16(xt, 2), T16_8x4);
    aie::vector<bfloat16, 32> x3 = ::shuffle(::extract_v4bfloat16_broadcast_to_v32bfloat16(xt, 3), T16_8x4);
    aie::vector<bfloat16, 32> x4 = ::shuffle(::extract_v4bfloat16_broadcast_to_v32bfloat16(xt, 4), T16_8x4);
    aie::vector<bfloat16, 32> x5 = ::shuffle(::extract_v4bfloat16_broadcast_to_v32bfloat16(xt, 5), T16_8x4);
    aie::vector<bfloat16, 32> x6 = ::shuffle(::extract_v4bfloat16_broadcast_to_v32bfloat16(xt, 6), T16_8x4);
    aie::vector<bfloat16, 32> x7 = ::shuffle(::extract_v4bfloat16_broadcast_to_v32bfloat16(xt, 7), T16_8x4);

    aie::vector<bfloat16, 32> y0 = ::extract_v8bfloat16_broadcast_to_v32bfloat16(y.extract<32>(0), 0);
    aie::vector<bfloat16, 32> y1 = ::extract_v8bfloat16_broadcast_to_v32bfloat16(y.extract<32>(0), 1);
    aie::vector<bfloat16, 32> y2 = ::extract_v8bfloat16_broadcast_to_v32bfloat16(y.extract<32>(0), 2);
    aie::vector<bfloat16, 32> y3 = ::extract_v8bfloat16_broadcast_to_v32bfloat16(y.extract<32>(0), 3);
    aie::vector<bfloat16, 32> y4 = ::extract_v8bfloat16_broadcast_to_v32bfloat16(y.extract<32>(1), 0);
    aie::vector<bfloat16, 32> y5 = ::extract_v8bfloat16_broadcast_to_v32bfloat16(y.extract<32>(1), 1);
    aie::vector<bfloat16, 32> y6 = ::extract_v8bfloat16_broadcast_to_v32bfloat16(y.extract<32>(1), 2);
    aie::vector<bfloat16, 32> y7 = ::extract_v8bfloat16_broadcast_to_v32bfloat16(y.extract<32>(1), 3);

    acc = ::mac_elem_32_conf(x0, y0, acc, zero, 0, 0);
    acc = ::mac_elem_32     (x1, y1, acc);
    acc = ::mac_elem_32     (x2, y2, acc);
    acc = ::mac_elem_32     (x3, y3, acc);
    acc = ::mac_elem_32     (x4, y4, acc);
    acc = ::mac_elem_32     (x5, y5, acc);
    acc = ::mac_elem_32     (x6, y6, acc);
    acc = ::mac_elem_32     (x7, y7, acc);

    return acc;
}

#if __AIE_API_FP32_EMULATION__

inline v16float extract_v4float_broadcast_to_v16float( v16float v, int idx ) {
    return (v16float) broadcast_elem_128( (v16int32)v, idx );
}

inline aie::accum<accfloat, 16> mul_4x8_8x4_fp32(v32float x, v32float y)
{
    aie::vector<float, 16> xl = ::extract_v16float(x, 0);
    aie::vector<float, 16> xh = ::extract_v16float(x, 1);
    aie::vector<float, 16> xa = ::shuffle(xl, xh, T32_4x8_lo);
    aie::vector<float, 16> xb = ::shuffle(xl, xh, T32_4x8_hi);

    aie::vector<float, 16> x0 = ::shuffle(::extract_v4float_broadcast_to_v16float(xa, 0), T32_4x4);
    aie::vector<float, 16> x1 = ::shuffle(::extract_v4float_broadcast_to_v16float(xa, 1), T32_4x4);
    aie::vector<float, 16> x2 = ::shuffle(::extract_v4float_broadcast_to_v16float(xa, 2), T32_4x4);
    aie::vector<float, 16> x3 = ::shuffle(::extract_v4float_broadcast_to_v16float(xa, 3), T32_4x4);
    aie::vector<float, 16> x4 = ::shuffle(::extract_v4float_broadcast_to_v16float(xb, 0), T32_4x4);
    aie::vector<float, 16> x5 = ::shuffle(::extract_v4float_broadcast_to_v16float(xb, 1), T32_4x4);
    aie::vector<float, 16> x6 = ::shuffle(::extract_v4float_broadcast_to_v16float(xb, 2), T32_4x4);
    aie::vector<float, 16> x7 = ::shuffle(::extract_v4float_broadcast_to_v16float(xb, 3), T32_4x4);

    aie::vector<float, 16> y0 = ::extract_v4float_broadcast_to_v16float(::extract_v16float(y, 0), 0);
    aie::vector<float, 16> y1 = ::extract_v4float_broadcast_to_v16float(::extract_v16float(y, 0), 1);
    aie::vector<float, 16> y2 = ::extract_v4float_broadcast_to_v16float(::extract_v16float(y, 0), 2);
    aie::vector<float, 16> y3 = ::extract_v4float_broadcast_to_v16float(::extract_v16float(y, 0), 3);
    aie::vector<float, 16> y4 = ::extract_v4float_broadcast_to_v16float(::extract_v16float(y, 1), 4);
    aie::vector<float, 16> y5 = ::extract_v4float_broadcast_to_v16float(::extract_v16float(y, 1), 5);
    aie::vector<float, 16> y6 = ::extract_v4float_broadcast_to_v16float(::extract_v16float(y, 1), 6);
    aie::vector<float, 16> y7 = ::extract_v4float_broadcast_to_v16float(::extract_v16float(y, 1), 7);

    v16accfloat acc = ::mul_elem_16(x0, y0);
    acc             = ::mac_elem_16(x1, y1, acc);
    acc             = ::mac_elem_16(x2, y2, acc);
    acc             = ::mac_elem_16(x3, y3, acc);
    acc             = ::mac_elem_16(x4, y4, acc);
    acc             = ::mac_elem_16(x5, y5, acc);
    acc             = ::mac_elem_16(x6, y6, acc);
    acc             = ::mac_elem_16(x7, y7, acc);

    return acc;
}

inline aie::accum<accfloat, 16> mac_4x8_8x4_fp32(v32float x, aie::vector<float, 32> y, v16accfloat acc, bool zero)
{
    aie::vector<float, 16> xl = ::extract_v16float(x, 0);
    aie::vector<float, 16> xh = ::extract_v16float(x, 1);
    aie::vector<float, 16> xa = ::shuffle(xl, xh, T32_4x8_lo);
    aie::vector<float, 16> xb = ::shuffle(xl, xh, T32_4x8_hi);

    aie::vector<float, 16> x0 = ::shuffle(::extract_v4float_broadcast_to_v16float(xa, 0), T32_4x4);
    aie::vector<float, 16> x1 = ::shuffle(::extract_v4float_broadcast_to_v16float(xa, 1), T32_4x4);
    aie::vector<float, 16> x2 = ::shuffle(::extract_v4float_broadcast_to_v16float(xa, 2), T32_4x4);
    aie::vector<float, 16> x3 = ::shuffle(::extract_v4float_broadcast_to_v16float(xa, 3), T32_4x4);
    aie::vector<float, 16> x4 = ::shuffle(::extract_v4float_broadcast_to_v16float(xb, 0), T32_4x4);
    aie::vector<float, 16> x5 = ::shuffle(::extract_v4float_broadcast_to_v16float(xb, 1), T32_4x4);
    aie::vector<float, 16> x6 = ::shuffle(::extract_v4float_broadcast_to_v16float(xb, 2), T32_4x4);
    aie::vector<float, 16> x7 = ::shuffle(::extract_v4float_broadcast_to_v16float(xb, 3), T32_4x4);

    aie::vector<float, 16> y0 = ::extract_v4float_broadcast_to_v16float(::extract_v16float(y, 0), 0);
    aie::vector<float, 16> y1 = ::extract_v4float_broadcast_to_v16float(::extract_v16float(y, 0), 1);
    aie::vector<float, 16> y2 = ::extract_v4float_broadcast_to_v16float(::extract_v16float(y, 0), 2);
    aie::vector<float, 16> y3 = ::extract_v4float_broadcast_to_v16float(::extract_v16float(y, 0), 3);
    aie::vector<float, 16> y4 = ::extract_v4float_broadcast_to_v16float(::extract_v16float(y, 1), 4);
    aie::vector<float, 16> y5 = ::extract_v4float_broadcast_to_v16float(::extract_v16float(y, 1), 5);
    aie::vector<float, 16> y6 = ::extract_v4float_broadcast_to_v16float(::extract_v16float(y, 1), 6);
    aie::vector<float, 16> y7 = ::extract_v4float_broadcast_to_v16float(::extract_v16float(y, 1), 7);

    acc = ::mac_elem_16_conf(x0, y0, acc, zero, 0, 0);
    acc = ::mac_elem_16     (x1, y1, acc);
    acc = ::mac_elem_16     (x2, y2, acc);
    acc = ::mac_elem_16     (x3, y3, acc);
    acc = ::mac_elem_16     (x4, y4, acc);
    acc = ::mac_elem_16     (x5, y5, acc);
    acc = ::mac_elem_16     (x6, y6, acc);
    acc = ::mac_elem_16     (x7, y7, acc);

    return acc;
}

#endif

#endif
