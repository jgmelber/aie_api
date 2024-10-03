// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

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

}

#endif
