// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_EMULATED_MUL_INTRINSICS_HPP__
#define __AIE_API_DETAIL_AIE2_EMULATED_MUL_INTRINSICS_HPP__

#include "../accum.hpp"
#include "../vector.hpp"

namespace aie::detail {

// Any 32bit width integer (signed, unsigned)
template <typename T> concept Integral32 = utils::is_one_of_v<T, int32, uint32>;

// Any 16bit width integer (signed, unsigned)
template <typename T> concept Integral16 = utils::is_one_of_v<T, int16, uint16>;

// Integral32 x Integral16
template <Integral32 T1, Integral16 T2>
inline accum<acc64, 16> mul_conv_16x4( vector<T1, 16> a0, vector<T1, 16> a1,  bool a_sign, vector<T2, 32> b, bool b_sign )
{
    using hi_type = std::conditional_t<is_signed_v<T1>, v32int16, v32uint16>;

    vector<uint16, 32> lo = (v32uint16)::shuffle( a0, a1, T16_32x2_lo );
    hi_type            hi = (hi_type)  ::shuffle( a0, a1, T16_32x2_hi );
    accum<acc64, 16>  acc = ::mul_conv_16x4( hi, a_sign, b, b_sign );
    acc = ::mac_conv_16x4_conf( lo, false, b, b_sign, acc, 0, 1, 0, 0 );
    return acc;
}

template <Integral32 T1, Integral16 T2>
inline accum<acc64, 16> negmul_conv_16x4( vector<T1, 16> a0, vector<T1, 16> a1, bool a_sign, vector<T2, 32> b, bool b_sign )
{
    using hi_type = std::conditional_t<is_signed_v<T1>, v32int16, v32uint16>;

    vector<uint16, 32> lo = (v32uint16)::shuffle( a0, a1, T16_32x2_lo );
    hi_type            hi = (hi_type)  ::shuffle( a0, a1, T16_32x2_hi );
    accum<acc64, 16>  acc = ::mul_conv_16x4( hi, a_sign, b, b_sign );
    acc = ::mac_conv_16x4_conf( lo, false, b, b_sign, acc, 0, 1, 1, 1 );
    return acc;
}

template <Integral32 T1, Integral16 T2>
inline accum<acc64, 16> mac_conv_16x4( vector<T1, 16> a0, vector<T1, 16> a1, bool a_sign, vector<T2, 32> b, bool b_sign, accum<acc64, 16> acc )
{
    using hi_type = std::conditional_t<is_signed_v<T1>, v32int16, v32uint16>;

    vector<uint16, 32>   lo = (v32uint16)::shuffle( a0, a1, T16_32x2_lo);
    hi_type              hi = (hi_type)  ::shuffle( a0, a1, T16_32x2_hi);
    accum<acc64, 16> acc_hi = ::mul_conv_16x4( hi, a_sign, b, b_sign);
    acc = ::addmac_conv_16x4_conf( lo, false, b, b_sign, acc_hi, acc, 0, 1, 0, 0, 0);
    return acc;
}

template <Integral32 T1, Integral16 T2>
inline accum<acc64, 16> msc_conv_16x4( vector<T1, 16> a0, vector<T1, 16> a1, bool a_sign, vector<T2, 32> b, bool b_sign, accum<acc64, 16> acc )
{
    using hi_type = std::conditional_t<is_signed_v<T1>, v32int16, v32uint16>;

    vector<uint16, 32>   lo = (v32uint16)::shuffle( a0, a1, T16_32x2_lo);
    hi_type              hi = (hi_type)  ::shuffle( a0, a1, T16_32x2_hi);
    accum<acc64, 16> acc_hi = ::mul_conv_16x4( hi, a_sign, b, b_sign);
    acc = ::addmac_conv_16x4_conf( lo, false, b, b_sign, acc_hi, acc, 0, 1, 1, 1, 0);
    return acc;
}

// Integral16 x Integral32
template <Integral16 T1, Integral32 T2>
inline accum<acc64, 16> mul_conv_16x4( vector<T1, 32> a,  bool a_sign, vector<T2, 16> b0, vector<T2, 16> b1, bool b_sign )
{
    using b_hi_type = std::conditional_t<is_signed_v<T2>, int16, uint16>;

    aie::vector<uint16, 32>    b_lo = ::shuffle(b0.template cast_to<uint16>(),    b1.template cast_to<uint16>(),    T16_32x2_lo);
    aie::vector<b_hi_type, 32> b_hi = ::shuffle(b0.template cast_to<b_hi_type>(), b1.template cast_to<b_hi_type>(), T16_32x2_hi);

    accum<acc64, 16> acc = ::mul_conv_16x4(a, a_sign, b_hi, b_sign);
    acc = ::mac_conv_16x4_conf(a, a_sign, b_lo, false, acc, 0, 1, 0, 0);
    return acc;
}

template <Integral16 T1, Integral32 T2>
inline accum<acc64, 16> negmul_conv_16x4( vector<T1, 32> a, bool a_sign, vector<T2, 16> b0, vector<T2, 16> b1, bool b_sign )
{
    using b_hi_type = std::conditional_t<is_signed_v<T2>, int16, uint16>;

    aie::vector<uint16, 32>    b_lo = ::shuffle(b0.template cast_to<uint16>(),    b1.template cast_to<uint16>(),    T16_32x2_lo);
    aie::vector<b_hi_type, 32> b_hi = ::shuffle(b0.template cast_to<b_hi_type>(), b1.template cast_to<b_hi_type>(), T16_32x2_hi);

    accum<acc64, 16> acc = ::mul_conv_16x4( a, a_sign, b_hi, b_sign );
    acc = ::mac_conv_16x4_conf( a, a_sign, b_lo, false, acc, 0, 1, 1, 1 );
    return acc;
}

template <Integral16 T1, Integral32 T2>
inline accum<acc64, 16> mac_conv_16x4( vector<T1, 32> a, bool a_sign, vector<T2, 16> b0, vector<T2, 16> b1, bool b_sign, accum<acc64, 16> acc )
{
    using b_hi_type = std::conditional_t<is_signed_v<T2>, int16, uint16>;

    aie::vector<uint16, 32>    b_lo = ::shuffle(b0.template cast_to<uint16>(),    b1.template cast_to<uint16>(),    T16_32x2_lo);
    aie::vector<b_hi_type, 32> b_hi = ::shuffle(b0.template cast_to<b_hi_type>(), b1.template cast_to<b_hi_type>(), T16_32x2_hi);

    accum<acc64, 16> acc_hi = ::mul_conv_16x4( a, a_sign, b_hi, b_sign);
    acc = ::addmac_conv_16x4_conf( a, a_sign, b_lo, false, acc_hi, acc, 0, 1, 0, 0, 0);
    return acc;
}

template <Integral16 T1, Integral32 T2>
inline accum<acc64, 16> msc_conv_16x4( vector<T1, 32> a, bool a_sign, vector<T2, 16> b0, vector<T2, 16> b1, bool b_sign, accum<acc64, 16> acc )
{
    using b_hi_type = std::conditional_t<is_signed_v<T2>, int16, uint16>;

    aie::vector<uint16, 32>    b_lo = ::shuffle(b0.template cast_to<uint16>(),    b1.template cast_to<uint16>(),    T16_32x2_lo);
    aie::vector<b_hi_type, 32> b_hi = ::shuffle(b0.template cast_to<b_hi_type>(), b1.template cast_to<b_hi_type>(), T16_32x2_hi);

    accum<acc64, 16> acc_hi = ::mul_conv_16x4( a, a_sign, b_hi, b_sign);
    acc = ::addmac_conv_16x4_conf(a , a_sign, b_lo, false, acc_hi, acc, 0, 1, 1, 1, 0);
    return acc;
}

// Integral32 x Integral32
template <Integral32 T1, Integral32 T2>
inline accum<acc64, 16> mul_conv_16x4(vector<T1, 16> a0, vector<T1, 16> a1, int sgn_x, vector<T2, 16> b0, vector<T2, 16> b1, int sgn_y)
{
    using a_hi_type = std::conditional_t<is_signed_v<T1>, int16, uint16>;
    using b_hi_type = std::conditional_t<is_signed_v<T2>, int16, uint16>;

    aie::vector<uint16, 32>    a_lo = ::shuffle(a0.template cast_to<uint16>(),    a1.template cast_to<uint16>(),    T16_32x2_lo);
    aie::vector<a_hi_type, 32> a_hi = ::shuffle(a0.template cast_to<a_hi_type>(), a1.template cast_to<a_hi_type>(), T16_32x2_hi);
    aie::vector<uint16, 32>    b_lo = ::shuffle(b0.template cast_to<uint16>(),    b1.template cast_to<uint16>(),    T16_32x2_lo);
    aie::vector<b_hi_type, 32> b_hi = ::shuffle(b0.template cast_to<b_hi_type>(), b1.template cast_to<b_hi_type>(), T16_32x2_hi);

    accum<acc64, 16> acc = ::mul_conv_16x4(a_hi, sgn_x, b_hi, sgn_y);
    acc = ::mac_conv_16x4_conf(a_hi,sgn_x,b_lo,false,acc,0,1,0,0);
    acc = ::mac_conv_16x4_conf(a_lo,false,b_hi,sgn_y,acc,0,0,0,0);
    acc = ::mac_conv_16x4_conf(a_lo,false,b_lo,false,acc,0,1,0,0);
    return acc;
}

template <Integral32 T1, Integral32 T2>
inline accum<acc64, 16> negmul_conv_16x4(vector<T1, 16> a0, vector<T1, 16> a1, int sgn_x, vector<T2, 16> b0, vector<T2, 16> b1, int sgn_y)
{
    using a_hi_type = std::conditional_t<is_signed_v<T1>, int16, uint16>;
    using b_hi_type = std::conditional_t<is_signed_v<T2>, int16, uint16>;

    aie::vector<uint16, 32>    a_lo = ::shuffle(a0.template cast_to<uint16>(),    a1.template cast_to<uint16>(),    T16_32x2_lo);
    aie::vector<a_hi_type, 32> a_hi = ::shuffle(a0.template cast_to<a_hi_type>(), a1.template cast_to<a_hi_type>(), T16_32x2_hi);
    aie::vector<uint16, 32>    b_lo = ::shuffle(b0.template cast_to<uint16>(),    b1.template cast_to<uint16>(),    T16_32x2_lo);
    aie::vector<b_hi_type, 32> b_hi = ::shuffle(b0.template cast_to<b_hi_type>(), b1.template cast_to<b_hi_type>(), T16_32x2_hi);

    accum<acc64, 16> acc = ::negmul_conv_16x4(a_hi,sgn_x,b_hi,sgn_y);
    acc = ::msc_conv_16x4_conf(a_hi,sgn_x,b_lo,false,acc,0,1,0,0);
    acc = ::msc_conv_16x4_conf(a_lo,false,b_hi,sgn_y,acc,0,0,0,0);
    acc = ::msc_conv_16x4_conf(a_lo,false,b_lo,false,acc,0,1,0,0);
    return acc;
}

template <Integral32 T1, Integral32 T2>
inline accum<acc64, 16> mac_conv_16x4(vector<T1, 16> a0, vector<T1, 16> a1, int sgn_x, vector<T2, 16> b0, vector<T2, 16> b1, int sgn_y, accum<acc64, 16> acc1)
{
    using a_hi_type = std::conditional_t<is_signed_v<T1>, int16, uint16>;
    using b_hi_type = std::conditional_t<is_signed_v<T2>, int16, uint16>;

    aie::vector<uint16, 32>    a_lo = ::shuffle(a0.template cast_to<uint16>(),    a1.template cast_to<uint16>(),    T16_32x2_lo);
    aie::vector<a_hi_type, 32> a_hi = ::shuffle(a0.template cast_to<a_hi_type>(), a1.template cast_to<a_hi_type>(), T16_32x2_hi);
    aie::vector<uint16, 32>    b_lo = ::shuffle(b0.template cast_to<uint16>(),    b1.template cast_to<uint16>(),    T16_32x2_lo);
    aie::vector<b_hi_type, 32> b_hi = ::shuffle(b0.template cast_to<b_hi_type>(), b1.template cast_to<b_hi_type>(), T16_32x2_hi);

    accum<acc64, 16> acc = ::mul_conv_16x4(a_hi,sgn_x,b_hi,sgn_y);
    acc = ::mac_conv_16x4_conf(a_hi,sgn_x,b_lo,false,acc,0,1,0,0);
    acc = ::mac_conv_16x4_conf(a_lo,false,b_hi,sgn_y,acc,0,0,0,0);
    acc = ::addmac_conv_16x4_conf(a_lo,false,b_lo,false,acc,acc1,0,1,0,0,0);
    return acc;
}

template <Integral32 T1, Integral32 T2>
inline accum<acc64, 16> msc_conv_16x4(vector<T1, 16> a0, vector<T1, 16> a1, int sgn_x, vector<T2, 16> b0, vector<T2, 16> b1, int sgn_y, accum<acc64, 16> acc1)
{
    using a_hi_type = std::conditional_t<is_signed_v<T1>, int16, uint16>;
    using b_hi_type = std::conditional_t<is_signed_v<T2>, int16, uint16>;

    aie::vector<uint16, 32>    a_lo = ::shuffle(a0.template cast_to<uint16>(),    a1.template cast_to<uint16>(),    T16_32x2_lo);
    aie::vector<a_hi_type, 32> a_hi = ::shuffle(a0.template cast_to<a_hi_type>(), a1.template cast_to<a_hi_type>(), T16_32x2_hi);
    aie::vector<uint16, 32>    b_lo = ::shuffle(b0.template cast_to<uint16>(),    b1.template cast_to<uint16>(),    T16_32x2_lo);
    aie::vector<b_hi_type, 32> b_hi = ::shuffle(b0.template cast_to<b_hi_type>(), b1.template cast_to<b_hi_type>(), T16_32x2_hi);

    accum<acc64, 16> acc = ::negmul_conv_16x4(a_hi,sgn_x,b_hi,sgn_y);
    acc = ::msc_conv_16x4_conf(a_hi,sgn_x,b_lo,false,acc,0,1,0,0);
    acc = ::msc_conv_16x4_conf(a_lo,false,b_hi,sgn_y,acc,0,0,0,0);
    acc = ::addmsc_conv_16x4_conf(a_lo,false,b_lo,false,acc,acc1,0,1,0,0,0);
    return acc;
}

// _conf variants

// Integral32 x Integral16
template <Integral32 T1, Integral16 T2>
inline accum<acc64, 16> mul_conv_16x4_conf( vector<T1, 16> a0, vector<T1, 16> a1,  bool a_sign, vector<T2, 32> b, bool b_sign, int sub_mul )
{
    using hi_type = std::conditional_t<is_signed_v<T1>, v32int16, v32uint16>;

    vector<uint16, 32> lo = (v32uint16)::shuffle( a0, a1, T16_32x2_lo );
    hi_type            hi = (hi_type)  ::shuffle( a0, a1, T16_32x2_hi );
    accum<acc64, 16>  acc = ::mul_conv_16x4( hi, a_sign, b, b_sign );
    acc = ::mac_conv_16x4_conf( lo, false, b, b_sign, acc, 0, 1, sub_mul, sub_mul );
    return acc;
}

template <Integral32 T1, Integral16 T2>
inline accum<acc64, 16> negmul_conv_16x4_conf( vector<T1, 16> a0, vector<T1, 16> a1, bool a_sign, vector<T2, 32> b, bool b_sign, int sub_mul )
{
    using hi_type = std::conditional_t<is_signed_v<T1>, v32int16, v32uint16>;

    vector<uint16, 32> lo = (v32uint16)::shuffle( a0, a1, T16_32x2_lo );
    hi_type            hi = (hi_type)  ::shuffle( a0, a1, T16_32x2_hi );
    accum<acc64, 16>  acc = ::mul_conv_16x4( hi, a_sign, b, b_sign );
    acc = ::mac_conv_16x4_conf( lo, false, b, b_sign, acc, 0, 1, !sub_mul, !sub_mul );
    return acc;
}

template <Integral32 T1, Integral16 T2>
inline accum<acc64, 16> mac_conv_16x4_conf( vector<T1, 16> a0, vector<T1, 16> a1, bool a_sign, vector<T2, 32> b, bool b_sign, accum<acc64, 16> acc, int zero_acc, int sub_mul, int sub_acc )
{
    using hi_type = std::conditional_t<is_signed_v<T1>, v32int16, v32uint16>;

    vector<uint16, 32>   lo = (v32uint16)::shuffle( a0, a1, T16_32x2_lo);
    hi_type              hi = (hi_type)  ::shuffle( a0, a1, T16_32x2_hi);
    accum<acc64, 16> acc_hi = ::mul_conv_16x4_conf( hi, a_sign, b, b_sign, sub_mul);
                     acc_hi = ::mac_conv_16x4_conf(lo, false, b, b_sign, acc_hi, 0, 1, sub_mul, 0);
    return ::add_conf(acc, acc_hi, zero_acc, 0, sub_acc, 0);
}

template <Integral32 T1, Integral16 T2>
inline accum<acc64, 16> msc_conv_16x4_conf( vector<T1, 16> a0, vector<T1, 16> a1, bool a_sign, vector<T2, 32> b, bool b_sign, accum<acc64, 16> acc, int zero_acc, int sub_mul, int sub_acc )
{
    using hi_type = std::conditional_t<is_signed_v<T1>, v32int16, v32uint16>;

    vector<uint16, 32>   lo = (v32uint16)::shuffle( a0, a1, T16_32x2_lo);
    hi_type              hi = (hi_type)  ::shuffle( a0, a1, T16_32x2_hi);
    accum<acc64, 16> acc_hi = ::mul_conv_16x4_conf( hi, a_sign, b, b_sign, sub_mul);
                     acc_hi = ::mac_conv_16x4_conf(lo, false, b, b_sign, acc_hi, 0, 1, sub_mul, 0);
    return ::sub_conf(acc, acc_hi, zero_acc, 0, sub_acc, 0);
}

// Integral16 x Integral32
template <Integral16 T1, Integral32 T2>
inline accum<acc64, 16> mul_conv_16x4_conf( vector<T1, 32> a,  bool a_sign, vector<T2, 16> b0, vector<T2, 16> b1, bool b_sign, int sub_mul )
{
    using b_hi_type = std::conditional_t<is_signed_v<T2>, int16, uint16>;

    aie::vector<uint16, 32>    b_lo = ::shuffle(b0.template cast_to<uint16>(),    b1.template cast_to<uint16>(),    T16_32x2_lo);
    aie::vector<b_hi_type, 32> b_hi = ::shuffle(b0.template cast_to<b_hi_type>(), b1.template cast_to<b_hi_type>(), T16_32x2_hi);

    accum<acc64, 16> acc = ::mul_conv_16x4(a, a_sign, b_hi, b_sign);
    acc = ::mac_conv_16x4_conf(a, a_sign, b_lo, false, acc, 0, 1, sub_mul, sub_mul);
    return acc;
}

template <Integral16 T1, Integral32 T2>
inline accum<acc64, 16> negmul_conv_16x4_conf( vector<T1, 32> a, bool a_sign, vector<T2, 16> b0, vector<T2, 16> b1, bool b_sign, int sub_mul )
{
    using b_hi_type = std::conditional_t<is_signed_v<T2>, int16, uint16>;

    aie::vector<uint16, 32>    b_lo = ::shuffle(b0.template cast_to<uint16>(),    b1.template cast_to<uint16>(),    T16_32x2_lo);
    aie::vector<b_hi_type, 32> b_hi = ::shuffle(b0.template cast_to<b_hi_type>(), b1.template cast_to<b_hi_type>(), T16_32x2_hi);

    accum<acc64, 16> acc = ::mul_conv_16x4( a, a_sign, b_hi, b_sign );
    acc = ::mac_conv_16x4_conf( a, a_sign, b_lo, false, acc, 0, 1, !sub_mul, !sub_mul );
    return acc;
}

template <Integral16 T1, Integral32 T2>
inline accum<acc64, 16> mac_conv_16x4_conf( vector<T1, 32> a, bool a_sign, vector<T2, 16> b0, vector<T2, 16> b1, bool b_sign, accum<acc64, 16> acc, int zero_acc, int sub_mul, int sub_acc )
{
    using b_hi_type = std::conditional_t<is_signed_v<T2>, int16, uint16>;

    aie::vector<uint16, 32>    b_lo = ::shuffle(b0.template cast_to<uint16>(),    b1.template cast_to<uint16>(),    T16_32x2_lo);
    aie::vector<b_hi_type, 32> b_hi = ::shuffle(b0.template cast_to<b_hi_type>(), b1.template cast_to<b_hi_type>(), T16_32x2_hi);

    accum<acc64, 16> acc_hi = ::mul_conv_16x4_conf(a, a_sign, b_hi, b_sign, sub_mul);
                     acc_hi = ::mac_conv_16x4_conf(a, a_sign, b_lo, false, acc_hi, 0, 1, sub_mul, 0);
    return ::add_conf(acc, acc_hi, zero_acc, 0, sub_acc, 0);
}

template <Integral16 T1, Integral32 T2>
inline accum<acc64, 16> msc_conv_16x4_conf( vector<T1, 32> a, bool a_sign, vector<T2, 16> b0, vector<T2, 16> b1, bool b_sign, accum<acc64, 16> acc, int zero_acc, int sub_mul, int sub_acc )
{
    using b_hi_type = std::conditional_t<is_signed_v<T2>, int16, uint16>;

    aie::vector<uint16, 32>    b_lo = ::shuffle(b0.template cast_to<uint16>(),    b1.template cast_to<uint16>(),    T16_32x2_lo);
    aie::vector<b_hi_type, 32> b_hi = ::shuffle(b0.template cast_to<b_hi_type>(), b1.template cast_to<b_hi_type>(), T16_32x2_hi);

    accum<acc64, 16> acc_hi = ::mul_conv_16x4_conf(a, a_sign, b_hi, b_sign, sub_mul);
                     acc_hi = ::mac_conv_16x4_conf(a, a_sign, b_lo, false, acc_hi, 0, 1, sub_mul, 0);
    return ::sub_conf(acc, acc_hi, zero_acc, 0, sub_acc, 0);
}

// Integral32 x Integral32
template <Integral32 T1, Integral32 T2>
inline accum<acc64, 16> mul_conv_16x4_conf(vector<T1, 16> a0, vector<T1, 16> a1, int sgn_x, vector<T2, 16> b0, vector<T2, 16> b1, int sgn_y, int sub_mul )
{
    using a_hi_type = std::conditional_t<is_signed_v<T1>, int16, uint16>;
    using b_hi_type = std::conditional_t<is_signed_v<T2>, int16, uint16>;

    aie::vector<uint16, 32>    a_lo = ::shuffle(a0.template cast_to<uint16>(),    a1.template cast_to<uint16>(),    T16_32x2_lo);
    aie::vector<a_hi_type, 32> a_hi = ::shuffle(a0.template cast_to<a_hi_type>(), a1.template cast_to<a_hi_type>(), T16_32x2_hi);
    aie::vector<uint16, 32>    b_lo = ::shuffle(b0.template cast_to<uint16>(),    b1.template cast_to<uint16>(),    T16_32x2_lo);
    aie::vector<b_hi_type, 32> b_hi = ::shuffle(b0.template cast_to<b_hi_type>(), b1.template cast_to<b_hi_type>(), T16_32x2_hi);

    accum<acc64, 16> acc = ::mul_conv_16x4(a_hi, sgn_x, b_hi, sgn_y);
    acc = ::mac_conv_16x4_conf(a_hi,sgn_x,b_lo,false,acc,0,1,sub_mul,sub_mul);
    acc = ::mac_conv_16x4_conf(a_lo,false,b_hi,sgn_y,acc,0,0,sub_mul,0);
    acc = ::mac_conv_16x4_conf(a_lo,false,b_lo,false,acc,0,1,sub_mul,0);
    return acc;
}

template <Integral32 T1, Integral32 T2>
inline accum<acc64, 16> negmul_conv_16x4_conf(vector<T1, 16> a0, vector<T1, 16> a1, int sgn_x, vector<T2, 16> b0, vector<T2, 16> b1, int sgn_y, int sub_mul )
{
    using a_hi_type = std::conditional_t<is_signed_v<T1>, int16, uint16>;
    using b_hi_type = std::conditional_t<is_signed_v<T2>, int16, uint16>;

    aie::vector<uint16, 32>    a_lo = ::shuffle(a0.template cast_to<uint16>(),    a1.template cast_to<uint16>(),    T16_32x2_lo);
    aie::vector<a_hi_type, 32> a_hi = ::shuffle(a0.template cast_to<a_hi_type>(), a1.template cast_to<a_hi_type>(), T16_32x2_hi);
    aie::vector<uint16, 32>    b_lo = ::shuffle(b0.template cast_to<uint16>(),    b1.template cast_to<uint16>(),    T16_32x2_lo);
    aie::vector<b_hi_type, 32> b_hi = ::shuffle(b0.template cast_to<b_hi_type>(), b1.template cast_to<b_hi_type>(), T16_32x2_hi);

    accum<acc64, 16> acc = ::negmul_conv_16x4(a_hi,sgn_x,b_hi,sgn_y);
    acc = ::msc_conv_16x4_conf(a_hi,sgn_x,b_lo,false,acc,0,1,sub_mul,sub_mul);
    acc = ::msc_conv_16x4_conf(a_lo,false,b_hi,sgn_y,acc,0,0,sub_mul,0);
    acc = ::msc_conv_16x4_conf(a_lo,false,b_lo,false,acc,0,1,sub_mul,0);
    return acc;
}

template <Integral32 T1, Integral32 T2>
inline accum<acc64, 16> mac_conv_16x4_conf(vector<T1, 16> a0, vector<T1, 16> a1, int sgn_x, vector<T2, 16> b0, vector<T2, 16> b1, int sgn_y, accum<acc64, 16> acc1, int zero_acc, int sub_mul, int sub_acc)
{
    using a_hi_type = std::conditional_t<is_signed_v<T1>, int16, uint16>;
    using b_hi_type = std::conditional_t<is_signed_v<T2>, int16, uint16>;

    aie::vector<uint16, 32>    a_lo = ::shuffle(a0.template cast_to<uint16>(),    a1.template cast_to<uint16>(),    T16_32x2_lo);
    aie::vector<a_hi_type, 32> a_hi = ::shuffle(a0.template cast_to<a_hi_type>(), a1.template cast_to<a_hi_type>(), T16_32x2_hi);
    aie::vector<uint16, 32>    b_lo = ::shuffle(b0.template cast_to<uint16>(),    b1.template cast_to<uint16>(),    T16_32x2_lo);
    aie::vector<b_hi_type, 32> b_hi = ::shuffle(b0.template cast_to<b_hi_type>(), b1.template cast_to<b_hi_type>(), T16_32x2_hi);

    accum<acc64, 16> acc = ::mul_conv_16x4_conf(a_hi,sgn_x,b_hi,sgn_y, sub_mul);
                     acc = ::mac_conv_16x4_conf(a_hi,sgn_x,b_lo,false,acc,0,1,sub_mul,0);
                     acc = ::mac_conv_16x4_conf(a_lo,false,b_hi,sgn_y,acc,0,0,sub_mul,0);
                     acc = ::mac_conv_16x4_conf(a_lo,false,b_lo,false,acc,0,1,sub_mul,0);
    return ::add_conf(acc1, acc, zero_acc, 0, sub_acc, 0);
}

template <Integral32 T1, Integral32 T2>
inline accum<acc64, 16> msc_conv_16x4_conf(vector<T1, 16> a0, vector<T1, 16> a1, int sgn_x, vector<T2, 16> b0, vector<T2, 16> b1, int sgn_y, accum<acc64, 16> acc1, int zero_acc, int sub_mul, int sub_acc)
{
    using a_hi_type = std::conditional_t<is_signed_v<T1>, int16, uint16>;
    using b_hi_type = std::conditional_t<is_signed_v<T2>, int16, uint16>;

    aie::vector<uint16, 32>    a_lo = ::shuffle(a0.template cast_to<uint16>(),    a1.template cast_to<uint16>(),    T16_32x2_lo);
    aie::vector<a_hi_type, 32> a_hi = ::shuffle(a0.template cast_to<a_hi_type>(), a1.template cast_to<a_hi_type>(), T16_32x2_hi);
    aie::vector<uint16, 32>    b_lo = ::shuffle(b0.template cast_to<uint16>(),    b1.template cast_to<uint16>(),    T16_32x2_lo);
    aie::vector<b_hi_type, 32> b_hi = ::shuffle(b0.template cast_to<b_hi_type>(), b1.template cast_to<b_hi_type>(), T16_32x2_hi);

    accum<acc64, 16> acc = ::negmul_conv_16x4(a_hi,sgn_x,b_hi,sgn_y, sub_mul);
                     acc = ::msc_conv_16x4_conf(a_hi,sgn_x,b_lo,false,acc,0,1,sub_mul,0);
                     acc = ::msc_conv_16x4_conf(a_lo,false,b_hi,sgn_y,acc,0,0,sub_mul,0);
                     acc = ::msc_conv_16x4_conf(a_lo,false,b_lo,false,acc,0,1,sub_mul,0);
    return ::sub_conf(acc1, acc, zero_acc, 0, sub_acc, 0);
}

__aie_inline
inline v8cacc64 mac_elem_8_conf(v8cint32 a, v8cint32 b, v8cacc64 acc1,
                                int zero_acc1, int sub_mask, int sub_mul,
                                int sub_acc1) {
    v16cint16 lo = (v16cint16)::shuffle(b, undef_v8cint32(), 2);
    v16cint16 hi = (v16cint16)::shuffle(b, undef_v8cint32(), 3);
    v8cacc64 acc = ::mul_elem_8_conf(a, hi, sub_mask, sub_mul);
    if (chess_manifest(!zero_acc1)) {
        acc = ::addmac_elem_8_conf(a, 1, lo, 0, acc, acc1, 0, 1, sub_mask, sub_mul, 0, sub_acc1);
    }
    else {
        acc = ::mac_elem_8_conf(a, 1, lo, 0, acc, 0, 1, sub_mask, sub_mul, 0);
        acc = ::add_conf(acc1, acc, zero_acc1, 0, sub_acc1, 0);
    }
    return acc;
}

__aie_inline
inline v8cacc64 mul_elem_8_conf(v8cint32 a, v8cint32 b, int sub_mask, int sub_mul)
{
    int zero_acc = 1, sub_acc = 0;
    return ::aie::detail::mac_elem_8_conf(a, b, undef_v8cacc64(), zero_acc, sub_mask, sub_mul, sub_acc);
}

__aie_inline
inline v8cacc64 msc_elem_8_conf(v8cint32 a, v8cint32 b, v8cacc64 acc, int zero_acc, int sub_mask, int sub_mul, int sub_acc)
{
    // Use mac with sub_mul=1 instead of msc due to cint32 x cint32 emulation bug. See CRVO-11861
    return ::aie::detail::mac_elem_8_conf(a, b, acc, zero_acc, sub_mask, !sub_mul, sub_acc);
}

__aie_inline
inline v8cacc64 addmac_elem_8_conf(v8cint32 a, v8cint32 b, v8cacc64 acc1, v8cacc64 acc2, int zero_acc, int sub_mask, int sub_mul, int sub_acc1, int sub_acc2)
{
    acc1 = ::aie::detail::mac_elem_8_conf(a, b, acc1, zero_acc, sub_mask, sub_mul, sub_acc1);
    acc1 = ::add_conf(acc1, acc2, 0, 0, 0, sub_acc2);
    return acc1;
}

__aie_inline
inline v8cacc64 addmsc_elem_8_conf(v8cint32 a, v8cint32 b, v8cacc64 acc1, v8cacc64 acc2, int zero_acc, int sub_mask, int sub_mul, int sub_acc1, int sub_acc2)
{
    // Use mac with sub_mul=1 instead of msc due to cint32 x cint32 emulation bug. See CRVO-11861
    acc1 = ::aie::detail::mac_elem_8_conf(a, b, acc1, zero_acc, sub_mask, !sub_mul, sub_acc1);
    acc1 = ::add_conf(acc1, acc2, 0, 0, 0, sub_acc2);
    return acc1;
}

__aie_inline
inline v8cacc64 submac_elem_8_conf(v8cint32 a, v8cint32 b, v8cacc64 acc1, v8cacc64 acc2, int zero_acc, int sub_mask, int sub_mul, int sub_acc1, int sub_acc2)
{
    acc1 = ::aie::detail::mac_elem_8_conf(a, b, acc1, zero_acc, sub_mask, sub_mul, sub_acc1);
    acc1 = ::sub_conf(acc1, acc2, 0, 0, 0, sub_acc2);
    return acc1;
}

__aie_inline
inline v8cacc64 submsc_elem_8_conf(v8cint32 a, v8cint32 b, v8cacc64 acc1, v8cacc64 acc2, int zero_acc, int sub_mask, int sub_mul, int sub_acc1, int sub_acc2)
{
    // Use mac with sub_mul=1 instead of msc due to cint32 x cint32 emulation bug. See CRVO-11861
    acc1 = ::aie::detail::mac_elem_8_conf(a, b, acc1, zero_acc, sub_mask, !sub_mul, sub_acc1);
    acc1 = ::sub_conf(acc1, acc2, 0, 0, 0, sub_acc2);
    return acc1;
}

#if __AIE_API_COMPLEX_FP32_EMULATION__
__aie_inline
inline accum<caccfloat, 4> mul_2x8_8x2_t(vector<cfloat, 16> a,
                                         vector<cfloat, 16> b)
{
    vector<float, 32> a_ = a.cast_to<float>();
    vector<float, 16> tmp_re = ::shuffle(a_.extract<16>(0), a_.extract<16>(1), DINTLV_lo_32o64);
    vector<float, 16> tmp_im = ::shuffle(a_.extract<16>(0), a_.extract<16>(1), DINTLV_hi_32o64);
    vector<float, 32> A;
    A.insert(0, tmp_re.extract<8>(0));
    A.insert(1, tmp_im.extract<8>(0));
    A.insert(2, tmp_re.extract<8>(1));
    A.insert(3, tmp_im.extract<8>(1));


    aie::accum<accfloat, 16> acc = ::mul_4x8_8x4(A, b.cast_to<float>());

    aie::accum<accfloat, 16> shifted1 = ::shift(acc, acc, 3);
    aie::accum<accfloat, 16> shifted2 = ::neg(::shift(acc, acc, 5));


    aie::accum<accfloat, 16> tmp, tmp2, tmp3;
    tmp.from_vector(vector<float, 16>(::sel(shifted1.to_vector<float>(),
                                            shifted2.to_vector<float>(), 0x5555)));
    tmp2 = ::add(acc, tmp);
    tmp3.from_vector(vector<float, 16>(::shuffle(tmp2.to_vector<float>(), DINTLV_lo_128o256)));

    return tmp3.extract<8>(0).cast_to<caccfloat>();
}

__aie_inline
inline accum<caccfloat, 4> mac_2x8_8x2_conf_t(vector<cfloat, 16> a,
                                              vector<cfloat, 16> b,
                                              accum<caccfloat, 4> acc,
                                              bool zero_acc)
{
    return ::add_conf(acc, mul_2x8_8x2_t(a, b), zero_acc, 0, 0);
}

__aie_inline
inline accum<caccfloat, 4> mul_2x8_8x2_t(vector< float, 16> a,
                                         vector<cfloat, 16> b)
{
    accum<accfloat, 16> acc = ::mul_4x8_8x4(a.grow<32>(), b.cast_to<float>());
    return acc.extract<8>(0).cast_to<caccfloat>();
}

__aie_inline
inline accum<caccfloat, 4> mac_2x8_8x2_conf_t(vector< float, 16> a,
                                              vector<cfloat, 16> b,
                                              accum<caccfloat, 4> acc,
                                              bool zero_acc)
{
    return ::add_conf(acc, mul_2x8_8x2_t(a, b), zero_acc, 0, 0);
}

__aie_inline
inline accum<caccfloat, 4> mul_2x8_8x2_t(vector<cfloat, 16> a,
                                         vector< float, 16> b)
{
    vector<float, 16> zero = zeros<float, 16>::run();
    auto [tmp1, tmp2] = interleave_zip<float, 16>::run(b, zero, 1);
    vector<float, 32> tmp;
    tmp.insert(0, tmp1);
    tmp.insert(1, tmp2);
    return mul_2x8_8x2_t(a, tmp.cast_to<cfloat>());
}

__aie_inline
inline accum<caccfloat, 4> mac_2x8_8x2_conf_t(vector<cfloat, 16> a,
                                              vector< float, 16> b,
                                              accum<caccfloat, 4> acc,
                                              bool zero_acc)
{
    return ::add_conf(acc, mul_2x8_8x2_t(a, b), zero_acc, 0, 0);
}

#endif

} // namespace aie::detail

#endif
